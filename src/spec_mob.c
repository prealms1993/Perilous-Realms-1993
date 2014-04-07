/* ************************************************************************
*  file: spec_procs.c , Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <fcns.h>
#include <externs.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <spells.h>
#include <limits.h>
#include <hash.h>
#include <rooms.h>

#undef howmany
#define INQ_SHOUT 1
#define INQ_LOOSE 0

#define SWORD_ANCIENTS 25000

#define INSURANCE_GUY 3054
#define INSURANCE_TOKEN 25
#define INSURANCE_PREMIUM_PER_LEVEL 1500
#define DEDUCTABLE_MULT 1.0
#define BUYBACK_MULT 1.05
/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct time_info_data time_info;
extern struct weather_data weather_info;
extern int top_of_world;
extern struct int_app_type int_app[26];

int deductable(struct char_data *ch,int threshold)
{
  return(value_of_char_equip(ch,threshold)*DEDUCTABLE_MULT);
}

struct char_data *InsuranceGuy()
{
  struct char_data *ch;
  for (ch=character_list; ch; ch=ch->next)
   if (ch->virtual==INSURANCE_GUY) return(ch);
  return(NULL);
}

int contains_item(struct obj_data *obj, ulong vnum)
{
  if (!obj) return(FALSE);
  if (obj->virtual==vnum) return(TRUE);
  if (contains_item(obj->contains,vnum)) return(TRUE);
  return(contains_item(obj->next_content,vnum));
}

int has_object(struct char_data *ch, ulong vnum)
{
  int i;
  for (i=0; i<MAX_WEAR; i++)
    if (contains_item(ch->equipment[i],vnum)) return(TRUE);
  return(contains_item(ch->carrying,vnum));
}


int insurance_salesman(struct char_data *ch,int cmd, char *arg)
{
  struct char_data *guy;
  int cost,use_cost;
  int count;
  struct obj_data *obj;

  guy=FindMobInRoomWithFunction(ch->in_room,insurance_salesman);
  if (!guy) {
	nlog("Can't find myself -- insurance_salesman.");
	return(FALSE);
  }
  if (cmd<=0) return(FALSE);
  cost = GetMaxLevel(ch)*INSURANCE_PREMIUM_PER_LEVEL;
  if (cmd==93) { /* offer */
	if (atoi(arg)>0)
	  use_cost = deductable(ch,atoi(arg));
	else
	  use_cost = deductable(ch,10000);
	sendf(ch,"If you were in need of my services right now,\n"
		 "the deductable would be %d.\n",use_cost);
	return(TRUE);
  }
  if (cmd==59) { /* list */
    count=0;
    if (!has_object(ch,INSURANCE_TOKEN))
	sendf(ch,"%2d. Insurance Token -- %d\n",++count,cost);
    for (obj=guy->carrying; obj; obj=obj->next_content)
	if (obj->held_for && strcmp(obj->held_for,GET_NAME(ch))==0) {
	  cost=(int)(BUYBACK_MULT*real_cost(ch,obj,0));
	  sendf(ch,"%2d. %s -- %d\n",++count,obj->short_description,cost);
	}
    if (!count)
	sendf(ch,"I have nothing of yours, and you can only have 1 token at a time.\n");
    return(TRUE);
  }
  if (cmd==56) { /* buy */
	int which;
	which=atoi(arg);
	if (which<1) {
		sendf(ch,"You must type buy 1, buy 2, etc... Type LIST to see what I have.\n");
		return(TRUE);
	}
	if (!has_object(ch,INSURANCE_TOKEN) && which==1) {
	  sendf(ch,"That will be %d please.\n",cost);
	  if (GET_GOLD(ch)<cost) {
		sendf(ch,"When you can afford it, come on back!\n");
		return(TRUE);
	  }
	  GET_GOLD(ch)-=cost;
	  obj=get_obj(25);
	  obj_to_char(obj,ch);
	  sendf(ch,"You give him the money.\nHe gives you the token.\n");
	  act("$n buys an insurance token.\n",TRUE,ch,0,0,TO_ROOM);
	  return(TRUE);
	}
	if (!has_object(ch,INSURANCE_TOKEN)) which--;
	if (!which) {
	  sendf(ch,"I don't have one of those.\n");
	  return(TRUE);
	}
	for (obj=guy->carrying; obj; obj=obj->next_content)
	  if (obj->held_for && strcmp(obj->held_for,GET_NAME(ch))==0)
		if (!--which) break;
	if (!obj) {
	  sendf(ch,"What exactly are you talking about?\n");
	  return(TRUE);
	}
	cost=(int)(BUYBACK_MULT*real_cost(ch,obj,0));
	sendf(ch,"That will be %d please.\n",cost);
	if (GET_GOLD(ch)<cost) {
	      sendf(ch,"When you can afford it, come on back!\n");
	      return(TRUE);
	}
	GET_GOLD(ch)-=cost;
	obj_from_char(obj);
	obj_to_char(obj,ch);
	act("$n buys something.\n",TRUE,ch,0,0,TO_ROOM);
	FREE(obj->held_for);
	return(TRUE);
  }
  return(FALSE);
}
struct social_type {
  char *cmd;
  int next_line;
};



/*************************************/
/* predicates for find_path function */

//Broken on 64-bit.  
int is_target_room_p(int room, void *tgt_room)
{
  return room == *((int*)tgt_room);
}


int named_object_on_ground(int room, void *c_data)
{
  char	*name = c_data;
  return NULL!=get_obj_in_list(name, real_roomp(room)->contents);
}

/* predicates for find_path function */
/*************************************/

void npc_steal(struct char_data *ch,struct char_data *victim)
{
  int gold;

  if(IS_NPC(victim)) return;
  if(GetMaxLevel(victim)>MAX_MORT) return;

  if (AWAKE(victim) && (number(0,GetMaxLevel(ch)) == 0)) {
    act("You discover that $n has $s hands in your wallet.",FALSE,ch,0,victim,TO_VICT);
    act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /* Steal some gold coins */
    gold = (int) ((GET_GOLD(victim)*number(1,10))/100);
    if (gold > 0) {
      GET_GOLD(ch) += gold;
      GET_GOLD(victim) -= gold;
    }
  }
}


int snake(struct char_data *ch, int cmd, char *arg)
{
  if (cmd || !AWAKE(ch))
    return(FALSE);

  if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n poisons $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n poisons you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_poison( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
		ch->specials.fighting, 0,GetMaxLevel(ch));
    return TRUE;
  }
  return FALSE;
}



#define PGShield 25100

int PaladinGuildGuard( struct char_data *ch, int cmd, char *arg)
{

  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      if (GET_POS(ch) == POSITION_FIGHTING) {
	FighterMove(ch);
      } else {
	StandUp(ch);
      }
    }
  } else if (cmd >= 1 && cmd <= 6) {
    if (cmd == 4) return(FALSE);  /* can always go west */
    if (!HasObject(ch, PGShield)) {
      send_to_char
	("The guard shakes his head, and blocks your way.\n", ch);
      act("The guard shakes his head, and blocks $n's way.",
	  TRUE, ch, 0, 0, TO_ROOM);
      return(TRUE);
    }
  }
  return(FALSE);
}

int AbyssGateKeeper( struct char_data *ch, int cmd, char *arg)
{

  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      if (GET_POS(ch) == POSITION_FIGHTING) {
	FighterMove(ch);
      } else {
	StandUp(ch);
      }
    }
  } else if ((cmd >= 1 && cmd <= 6)&&(!IS_IMMORTAL(ch))) {
    if ((cmd == 6) || (cmd == 1)) {
      send_to_char
	("The gatekeeper shakes his head, and blocks your way.\n", ch);
      act("The guard shakes his head, and blocks $n's way.",
	  TRUE, ch, 0, 0, TO_ROOM);
      return(TRUE);
    }
  }
  return(FALSE);
}

int blink( struct char_data *ch, int cmd, char *arg)
{
  if (cmd || !AWAKE(ch))
    return(FALSE);


  if (GET_HIT(ch) < (int)GET_MAX_HIT(ch) / 3) {
    act("$n blinks.",TRUE,ch,0,0,TO_ROOM);
    cast_teleport( 12, ch, "", SPELL_TYPE_SPELL, ch, 0,GetMaxLevel(ch));
    return(TRUE);
  } else {
    return(FALSE);
  }
}

int WeapRepairGuy( struct char_data *ch, int cmd, char *arg)
{
  char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
  int cost;
  struct char_data *vict;
  struct obj_data *obj, *temp;
  int (*wrep_guy)();  /* special procedure for this mob/obj       */


  if (!AWAKE(ch))
    return(FALSE);

  wrep_guy = WeapRepairGuy;


  if (cmd == 72) { /* give */
    /* determine the correct obj */
    arg=one_argument(arg,obj_name);
    if (!*obj_name) {
      send_to_char("Give what?\n",ch);
      return(FALSE);
    }
    if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
      send_to_char("Give what?\n",ch);
      return(FALSE);
    }
    arg=one_argument(arg, vict_name);
    if(!*vict_name)	{
      send_to_char("To who?\n",ch);
      return(FALSE);
    }
    if (!(vict = get_char_room_vis(ch, vict_name)))	{
      send_to_char("To who?\n",ch);
      return(FALSE);
    }
    /* the target is the repairman, or an NPC */
    if (!IS_NPC(vict))
      return(FALSE);
    if (real_mobp(vict->virtual)->func == wrep_guy) {
      /* we have the repair guy, and we can give him the stuff */
      act("You give $p to $N.",TRUE,ch,obj,vict,TO_CHAR);
      act("$n gives $p to $N.",TRUE,ch,obj,vict,TO_ROOM);
    } else {
      return(FALSE);
    }

    act("$N looks at $p.", TRUE, ch, obj, vict, TO_CHAR);
    act("$N looks at $p.", TRUE, ch, obj, vict, TO_ROOM);

    /* make all the correct tests to make sure that everything is kosher */

    if (ITEM_TYPE(obj,ITEM_WEAPON)) {
       cost = obj->obj_flags.cost;
       temp = real_objp(obj->virtual)->obj;
       if (obj->obj_flags.value[2])
         cost /= obj->obj_flags.value[2];

       cost *= ((temp->obj_flags.value[2] - obj->obj_flags.value[2])*2);

       if (cost > GET_GOLD(ch)) {
         act("$N says 'I'm sorry, you don't have enough money.'",TRUE, ch, 0, vict, TO_ROOM);
         act("$N says 'I'm sorry, you don't have enough money.'",TRUE, ch, 0, vict, TO_CHAR);
        } else {
          GET_GOLD(ch) -= cost;
          sprintf(buf, "You give $N %d coins.",cost);
          act(buf,TRUE,ch,0,vict,TO_CHAR);
          act("$n gives some money to $N.",TRUE,ch,obj,vict,TO_ROOM);

          /* fix the weapon */
          act("$N fiddles with $p.",TRUE,ch,obj,vict,TO_ROOM);
          act("$N fiddles with $p.",TRUE,ch,obj,vict,TO_CHAR);
          obj->obj_flags.value[2] = temp->obj_flags.value[2];
          act("$N says 'All fixed.'",TRUE,ch,0,vict,TO_ROOM);
          act("$N says 'All fixed.'",TRUE,ch,0,vict,TO_CHAR);
        }
        act("$N gives you $p.",TRUE,ch,obj,vict,TO_CHAR);
        act("$N gives $p to $n.",TRUE,ch,obj,vict,TO_ROOM);
        return(TRUE);
       } else {
          act("$N says 'That isn't a weapon.'",TRUE,ch,0,vict,TO_ROOM);
          act("$N says 'That isn't a weapon.'",TRUE,ch,0,vict,TO_CHAR);
          act("$N says 'I can't fix that...'", TRUE, ch, 0, vict, TO_CHAR);
          act("$N says 'I can't fix that...'", TRUE, ch, 0, vict, TO_ROOM);
         }
	 }
	 else {
       if (cmd) return FALSE; 
         return(puff(ch, cmd, arg));
       }
  return(FALSE);
}

int RepairGuy( struct char_data *ch, int cmd, char *arg)
 {
  char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
  int cost, ave;
  struct char_data *vict;
  struct obj_data *obj;
  int (*rep_guy)();  /* special procedure for this mob/obj       */


  if (!AWAKE(ch))
    return(FALSE);

  rep_guy = RepairGuy;


  if (cmd == 72) { /* give */
    /* determine the correct obj */
    arg=one_argument(arg,obj_name);
    if (!*obj_name) {
      send_to_char("Give what?\n",ch);
      return(FALSE);
    }
    if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
      send_to_char("Give what?\n",ch);
      return(FALSE);
    }
    arg=one_argument(arg, vict_name);
    if(!*vict_name)	{
      send_to_char("To who?\n",ch);
      return(FALSE);
    }
    if (!(vict = get_char_room_vis(ch, vict_name)))	{
      send_to_char("To who?\n",ch);
      return(FALSE);
    }
    /* the target is the repairman, or an NPC */
    if (!IS_NPC(vict))
      return(FALSE);
    if (real_mobp(vict->virtual)->func == rep_guy) {
      /* we have the repair guy, and we can give him the stuff */
      act("You give $p to $N.",TRUE,ch,obj,vict,TO_CHAR);
      act("$n gives $p to $N.",TRUE,ch,obj,vict,TO_ROOM);
    } else {
      return(FALSE);
    }

    act("$N looks at $p.", TRUE, ch, obj, vict, TO_CHAR);
    act("$N looks at $p.", TRUE, ch, obj, vict, TO_ROOM);

    /* make all the correct tests to make sure that everything is kosher */

    if (ITEM_TYPE(obj,ITEM_ARMOR)) {
      if (obj->obj_flags.value[1] > obj->obj_flags.value[0]) {
	/* get the value of the object */
	cost = obj->obj_flags.cost;
	/* divide by value[1]   */
	cost /= MAX(1,obj->obj_flags.value[1]);
	/* then cost = difference between value[0] and [1] */
	cost *= (obj->obj_flags.value[1] - obj->obj_flags.value[0]);
	if (GetMaxLevel(vict) > 25) /* super repair guy */
	  cost *= 2;
	if (cost > GET_GOLD(ch)) {
	  act("$N says 'I'm sorry, you don't have enough money.'",
	      TRUE, ch, 0, vict, TO_ROOM);
	  act("$N says 'I'm sorry, you don't have enough money.'",
	      TRUE, ch, 0, vict, TO_CHAR);
	} else {
	  GET_GOLD(ch) -= cost;

	  sprintf(buf, "You give $N %d coins.",cost);
	  act(buf,TRUE,ch,0,vict,TO_CHAR);
	  act("$n gives some money to $N.",TRUE,ch,obj,vict,TO_ROOM);

	  /* fix the armor */
	  act("$N fiddles with $p.",TRUE,ch,obj,vict,TO_ROOM);
	  act("$N fiddles with $p.",TRUE,ch,obj,vict,TO_CHAR);
	  if (GetMaxLevel(vict) > 25) {
	    obj->obj_flags.value[0] = obj->obj_flags.value[1];
	    CLEAR_BIT(obj->obj_flags.wear_flags,ITEM_DAMAGED);
	  } else {
	    ave = MAX(obj->obj_flags.value[0],
		      (obj->obj_flags.value[0] +
		       obj->obj_flags.value[1] ) /2);
	    obj->obj_flags.value[0] = ave;
	    obj->obj_flags.value[1] = ave;
	    CLEAR_BIT (obj->obj_flags.wear_flags, ITEM_DAMAGED);
	  }
	  act("$N says 'All fixed.'",TRUE,ch,0,vict,TO_ROOM);
	  act("$N says 'All fixed.'",TRUE,ch,0,vict,TO_CHAR);
	}
      } else {
	act("$N says 'Your armor looks fine to me.'",TRUE,ch,0,vict,TO_ROOM);
	act("$N says 'Your armor looks fine to me.'",TRUE,ch,0,vict,TO_CHAR);
      }
    } else {
      act("$N says 'That isn't armor.'",TRUE,ch,0,vict,TO_ROOM);
      act("$N says 'That isn't armor.'",TRUE,ch,0,vict,TO_CHAR);
    }

    act("$N gives you $p.",TRUE,ch,obj,vict,TO_CHAR);
    act("$N gives $p to $n.",TRUE,ch,obj,vict,TO_ROOM);
    return(TRUE);
  } else {
    return FALSE;
  }
}

