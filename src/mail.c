/* ==================================================================
 * FILE     mail.c (Project: dwmstatus)
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
 * DATE     26.09.2015
 * OWNER    Bischofberger
 * ==================================================================
 */

/* TODO: better error handling
 */

#include <string.h>
#include <dirent.h>  /* check directory for new files */

#include "dwmstatus.h"

mailbox initmail(char *machine)
{
    mailbox tmp;

    if(!strcmp("laptop", machine)) {
        tmp.mail_fast = MAIL_LAP_FAST;
        tmp.mail_bmz  = MAIL_LAP_BMZ;
        tmp.mail_uzh  = MAIL_LAP_UZH;
        return tmp;
    }
    else if(!strcmp("desktop", machine)) {
        tmp.mail_fast = MAIL_DESK_FAST;
        tmp.mail_bmz  = MAIL_DESK_BMZ;
        tmp.mail_uzh  = MAIL_DESK_UZH;
        return tmp;
    }
    else {
        die("initmail: invalid argument\n");
    }
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
