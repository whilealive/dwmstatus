/* ==================================================================
 * FILE     main_desktop.c (Project: dwmstatus)
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
 *          Depending on the machine we use different main.c versions
 *          For laptop: un-comment "laptop" section, for desktop do
 *          so with the "desktop" section
 *
 * DATE     26.09.2015
 * OWNER    Bischofberger
 * ==================================================================
 */

#include <unistd.h>  /* sleep() ... */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <X11/Xlib.h>

#include "dwmstatus.h"

static Display *dpy;

void setstatus(char *str)
{
    XStoreName(dpy, DefaultRootWindow(dpy), str);
    XSync(dpy, False);
}

/* DESKTOP VERSION */
int main()
{
    if (!(dpy = XOpenDisplay(NULL))) {
        die("dwmstatus: cannot open display.\n");
    }

    snd_mixer_t *handle = initvol();
    mailbox box = initmail("desktop");

    for (;;sleep(INTERVAL)) {
        char* vol = getvol(handle);
        char* time = getTimeAndDate();
        char* new_fastmail = get_nmail(box.mail_fast);
        char* new_bmz = get_nmail(box.mail_bmz);
        char* new_uzh = get_nmail(box.mail_uzh);

        char* status = smprintf("[mail %s|%s|%s] %s %s", new_fastmail, new_bmz, new_uzh, vol, time);
        setstatus(status);

        free(new_fastmail);
        free(new_bmz);
        free(new_uzh);
        free(time);
        free(vol);
        free(status);
    }
    XCloseDisplay(dpy);
    return 0;
}
