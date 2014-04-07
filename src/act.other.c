/* ************************************************************************
*  file: act.other.c , Implementation of commands.        Part of DIKUMUD *
*  Usage : Other commands.                                                *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <fcns.h>
#include <limits.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <spells.h>
#include <externs.h>
#include <ticks.h>


void do_gain(struct char_data *ch, char *argument, int cmd)
{

}

void do_guard(struct char_data *ch, char *argument, int cmd)
{

  if (!IS_NPC(ch)) {
    send_to_char("Sorry. you can't just put your brain on autopilot!\n",ch);
    return;
  }

  if (IS_SET(ch->specials.act, ACT_GUARDIAN)) {
    act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You relax.\n",ch);
    CLEAR_BIT(ch->specials.act, ACT_GUARDIAN);
  } else {
    SET_BIT(ch->specials.act, ACT_GUARDIAN);
    act("$n alertly watches you.", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You snap to attention\n", ch);    
  }
  return;
}

void do_sacrifice(struct char_data *ch, char *argument, int cmd)
{
  char arg[100];
  int align_delta;
  struct obj_data *obj;
  struct affected_type af;
  struct index_mem *o;

  only_argument(argument, arg);
  if (!*arg) {
    sendf(ch, "Sacrifice what?\n");
    return;
  }
  obj = get_obj_in_list_vis(ch, arg, ch->carrying);
  if(!obj){
    sendf(ch, "You can't find that item to sacrifice.\n");
    return;
  }
  if(!(o = real_objp(obj->virtual))){
    sendf(ch, "This item is worthless to your deity.\n");
    return;
  }
  if (!IS_ALTAR(ch)) {
    sendf(ch, "You must be at an altar to sacrifice an item.\n");
    return;
  }
  if(!IS_OBJ_STAT(obj, ITEM_MAGIC) && (o->max_at_boot >= 20)){
    sendf(ch, "You can only sacrifice magical items or rare artifacts.\n");
    return;
  }
  act("$p bursts into flames and is consumed.", FALSE, ch, obj, NULL,
    TO_CHAR);
  act("$n offers $p to the gods and it is consumed in fire.", FALSE, ch, obj,
    NULL, TO_ROOM);
  if(o->max_at_boot < 20){
    sendf(ch, "Your deity is most impressed with your sacrifice and blesses you.\n");
    obj_from_char(obj);
    extract_obj(obj);
    if(!affected_by_spell(ch, SPELL_BLESS)){
      af.type = SPELL_BLESS;
      af.duration = 24;
      af.location = APPLY_HITROLL;
      af.modifier = 2;
      init_bits(af.bitvector);
      affect_to_char(ch, &af);
      af.location = APPLY_SAVING_SPELL;
      af.modifier = -2;
      affect_to_char(ch, &af);
    }
    if(IS_TRUE_GOOD(ch)) GET_ALIGNMENT(ch) = 1000;
    else if(IS_TRUE_EVIL(ch)) GET_ALIGNMENT(ch) = -1000;
    else GET_ALIGNMENT(ch) = 0;
    return;
  }
  else{
    sendf(ch, "You feel your deity smiling down upon you.\n");
    align_delta = obj->obj_flags.cost / (10 * GET_LEVEL(ch));
    obj_from_char(obj);
    extract_obj(obj);
    if(IS_TRUE_GOOD(ch)) GET_ALIGNMENT(ch) = MIN(1000, GET_ALIGNMENT(ch) +
	align_delta);
    else if(IS_TRUE_EVIL(ch)) GET_ALIGNMENT(ch) = MAX(-1000,
	GET_ALIGNMENT(ch) - align_delta);
    else if(GET_ALIGNMENT(ch) > 0) GET_ALIGNMENT(ch) = MAX(0,
	GET_ALIGNMENT(ch) - align_delta);
    else GET_ALIGNMENT(ch) = MIN(0, GET_ALIGNMENT(ch) + align_delta);
    if((IS_TRUE_GOOD(ch) && !IS_GOOD(ch)) ||
	(IS_TRUE_EVIL(ch) && !IS_EVIL(ch)) ||
	(IS_TRUE_NEUTRAL(ch) && !IS_NEUTRAL(ch))){
      sendf(ch, "However, you feel that you still need to do more.\n");
    }
    return;
  }
}

void do_donate(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  struct obj_data *tmp_object;

/*  get object name & verify */

  only_argument(argument, arg);
  if (!*arg) {
    sendf(ch,"Donate what?\n");
    return;
  }
  tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
  if (tmp_object) {
  	if (IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP) ||
		(tmp_object->virtual==25) ||
		(GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER) ||
		(GET_ITEM_TYPE(tmp_object) == ITEM_SPELL_POUCH) ||
		(GET_ITEM_TYPE(tmp_object) == ITEM_NOTE) ||     
		(GET_ITEM_TYPE(tmp_object) == ITEM_DRINKCON) || 
		(GET_ITEM_TYPE(tmp_object) == ITEM_KEY) ||      
		(GET_ITEM_TYPE(tmp_object) == ITEM_FOOD) ||     
		(GET_ITEM_TYPE(tmp_object) == ITEM_PEN) ||      
		(GET_ITEM_TYPE(tmp_object) == ITEM_TRAP)) {
		if (!IS_IMMORTAL (ch)) {
			send_to_char ("You can't donate that.\n", ch);
			return;
		} else {
			send_to_char ("Item is CURSED. (For your information).\n", ch);
		}
  	}
    act("You donate $p.", FALSE, ch, tmp_object, NULL, TO_CHAR);
    act("$n donates $p.", FALSE, ch, tmp_object, NULL, TO_ROOM);
    if(real_roomp(99)->people){
      sprintf(buf, "%s donates $p.", GET_NAME(ch));
      act(buf, FALSE, real_roomp(99)->people, tmp_object, NULL, TO_CHAR);
      act(buf, FALSE, real_roomp(99)->people, tmp_object, NULL, TO_ROOM);
    }
    obj_from_char(tmp_object);
    obj_to_room(tmp_object,99);
    tmp_object->obj_flags.timer2=DROP_TICKS*4;
  } else
    send_to_char("You don't have one of those?\n", ch);
}