int citizen(struct char_data *ch, int cmd, char *arg)
{
  int lev;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  lev = 3;

  if (ch->specials.fighting) {
    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
    } else {
      StandUp(ch);
    }


    if (number(0,18) == 0) {
      do_shout(ch, "Guards! Help me! Please!", 0);
    } else {
      act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0, TO_ROOM);
    }

    if (ch->specials.fighting)
      CallForGuard(ch, ch->specials.fighting, 3);

    return(TRUE);

  } else {
    return(FALSE);
  }
}

int ghoul(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *tar;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  tar = ch->specials.fighting;

  if (tar && (tar->in_room == ch->in_room)) {
    if ((!IS_AFFECTED(tar, AFF_PROTECT_EVIL)) &&
	(!IS_AFFECTED(tar, AFF_SANCTUARY))) {
      act("$n touches $N!", 1, ch, 0, tar, TO_NOTVICT);
      act("$n touches you!", 1, ch, 0, tar, TO_VICT);
      if (!IS_AFFECTED(tar, AFF_PARALYSIS)) {
	cast_paralyze( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,tar, 0,GetMaxLevel(ch));
	return TRUE;
      }
    }
  }
  return FALSE;
}

int WizardGuard(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
    } else {
      StandUp(ch);
    }
    CallForGuard(ch, ch->specials.fighting, 9);
  }
  max_evil = 1000;
  evil = 0;

  for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
	  (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
	max_evil = GET_ALIGNMENT(tch);
	evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0) &&
      !check_peaceful(ch, "")) {
    act("$n screams 'DEATH!!!!!!!!'",
	FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return(TRUE);
  }
  return(FALSE);
}



int vampire(struct char_data *ch, int cmd, char *arg)
{
  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_energy_drain( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
		      ch->specials.fighting, 0,GetMaxLevel(ch));
    if (ch->specials.fighting &&
	(ch->specials.fighting->in_room == ch->in_room)) {
      cast_energy_drain( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
			ch->specials.fighting, 0,GetMaxLevel(ch));
    }
    return TRUE;
  }
  return FALSE;
}

int wraith(struct char_data *ch, int cmd, char *arg)
{
  if (cmd || !AWAKE(ch))
    return(FALSE);


  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_energy_drain( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
		      ch->specials.fighting, 0,GetMaxLevel(ch));
    return TRUE;
  }
  return FALSE;
}


int shadow(struct char_data *ch, int cmd, char *arg)
{


  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_chill_touch( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
		     ch->specials.fighting, 0,GetMaxLevel(ch));
    if (ch->specials.fighting)
      cast_weakness( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
		    ch->specials.fighting, 0,GetMaxLevel(ch));
    return TRUE;
  }
  return FALSE;
}



int geyser(struct char_data *ch, int cmd, char *arg)
{

  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (number(0,3)==0) {
    act("You erupt.", 1, ch, 0, 0, TO_CHAR);
    cast_geyser( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, 0, 0,GetMaxLevel(ch));
    return(TRUE);
  }
  return(FALSE);
}


int green_slime(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *cons;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  for (cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room )
    if((!IS_NPC(cons)) && (GetMaxLevel(cons)<LOW_IMMORTAL))
      cast_green_slime( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, cons, 0,GetMaxLevel(ch));

  return(TRUE);
}

struct breath_victim {
  struct char_data	*ch;
  int	yesno; /* 1 0 */
  struct breath_victim	*next;
};

static struct breath_victim *choose_victims(struct char_data *ch,
				     struct char_data *first_victim)
{
  /* this is goofy, dopey extraordinaire */
  struct char_data *cons;
  struct breath_victim *head = NULL, *temp=NULL;

  for (cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room ) {
    temp = (void*)malloc(sizeof(*temp));
    temp->ch = cons;
    temp->next = head;
    head = temp;
    if (first_victim == cons) {
      temp->yesno = 1;
    } else if (ch == cons) {
      temp->yesno = 0;
    } else if ((in_group(first_victim, cons) ||
		cons == first_victim->master ||
		cons->master == first_victim) &&
	       (temp->yesno = (3 != dice(1,5))) ) {
      /* group members will get hit 4/5 times */
    } else if (cons->specials.fighting == ch) {
      /* people fighting the dragon get hit 4/5 times */
      temp->yesno = (3 != dice(1,5));
    } else /* bystanders get his 2/5 times */
      temp->yesno = (dice(1,5)<3);
  }
  return head;
}

static void free_victims(struct breath_victim *head)
{
  struct  breath_victim *temp;

  while (head) {
    temp = head->next;
    FREE(head);
    head = temp;
  }
}

int breath_weapon(struct char_data *ch, struct char_data *target, int mana_cost, void (*func)())
{
  struct breath_victim *hitlist, *scan;
  int	victim;


  hitlist = choose_victims(ch, target);

  act("$n rears back and inhales",1,ch,0,ch->specials.fighting,TO_ROOM);
  victim=0;
  for (scan = hitlist; scan; scan = scan->next) {
    if (!scan->yesno ||
	IS_IMMORTAL(scan->ch) ||
	scan->ch->in_room != ch->in_room /* this should not happen */
	)
      continue;
    victim=1;
    cast_fear( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, scan->ch, 0,GetMaxLevel(ch));
  }

  if (func!=NULL && victim) {
    act("$n Breathes...", 1, ch, 0, ch->specials.fighting, TO_ROOM);

    for (scan = hitlist; scan; scan = scan->next) {
      if (!scan->yesno ||
	  IS_IMMORTAL(scan->ch) ||
	  scan->ch->in_room != ch->in_room /* this could happen if
					      someone fled, I guess */
	  )
	continue;
      func( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, scan->ch, 0,GetMaxLevel(ch));
    }
    GET_MANA(ch) -= mana_cost;
  } else {
    act("$n Breathes...coughs and sputters...",
	1, ch, 0, ch->specials.fighting, TO_ROOM);
    do_flee(ch, "", 0);
  }

  free_victims(hitlist);
  return(1);
}

int use_breath_weapon(struct char_data *ch, struct char_data *target, int cost, void (*func)())
{
  if (GET_MANA(ch)>=0) {
    breath_weapon(ch, target, cost, func);
  } else if ((GET_HIT(ch) < GET_MAX_HIT(ch)/2) &&
	     (GET_MANA(ch) >= -cost)) {
    breath_weapon(ch, target, cost, func);
  } else if ((GET_HIT(ch) < GET_MAX_HIT(ch)/4) &&
	     (GET_MANA(ch) >= -2*cost)) {
    breath_weapon(ch, target, cost, func);
  } else if (GET_MANA(ch)<=-3*cost) {
    breath_weapon(ch, target, 0, NULL); /* sputter */
  }
  return(1);
}

static funcp breaths[] = {
  cast_acid_breath, 0, cast_frost_breath, 0, cast_lightning_breath, 0,
  cast_fire_breath, 0,
  cast_acid_breath, cast_fire_breath, cast_lightning_breath, 0
};

struct breather breath_monsters[] = {
  { 230,   55, breaths+0 },
  { 233,   55, breaths+4 },
  { 243,   55, breaths+2 },
  { 3952,  20, breaths+8 },
  { 5005,  55, breaths+4 },
  { 6112,  55, breaths+4 },
  { 6801,  55, breaths+2 },
  { 6802,  55, breaths+2 },
  { 6824,  55, breaths+0 },
  { 7040,  55, breaths+6 },
  { 18003, 20, breaths+8 },
  { 20002, 55, breaths+6 },
  { 20017, 55, breaths+6 },
  { 20016, 55, breaths+6 },
  { 20016, 55, breaths+6 },
  { 25009, 30, breaths+6 },
  { 25504, 30, breaths+4 },
  { 27016, 30, breaths+6 },
  { -1 },
};

int BreathWeapon(struct char_data *ch, int cmd, char *arg)
{
  char	buf[MAX_STRING_LENGTH];
  struct breather *scan;
  int	count;

  if (cmd)
    return FALSE;


  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {

    for (scan = breath_monsters;
	 scan->vnum >= 0 && scan->vnum != ch->virtual;
	 scan++)
      ;

    if (scan->vnum < 0) {
      sprintf(buf, "monster %s tries to breath, but isn't listed.",
	      ch->player.short_descr);
      log(buf);
      return FALSE;
    }

    for (count=0; scan->breaths[count]; count++)
      ;

    if (count<1) {
      sprintf(buf, "monster %s has no breath weapons",
	      ch->player.short_descr);
      log(buf);
      return FALSE;
    }

    use_breath_weapon(ch, ch->specials.fighting, scan->cost,
		      scan->breaths[dice(1,count)-1]);
  }

  return TRUE;
}

int DracoLich(struct char_data *ch, int cmd, char *arg)
{
  return(FALSE);
}
int Drow(struct char_data *ch, int cmd, char *arg)
{
  return(FALSE);
}
int Leader(struct char_data *ch, int cmd, char *arg)
{
  return(FALSE);
}


int thief(struct char_data *ch, int cmd, char *arg)
{
	struct char_data *cons;

  if (cmd || !AWAKE(ch))
    return(FALSE);

	if(GET_POS(ch)!=POSITION_STANDING)return FALSE;

	for(cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room )
		if((!IS_NPC(cons)) && (GetMaxLevel(cons)<LOW_IMMORTAL) && (number(1,5)==1))
			npc_steal(ch,cons);

	return TRUE;
}

int magic_user(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;
  byte lspell;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (!ch->specials.fighting) return FALSE;

  if ((GET_POS(ch) > POSITION_STUNNED) &&
      (GET_POS(ch) < POSITION_FIGHTING)) {
    StandUp(ch);
    return(TRUE);
  }

  /* Find a dude to to evil things upon ! */

  vict = FindVictim(ch);

  if (!vict)
    vict = ch->specials.fighting;

  if (!vict) return(FALSE);

  lspell = number(0,GetMaxLevel(ch)); /* gen number from 0 to level */

  if (lspell < 1)
    lspell = 1;

  if (IS_AFFECTED(ch, AFF_BLIND) && (lspell > 10)) {
    act("$n utters the words 'Let me see the light!'.",
	TRUE, ch, 0, 0, TO_ROOM);
    cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0,GetMaxLevel(ch));
    return TRUE;
  }

  if (IS_AFFECTED(ch, AFF_BLIND))
    return(FALSE);

  if ((vict!=ch->specials.fighting) && (lspell>13) && (number(0,7)==0)) {
    act("$n utters the words 'We begin the lecture...'.", 1, ch, 0, 0, TO_ROOM);
    cast_sleep(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
    return TRUE;
  }

  if ((lspell>5) && (number(0,6)==0)) {
    act("$n utters the words 'You wimp'.", 1, ch, 0, 0, TO_ROOM);
    cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
    return TRUE;
  }

  if ((lspell>5) && (number(0,7)==0)) {
    act("$n utters the words 'Bippety boppity Boom'.",1,ch,0,0,TO_ROOM);
    cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0,GetMaxLevel(ch));
    return TRUE;
  }

  if( (lspell>12) && (number(0,7)==0) )	{
    act("$n utters the words '&#%^^@%*#'.", 1, ch, 0, 0, TO_ROOM);
    cast_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
    return TRUE;
  }

  if( (lspell>7) && (number(0,5)==0) )	{
    act("$n utters the words 'yabba dabba do'.", 1, ch, 0, 0, TO_ROOM);
    cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
    return TRUE;
  }

  switch (lspell) {
  case 1:
  case 2:
  case 3:
  case 4:
    act("$n utters the words 'bang! bang! pow!'.", 1, ch, 0, 0, TO_ROOM);
    cast_magic_missile(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
    break;
  case 5:
    act("$n utters the words 'ZZZZzzzzzzTTTT'.", 1, ch, 0, 0, TO_ROOM);
    cast_shocking_grasp(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
    break;
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
    act("$n utters the words 'KAZAP!'.", 1, ch, 0, 0, TO_ROOM);
    cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
    break;
  case 11:
    act("$n utters the words 'Use MagicAway Instant Magic Remover'.", 1, ch, 0, 0, TO_ROOM);
    cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
    break;
  case 12:
  case 13:
  case 14:
    act("$n utters the words 'Look! A rainbow!'.", 1, ch, 0, 0, TO_ROOM);
    cast_colour_spray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
    break;
  case 15:
  case 16:
    act("$n utters the words 'Hasta la vista, Baby'.", 1, ch,0,0,TO_ROOM);
    cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
    break;
  case 17:
  case 18:
  case 19:
    if (IS_EVIL(ch))	{
      act("$n utters the words 'slllrrrrrrpppp'.", 1, ch, 0, 0, TO_ROOM);
      cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
      return TRUE;
    }
  default:
    act("$n utters the words 'Hasta la vista, Baby'.", 1, ch,0,0,TO_ROOM);
    cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
    break;


  }

  return TRUE;

}

int cleric(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;
  byte lspell, healperc=0;


  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (GET_POS(ch)!=POSITION_FIGHTING) {
    if ((GET_POS(ch)<POSITION_STANDING) && (GET_POS(ch)>POSITION_STUNNED)) {
      StandUp(ch);
    }
    return FALSE;
  }

  if (!ch->specials.fighting) return FALSE;


  /* Find a dude to to evil things upon ! */

  vict = FindVictim(ch);

  if (!vict)
    vict = ch->specials.fighting;

  if (!vict) return(FALSE);

  /*
    gen number from 0 to level
    */

  lspell = number(0,GetMaxLevel(ch));

  if (lspell < 1)
    lspell = 1;

  /*
    first -- hit a foe, or help yourself?
    */

  if (ch->points.hit < (ch->points.max_hit / 2))
    healperc = 3;
  else if (ch->points.hit < (ch->points.max_hit / 4))
    healperc = 5;
  else if (ch->points.hit < (ch->points.max_hit / 8))
    healperc=7;

  if (number(1,healperc+2)>2) {
    /* do harm */

    /* call lightning */
    if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING) && (lspell >= 15) &&
	(number(0,5)==0)) {
      act("$n whistles.",1,ch,0,0,TO_ROOM);
      act("$n utters the words 'Here Lightning!'.",1,ch,0,0,TO_ROOM);
      cast_call_lightning(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
      return(TRUE);
    }

    switch(lspell) {
    case 1:
    case 2:
    case 3:
      act("$n utters the words 'Moo ha ha!'.",1,ch,0,0,TO_ROOM);
      cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
      break;
    case 4:
    case 5:
    case 6:
      act("$n utters the words 'Hocus Pocus!'.",1,ch,0,0,TO_ROOM);
      cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
      break;
    case 7:
      act("$n utters the words 'Va-Voom!'.",1,ch,0,0,TO_ROOM);
      cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
      break;
    case 8:
      act("$n utters the words 'Urgle Blurg'.",1,ch,0,0,TO_ROOM);
      cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
      break;
    case 9:
    case 10:
      act("$n utters the words 'Take That!'.",1,ch,0,0,TO_ROOM);
      cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
      break;
    case 11:
      act("$n utters the words 'Burn Baby Burn'.",1,ch,0,0,TO_ROOM);
      cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
      break;
    case 12:
      act("$n utters the words 'Damn you!'.",1,ch,0,0,TO_ROOM);
      cast_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
      break;
    case 13:
    case 14:
    case 15:
    case 16:
      {
	if ((GET_ALIGNMENT(vict)<=0) && (GET_ALIGNMENT(ch)>0)) {
	  act("$n utters the words 'Begone Fiend!'.",1,ch,0,0,TO_ROOM);
	  cast_dispel_evil(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
	} else if ((GET_ALIGNMENT(vict)>=0) && (GET_ALIGNMENT(ch)<0)) {
	  act("$n utters the words 'Begone, Goody Goody!'.",1,ch,0,0,TO_ROOM);
	  cast_dispel_good(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
	} else {
	  if (!IS_SET(vict->specials.M_immune, IMM_FIRE)) {
	    act("$n utters the words 'Burn Baby Burn'.",1,ch,0,0,TO_ROOM);
	    cast_flamestrike(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0,GetMaxLevel(ch));
	  } else if (IS_AFFECTED(vict, AFF_SANCTUARY)) {
	    act("$n utters the words 'Va-Voom!'.",1,ch,0,0,TO_ROOM);
	    cast_dispel_magic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0,GetMaxLevel(ch));
	  } else {
	    act("$n utters the words 'Take That!'.",1,ch,0,0,TO_ROOM);
	    cast_cause_critic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
	  }
	}
       	break;
      }
    case 17:
    case 18:
    case 19:
    default:
      act("$n utters the words 'Hurts, doesn't it?\?'.",1,ch,0,0,TO_ROOM);
      cast_harm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
      break;
    }

    return(TRUE);

  } else {
    /* do heal */

    if (IS_AFFECTED(ch, AFF_BLIND) && (lspell >= 4) & (number(0,3)==0)) {
      act("$n utters the words 'Praise <Deity Name>, I can SEE!'.", 1, ch,0,0,TO_ROOM);
      cast_cure_blind( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0,GetMaxLevel(ch));
      return(TRUE);
    }

    if (IS_AFFECTED(ch, AFF_CURSE) && (lspell >= 6) && (number(0,6)==0)) {
      act("$n utters the words 'I'm rubber, you're glue.", 1, ch,0,0,TO_ROOM);
      cast_remove_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0,GetMaxLevel(ch));
      return(TRUE);
    }

    if (IS_AFFECTED(ch, AFF_POISON) && (lspell >= 5) && (number(0,6)==0)) {
      act("$n utters the words 'Praise <Deity Name> I don't feel sick no more!'.", 1, ch,0,0,TO_ROOM);
      cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0,GetMaxLevel(ch));
      return(TRUE);
    }


    switch(lspell) {
    case 1:
    case 2:
    case 3:
    case 4:
      act("$n utters the words 'Abrazak'.",1,ch,0,0,TO_ROOM);
      cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0,GetMaxLevel(ch));
      break;
    case 5:
    case 6:
    case 7:
    case 8:
      act("$n utters the words 'I feel good!'.", 1, ch,0,0,TO_ROOM);
      cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0,GetMaxLevel(ch));
      break;
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      act("$n utters the words 'Woah! I feel GOOD! Heh.'.", 1, ch,0,0,TO_ROOM);
      cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0,GetMaxLevel(ch));
      break;
    case 17:
    case 18: /* heal */
      act("$n utters the words 'What a Rush!'.", 1, ch,0,0,TO_ROOM);
      cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0,GetMaxLevel(ch));
      break;
    default:
      act("$n utters the words 'Oooh, pretty!'.", 1, ch,0,0,TO_ROOM);
      cast_sanctuary(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0,GetMaxLevel(ch));
      break;

    }

    return(TRUE);

  }
}


