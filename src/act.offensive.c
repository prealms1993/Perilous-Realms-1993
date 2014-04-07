/* ************************************************************************
 *  file: act.offensive.c , Implementation of commands.    Part of DIKUMUD *
 *  Usage : Offensive commands.                                            *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include <fcns.h>
#include <externs.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <spells.h>
#include <limits.h>
#include <race.h>
#include <ticks.h>

int CORRUPT(struct char_data *ch,struct char_data *v)
{
  if (ch==v) return(1);
  if (IS_IMMORTAL (ch)) return (1);
  if (IS_MOB(ch) || IS_MOB(v)) return(1);
  if (ROOM_FLAG(ch->in_room,ARENA) && ch->in_room==v->in_room) return(1);
  if (!ch || !v) return(1);
  /* if the levels aren't right, don't allow the fight.
   * if the victim is attacking the char, then the fight has started
   */
  if ((GetMaxLevel (ch) - MIN_CHOSEN_LEVEL_DIFF) > GetMaxLevel (v)) {
	/* not fighting, but was fighting. either stopped or incapac. */
	if (!v->specials.fighting &&
	    ch->specials.fighting && ch->specials.fighting == v) {
		; /* allow the fight */
	/* not fighting, or not fighting me */
	} else if (!v->specials.fighting ||
	    (v->specials.fighting && v->specials.fighting != ch)) {
		sendf (ch, "You don't think attacking %s would be a good idea.\n", GET_NAME (v));
		return (0);
	}
  }
  if (!IS_SET(ch->specials.act,PLR_CORRUPT)) {
	if (IS_GOOD(v))
	  sendf(ch,"You are not corrupt enough to hurt %s.\n",GET_NAME(v));
	else
	  sendf(ch,"You are not lawful enought to punish %s.\n",GET_NAME(v));
	return(0);
  }
  if (!IS_SET(v->specials.act,PLR_CORRUPT)) {
	if (IS_GOOD(v) && IS_GOOD(ch))
	  sendf(ch,"It would be pure lawlessness of you to hurt %s.\n",
		GET_NAME(v));
	else if (IS_GOOD(v) && !IS_GOOD(ch))
	  sendf(ch,"%s has not choosen the path of lawfullness yet.\n",
		GET_NAME(v));
	else if (IS_EVIL(v) && !IS_EVIL(ch))
	  sendf(ch,"%s has not choosen the path of corruption yet.\n",
		GET_NAME(v));
	else if (IS_EVIL(ch))
	  sendf(ch,"%s is not as corrupted as you are.\n",GET_NAME(v));
	else sendf(ch,"%s is not as lawful as you are.\n",GET_NAME(v));
	return(0);
  }
  if (GET_TRUE_ALIGN(ch)==1000 && GET_TRUE_ALIGN(v)==1000) {
	sendf(ch,"%s is just as lawful as you are.\n",GET_NAME(v));
	return(0);
  }
  return(1);
}

int find_kill_index(struct char_data *ch, ulong vnum)
{
  int i;

  if (!ch->kills) return(-1);
  for (i=0; i<MAX_KILL_KEEP; i++)
    if (ch->kills[i].vnum==vnum) return(i);
  return(-1);
}

int num_kills(struct char_data *ch, ulong vnum)
{
  int i;

  i=find_kill_index(ch,vnum);
  if (i==-1) return(0);
  else return(ch->kills[i].nkills);
}

void add_kill(struct char_data *ch, ulong vnum)
{
  int j,k;
  struct kill_info x;

  if (IS_MOB(ch)) return;
  if (!ch->kills) {
    CREATE(ch->kills,kill_info,MAX_KILL_KEEP);
    bzero(ch->kills,MAX_KILL_KEEP*sizeof(kill_info));
  }
  k=find_kill_index(ch,vnum);
  if (k==0) {
    ch->kills[0].nkills++;
  } else if (k>0) {
    x=ch->kills[k];
    for (j=k; j; j--)
      ch->kills[j]=ch->kills[j-1];
    ch->kills[0]=x;
    ch->kills[0].nkills++;
  } else {
    for (j=MAX_KILL_KEEP-1; j; j--)
      ch->kills[j]=ch->kills[j-1];
    ch->kills[0].vnum=vnum;
    ch->kills[0].nkills=1;
  }
}