int phys_junk(struct char_data *ch, struct obj_data *obj)
{
  char buf[MAX_INPUT_LENGTH];

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP) && !IS_IMMORTAL(ch)) {
    sendf(ch, "You can't junk that. It must be cursed.\n", ch);
    return(0);
  }
  /* moved things around for aesthetics and clarity - SLB */
  sendf(ch, "You junk %s.\n", obj->short_description);
  sprintf(buf, "$n junks %s.", obj->short_description);
  act(buf, 1, ch, 0, 0, TO_ROOM);
  obj_from_char(obj);
  extract_obj(obj);
  return(1);
}

void do_junk(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *tmp_object;

/*  get object name & verify */

  only_argument(argument, arg);
  if (!*arg) {
    sendf(ch,"Junk what?\n");
    return;
  }
  tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
  if (tmp_object) {
    phys_junk(ch, tmp_object);
  } else
    send_to_char("You don't have one of those?\n", ch);
}

void do_qui(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("You have to write quit - no less, to quit!\n",ch);
	return;
}


void do_title(struct char_data *ch, char *argument, int cmd)
{
   char buf[200];

   if (IS_SET(ch->specials.act, PLR_STUPID)) {  
      sendf(ch,"Stop being stupid or go away.\n");
      return;                                   
    }

   if (IS_NPC(ch) || !ch->desc)
       return;

  for(;isspace(*argument); argument++)  ;

  if (*argument && strlen(argument)<65) {
    sprintf(buf, "Your title has been set to : <%s>\n", argument);
    send_to_char(buf, ch);
    FREE(ch->player.title);
    ch->player.title = mystrdup(argument);
  } else if (*argument) {
    sendf(ch,"Too long.\n");
  }
}

void all_to_room(struct char_data *ch)
{
  struct obj_data *o,*next;
  int i;

  for (i=0; i<MAX_WEAR; i++)
    if (ch->equipment[i])
      obj_to_room(unequip_char(ch,i),ch->in_room);
  if (ch->carrying) {
    for (o=ch->carrying; o; o=next) {
      next=o->next_content;
      obj_from_char(o);
      o->obj_flags.timer2=DROP_TICKS;
      obj_to_room(o,ch->in_room);
    }
  }
}

void do_quit(struct char_data *ch, char *argument, int cmd)
{
	if (IS_NPC(ch))
		return;

	if (GET_POS(ch) == POSITION_FIGHTING) {
		send_to_char("No way! You are fighting.\n", ch);
		return;
	}

	if (GET_POS(ch) < POSITION_STUNNED) {
		send_to_char("You die before your time!\n", ch);
		die(ch);
		return;
	}

      	act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
	act("$n has left the game.", TRUE, ch,0,0,TO_ROOM);
        if (!ch->desc) {
	  remove_junk(ch, 1);
	  save_char(ch,NOWHERE);
	  extract_char(ch);
	  return;
	}
	if (!ROOM_FLAG(ch->in_room,RENTABLE)) {
	  all_to_room(ch);
	  extract_char(ch);
	} else {
	  remove_junk(ch, 1);
	  ch->specials.was_in_room=ch->in_room;
	  extract_char(ch);
        }
}