/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

int guild_guard(struct char_data *ch, int cmd, char *arg)
{
  char buf[256], buf2[256];

  if (cmd>6 || cmd<1)
    return FALSE;

  strcpy(buf,  "The guard humiliates you, and block your way.\n");
  strcpy(buf2, "The guard humiliates $n, and blocks $s way.");

  if ((IS_NPC(ch) && (IS_POLICE(ch))) || (GetMaxLevel(ch) >= DEMIGOD) ||
      (IS_AFFECTED(ch, AFF_SNEAK)))
    return(FALSE);

  /*
   **  Remove-For-Multi-Class
   */
  if ((ch->in_room == 3017) && (cmd == 3)) {
    if (!is_magicu(ch)) {
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      return TRUE;
    }
  } else if ((ch->in_room == 3004) && (cmd == 1)) {
    if (!is_cleric(ch)) {
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      return TRUE;
    }
  } else if ((ch->in_room == 3027) && (cmd == 2)) {
    if (!is_thief(ch)) {
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      return TRUE;
    }
  } else if ((ch->in_room == 3021) && (cmd == 2)) {
    if (!is_fighter(ch)) {
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      return TRUE;
    }

  }

  return FALSE;

}




int Inquisitor(struct char_data *ch, int cmd, char *arg)
{
  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (ch->specials.fighting) {
    return(Fighter(ch, cmd, arg));
  }

  switch(ch->act_ptr) {
  case INQ_SHOUT:
    do_shout(ch, "NOONE expects the Spanish Inquisition!", 0);
    ch->act_ptr = 0;
    break;
  default:
    break;
  }
  return(TRUE);
}

int puff(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *i, *tmp_ch;
  char buf[80];


  if (cmd)
    return(0);

  switch (number(0, 250))
    {
    case 0: do_say(ch, "Are you my mother?", 0); return(1);
    case 1: do_say(ch, "How'd all those fish get up here?", 0); return(1);
    case 2: do_say(ch, "I'm a very female dragon.", 0); return(1);
    case 3: do_shout(ch, "SAVE, The game is going to crash!", 0); return(1);
    case 4:
    case 5:
    case 6:
    case 7:
#if 0
      {
	for (i = character_list; i; i = i->next) {
	  if (!IS_NPC(i)) {
	    if (number(0,5)==0) {
	      if (!strcmp(GET_NAME(i),"Multrix")) {
		do_shout(ch,"DrBones, come ravish me now!",0);
	      } else if (!strcmp(GET_NAME(i), "ory")) {
		do_shout(ch,"I'm Puff the PMS dragon!",0);
	      } else if (!strcmp(GET_NAME(i), "eth")) {
		do_shout(ch, "Deth!  I need reimbursement!",0);
	      } else if (GET_SEX(i)==SEX_MALE) {
		sprintf(buf,"%s is my hero!",GET_NAME(i));
		do_say(ch,buf,0);
	      } else {
		sprintf(buf,"I'm much prettier than %s, don't you think?",GET_NAME(i));
		do_shout(ch,buf,0);
	      }
	    }
	  }
	}
      }
      return(1);
#endif
    case 8:
      do_say(ch, "Deth is my hero!", 0);
      return(1);
    case 9:
      do_say(ch, "So, wanna neck?", 0);
      return(1);
    case 10:
      {
	tmp_ch = (struct char_data *)FindAnyVictim(ch);
	if (!IS_NPC(ch)) {
	  sprintf(buf, "Party on, %s", GET_NAME(tmp_ch));
	  do_say(ch, buf, 0);
	  return(1);
	} else {
	  return(0);
	}
      }
    case 11: do_say(ch, "Haven't I seen you here before?", 0); return(1);
    case 12: do_say(ch, "Bad news.  Termites.", 0); return(1);
    case 13:
      for (i = character_list; i; i = i->next) {
	if (!IS_NPC(i)) {
	  if (number(0,15)==0) {
	    sprintf(buf, "%s shout Puff is Awesome!",GET_NAME(i));
	    do_force(ch, buf, 0);
	    do_restore(ch, GET_NAME(i), 0);
	    return(TRUE);
	  }
	}
      }
      return(1);
    case 14: do_say(ch, "Hasta La Vista, Baby.", 0); return(1);
    case 15: do_say(ch, "I've got a headache This big!", 0); return(1);
    case 16: do_emote(ch, "nudges you.", 0); return(1);
    case 17: do_emote(ch, "winks at you.", 0); return(1);
    case 18: do_say(ch, "This mud is too weird!", 0); return(1);
    case 19:
      do_say(ch, "If the Mayor is in a room alone, ", 0);
      do_say(ch, "Does he say 'Good morning citizens.'?",0);
      return(0);
    case 20:
      for (i = character_list; i; i = i->next) {
	if (!IS_NPC(i)) {
	  if (number(0,15)==0) {
	    sprintf(buf, "Top of the morning to you %s!", GET_NAME(i));
	    do_shout(ch, buf, 0);
	    return(TRUE);
	  }
	}
      }
      break;
    case 21:
      for (i = real_roomp(ch->in_room)->people; i; i= i->next_in_room) {
	if (!IS_NPC(i)) {
	  if (number(0,3)==0) {
	    sprintf(buf, "Pardon me, %s, but are those bugle boy jeans you are wearing?", GET_NAME(i));
	    do_shout(ch, buf, 0);
	    return(TRUE);
	  }
	}
      }
      break;
    case 22:
      for (i = real_roomp(ch->in_room)->people; i; i= i->next_in_room) {
	if (!IS_NPC(i)) {
	  if (number(0,3)==0) {
	    sprintf(buf, "Pardon me, %s, would you have any Grey Poupon?", GET_NAME(i));
	    do_shout(ch, buf, 0);
	    return(TRUE);
	  }
	}
      }
      break;
    case 23:
      if (number(0,20)==0) {
	do_shout(ch, "Level!!!!!!", 0);
      }
      break;
    case 24: do_say(ch, "MTV... everyone else is lying scum.", 0); return(TRUE);
    case 25: do_say(ch, "Bats = Bugs!", 0); return(TRUE);
    case 26:
      if (number(0,10)==0)
      do_shout(ch, "Don't touch that you fool!  Thats the history eraser button!", 0);
      break;
    case 27: do_say(ch, "RESOLVED:  The future's so bright, I gotta wear shades!", 0); return(TRUE);
    case 28: do_shout(ch, "SAVE!  They are adding too many classes!", 0); return(TRUE);
    case 29: do_say(ch, "HEY!  KOOLAID!!!", 0); return(TRUE);
    case 30: do_say(ch, "I'm fully functional, you know.", 0); return(TRUE);
    case 31: do_say(ch, "Become a demon, and live forever.", 0); return(TRUE);
    case 32:
      if (number(0,10)==0) {
	do_shout(ch, "Think you're tough? Try the challange of the gods!", 0);
	return(TRUE);
      }
      break;

    case 33:
      if (number(0,10)==0) {
	do_shout(ch, "LEVEL 500!! I made it YAY!!", 0);
	return(TRUE);
      }
      break;
    case 34:
      if (number(0,5)==0) {
	for (i = character_list; i; i=i->next) {
	  if (IS_MOB(i))
	    if (real_mobp(i->virtual)->func == Inquisitor) {
	       i->act_ptr = INQ_SHOUT;
	       do_shout(ch, "I wasn't expecting the Spanish Inquisition!", 0);
	       return(TRUE);
	     }
	}
	return(TRUE);
      }
      break;

    case 35:
      do_say(ch, "Are you crazy, is that your problem?", 0);
      return(TRUE);

    case 36:
      for (i = real_roomp(ch->in_room)->people; i; i=i->next_in_room) {
	if (!IS_NPC(i)) {
	  if (number(0,3)==0) {
	    sprintf(buf, "%s, what's your damage?",GET_NAME(i));
	    do_say(ch, buf, 0);
	    return(TRUE);
	  }
	}
      }
      break;
    case 37:
      do_say(ch, "This is your brain.", 0);
      do_say(ch, "This is drugs.", 0);
      do_say(ch, "This is your brain on drugs.", 0);
      do_say(ch, "Any questions?", 0);
      return(TRUE);
    case 38:
      for (i = character_list; i; i=i->next) {
	if (!IS_NPC(i)) {
	  if (number(0,20) == 0) {
	    if (i->in_room != NOWHERE) {
	      sprintf(buf, "%s save", GET_NAME(i));
	      do_force(ch, buf, 0);
	      return(TRUE);
	    }
	  }
	}
      }
      return(TRUE);
    case 39: do_shout(ch, "I'm Puff the Fractal Dragon, who the hell are you?", 0); return(TRUE);
    case 40: do_say(ch, "Roxanne.. You don't have to put on the red light.", 0); return(TRUE);
    case 41:
      if (number(0,10)==0) {
	do_shout(ch, "VMS sucks!!!!!", 0);
	return(TRUE);
      }
      break;
    case 42:
      if (number(0,10)==0) {
	do_shout(ch, "SPOON!", 0);
	return(TRUE);
      }
      break;
    case 43: do_say(ch, "Pardon me boys, is this the road to Great Cthulhu?", 0); return(TRUE);
    case 44: do_say(ch, "May the Force be with you... Always.", 0); return(TRUE);
    case 45: do_say(ch, "Eddies in the space time continuum.", 0); return(TRUE);
    case 46: do_say(ch, "Quick!  Reverse the polarity of the neutron flow!", 0); return(TRUE);
    case 47:
      if (number(0,10) == 0) {
	do_shout(ch, "I know what you want! You coveteth my ice-cream bar!", 0);
	return(TRUE);
      }
      break;
    case 48: do_say(ch, "Would you like to talk to my log?", 0); return(TRUE);
    case 49: do_say(ch, "Do you have any more of that Plutonium Nyborg!", 0); return(TRUE);
    case 50: do_say(ch, "I feel very strongly about my pots and pans.", 0); return(TRUE);

    case 51: do_shout (ch, "Hey, anyone have a kris knife for sale?", 0); return (TRUE);
    case 52: do_shout (ch, "Help! Guards! My evil twin is after me!", 0); return (TRUE);
    case 53: do_shout (ch, "Hey lets change worlds! reboot!", 0); return (TRUE);
    default:
      return(0);
    }
    return(0);
}

int regenerator( struct char_data *ch, int cmd, char *arg)
{

  if (cmd) return(FALSE);

  if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
    GET_HIT(ch) += 9;
    GET_HIT(ch) = MIN(GET_HIT(ch), GET_MAX_HIT(ch));

    act("$n regenerates.", TRUE, ch, 0, 0, TO_ROOM);
    return(TRUE);
  }
  return(FALSE);
}

int replicant( struct char_data *ch, int cmd, char *arg)
{
  struct char_data *mob;

  if (cmd) return FALSE;

  if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
    act("Drops of $n's blood hit the ground, and spring up into another one!",
	TRUE, ch, 0, 0, TO_ROOM);
    mob = get_mob(ch->virtual);
    char_to_room(mob, ch->in_room);
    act("Two undamaged opponents face you now.", TRUE, ch, 0, 0, TO_ROOM);
    GET_HIT(ch) = GET_MAX_HIT(ch);
  }

   return FALSE;

}

#define TYT_NONE 0
#define TYT_CIT  1
#define TYT_WHAT 2
#define TYT_TELL 3
#define TYT_HIT  4