void do_hit(struct char_data *ch, char *argument, int cmd)
{
  char arg[80];
  char arg2[80];
  char location[100];
  int num;
  struct char_data *victim;
  extern int locations[12][10];
  
  if (check_peaceful(ch,"You feel too peaceful to contemplate violence.\n"))
    return;

  argument_interpreter(argument, arg, arg2);
  
  if (*arg) {
    victim = get_char_room_vis(ch, arg);
    if (victim) {
      if (victim == ch) {
	send_to_char("You hit yourself..OUCH!.\n", ch);
	act("$n hits $mself, and says OUCH!", FALSE, ch, 0, victim, TO_ROOM);
      } else {
	if (!CORRUPT(ch,victim)) return;
	if (IS_AFFECTED(ch,AFF_CHARM) && !CORRUPT(ch->master,victim))
		return;
	if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
	  act("$N is just such a good friend, you simply can't hit $M.",
	      FALSE, ch,0,victim,TO_CHAR);
	  return;
	}
	if ((GET_POS(ch)==POSITION_STANDING) &&
	    (victim != ch->specials.fighting)) {
           if(!arg2) {
               ch->points.aimloc = LOCATION_BODY;
           } else {
              switch(*arg2) {
                case 'f': /* Feet */
                case 'c': /* Claws */
                case 't': /* Tail */
                    num = 1;
                break;
                case 'l': /* Legs */
                    num = 2;
                break;
                case 'a': /* Arms */
                case 'w': /* Wings */
                    num = 3;
                break;
                case 'h': /* Head */
                    num = 5;
                break;
                default:
                    num = 4;
                break;
              }
              ch->points.aimloc = num;
           }
           while(ch->points.aimloc >
		  locations[HeightClass(ch)-HeightClass(victim)+6][9])
	    	    ch->points.aimloc--;


	   strcpy(location,location_name(victim,ch->points.aimloc));
           sendf(ch,"Aiming for the %s....\n",location);
	   hit(ch, victim, TYPE_UNDEFINED);
	   WAIT_STATE(ch, PULSE_VIOLENCE+2);
	} else {
	  send_to_char("You do the best you can!\n",ch);
	}
      }
    } else {
      send_to_char("They aren't here.\n", ch);
    }
  } else {
    send_to_char("Hit who?\n", ch);
  }
}



void do_kill(struct char_data *ch, char *argument, int cmd)
{
  static char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct char_data *victim;
  
  if ((GetMaxLevel(ch) < SILLYLORD) || IS_NPC(ch)) {
    do_hit(ch, argument, 0);
    return;
  }
  
  argument_interpreter(argument, arg, arg2);
  
  if (!*arg) {
    send_to_char("Kill who?\n", ch);
  } else {
    if (!(victim = get_char_room_vis(ch, arg)))
      send_to_char("They aren't here.\n", ch);
    else if (ch == victim)
      send_to_char("Your mother would be so sad.. :(\n", ch);
    else {
      act("You chop $M to pieces! Ah! The blood!", FALSE, ch, 0, victim, TO_CHAR);
      act("$N chops you to pieces!", FALSE, victim, 0, ch, TO_CHAR);
      act("$n brutally slays $N", FALSE, ch, 0, victim, TO_NOTVICT);
      raw_kill(victim);
    }
  }
}



