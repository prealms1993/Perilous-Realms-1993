#ifndef _TICKSH_
#define _TICKSH_

#include <sys/types.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


typedef struct tick_pulse {
  ulong timeout;	/* How long to sleep in loop */
  ulong zone;		/* How many pulses before zone update */
  ulong mobile;		/* How many pulses before mobile update */
  ulong river;		/* How many pulses before river update */
  ulong audio;		/* How many pulses before audio update */
  ulong teleport;	/* How many pulses before teleport updates */
  ulong violence;	/* How many pulses before violence update */
  ulong fall;		/* How many pulses before fall update - NOT USED */
  ulong drown;		/* How many pulses before drown update - NOT USED */
  ulong player;		/* How many pulses before player update */
  ulong healer;		/* How many pulses before healer update */ 
  ulong tics;		/* Current tick number */
  ulong pulse;		/* Current pulse number */
  ulong maxpulse;	/* Max pulse number before it is reset */
} tick_pulse;


typedef struct general_timer {
  ulong timer[2];	/* timer = timer[1] - timer[0] */
} general_timer;

typedef struct tick_timer {
  general_timer game_loop;	/* How long it takes to finish the game_loop */
  general_timer zone;		/* How long to perform the zone_update */
  general_timer mobile;		/* How long to perform the mobile_activity */
  general_timer river;		/* How long to perform the down_river */
  general_timer audio;		/* How long to perform the MakeSound */
  general_timer teleport;	/* How long to perform the Teleport */
  general_timer violence;	/* How long to perform the perform_violence */
  general_timer fall;		/* How long to perform the all_fall_down */
  general_timer drown;		/* How long to perform the glug_glug_glug */
  general_timer player;		/* How long it takes to do the player tick */
  general_timer weather;	/* How long to perform the weather_and_time */
  general_timer affect;		/* How long to perform the point_update */
  general_timer point;
} tick_timer;


#ifdef _TICKC_
tick_pulse tick_pulse_data;
tick_timer tick_timer_data;
int TIMER = 0;
#else
extern tick_pulse tick_pulse_data;
extern tick_timer tick_timer_data;
extern int TIMER;
#endif


#define DEF_PULSE_TIMEOUT	250000
#define DEF_PULSE_ZONE    	240		/* once/min */
#define DEF_PULSE_MOBILE	36
#define DEF_PULSE_RIVER    	8
#define DEF_PULSE_AUDIO   	240
#define DEF_PULSE_TELEPORT  	4
#define DEF_PULSE_VIOLENCE	12
#define DEF_PULSE_FALL		4 		/* Not used */
#define DEF_PULSE_DROWN		4 		/* Not used */
#define DEF_PULSE_PLAYER	300
#define DEF_PULSE_HEALER	30
#define DEF_TICS		0
#define DEF_PULSE		0
#define DEF_PULSE_MAX		2400

#define PULSE_TIMEOUT		tick_pulse_data.timeout
#define PULSE_ZONE		tick_pulse_data.zone
#define PULSE_MOBILE		tick_pulse_data.mobile
#define PULSE_RIVER		tick_pulse_data.river
#define PULSE_AUDIO		tick_pulse_data.audio
#define PULSE_TELEPORT		tick_pulse_data.teleport
#define PULSE_VIOLENCE		tick_pulse_data.violence
#define PULSE_FALL		tick_pulse_data.fall
#define PULSE_DROWN		tick_pulse_data.drown
#define PULSE_PLAYER		tick_pulse_data.player
#define PULSE_HEALER		tick_pulse_data.healer
#define TICS			tick_pulse_data.tics
#define PULSE			tick_pulse_data.pulse
#define PULSE_MAX		tick_pulse_data.maxpulse


#define TIMER_GAME		tick_timer_data.game_loop
#define TIMER_ZONE		tick_timer_data.zone
#define TIMER_MOBILE		tick_timer_data.mobile
#define TIMER_RIVER		tick_timer_data.river
#define TIMER_AUDIO		tick_timer_data.audio
#define TIMER_TELEPORT		tick_timer_data.teleport
#define TIMER_VIOLENCE		tick_timer_data.violence
#define TIMER_FALL		tick_timer_data.fall
#define TIMER_DROWN		tick_timer_data.drown
#define TIMER_PLAYER		tick_timer_data.player
#define TIMER_WEATHER		tick_timer_data.weather
#define TIMER_AFFECT		tick_timer_data.affect
#define TIMER_POINT		tick_timer_data.point

#define INIT_TIMER(x)  x.timer[0] = x.timer[1] = 0
#define SET_TIMER(x)   x.timer[0] = x.timer[1]; x.timer[1] = time(0)
#define DIFF_TIMER(x)  x.timer[1] - x.timer[0]


#define WAIT_SEC       	4
#define WAIT_ROUND 	4

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR  60
#define SECS_PER_MUD_DAY   (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR  (17*SECS_PER_MUD_MONTH)

#endif