void do_save(struct char_data *ch, char *argument, int cmd)
{
	if (IS_NPC(ch)) return;

	if (cmd>0)
	  sendf(ch,"Saving %s.\n", GET_NAME(ch));
	else
	  sendf(ch,"Autosaved.\n");
	save_char(ch, NOWHERE);
}


void do_not_here(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("Sorry, but you cannot do that here!\n",ch);
}



void do_sneak(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type af;
  byte percent;
  
  if (IS_AFFECTED(ch, AFF_SNEAK)) {
    affect_from_char(ch, SKILL_SNEAK);
    send_to_char("You are no longer sneaking.\n",ch);
    return;
  }
  if ((get_skill (ch, SKILL_SNEAK)) <= 0) {
	send_to_char ("Sneak?!? You don't have a clue what class you are!\n", ch);
	return;
  }
  send_to_char("Ok, you'll try to move silently for a while.\n", ch);
  
  percent=number(1,101); /* 101% is a complete failure */
  
  if (percent > (get_skill(ch,SKILL_SNEAK) + dex_app_skill[GET_DEX(ch)].sneak +
      stealth_penalty(ch)))
    return;
  
  af.type = SKILL_SNEAK;
  af.duration = GET_LEVEL(ch);
  af.modifier = 0;
  af.location = APPLY_NONE;
  init_set(af.bitvector,AFF_SNEAK);
  affect_to_char(ch, &af);
}

void do_hide(struct char_data *ch, char *argument, int cmd)
{
	byte percent;

        if (GET_POS(ch) <= POSITION_FIGHTING) {
                send_to_char ("Hide?!? NO WAY! You must be standing and not fighting!\n", ch);
                return;
        }

	if ((get_skill (ch, SKILL_HIDE)) <= 0) {
		send_to_char ("Hide?!? You were never very good at that as a child.\n", ch);
		return;
	}
	percent=number(1,101); /* 101% is a complete failure */

	send_to_char("Ok, you'll try to hide for a while.\n", ch);
	if (percent > (get_skill(ch,SKILL_HIDE) + dex_app_skill[GET_DEX(ch)].hide + stealth_penalty(ch)))
		return;


	set_bit(ch->specials.affected_by, AFF_HIDE);
}