void do_backstab(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  byte percent, base=0;
  
  if (check_peaceful(ch, "Naughty, naughty.  None of that here.\n"))
    return;
  
  only_argument(argument, name);
  
  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Backstab who?\n", ch);
    return;
  }
  
  if (victim == ch) {
    send_to_char("How can you sneak up on yourself?\n", ch);
    return;
  }
  
  if (!ch->equipment[WIELD]) {
    send_to_char("You need to wield a weapon, to make it a succes.\n",ch);
    return;
  }

  if (ch->specials.fighting) {
    send_to_char("There's no way to reach that back while you're fighting!\n", ch);
    return;
  }

  if (victim->attackers >= 2) {
    send_to_char("You can't get close enough to them to backstab!\n", ch);
    return;
  }
  
  if (!CORRUPT(ch,victim)) return;

  if (ch->equipment[WIELD]->obj_flags.value[3] != 11 &&
      ch->equipment[WIELD]->obj_flags.value[3] != 1  &&
      ch->equipment[WIELD]->obj_flags.value[3] != 10) {
    send_to_char("Only piercing or stabbing weapons can be used for backstabbing.\n",ch);
    return;
  }
  
  if (ch->specials.fighting) {
    send_to_char("You're too busy to backstab\n", ch);
    return;
  }
  
  base=0;
  if (victim->specials.fighting) {
    if (IsClass(ch,ASSASSIN)) {
      base = 0;
    } else {
      sendf(ch,"You can't backstab someon who is already fighting.\n");
      return;
    }
  } else {
    base = 4;
  }
  
  percent=number(1,101); /* 101% is a complete failure */
  if (!AWAKE(victim))  base+=8;
  
  if (IS_SET(ch->specials.act,PLR_DEBUG)) {
    sendf(ch,"Skill: %d Percent: %d Base: %d Sucess: %d\n",
		get_skill(ch,SKILL_BACKSTAB),
		percent,
		base,
		percent<(get_skill(ch,SKILL_BACKSTAB) + stealth_penalty(ch)));
  }
  if (percent<(get_skill(ch,SKILL_BACKSTAB) + stealth_penalty(ch))) {
    AddHated(victim, ch);
    base += 2;
    GET_HITROLL(ch) += base;
    hit(ch,victim,SKILL_BACKSTAB);
    GET_HITROLL(ch) -= base;
  } else {
    AddHated(victim, ch);
    damage(ch, victim, 0, SKILL_BACKSTAB);
    FailSkill(ch,SKILL_BACKSTAB);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE);
}



