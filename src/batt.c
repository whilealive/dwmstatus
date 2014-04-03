/* ==================================================================
 * FILE     batt.c  (Project: dwmstatus)
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dwmstatus.h"

char * getbattery()
{
    long full, now = 0;
    char *status = malloc(sizeof(char)*12);
    char s = '?';
    FILE *fp = NULL;
    if ((fp = fopen(BATT_NOW, "r"))) {
        fscanf(fp, "%ld\n", &full);
        fclose(fp);
        fp = fopen(BATT_FULL, "r");
        fscanf(fp, "%ld\n", &now);
        fclose(fp);
        fp = fopen(BATT_STATUS, "r");
        fscanf(fp, "%s\n", status);
        fclose(fp);
        if (strcmp(status,"Charging") == 0) {
            s = '+';
        }
        if (strcmp(status,"Discharging") == 0) {
            s = '-';
        }
        if (strcmp(status,"Full") == 0) {
            s = '=';
        }
        return smprintf("[bat %c%ld] ", s,(full/(now/100)));
    }
    else {
        return smprintf("");
    }
}