void do_steal(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  struct obj_data *obj;
  char victim_name[240];
  char obj_name[240];
  char buf[240];
  int percent;
  int gold, eq_pos;
  bool ohoh = FALSE;

  if (IsBuilder(ch)) {
    sendf(ch,"Nope, not gonna happen :)\n");
    vlog(LOG_URGENT,"STEAL: %s tried to steal from %s",GET_NAME(ch),argument);
    return;
  }
  if (check_peaceful(ch, "What if he caught you?\n"))
    return;
  
  argument = one_argument(argument, obj_name);
  only_argument(argument, victim_name);
  
  
  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Steal what from who?\n", ch);
    return;
  } else if (victim == ch) {
    send_to_char("Come on now, that's rather stupid!\n", ch);
    return;
  }
  
  if ((GetMaxLevel(ch) < 2) && (!IS_NPC(victim))) {
    send_to_char("Due to misuse of steal, you can't steal from other players\n", ch);
    send_to_char("unless you are at least 2cnd level. \n", ch);
    return;
  }

  if (!CORRUPT(ch,victim)) return;
  if (IS_SET(ch->specials.act,PLR_CORRUPT) && !IS_NPC(victim) &&
		GET_TRUE_ALIGN(ch)==1000) {
	sendf(ch,"You should have choosen the path of corruption.\n");
	return;
  }
  
  if ((!victim->desc) && (!IS_NPC(victim)))
    return;
  
  if (!IS_NPC(ch) && !IS_NPC(victim))
    vlog(LOG_MISC,"STEAL: %s from %s",GET_NAME(ch),GET_NAME(victim));
  /* 101% is a complete failure */
  percent=number(1,101) - dex_app_skill[GET_DEX(ch)].p_pocket;
  
  if (GET_POS(victim) < POSITION_SLEEPING)
    percent = -1; /* ALWAYS SUCCESS */
  
  percent += GET_LEVEL(victim);
  
  if (GetMaxLevel(victim)>MAX_MORT)
    percent = 101; /* Failure */
  
  if (str_cmp(obj_name, "coins") && str_cmp(obj_name,"gold")) {
    
    if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {
      
      for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
	if (victim->equipment[eq_pos] &&
	    (isname(obj_name, victim->equipment[eq_pos]->name)) &&
	    CAN_SEE_OBJ(ch,victim->equipment[eq_pos])) {
	  obj = victim->equipment[eq_pos];
	  break;
	}
      
      if (!obj) {
	act("$E has not got that item.",FALSE,ch,0,victim,TO_CHAR);
	return;
      } else { /* It is equipment */
	if ((GET_POS(victim) > POSITION_STUNNED)) {
	  send_to_char("Steal the equipment now? Impossible!\n", ch);
	  return;
	} else {
	  act("You unequip $p and steal it.",FALSE, ch, obj ,0, TO_CHAR);
	  act("$n steals $p from $N.",FALSE,ch,obj,victim,TO_NOTVICT);
	  obj_to_char(unequip_char(victim, eq_pos), ch);
	}
      }
    } else {  /* obj found in inventory */
      
      percent += GET_OBJ_WEIGHT(obj); /* Make heavy harder */
      
      if (AWAKE(victim) && (percent > get_skill(ch,SKILL_STEAL) +
          stealth_penalty(ch))) {
	if (!IS_NPC(ch) && !IS_NPC(victim))
	  vlog(LOG_MISC,"STEAL: %s from %s FAIL",
		GET_NAME(ch),GET_NAME(victim));
	ohoh = TRUE;
	act("Oops ...", FALSE, ch,0,0,TO_CHAR);
	act("$n tried to steal something from you!",FALSE,ch,0,victim,TO_VICT);
	act("$n tries to steal something from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
      } else { /* Steal the item */
        if (CAN_CARRY_OBJ(ch, obj)) {
	    obj_from_char(obj);
	    obj_to_char(obj, ch);
	    send_to_char("Got it!\n", ch);
	} else
	  send_to_char("You cannot carry that much.\n", ch);
      }
    }
  } else { /* Steal some coins */
    if (AWAKE(victim) && (percent > get_skill(ch,SKILL_STEAL))) {
      if (!IS_NPC(ch) && !IS_NPC(victim))
	vlog(LOG_MISC,"STEAL: %s from %s FAIL",GET_NAME(ch),GET_NAME(victim));
      ohoh = TRUE;
      act("Oops..", FALSE, ch,0,0,TO_CHAR);
      act("You discover that $n has $s hands in your wallet.",FALSE,ch,0,victim,TO_VICT);
      act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT);
    } else {
      /* Steal some gold coins */
      gold = (int) ((GET_GOLD(victim)*number(1,10))/100);
      gold = MIN(1782, gold);
      if (gold > 0) {
	GET_GOLD(ch) += gold;
	GET_GOLD(victim) -= gold;
	sprintf(buf, "Bingo! You got %d gold coins.\n", gold);
	send_to_char(buf, ch);
      } else {
	send_to_char("You couldn't get any gold...\n", ch);
      }
    }
  }
  
  if (ohoh && IS_NPC(victim) && AWAKE(victim))
    if (IS_SET(victim->specials.act, ACT_NICE_THIEF)) {
      sprintf(buf, "%s is a bloody thief.", GET_NAME(ch));
      do_shout(victim, buf, 0);
      log(buf);
      send_to_char("Don't you ever do that again!\n", ch);
    } else {
      hit(victim, ch, TYPE_UNDEFINED);
    }
  
}

void do_practice(struct char_data *ch, char *arg, int cmd)
{
  for (;isspace(*arg);arg++);
  if (*arg) {
    sendf(ch,"There is no guildmaster here.\n");
    return;
  }
  GuildMaster(ch,cmd,arg);
}







void do_idea(struct char_data *ch, char *argument, int cmd)
{
      FILE *fl;
      char str[MAX_INPUT_LENGTH+20];

      if (IS_SET(ch->specials.act, PLR_STUPID)) {  
         sendf(ch,"Stop being stupid or go away.\n");
         return;                                   
       }

	if (IS_NPC(ch))	{
		send_to_char("Monsters can't have ideas - Go away.\n", ch);
		return;
	}

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)	{
	      send_to_char
		("That doesn't sound like a good idea to me.. Sorry.\n",ch);
		return;
	}
	if (!(fl = fopen(IDEA_FILE, "a")))	{
		perror ("do_idea");
		send_to_char("Could not open the idea-file.\n", ch);
		return;
	}

	sprintf(str, "**%s: %s\n", GET_NAME(ch), argument);

	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok. Thanks.\n", ch);
}







void do_typo(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_INPUT_LENGTH+20];

      if (IS_SET(ch->specials.act, PLR_STUPID)) {  
         sendf(ch,"Stop being stupid or go away.\n");
         return;                                   
       }

	if (IS_NPC(ch))	{
		send_to_char("Monsters can't spell - leave me alone.\n", ch);
		return;
	}

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)	{
		send_to_char("I beg your pardon?\n", 	ch);
		return;
	}
	if (!(fl = fopen(TYPO_FILE, "a")))	{
		perror ("do_typo");
		send_to_char("Could not open the typo-file.\n", ch);
		return;
	}

	sprintf(str, "**%s[%d]: %s\n",
		GET_NAME(ch), ch->in_room, argument);
	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok. thanks.\n", ch);

}