void do_order(struct char_data *ch, char *argument, int cmd)
{
  char name[100], message[256];
  char buf[256], *p;
  bool found = FALSE;
  int org_room, cmd_no;
  struct char_data *victim;
  struct follow_type *k;
  
  half_chop(argument, name, message);
  
  if (!*name || !*message)
    send_to_char("Order who to do what?\n", ch);
  else if (!(victim = get_char_room_vis(ch, name)) &&
	   str_cmp("follower", name) && str_cmp("followers", name))
    send_to_char("That person isn't here.\n", ch);
  else if (ch == victim)
    send_to_char("You obviously suffer from Multiple Personality Disorder.\n", ch);
  
  else {
    if (IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not aprove of you giving orders.\n",ch);
      return;
    }
    
    if (victim) {
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, victim, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);
      
      if ( (victim->master!=ch) || !IS_AFFECTED(victim, AFF_CHARM) )
	act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
      else {
	send_to_char("Ok.\n", ch);
	p = message;
	cmd_no = command_interpreter(victim, &p);
	command_process(victim, cmd_no, p);
      }
    } else {  /* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, victim, TO_ROOM);
      
      org_room = ch->in_room;
      
      for (k = ch->followers; k; k = k->next) {
	if (org_room == k->follower->in_room)
	  if (IS_AFFECTED(k->follower, AFF_CHARM)) {
	    found = TRUE;
	    p = message;
	    cmd = command_interpreter(k->follower, &p);
	    command_process(k->follower, cmd, p);
	  }
      }
      if (found)
	send_to_char("Ok.\n", ch);
      else
	send_to_char("Nobody here is a loyal subject of yours!\n", ch);
    }
  }
}

void do_flee(struct char_data *ch, char *argument, int cmd)
{
  int i, attempt, loose=0, die=0,percent,nolose;
  
  assert(ch);
  if (IS_AFFECTED(ch, AFF_PARALYSIS) || (ch->specials.wait > PULSE_VIOLENCE))
    return;

  if (ROOM_FLAG(ch->in_room,ARENA)) nolose=1; else nolose=0;
  
  if ((GET_POS(ch) <= POSITION_SITTING)&&(GET_RACE(ch) != RACE_BIRD)) {
    act("$n scrambles madly to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
    act("Panic-stricken, you scramble to your feet.", TRUE, ch, 0, 0,
	TO_CHAR);
    GET_POS(ch) = POSITION_STANDING;
    WAIT_STATE(ch, PULSE_VIOLENCE);
    return;
  }
  
  if (!(ch->specials.fighting)) {
    for(i=0; i<6; i++) {
      attempt = number(0, 5);  /* Select a random direction */
      if (CAN_GO(ch, attempt) &&
	  !IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags, DEATH)) {
	act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
	if ((die = MoveOne(ch, attempt))== 1) {
	  /* The escape has succeded */
	  send_to_char("You flee head over heels.\n", ch);
	  return;
	} else {
	  if (!die) act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
	  return;
	}
      }
    } /* for */
    /* No exits was found */
    send_to_char("PANIC! You couldn't escape!\n", ch);
    return;
  }
  
  for(i=0; i<6; i++) {
    attempt = number(0, 5);  /* Select a random direction */
    if (CAN_GO(ch, attempt) &&
	!IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags, DEATH)) {
      act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
      if ((die = MoveOne(ch, attempt))== 1) { 
	/* The escape has succeded. We'll be nice. */
	if (GetMaxLevel(ch) > 3) {
	  loose = GET_EXP(ch->specials.fighting);
	  if (GET_HIT(ch)>GET_MAX_HIT(ch)/2) loose /= 10;
          else if (GET_HIT(ch)>=GET_MAX_HIT(ch)/20) loose /= 25;
          else loose /= 100;
          loose = MIN(MIN(100000,GET_EXP(ch)/100),loose);
	} else {
	  loose = 0;
	}
	if (loose < 0) loose = 0;
	if (IS_NPC(ch)) {
	  AddFeared(ch, ch->specials.fighting);
	} else {
	  percent = (100 * GET_HIT(ch->specials.fighting)) /
	    GET_MAX_HIT(ch->specials.fighting);
	  if (number(1,101) < percent && IS_NPC(ch->specials.fighting)) {
	    if (Hates(ch->specials.fighting, ch) || 
		(IS_GOOD(ch) && IS_EVIL(ch->specials.fighting)) ||
		(IS_EVIL(ch) && IS_GOOD(ch->specials.fighting))) {
	      SetHunting(ch->specials.fighting, ch);
	    }
	  }
	}
	
	if (!IS_NPC(ch) && !nolose)
	  gain_exp(ch, -loose);
        if (nolose) sports_cast("%s flees from %s",GET_NAME(ch),ch->specials.fighting?GET_NAME(ch->specials.fighting):"somone");
	
	send_to_char("You flee head over heels.\n", ch);
	if (IS_SET(ch->specials.act,PLR_DEBUG) && loose)
	   sendf(ch,"You lost %d experience points because of that.\n",loose);
	if (ch->specials.fighting->specials.fighting == ch)
	  stop_fighting(ch->specials.fighting);
	if (ch->specials.fighting)
	  stop_fighting(ch);
	return;
      } else {
	if (!die) act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
	return;
      }
    }
  } /* for */
  
  /* No exits were found */
  send_to_char("PANIC! You couldn't escape!\n", ch);
}

