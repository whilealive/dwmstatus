/* ==================================================================
 * FILE     dwmstatus.c
 * MACHINE  desktop
 * INFO     dwm statusline in C
 *          based on the suckless dwmstatus project under
 *          git://git.suckless.org/dwmstatus
 *
 * DATE     03.03.2014
 * OWNER    Bischofberger
 * ==================================================================
 */

/* TODO:   - check for power supply AC on
 */

#include <unistd.h>  /* sleep() ... */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>  /* va_list ... */
#include <string.h>

#include <X11/Xlib.h>

#define BATT_NOW     "/sys/class/power_supply/BAT1/charge_now"
#define BATT_FULL    "/sys/class/power_supply/BAT1/charge_full"
#define BATT_STATUS  "/sys/class/power_supply/BAT1/status"

static Display *dpy;

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
        if (strcmp(status,"Charging") == 0)
            s = '+';
        if (strcmp(status,"Discharging") == 0)
            s = '-';
        if (strcmp(status,"Full") == 0)
            s = '=';
        return smprintf("%c%ld%%", s,(full/(now/100)));
    }
    else return smprintf("");
}

void setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

int main(void)
{
    char *status;
    char *bat;

    if (!(dpy = XOpenDisplay(NULL))) {
        fprintf(stderr, "dwmstatus: cannot open display.\n");
        return 1;
    }

    for (;;sleep(90)) {
        bat = getbattery();
        status = smprintf("[BAT %s]", bat);
        setstatus(status);
        free(bat);
        free(status);
    }

    XCloseDisplay(dpy);

    return 0;
}
