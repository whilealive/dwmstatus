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
 * DATE     12.03.2014
 * OWNER    Bischofberger
 * ==================================================================
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

#define INTERVAL      60  /* seconds */
#define BATT_NOW      "/sys/class/power_supply/BAT1/charge_now"
#define BATT_FULL     "/sys/class/power_supply/BAT1/charge_full"
#define BATT_STATUS   "/sys/class/power_supply/BAT1/status"

static Display *dpy;

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

/* argument for machine type checking,
 * does nothing if type is desktop
 */
char * getbattery(bool laptop)
{
    if (!laptop)
        return smprintf("");
    else {
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
            return smprintf(" [bat %c%ld%%]", s,(full/(now/100)));
        }
        else return smprintf("");
    }
}

char * getvol(snd_mixer_t *handle) {
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
        return smprintf("MUTE");
    return smprintf("%d%%", (vol * 100) / max);
}

char *get_nmail(char *directory, char *label)
{
    /* directory : Maildir path 
    * return label : number_of_new_mails
    */
    int n = 0;
    DIR* dir = NULL;
    struct dirent* rf = NULL;

    dir = opendir(directory); /* try to open directory */
    if (dir == NULL)
        perror("");

    while ((rf = readdir(dir)) != NULL) /*count number of file*/
    {
        if (strcmp(rf->d_name, ".") != 0 &&
            strcmp(rf->d_name, "..") != 0)
            n++;
    }
    closedir(dir);

   return smprintf("%s%d", label, n);
}

void setstatus(char *str)
{
    XStoreName(dpy, DefaultRootWindow(dpy), str);
    XSync(dpy, False);
}

int main(int argc, char *argv[])
{
    bool laptop;

    if (argc != 2)
        die("usage: dwmstatus [-laptop/-desktop]\n");
    else if (!strcmp("-laptop", argv[1])) {
        laptop = true;
    }
    else if (!strcmp("-desktop", argv[1])) {
        laptop = false;
    }
    else
        die("usage: dwmstatus [-laptop/-desktop]\n");

    char *status;
    char *bat;
    char *new_fastmail;
    char *new_uzh;
    char *new_zhaw;
    char *vol;
    snd_mixer_t *handle;

    if (!(dpy = XOpenDisplay(NULL)))
        die("dwmstatus: cannot open display.\n");

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, "default");
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    for (;;sleep(INTERVAL)) {
        bat = getbattery(laptop);
        vol = getvol(handle);
        new_fastmail = get_nmail("/home/laptop/Maildir/fastmail/INBOX/new", "");
        new_uzh= get_nmail("/home/laptop/Maildir/uzh-pseudo/INBOX_uzh/new", "");
        new_zhaw= get_nmail("/home/laptop/Maildir/zhaw-pseudo/INBOX_zhaw/new", "");

        status = smprintf("[mail %s|%s|%s]%s [vol %s]", new_fastmail, new_uzh, new_zhaw, bat, vol);
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
