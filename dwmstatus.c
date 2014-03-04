/* ==================================================================
 * FILE     dwmstatus.c
 * MACHINE  desktop
 * INFO     dwm statusline in C
 *          based on the suckless dwmstatus project under
 *          git://git.suckless.org/dwmstatus,
 *
 * DATE     04.03.2014
 * OWNER    Bischofberger
 * ==================================================================
 */

/* TODO:    - get a bit into detail with the alsa stuff
 */

#include <unistd.h>  /* sleep() ... */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <alsa/asoundlib.h>

#include <X11/Xlib.h>

#define INTERVAL      90
#define BUFLENGTH     100
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
        return smprintf("%c%ld%%", s,(full/(now/100)));
    }
    else return smprintf("");
}

/*
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
*/

static snd_mixer_t *alsainit(const char *card)
{
    snd_mixer_t *handle;

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);
    return handle;
}

static snd_mixer_elem_t *alsamixer(snd_mixer_t *handle, const char *mixer)
{
    snd_mixer_selem_id_t *sid;

    //snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_malloc(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, mixer);
    return snd_mixer_find_selem(handle, sid);
}

static int ismuted(snd_mixer_elem_t *mixer)
{
    int on;

    snd_mixer_selem_get_playback_switch(mixer, SND_MIXER_SCHN_MONO, &on);
    return !on;
}

static int getvol(snd_mixer_elem_t *mixer)
{
    long vol, min, max;

    snd_mixer_selem_get_playback_volume_range(mixer, &min, &max);
    snd_mixer_selem_get_playback_volume(mixer, SND_MIXER_SCHN_MONO, &vol);

    vol = vol < max ? (vol > min ? vol : min) : max;
    return vol * 100.0 / max + 0.5;
}

void setstatus(char *str)
{
    XStoreName(dpy, DefaultRootWindow(dpy), str);
    XSync(dpy, False);
}

int main(void)
{
    //char *status;
    char status[BUFLENGTH];
    //char *bat;
    //char *vol;
    //snd_mixer_t *handle;
    snd_mixer_t *alsa;
    snd_mixer_elem_t *mixer;

    if (!(dpy = XOpenDisplay(NULL)))
        die("dwmstatus: cannot open display.\n");
    if (!(alsa = alsainit("default")))
        die("dwmstatus: cannot initialize alsa\n");
    if (!(mixer = alsamixer(alsa, "Master")))
        die("dwmstatus: cannot get mixer\n");

    /*
    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, "default");
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);
    */

    while (1) {
        snprintf(status, sizeof(status), "[BAT %s] [VOL %d%%%s]", getbattery(),
                getvol(mixer), ismuted(mixer) ? " MUTE" : "");
        setstatus(status);

        snd_mixer_wait(alsa, INTERVAL);
        snd_mixer_handle_events(alsa);
    }

    snd_mixer_close(alsa);
    XCloseDisplay(dpy);
    return 0;


    /*
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
    */
}
