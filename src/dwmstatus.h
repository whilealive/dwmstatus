/* ==================================================================
 * FILE     dwmstatus.h
 * MACHINE  all
 * INFO     dwm statusline in C
 *          based on the suckless dwmstatus project under
 *          git://git.suckless.org/dwmstatus,
 *          and alsa stuff from https://github.com/Unia/dwmst
 *
 * DATE     05.07.2017
 * OWNER    Bischofberger
 * ==================================================================
 */

#include <alsa/asoundlib.h>  /* needed for snd_mixer_t type */

#define INTERVAL            60  /* seconds */

#define BATT_NOW            "/sys/class/power_supply/BAT0/energy_now"
#define BATT_FULL           "/sys/class/power_supply/BAT0/energy_full"
#define BATT_STATUS         "/sys/class/power_supply/BAT0/status"
#define BATT_LOW            5  /* percent */

#define MAIL_FAST           "/Maildir/fastmail/INBOX/new"
#define MAIL_BMZ            "/Maildir/bmz/INBOX/new"
#define MAIL_BMZ_EX         "/Maildir/bmz/Erstellungsteam/new"
#define MAIL_UZH            "/Maildir/uzh-pseudo/uzh/new"

#define ROUND_UNSIGNED(d)   ( (int) ((d) + ((d) > 0 ? 0.5 : -0.5)) )

typedef struct {
  char *mail_fast;
  char *mail_bmz;
  char *mail_bmz_ex;
  char *mail_uzh;
} mailbox;

void die();
char * smprintf(char *fmt, ...);
char *getHomeDir();
char * getbattery();
snd_mixer_t *initvol();
char * getvol(snd_mixer_t *handle);
char *get_nmail(int n_args, ...);
char *gettime();
char *getTimeAndDate();
mailbox initmail();
void setstatus(char *str);
