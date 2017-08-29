/* ==================================================================
 * FILE     mail.c (Project: dwmstatus)
 * MACHINE  all
 * INFO     dwm statusline in C
 *          based on the suckless dwmstatus project under
 *          git://git.suckless.org/dwmstatus,
 *          and alsa stuff from https://github.com/Unia/dwmst
 *
 * DATE     19.08.2017
 * OWNER    Bischofberger
 * ==================================================================
 */

#define _GNU_SOURCE  /* asprintf */
#include <stdio.h>
#include <string.h>
#include <dirent.h>  /* check directory for new files */
#include <stdarg.h>

#include "dwmstatus.h"


mailbox initmail()
{
  const char* homedir = getHomeDir();

  mailbox tmp;

  asprintf(&tmp.mail_fast, "%s%s", homedir, MAIL_FAST);
  asprintf(&tmp.mail_bmz, "%s%s", homedir, MAIL_BMZ);
  asprintf(&tmp.mail_bmz_ex, "%s%s", homedir, MAIL_BMZ_EX);
  asprintf(&tmp.mail_uzh, "%s%s", homedir, MAIL_UZH);

  return tmp;
}

char *get_nmail(int n_args, ...)
{
  va_list valist;
  va_start(valist, n_args);

  int n = 0;

  for (int i = 0 ; i < n_args ; ++i)
  {
    DIR* dir = NULL;
    struct dirent* rf = NULL;

    dir = opendir(va_arg(valist, char *));  /* try to open directory */
    if (dir == NULL) {
      return smprintf("e");
    }
    while ((rf = readdir(dir)) != NULL) {  /*count number of files */
      if (strcmp(rf->d_name, ".") != 0 && strcmp(rf->d_name, "..") != 0) {
        n++;
      }
    }
    closedir(dir);
  }

  va_end(valist);
  return smprintf("%d", n);
}