void do_batter(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  byte percent;

  if (check_peaceful(ch,"You feel too peaceful to contemplate violence.\n"))
     return;

  only_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
     if (ch->specials.fighting) {
        victim = ch->specials.fighting;
     } else {
        send_to_char("Batter who?\n", ch);
        return;
     }
  }

  if (victim == ch) {
     send_to_char("Aren't we funny today...\n", ch);
     return;
  }

  if (ch->attackers > 3) {
     send_to_char("There's no room to batter!\n",ch);
     return;
  }

  if (victim->attackers >= 6) {
     send_to_char("You can't get close enough to them to batter!\n", ch);
     return;
  }

  if (!ch->equipment[WEAR_SHIELD]) {
     send_to_char("You have to be wearing a shield to batter someone!\n",ch);
     return;
  }

  if(ch->equipment[WIELD]) {
     send_to_char("You can't batter someone while wielding a weapon!\n",ch);
     return;
  }

  if (!CORRUPT(ch,victim)) return;
  percent=number(1,101); /* 101% is a complete failure */
  percent -= GET_STR(ch) + GET_CON(ch);
  percent += GET_DEX(victim) + GET_CON(victim);
  if (percent > get_skill(ch,SKILL_BATTER)) {                    /* Failure */
     if (GET_POS(victim) > POSITION_DEAD) {
        damage(ch, victim, 0, SKILL_BATTER);
        GET_POS(ch) = POSITION_SITTING;
        WAIT_STATE(ch, PULSE_VIOLENCE*4);
        FailSkill(ch,SKILL_BATTER);
     }
  } else {                                                /* Success */
     if (GET_POS(victim) > POSITION_DEAD) {
        damage(ch, victim, 1+ch->equipment[WEAR_SHIELD]->obj_flags.value[0],
           SKILL_BATTER);
        GET_POS(victim) = POSITION_SITTING;
        WAIT_STATE(victim, PULSE_VIOLENCE*2);
        WAIT_STATE(ch, PULSE_VIOLENCE*2);
     }
  }
}

int phys_bash(struct char_data *ch, struct char_data *victim)
{
  byte percent;

  if (check_peaceful(ch, "You feel too peaceful to contemplate violence.\n"))
    return(0);

  if (ch->attackers > 3) {
    send_to_char("There's no room to bash!\n",ch);
    return(0);
  }

  if (victim->attackers >= 6) {
    send_to_char("You can't get close enough to them to bash!\n", ch);
    return(0);
  }

  if (!CORRUPT(ch,victim)) return(0);
  
  percent=number(1,101); /* 101% is a complete failure */
  
  /* some modifications to account for dexterity, and level */
  percent -= dex_app[GET_DEX(ch)].reaction * 10;
  percent += dex_app[GET_DEX(victim)].reaction * 10;
  if (percent > get_skill(ch,SKILL_BASH)) {
    if (GET_POS(victim) > POSITION_DEAD) {
      GET_POS(ch) = POSITION_SITTING;
      damage(ch, victim, 0, SKILL_BASH);
      FailSkill(ch,SKILL_BASH);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    return(0);
  } else {
    if (GET_POS(victim) > POSITION_DEAD) {
      WAIT_STATE(victim, PULSE_VIOLENCE*2);
      GET_POS(victim) = POSITION_SITTING;
      damage(ch, victim, 1, SKILL_BASH);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    return(1);
  }
}

void do_bash(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  
  only_argument(argument, name);
  
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Bash who?\n", ch);
      return;
    }
  }
  
  if (victim == ch) {
    send_to_char("Aren't we funny today...\n", ch);
    return;
  }

/*
  if(HeightClass(ch) < HeightClass(victim)) {
    act("$M is too tall fo you to try that.", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }
  */
  phys_bash(ch, victim);
}

