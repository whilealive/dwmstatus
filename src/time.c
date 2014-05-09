/* ==================================================================
 * FILE     time.c (Project: dwmstatus)
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
 * DATE     03.04.2014
 * OWNER    Bischofberger
 * ==================================================================
 */

#include <time.h>

#include "dwmstatus.h"

char *gettime()
{
    char outstr[20];
    time_t t;
    struct tm *tmp;

    t = time(NULL);
    tmp = localtime(&t);

    if (tmp == NULL) {
        die("localtime failure");
    }
    if (strftime(outstr, sizeof(outstr), "%R", tmp) == 0) {
        die("strftime returned 0");
    }
    return smprintf("%s", outstr);
}