void do_bug(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_INPUT_LENGTH+20];

      if (IS_SET(ch->specials.act, PLR_STUPID)) {  
         sendf(ch,"Stop being stupid or go away.\n");
         return;                                   
       }

	if (IS_NPC(ch))	{
		send_to_char("You are a monster! Bug off!\n", ch);
		return;
	}

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)	{
		send_to_char("Pardon?\n",ch);
		return;
	}
	if (!(fl = fopen(BUG_FILE, "a")))	{
		perror ("do_bug");
		send_to_char("Could not open the bug-file.\n", ch);
		return;
	}

	sprintf(str, "**%s[%d]: %s\n",
		GET_NAME(ch), ch->in_room, argument);
	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok.\n", ch);
}



void do_noexits(struct char_data *ch, char *argument, int cmd)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->specials.act, PLR_NOEXITS))	{
		send_to_char("Exits will now be shown.\n", ch);
		CLEAR_BIT(ch->specials.act, PLR_NOEXITS);
	}	else	{
		send_to_char("Exits will no longer be shown.\n", ch);
		SET_BIT(ch->specials.act, PLR_NOEXITS);
	}
}


void do_brief(struct char_data *ch, char *argument, int cmd)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->specials.act, PLR_BRIEF))	{
		send_to_char("Brief mode off.\n", ch);
		CLEAR_BIT(ch->specials.act, PLR_BRIEF);
	}	else	{
		send_to_char("Brief mode on.\n", ch);
		SET_BIT(ch->specials.act, PLR_BRIEF);
	}
}


void do_compact(struct char_data *ch, char *argument, int cmd)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->specials.act, PLR_COMPACT))	{
		send_to_char("You are now in the uncompacted mode.\n", ch);
		CLEAR_BIT(ch->specials.act, PLR_COMPACT);
	}	else	{
		send_to_char("You are now in compact mode.\n", ch);
		SET_BIT(ch->specials.act, PLR_COMPACT);
	}
}


