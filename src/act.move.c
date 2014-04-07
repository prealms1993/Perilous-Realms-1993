/* ************************************************************************
 *  Part of Perilous Realms Diku Mud                                       *
 *  file: act.movement.c                                                   *
 *  Usage : Movement commands, close/open & lock/unlock doors.             *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

int DBGLV=0;

#define DBGI(proc) { char buf[1000]; memset(buf,' ',++DBGLV);\
		sprintf(buf+DBGLV-1,"->%s\n",proc); printf("%s",buf);}
#define DBGO(proc) { char buf[1000]; memset(buf,' ',DBGLV--);\
		sprintf(buf+DBGLV,"<-%s\n",proc); printf("%s",buf);}

#include <stdio.h>
#include <string.h>

#include <fcns.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <spells.h>
#include <trap.h>
#include <ticks.h>
#include <externs.h>

/*
  Some new movement commands for diku-mud.. a bit more object oriented.
  */


void NotLegalMove(struct char_data *ch)
{
  send_to_char("Alas, you cannot go that way...\n", ch);
}


int ValidMove( struct char_data *ch, int cmd)
{
  struct room_direction_data	*exitp;

  exitp = EXIT(ch, cmd);

  if (!exit_ok(exitp,NULL)) {
    NotLegalMove(ch);
    return(FALSE);
  } else if (IS_SET(exitp->exit_info, EX_CLOSED)) {
    if (exitp->keyword) {
      if (!IS_SET(exitp->exit_info, EX_SECRET) && (strcmp(fname(exitp->keyword), "secret"))) {
	  sendf(ch,"The %s seems to be closed.\n", fname(exitp->keyword));
          return(FALSE);
       } else {
         NotLegalMove(ch);
         return(FALSE);
       }
     } else {
       NotLegalMove(ch);
       return(FALSE);
     }
  } else {
    return(TRUE);
  }
}

int RawMove(struct char_data *ch, int dir)
{
  int need_movement;
  struct obj_data *obj;
  bool has_boat;
  struct room_data *from_here, *to_here;

  if (special(ch, dir+1, ""))/* Check for special routines(North is 1)*/
    return(FALSE);

  if (!ValidMove(ch, dir)) {
    return(FALSE);
  }

  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master) &&
      (ch->in_room == ch->master->in_room)) {
    act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
    act("You burst into tears at the thought of leaving $N",
	FALSE, ch, 0, ch->master, TO_CHAR);

    return(FALSE);
  }

  from_here = real_roomp(ch->in_room);
  to_here = real_roomp(from_here->dir_option[dir]->to_room);

  if (GetMaxLevel(ch)<IMPLEMENTOR)
  if (IS_SET(to_here->room_flags,NO_GOTO) ||
        (IS_SET(to_here->room_flags,LORD_ONLY) && (GetMaxLevel(ch) < 2009)) ||
	(IS_SET(to_here->room_flags,GOOD_ONLY) && !IS_TRUE_GOOD(ch)) ||
	(IS_SET(to_here->room_flags,EVIL_ONLY) && !IS_TRUE_EVIL(ch)) ||
	(IS_SET(to_here->room_flags,NEUTRAL_ONLY) && !IS_TRUE_NEUTRAL(ch))) {
	send_to_char("A strange force bars your movement.\n",ch);
	return(FALSE);
  }

  if (to_here==NULL) {
    char_from_room(ch);
    char_to_room(ch, 0);

    send_to_char("Uh-oh.  The ground melts beneath you as you fall into the swirling chaos.\n",ch);
    do_look(ch, "\0",15);

/*    if(!IS_IMMORTAL(ch))
      WAIT_STATE(ch, 12); */

    return TRUE;
  }

  if (IS_AFFECTED(ch,AFF_FLYING)) {
    need_movement = 1;
  } else {
    need_movement = (movement_loss[from_here->sector_type]+
		     movement_loss[to_here->sector_type]) / 2;
  }

  /*
   **   Movement in water_nowswim
   */

  if ((from_here->sector_type == SECT_WATER_NOSWIM) ||
      (to_here->sector_type ==
       SECT_WATER_NOSWIM)) {
    if ((!IS_AFFECTED(ch,AFF_WATERBREATH)) &&
	(!IS_AFFECTED(ch,AFF_FLYING))) {
      has_boat = FALSE;
      /* See if char is carrying a boat */
      for (obj=ch->carrying; obj; obj=obj->next_content)
	if (obj->obj_flags.type_flag == ITEM_BOAT)
	  has_boat = TRUE;
      if (!has_boat && GetMaxLevel(ch) < LOW_IMMORTAL) {
	send_to_char("You need a boat to go there.\n", ch);
	return(FALSE);
      }
      if (has_boat)
	need_movement = 1;
    }
  }

  /*
    Movement in SECT_AIR
    */
  if ((from_here->sector_type == SECT_AIR) ||
      (to_here->sector_type ==
       SECT_AIR)) {
    if (!IS_AFFECTED(ch,AFF_FLYING)) {
      send_to_char("You would have to Fly to go there!\n",ch);
      return(FALSE);
    }
  }

  /*
    Movement in SECT_UNDERWATER
    */
  if ((from_here->sector_type == SECT_UNDERWATER) ||
      (to_here->sector_type ==
       SECT_UNDERWATER)) {
    if (!IS_AFFECTED(ch,AFF_WATERBREATH)) {
      send_to_char("You would need gills to go there!\n",ch);
      return(FALSE);
    }
  }


  if (GET_MOVE(ch)<need_movement) {
    send_to_char("You are too exhausted.\n",ch);
    return(FALSE);
  }

  if (!IS_IMMORTAL(ch)) {
    if (IS_NPC(ch)) {
      GET_MOVE(ch) -= 1;
    } else {
      GET_MOVE(ch) -= need_movement;
    }
  }

  /*
   *  nail the unlucky with traps.
   */
  if (CheckForMoveTrap(ch, dir))
    return(FALSE);

  char_from_room(ch);
  char_to_room(ch, from_here->dir_option[dir]->to_room);

  do_look(ch, "\0",15);

  if (IS_SET(to_here->room_flags, DEATH) && !IS_IMMORTAL(ch)) {
    death_trap (ch);
    return (FALSE);
  }

