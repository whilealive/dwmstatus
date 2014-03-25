/* ==================================================================
 * FILE     dwmstatus.c
 * MACHINE  laptop
 * INFO     dwm statusline in C
 *          based on the suckless dwmstatus project under
 *          git://git.suckless.org/dwmstatus,
 *          and alsa stuff from https://github.com/Unia/dwmst
 *
 *          Attention: dwmstatus is likely to crash when suspending 
 *          computer while it is not in sleep() state. Therefore, 
 *          we have to run a custom systemd service which kills 
 *          (if necessary) and reruns it after waking up.
 *
 * DATE     20.03.2014
 * OWNER    Bischofberger
 * ==================================================================
 */

/* TODO: - implement a gtk+ warning window if bat is really low
 */

#include <unistd.h>  /* sleep() ... */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <time.h>
#include <dirent.h>  /* check directory for new files */

#include <X11/Xlib.h>

#define INTERVAL            60  /* seconds */
#define BATT_NOW            "/sys/class/power_supply/BAT1/charge_now"
#define BATT_FULL           "/sys/class/power_supply/BAT1/charge_full"
#define BATT_STATUS         "/sys/class/power_supply/BAT1/status"
#define ROUND_UNSIGNED(d)   ( (int) ((d) + ((d) > 0 ? 0.5 : -0.5)) )

static Display *dpy;

typedef struct {
    char *mail_fast;
    char *mail_uzh;
    char *mail_zhaw;
} mailbox;

static void die(const char *errmsg)
{
    fputs(errmsg, stderr);
    exit(EXIT_FAILURE);
}

char * smprintf(char *fmt, ...)
{
    va_list fmtargs;
    char *ret;
    int len;

    va_start(fmtargs, fmt);
    len = vsnprintf(NULL, 0, fmt, fmtargs);
    va_end(fmtargs);

    ret = malloc(++len);
    if (ret == NULL) {
        perror("malloc");
        exit(1);
    }

    va_start(fmtargs, fmt);
    vsnprintf(ret, len, fmt, fmtargs);
    va_end(fmtargs);

    return ret;
}

char * getbattery()
{
    long full, now = 0;
    char *status = malloc(sizeof(char)*12);
    char s = '?';
    FILE *fp = NULL;
    if ((fp = fopen(BATT_NOW, "r"))) {
        fscanf(fp, "%ld\n", &full);
        fclose(fp);
        fp = fopen(BATT_FULL, "r");
        fscanf(fp, "%ld\n", &now);
        fclose(fp);
        fp = fopen(BATT_STATUS, "r");
        fscanf(fp, "%s\n", status);
        fclose(fp);
        if (strcmp(status,"Charging") == 0) {
            s = '+';
        }
        if (strcmp(status,"Discharging") == 0) {
            s = '-';
        }
        if (strcmp(status,"Full") == 0) {
            s = '=';
        }
        return smprintf("[bat %c%ld] ", s,(full/(now/100)));
    }
    else {
        return smprintf("");
    }
}

snd_mixer_t *initvol()
{
    snd_mixer_t *handle;

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, "default");
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    return handle;
}

char * getvol(snd_mixer_t *handle) 
{
    int mute = 0;
    long vol = 0, max = 0, min = 0;
    snd_mixer_elem_t *pcm_mixer, *max_mixer;
    snd_mixer_selem_id_t *vol_info, *mute_info;

    snd_mixer_handle_events(handle);
    snd_mixer_selem_id_malloc(&vol_info);
    snd_mixer_selem_id_malloc(&mute_info);
    snd_mixer_selem_id_set_name(vol_info, "Master");
    snd_mixer_selem_id_set_name(mute_info, "Master");
    pcm_mixer = snd_mixer_find_selem(handle, vol_info);
    max_mixer = snd_mixer_find_selem(handle, mute_info);
    snd_mixer_selem_get_playback_volume_range(pcm_mixer, &min, &max);
    snd_mixer_selem_get_playback_volume(pcm_mixer, 0, &vol);
    snd_mixer_selem_get_playback_switch(max_mixer, 0, &mute);
    snd_mixer_selem_id_free(vol_info);
    snd_mixer_selem_id_free(mute_info);

    if (mute == 0) {
        return smprintf("[MUTE]");
    }
    return smprintf("[vol %d]", ROUND_UNSIGNED((vol*10.0)/max));
    //return smprintf("%d%%", (vol * 100) / max);
}