void do_rescue(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim, *tmp_ch;
  int percent;
  char victim_name[240];
  
  if (check_peaceful(ch,"No one should need rescuing here.\n"))
    return;

  only_argument(argument, victim_name);
  
  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Who do you want to rescue?\n", ch);
    return;
  }
  
  if (victim == ch) {
    send_to_char("What about fleeing instead?\n", ch);
    return;
  }
  
  if (ch->specials.fighting == victim) {
    send_to_char("How can you rescue someone you are trying to kill?\n",ch);
    return;
  }

  if (victim->attackers >= 3) {
    send_to_char("You can't get close enough to them to rescue!\n", ch);
    return;
  }
  
  for (tmp_ch=real_roomp(ch->in_room)->people; tmp_ch &&
       (tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room)  ;
  
  if (!tmp_ch) {
    act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }
  
  
  if (!is_fighter(ch))
    send_to_char("But only true warriors can do this!", ch);
  else {
    percent=number(1,101); /* 101% is a complete failure */
    if (percent > get_skill(ch,SKILL_RESCUE)) {
      send_to_char("You fail the rescue.\n", ch);
      FailSkill(ch,SKILL_RESCUE);
      return;
    }
    
    if (!CORRUPT(ch,tmp_ch)) return;
    send_to_char("Banzai! To the rescue...\n", ch);
    act("You are rescued by $N, you are confused!", FALSE, victim, 0, ch, TO_CHAR);
    act("$n heroically rescues $N.", FALSE, ch, 0, victim, TO_NOTVICT);
    
    if (victim->specials.fighting == tmp_ch)
      stop_fighting(victim);
    if (tmp_ch->specials.fighting)
      stop_fighting(tmp_ch);
    if (ch->specials.fighting)
      stop_fighting(ch);
    
    set_fighting(ch, tmp_ch);
    set_fighting(tmp_ch, ch);
    
    WAIT_STATE(victim, 2*PULSE_VIOLENCE);
  }
  
}



void do_assist(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim, *tmp_ch;
  char victim_name[240];
  
  if (check_peaceful(ch,"Noone should need assistance here.\n"))
    return;
  
  only_argument(argument, victim_name);
  
  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Who do you want to assist?\n", ch);
    return;
  }
  
  if (victim == ch) {
    send_to_char("Oh, by all means, help yourself...\n", ch);
    return;
  }
  
  if (ch->specials.fighting == victim) {
    send_to_char("That would be counterproductive?\n",ch);
    return;
  }
  
  if (ch->specials.fighting) {
    send_to_char("You have your hands full right now\n",ch);
    return;
  }

  if (victim->attackers >= 6) {
    send_to_char("You can't get close enough to them to assist!\n", ch);
    return;
  }

  
  tmp_ch = victim->specials.fighting;
  /*	for (tmp_ch=real_roomp(ch->in_room)->people; tmp_ch &&
	(tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room)  ;
	*/
  if (!tmp_ch) {
    act("But he's not fighting anyone.", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }
  
  if (!CORRUPT(ch,tmp_ch)) return;
  hit(ch, tmp_ch, TYPE_UNDEFINED);
  
  WAIT_STATE(victim, PULSE_VIOLENCE+2); /* same as hit */
}



void do_kick(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  byte percent;
  
  if (check_peaceful(ch,"You feel too peaceful to contemplate violence.\n"))
    return;
  
  only_argument(argument, name);
  
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Kick who?\n", ch);
      return;
    }
  }
  
  if (victim == ch) {
    send_to_char("Aren't we funny today...\n", ch);
    return;
  }

  if (ch->attackers > 2) {
    send_to_char("There's no room to kick!\n",ch);
    return;
  }

  if (victim->attackers >= 3) {
    send_to_char("You can't get close enough to them to kick!\n", ch);
    return;
  }
  
  if (!CORRUPT(ch,victim)) return;
  percent=((10-(GET_BODY_AC(victim)/10))<<1) + number(1,101); /* 101% is a complete failure */
  
  if (percent > get_skill(ch,SKILL_KICK)) {
    if (GET_POS(victim) > POSITION_DEAD)
      damage(ch, victim, 0, SKILL_KICK);
      FailSkill(ch,SKILL_KICK);
  } else {
    if (GET_POS(victim) > POSITION_DEAD)
      if (IsClass(ch, NINJA))
       { damage(ch, victim, MIN(GetMaxLevel(ch)>>1,70)*1.5, SKILL_KICK); }
        else 
       { damage(ch, victim, MIN(GetMaxLevel(ch)>>1,50), SKILL_KICK); }
  }
  WAIT_STATE(ch, PULSE_VIOLENCE*3);
}

