/* ==================================================================
 * FILE     batt.c  (Project: dwmstatus)
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  /* for calling "susp"-script if battery is low */

#include "dwmstatus.h"

//#define SUSPEND_ON_LOW_BAT

void suspendOnLowBat(long const current)
{
  if (current <= BATT_LOW) {
    execl("/bin/bash", "bash", "/usr/local/bin/susp", "-s", (char *) NULL);
  }
}

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
    long current = full/(now/100);
#ifdef SUSPEND_ON_LOW_BAT
    suspendOnLowBat(current);
#endif
    return smprintf("[bat %c%ld] ",s , current);
  }
  else {
    return smprintf("");
  }
}
