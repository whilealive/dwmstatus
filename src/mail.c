/* ==================================================================
 * FILE     mail.c (Project: dwmstatus)
 * MACHINE  all
 * INFO     dwm statusline in C
 *          based on the suckless dwmstatus project under
 *          git://git.suckless.org/dwmstatus,
 *          and alsa stuff from https://github.com/Unia/dwmst
 *
 * DATE     02.07.2017
 * OWNER    Bischofberger
 * ==================================================================
 */

#define _GNU_SOURCE  /* asprintf */
#include <stdio.h>
#include <string.h>
#include <dirent.h>  /* check directory for new files */

#include "dwmstatus.h"


mailbox initmail()
{
    const char* homedir = getHomeDir();

    mailbox tmp;

    asprintf(&tmp.mail_fast, "%s%s", homedir, MAIL_FAST);
    asprintf(&tmp.mail_bmz, "%s%s", homedir, MAIL_BMZ);
    asprintf(&tmp.mail_uzh, "%s%s", homedir, MAIL_UZH);

    return tmp;
}

char *get_nmail(char *directory)
{
    /* directory : Maildir path */
    int n = 0;
    DIR* dir = NULL;
    struct dirent* rf = NULL;

    dir = opendir(directory);  /* try to open directory */
    if (dir == NULL) {
        return smprintf("e");
    }
    while ((rf = readdir(dir)) != NULL) {  /*count number of files */
        if (strcmp(rf->d_name, ".") != 0 && strcmp(rf->d_name, "..") != 0) {
            n++;
        }
    }
    closedir(dir);

   return smprintf("%d", n);
}
