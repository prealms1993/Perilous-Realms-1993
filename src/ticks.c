/*
** File: tick.c , tick utitilites.
*/

#include <stdio.h>
#include <fcns.h>
#include <externs.h>

#define _TICKC_
#include "ticks.h"
#undef _TICKC_


typedef struct tick_field_data {
    char *name;
    int type;
    ulong min_val;
    ulong max_val;
} tick_field_data;

static tick_field_data tickfield[] = {
/*  0 */  { "timeout",		1,	0,	(1<<30)-1 },
/*  1 */  { "zone",		1,	1,	32000     },
/*  2 */  { "mobile",		1,	1,	32000     },
/*  3 */  { "river",		1,	1,	32000     },
/*  4 */  { "audio",		1,	1,	32000     },
/*  5 */  { "teleport",		1,	1,	32000     },
/*  6 */  { "violence",		1,	1,	32000     },
/*  7 */  { "fall",		1,	1,	32000     },
/*  8 */  { "drown",		1,	1,	32000     },
/*  9 */  { "player",		1,	1,	32000     },
/* 10 */  { "healer",		1,	1,	32000     },
/* 11 */  { "tics",		0,	0,	0         },
/* 12 */  { "pulse",		0,	0,	0         },
/* 13 */  { "maxpulse",		1,	1,	32000     },
/* 14 */  { "all",		0,	0,	0         },
          { "",  	       -1,      0,      0         }
};

void do_ticks(struct char_data *ch, char *arg, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    int which, show, set, reset;
    long val = 0;

    arg = one_argument(arg, buf);
    if (!*buf) {
	tick_usage(ch, 0);
	return;
    }

    show = set = reset = 0;
    if (is_abbrev(buf, "show")) show = 1;
    else if (is_abbrev(buf, "set")) set = 1;
    else if (is_abbrev(buf, "reset")) reset = 1;
    else { tick_usage(ch, 0); return; }

    arg = one_argument(arg, buf);

    which = -1;
    while (tickfield[++which].type >= 0) {
	if (is_abbrev(buf, tickfield[which].name)) break;
    }

    if (show) which = 14;
    else if (tickfield[which].type < 0) {
	if (set) tick_usage(ch, 1);
	else tick_usage(ch, 2);
	return;
    }

    arg = one_argument(arg, buf);

    if (set) {
	if (which == 14) {
	    sendf(ch,"Not allowed to set all!\n");
	    return;
	}

        val = atol(buf);
        if (!*buf || val < tickfield[which].min_val || 
		     val > tickfield[which].max_val) {
	    sendf(ch,"Looking for a value in the range of %ld to %ld.\n",
                  tickfield[which].min_val, tickfield[which].max_val);
	    return;
	}
    }

    switch (which) {
      case 0:  /* timeout */
	if (set) PULSE_TIMEOUT = val;
	if (reset) PULSE_TIMEOUT = DEF_PULSE_TIMEOUT;
	break;
      case 1:  /* zone */
	if (set) PULSE_ZONE = val;
	if (reset) PULSE_ZONE = DEF_PULSE_ZONE;
	break;
      case 2:  /* mobile */
	if (set) PULSE_MOBILE = val;
	if (reset) PULSE_MOBILE = DEF_PULSE_MOBILE;
	break;
      case 3:  /* river */
	if (set) PULSE_RIVER = val;
	if (reset) PULSE_RIVER = DEF_PULSE_RIVER;
	break;
      case 4:  /* audio */
	if (set) PULSE_AUDIO = val;
	if (reset) PULSE_AUDIO = DEF_PULSE_AUDIO;
	break;
      case 5:  /* teleport */
	if (set) PULSE_TELEPORT = val;
	if (reset) PULSE_TELEPORT = DEF_PULSE_TELEPORT;
	break;
      case 6:  /* violence */
	if (set) PULSE_VIOLENCE = val;
	if (reset) PULSE_VIOLENCE = DEF_PULSE_VIOLENCE;
	break;
      case 7:  /* fall */
	if (set) PULSE_FALL = val;
	if (reset) PULSE_FALL = DEF_PULSE_FALL;
	break;
      case 8:  /* drown */
	if (set) PULSE_DROWN = val;
	if (reset) PULSE_DROWN = DEF_PULSE_DROWN;
	break;
      case 9:  /* player */
	if (set) PULSE_PLAYER = val;
	if (reset) PULSE_PLAYER = DEF_PULSE_PLAYER;
	break;
      case 10:  /* healer */
	if (set) PULSE_HEALER = val;
	if (reset) PULSE_HEALER = DEF_PULSE_HEALER;
	break;
      case 11:  /* tics */
	if (set) TICS = val;
	if (reset) TICS = DEF_TICS;
	break;
      case 12:  /* pulse */
	if (set) PULSE = val;
	if (reset) PULSE = DEF_PULSE;
	break;
      case 13:  /* maxpulse */
	if (set) PULSE_MAX = val;
	if (reset) PULSE_MAX = DEF_PULSE_MAX;
	break;
      case 14:  /* all */
	if (set) sendf(ch,"Not allowed to set all!\n");
	if (reset) tick_reset();
	if (show) tick_show(ch);
	break;
      default:
	sendf(ch,"Invalid flag sent!\n");
    }

    if (!show) sendf(ch,"Ok.\n");
}

