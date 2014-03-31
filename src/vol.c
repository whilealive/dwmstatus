/* ==================================================================
 * FILE     vol.c (Project: dwmstatus)
 * MACHINE  all
 * INFO     dwm statusline in C
 *          based on the suckless dwmstatus project under
 *          git://git.suckless.org/dwmstatus,
 *          and alsa stuff from https://github.com/Unia/dwmst
 *
 *          Attention: dwmstatus is likely to crash when suspending 
 *          computer while it is not in sleep() state. Therefore, 
 *          we have to run a custom systemd service (or bash script) 
 *          which kills it (if necessary) and re-runs it after 
 *          waking up.
 *
 * DATE     31.03.2014
 * OWNER    Bischofberger
 * ==================================================================
 */

#include <unistd.h>  /* sleep() ... */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <time.h>
#include <dirent.h>  /* check directory for new files */
#include "dwmstatus.h"

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