int Tytan(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (ch->specials.fighting) {
    return(magic_user(ch, cmd, arg));
  } else {
    switch(ch->act_ptr) {
    case TYT_NONE:
      if ((vict = FindVictim(ch))) {
	ch->act_ptr = TYT_CIT;
	SetHunting(ch, vict);
      }
      break;
    case TYT_CIT:
      if (ch->specials.hunting) {
	if (ch->in_room == ch->specials.hunting->in_room) {
          do_say(ch, "Where is the Citadel?", 0);
	  ch->act_ptr = TYT_WHAT;
	}
      } else {
	ch->act_ptr = TYT_NONE;
      }
      break;
    case TYT_WHAT:
      if (ch->specials.hunting) {
	if (ch->in_room == ch->specials.hunting->in_room) {
          do_say(ch, "What must we do?", 0);
	  ch->act_ptr = TYT_TELL;
	}
      } else {
	ch->act_ptr = TYT_NONE;
      }
      break;
    case TYT_TELL:
      if (ch->specials.hunting) {
	if (ch->in_room == ch->specials.hunting->in_room) {
          do_say(ch, "Tell Us!  Command Us!", 0);
	  ch->act_ptr = TYT_HIT;
	}
      } else {
	ch->act_ptr = TYT_NONE;
      }
      break;
    case TYT_HIT:
      if (ch->specials.hunting) {
	if (ch->in_room == ch->specials.hunting->in_room) {
	  if (!check_peaceful(ch, "")) {
	    hit(ch, ch->specials.hunting, TYPE_UNDEFINED);
	    ch->act_ptr = TYT_NONE;
	  } else {
	    ch->act_ptr = TYT_CIT;
	  }
	}
      } else {
	ch->act_ptr = TYT_NONE;
      }
      break;
    default:
      ch->act_ptr = TYT_NONE;
    }
  }
  return(TRUE);
}

int AbbarachDragon(struct char_data *ch, int cmd, char *arg)
{

  struct char_data *targ;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (!ch->specials.fighting) {
     targ = (struct char_data *)FindAnyVictim(ch);
     if (targ && !check_peaceful(ch, "")) {
        hit(ch, targ, TYPE_UNDEFINED);
        act("You have now payed the price of crossing.",
	 TRUE, ch, 0, 0, TO_ROOM);
        return(TRUE);
     }
   } else {
     return(BreathWeapon(ch, cmd, arg));
   }
  return(FALSE);
}


int fido(struct char_data *ch, int cmd, char *arg)
{

  struct obj_data *i, *temp, *next_obj;
  struct char_data *v, *next;
  char found = FALSE;

  if (cmd || !AWAKE(ch))
    return(FALSE);


  for (v = character_list; (v && (!found)); v = next) {
    next = v->next;
    if ((IS_NPC(v)) && (v->virtual == 100) &&
	(v->in_room == ch->in_room) && CAN_SEE(ch, v)) {
      if (v->specials.fighting)
	stop_fighting(v);
      make_corpse(v);
      extract_char(v);
      found = TRUE;
    }
  }


  for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
    if (!i->held_for && GET_ITEM_TYPE(i)==ITEM_CONTAINER && i->obj_flags.value[3]) {
      act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
      for(temp = i->contains; temp; temp=next_obj)	{
	next_obj = temp->next_content;
	obj_from_obj(temp);
	obj_to_room(temp,ch->in_room);
      }
      extract_obj(i);
      return(TRUE);
    }
  }
  return(FALSE);
}



int janitor(struct char_data *ch, int cmd, char *arg)
{
  struct obj_data *i;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
    if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE) &&
	((i->obj_flags.type_flag == ITEM_DRINKCON) ||
	 (i->obj_flags.cost <= 10))) {
      act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);

      obj_from_room(i);
      obj_to_char(i, ch);
      return(TRUE);
    }
  }
  return(FALSE);
}

int tormentor(struct char_data *ch, int cmd, char *arg)
{

  if (!cmd) return(FALSE);

  if (IS_NPC(ch)) return(FALSE);

  if (IS_IMMORTAL(ch)) return(FALSE);

  return(TRUE);

}

int Fighter(struct char_data *ch, int cmd, char *arg)
{
  if (cmd) return(FALSE);
  if (cmd || !AWAKE(ch))
    return(FALSE);


  if (ch->specials.fighting) {
    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
    } else {
      StandUp(ch);
    }
    return(TRUE);
  }
  return(FALSE);
}

int RustMonster(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;
  struct obj_data *t_item;
  int t_pos;

  if (cmd || !AWAKE(ch))
    return(FALSE);

/*
**   find a victim
*/
  if (ch->specials.fighting) {
    vict = ch->specials.fighting;
  } else {
    vict = FindVictim(ch);
    if (!vict) {
      return(FALSE);
    }
  }

/*
**   choose an item of armor or a weapon that is metal
**  since metal isn't defined, we'll just use armor and weapons
*/

   /*
   **  choose a weapon first, then if no weapon, choose a shield,
   **  if no shield, choose breast plate, then leg plate, sleeves,
   **  helm
   */

  if (vict->equipment[WIELD]) {
    t_item = vict->equipment[WIELD];
    t_pos = WIELD;
  } else if (vict->equipment[WEAR_SHIELD]) {
    t_item = vict->equipment[WEAR_SHIELD];
    t_pos = WEAR_SHIELD;
  } else if (vict->equipment[WEAR_BODY]) {
    t_item = vict->equipment[WEAR_BODY];
    t_pos = WEAR_BODY;
  } else if (vict->equipment[WEAR_LEGS]) {
    t_item = vict->equipment[WEAR_LEGS];
    t_pos = WEAR_LEGS;
  } else if (vict->equipment[WEAR_ARMS]) {
    t_item = vict->equipment[WEAR_ARMS];
    t_pos = WEAR_ARMS;
  } else if (vict->equipment[WEAR_HEAD]) {
    t_item = vict->equipment[WEAR_HEAD];
    t_pos = WEAR_HEAD;
  } else {
    return(FALSE);
  }

/*
**  item makes save (or not).
*/
    if (DamageOneItem(vict, ACID_DAMAGE, t_item)) {
      t_item = unequip_char(vict, t_pos);
      if (t_item) {
/*
**  if it doesn't make save, falls into a pile of scraps
*/
	MakeScrap(vict, t_item);
      }
    }

  return(FALSE);

}

int temple_labrynth_liar(struct char_data *ch, int cmd, char *arg)
{
  /*void do_say(struct char_data *ch, char *argument, int cmd);*/

  if (cmd || !AWAKE(ch))
    return(0);

  switch (number(0, 15)) {
    case 0:
      do_say(ch, "I'd go west if I were you.", 0);
      return(1);
    case 1:
      do_say(ch, "I heard that Addiction is a cute babe.", 0);
      return(1);
    case 2:
      do_say(ch, "Going east will avoid the beast!", 0);
      return(1);
    case 4:
      do_say(ch, "North is the way to go.", 0);
      return(1);
    case 6:
      do_say(ch, "Dont dilly dally go south.", 0);
      return(1);
    case 8:
      do_say(ch, "Great treasure lies ahead", 0);
      return(1);
    case 10:
      do_say(ch, "I wouldn't kill the sentry if I were more than level 9. No way!", 0);
      return(1);
    case 12:
      do_say(ch, "I am a very clever liar.", 0);
      return(1);
    case 14:
      do_say(ch, "Loki is a really great guy!", 0);
      return(1);
    default:
      do_say(ch, "Then again I could be wrong!", 0);
      return(1);
    }
}

int temple_labrynth_sentry(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *tch;
  int counter;

  if(cmd || !AWAKE(ch)) return FALSE;

  if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;

  if(!ch->specials.fighting) return FALSE;

  /* Find a dude to do very evil things upon ! */

  for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if( GetMaxLevel(tch)>10 && CAN_SEE(ch, tch)) {
      act("The sentry snaps out of his trance and ...", 1, ch, 0, 0, TO_ROOM);
	do_say(ch, "You will die for your insolence, pig-dog!", 0);
	for ( counter = 0 ; counter < 4 ; counter++ )
	  if ( GET_POS(tch) > POSITION_SITTING) {
	    cast_fireball(15, ch, "", SPELL_TYPE_SPELL, tch, 0,GetMaxLevel(ch));
	  } else {
	    return TRUE;
	  }
	return TRUE;
      } else {
	act("The sentry looks concerned and continues to push you away",
	    1, ch, 0, 0, TO_ROOM);
	do_say(ch, "Leave me alone. My vows do not permit me to kill you!", 0);
      }
  }
  return TRUE;
}

#define WW_LOOSE 0
#define WW_FOLLOW 1

int Whirlwind (struct char_data *ch, int cmd, char *arg)
{
  struct char_data *tmp;
  static char *names[] = { "Loki", "Belgarath", 0};
  int i = 0;

  if (ch->in_room == -1) return(FALSE);

  if (cmd == 0 && ch->act_ptr == WW_LOOSE)  {
    for (tmp = real_roomp(ch->in_room)->people; tmp ; tmp = tmp->next_in_room) {
      while (names[i])  {
	if ( !strcmp(GET_NAME(tmp), names[i] ) && ch->act_ptr == WW_LOOSE)  {
	  /* start following */
	  if (circle_follow(ch, tmp))
	    return(FALSE);
	  if (ch->master)
	    stop_follower(ch);
	  add_follower(ch, tmp);
	  ch->act_ptr = WW_FOLLOW;
	}
	i++;
      }
    }
    if (ch->act_ptr == WW_LOOSE && !cmd )  {
      act("The $n suddenly dissispates into nothingness.",0,ch,0,0,TO_ROOM);
      extract_char(ch);
    }
  }
  return(FALSE);
}

#define NN_LOOSE  0
#define NN_FOLLOW 1
#define NN_STOP   2

int NudgeNudge(struct char_data *ch, int cmd, char *arg)
{

  struct char_data *vict;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    return(FALSE);
  }

  switch(ch->act_ptr) {
  case NN_LOOSE:
     /*
     ** find a victim
     */
    vict = FindVictim(ch);
    if (!vict)
      return(FALSE);
    /* start following */
    if (circle_follow(ch, vict)) {
      return(FALSE);
    }
    if (ch->master)
      stop_follower(ch);
    add_follower(ch, vict);
    ch->act_ptr = NN_FOLLOW;
    do_say (ch, "Good Evenin' Squire!" , 0 );
    act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
    act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
    break;
  case NN_FOLLOW:
    switch(number(0,20)) {
    case 0:
      do_say  (ch, "Is your wife a goer?  Know what I mean, eh?", 0 );
      act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      break;
    case 1:
      act ("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act ("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      do_say  (ch, "Say no more!  Say no MORE!", 0);
      break;
    case 2:
      do_say  (ch, "You been around, eh?", 0);
      do_say  (ch, "...I mean you've ..... done it, eh?", 0);
      act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      break;
    case 3:
      do_say  (ch, "A nod's as good as a wink to a blind bat, eh?", 0);
      act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      break;
    case 4:
      do_say  (ch, "You're WICKED, eh!  WICKED!", 0);
      act ("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
      act ("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act ("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      break;
    case 5:
      do_say  (ch, "Wink. Wink.", 0);
      break;
    case 6:
      do_say  (ch, "Nudge. Nudge.", 0);
      break;
    case 7:
    case 8:
      ch->act_ptr = NN_STOP;
      break;
    default:
      break;
    }
    break;
  case NN_STOP:
    /*
    **  Stop following
    */
    do_say(ch, "Evening, Squire", 0);
    stop_follower(ch);
    ch->act_ptr = NN_LOOSE;
    break;
  default:
    ch->act_ptr = NN_LOOSE;
    break;
  }
  return(TRUE);
}

int AGGRESSIVE(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *i, *next;

  if (cmd || !AWAKE(ch)) return(FALSE);

  if (ch->in_room > -1) {
    for (i = real_roomp(ch->in_room)->people; i; i = next) {
      next = i->next_in_room;
      if (i->virtual != ch->virtual) {
	if (!IS_IMMORTAL(i)) {
	  hit(ch, i, TYPE_UNDEFINED);
          return(TRUE);
	}
      }
    }
  }
  return(FALSE);
}

int cityguard(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *tch;
  int lev=0;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
    } else {
      StandUp(ch);
    }

    if (number(0,20) == 0) {
      do_shout(ch, "To me, my fellows! I am in need of thy aid!", 0);
    } else {
      act("$n shouts 'To me, my fellows! I need thy aid!'",
	  TRUE, ch, 0, 0, TO_ROOM);
    }

    if (ch->specials.fighting)
      CallForGuard(ch, ch->specials.fighting, lev);

    return(TRUE);
  }

  for(tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if(CAN_SEE(ch, tch) && IS_NPC(tch)){
      if(IS_SET(tch->specials.act, ACT_AGGRESSIVE) &&
          !IS_SET(tch->specials.act, ACT_WIMPY) &&
          !IS_POLICE(tch) && !IS_CITIZEN(tch) && (ch != tch)){
        act("$n screams 'Die, foul creature!'", FALSE, ch, 0, 0, TO_ROOM);
        hit(ch, tch, TYPE_UNDEFINED);
        return(TRUE);
      }
      else if(IS_SET(tch->specials.act, ACT_CITIZEN) &&
          tch->specials.fighting){
        if(CAN_SEE(ch, tch->specials.fighting)){
          if(IS_AFFECTED(tch, AFF_CHARM)){
            act("$n screams 'Die, foul snake-charmer!'", FALSE, ch, 0, 0,
              TO_ROOM);
            hit(ch, tch->master, TYPE_UNDEFINED);
	    return(TRUE);
          }
          else{
            act("$n screams 'Unhand that citizen!'", FALSE, ch, 0, 0, TO_ROOM);
            hit(ch, tch->specials.fighting, TYPE_UNDEFINED);
            return(TRUE);
          }
        }
      }
      else if(IS_POLICE(tch) && tch->specials.fighting){
	if(CAN_SEE(ch, tch->specials.fighting)){
	  act("$n screams 'Die, criminal!'", FALSE, ch, 0, 0, TO_ROOM);
	  hit(ch, tch->specials.fighting, TYPE_UNDEFINED);
	  return(TRUE);
        }
      }
    }
  }

/*
  max_evil = 1000;
  evil = 0;

  if (check_peaceful(ch, ""))
    return FALSE;

  for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if ((IS_NPC(tch)) && (IsUndead(tch)) && CAN_SEE(ch, tch)) {
      max_evil = -1000;
      evil = tch;
      act("$n screams 'EVIL!!!  BANZAI!  SPOOON!'",
	  FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, evil, TYPE_UNDEFINED);
      return(TRUE);

    }
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
	  (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
	max_evil = GET_ALIGNMENT(tch);
	evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
    act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!! SPOON!'",
	FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return(TRUE);
  }
*/
  return(FALSE);
}

#define ONE_RING 1105