char *get_nmail(char *directory)
{
    /* directory : Maildir path */
    int n = 0;
    DIR* dir = NULL;
    struct dirent* rf = NULL;

    dir = opendir(directory);  /* try to open directory */
    if (dir == NULL) {
        perror("");
    }
    while ((rf = readdir(dir)) != NULL) {  /*count number of files */
        if (strcmp(rf->d_name, ".") != 0 && strcmp(rf->d_name, "..") != 0) {
            n++;
        }
    }
    closedir(dir);

   return smprintf("%d", n);
}

char *gettime()
{
    char outstr[20];
    time_t t;
    struct tm *tmp;

    t = time(NULL);
    tmp = localtime(&t);

    if (tmp == NULL) {
        die("localtime failure");
    }
    if (strftime(outstr, sizeof(outstr), "%R", tmp) == 0) {
        die("strftime returned 0");
    }
    return smprintf("%s", outstr);
}

mailbox initmail(char *machine)
{
    mailbox tmp;

    if(!strcmp("--laptop", machine)) {
        tmp.mail_fast = "/home/laptop/Maildir/fastmail/INBOX/new";
        tmp.mail_uzh  = "/home/laptop/Maildir/uzh-pseudo/INBOX_uzh/new";
        tmp.mail_zhaw = "/home/laptop/Maildir/zhaw-pseudo/INBOX_zhaw/new";
        return tmp;
    }
    else if(!strcmp("--desktop", machine)) {
        tmp.mail_fast = "/home/desktop/Maildir/fastmail/INBOX/new";
        tmp.mail_uzh  = "/home/desktop/Maildir/uzh-pseudo/INBOX_uzh/new";
        tmp.mail_zhaw = "/home/desktop/Maildir/zhaw-pseudo/INBOX_zhaw/new";
        return tmp;
    }
    else {
        die("usage: dwmstatus [--laptop/--desktop]\n");
    }
    return tmp;
}

void setstatus(char *str)
{
    XStoreName(dpy, DefaultRootWindow(dpy), str);
    XSync(dpy, False);
}

int main(int argc, char *argv[])
{
    if (!(dpy = XOpenDisplay(NULL))) {
        die("dwmstatus: cannot open display.\n");
    }
    if (argc != 2) {
        die("usage: dwmstatus [--laptop/--desktop]\n");
    }
    if (!strcmp("--laptop", argv[1])) {
        char *status;
        char *bat;
        char *vol;
        char *time;
        char *new_fastmail;
        char *new_uzh;
        char *new_zhaw;

        snd_mixer_t *handle;
        handle = initvol();

        mailbox box;
        box = initmail(argv[1]);

        for (;;sleep(INTERVAL)) {
            bat = getbattery();
            vol = getvol(handle);
            time = gettime();
            new_fastmail = get_nmail(box.mail_fast);
            new_uzh= get_nmail(box.mail_uzh);
            new_zhaw= get_nmail(box.mail_zhaw);

            status = smprintf("[mail %s|%s|%s] %s%s %s", new_fastmail, new_uzh, new_zhaw, bat, vol, time);
            setstatus(status);

            free(new_zhaw);
            free(new_uzh);
            free(new_fastmail);
            free(time);
            free(vol);
            free(bat);
            free(status);
        }
    }
    else if (!strcmp("--desktop", argv[1])) {
        char *status;
        char *time;
        char *new_fastmail;
        char *new_uzh;
        char *new_zhaw;

        mailbox box;
        box = initmail(argv[1]);

        for (;;sleep(INTERVAL)) {
            time = gettime();
            new_fastmail = get_nmail(box.mail_fast);
            new_uzh= get_nmail(box.mail_uzh);
            new_zhaw= get_nmail(box.mail_zhaw);

            status = smprintf("[mail %s|%s|%s] %s", new_fastmail, new_uzh, new_zhaw, time);
            setstatus(status);

            free(new_zhaw);
            free(new_uzh);
            free(new_fastmail);
            free(time);
            free(status);
        }
    }
    else {
        die("usage: dwmstatus [--laptop/--desktop]\n");
    }

    XCloseDisplay(dpy);

    return 0;
}