void tick_usage(struct char_data *ch, int flag)
{
    register int i, count;

    sendf(ch,"Syntax: ticks show | set <field> <value> | reset <field>\n");
    sendf(ch,"Valid fields for the set and reset options are:\n");

    for (i = 0, count = 0; tickfield[i].type >= 0; i++)
        sendf(ch,"%18s%s",tickfield[i].name,++count%4==0?"\n":"");
    sendf(ch,"\n");
}

void tick_reset(void)
{
    PULSE_TIMEOUT =	DEF_PULSE_TIMEOUT;
    PULSE_ZONE =	DEF_PULSE_ZONE;
    PULSE_MOBILE =	DEF_PULSE_MOBILE;
    PULSE_RIVER =	DEF_PULSE_RIVER;
    PULSE_AUDIO =	DEF_PULSE_AUDIO;
    PULSE_TELEPORT =	DEF_PULSE_TELEPORT;
    PULSE_VIOLENCE =	DEF_PULSE_VIOLENCE;
    PULSE_FALL =	DEF_PULSE_FALL;		/* Not Used */
    PULSE_DROWN =	DEF_PULSE_DROWN;	/* Not Used */
    PULSE_PLAYER =	DEF_PULSE_PLAYER;
    PULSE_HEALER =	DEF_PULSE_HEALER;
    TICS =		DEF_TICS;
    PULSE =		DEF_PULSE;
    PULSE_MAX =		DEF_PULSE_MAX;
}

void tick_show(struct char_data *ch)
{
    sendf(ch,"                Tick Statistics\n\n");

    sendf(ch,"  Field Name     Current Value  Default Value\n");
    sendf(ch,"---------------  -------------  -------------\n");
    sendf(ch,"Select Timeout %15ld%15ld\n",PULSE_TIMEOUT,DEF_PULSE_TIMEOUT);
    sendf(ch,"Pulse Zone     %15ld%15ld\n",PULSE_ZONE,DEF_PULSE_ZONE);
    sendf(ch,"Pulse Mobile   %15ld%15ld",PULSE_MOBILE,DEF_PULSE_MOBILE);
    sendf(ch,"    For Select Timeout and all\n");
    sendf(ch,"Pulse River    %15ld%15ld",PULSE_RIVER,DEF_PULSE_RIVER);
    sendf(ch,"    of the Pulse ticks, the lower\n");
    sendf(ch,"Pulse Audio    %15ld%15ld",PULSE_AUDIO,DEF_PULSE_AUDIO);
    sendf(ch,"    the number, the more often\n");
    sendf(ch,"Pulse Telport  %15ld%15ld",PULSE_TELEPORT,DEF_PULSE_TELEPORT);
    sendf(ch,"    it happens.\n");
    sendf(ch,"Pulse Violence %15ld%15ld\n",PULSE_VIOLENCE,DEF_PULSE_VIOLENCE);
    sendf(ch,"Pulse Fall     %15ld%15ld\n",PULSE_FALL,DEF_PULSE_FALL);
    sendf(ch,"Pulse Drown    %15ld%15ld\n",PULSE_DROWN,DEF_PULSE_DROWN);
    sendf(ch,"Pulse Player   %15ld%15ld\n",PULSE_PLAYER,DEF_PULSE_PLAYER);
    sendf(ch,"Pulse Healer   %15ld%15ld\n\n",PULSE_HEALER,DEF_PULSE_HEALER);

    sendf(ch,"Checkpoint Tics%15ld%15ld\n",TICS,DEF_TICS);
    sendf(ch,"Current Pulse  %15ld%15ld\n",PULSE,DEF_PULSE);
    sendf(ch,"Maximum Pulse  %15ld%15ld\n",PULSE_MAX,DEF_PULSE_MAX);
}