/*
 Deths joyous attempt at making fall rooms 12-23-92, object is to
 remove DT's and create rooms that you can fall in, hurt yourself
 and possibly your eq as well
*/ 
 if((IS_SET(to_here->room_flags, FALL)) && (!IS_AFFECTED(ch,AFF_FLYING)) && (from_here != to_here))
  {
   fall_down(ch);
   return (FALSE);   /* why though? i just copyed this from dt's */
  }

  /* suffer damage in fire/cold/water rooms if not protected */
/*
   if(to_here->sector_type == SECT_FIRE) {
	sendf (ch, "You are entering a firey room\n");
	dam = dice (5, 10);
	damage (ch, ch, dam, ROOM_FIRE);
	nlog ("End of firey room\n");
   }
   if (to_here->sector_type == SECT_COLD) {
	sendf (ch, "You are entering a cold room\n");
	dam = dice (5, 10);
	damage (ch, ch, dam, ROOM_COLD);
	nlog ("End of cold room\n");
   }
   if (ch == 0)
	return (FALSE);
*/

  /* delay movement - SLB */
/*  if(!IS_IMMORTAL(ch))
    WAIT_STATE(ch, 12); */

  return(TRUE);

}
/* do actual fall */
/*
   Fall and take damage from fall Craig Mohr (mohrc@odin.wosc.osshe.edu)
   written 12-23-93, ok so i was bored and yes alc I wrote it with bans
   explaination of how real_roomp works.
*/
void fall_down (struct char_data *ch)
{
 struct room_data *from_here, *to_here;
 int roomcnt = 0;
 int dam;

 from_here = real_roomp(ch->in_room);
 to_here = real_roomp(from_here->dir_option[5]->to_room); /* dir option 5 is down */

 vlog(LOG_DEATH,"%s walks into a fall room [%d]",GET_NAME(ch),ch->in_room);
 send_to_char("You fall...\n", ch);

 while(ValidMove(ch, 5) && (IS_SET (to_here->room_flags, FALL)) && (from_here != to_here))
  {
   send_to_char("and fall...\n", ch);
   char_from_room(ch);
   char_to_room(ch, from_here->dir_option[5]->to_room);
   from_here = real_roomp(ch->in_room);
   to_here = real_roomp(from_here->dir_option[5]->to_room);

  do_look (ch, "\0", 15);
  act("$n falls through the room screaming wildly!", FALSE, ch, 0, 0, TO_ROOM);
  roomcnt++;
  }

    if(ValidMove(ch, 5))
     {
      RawMove(ch, 5);
      roomcnt++;
      act("$n falls in from above!", FALSE, ch, 0, 0, TO_ROOM);
     }
   dam = dice (roomcnt, 8);
   if (ch->equipment[WEAR_BODY])
	dam += dice (roomcnt, 4);
   nlog ("%s suffers %d damage from falling thru %d rooms.", GET_NAME(ch), dam, roomcnt);
   damage (ch, ch, dam, TYPE_UNDEFINED);
}

void strip_char(struct char_data *ch)
{
  int i;
  struct obj_data *obj;

  for (i=0; i<MAX_WEAR; i++) {
    if (ch->equipment[i]) {
      obj=unequip_char(ch,i);
      extract_obj(obj);
    }
  }
  while (ch->carrying) {
    obj=ch->carrying;
    obj_from_char(obj);
    extract_obj(obj);
  }
}

int death_trap (struct char_data *ch)
{

   if (HasObject(ch, 25)) {
    do_save_player(ch);
    return FALSE;
   }
   vlog(LOG_DEATH,"%s dies in a death trap [%d]",GET_NAME(ch),ch->in_room);
   death_cry(ch);

   strip_char(ch);
   extract_char(ch);
   return(TRUE);
}

int MoveOne(struct char_data *ch, int dir)
{
  int was_in;

  was_in = ch->in_room;
  if (RawMove(ch, dir)) {  /* no error */
    DisplayOneMove(ch, dir, was_in);
    return TRUE;
  } else return FALSE;

}