void do_group(struct char_data *ch, char *argument, int cmd)
{
  char name[256];
  struct char_data *victim, *k;
  struct follow_type *f;
  int count;
  bool found;
  int low_lvl;
  int hi_lvl;
  int spread;
  
  only_argument(argument, name);
  
  /* no arg, so display the list if grouped */
  if (!*name) {
    	if (!IS_AFFECTED(ch, AFF_GROUP)) {
      		send_to_char("But you are a member of no group?!\n", ch);
    	} else {
      		send_to_char("Your group consists of:\n", ch);
      		if (ch->master)
			k = ch->master;
      		else
			k = ch;

    if (IS_AFFECTED(k, AFF_GROUP))
      sendf(ch,"Leader:    %-20s <%4dhp %5dma %5dpw %5dmv>\n",
        GET_NAME(k),GET_HIT(k),GET_MANA(k),GET_POWER(k),GET_MOVE(k));
      
      		count=1;
      		if (k->followers)
      			for(f=k->followers; f; f=f->next)
				if (IS_AFFECTED(f->follower, AFF_GROUP) && f->follower!=k)
        				sendf(ch,"[%3d]      %-20s <%4dhp %5dma %5dpw %5dmv>\n",++count, GET_NAME(f->follower),GET_HIT(f->follower), GET_MANA(f->follower), GET_POWER(f->follower), GET_MOVE(f->follower));
    	}
    
    	return;
  }
  
	/* must be in the room to group */
	if (!(victim = get_char_room_vis(ch, name))) {
    		send_to_char("No one here by that name.\n", ch);
		return;
	}
    
    	if (ch->master) {
      		send_to_char("You can not enroll group members without being head of a group.\n", ch);
      		return;
    	}
    	found = FALSE;
    
    	if (victim == ch)
      		found = TRUE;
    	else {
      		for(f=ch->followers; f; f=f->next) {
			if (f->follower == victim) {
	  			found = TRUE;
	  			break;
			}
      		}
    	}
	/* not following you, can not be grouped */
	if (!found) {
      		act("$N must follow you, to enter the group",
			FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	/* in group, so kick them out */
	if (IS_AFFECTED(victim, AFF_GROUP)) {
		act("$n has been kicked out of $N's group!",
			FALSE, victim, 0, ch, TO_ROOM);
		act("You are no longer a member of $N's group!",
			FALSE, victim, 0, ch, TO_CHAR);
		if (victim == ch) {
      			for(f=ch->followers; f; f=f->next) {
				clear_bit (f->follower->specials.affected_by,
					AFF_GROUP);
				sendf (f->follower,
					"%s has disbanded the group.\n",
					GET_NAME(ch));
			}
      		}
		clear_bit(victim->specials.affected_by, AFF_GROUP);
		return;
	}
	/* don't group immortals */
	if (GetMaxLevel(victim)>=LOW_IMMORTAL &&
	   GetMaxLevel(victim)<IMPLEMENTOR) {
	  	sprintf(name,"%s tried to form a group %s.",
			GET_NAME(ch),GET_NAME(victim));
	  	log2(name);
	  	act("You really don't want $n in your group.",
			FALSE, ch, 0, 0, TO_CHAR);
	  	return;
	}
	/* prevent cheating */
	if (GetMaxLevel(ch)>=LOW_IMMORTAL && GetMaxLevel(ch)<IMPLEMENTOR) {
		act("Now now.  That would be CHEATING!",FALSE,ch,0,0,TO_CHAR);
	  	sprintf(name,"%s tried to form a group with %s.",GET_NAME(ch),GET_NAME(victim));
	  	log2(name);
	  	return;  
	}

	/* get lowest level of the 2 players
	 * double the lowlvl, if < MAX_LEVEL_DIFF set to that.
	 * if the lowlvl is >= higher player's lvl, they can group
	 */
	if (GetMaxLevel (ch) < GetMaxLevel (victim)) {
		low_lvl = GetMaxLevel (ch);
		hi_lvl = GetMaxLevel (victim);
	} else {
		low_lvl = GetMaxLevel (victim);
		hi_lvl = GetMaxLevel (ch);
	}
	if (low_lvl < MAX_LEVEL_DIFF)
		spread = MAX_LEVEL_DIFF + low_lvl;
	else
		spread = low_lvl * MAX_LEVEL_SPREAD;

	if (spread < hi_lvl) { /* can not group */

		sendf(victim, "%s appears to want to ask you to join the group, but doesn't.\n", GET_NAME (ch));
		sendf (ch, "You do not think %s would like to join your group.\n", GET_NAME (victim));
		return;
	}

	act("$n is now a member of $N's group.", FALSE, victim, 0, ch, TO_ROOM);
	act("You are now a member of $N's group.", FALSE, victim, 0, ch, TO_CHAR);
	set_bit(victim->specials.affected_by, AFF_GROUP);
}


void do_quaff(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *temp;
  int i;
  bool equipped;
  
  equipped = FALSE;
  
  only_argument(argument,buf);
  
  if (!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
    temp = ch->equipment[HOLD];
    equipped = TRUE;
    if ((temp==0) || !isname(buf, temp->name)) {
      act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  }
  
  if (!IS_IMMORTAL(ch)) {
    if (GET_COND(ch,FULL)>20) {
      act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR);
      return;
    } else {
      GET_COND(ch, FULL)+=1;
    }
  }
  
  if (temp->obj_flags.type_flag!=ITEM_POTION) {
    act("You can only quaff potions.",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  if (!IS_IMMORTAL(ch) && ROOM_FLAG(ch->in_room,NO_MAGIC)) {
    sendf(ch,"Something about this place seems to disappate your magic before you can start.\n");
    return;
  }

  if (IsClass(ch,BARBARIAN)) {
    sendf(ch,"MAGIC BAD! DESTROY! DESTROY!\n");
    act("$n is throw into a frenzy at the thought of using magic.\n",
		FALSE,ch,0,0,TO_ROOM);
    do_junk(ch,buf,-1);
    return;
  }
  
  act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
  act("You quaff $p which dissolves.",FALSE, ch, temp,0, TO_CHAR);
  
  /*  my stuff */
  if (ch->specials.fighting) {
    if (equipped) {
      if (number(1,20) > ch->abilities.dex) {
	act("$n is jolted and drops $p!  It shatters!", 
	    TRUE, ch, temp, 0, TO_ROOM);
	act("You arm is jolted and $p flies from your hand, *SMASH*",
	    TRUE, ch, temp, 0, TO_CHAR);
	if (equipped)
	  temp = unequip_char(ch, HOLD);
	extract_obj(temp);
	return;
      }
    } else {
      if (number(1,20) > ch->abilities.dex - 4) {
	act("$n is jolted and drops $p!  It shatters!", 
	    TRUE, ch, temp, 0, TO_ROOM);
	act("You arm is jolted and $p flies from your hand, *SMASH*",
	    TRUE, ch, temp, 0, TO_CHAR);
	extract_obj(temp);
	return;
      }
    }
  }
  
  for (i=1; i<4; i++)
    if (temp->obj_flags.value[i] >= 1)
      if (spell_info[temp->obj_flags.value[i]].spell_pointer)
	((*spell_info[temp->obj_flags.value[i]].spell_pointer)
	 (temp->obj_flags.value[0], ch, "", SPELL_TYPE_POTION, ch, temp,temp->obj_flags.value[0]));
  
  if (equipped)
    temp = unequip_char(ch, HOLD);
  
  extract_obj(temp);
  
  WAIT_STATE(ch, PULSE_VIOLENCE);
  
}


void do_recite(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *scroll, *obj;
  struct char_data *victim;
  int i, bits;
  bool equipped;
  
  equipped = FALSE;
  obj = 0;
  victim = 0;
  
  argument = one_argument(argument,buf);
  
  if (!(scroll = get_obj_in_list_vis(ch,buf,ch->carrying))) {
    scroll = ch->equipment[HOLD];
    equipped = TRUE;
    if ((scroll==0) || !isname(buf, scroll->name)) {
      act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  }
  
  if (scroll->obj_flags.type_flag!=ITEM_SCROLL)  {
    act("Recite is normally used for scrolls.",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  if (*argument) {
    bits = generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM |
			FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &victim, &obj);
    if (bits == 0) {
      send_to_char("No such thing around to recite the scroll on.\n", ch);
      return;
    }
  } else {
    victim = ch;
  }
  
  if (!IS_IMMORTAL(ch) && ROOM_FLAG(ch->in_room,NO_MAGIC)) {
    sendf(ch,"Something about this place seems to disappate your magic before you can start.\n");
    return;
  }

  if (IsClass(ch,BARBARIAN) && GetMaxLevel(ch)<(400/GET_INT(ch))) {
    sendf(ch,"MAGIC BAD! DESTROY! DESTROY!\n");
    act("$n is throw into a frenzy at the thought of using magic.\n",
		FALSE,ch,0,0,TO_ROOM);
    do_junk(ch,buf,-1);
    return;
  }

  if (IsClass(ch,BARBARIAN) && number(1,100)<21-GET_INT(ch)) {
    sendf(ch,"Frustrated at your inability to read, but destroy the scroll.\n");
    act("$n is throw into a frenzy by his inability to read.\n",
		FALSE,ch,0,0,TO_ROOM);
    do_junk(ch,buf,-1);
    return;
  }

  if (!IsClass(ch, MAGE) && !IsClass(ch,ILLUSIONIST) &&
      !IsClass(ch, CLERIC)) {

    if (get_skill(ch,SKILL_READ_MAGIC) < number(1,101)) {
      if (scroll->obj_flags.value[1] != SPELL_WORD_OF_RECALL) {
	send_to_char("You can't understand this...\n",ch);
	return;
      }
    }
  }
  
  act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
  act("You recite $p which bursts into flame.",FALSE,ch,scroll,0,TO_CHAR);
  
  for (i=1; i<4; i++)
    if (scroll->obj_flags.value[i] >= 1) {
      if (IS_SET(spell_info[scroll->obj_flags.value[i]].targets, TAR_VIOLENT) &&
	  check_peaceful(ch, "Impolite magic is banned here."))
	continue;
      ((*spell_info[scroll->obj_flags.value[i]].spell_pointer)
       (scroll->obj_flags.value[0], ch, "", SPELL_TYPE_SCROLL, victim, obj,scroll->obj_flags.value[0]));
    }
  if (equipped)
    scroll = unequip_char(ch, HOLD);
  
  extract_obj(scroll);
}



void do_use(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct char_data *tmp_char;
  struct obj_data *tmp_object, *stick;
  
  int bits;

  argument = one_argument(argument,buf);
  
  if (ch->equipment[HOLD] == 0 ||
      !isname(buf, ch->equipment[HOLD]->name)) {
    act("You do not hold that item in your hand.",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  stick = ch->equipment[HOLD];
  
  if (ROOM_FLAG(ch->in_room,NO_MAGIC)) {
	sendf(ch,"Magic isn't allowed here.\n");
	return;
  }
  if (stick->obj_flags.type_flag == ITEM_STAFF)  {
    act("$n taps $p three times on the ground.",TRUE, ch, stick, 0,TO_ROOM);
    act("You tap $p three times on the ground.",FALSE,ch, stick, 0,TO_CHAR);
    if (stick->obj_flags.value[2] > 0) {  /* Is there any charges left? */
      stick->obj_flags.value[2]--;
      ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
       (stick->obj_flags.value[0], ch, "", SPELL_TYPE_STAFF, 0, 0,stick->obj_flags.value[0]));
      WAIT_STATE(ch, PULSE_VIOLENCE);
    } else {
      send_to_char("The staff seems powerless.\n", ch);
    }
  } else if (stick->obj_flags.type_flag == ITEM_WAND) {
    bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV | 
			FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
    if (bits) {
      struct spell_info_type	*spellp;

      spellp = spell_info + (stick->obj_flags.value[3]);

      if (bits == FIND_CHAR_ROOM) {
	act("$n point $p at $N.", TRUE, ch, stick, tmp_char, TO_ROOM);
	act("You point $p at $N.",FALSE,ch, stick, tmp_char, TO_CHAR);
      } else {
	act("$n point $p at $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
	act("You point $p at $P.",FALSE,ch, stick, tmp_object, TO_CHAR);
      }

      if (IS_SET(spellp->targets, TAR_VIOLENT) &&
	  check_peaceful(ch, "Impolite magic is banned here."))
	return;
      
      if (stick->obj_flags.value[2] > 0) { /* Is there any charges left? */
	stick->obj_flags.value[2]--;
	((*spellp->spell_pointer)
	 (stick->obj_flags.value[0], ch, "", SPELL_TYPE_WAND, 
	  tmp_char, tmp_object,stick->obj_flags.value[0]));
	WAIT_STATE(ch, PULSE_VIOLENCE);
      } else {
	send_to_char("The wand seems powerless.\n", ch);
      }
    } else {
      send_to_char("What should the wand be pointed at?\n", ch);
    }
  } else {
    send_to_char("Use is normally only for wand's and staff's.\n", ch);
  }
}



void do_plr_noshout(struct char_data *ch, char *argument, int cmd)
{
  char buf[128];
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, buf);
  
  if (!*buf) {
    if (IS_SET(ch->specials.act, PLR_DEAF)) {
      send_to_char("You can now hear shouts again.\n", ch);
      CLEAR_BIT(ch->specials.act, PLR_DEAF);
    } else {
      send_to_char("From now on, you won't hear shouts.\n", ch);
      SET_BIT(ch->specials.act, PLR_DEAF);
    }
  } else {
    send_to_char("Only the gods can shut up someone else. \n",ch);
  }
}

static void set_corrupt(struct char_data *ch)
{
  ch->user_data=0;
  if (ch->act_ptr==0) {
	sendf(ch,"So be it.\n");
	return;
  }
  ch->act_ptr=0;
  SET_BIT(ch->specials.act,PLR_CORRUPT);
  GET_TRUE_ALIGN(ch)=GET_ALIGNMENT(ch);
  if (GET_TRUE_ALIGN(ch)==1000)
	sendf(ch,"You have choosen the path of lawfulness and goodness.\nStay on that path, and do not stray from it.  And most of all, be wary of\nthose who will try to lead you from the path.  They will try to\ncorrupt you, but you must be stronger than that.  Now, go, and bring a\ngreate sense of lawfullness to this world, and those around you.\n");
  else
	sendf(ch,"You have choosen the path of corruption.  There are some people who will try\nto convert you, saying you are evil.  Do not believe them.  Most of all,\nkill the suckers!  Put yourself above all others, and create some havoc!\n");
}


void do_choose(struct char_data *ch, char *arg, int cmd)
{
  char choice[MAX_INPUT_LENGTH];

  if (IS_NPC(ch) || !ch->desc) return;
  arg=one_argument(arg,choice);
  if (!choice || !(is_abbrev(choice,"laws") ||
		is_abbrev(choice,"corruption"))) {
	sendf(ch,"To choose your path in life, you must 'choose laws' or 'choose corruption'.\n");
	return;
  }
  if (GetMaxLevel(ch) < 10) {
	sendf(ch,"You are not ready to choose your way of life.\n");
	return;
  }
  if (IS_SET(ch->specials.act,PLR_CORRUPT)) {
	sendf(ch,"You have already choosen a way of life.\n");
	return;
  }
  if (is_abbrev(choice,"laws")) {
	if (GET_ALIGNMENT(ch)<1000) {
		sendf(ch,"You aren't good enough.\n");
		return;
	}
	sendf(ch,"By choosing the path of laws, you are affirming the need for laws, and you\npromise to help those in need.  Once on this path, you may not leave it.\n");
	pre_CONFIRM_GET(ch->desc);
	ch->user_data=set_corrupt;
	return;
  }
  if (GET_ALIGNMENT(ch)> -1000) {
	  sendf(ch,"You aren't evil enough.\n");
	  return;
  }
  sendf(ch,"By choosing the path of corruption, you are affirming the need for corruption\nin the system.  Once on this path, you man not leave it.\n");
  pre_CONFIRM_GET(ch->desc);
  ch->user_data=set_corrupt;
}
