/* ==================================================================
 * FILE     main_desktop.c (Project: dwmstatus)
 * MACHINE  all
 * INFO     dwm statusline in C
 *          based on the suckless dwmstatus project under
 *          git://git.suckless.org/dwmstatus,
 *          and alsa stuff from https://github.com/Unia/dwmst
 *
 * DATE     05.07.2017
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

int main()
{
  if (!(dpy = XOpenDisplay(NULL))) {
    die("dwmstatus: cannot open display.\n");
  }

  snd_mixer_t *handle = initvol();
  mailbox box = initmail();

  for (;;sleep(INTERVAL)) {
    char* bat = getbattery();
    char* vol = getvol(handle);
    char* time = getTimeAndDate();
    char* new_fastmail = get_nmail(1, box.mail_fast);
    char* new_bmz = get_nmail(2, box.mail_bmz, box.mail_bmz_ex);
    char* new_uzh = get_nmail(1, box.mail_uzh);

    char* status = smprintf("[mail %s|%s|%s] %s%s %s", new_fastmail, new_bmz, new_uzh, bat, vol, time);
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