int MoveGroup( struct char_data *ch, int dir)
{
  struct char_data *heap_ptr[50];
  int was_in, i, heap_top, heap_tot[50];
  struct follow_type *k, *next_dude;

  /*
   *   move the leader. (leader never duplicates)
   */

  was_in = ch->in_room;
  if (RawMove(ch, dir)) {  /* no error */
    DisplayOneMove(ch, dir, was_in);
    if (ch->followers) {
      heap_top = 0;
      for(k = ch->followers; k; k = next_dude) {
	next_dude = k->next;
	/*
	 *  compose a list of followers, w/heaping
	 */
	if ((was_in == k->follower->in_room) &&
	    (GET_POS(k->follower) >= POSITION_STANDING)) {
	  act("You follow $N.", FALSE, k->follower, 0, ch, TO_CHAR);
	  if (k->follower->followers) {
	    MoveGroup(k->follower, dir);
	  } else {
	    if (RawMove(k->follower, dir)) {
	      if (!AddToCharHeap(heap_ptr,&heap_top, heap_tot,k->follower))  {
		DisplayOneMove(k->follower, dir, was_in);
	      }
	    } else return FALSE;
	  }
	}
      }
      /*
       *  now, print out the heaped display message
       */
      for (i=0;i<heap_top;i++) {
	if (heap_tot[i] > 1) {
	  DisplayGroupMove(heap_ptr[i], dir, was_in, heap_tot[i]);
	} else {
	  DisplayOneMove(heap_ptr[i], dir, was_in);
	}
      }
    }
  }
  return FALSE;
}

void DisplayOneMove(struct char_data *ch, int dir, int was_in)
{
  DisplayMove(ch, dir, was_in, 1);
}

void DisplayGroupMove(struct char_data *ch, int dir, int was_in, int total)
{
  DisplayMove(ch, dir, was_in, total);
}


void do_move(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *monster;
  struct room_data *rp;
  cmd -= 1;

  /*
   ** the move is valid, check for follower/master conflicts.
   */

  if (ch->attackers > 1) {
    if (!ch->specials.fighting) {
      ch->attackers=0;
      vlog(LOG_DEBUG,"fixed %s -- Game thought he was being attacked\n",
	      GET_NAME(ch));
    } else {
      send_to_char("There's too many people around, no place to flee!\n", ch);
      return;
    }
  }

  if (!ch->followers && !ch->master) {
    if (!MoveOne(ch,cmd)) return;
  } else {
    if (!ch->followers) {
      if (!MoveOne(ch, cmd)) return;
    } else {
      if (!MoveGroup(ch, cmd)) return;
    }
  }

  rp=real_roomp(ch->in_room);

  if (!rp) {
     vlog(LOG_URGENT,"Uh-oh... something moving in non-existant room %d",
		ch->in_room);
     return;
  }

  for (monster=real_roomp(ch->in_room)->people; monster;
		monster=monster->next_in_room) {
  if (ch->aggressive != -1 &&
	GET_POS(ch) > POSITION_SITTING &&
	GET_POS(ch) != POSITION_FIGHTING &&
	CAN_SEE(ch, monster) &&
	IS_NPC(monster) &&
	!IS_NPC(ch) &&
	!IS_SET(real_roomp(ch->in_room)->room_flags,PEACEFUL) &&
	IS_SET(monster->specials.act,ACT_AGGRESSIVE) &&
	GET_HIT(ch) >= ch->aggressive) {

      send_to_char("You feel INTREPID and charge towards the enemy\n",
		   ch);

      hit(ch, monster, TYPE_UNDEFINED);
      WAIT_STATE(ch, PULSE_VIOLENCE + 2);
      return;
    }
  }
}


/*


  MoveOne and MoveGroup print messages.  Raw move sends success or failure.

  */


void DisplayMove(struct char_data *ch, int dir, int was_in, int total)
{
  struct char_data *tmp_ch;
  int msg_id;
  char tmp[256];

  msg_id=0;
  for (tmp_ch=real_roomp(was_in)->people; tmp_ch; tmp_ch=tmp_ch->next_in_room) {
    if ((!IS_AFFECTED(ch, AFF_SNEAK)) || (IS_IMMORTAL(tmp_ch))) {
      if ((ch != tmp_ch) && (AWAKE(tmp_ch)) && (CAN_SEE(tmp_ch, ch))) {
	if (total > 1) {
	  if (IS_NPC(ch)) {
	    sprintf(tmp,"%s leaves %s. [%d]\n",ch->player.short_descr,
		    dirs[dir], total);
	  } else {
	    sprintf(tmp,"%s leaves %s. [%d]\n",GET_NAME(ch),dirs[dir],
		    total);
	  }
	} else {
	  if (IS_NPC(ch)) {
	    sprintf(tmp,"%s leaves %s.\n",ch->player.short_descr,
		    dirs[dir]);
	  } else {
	    sprintf(tmp,"%s leaves %s\n",GET_NAME(ch),dirs[dir]);
	  }
	}
	send_to_char(tmp, tmp_ch);
      }
    }
  }

  for (tmp_ch=real_roomp(ch->in_room)->people; tmp_ch; tmp_ch=tmp_ch->next_in_room) {
    if (((!IS_AFFECTED(ch, AFF_SNEAK)) || (IS_IMMORTAL(tmp_ch))) &&
	(CAN_SEE(tmp_ch,ch)) &&
	(AWAKE(tmp_ch))) {
      if (tmp_ch != ch) {
	if (dir < 4) {
	  sprintf(tmp, "%s has arrived from the %s.",
		  PERS(ch, tmp_ch),dirs[rev_dir[dir]]);
	} else if (dir == 4) {
	  sprintf(tmp, "%s has arrived from below.",
		  PERS(ch, tmp_ch));
	} else if (dir == 5) {
	  sprintf(tmp, "%s has arrived from above.",
		  PERS(ch, tmp_ch));
	} else {
	  sprintf(tmp, "%s has arrived from somewhere.",
		  PERS(ch, tmp_ch));
	}
	if (total > 1) {
	  sprintf(tmp+strlen(tmp), " [%d]", total);
	}
	strcat(tmp, "\n");
	send_to_char(tmp, tmp_ch);
      }
    }
  }
}


