/* ==================================================================
 * FILE     dwmstatus.c
 * MACHINE  laptop
 * INFO     dwm statusline in C
 *          based on the suckless dwmstatus project under
 *          git://git.suckless.org/dwmstatus,
 *          and alsa stuff from https://github.com/Unia/dwmst
 *
 *          Don't need time in statusbar since tmux does that for me
 *
 * DATE     18.03.2014
 * OWNER    Bischofberger
 * ==================================================================
 */

/* TODO: - implement a gtk+ warning window if bat is really low
 *       - write as a daemon to prevent from crashing when suspending
 *       - maybe with time anyway...?
 */

#include <unistd.h>  /* sleep() ... */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <alsa/asoundlib.h>
#include <dirent.h>  /* check directory for new files */

#include <X11/Xlib.h>

#define INTERVAL            10  /* seconds */
#define BATT_NOW            "/sys/class/power_supply/BAT1/charge_now"
#define BATT_FULL           "/sys/class/power_supply/BAT1/charge_full"
#define BATT_STATUS         "/sys/class/power_supply/BAT1/status"
#define ROUND_UNSIGNED(d)   ( (int) ((d) + ((d) > 0 ? 0.5 : -0.5)) )

static Display *dpy;

typedef struct {
    bool checkbat;
    bool checkvol;
    char *mail_fast;
    char *mail_uzh;
    char *mail_zhaw;
} init;

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

char * getbattery(bool checkbat)
{
    if (checkbat) {
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
            if (strcmp(status,"Charging") == 0)
                s = '+';
            if (strcmp(status,"Discharging") == 0)
                s = '-';
            if (strcmp(status,"Full") == 0)
                s = '=';
            return smprintf("[bat %c%ld] ", s,(full/(now/100)));
        }
        else
            return smprintf("");
    }
    else  /* return empty string if machine does not support bat checking */
        return smprintf("");
}

snd_mixer_t *initvol(bool checkvol)
{
    snd_mixer_t *handle;

    if(checkvol) {
        snd_mixer_open(&handle, 0);
        snd_mixer_attach(handle, "default");
        snd_mixer_selem_register(handle, NULL, NULL);
        snd_mixer_load(handle);
    }
    return handle;
}

char * getvol(bool checkvol, snd_mixer_t *handle) {
    if (checkvol) {
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

        if(mute == 0)
            return smprintf("[MUTE]");
        return smprintf("[vol %d]", ROUND_UNSIGNED((vol*10.0)/max));
        //return smprintf("%d%%", (vol * 100) / max);
    }
    else
        return smprintf("");
}

char *get_nmail(char *directory, char *label)
{
    /* directory : Maildir path 
    * return label : number_of_new_mails
    */
    int n = 0;
    DIR* dir = NULL;
    struct dirent* rf = NULL;

    dir = opendir(directory);  /* try to open directory */
    if (dir == NULL)
        perror("");

    while ((rf = readdir(dir)) != NULL) {  /*count number of files */
        if (strcmp(rf->d_name, ".") != 0 && strcmp(rf->d_name, "..") != 0)
            n++;
    }
    closedir(dir);

   return smprintf("%s%d", label, n);
}

init initstatus(char *machine)
{
    init tmp;

    if(!strcmp("--laptop", machine)) {
        tmp.checkbat = true;
        tmp.checkvol = true;
        tmp.mail_fast = "/home/laptop/Maildir/fastmail/INBOX/new";
        tmp.mail_uzh  = "/home/laptop/Maildir/uzh-pseudo/INBOX_uzh/new";
        tmp.mail_zhaw = "/home/laptop/Maildir/zhaw-pseudo/INBOX_zhaw/new";
        return tmp;
    }
    else if(!strcmp("--desktop", machine)) {
        tmp.checkbat = false;
        tmp.checkvol = false;
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
    if (argc != 2)
        die("usage: dwmstatus [--laptop/--desktop]\n");
    if (!(dpy = XOpenDisplay(NULL)))
        die("dwmstatus: cannot open display.\n");

    init mach;
    mach = initstatus(argv[1]);

    char *status;
    char *bat;
    char *new_fastmail;
    char *new_uzh;
    char *new_zhaw;
    char *vol;

    snd_mixer_t *handle;
    handle = initvol(mach.checkvol);

    for (;;sleep(INTERVAL)) {
        bat = getbattery(mach.checkbat);
        vol = getvol(mach.checkvol, handle);
        new_fastmail = get_nmail(mach.mail_fast, "");
        new_uzh= get_nmail(mach.mail_uzh, "");
        new_zhaw= get_nmail(mach.mail_zhaw, "");

        status = smprintf("[mail %s|%s|%s] %s%s", new_fastmail, new_uzh, new_zhaw, bat, vol);
        setstatus(status);

        free(bat);
        free(vol);
        free(new_fastmail);
        free(new_uzh);
        free(new_zhaw);
        free(status);
    }

    XCloseDisplay(dpy);

    return 0;
}
