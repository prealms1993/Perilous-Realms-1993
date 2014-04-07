/* 
  holds all of the new skills that i have designed....

  Much thanks to Whitegold of  epic Dikumud for the hunt code.
*/

#include <fcns.h>
#include <utils.h>
#include <race.h>
#include <spells.h>
#include <comm.h>
#include <handler.h>
#include <externs.h>
#include <ticks.h>


struct hunting_data {
  char	*name;
  struct char_data **victim;
};

/*
**  Disarm:
*/

void do_disarm(struct char_data *ch, char *argument, int cmd)
{
  char name[30];
  int percent;
  struct char_data *victim;
  struct obj_data *w;

  if (check_peaceful(ch,"You feel too peaceful to contemplate violence.\n"))
    return;
  
  /*
   *   get victim
   */
  only_argument(argument, name);
    if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Disarm who?\n", ch);
      return;
    }
  }
  
  
  if (victim == ch) {
    send_to_char("Aren't we funny today...\n", ch);
    return;
  }

  if (ch->attackers > 3) {
    send_to_char("There is no room to disarm!\n", ch);
    return;
  }

	if (!CORRUPT(ch, victim)) return;
  
  /*
   *   make roll - modified by dex && level
   */
  percent=number(1,101); /* 101% is a complete failure */
  
  percent -= dex_app[GET_DEX(ch)].reaction*10;
  percent += dex_app[GET_DEX(victim)].reaction*10;
  if (!ch->equipment[WIELD]) {
    percent -= 50;
  }
  if (GetMaxLevel(victim) > 20) {
    percent += ((GetMaxLevel(victim)-18) * 5);
  }
  if (percent > get_skill(ch,SKILL_DISARM)) {
    /*
     *   failure.
     */
    act("You try to disarm $N, but fail miserably.", 
	TRUE, ch, 0, victim, TO_CHAR);
    act("$n does a nifty fighting move, but then falls on $s butt.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_SITTING;
    FailSkill(ch,SKILL_DISARM);
    if ((IS_NPC(victim)) && (GET_POS(victim) > POSITION_SLEEPING) &&
        (!victim->specials.fighting)) {
      set_fighting(victim, ch);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
  } else {
    /*
     *  success
     */
    if (victim->equipment[WIELD]) {
      w = unequip_char(victim, WIELD);
      obj_to_char (w, victim);
/* Disarm the player but let it stay in player's inventory
      obj_from_char(w);
      obj_to_room(w, victim->in_room);
*/
      act("$n makes an impressive fighting move.", 
	  TRUE, ch, 0, 0, TO_ROOM);
      act("You send $p flies from $N's grasp.", TRUE, ch, w, victim, TO_CHAR);
      act("$p flies from your grasp.", TRUE, ch, w, victim, TO_VICT);
    } else {
      act("You try to disarm $N, but $E doesn't have a weapon.", 
	  TRUE, ch, 0, victim, TO_CHAR);
      act("$n makes an impressive fighting move, but does little more.",
	  TRUE, ch, 0, 0, TO_ROOM);
    }
    if ((IS_NPC(victim)) && (GET_POS(victim) > POSITION_SLEEPING) &&
        (!victim->specials.fighting)) {
      set_fighting(victim, ch);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*1);

  }  
}


/*
**   Track:
*/

static int named_mobile_in_room(int room, struct hunting_data *c_data)
{
  struct char_data	*scan;
  for (scan = real_roomp(room)->people; scan; scan = scan->next_in_room)
    if (isname(c_data->name, scan->player.name)) {
      *(c_data->victim) = scan;
      return 1;
    }
  return 0;
}

void do_track(struct char_data *ch, char *argument, int cmd)
{
   char name[256], buf[256];
   int dist, code;
  struct hunting_data	huntd;

  only_argument(argument, name);

   dist = get_skill(ch,SKILL_HUNT);

/* NEW CLASS <- Marker */
   if (IsClass(ch, ASSASSIN)) {
     dist *=5;
   } else if (IsClass(ch,RANGER)) {
     dist*=8;
   } else if (IsClass(ch,DRUID)) {
     dist *=6;
   } else if (IsClass(ch, THIEF)) {
     dist *= 4;
   } else if (IsClass(ch, WARRIOR)) {
     dist *= 2;
   } else if (IsClass(ch, MAGE)) {
     dist += GET_LEVEL(ch);
   } else {
     dist = dist;
   }

   switch(GET_RACE(ch)){
   case RACE_ELVEN:
     dist *= 2;               /* even better */
     break;
   case RACE_DEVIL:
   case RACE_DEMON:
     dist = MAX_ROOMS;   /* as good as can be */
     break;
   default:
     break;
   }

   if (GetMaxLevel(ch) >= IMMORTAL)
    dist = MAX_ROOMS;
 
  ch->hunt_dist = dist;

  ch->specials.hunting = 0;
  huntd.name = name;
  huntd.victim = &ch->specials.hunting;
  code = find_path( ch->in_room, named_mobile_in_room, &huntd, -dist);
  
   WAIT_STATE(ch, PULSE_VIOLENCE*1);

   if (code == -1) {
    send_to_char("You are unable to find traces of one.\n", ch);
     return;
   } else {
     if (IS_LIGHT(ch->in_room)) {
        SET_BIT(ch->specials.act, PLR_HUNTING);
       sprintf(buf, "You see traces of your quarry to the %s\n", dirs[code]);
        send_to_char(buf,ch);
      } else {
      ch->specials.hunting = 0;
	send_to_char("It's too dark in here to track...\n",ch);
	return;
      }
   }
 }

int track( struct char_data *ch, struct char_data *vict)
{

  char buf[256];
  int code;

  if ((!ch) || (!vict))
    return(-1);

  code = choose_exit(ch->in_room, vict->in_room, ch->hunt_dist);

  if ((!ch) || (!vict))
    return(-1);


  if (ch->in_room == vict->in_room) {
    send_to_char("##You have found your target!\n",ch);
    return(FALSE);  /* false to continue the hunt */
  }
  if (code == -1) {
    send_to_char("##You have lost the trail.\n",ch);
    return(FALSE);
  } else {
    sprintf(buf, "##You see a faint trail to the %s\n", dirs[code]);
    send_to_char(buf, ch);
    return(TRUE);
  }

}

int dir_track( struct char_data *ch, struct char_data *vict)
{
  char buf[256];
  int code;

  if ((!ch) || (!vict))
    return(-1);

  code = choose_exit(ch->in_room, vict->in_room, ch->hunt_dist);
  if((code == -1) && IsHumanoid(ch)){
    code = choose_exit(ch->in_room, vict->in_room, -ch->hunt_dist);
  }

  if (code == -1) {
    if (ch->in_room == vict->in_room) {
      send_to_char("##You have found your target!\n",ch);
    } else {
      send_to_char("##You have lost the trail.\n",ch);
    }
    return(-1);  /* false to continue the hunt */
  } else {
    sprintf(buf, "##You see a faint trail to the %s\n", dirs[code]);
    send_to_char(buf, ch);
    return(code);
  }

}



void slam_into_wall( struct char_data *ch, struct room_direction_data *exitp)
{
  char doorname[128];
  char buf[256];
 
  if (exitp->keyword && *exitp->keyword) {
    if ((strcmp(fname(exitp->keyword), "secret")==0) ||
	(IS_SET(exitp->exit_info, EX_SECRET))) {
      strcpy(doorname, "wall");
    } else {
      strcpy(doorname, fname(exitp->keyword));
    }
  } else {
    strcpy(doorname, "barrier");
  }
  sprintf(buf, "You slam against the %s with no effect\n", doorname);
  send_to_char(buf, ch);
  send_to_char("OUCH!  That REALLY Hurt!\n", ch);
  sprintf(buf, "$n crashes against the %s with no effect\n", doorname);
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  GET_HIT(ch) -= number(1, 10)*2;
  if (GET_HIT(ch) < 0)
    GET_HIT(ch) = 0;
  GET_POS(ch) = POSITION_STUNNED;
  return;
}
 
 
/*
  skill to allow fighters to break down doors
*/
void do_doorbash( struct char_data *ch, char *arg, int cmd)
{
  extern char *dirs[];
  int dir;
  int ok;
  struct room_direction_data *exitp;
  int was_in, roll;
 
  char buf[256], type[128], direction[128];
 
  if (GET_MOVE(ch) < 10) {
    send_to_char("You're too tired to do that\n", ch);
    return;
  }
/* 
  if (MOUNTED(ch)) {
    send_to_char("Yeah... right... while mounted\n", ch);
    return;
  }
*/ 
  /*
    make sure that the argument is a direction, or a keyword.
  */
 
  for (;*arg == ' '; arg++);
 
  argument_interpreter(arg, type, direction);
 
  if ((dir = find_door(ch, type, direction)) >= 0) {
    ok = TRUE;
  } else {
    act("$n looks around, bewildered.", FALSE, ch, 0, 0, TO_ROOM);
    return;
  }
 
  if (!ok) {
    send_to_char("Hmm, you shouldn't have gotten this far\n", ch);
    return;
  }
 
  exitp = EXIT(ch, dir);
  if (!exitp) {
    send_to_char("you shouldn't have gotten here.\n", ch);
    return;
  }
 
  if (dir == UP) {
    if (real_roomp(exitp->to_room)->sector_type == SECT_AIR &&
	!IS_AFFECTED(ch, AFF_FLYING)) {
      send_to_char("You have no way of getting there!\n", ch);
      return;
    }
  }
 
  if (!IS_SET(exitp->exit_info, EX_CLOSED)) {
    sendf(ch,"Try a direction where there is a closed door!\n");
    return;
  }

  sprintf(buf, "$n charges %swards", dirs[dir]);
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  sprintf(buf, "You charge %swards\n", dirs[dir]);
  send_to_char(buf, ch);
 
  if (!IS_SET(exitp->exit_info, EX_CLOSED)) {
    was_in = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, exitp->to_room);
    do_look(ch, "", 0);
 
    DisplayMove(ch, dir, was_in, 1);
    if (!check_falling(ch)) {
      if (IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
	  GetMaxLevel(ch) < LOW_IMMORTAL) {
	death_trap(ch);
	return;
      } else {
	WAIT_STATE(ch, PULSE_VIOLENCE*3);
	GET_MOVE(ch) -= 10;
      }
    } else {
      return;
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
    GET_MOVE(ch) -= 10;
    return;
  }
 
  GET_MOVE(ch) -= 10;
 
  if (IS_SET(exitp->exit_info, EX_LOCKED) &&
      IS_SET(exitp->exit_info, EX_PICKPROOF)) {
    slam_into_wall(ch, exitp);
    return;
  }
 
  /*
    now we've checked for failures, time to check for success;
    */
  if (ch->skills) {
    if (get_skill(ch,SKILL_DOORBASH)) {
      roll = number(1, 100);
      if (roll > get_skill(ch,SKILL_DOORBASH)) {
	slam_into_wall(ch, exitp);
/*	LearnFromMistake(ch, SKILL_DOORBASH, 0, 95); */
      } else {
	/*
	  unlock and open the door
	  */
	sprintf(buf, "$n slams into the %s, and it bursts open!",
		fname(exitp->keyword));
	act(buf, FALSE, ch, 0, 0, TO_ROOM);
	sprintf(buf, "You slam into the %s, and it bursts open!\n",
		fname(exitp->keyword));
	send_to_char(buf, ch);
	raw_unlock_door(ch, exitp, dir);
	raw_open_door(ch, dir);
	GET_HIT(ch) -= number(1,5);
	/*
	  Now a dex check to keep from flying into the next room
	  */
	roll = number(1, 20);
	if (roll > GET_DEX(ch)) {
	  was_in = ch->in_room;
 
	  char_from_room(ch);
	  char_to_room(ch, exitp->to_room);
	  do_look(ch, "", 0);
	  DisplayMove(ch, dir, was_in, 1);
	  if (!check_falling(ch)) {
	    if (IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
		GetMaxLevel(ch) < LOW_IMMORTAL) {
	      death_trap(ch);
	      return;
	    }
	  } else {
	    return;
	  }
	  WAIT_STATE(ch, PULSE_VIOLENCE*3);
	  GET_MOVE(ch) -= 10;
	  return;
	} else {
	  WAIT_STATE(ch, PULSE_VIOLENCE*1);
	  GET_MOVE(ch) -= 5;
	  return;
	}
      }
    } else {
      send_to_char("You just don't know the nuances of door-bashing.\n", ch);
      slam_into_wall(ch, exitp);
      return;
    }
  } else {
    send_to_char("You're just a goofy mob.\n", ch);
    return;
  }
}

void do_retreat(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int dir;

  if (!ch->specials.fighting) {
    sendf(ch,"You aren't fighting anyone!\n");
    return;
  }
  if (get_skill(ch,SKILL_RETREAT)==0) {
    sendf(ch,"You don't know the art of strategic withdrawal.\n");
    return;
  }
  if (ch->attackers>5) {
    sendf(ch,"You can't successfully flee from this many attackers.\n");
    return;
  }
  arg=one_argument(arg,buf);
  if (!*buf) {
    sendf(ch,"Which direction do you want to retreat in?\n");
    return;
  }
  for (dir=0; dir<6; dir++)
    if (is_abbrev(buf,dirs[dir])) break;
  if (dir>=6) {
    sendf(ch,"That's not a direction.\n");
    return;
  }
  if (ValidMove(ch,dir)) {
    if (number(1,101)>get_skill(ch,SKILL_RETREAT)) {
      sendf(ch,"You can't seem to get away.\n");
      WAIT_STATE(ch, PULSE_VIOLENCE*2);
      FailSkill(ch,SKILL_RETREAT);
      return;
    }
    stop_fighting(ch);
    sendf(ch,"You carefully back out of your current predicament.\n");
    act("$n strategically retreats.",TRUE,ch,0,0,TO_ROOM);
    RawMove(ch,dir);
  }
}