int AddToCharHeap( struct char_data *heap[50], int *top, int total[50], struct char_data *k)
{
  int found, i;

  if (*top > 50) {
    return(FALSE);
  } else {
    found = FALSE;
    for (i=0;(i<*top&& !found);i++) {
      if (*top>0) {
	if ((IS_NPC(k)) &&
	    (k->virtual == heap[i]->virtual) &&
	    (heap[i]->player.short_descr) &&
	    (!strcmp(k->player.short_descr,
		     heap[i]->player.short_descr))) {
	  total[i] += 1;
	  found=TRUE;
	}
      }
    }
    if (!found) {
      heap[*top] = k;
      total[*top] = 1;
      *top+=1;
    }
  }
  return(TRUE);
}


int find_door(struct char_data *ch, char *type, char *dir)
{
  char buf[MAX_STRING_LENGTH];
  int door;
  static char *dirs[] =
    {
      "north",
      "east",
      "south",
      "west",
      "up",
      "down",
      "\n"
      };
  struct room_direction_data *exitp;

  if (*dir) { /* a direction was specified */
    if ((door = search_block(dir, dirs, FALSE)) == -1) { /* Partial Match */
      send_to_char("That's not a direction.\n", ch);
      return(-1);
    }
    exitp = EXIT(ch, door);
    if (exitp) {
      if (!exitp->keyword)
	return(door);
      if ((isname(type, exitp->keyword))&&
	  (strcmp(type,"secret"))) {
	return(door);
      } else {
	sprintf(buf, "I see no %s there.\n", type);
	send_to_char(buf, ch);
	return(-1);
      }
    } else {
      sprintf(buf, "I see no %s there.\n", type);
      send_to_char(buf, ch);
      return(-1);
    }
  } else { /* try to locate the keyword */
    for (door = 0; door <= 5; door++)
      if ((exitp=EXIT(ch, door)) &&
	  exitp->keyword &&
	  isname(type, exitp->keyword))
	return(door);

    sprintf(buf, "I see no %s here.\n", type);
    send_to_char(buf, ch);
    return(-1);
  }
}


void open_door(struct char_data *ch, int dir)
     /* remove all necessary bits and send messages */
{
  struct room_direction_data *exitp, *back;
  struct room_data	*rp;
  char	buf[MAX_INPUT_LENGTH];

  rp = real_roomp(ch->in_room);
  if (rp==NULL) {
    vlog(LOG_URGENT,"NULL rp in open_door() for %s.", PERS(ch,ch));
  }

  exitp = rp->dir_option[dir];

  CLEAR_BIT(exitp->exit_info, EX_CLOSED);
  if (exitp->keyword) {
    if (strcmp(exitp->keyword, "secret") &&
	(!IS_SET(exitp->exit_info, EX_SECRET))) {
      sprintf(buf, "$n opens the %s", fname(exitp->keyword));
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
    } else {
      act("$n reveals a hidden passage!", FALSE, ch, 0, 0, TO_ROOM);
    }
  } else
    act("$n opens the door.", FALSE, ch, 0, 0, TO_ROOM);

  /* now for opening the OTHER side of the door! */
  if (exit_ok(exitp, &rp) &&
      (back = rp->dir_option[rev_dir[dir]]) &&
      (back->to_room == ch->in_room))
    {
      CLEAR_BIT(back->exit_info, EX_CLOSED);
      if (back->keyword)	{
	sprintf(buf, "The %s is opened from the other side.\n",
		fname(back->keyword));
	send_to_room(buf, exitp->to_room);
      }
      else
	send_to_room("The door is opened from the other side.\n",
		     exitp->to_room);
    }
}

