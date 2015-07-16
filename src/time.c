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
 * DATE     14.07.2015
 * OWNER    Bischofberger
 * ==================================================================
 */

#include <time.h>

#include "dwmstatus.h"

char* getTimeAndDate()
{
    time_t t = time(NULL);
    struct tm *tmp = localtime(&t);

    if (tmp == NULL) {
        die("localtime failure");
    }

    char outstr[40];
    if (strftime(outstr, sizeof(outstr), "%e.%m.%y %R", tmp) == 0) {
        die("strftime returned 0");
    }
    return smprintf("%s", outstr);
}


char* gettime()
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
