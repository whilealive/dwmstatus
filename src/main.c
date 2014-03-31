/* ==================================================================
 * FILE     main.c (Project: dwmstatus)
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

#include <X11/Xlib.h>
#include "dwmstatus.h"

static Display *dpy;

void die(const char *errmsg)
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

void setstatus(char *str)
{
    XStoreName(dpy, DefaultRootWindow(dpy), str);
    XSync(dpy, False);
}

/* LAPTOP VERSION */
int main()
{
    if (!(dpy = XOpenDisplay(NULL))) {
        die("dwmstatus: cannot open display.\n");
    }
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
    box = initmail("laptop");

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
    XCloseDisplay(dpy);
    return 0;
}

/* DESKTOP VERSION:
 * uncomment and recompile
 */
/*
int main()
{
    if (!(dpy = XOpenDisplay(NULL))) {
        die("dwmstatus: cannot open display.\n");
    }
    char *status;
    char *time;
    char *new_fastmail;
    char *new_uzh;
    char *new_zhaw;

    mailbox box;
    box = initmail("desktop");

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
    XCloseDisplay(dpy);
    return 0;
}
*/