int Ringwraith( struct char_data *ch, int cmd, char *arg)
{
  static char buf[256];
  struct char_data *victim;
  static slong	howmanyrings= -1;
  struct obj_data	*ring;
  struct wraith_hunt {
    int	ringnumber;
    int	chances;
  } *wh;
  int	rnum,dir;

  if (!AWAKE(ch) || !IS_NPC(ch) || cmd) {
    return(FALSE);
  }

  if (ch->specials.fighting) {
    if (GET_POS(ch) == POSITION_FIGHTING) {
     FighterMove(ch);
    } else {
      StandUp(ch);
    }
    return(FALSE);
  }

  if (howmanyrings==-1) { /* how many one rings are in the game? */
    howmanyrings = 1;
    get_obj_vis_world(ch, "999.one ring.", &howmanyrings);
  }

  if (ch->user_data==0) { /* does our ringwraith have his state info? */
    ch->user_data = (void *)malloc(sizeof(struct wraith_hunt));
    wh=(void *)ch->user_data;
    wh->ringnumber=0;
  } else
    wh = (void*)ch->user_data;

  if (!wh->ringnumber) { /* is he currently tracking a ring */
    wh->chances=0;
    wh->ringnumber = number(1,howmanyrings++);
  }

  sprintf(buf, "%d.one ring.", (int)wh->ringnumber); /* where is this ring? */
  if (NULL== (ring=get_obj_vis_world(ch, buf, NULL))) {
    /* there aren't as many one rings in the game as we thought */
    howmanyrings = 1;
    get_obj_vis_world(ch, "999.one ring.", &howmanyrings);
    wh->ringnumber = 0;
    return FALSE;
  }

  rnum = room_of_object(ring);

  if (rnum != ch->in_room) {
    dir = find_path(ch->in_room, is_target_room_p, (void *)&rnum, 100);
    if (dir<0) { /* we can't find the ring */
      wh->ringnumber = 0;
      return FALSE;
    }
    go_direction(ch, dir);
    return TRUE;
  }

  /* the ring is in the same room! */

  if ((victim = char_holding(ring))) {
    if (victim==ch) {
      obj_from_char(ring);
      extract_obj(ring);
      wh->ringnumber=0;
      act("$n grimaces happily.", FALSE, ch, NULL, victim, TO_ROOM);
    } else {
      switch (wh->chances) {
      case 0:
	do_wake(ch, GET_NAME(victim), 0);
	act("$n says '$N, give me The Ring'.", FALSE, ch, NULL, victim,
	    TO_ROOM);
	wh->chances++;
	return(TRUE);
	break;
      case 1:
	if (IS_NPC(victim)) {
	  act("$N quickly surrenders The Ring to $n.", FALSE, ch, NULL, victim,
	      TO_ROOM);
	  obj_from_char(ring);
	  obj_to_char(ring, ch);
	} else {
	  act("$n says '$N, give me The Ring *NOW*'.", FALSE, ch, NULL, victim,
	      TO_ROOM);
	  wh->chances++;
	}
	return(TRUE);
	break;
      default:
	if (check_peaceful(ch, "Damn, he's in a safe spot.")) {
	  act("$n says 'You can't stay here forever, $N'.", FALSE, ch,
	      NULL, victim, TO_ROOM);
	} else {
	act("$n says 'I guess I'll just have to get it myself'.", FALSE, ch,
	    NULL, victim, TO_ROOM);
	hit(ch, victim, TYPE_UNDEFINED);
	}
	break;
      }
    }
  } else if (ring->in_obj) {
    /* the ring is in an object */
    obj_from_obj(ring);
    obj_to_char(ring, ch);
    act("$n gets the One Ring.", FALSE, ch, NULL, victim, TO_ROOM);
  } else if (ring->in_room != NOWHERE) {
    obj_from_room(ring);
    obj_to_char(ring, ch);
    act("$n gets the Ring.", FALSE, ch, NULL, 0, TO_ROOM);
  } else {
    log("a One Ring was completely disconnected!?");
    wh->ringnumber = 0;
  }
  return TRUE;


}

int WarrenGuard(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *tch, *good;
  int max_good;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
    } else {
      StandUp(ch);
    }

    return(TRUE);
  }

  max_good = -1000;
  good = 0;

  if (check_peaceful(ch, ""))
    return FALSE;

  for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) > max_good) &&
	  (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
	max_good = GET_ALIGNMENT(tch);
	good = tch;
      }
    }
  }

  if (good && (GET_ALIGNMENT(good->specials.fighting) <= 0)) {
    act("$n screams 'DEATH TO GOODY-GOODIES!!!!'",
	FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, good, TYPE_UNDEFINED);
    return(TRUE);
  }

  return(FALSE);
}



/** Perform breadth first search on rooms from start (in_room) **/
/** until end (tgt_room) is reached. Then return the correct   **/
/** direction to take from start to reach end.                 **/

/* thoth@manatee.cis.ufl.edu
   if dvar<0 then search THROUGH closed but not locked doors,
   for mobiles that know how to open doors.
 */
/* dvar<0 now = search through closed and locked doors, for mobiles that can
   also unlock them - SLB */
#define IS_DIR    (real_roomp(q_head->room_nr)->dir_option[i])
#define GO_OK  (!IS_SET(IS_DIR->exit_info,EX_CLOSED)\
		 && (IS_DIR->to_room != NOWHERE))
#define GO_OK_SMARTER (IS_DIR->to_room != NOWHERE)

static void donothing()
{
  return;
}

int find_path(int in_room, int (*predicate)(), void *c_data, int depth)
{
  struct room_q *tmp_q, *q_head, *q_tail;
/*  struct nodes x_room[MAX_ROOMS];*/
  int i, tmp_room, count=0, thru_doors;
  static struct hash_header	x_room;
  struct room_data	*herep, *therep;
  struct room_direction_data	*exitp;

  /* If start = destination we are done */
  if ((predicate)(in_room, c_data))
    return -1;

  if (depth<0) {
    thru_doors = TRUE;
    depth = - depth;
  } else {
    thru_doors = FALSE;
  }
  if (x_room.buckets) { /* junk left over from a previous track */
    destroy_hash_table(&x_room, donothing);
  }
  init_hash_table(&x_room, sizeof(int), 2048);
  hash_enter(&x_room, in_room, (void*)-1);

  /* initialize queue */
  q_head = (struct room_q *) malloc(sizeof(struct room_q));
  q_tail = q_head;
  q_tail->room_nr = in_room;
  q_tail->next_q = 0;

  while(q_head) {
    herep = real_roomp(q_head->room_nr);
    /* for each room test all directions */
    for(i = 0; i <= 5; i++) {
      exitp = herep->dir_option[i];
      if (exit_ok(exitp, &therep) && (thru_doors ? GO_OK_SMARTER : GO_OK)) {
        /* next room */
	tmp_room = herep->dir_option[i]->to_room;
	if(!((predicate)(tmp_room, c_data))) {
          /* shall we add room to queue ? */
          /* count determines total breadth and depth */
	  if(!hash_find(&x_room,tmp_room) && (count < depth)
	      && !IS_SET(RM_FLAGS(tmp_room),DEATH)) {
            count++;
            /* mark room as visted and put on queue */
            tmp_q = (struct room_q *) malloc(sizeof(struct room_q));
            tmp_q->room_nr = tmp_room;
            tmp_q->next_q = 0;
            q_tail->next_q = tmp_q;
            q_tail = tmp_q;
            /* ancestor for first layer is the direction */
	    hash_enter(&x_room, tmp_room,
              ((int)hash_find(&x_room,q_head->room_nr) == -1) ?
              (void*)(i+1) : hash_find(&x_room,q_head->room_nr));
          }
	}
        else {
          /* have reached our goal so free queue */
          tmp_room = q_head->room_nr;
          for(;q_head;q_head = tmp_q) {
            tmp_q = q_head->next_q;
            FREE(q_head);
          }
          /* return direction if first layer */
	  if ((int)hash_find(&x_room,tmp_room)==-1)
            return(i);
	  else	  /* else return the ancestor */
	    return( -1+(int)hash_find(&x_room,tmp_room));
        }
      }
    }
    /* free queue head and point to next entry */
    tmp_q = q_head->next_q;
    FREE(q_head);
    q_head = tmp_q;
  }
  /* couldn't find path */
  return(-1);
}

int choose_exit(int in_room, int tgt_room, int depth)
{
  return find_path(in_room, is_target_room_p, (void*)&tgt_room, depth);
}

int go_direction(struct char_data *ch, int dir)
{
  struct obj_data *key;

  if (ch->specials.fighting)
    return(0);

  if (!IS_SET(EXIT(ch,dir)->exit_info, EX_CLOSED)) {
    do_move(ch, "", dir+1);
    return 1;
  } else if ( IsHumanoid(ch) && !IS_SET(EXIT(ch,dir)->exit_info, EX_LOCKED) ) {
    open_door(ch, dir);
    return 0;
  }
  else if (IsHumanoid(ch) && (key = has_readied(ch, EXIT(ch, dir)->key))) {
    phys_unlock_door(ch, key, dir);
  }
  return(0);
}

/***********************************************************************

			   CHESSBOARD PROCS

 ***********************************************************************/

#define SISYPHUS_MAX_LEVEL 9

/* This is the highest level of PC that can enter.  The highest level
   monster currently in the section is 14th.  It should require a fairly
   large party to sweep the section. */

int sisyphus(struct char_data *ch, int cmd, char *arg)
{

  if (cmd) {
    if (cmd<=6 && cmd>=1 && !IS_NPC(ch)) {
      act("Sisyphus looks at you.",TRUE, ch, 0, 0, TO_CHAR);
      act("Sisyphus looks at $n",TRUE, ch, 0, 0, TO_ROOM);

      if ((ch->in_room == Ivory_Gate) && (cmd == 4)) {
	if ((SISYPHUS_MAX_LEVEL < GetMaxLevel(ch)) &&
	    (GetMaxLevel(ch) < LOW_IMMORTAL))	    {
	  act("Sisyphus tells you 'First you'll have to get past me.'",
	      TRUE, ch, 0, 0, TO_CHAR);
	  act("Sisyphus grins evilly.", TRUE, ch, 0, 0, TO_CHAR);
	  act("Sisyphus grins evilly.", TRUE, ch, 0, 0, TO_ROOM);
	  return(TRUE);
	}
      }
      return(FALSE);
    } /* cmd 1 - 6 */
    return(FALSE);
  } else {
    if (ch->specials.fighting) {
      if ((GET_POS(ch) < POSITION_FIGHTING) &&
	  (GET_POS(ch) > POSITION_STUNNED)){
	StandUp(ch);
      } else {
	FighterMove(ch);
      }
      return(FALSE);
    }
  }
  return(FALSE);
} /* end sisyphus */

int abyss_guard(struct char_data *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  struct char_data *guard;

  if (cmd) {
    if (cmd<=6 && cmd>=1 && !IS_NPC(ch)) {
      if ((ch->in_room == 25003) && (cmd == 6)) {
	guard=FindMobInRoomWithFunction(ch->in_room,abyss_guard);
	if ((GetMaxLevel(ch)>ABYSS_MAX_LEVEL || GetMaxLevel(ch)<ABYSS_MIN_LEVEL)
			  && !IS_IMMORTAL(ch)) {
	  sprintf(buf," %s First you'll have to get past me.",GET_NAME(ch));
	  do_tell(guard,buf,0);
	  act("$n grins evilly.\n",TRUE, guard,0, 0, TO_ROOM);
	  return(TRUE);
	}
      }
      return(FALSE);
    } /* cmd 1 - 6 */
    return(FALSE);
  } else {
    if (ch->specials.fighting) {
      if ((GET_POS(ch) < POSITION_FIGHTING) &&(GET_POS(ch) > POSITION_STUNNED)){
	StandUp(ch);
      } else {
	FighterMove(ch);
      }
      return(FALSE);
    }
  }
  return(FALSE);
} /* end sisyphus */


int jabberwocky(struct char_data *ch, int cmd, char *arg)
{
  if (cmd) return(FALSE);

  if (ch->specials.fighting) {
    if ((GET_POS(ch) < POSITION_FIGHTING) &&
	(GET_POS(ch) > POSITION_STUNNED)){
      StandUp(ch);
      return(TRUE);
    } else {
      FighterMove(ch);
      return(TRUE);
    }
  }
  return(FALSE);
}

int flame(struct char_data *ch, int cmd, char *arg)
{
  return(jabberwocky(ch,cmd,arg));
}

int banana(struct char_data *ch, int cmd, char *arg)
{
  if (!cmd) return(FALSE);

  if ((cmd >= 1) && (cmd <= 6) &&
      (GET_POS(ch) == POSITION_STANDING) &&
      (!IS_NPC(ch))) {
    if (!saves_spell(ch, SAVING_PARA)) {
      act("$N tries to leave, but slips on a banana and falls.",
	  TRUE, ch, 0, ch, TO_NOTVICT);
      act("As you try to leave, you slip on a banana.",
	  TRUE, ch, 0, ch, TO_VICT);
      GET_POS(ch) = POSITION_SITTING;
      return(TRUE); /* stuck */
    }
    return(FALSE);	/* he got away */
  }
  return(FALSE);
}

int paramedics(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict, *most_hurt;

  if (!cmd) {
    if (ch->specials.fighting) {
      return(cleric(ch, 0, ""));
    } else {
      if (GET_POS(ch) == POSITION_STANDING) {

	/* Find a dude to do good things upon ! */

	most_hurt = real_roomp(ch->in_room)->people;
	for (vict = real_roomp(ch->in_room)->people; vict;
	     vict = vict->next_in_room ) {
	  if (((float)GET_HIT(vict)/(float)hit_limit(vict) <
	       (float)GET_HIT(most_hurt)/(float)hit_limit(most_hurt))
	      && (CAN_SEE(ch, vict)))
	    most_hurt = vict;
	}
	if (!most_hurt) return(FALSE); /* nobody here */

	if ((float)GET_HIT(most_hurt)/(float)hit_limit(most_hurt) >
	    0.66) {
	  if (number(0,5)==0) {
	    act("$n shrugs helplessly in unison.", 1, ch, 0, 0, TO_ROOM);
	  }
	  return TRUE;	/* not hurt enough */
	}

	if(number(0,4)==0) {
	  if (most_hurt != ch) {
	    act("$n looks at $N.", 1, ch, 0, most_hurt, TO_NOTVICT);
	    act("$n looks at you.", 1, ch, 0, most_hurt, TO_VICT);
	  }
	  act("$n utters the words 'judicandus dies' in unison.", 1, ch, 0, 0, TO_ROOM);
	  cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, most_hurt, 0,GetMaxLevel(ch));
	  return(TRUE);
	}
      } else {/* I'm asleep or sitting */
	return(FALSE);
      }
    }
  }
  return(FALSE);
}

int jugglernaut(struct char_data *ch, int cmd, char *arg)
{
  struct obj_data *tmp_obj;

  if (cmd) return(FALSE);

  if (GET_POS(ch) == POSITION_STANDING) {

    if (random()%3) return FALSE;

    /* juggle something */
    if (!ch->carrying) return FALSE;

    for (tmp_obj = ch->carrying; (tmp_obj) && (number(0, 10) != 0);
        tmp_obj = tmp_obj->next_content) ;

    if ((random()%6) && tmp_obj) {
      if (random()%2) {
	act("$n tosses $p high into the air and catches it.", TRUE, ch, tmp_obj, NULL, TO_ROOM);
      }  else {
	act("$n sends $p whirling.", TRUE, ch, tmp_obj, NULL, TO_ROOM);
      }
      return(TRUE);
    } else if(tmp_obj){
      act("$n tosses $p but fumbles it!", TRUE, ch, tmp_obj, NULL, TO_ROOM);
	if (tmp_obj) {
          obj_from_char(tmp_obj);
	  if (tmp_obj)
            obj_to_room(tmp_obj, ch->in_room);
	}
      return(TRUE);
    }
    return(FALSE);	/* don't move, I dropped something */
  } else  { /* I'm asleep or sitting */
    return FALSE;
  }
}

#if 0
static char *elf_comm[] = {
  "wake", "yawn",
  "stand", "say Well, back to work.", "get all",
  "eat bread", "wink",
  "w", "w", "s", "s", "s", "d", "open gate", "e",  /* home to gate*/
  "close gate",
  "e", "e", "e", "e", "n", "w", "n", /* gate to baker */
  "give all.bread baker", /* pretend to give a bread */
  "give all.pastry baker", /* pretend to give a pastry */
  "say That'll be 33 coins, please.",
  "echo The baker gives some coins to the Elf",
  "wave",
  "s", "e", "n", "n", "e", "drop all.bread", "drop all.pastry",
  "w", "s", "s", /* to main square */
  "s", "w", "w", "w", "w", /* back to gate */
  "pat sisyphus",
  "open gate", "w", "close gate", "u", "n", "n", "n", "e", "e", /* to home */
  "say Whew, I'm exhausted.", "rest", "$"};
#endif


