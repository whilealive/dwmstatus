/* ==================================================================
 * FILE     dwmstatus.c
 * MACHINE  desktop
 * INFO     dwm statusline in C
 *          based on the suckless dwmstatus project under
 *          git://git.suckless.org/dwmstatus,
 *          including parts from https://github.com/Unia/dwmst/
 *
 * DATE     03.03.2014
 * OWNER    Bischofberger
 * ==================================================================
 */

/* TODO:    - get a bit into detail with the alsa stuff
 */

#include <unistd.h>  /* sleep() ... */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>  /* va_list ... */
#include <string.h>
#include <alsa/asoundlib.h>

#include <X11/Xlib.h>

#define INTERVAL      90
#define BATT_NOW      "/sys/class/power_supply/BAT1/charge_now"
#define BATT_FULL     "/sys/class/power_supply/BAT1/charge_full"
#define BATT_STATUS   "/sys/class/power_supply/BAT1/status"

static Display *dpy;

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
        if (strcmp(status,"Charging") == 0)
            s = '+';
        if (strcmp(status,"Discharging") == 0)
            s = '-';
        if (strcmp(status,"Full") == 0)
            s = '=';
        return smprintf("VOL %c%ld%%", s,(full/(now/100)));
    }
    else return smprintf("");
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
    return smprintf("VOL %d%%", (vol * 100) / max);
}

void setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

int main(void)
{
    char *status;
    char *bat;
    char *vol;
    snd_mixer_t *handle;

    if (!(dpy = XOpenDisplay(NULL))) {
        fprintf(stderr, "dwmstatus: cannot open display.\n");
        return 1;
    }

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, "default");
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    for (;;sleep(INTERVAL)) {
        bat = getbattery();
        vol = getvol(handle);
        status = smprintf("[%s] [%s]", bat, vol);
        setstatus(status);
        free(bat);
        free(vol);
        free(status);
    }

    XCloseDisplay(dpy);

    return 0;
}
