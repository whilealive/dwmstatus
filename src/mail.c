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

char *get_nmail(char *directory)
{
    /* directory : Maildir path */
    int n = 0;
    DIR* dir = NULL;
    struct dirent* rf = NULL;

    dir = opendir(directory);  /* try to open directory */
    if (dir == NULL) {
        perror("");
    }
    while ((rf = readdir(dir)) != NULL) {  /*count number of files */
        if (strcmp(rf->d_name, ".") != 0 && strcmp(rf->d_name, "..") != 0) {
            n++;
        }
    }
    closedir(dir);

   return smprintf("%d", n);
}