int delivery_elf(struct char_data *ch, int cmd, char *arg)
{
#define ELF_INIT     0
#define ELF_RESTING  1
#define ELF_GETTING  2
#define ELF_DELIVERY 3
#define ELF_DUMP 4
#define ELF_RETURN_TOWER   5
#define ELF_RETURN_HOME    6

  if (cmd) return(FALSE);

  if (ch->specials.fighting)
    return FALSE;

  switch(ch->act_ptr) {

  case ELF_INIT:
    if (ch->in_room == 0) {
      /* he has been banished to the Void */
    } else if (ch->in_room != Elf_Home) {
      if (GET_POS(ch) == POSITION_SLEEPING) {
	do_wake(ch, "", 0);
	do_stand(ch, "", 0);
      }
      do_say(ch, "Woah! How did i get here!", 0);
      do_emote(ch, "waves his arm, and vanishes!", 0);
      char_from_room(ch);
      char_to_room(ch, Elf_Home);
      do_emote(ch, "arrives with a Bamf!", 0);
      do_emote(ch, "yawns", 0);
      do_sleep(ch, "", 0);
      ch->act_ptr = ELF_RESTING;
    } else {
      ch->act_ptr = ELF_RESTING;
    }
    return(FALSE);
    break;
  case ELF_RESTING:
    {
      if ((time_info.hours > 6) && (time_info.hours < 9)) {
	do_wake(ch, "", 0);
	do_stand(ch, "", 0);
	ch->act_ptr = ELF_GETTING;
      }
      return(FALSE);
    } break;

  case ELF_GETTING:
    {
      do_get(ch, "all.loaf", 0);
      do_get(ch, "all.biscuit", 0);
      ch->act_ptr = ELF_DELIVERY;
      return(FALSE);
    } break;
  case ELF_DELIVERY:
    {
      if (ch->in_room != Bakery) {
	int	dir;
	dir = choose_exit(ch->in_room, Bakery, -100);
	if (dir<0) {
	  ch->act_ptr = ELF_INIT;
	  return(FALSE);
	} else {
	  go_direction(ch, dir);
	}
      } else {
	do_give(ch, "6*biscuit baker", 0);
	do_give(ch, "6*loaf baker", 0);
	do_say(ch, "That'll be 33 coins, please.", 0);
	ch->act_ptr = ELF_DUMP;
      }
      return(FALSE);
    } break;
  case ELF_DUMP:
    {
      if (ch->in_room != Dump)   {
	int	dir;
	dir = choose_exit(ch->in_room, Dump, -100);
	if (dir<0) {
	  ch->act_ptr = ELF_INIT;
	  return(FALSE);
	} else {
	  go_direction(ch, dir);
	}
      } else {
	do_drop(ch, "10*biscuit", 0);
	do_drop(ch, "10*loaf", 0);
	ch->act_ptr = ELF_RETURN_TOWER;
      }
      return(FALSE);
    } break;
  case ELF_RETURN_TOWER:
    {
      if (ch->in_room != Ivory_Gate)   {
	int	dir;
	dir = choose_exit(ch->in_room, Ivory_Gate, -200);
	if (dir<0) {
	  ch->act_ptr = ELF_INIT;
	  return(FALSE);
	} else {
	  go_direction(ch, dir);
	}
      } else {
	ch->act_ptr = ELF_RETURN_HOME;
      }
      return(FALSE);
    }
    break;
  case ELF_RETURN_HOME:
    if (ch->in_room != Elf_Home)   {
      int	dir;
      dir = choose_exit(ch->in_room, Elf_Home, -200);
      if (dir<0) {
	ch->act_ptr = ELF_INIT;
	return(FALSE);
      } else {
	go_direction(ch, dir);
      }
    } else {
      if (time_info.hours > 21) {
	do_say(ch, "Done at last!", 0);
	do_sleep(ch, "", 0);
	ch->act_ptr = ELF_RESTING;
      } else {
	do_say(ch, "An elf's work is never done.", 0);
	ch->act_ptr = ELF_GETTING;
      }
    }
    return(FALSE);
    break;
  default:
    ch->act_ptr = ELF_INIT;
    return(FALSE);
  }
}



int delivery_beast(struct char_data *ch, int cmd, char *arg)
{
  struct obj_data *o;

  if (cmd) return(FALSE);

  if (time_info.hours == 6) {
    do_drop(ch, "all.loaf",0);
    do_drop(ch, "all.biscuit", 0);
  } else if (time_info.hours < 2) {
    if (number(0,1)) {
      o = get_obj(3012);
      obj_to_char(o, ch);
    } else {
      o = get_obj(3013);
      obj_to_char(o, ch);
    }
  } else {
    if (GET_POS(ch) > POSITION_SLEEPING) {
      do_sleep(ch, "", 0);
    }
  }
  return(TRUE);
}

int Keftab(struct char_data *ch, int cmd, char *arg)
{
  int found, targ_item;
  struct char_data *i;

  if (cmd) return(FALSE);

  if (!ch->specials.hunting) {
    /* find a victim */

    for (i = character_list; i; i = i->next) {
      if (!IS_NPC(ch)) {
	targ_item = SWORD_ANCIENTS;
	found = FALSE;
	while (!found) {
	  if ((HasObject(i, targ_item))&&(GetMaxLevel(i) < 30)) {
	    AddHated(ch, i);
	    SetHunting(ch, i);
	    return(TRUE);
	  } else {
	    targ_item++;
	    if (targ_item > SWORD_ANCIENTS+10)
	      found = TRUE;
	  }
	}
      }
    }
    return(FALSE);
  } else {
    /* check to make sure that the victim still has an item */
    found = FALSE;
    targ_item = SWORD_ANCIENTS;
    while (!found) {
      if (HasObject(ch->specials.hunting, targ_item)) {
        return(FALSE);
      } else {
	targ_item++;
	if (targ_item == SWORD_ANCIENTS+3)
	  found = FALSE;
      }
      ch->specials.hunting = 0;
      return(FALSE);
    }
  }
  return(FALSE);
}

int StormGiant(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;

  if (cmd) return(FALSE);

  if (ch->specials.fighting) {
    if ((GET_POS(ch) < POSITION_FIGHTING) &&
	(GET_POS(ch) > POSITION_STUNNED)){
      StandUp(ch);
    } else {
      if (number(0, 5)) {
	FighterMove(ch);
      } else {
	act("$n creates a lightning bolt", TRUE, ch, 0,0,TO_ROOM);
	vict = FindVictim(ch);
	if (!vict) return(FALSE);
       	cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0,GetMaxLevel(ch));
	return(TRUE);
      }
    }
  }
  return(FALSE);
}

int Manticore(struct char_data *ch, int cmd, char *arg)
{
  return(FALSE);
}

int Kraken(struct char_data *ch, int cmd, char *arg)
{
  return(FALSE);
}

