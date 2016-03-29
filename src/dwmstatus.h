/* ==================================================================
 * FILE     dwmstatus.h
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
 * DATE     23.03.2016
 * OWNER    Bischofberger
 * ==================================================================
 */

#include <alsa/asoundlib.h>  /* needed for snd_mixer_t type */

#define INTERVAL            60  /* seconds */

#define BATT_NOW            "/sys/class/power_supply/BAT0/energy_now"
#define BATT_FULL           "/sys/class/power_supply/BAT0/energy_full"
#define BATT_STATUS         "/sys/class/power_supply/BAT0/status"
#define BATT_LOW            5  /* percent */

#define MAIL_LAP_FAST       "/home/laptop2/Maildir/fastmail/INBOX/new"
#define MAIL_LAP_BMZ        "/home/laptop2/Maildir/bmz/INBOX/new"
#define MAIL_LAP_UZH        "/home/laptop2/Maildir/uzh-pseudo/uzh/new"

#define MAIL_DESK_FAST      "/home/desktop/Maildir/fastmail/INBOX/new"
#define MAIL_DESK_BMZ       "/home/desktop/Maildir/bmz/INBOX/new"
#define MAIL_DESK_UZH       "/home/desktop/Maildir/uzh-pseudo/uzh/new"

#define ROUND_UNSIGNED(d)   ( (int) ((d) + ((d) > 0 ? 0.5 : -0.5)) )

typedef struct {
    char *mail_fast;
    char *mail_bmz;
    char *mail_uzh;
} mailbox;

void die();
char * smprintf(char *fmt, ...);
char * getbattery();
snd_mixer_t *initvol();
char * getvol(snd_mixer_t *handle);
char *get_nmail(char *directory);
char *gettime();
char *getTimeAndDate();
mailbox initmail(char *machine);
void setstatus(char *str);
