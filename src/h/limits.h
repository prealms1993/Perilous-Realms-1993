/* ************************************************************************
*  file: limits.h , Limit/Gain control module             Part of DIKUMUD *
*  Usage: declaration of title type                                       *
************************************************************************* */
#ifndef _LIMITSH_
#define _LIMITSH_

#define MAX_MOVE 3000
#define MAX_MANA 800
#define MAX_HIT 3000
#define MAX_SLONG ((1<<30)-1)

/* LEVEL_DIFF is for the max level difference when grouping */
/* LEVEL_SPREAD is the multiplier so that higher lvls can group
 * with more lvls */
/* To recreate old conditions set diff to 0 and spread to 0
 * to allow a spread of only 50 lvls, set spread to 0
 * to allow a spread of +/- 1 * a person's level set diff to 2000
 * and spread to 1 (and so on) */
# define MAX_LEVEL_DIFF		50
# define MAX_LEVEL_SPREAD	2
# define MIN_CHOSEN_LEVEL_DIFF	50	/* lawful/corrupt kill spread */

#define MAX_GAIN_EXP 300000
#endif