int tarin_good(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch) || (GET_POS(ch) == POSITION_FIGHTING))
    return (FALSE);

  max_evil = -999;
  evil = 0;

  for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
   if (tch->specials.fighting) {
    if ((GET_ALIGNMENT(tch) < max_evil) && (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
       max_evil = GET_ALIGNMENT(tch);
       evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
    act("$n goes berserk screaming, 'ALARM!!!!'", FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return(TRUE);
  }

  return(FALSE);
}


int tarin(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *victim;
  char buf[256];
  int antal,i;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (!(ch->specials.fighting))
    return(0);
  else {
    switch(number(0,38)) {
    case 0:
    case 10:
    case 15:
      antal = number(2,3);
      sprintf(buf,"$n summons %d dark lords from a shadow dimension.",
	      antal);
      act(buf,FALSE,ch,0,0,TO_ROOM);
      for (i=0;i<antal;i++) {
	victim = get_mob(33002);;
	char_to_room(victim,ch->in_room);
      }
      return(1);
    case 20:
    case 21:
      antal = number(2,3);
      sprintf(buf,"$n magically creates %d red devils.",
	      antal);
      act(buf,FALSE,ch,0,0,TO_ROOM);
      for (i=0;i<antal;i++) {
	victim = get_mob(33010);
	char_to_room(victim,ch->in_room);
      }
      return(1);
    case 29:
    case 30:
      for (victim=real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room) {
        if (IS_AFFECTED(victim,AFF_SANCTUARY) && (!IS_NPC(victim))) {
        act("$n utters the words, instant sanctuary away",FALSE, ch, 0, 0, TO_ROOM);
        act("$n's white glow fades and flickers out.",FALSE ,victim,0,0,TO_ROOM);
        clear_bit(ch->specials.affected_by,AFF_SANCTUARY);
       }
     }
      return(1);
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
      for (victim=real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room) {
        if (IS_AFFECTED(victim,AFF_FIRESHIELD) && (!IS_NPC(victim))) {
        act("$n utters the words, eww yuk fireshield and waves his hand",FALSE, ch, 0, 0, TO_ROOM);
        act("$n's red glow super novas and flickers out.",FALSE ,victim,0,0,TO_ROOM);
        clear_bit(ch->specials.affected_by,AFF_FIRESHIELD);
       }
    }
      return(1);
    default:
      return(1);
    }
    return(1);
  }
  return(1);
}

int god(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *victim;
  char buf[256];
  int help,i;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (ch->specials.fighting) {
    if (IS_AFFECTED(ch,AFF_BLIND)) {
      act("$n utters the words, 'cure blindness'",FALSE, ch, 0, 0, TO_ROOM);
      act("$n's vision returns.",FALSE ,ch,0,0,TO_ROOM);
      clear_bit(ch->specials.affected_by,AFF_BLIND);
    }
    if (IS_AFFECTED(ch,AFF_POISON)) {
      act("$n utters the words, 'PoisonSucks'",FALSE,ch,0,0,TO_ROOM);
      clear_bit(ch->specials.affected_by,AFF_POISON);
    }

  }

  if (ch->specials.fighting) {
    switch(number(0,20)) {
    case 0:
    case 1:
      help = number(1,5);
      sprintf(buf,"$n summons %d dark lords from a shadow dimension.",help);
      act(buf,FALSE,ch,0,0,TO_ROOM);
      for (i=0;i<help;i++) {
	victim = get_mob(33002);
	char_to_room(victim,ch->in_room);
      }
      return(1);
    case 2:
    case 3:
      help = number(4,8);
      sprintf(buf,"$n magically creates %d red devils.",help);
      act(buf,FALSE,ch,0,0,TO_ROOM);
      for (i=0;i<help;i++) {
	victim = get_mob(33010);
	char_to_room(victim,ch->in_room);
      }
      return(1);
    case 10:
    case 11:
    case 12:
      act("$n breathes acid, but fails in the try.",FALSE ,ch,0,0,TO_ROOM);
      return(1);
    case 17:
    case 18:
      for (victim=real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room) {
        if (IS_AFFECTED(victim,AFF_SANCTUARY) && (!IS_NPC(victim))) {
        act("$n utters the words, instant sanctuary away",FALSE, ch, 0, 0, TO_ROOM);
        act("$n's white glow fades and flickers out.",FALSE ,victim,0,0,TO_ROOM);
        clear_bit(ch->specials.affected_by,AFF_SANCTUARY);
       }
     }
      return(1);
    case 19:
      for (victim=real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room) {
        if (IS_AFFECTED(victim,AFF_FIRESHIELD) && (!IS_NPC(victim))) {
        act("$n utters the words, eww yuk fireshield and waves his hand",FALSE, ch, 0, 0, TO_ROOM);
        act("$n's red glow super novas and flickers out.",FALSE ,victim,0,0,TO_ROOM);
        clear_bit(ch->specials.affected_by,AFF_FIRESHIELD);
       }
     }
      return(1);
    default:
      if(!IS_AFFECTED(ch,AFF_SANCTUARY)) {
	act("$n utters the word, 'pqeynszc'",FALSE,ch,0,0,TO_ROOM);
	act("$n is surrounded with a white aura.",FALSE,ch,0,0,TO_ROOM);
	set_bit(ch->specials.affected_by,AFF_SANCTUARY);
      }
      if(IS_AFFECTED(ch,AFF_BLIND)) {
	act("$n utters the words, 'cure blindness'",FALSE,ch,0,0,TO_ROOM);
	act("$n's eyes glows strange blue, then back to normal.",FALSE,ch,0,0,TO_ROOM);
	clear_bit(ch->specials.affected_by,AFF_BLIND);
      }
      return(TRUE);

    }
    return(1);
  }
  return(1);
}


#if 0
int andy_wilcox(struct char_data *ch, int cmd, char *arg)

{
  /* things you MUST change if you install this on another mud:
     THE_PUB, room number of the Pub where he will sell beer.
     sold_here, object numbers of the containers and the
     corresponding beer that they will contain.  If you don't
     have multi-buy code, change the #if 1 to #if 0.
     */
#define THE_PUB	3940
  static int	open=1; /* 0 closed;  1 open;  2 last call */
  char argm[100], newarg[100], buf[MAX_STRING_LENGTH];
  struct obj_data *temp1, *temp2;
  struct char_data *temp_char;
  struct char_data *andy;
  int num, i, cost;
  static struct pub_beers {
    int	container, contains, howmany, actflag;
#define ACT_OVER_21 1
#define ACT_SNICKER 2
  } sold_here[] = {
    {3903, 3902, 6, 1 },
    {3905, 3904, 6, 1 },
    {3907, 3906, 6, 1 },
    {3909, 3908, 6, 3 },
    {3911, 3910, 6, 3 },
    {3913, 3912, 6, 3 },
    {3914, 0, 0, 1 },
    {3930, 0, 0, 0 },
    {3931, 0, 0, 0 },
    {3932, 0, 0, 0 },
    {3102, 0, 0, 0 },
    {-1}
  }, *scan;

  andy = 0;

  andy=FindMobInRoomWithFunction(ch->in_room,andy_wilcox);

  if (open==0 && time_info.hours == 11) {
    open = 1;
    do_unlock(andy, "door", 0);
    do_open(andy, "door", 0);
    act("$n says 'We're open for lunch, come on in.'", FALSE, andy, 0,0, TO_ROOM);
  }
  if (open==1 && time_info.hours == 1) {
    open = 2;
    act("$n says 'Last call, guys and gals.'", FALSE, andy, 0,0, TO_ROOM);
  }
  if (open==2 && time_info.hours == 2) {
    open = 0;
    act("$n says 'We're closing for the night.\n  Thanks for coming, all, and come again!'", FALSE, andy, 0,0, TO_ROOM);
    do_close(andy, "door", 0);
    do_lock(andy, "door", 0);
  }

  switch (cmd) {
  case 25:	/* kill */
  case 70:	/* hit */
  case 157:	/* bash */
  case 159:	/* kick */
    only_argument(arg, argm);

    if (andy == ch)
      return TRUE;
    if (andy == get_char_room(argm, ch->in_room))
      {
	int	hitsleft;
	act("$n says 'Get this, $N wants to kill me', and\n falls down laughing.", FALSE, andy, 0, ch, TO_ROOM);
	hitsleft = dice(2,6) + 6;
	if (hitsleft < GET_HIT(ch) && GetMaxLevel(ch) <= MAX_MORT) {
	  act("$n beats the shit out of $N.", FALSE, andy, 0, ch, TO_NOTVICT);
	  act("$n beats the shit out of you.  OUCH!", FALSE, andy, 0, ch, TO_VICT);
	  GET_HIT(ch) = hitsleft;
	} else {
	  act("$n grabs $N in a vicious sleeper hold.", FALSE, andy, 0, ch, TO_NOTVICT);
	  act("$n puts you in a vicious sleeper hold.", FALSE, andy, 0, ch, TO_VICT);
	}
	GET_POS(ch) = POSITION_SLEEPING;
      }
    else
      {
	do_action(andy, ch->player.name, 130 /* slap */);
	act("$n says 'Hey guys, I run a quiet pub.  Take it outside.'",
	    FALSE, andy, 0, 0, TO_ROOM);
      }
    return TRUE;
    break;

  case 156:	/* steal */
    if (andy == ch)
      return TRUE;
    do_action(andy, ch->player.name, 130 /* slap */);
    act("$n tells you 'Who the hell do you think you are?'",
	FALSE, andy, 0, ch, TO_VICT);
    do_action(andy, ch->player.name, 116 /* glare */);
    return TRUE;
    break;

  case 84:
  case 207:
  case 172:	/* cast, recite, use */
    if (andy == ch)
      return TRUE;
    do_action(andy, ch->player.name, 94 /* poke */);
    act("$n tells you 'Hey, no funny stuff.'.", FALSE, andy, 0, ch, TO_VICT);
    return TRUE;
    break;

  case 56: /* buy */
    if (ch->in_room != THE_PUB) {
      act("$n tells you 'Hey man, I'm on my own time, but stop by the Pub some time.'", FALSE, andy, 0, ch, TO_VICT);
      return TRUE;
    }
    if (open==0) {
      act("$n tells you 'Sorry, we're closed, come back for lunch.'",
	  FALSE, andy, 0, ch, TO_VICT);
      return TRUE;
    }
    only_argument(arg, argm);
    if (!(*argm)) {
      act("$n tells you 'Sure, what do you want to buy?'",
	  FALSE, andy, 0, ch, TO_VICT);
      return TRUE;
    }

#if 1
    /* multiple buy code */
    if ((num = getabunch(argm,newarg))!=NULL) {
      strcpy(argm,newarg);
    }
    if (num == 0) num = 1;
#endif

    if(!( temp1 = get_obj_in_list_vis(ch,argm,andy->carrying)))
      {
	act("$n tells you 'Sorry, but I don't sell that.'", FALSE, andy, 0, ch, TO_VICT);
	return TRUE;
      }
    for (scan = sold_here; scan->container>=0; scan++) {
      if (scan->container == temp1->virtual)
	break;
    }
    if (scan->container<0)
      {
	act("$n tells you 'Sorry, that's not for sale.'", FALSE, andy, 0, ch, TO_VICT);
	return TRUE;
      }

    if (scan->actflag&ACT_OVER_21 && GET_AGE(ch)<21 ) {
      if (IS_IMMORTAL(ch)) {
	act("$N manages to slip a fake ID past $n.",
	    FALSE, andy, 0, ch, TO_NOTVICT);
      } else if ( !IS_NPC(ch) ) {
	act("$n tells you 'Sorry, I could lose my license if I served you alcohol.'", FALSE, andy, 0, ch, TO_VICT);
	act("$n cards $N and $N is BUSTED.", FALSE, andy, 0, ch, TO_NOTVICT);
	return TRUE;
      }
    }

    temp2 = get_obj(scan->contains);
    cost = ( temp2 ? (scan->howmany * temp2->obj_flags.cost) : 0 )
      + temp1->obj_flags.cost;
    cost *= 9;
    cost /=10;
    cost++;
    if (temp2)
      extract_obj(temp2);

    for (; num>0; num--)
      {
	if (GET_GOLD(ch) < cost) {
	  act("$n tells you 'Sorry, man, no bar tabs.'",
	      FALSE, andy, 0, ch, TO_VICT);
	  return TRUE;
	}
	temp1 = get_obj(temp1->virtual);
	for (i=0; i<scan->howmany; i++) {
	  temp2 = get_obj(scan->contains);
	  obj_to_obj(temp2, temp1);
	}
	obj_to_char(temp1, ch);
	GET_GOLD(ch) -= cost;
	act("$N buys a $p from $n", FALSE, andy, temp1, ch, TO_NOTVICT);
	if (scan->actflag&ACT_SNICKER) {
	  act("$n snickers softly.", FALSE, andy, NULL, ch, TO_ROOM);
	} else {
	  act((scan->actflag&ACT_OVER_21) ?
	      "$n tells you 'Drink in good health' and gives you $p" :
	      "$n tells you 'Enjoy' and gives you $p",
	      FALSE, andy, temp1, ch, TO_VICT);
	}
      }
    return TRUE;
    break;

  case 59: /* list */
    act("$n says 'We have", FALSE, andy, NULL, ch, TO_VICT);
    for (scan = sold_here; scan->container>=0; scan++) {
      temp1 = get_obj(scan->container);
      temp2 = scan->contains ? get_obj(scan->contains) : NULL;
      cost = (temp2 ? (scan->howmany * temp2->obj_flags.cost) : 0 )
	+ temp1->obj_flags.cost;
      cost *= 9;
      cost /=10;
      cost++;
      sprintf(buf,"%s for %d gold coins.\n", temp1->short_description, cost);
      send_to_char(buf, ch);
      extract_obj(temp1);
      if (temp2)
	extract_obj(temp2);
    }
    return TRUE;
    break;
  }

  return FALSE;
}


int eric_johnson(struct char_data *ch, int cmd, char *arg)

{
  /* if more than one eric johnson exists in a game, it will
     get confused because of the state variables */
#define	E_HACKING	0
#define	E_SLEEPING	1
#define	E_SHORT_BEER_RUN 2
#define	E_LONG_BEER_RUN	3
#define E_STOCK_FRIDGE	4
#define	E_SKYDIVING	5
#define Erics_Lair	3941
#define DanjerKitchen	3904
#define DanjerLiving	3901
#define DanjerPorch	3900
  static int	fighting=0, state=E_HACKING;
  struct obj_data *temp1;
  struct char_data	*eric, *temp_char;
  char	buf[100];

  eric = 0;

  eric = FindMobInRoomWithFunction(ch->in_room,eric_johnson);

  if (ch==eric) {

    if (cmd!=0)
      return FALSE; /* prevent recursion when eric wants to move */

    if (!fighting && ch->specials.fighting) {
      act("$n says 'What the fuck?'", FALSE, eric, 0, 0, TO_ROOM);
      fighting=1;
    }
    if (fighting && !ch->specials.fighting) {
      act("$n says 'I wonder what their problem was.'", FALSE, eric, 0, 0, TO_ROOM);
      fighting=0;
    }
    if (fighting) {
      struct char_data	*target = eric->specials.fighting;
      act("$n yells for help.", FALSE, eric, 0, 0, TO_ROOM);
      act("$n throws some nasty judo on $N.", FALSE, eric, 0, target, TO_NOTVICT);
      act("$n throws some nasty judo on you.", FALSE, eric, 0, target, TO_VICT);
      damage(eric, target, dice(2,4), TYPE_HIT);
      if (!saves_spell(target, SAVING_SPELL)) {
	struct affected_type af;
	af.type = SPELL_SLEEP;
	af.duration = 2;
	af.modifier = 0;
	af.location = APPLY_NONE;
	init_bits(af.bitvector);
	set_bit(af.bitvector,AFF_SLEEP);
	affect_join(target, &af, FALSE, FALSE);
	if (target->specials.fighting)
	  stop_fighting(target);
	if (eric->specials.fighting)
	  stop_fighting(eric);
	act("$N is out cold.", FALSE, eric, 0, target, TO_NOTVICT);
	act("You are out cold.", FALSE, eric, 0, target, TO_VICT);
	GET_POS(target) = POSITION_SLEEPING;
	RemHated(eric, target);
	RemHated(target, eric);
      }
      return FALSE;
    }

    switch(state) {
    case E_HACKING:
      if (GET_POS(eric)==POSITION_SLEEPING) {
	do_wake(eric, "", -1);
	return TRUE;
      }
      break;
    case E_SLEEPING:
      if (GET_POS(eric)!=POSITION_SLEEPING) {
	act("$n says 'Go away, I'm sleeping'", FALSE, eric, 0,0, TO_ROOM);
	do_sleep(eric, "", -1);
	return TRUE;
      }
      break;
    default:
      if (GET_POS(eric)==POSITION_SLEEPING) {
	do_wake(eric, "", -1);
	return TRUE;
      } else if (GET_POS(eric)!=POSITION_STANDING) {
	do_stand(eric, "", -1);
	return TRUE;
      }
      break;
    }

    switch(state) {
      char	*s;
    case E_SLEEPING:
      if (time_info.hours>9 && time_info.hours<12) {
	do_wake(eric, "", -1);
	act("$n says 'Ahh, that was a good night's sleep'", FALSE, eric,
	    0,0, TO_ROOM);
	state = E_HACKING;
	return TRUE;
      }
      return TRUE;
      break;
    case E_HACKING:
      if (eric->in_room != Erics_Lair) {
	/* he's not in his lair, get him there. */
	int	dir;
	if (eric->in_room == DanjerLiving) {
	  do_close(eric, "front",-1);
	  do_lock(eric, "front",-1);
	}
	dir = choose_exit(eric->in_room, Erics_Lair, -100);
	if (dir<0)
	  {
	    if (eric->in_room == DanjerPorch) {
	      do_unlock(eric, "front",-1);
	      do_open(eric, "front",-1);
	      return TRUE;
	    }
	    dir = choose_exit(eric->in_room, DanjerPorch, -100);
	  }
	if (dir<0) {
	  if (dice(1,2)==1)
	    act("$n says 'Shit, I'm totally lost.", FALSE, eric, 0,0,TO_ROOM);
	  else
	    act("$n says 'Can you show me the way back to the DanjerHaus?'",
		FALSE, eric, 0,0, TO_ROOM);
	} else
	  go_direction(eric, dir);

      } else {
	if (time_info.hours>22 || time_info.hours<3) {
	  state = E_SLEEPING;
	  do_sleep(eric, 0, -1);
	  return TRUE;
	}

	do_sit(eric, "", -1);
	if (3==dice(1,5)) {
	  /* he's in his lair, do lair things */
	  switch (dice(1,5)) {
	  case 1:
	    s = "$n looks at you, then resumes hacking";
	    break;
	  case 2:
	    s = "$n swears at the terminal and resumes hacking";
	    break;
	  case 3:
	    s = "$n looks around and says 'Where's Big Guy?'";
	    break;
	  case 4:
	    s = "$n says 'Dude, RS/6000s suck.'";
	    break;
	  case 5:
	    temp1 = get_obj_in_list_vis(eric, "beer", eric->carrying);
	    if (temp1==NULL ||
		temp1->obj_flags.type_flag != ITEM_DRINKCON ||
		temp1->obj_flags.value[1] <= 0) {
	      s = "$n says 'Damn, out of beer'";
	      do_stand(eric, "", -1);
	      state = E_SHORT_BEER_RUN;
	    } else {
	      do_drink(eric, "beer", -1 /* irrelevant */);
	      s = "$n licks his lips";
	    }
	    break;
	  }
	  act(s, FALSE, eric, 0, 0, TO_ROOM);
	}
      }
      break;
    case E_SHORT_BEER_RUN:
      if (eric->in_room != DanjerKitchen) {
	int	dir;
	dir = choose_exit(eric->in_room, DanjerKitchen, -100);
	if (dir<0) {
	  if (dice(1,3)!=1)
	    act("$n says 'Dammit, where's the beer?",
		FALSE, eric, 0,0,TO_ROOM);
	  else
	    act("$n says 'Christ, who stole my kitchen?'",
		FALSE, eric, 0,0, TO_ROOM);
	} else {
	  go_direction(eric, dir);
	}
      } else {
	/* we're in the kitchen, find beer */
	temp1 = get_obj_in_list_vis(eric, "fridge",
				    real_roomp(eric->in_room)->contents);
	if (temp1==NULL) {
	  act("$n says 'Alright, who stole my refrigerator!'", FALSE, eric,
	      0, 0, TO_ROOM);
	} else if (IS_SET(temp1->obj_flags.value[1], CONT_CLOSED)) {
	  do_drop(eric, "bottle", -1 /* irrelevant */);
	  do_open(eric, "fridge", -1 /* irrelevant */);
	} else if (NULL == (temp1 = get_obj_in_list_vis(eric, "sixpack",
							eric->carrying))) {
	  do_get(eric, "sixpack fridge", 0);
	  if (NULL == get_obj_in_list_vis(eric, "sixpack",
					  eric->carrying)) {
	    act("$n says 'Aw, man.  Someone's been drinking all the beer.",
		FALSE, eric, 0, 0, TO_ROOM);
	    do_close(eric, "fridge", -1 /* irrelevant */);
	    state = E_LONG_BEER_RUN;
	  }
	} else if (NULL == (temp1 = get_obj_in_list_vis(eric, "beer",
							eric->carrying))) {
	  do_get(eric, "beer sixpack", 0);
	  if (NULL == get_obj_in_list_vis(eric, "beer",
					  eric->carrying)) {
	    act("$n says 'Well, that one's finished...'", FALSE, eric,
		0, 0, TO_ROOM);
	    do_drop(eric, "sixpack", -1 /* irrelevant */);
	  }
	} else {
	  do_put(eric, "sixpack fridge", 0);
	  do_close(eric, "fridge", -1 /* irrelevant */);
	  state = E_HACKING;
	}
      }
      break;
    case E_LONG_BEER_RUN:
      {
	static struct char_data *andy = 0;
	int	dir;
	static char	**scan,*shopping_list[] =
	  { "guinness", "harp", "sierra", "2.harp", NULL };

	for (temp_char = character_list; temp_char; temp_char = temp_char->next)
	  if (IS_MOB(temp_char))
	    if (real_mobp(temp_char->virtual)->func == andy_wilcox)
	      andy = temp_char;

	if (eric->in_room != andy->in_room) {
	  if (eric->in_room == DanjerPorch) {
	    do_close(eric, "front",-1);
	    do_lock(eric, "front",-1); /* this takes no time */
	  } else if (eric->in_room == DanjerLiving) {
	      do_unlock(eric, "front",-1);
	      do_open(eric, "front",-1);
	    return TRUE; /* this takes one turn */
	    }
	  dir = choose_exit(eric->in_room, andy->in_room, -100);
	  if (dir<0) {
	    dir = choose_exit(eric->in_room, DanjerLiving, -100);
	  }
	  if (dir<0) {
	    act("$n says 'Aw, man.  Where am I going to get more beer?",
		FALSE, eric, 0,0, TO_ROOM);
	    state = E_HACKING;
	  } else
	    go_direction(eric, dir);
	} else {
	  for (scan = shopping_list; *scan; scan++) {
	    if (NULL == get_obj_in_list_vis(eric, *scan,
					    eric->carrying)) {
	      char	*s;
	      s = (scan[0][1] == '.') ? scan[0]+2 : scan[0];
	      do_buy(eric, s, 0);
	      if (NULL == get_obj_in_list_vis(eric, *scan,
					      eric->carrying)) {
		act("$n says 'ARGH, where's my deadbeat roommate with the rent.'", FALSE, eric, 0,0, TO_ROOM);
		act("$n says 'I need beer money.'", FALSE, eric, 0,0, TO_ROOM);
		state = (scan==shopping_list) ? E_HACKING : E_STOCK_FRIDGE;
		return TRUE;
	      }
	      break;
	    }
	  }
	  if (*scan==NULL || 1 == dice(1,4)) {
	    act("$n says 'Catch you later, dude.'", FALSE, eric, 0,0, TO_ROOM);
	    state = E_STOCK_FRIDGE;
	  }
	}
      }
      break;
    case E_STOCK_FRIDGE:
      if (eric->in_room != DanjerKitchen) {
	int	dir;
	if (eric->in_room == DanjerLiving) {
	  do_close(eric, "front",-1);
	  do_lock(eric, "front",-1);
	}
	dir = choose_exit(eric->in_room, DanjerKitchen, -100);
	if (dir<0) {
	  if (eric->in_room == DanjerPorch) {
	    do_unlock(eric, "front",-1);
	    do_open(eric, "front",-1);
	    return TRUE;
	  }
	  dir = choose_exit(eric->in_room, DanjerPorch, -100);
	}
	if (dir<0) {
	  if (dice(1,3)!=1)
	    act("$n says 'Dammit, where's the fridge?",
		FALSE, eric, 0,0,TO_ROOM);
	  else
	    act("$n says 'Christ, who stole my kitchen?'",
		FALSE, eric, 0,0, TO_ROOM);
	} else {
	  go_direction(eric, dir);
	}
      } else {
	/* we're in the kitchen, find beer */
	temp1 = get_obj_in_list_vis(eric, "fridge",
				    real_roomp(eric->in_room)->contents);
	if (temp1==NULL) {
	  act("$n says 'Alright, who stole my refrigerator!'", FALSE, eric,
	      0, 0, TO_ROOM);
	} else if (IS_SET(temp1->obj_flags.value[1], CONT_CLOSED)) {
	  do_open(eric, "fridge", -1 /* irrelevant */);
	} else if (NULL == (temp1 = get_obj_in_list_vis(eric, "beer",
							eric->carrying))) {
	  do_get(eric, "beer sixpack", 0);
	  if (NULL == get_obj_in_list_vis(eric, "beer",
					  eric->carrying)) {
	    act("$n says 'What the hell, I just bought this?!'", FALSE, eric,
		0, 0, TO_ROOM);
	    do_drop(eric, "sixpack", -1 /* irrelevant */);
	    if (NULL == get_obj_in_list_vis(eric, "sixpack", eric->carrying))
	      state = E_HACKING;
	  }
	} else {
	  do_put(eric, "all.sixpack fridge", 0);
	  do_close(eric, "fridge", -1 /* irrelevant */);
	  state = E_HACKING;
	}
      }
      break;
    }
  }

  return FALSE;
}


#define TAX_INTERVAL (60*60*48) /* once every 2 real days */

int tax_collector(struct char_data *ch,int cmd, char *arg)
{
  struct char_data *tc;
  char buf[MAX_STRING_LENGTH];
  int amount,tax_times;

  if (cmd==93) {
	sendf(ch,"It would cost you %d coins if I taxed you right now.\n",
		MAX(100000,amount_of_tax(ch)));
	return(TRUE);
  }
  if (IS_MOB(ch) || IS_IMMORTAL(ch)) return(FALSE);
  if (ch->specials.last_taxed==0)
    tax_times=1;
  else
    tax_times=abs(time(0) - ch->specials.last_taxed)/TAX_INTERVAL;
  if (tax_times==0) return(FALSE);
  if (tax_times>3) tax_times=3;
  if (GET_EXP(ch)<10) return(0);
  tc=FindMobInRoomWithFunction(ch->in_room, tax_collector);
  if (!tc) {
	nlog("Tax collector mixup");
	return(FALSE);
  }
  act("$n is stopped by $N.\n",TRUE,ch,0,tc,TO_ROOM);
  act("You have been stopped by $N",FALSE,ch,0,tc,TO_CHAR);
  act("$n does some quick math.\n",TRUE,tc,0,0,TO_ROOM);
  amount=MIN(100000,amount_of_tax(ch)*tax_times);
  sprintf(buf,"%s, Your tax comes to %d.",GET_NAME(ch),amount);
  do_say(tc,buf,0);
  fine(ch,amount,tc);
  ch->specials.last_taxed=time(0);
  return(TRUE);
}


int mayor(struct char_data *ch, int cmd, char *arg)
{
  static char open_path[] =
    "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static char close_path[] =
    "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static char *path;
  static int index;
  static bool move = FALSE;



  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      index = 0;
    }
  }

  if (cmd || !move || (GET_POS(ch) < POSITION_SLEEPING) ||
      (GET_POS(ch) == POSITION_FIGHTING))
    return FALSE;

  switch (path[index]) {
  case '0' :
  case '1' :
  case '2' :
  case '3' :
    do_move(ch,"",path[index]-'0'+1);
    break;

  case 'W' :
    GET_POS(ch) = POSITION_STANDING;
    act("$n awakens and groans loudly.",FALSE,ch,0,0,TO_ROOM);
    break;

  case 'S' :
    GET_POS(ch) = POSITION_SLEEPING;
    act("$n lies down and instantly falls asleep.",FALSE,ch,0,0,TO_ROOM);
    break;

  case 'a' :
    act("$n says 'Hello Honey!'",FALSE,ch,0,0,TO_ROOM);
    act("$n smirks.",FALSE,ch,0,0,TO_ROOM);
    break;

  case 'b' :
    act("$n says 'What a view! I must get something done about that dump!'",
        FALSE,ch,0,0,TO_ROOM);
    break;

  case 'c' :
    act("$n says 'Vandals! Youngsters nowadays have no respect for anything!'",
        FALSE,ch,0,0,TO_ROOM);
    break;

  case 'd' :
    act("$n says 'Good day, citizens!'", FALSE, ch, 0,0,TO_ROOM);
    break;

  case 'e' :
    act("$n says 'I hereby declare the bazaar open!'",FALSE,ch,0,0,TO_ROOM);
    break;

  case 'E' :
    act("$n says 'I hereby declare Midgaard closed!'",FALSE,ch,0,0,TO_ROOM);
    break;

  case 'O' :
    do_unlock(ch, "gate", 0);
    do_open(ch, "gate", 0);
    break;

  case 'C' :
    do_close(ch, "gate", 0);
    do_lock(ch, "gate", 0);
    break;

  case '.' :
    move = FALSE;
    break;

  }

  index++;
  return FALSE;
}