void do_timer(struct char_data *ch, char *arg, int cmd)
{
    char buf[MAX_STRING_LENGTH];

    arg = one_argument(arg, buf);

    if (!*buf) timer_show(ch);
    else if (!strcmp(buf, "1")) {
	TIMER = 1;
	sendf(ch,"Timer is now ON.\n");
    } else if (!str_cmp(buf, "0")) {
	TIMER = 0;
	sendf(ch,"Timer is now OFF.\n");
    } else if (*buf) timer_usage(ch);
}

void timer_usage(struct char_data *ch)
{
    sendf(ch,"Syntax:  ticks [0 | 1]\n");
}

void timer_reset(void)
{
    INIT_TIMER(TIMER_ZONE);
    INIT_TIMER(TIMER_MOBILE);
    INIT_TIMER(TIMER_RIVER);
    INIT_TIMER(TIMER_AUDIO);
    INIT_TIMER(TIMER_TELEPORT);
    INIT_TIMER(TIMER_VIOLENCE);
    INIT_TIMER(TIMER_FALL);
    INIT_TIMER(TIMER_DROWN);
    INIT_TIMER(TIMER_PLAYER);
    INIT_TIMER(TIMER_WEATHER);
    INIT_TIMER(TIMER_AFFECT);
    INIT_TIMER(TIMER_POINT);
    INIT_TIMER(TIMER_GAME);
}

void timer_show(struct char_data *ch)
{
    sendf(ch,"           Timer Statistics\n\n");

    sendf(ch,"   Function Name      Total Duration\n");
    sendf(ch,"-------------------  --------------\n");
    sendf(ch,"Zone Update        %15ld\n", DIFF_TIMER(TIMER_ZONE));
    sendf(ch,"Mobile Activity    %15ld\n", DIFF_TIMER(TIMER_MOBILE));
    sendf(ch,"River Flow         %15ld\n", DIFF_TIMER(TIMER_RIVER));
    sendf(ch,"Sound Generation   %15ld\n", DIFF_TIMER(TIMER_AUDIO));
    sendf(ch,"Teleport Rooms     %15ld\n", DIFF_TIMER(TIMER_TELEPORT));
    sendf(ch,"Violence Update    %15ld\n\n", DIFF_TIMER(TIMER_VIOLENCE));

    sendf(ch,"Player Tick        %15ld\n", DIFF_TIMER(TIMER_PLAYER));
    sendf(ch," * Weather Update  %15ld\n", DIFF_TIMER(TIMER_WEATHER));
    sendf(ch," * Affect Update   %15ld\n", DIFF_TIMER(TIMER_AFFECT));
    sendf(ch," * Point Update    %15ld\n\n", DIFF_TIMER(TIMER_POINT));

    sendf(ch,"Game Loop          %15ld\n\n", DIFF_TIMER(TIMER_GAME));

    if (TIMER) sendf(ch,"Timer is currently ON.\n");
    else sendf(ch,"Timer is currently OFF.\n");
}