void do_open(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct obj_data *obj;
  struct char_data *victim;
  struct room_direction_data	*exitp;

  argument_interpreter(argument, type, dir);

  if (!*type)
    send_to_char("Open what?\n", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &victim, &obj)) {

    /* this is an object */

    if (obj->obj_flags.type_flag != ITEM_CONTAINER &&
	obj->obj_flags.type_flag != ITEM_SPELL_POUCH)
      send_to_char("That's not a container.\n", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("But it's already open!\n", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
      send_to_char("You can't do that.\n", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("It seems to be locked.\n", ch);
    else   	{
      CLEAR_BIT(obj->obj_flags.value[1], CONT_CLOSED);
      send_to_char("Ok.\n", ch);
      act("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  } else if ((door = find_door(ch, type, dir)) >= 0) {

    /* perhaps it is a door */
    exitp = EXIT(ch, door);
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("That's impossible, I'm afraid.\n", ch);
    else if (!IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("It's already open!\n", ch);
    else if (IS_SET(exitp->exit_info, EX_LOCKED))
      send_to_char("It seems to be locked.\n", ch);
    else
      {
	open_door(ch, door);
	send_to_char("Ok.\n", ch);
      }
  }
}


void do_close(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct room_direction_data *back, *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  struct room_data	*rp;


  argument_interpreter(argument, type, dir);

  if (!*type)
    send_to_char("Close what?\n", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &victim, &obj)) {

    /* this is an object */

    if (obj->obj_flags.type_flag != ITEM_CONTAINER &&
	obj->obj_flags.type_flag != ITEM_SPELL_POUCH)
      send_to_char("That's not a container.\n", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("But it's already closed!\n", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
      send_to_char("That's impossible.\n", ch);
    else
      {
	SET_BIT(obj->obj_flags.value[1], CONT_CLOSED);
	send_to_char("Ok.\n", ch);
	act("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM);
      }
  } else if ((door = find_door(ch, type, dir)) >= 0) {

    /* Or a door */
    exitp = EXIT(ch, door);
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n", ch);
    else if (IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("It's already closed!\n", ch);
    else      {
      SET_BIT(exitp->exit_info, EX_CLOSED);
      if (exitp->keyword)
	act("$n closes the $F.", 0, ch, 0, exitp->keyword,
	    TO_ROOM);
      else
	act("$n closes the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("Ok.\n", ch);
      /* now for closing the other side, too */
      if (exit_ok(exitp,&rp) &&
	  (back = rp->dir_option[rev_dir[door]]) &&
	  (back->to_room == ch->in_room) ) {
	SET_BIT(back->exit_info, EX_CLOSED);
	if (back->keyword)    {
	  sprintf(buf, "The %s closes quietly.\n", back->keyword);
	  send_to_room(buf, exitp->to_room);
	}
	else
	  send_to_room( "The door closes quietly.\n", exitp->to_room);
      }
    }
  }
}

/* a lot like has_key, but useful for other stuff and returns obj - SLB */
struct obj_data *has_readied(struct char_data *ch, int vnum)
{
  struct obj_data *obj;

  for(obj = ch->carrying; obj; obj = obj->next_content)
    if(obj->virtual == vnum) return(obj);

  if(ch->equipment[HOLD])
    if(ch->equipment[HOLD]->virtual == vnum) return(obj);

  return(NULL);
}

int has_key(struct char_data *ch, int key)
{
  struct obj_data *o;

  for (o = ch->carrying; o; o = o->next_content)
    if (o->virtual== key)
      return(1);

  if (ch->equipment[HOLD])
    if (ch->equipment[HOLD]->virtual == key)
      return(1);

  return(0);
}


void do_lock(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back, *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  struct room_data *rp;

  argument_interpreter(argument, type, dir);

  if (!*type)
    send_to_char("Lock what?\n", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &victim, &obj)) {

    /* this is an object */

    if (obj->obj_flags.type_flag != ITEM_CONTAINER &&
	obj->obj_flags.type_flag != ITEM_SPELL_POUCH)
      send_to_char("That's not a container.\n", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("Maybe you should close it first...\n", ch);
    else if (obj->obj_flags.value[2] < 0)
      send_to_char("That thing can't be locked.\n", ch);
    else if (!has_key(ch, obj->obj_flags.value[2]))
      send_to_char("You don't seem to have the proper key.\n", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("It is locked already.\n", ch);
    else
      {
	SET_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	send_to_char("*Cluck*\n", ch);
	act("$n locks $p - 'cluck', it says.", FALSE, ch, obj, 0, TO_ROOM);
      }
  } else if ((door = find_door(ch, type, dir)) >= 0) {

    /* a door, perhaps */
    exitp = EXIT(ch, door);

    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n", ch);
    else if (!IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("You have to close it first, I'm afraid.\n", ch);
    else if (exitp->key < 0)
      send_to_char("There does not seem to be any keyholes.\n", ch);
    else if (!has_key(ch, exitp->key))
      send_to_char("You don't have the proper key.\n", ch);
    else if (IS_SET(exitp->exit_info, EX_LOCKED))
      send_to_char("It's already locked!\n", ch);
    else
      {
	SET_BIT(exitp->exit_info, EX_LOCKED);
	if (exitp->keyword)
	  act("$n locks the $F.", 0, ch, 0,  exitp->keyword,
	      TO_ROOM);
	else
	  act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("*Click*\n", ch);
	/* now for locking the other side, too */
	rp = real_roomp(exitp->to_room);
	if (rp &&
	    (back = rp->dir_option[rev_dir[door]]) &&
	    back->to_room == ch->in_room)
	  SET_BIT(back->exit_info, EX_LOCKED);
      }
  }
}

int phys_unlock_door(struct char_data *ch, struct obj_data *key, int dir)
{
  struct room_direction_data *back, *exitp;
  struct room_data *rp;

  exitp = EXIT(ch, dir);

  if(!IS_SET(exitp->exit_info, EX_ISDOOR)){
    sendf(ch, "That's absurd.\n");
    return(0);
  }
  else if (!IS_SET(exitp->exit_info, EX_CLOSED)){
    sendf(ch, "Perhaps you'd like to close it first.\n");
    return(0);
  }
  else if (exitp->key < 0){
    sendf(ch, "There doesn't seem to be a keyhole.\n");
    return(0);
  }
  else if (!IS_SET(exitp->exit_info, EX_LOCKED)){
    sendf(ch, "It's already unlocked, it seems.\n", ch);
    return(0);
  }
  else {
    CLEAR_BIT(exitp->exit_info, EX_LOCKED);
    if (exitp->keyword)
      act("$n unlocks the $F with $p.", 0, ch, key, exitp->keyword,
        TO_ROOM);
    else
      act("$n unlocks the door with $p.", FALSE, ch, key, 0, TO_ROOM);
    send_to_char("*click*\n", ch);
    /* now for unlocking the other side, too */
    rp = real_roomp(exitp->to_room);
    if (rp && (back = rp->dir_option[rev_dir[dir]]) &&
        back->to_room == ch->in_room){
      CLEAR_BIT(back->exit_info, EX_LOCKED);
      /* in here put some sort of message for alert people to hear the door */
    }
    return(1);
  }
}

void do_unlock(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *exitp;
  struct obj_data *obj, *key;
  struct char_data *victim;

  argument_interpreter(argument, type, dir);

  if (!*type)
    send_to_char("Unlock what?\n", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &victim, &obj)) {

    /* this is an object */

    if (obj->obj_flags.type_flag != ITEM_CONTAINER &&
	obj->obj_flags.type_flag != ITEM_SPELL_POUCH)
      send_to_char("That's not a container.\n", ch);
    else if (obj->obj_flags.value[2] < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\n", ch);
    else if (!has_key(ch, obj->obj_flags.value[2]))
      send_to_char("You don't seem to have the proper key.\n", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("Oh.. it wasn't locked, after all.\n", ch);
    else
      {
	CLEAR_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	send_to_char("*Click*\n", ch);
	act("$n unlocks $p.", FALSE, ch, obj, 0, TO_ROOM);
      }
  }
  else if ((door = find_door(ch, type, dir)) >= 0){
    exitp = EXIT(ch, door);
    if ((key = has_readied(ch, exitp->key))) {
      phys_unlock_door(ch, key, door);
    }
    else{
      sendf(ch, "You don't have the proper key.");
    }
  }
}





void do_pick(struct char_data *ch, char *argument, int cmd)
{
  byte percent;
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back, *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  struct room_data	*rp;

  argument_interpreter(argument, type, dir);

  percent=number(1,101); /* 101% is a complete failure */

  if (percent > get_skill(ch,SKILL_PICK_LOCK)) {
    send_to_char("You failed to pick the lock.\n", ch);
    return;
  }

  if (!*type) {
    send_to_char("Pick what?\n", ch);
  } else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			  ch, &victim, &obj)) {

    /* this is an object */

    if (obj->obj_flags.type_flag != ITEM_CONTAINER &&
	obj->obj_flags.type_flag != ITEM_SPELL_POUCH)
      send_to_char("That's not a container.\n", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("Silly - it ain't even closed!\n", ch);
    else if (obj->obj_flags.value[2] < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\n", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("Oho! This thing is NOT locked!\n", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF))
      send_to_char("It resists your attempts at picking it.\n", ch);
    else
      {
	CLEAR_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	send_to_char("*Click*\n", ch);
	act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM);
      }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    exitp = EXIT(ch, door);
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n", ch);
    else if (!IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("You realize that the door is already open.\n", ch);
    else if (exitp->key < 0)
      send_to_char("You can't seem to spot any lock to pick.\n", ch);
    else if (!IS_SET(exitp->exit_info, EX_LOCKED))
      send_to_char("Oh.. it wasn't locked at all.\n", ch);
    else if (IS_SET(exitp->exit_info, EX_PICKPROOF))
      send_to_char("You seem to be unable to pick this lock.\n", ch);
    else {
      CLEAR_BIT(exitp->exit_info, EX_LOCKED);
      if (exitp->keyword)
	act("$n skillfully picks the lock of the $F.", 0, ch, 0,
	    exitp->keyword, TO_ROOM);
      else
	act("$n picks the lock.",TRUE,ch,0,0,TO_ROOM);
      send_to_char("The lock quickly yields to your skills.\n", ch);
      /* now for unlocking the other side, too */
      rp = real_roomp(exitp->to_room);
      if (rp &&
	  (back = rp->dir_option[rev_dir[door]]) &&
	  back->to_room == ch->in_room )
	CLEAR_BIT(back->exit_info, EX_LOCKED);
    }
  }
}

void do_enter(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char buf[MAX_INPUT_LENGTH], tmp[MAX_STRING_LENGTH];
  struct room_direction_data	*exitp;
  struct room_data	*rp;

  one_argument(argument, buf);

  if (*buf) { /* an argument was supplied, search for door keyword */
    for (door = 0; door <= 5; door++)
      if (exit_ok(exitp=EXIT(ch, door), NULL) && exitp->keyword &&
	  0==str_cmp(exitp->keyword, buf)) {
	do_move(ch, "", ++door);
	return;
      }
    sprintf(tmp, "There is no %s here.\n", buf);
    send_to_char(tmp, ch);
  } else if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    send_to_char("You are already indoors.\n", ch);
  } else {
    /* try to locate an entrance */
    for (door = 0; door <= 5; door++)
      if (exit_ok(exitp=EXIT(ch, door), &rp) &&
	  !IS_SET(exitp->exit_info, EX_CLOSED) &&
	  IS_SET(rp->room_flags, INDOORS))
	{
	  do_move(ch, "", ++door);
	  return;
	}
    send_to_char("You can't seem to find anything to enter.\n", ch);
  }
}


void do_leave(struct char_data *ch, char *argument, int cmd)
{
  int door;
  struct room_direction_data	*exitp;
  struct room_data	*rp;

  if (!IS_SET(RM_FLAGS(ch->in_room), INDOORS))
    send_to_char("You are outside.. where do you want to go?\n", ch);
  else    {
      for (door = 0; door <= 5; door++)
	if (exit_ok(exitp=EXIT(ch, door), &rp) &&
	    !IS_SET(exitp->exit_info, EX_CLOSED) &&
	    !IS_SET(rp->room_flags, INDOORS)) {
	  do_move(ch, "", ++door);
	  return;
	}
      send_to_char("I see no obvious exits to the outside.\n", ch);
    }
}

void do_pray(struct char_data *ch, char *argument, int cmd)
{
  switch(GET_POS(ch)) {
    case POSITION_STANDING:
      act("You kneel down and begin to pray.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n kneels down and begins to pray.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_PRAYING;
      break;
    case POSITION_SITTING:
      act("You kneel where you are sitting and begin to pray.", FALSE, ch, 0,
	0, TO_CHAR);
      act("$n stops sitting, kneels, and begins to pray.", TRUE, ch, 0, 0,
	TO_ROOM);
      GET_POS(ch) = POSITION_PRAYING;
      break;
    case POSITION_PRAYING:
      act("You are already kneeled down in prayer.", FALSE, ch, 0, 0, TO_CHAR);
      break;
    case POSITION_RESTING:
      act("You clamber on your feet and kneel down in prayer.", FALSE, ch, 0,
	0, TO_CHAR);
      act("$n clambers on $s feet and then kneels down in prayer.", TRUE, ch,
	0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_PRAYING;
      break;
    case POSITION_SLEEPING:
      act("You have to wake up first!", FALSE, ch, 0, 0, TO_CHAR);
      break;
    case POSITION_FIGHTING:
      act("You may have a wee bit of difficulty concentrating.", FALSE, ch, 0,
	0, TO_CHAR);
      break;
    default:
      act("You fall on your knees to the ground with a thud.", FALSE, ch, 0, 0,
	TO_CHAR);
      act("$n plummets from the air and lands on $s knees with a thud.", TRUE,
	ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_PRAYING;
      break;
  }
}

void do_stand(struct char_data *ch, char *argument, int cmd)
{
  switch(GET_POS(ch)) {
  case POSITION_STANDING : {
    act("You are already standing.",FALSE, ch,0,0,TO_CHAR);
  } break;
  case POSITION_SITTING	: {
    act("You stand up.", FALSE, ch,0,0,TO_CHAR);
    act("$n clambers on $s feet.",TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_STANDING;
  } break;
  case POSITION_PRAYING:
    act("You stop praying and stand up.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops praying and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_STANDING;
    break;
  case POSITION_RESTING	: {
    act("You stop resting, and stand up.", FALSE, ch,0,0,TO_CHAR);
    act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_STANDING;
  } break;
  case POSITION_SLEEPING : {
    act("You have to wake up first!", FALSE, ch, 0,0,TO_CHAR);
  } break;
  case POSITION_FIGHTING : {
    act("Do you not consider fighting as standing?",FALSE, ch, 0, 0, TO_CHAR);
  } break;
    default : {
      act("You stop floating around, and put your feet on the ground.",
	  FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and puts $s feet on the ground.",
	  TRUE, ch, 0, 0, TO_ROOM);
    } break;
  }
}


void do_sit(struct char_data *ch, char *argument, int cmd)
{
  switch(GET_POS(ch)) {
  case POSITION_STANDING : {
    act("You sit down.", FALSE, ch, 0,0, TO_CHAR);
    act("$n sits down.", FALSE, ch, 0,0, TO_ROOM);
    GET_POS(ch) = POSITION_SITTING;
  } break;
  case POSITION_SITTING	: {
    send_to_char("You're sitting already.\n", ch);
  } break;
  case POSITION_PRAYING:
    act("You stop praying and sit.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops praying and sits.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_SITTING;
    break;
  case POSITION_RESTING	: {
    act("You stop resting, and sit up.", FALSE, ch,0,0,TO_CHAR);
    act("$n stops resting and sits up.", TRUE, ch, 0,0,TO_ROOM);
    GET_POS(ch) = POSITION_SITTING;
  } break;
  case POSITION_SLEEPING : {
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_FIGHTING : {
    act("Sit down while fighting? are you MAD?", FALSE, ch,0,0,TO_CHAR);
  } break;
    default : {
      act("You stop floating around, and sit down.", FALSE, ch,0,0,TO_CHAR);
      act("$n stops floating around, and sits down.", TRUE, ch,0,0,TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
    } break;
  }
}


void do_rest(struct char_data *ch, char *argument, int cmd)
{
  switch(GET_POS(ch)) {
  case POSITION_STANDING : {
    act("You sit down and rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_RESTING;
  } break;
  case POSITION_SITTING : {
    act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_RESTING;
  } break;
  case POSITION_PRAYING:
    act("You stop praying and rest yourself.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops praying and rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_RESTING;
    break;
  case POSITION_RESTING : {
    act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_SLEEPING : {
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_FIGHTING : {
    act("Rest while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
  } break;
    default : {
      act("You stop floating around, and stop to rest your tired bones.",
	  FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and rests.", FALSE, ch, 0,0, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
    } break;
  }
}


void do_sleep(struct char_data *ch, char *argument, int cmd)
{

  switch(GET_POS(ch)) {
  case POSITION_STANDING :
  case POSITION_SITTING  :
  case POSITION_PRAYING  :
  case POSITION_RESTING  : {
    send_to_char("You go to sleep.\n", ch);
    act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_SLEEPING;
  } break;
  case POSITION_SLEEPING : {
    send_to_char("You are already sound asleep.\n", ch);
  } break;
  case POSITION_FIGHTING : {
    send_to_char("Sleep while fighting? are you MAD?\n", ch);
  } break;
    default : {
      act("You stop floating around, and lie down to sleep.",
	  FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and lie down to sleep.",
	  TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SLEEPING;
    } break;
  }
}


void do_wake(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *tmp_char;
  char arg[MAX_STRING_LENGTH];


  one_argument(argument,arg);
  if (*arg) {
    if (GET_POS(ch) == POSITION_SLEEPING) {
      act("You can't wake people up if you are asleep yourself!",
	  FALSE, ch,0,0,TO_CHAR);
    } else {
      tmp_char = get_char_room_vis(ch, arg);
      if (tmp_char) {
	if (tmp_char == ch) {
	  act("If you want to wake yourself up, just type 'wake'",
	      FALSE, ch,0,0,TO_CHAR);
	} else {
	  if (GET_POS(tmp_char) == POSITION_SLEEPING) {
	    if (IS_AFFECTED(tmp_char, AFF_SLEEP)) {
	      act("You can not wake $M up!", FALSE, ch, 0, tmp_char, TO_CHAR);
	    } else {
	      act("You wake $M up.", FALSE, ch, 0, tmp_char, TO_CHAR);
	      GET_POS(tmp_char) = POSITION_SITTING;
	      act("You are awakened by $n.", FALSE, ch, 0, tmp_char, TO_VICT);
	    }
	  } else {
	    act("$N is already awake.",FALSE,ch,0,tmp_char, TO_CHAR);
	  }
	}
      } else {
	send_to_char("You do not see that person here.\n", ch);
      }
    }
  } else {
    if (IS_AFFECTED(ch,AFF_SLEEP)) {
      send_to_char("You can't wake up!\n", ch);
    } else {
      if (GET_POS(ch) > POSITION_SLEEPING)
	send_to_char("You are already awake...\n", ch);
      else {
	send_to_char("You wake, and sit up.\n", ch);
	act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POSITION_SITTING;
      }
    }
  }
}


void do_follow(struct char_data *ch, char *argument, int cmd)
{
  char name[160];
  struct char_data *leader;


  only_argument(argument, name);

  if (*name) {
    if (!(leader = get_char_room_vis(ch, name))) {
      send_to_char("I see no person by that name here!\n", ch);
      return;
    }
  } else {
    send_to_char("Who do you wish to follow?\n", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master)) {

    act("But you only feel like following $N!",
	FALSE, ch, 0, ch->master, TO_CHAR);

  } else { /* Not Charmed follow person */

    if (leader == ch) {
      if (!ch->master) {
	send_to_char("You are already following yourself.\n", ch);
	return;
      }
      stop_follower(ch);
    } else {
      if (circle_follow(ch, leader)) {
	act("Sorry, but following in 'loops' is not allowed", FALSE, ch, 0, 0, TO_CHAR);
	return;
      }
	if (IS_NPC (leader)) {
		send_to_char("Don't you know following monsters is bad for your health\n.", ch);
		return;
	}
      	if (ch->master)
		stop_follower(ch);
	clear_bit(ch->specials.affected_by, AFF_GROUP);

		if (count_followers (leader) >= (((GET_CHR (leader) - 8) / 2) + 1)) {
			sendf (ch, "%s already has enough followers. You can not join.\n", GET_NAME (leader));
			return;
		}
      	add_follower(ch, leader);
    }
  }
}


void raw_unlock_door( struct char_data *ch, struct room_direction_data *exitp, int door)
{
  struct room_data *rp;
  struct room_direction_data *back;

  REMOVE_BIT(exitp->exit_info, EX_LOCKED);
  /* now for unlocking the other side, too */
  rp = real_roomp(exitp->to_room);
  if (rp &&
      (back = rp->dir_option[rev_dir[door]]) &&
      back->to_room == ch->in_room) {
    REMOVE_BIT(back->exit_info, EX_LOCKED);
  } else {
    vlog(LOG_URGENT,"Inconsistent door locks in rooms %d->%d", 
	    ch->in_room, exitp->to_room);
  }
}

void raw_lock_door( struct char_data *ch, 
		   struct room_direction_data *exitp, int door)
{
  struct room_data *rp;
  struct room_direction_data *back;

  SET_BIT(exitp->exit_info, EX_LOCKED);
  /* now for locking the other side, too */
  rp = real_roomp(exitp->to_room);
  if (rp &&
      (back = rp->dir_option[rev_dir[door]]) &&
      back->to_room == ch->in_room) {
    SET_BIT(back->exit_info, EX_LOCKED);
  } else {
    vlog(LOG_URGENT,"Inconsistent door locks in rooms %d->%d", 
	    ch->in_room, exitp->to_room);
  }
}


void raw_open_door(struct char_data *ch, int dir)
     /* remove all necessary bits and send messages */
{
  struct room_direction_data *exitp, *back;
  struct room_data	*rp;
  char	buf[MAX_INPUT_LENGTH];
  
  rp = real_roomp(ch->in_room);
  if (rp==NULL) {
    vlog(LOG_URGENT,"NULL rp in open_door() for %s.", PERS(ch,ch));
  }
  
  exitp = rp->dir_option[dir];
  
  REMOVE_BIT(exitp->exit_info, EX_CLOSED);
  /* now for opening the OTHER side of the door! */
  if (exit_ok(exitp, &rp) &&
      (back = rp->dir_option[rev_dir[dir]]) &&
      (back->to_room == ch->in_room))    {
      REMOVE_BIT(back->exit_info, EX_CLOSED);
      if (back->keyword && (strcmp("secret", fname(back->keyword))))	{
	sprintf(buf, "The %s is opened from the other side.\n",
		fname(back->keyword));
	send_to_room(buf, exitp->to_room);
      } else {
	send_to_room("The door is opened from the other side.\n",
		     exitp->to_room);
      }
    }						 
}

int check_falling( struct char_data *ch)
 {
 return(FALSE);
 }