int zm_tired(struct char_data *zmaster)
{
  return GET_HIT(zmaster) < GET_MAX_HIT(zmaster)/2 ||
    GET_MANA(zmaster) < 40;
}

int zm_stunned_followers(struct char_data *zmaster)
{
  struct follow_type	*fwr;
  for (fwr = zmaster->followers; fwr; fwr = fwr->next)
    if (GET_POS(fwr->follower)==POSITION_STUNNED)
      return TRUE;
  return FALSE;
}

void zm_init_combat(struct char_data *zmaster, struct char_data *target)
{
  struct follow_type	*fwr;
  for (fwr = zmaster->followers; fwr; fwr = fwr->next)
    if (IS_AFFECTED(fwr->follower, AFF_CHARM) &&
	fwr->follower->specials.fighting==NULL &&
	fwr->follower->in_room == target->in_room)
      if (GET_POS(fwr->follower) == POSITION_STANDING) {
	hit(fwr->follower, target, TYPE_UNDEFINED);
      } else if (GET_POS(fwr->follower)>POSITION_SLEEPING &&
		 GET_POS(fwr->follower)<POSITION_FIGHTING) {
	do_stand(fwr->follower, "", -1);
      }
}

int zm_kill_fidos(struct char_data *zmaster)
{
  struct char_data	*fido_b;
  fido_b = FindMobInRoomWithFunction(zmaster->in_room,fido);
  if (fido_b) {
    act("$n shrilly screams 'Kill that carrion beast!'", FALSE,
	zmaster, 0,0, TO_ROOM);
    zm_init_combat(zmaster, fido_b);
    return TRUE;
  }
  return FALSE;
}

int zm_kill_aggressor(struct char_data *zmaster)
{
  struct follow_type	*fwr;
  if (zmaster->specials.fighting)
    {
      act("$n bellows 'Kill that mortal that dares lay hands on me!'", FALSE,
	  zmaster, 0,0, TO_ROOM);
      zm_init_combat(zmaster, zmaster->specials.fighting);
      return TRUE;
    }
  for (fwr = zmaster->followers; fwr; fwr = fwr->next)
    if (fwr->follower->specials.fighting &&
	IS_AFFECTED(fwr->follower, AFF_CHARM))
      {
	act("$n bellows 'Assist your brethren, my loyal servants!'", FALSE,
	    zmaster, 0,0, TO_ROOM);
	zm_init_combat(zmaster, fwr->follower->specials.fighting);
	return TRUE;
      }
  return FALSE;
}

int zombie_master(struct char_data *ch, int cmd, char *arg)
#define ZM_MANA	10
#define ZM_NEMESIS 3060
{
  struct obj_data *temp1;
  struct char_data	*zmaster;
  char	buf[240];
  int	dir;

  zmaster = FindMobInRoomWithFunction(ch->in_room,zombie_master);

  if (cmd!=0 || ch != zmaster || !AWAKE(ch))
    return FALSE;

  if (!check_peaceful(ch, "") &&
      (zm_kill_fidos(zmaster) || zm_kill_aggressor(zmaster))
      ) {
    do_stand(zmaster, "", -1);
    return TRUE;
  }

  switch (GET_POS(zmaster)) {
  case POSITION_RESTING:
    if (!zm_tired(zmaster))
      do_stand(zmaster, "", -1);
    break;
  case POSITION_SITTING:
    if (!zm_stunned_followers(zmaster)) {
      act("$n says 'It took you long enough...'", FALSE,
	  zmaster, 0, 0, TO_ROOM);
      do_stand(zmaster, "", -1);
    }
    break;
  case POSITION_STANDING:
    if (zm_tired(zmaster)) {
      do_rest(zmaster, "", -1);
      return TRUE;
    }

    temp1 = get_obj_in_list_vis(zmaster, "corpse",
				real_roomp(zmaster->in_room)->contents);

    if (temp1) {
      if (GET_MANA(zmaster) < ZM_MANA) {
	if (1==dice(1,20))
	  act("$n says 'So many bodies, so little time' and sighs.",
	      FALSE, zmaster, 0,0, TO_ROOM);
      } else {
	act("$n says 'Wonderful, another loyal follower!' and grins maniacly.",
	    FALSE, zmaster, 0,0, TO_ROOM);
	GET_MANA(zmaster) -= ZM_MANA;
	spell_animate_dead(GetMaxLevel(zmaster), ch, NULL, temp1,GetMaxLevel(zmaster));
	/* assume the new follower is top of the list? */
	AddHatred( zmaster->followers->follower, OP_VNUM, ZM_NEMESIS);
      }
      return TRUE;
    } else if (zm_stunned_followers(zmaster)) {
      do_sit(zmaster, "", -1);
      return TRUE;
    } else if (1==dice(1,20)) {
      act("$n searches for bodies.", FALSE, zmaster, 0,0, TO_ROOM);
      return TRUE;
    } else if ( 0<=(dir = find_path(zmaster->in_room,
				    named_object_on_ground, "corpse", -200))) {
      go_direction(zmaster, dir);
      return TRUE;
    } else if (1==dice(1,5)) {
      act("$n can't find any bodies.", FALSE, zmaster, 0,0, TO_ROOM);
      return TRUE;
    } else {
      mobile_wander(zmaster);
    }
  }

  return FALSE;
}


/*
  house routine for saved items.
*/
int House(struct char_data *ch, int cmd, char *arg)
{
  struct obj_cost cost;
  int i, save_room;
  return(FALSE);
#if 0

  if (IS_NPC(ch)) return(FALSE);

  /* if (cmd != rent) ignore */
  if (cmd != 92) {
    return(FALSE);
  } else {

    /*  verify the owner */
    if (strncmp(GET_NAME(ch), real_roomp(ch->in_room)->name, strlen(GET_NAME(ch)))) {
      send_to_char("Sorry, you'll have to find your own house.\n",ch);
      return(FALSE);
    }

    cost.total_cost = 0; /* Minimum cost */
    cost.no_carried = 0;
    cost.ok = TRUE; /* Use if any "-1" objects */

    add_obj_cost(ch, 0, ch->carrying, &cost);

    for(i = 0; i<MAX_WEAR; i++)
      add_obj_cost(ch, 0, ch->equipment[i], &cost);

    if (!cost.ok) {
      return(FALSE);
    }
    cost.total_cost = 0;

    save_obj(ch, &cost,1);
    save_room = ch->in_room;
    extract_char(ch);
    ch->in_room = save_room;
    save_char(ch, ch->in_room);
  }
#endif
}



/* ********************************************************************
*  General special procedures for mobiles                                      *
******************************************************************** */

/* SOCIAL GENERAL PROCEDURES

If first letter of the command is '!' this will mean that the following
command will be executed immediately.

"G",n      : Sets next line to n
"g",n      : Sets next line relative to n, fx. line+=n
"m<dir>",n : move to <dir>, <dir> is 0,1,2,3,4 or 5
"w",n      : Wake up and set standing (if possible).
"c<txt>",n : Look for a person named <txt> in the room
"o<txt>",n : Look for an object named <txt> in the room
"r<int>",n : Test if the npc in room number <int>?
"s",n      : Go to sleep, return false if can't go sleep
"e<txt>",n : echo <txt> to the room, can use $o/$p/$N depending on
             contents of the **thing
"E<txt>",n : Send <txt> to person pointed to by thing
"B<txt>",n : Send <txt> to room, except to thing
"?<num>",n : <num> in [1..99]. A random chance of <num>% success rate.
             Will as usual advance one line upon sucess, and change
             relative n lines upon failure.
"O<txt>",n : Open <txt> if in sight.
"C<txt>",n : Close <txt> if in sight.
"L<txt>",n : Lock <txt> if in sight.
"U<txt>",n : Unlock <txt> if in sight.    */

/* Execute a social command.                                        */
void exec_social(struct char_data *npc, char *cmd, int next_line,
                 int *cur_line, void **thing)
{
  bool ok;

  /*void do_move(struct char_data *ch, char *argument, int cmd);
  void do_open(struct char_data *ch, char *argument, int cmd);
  void do_lock(struct char_data *ch, char *argument, int cmd);
  void do_unlock(struct char_data *ch, char *argument, int cmd);
  void do_close(struct char_data *ch, char *argument, int cmd);*/

  if (GET_POS(npc) == POSITION_FIGHTING)
    return;

  ok = TRUE;

  switch (*cmd) {

    case 'G' :
      *cur_line = next_line;
      return;

    case 'g' :
      *cur_line += next_line;
      return;

    case 'e' :
      act(cmd+1, FALSE, npc, *thing, *thing, TO_ROOM);
      break;

    case 'E' :
      act(cmd+1, FALSE, npc, 0, *thing, TO_VICT);
      break;

    case 'B' :
      act(cmd+1, FALSE, npc, 0, *thing, TO_NOTVICT);
      break;

    case 'm' :
      do_move(npc, "", *(cmd+1)-'0'+1);
      break;

    case 'w' :
      if (GET_POS(npc) != POSITION_SLEEPING)
        ok = FALSE;
      else
        GET_POS(npc) = POSITION_STANDING;
      break;

    case 's' :
      if (GET_POS(npc) <= POSITION_SLEEPING)
        ok = FALSE;
      else
        GET_POS(npc) = POSITION_SLEEPING;
      break;

    case 'c' :  /* Find char in room */
      *thing = get_char_room_vis(npc, cmd+1);
      ok = (*thing != 0);
      break;

    case 'o' : /* Find object in room */
      *thing = get_obj_in_list_vis(npc, cmd+1, real_roomp(npc->in_room)->contents);
      ok = (*thing != 0);
      break;

    case 'r' : /* Test if in a certain room */
      ok = (npc->in_room == atoi(cmd+1));
      break;

    case 'O' : /* Open something */
      do_open(npc, cmd+1, 0);
      break;

    case 'C' : /* Close something */
      do_close(npc, cmd+1, 0);
      break;

    case 'L' : /* Lock something  */
      do_lock(npc, cmd+1, 0);
      break;

    case 'U' : /* UnLock something  */
      do_unlock(npc, cmd+1, 0);
      break;

    case '?' : /* Test a random number */
      if (atoi(cmd+1) <= number(1,100))
        ok = FALSE;
      break;

    default:
      break;
  }  /* End Switch */

  if (ok)
    (*cur_line)++;
  else
    (*cur_line) += next_line;
}


int ninja_master(struct char_data *ch, int cmd, char *arg)
{
  return(FALSE);
}

#endif