void do_wimp(struct char_data *ch, char *argument, int cmd)
{
  for (;*argument && isspace(*argument);argument++);

  if (isdigit(*argument)) {
    if(atoi(argument) > (GET_MAX_HIT(ch) / 5)){
      SET_BIT(ch->specials.act, PLR_WIMPY);
      sendf(ch, "You can't set wimpy above 1/5th of your total hit points.\nYou will now flee if you fall below 1/5th of your total hit points.\n");
    }
    else{
       ch->specials.whimpy_level = MIN(GET_MAX_HIT(ch) / 5, atoi(argument));
       CLEAR_BIT(ch->specials.act, PLR_WIMPY);
       sendf(ch,"You will flee now if you fall below %d hit points.\n",
         ch->specials.whimpy_level);
    }
    return;
  } else if (ch->specials.whimpy_level>0) {
    ch->specials.whimpy_level= -10;
    send_to_char("You will no longer flee if you are low on hit points.\n",ch);
    return;
  }
  
  if (IS_SET(ch->specials.act, PLR_WIMPY)) {
    CLEAR_BIT(ch->specials.act, PLR_WIMPY);
    send_to_char("Ok, you are no longer a wimp...\n",ch);
  } else {
    SET_BIT(ch->specials.act, PLR_WIMPY);
    send_to_char("Ok, you are now flee if you fall below 1/5 of your total hitpoints.\n", ch);
  }
  
}


extern struct breather breath_monsters[];

typedef void (*bwf)(int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj,int eff_level);

bwf bweapons[] = {
  cast_geyser,
  cast_fire_breath, cast_gas_breath, cast_frost_breath, cast_acid_breath,
  cast_lightning_breath};

void do_breath(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char	name[MAX_STRING_LENGTH];
  int	count, manacost=0;
  bwf	weapon;
  
  if (check_peaceful(ch,"That wouldn't be nice at all.\n"))
    return;
  
  only_argument(argument, name);
  
  for (count = FIRST_BREATH_WEAPON;
       count <= LAST_BREATH_WEAPON && !affected_by_spell(ch, count);
       count++)
    ;
  
  if (count>LAST_BREATH_WEAPON) {
    struct breather *scan;
    
    for (scan = breath_monsters;
	 scan->vnum >= 0 && scan->vnum != ch->virtual;
	 scan++)
      ;
    
    if (scan->vnum < 0) {
      send_to_char("You don't have a breath weapon, potatohead.\n", ch);
      return;
    }
    
    for (count=0; scan->breaths[count]; count++)
      ;
    
    if (count<1) {
      vlog(LOG_URGENT,"monster %s has no breath weapons",
	      ch->player.short_descr);
      send_to_char("Hey, why don't you have any breath weapons!?\n",ch);
      return;
    }
    
    weapon = scan->breaths[dice(1,count)-1];
    if (GET_MANA(ch) <= -3*manacost) {
      weapon = NULL;
    }
    manacost = scan->cost;
  } else {
    manacost = 0;
    weapon = bweapons[count-FIRST_BREATH_WEAPON];
    affect_from_char(ch, count);
  }
  
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Breath on who?\n", ch);
      return;
    }
  }
  
  if (!CORRUPT(ch,victim)) return;
  breath_weapon(ch, victim, manacost, weapon);
  
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

#if 0
void do_shoot(struct char_data *ch, char *argument, int cmd)
{
  char arg[80];
  struct char_data *victim;
  
  if (check_peaceful(ch,"You feel too peaceful to contemplate violence.\n"))
    return;

  only_argument(argument, arg);
  
  if (*arg) {
    victim = get_char_room_vis(ch, arg);
    if (victim) {
      if (victim == ch) {
	send_to_char("You can't shoot things at yourself!", ch);
	return;
      } else {
	if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
	  act("$N is just such a good friend, you simply can't shoot at $M.",
	      FALSE, ch,0,victim,TO_CHAR);
	  return;
	}
	shoot(ch, victim);
      }
    } else {
      send_to_char("They aren't here.\n", ch);
    }
  } else {
    send_to_char("Shoot who?\n", ch);
  }
}
#endif
