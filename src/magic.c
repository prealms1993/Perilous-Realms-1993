/* ************************************************************************
*  file: magic.c , Implementation of spells.              Part of DIKUMUD *
*  Usage : The actual effect of magic.                                    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <assert.h>

#include <fcns.h>
#include <rooms.h>
#include <utils.h>
#include <comm.h>
#include <spells.h>
#include <handler.h>
#include <limits.h>
#include <db.h>
#include <externs.h>
#include <race.h>
#include <ticks.h>

typedef struct ll {
  struct char_data *p;
  struct ll *next;
} ll;

/* Extern structures */
extern struct hash_header room_db;
extern struct obj_data  *object_list;
extern struct char_data *character_list;

/* Extern procedures */

#define ELVL20 MIN(eff_level,20)

/* Offensive Spells */
void spell_nova(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  ll *targets = NULL, *new, *curr, *next;
  int dam, i;
  struct char_data *mob;
  struct affected_type af;

  assert(ch);

  for(mob = real_roomp(ch->in_room)->people; mob; mob = mob->next_in_room){
    if(!IS_AFFECTED(mob, AFF_BLIND) && AWAKE(mob)){
      CREATE(new, ll, 1);
      new->p = mob;
      new->next = targets;
      targets = new;
    }
  }
  for(i = 0; i < 6; i++){
    if(EXIT(ch, i)){
      if(EXIT(ch, i)->to_room != NOWHERE){
        for(mob = real_roomp(EXIT(ch, i)->to_room)->people; mob;
            mob = mob->next_in_room){
          if(!IS_AFFECTED(mob, AFF_BLIND) && AWAKE(mob)){
            CREATE(new, ll, 1);
            new->p = mob;
            new->next = targets;
            targets = new;
          }
        }
      }
    }
  }

  sendf(ch, "You cross your arms and throw out your hands.\n");

  for(curr = targets; curr; curr = next){
    mob = curr->p;
    next = curr->next;
    dam = dice(ELVL20, 8);
    if((mob->in_room == ch->in_room) && (mob != ch)){
      act("$n explodes in a burst of blinding light!", TRUE, ch, 0, mob,
        TO_VICT);
    }
    else{
      sendf(mob, "A searing light suddenly blazes around you!\n");
    }
    if(!saves_spell(mob, SAVING_SPELL)){
      send_to_char("The light leaves you blind!\n", mob);
      af.type     = SPELL_BLINDNESS;
      af.location = APPLY_HITROLL;
      af.modifier = -4;  /* Make hitroll worse */
      af.duration =  4;
      init_set(af.bitvector,AFF_BLIND);
      affect_to_char(mob, &af);
      af.location = APPLY_ALL_AC;
      af.modifier = 20; /* Make AC Worse! */
      affect_to_char(mob, &af);
      damage(ch, mob, dam, SPELL_NOVA);
    }
    else{
      damage(ch, mob, dam >> 1, SPELL_NOVA);
    }
    FREE(curr);
  }
}

void spell_magic_missile(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(ELVL20,4);

  if (affected_by_spell(victim,SPELL_SHIELD))
    dam = 0;

  damage(ch, victim, dam, SPELL_MAGIC_MISSILE);
}



void spell_chill_touch(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  int dam;

  dam = 2 * dice(1, 8);

  assert(victim && ch);

  if ( !saves_spell(victim, SAVING_SPELL) )
  {
    af.type      = SPELL_CHILL_TOUCH;
    af.duration  = 6;
    af.modifier  = -1;
    af.location  = APPLY_STR;
    init_bits(af.bitvector);
    affect_join(victim, &af, TRUE, FALSE);
  } else {
    dam >>= 1;
  }
  damage(ch, victim, dam, SPELL_CHILL_TOUCH);
}

void spell_burning_hands(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;
  struct char_data *tmp_victim, *temp;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(ELVL20,3);

  send_to_char("Searing flame fans out in front of you!\n", ch);
  act("$n sends a fan of flame shooting from the fingertips!\n",
	  FALSE, ch, 0, 0, TO_ROOM);

   for ( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
	tmp_victim = temp) {
      temp = tmp_victim->next_in_room;
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
         if ((GetMaxLevel(tmp_victim)>LOW_IMMORTAL) && (!IS_NPC(tmp_victim)))
            return;
         if (!in_group(ch, tmp_victim)) {
            act("You are seared by the burning flame!\n",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
            if ( saves_spell(tmp_victim, SAVING_SPELL) )
                dam >>= 1;
   	    damage(ch, tmp_victim, dam, SPELL_BURNING_HANDS);
	 } else {
            act("You are able to avoid the flames!\n",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
	  }
       }
    }
}



void spell_shocking_grasp(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(ELVL20,6);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_SHOCKING_GRASP);
}



void spell_lightning_bolt(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(ELVL20,8);
  dam = MIN(200, dam);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_LIGHTNING_BOLT);
}



void spell_colour_spray(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

	assert(victim && ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(ELVL20*3,10);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_COLOUR_SPRAY);

}


/* Drain XP, MANA, HP - caster gains HP and MANA */
void spell_energy_drain(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <=  ABS_MAX_LVL));
  if (ROOM_FLAG(ch->in_room,ARENA)) {
    sendf(ch,"Not here.\n");
    return;
  }

  if ( !saves_spell(victim, SAVING_SPELL) ) {
    GET_ALIGNMENT(ch) = MIN(-1000, GET_ALIGNMENT(ch)-200);

    if (GetMaxLevel(victim) <= 1) {
      damage(ch, victim, 100, SPELL_ENERGY_DRAIN); /* Kill the sucker */
    } else if ((!IS_NPC(victim)) && (GetMaxLevel(victim) >= LOW_IMMORTAL)) {
      send_to_char("Some puny mortal just tried to drain you...\n",victim);
    } else {
      if (!IS_SET(victim->specials.M_immune, IMM_DRAIN)) {

         send_to_char("Your life energy is drained!\n", victim);
	 dam = 1;
	 if (!IS_NPC(victim)) {
            damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
	    drop_level(victim);
	  } else {
            dam = dice(level,8);  /* nasty spell */
	    dam = MIN(400, dam);
            damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
	  }
      } else {
	 if (!IS_SET(ch->specials.M_immune, IMM_DRAIN)) {
	    send_to_char("Your spell backfires!\n",ch);
	    if (!IS_NPC(ch)) {
	       drop_level(ch);
	     } else {
               dam = dice(level,8);  /* nasty spell */
	       dam = MIN(400, dam);
               damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
	     }
         } else {
	   send_to_char("Your spell fails utterly.\n",ch);
	 }
       }

     }
  } else {
    damage(ch, victim, 0, SPELL_ENERGY_DRAIN); /* Miss */
  }
}



void spell_fireball(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;
  struct char_data *tmp_victim, *temp;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(ELVL20*4,8);
/*
  this one should be in_world, not in room, so that the message can
  be sent to everyone.
*/

   for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
      temp = tmp_victim->next;
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
         if (!in_group(ch,tmp_victim)) {

            if ((GetMaxLevel(tmp_victim)>=LOW_IMMORTAL)&&(!IS_NPC(tmp_victim))) {
               send_to_char("Some puny mortal tries to toast you with a fireball",tmp_victim);
               return;
            } else {
               if ( saves_spell(tmp_victim, SAVING_SPELL) )
                   dam >>= 1;
                   damage(ch, tmp_victim, dam, SPELL_FIREBALL);
	     }
	 } else {
            act("You dodge the mass of flame!!\n",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
	 }
      } else {
      if (real_roomp(ch->in_room)->zone == real_roomp(tmp_victim->in_room)->zone)
            send_to_char("You feel a blast of hot air.\n", tmp_victim);
      }
   }
}


void spell_earthquake(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  struct char_data *tmp_victim, *temp;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

	dam =  dice(1,4) + level + 1;
	dam = MIN(100, dam);

  send_to_char("The earth trembles beneath your feet!\n", ch);
  act("$n makes the earth tremble and shiver\n",
	  FALSE, ch, 0, 0, TO_ROOM);


   for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
      temp = tmp_victim->next;
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim))
         if (!in_group(ch,tmp_victim)) {

            if ((GetMaxLevel(tmp_victim)<LOW_IMMORTAL)||(IS_NPC(tmp_victim))) {
      	       damage(ch, tmp_victim, dam, SPELL_EARTHQUAKE);
               act("You fall and hurt yourself!!\n",
                   FALSE, ch, 0, tmp_victim, TO_VICT);
	    }

	 } else {
            act("You almost fall and hurt yourself!!\n",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
      } else {
         if (real_roomp(ch->in_room)->zone == real_roomp(tmp_victim->in_room)->zone)
            send_to_char("The earth trembles...", tmp_victim);
      }
   }
}



void spell_dispel_evil(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{

	assert(ch && victim);
	assert((level >= 1) && (level<=ABS_MAX_LVL));


  if (IsExtraPlanar(victim)) {
	if (IS_EVIL(ch)) {
		victim = ch;
	} else {
	  if (IS_GOOD(victim)) {
	     act("Good protects $N.", FALSE, ch, 0, victim, TO_CHAR);
  	     return;
	  }
	}
	  if (!saves_spell(victim, SAVING_SPELL) ) {
	    act("$n forces $N from this plane.", TRUE, ch, 0, victim, TO_ROOM);
	    act("You force $N from this plane.", TRUE, ch, 0, victim, TO_CHAR);
	    act("$n forces you from this plane.", TRUE, ch, 0, victim,TO_VICT);
	    gain_exp(ch, MIN(GET_EXP(victim)/2, 50000));
	    extract_char(victim);
	  }
    } else {
	act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
	act("$N laughs at $n.", TRUE,ch, 0, victim, TO_NOTVICT);
	act("You laugh at $n.", TRUE,ch,0,victim,TO_VICT);
    }
}


void spell_call_lightning(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  extern struct weather_data weather_info;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

	dam = dice(MAX(level,15), 6);
	dam = MIN(300, dam);

  if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {

	  if ( saves_spell(victim, SAVING_SPELL) )
  	  dam >>= 1;

  	damage(ch, victim, dam, SPELL_CALL_LIGHTNING);
	}
}



void spell_harm(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

	assert(victim && ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	dam = GET_HIT(victim) - dice(1,4);

	if (dam < 0)
		dam = 100; /* Kill the suffering bastard */
	else {
	  if ( saves_spell(victim, SAVING_SPELL) )
       		dam = 0;
	}

  dam = MIN(dam,100);

  damage(ch, victim, dam, SPELL_HARM);
}



/* spells2.c - Not directly offensive spells */

void spell_haven(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

	if (!affected_by_spell(victim, SPELL_ARMOR)) {
	  af.type      = SPELL_HAVEN;
    	  af.duration  = 5;
	  af.modifier  = -5;
	  af.location  = APPLY_ALL_AC;
	  init_bits(af.bitvector);
	  affect_to_char(victim, &af);
	  af.modifier = 2;
	  af.location = APPLY_BODY_STOPPING;
	  affect_to_char(victim, &af);

  	  send_to_char("A warm feeling spreads through your body.\n", victim);
	} else {
	  send_to_char("A beautiful song, but it doesn't seem to have any affect.n", ch);
	}
}

void spell_armor(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

	if (!affected_by_spell(victim, SPELL_ARMOR)) {
	  af.type      = SPELL_ARMOR;
    	  af.duration  = 24;
	  af.modifier  = -10;
	  af.location  = APPLY_BODY_AC;
	  init_bits(af.bitvector);

	  affect_to_char(victim, &af);
		send_to_char("You feel someone protecting you.\n", victim);
	} else {
	  send_to_char("Nothing New seems to happen\n", ch);
	}
}

struct room_data *spell_char_to_mob (int location, int success, struct char_data *ch)
{
int bottom = 0;
int top = 0;
int new_loc = 0;
int random = 0;
int start = 0;
int percent;
struct room_data *rp;
extern struct zone_data *zone_table;

	percent = success / 2;
	if (is_thief (ch) || is_fighter (ch))
		percent -= 20;
	if (dice (1, 101) < percent)
		return (real_roomp (location));

  	rp = real_roomp(location);
  	top = zone_table[rp->zone].top;
  	bottom = (!rp->zone ? 0 : zone_table[rp->zone - 1].top);
  	random = dice (20, 10);
  	start = (dice (1, 2) == 1 ? random : -random) + location;
	if (start < bottom || start > top)
		start = bottom;
	
	new_loc = start;
	while (1) {
		rp = real_roomp (new_loc);

		if (!IS_SET(rp->room_flags,  PRIVATE) &&
		((IS_SET(rp->room_flags,EVIL_ONLY) && IS_EVIL(ch)) ||
		(IS_SET(rp->room_flags,NEUTRAL_ONLY) && IS_NEUTRAL(ch)) ||
		(IS_SET(rp->room_flags,GOOD_ONLY) && IS_GOOD(ch))) &&
		!IS_SET (rp->room_flags, DEATH) &&
	   	   !IS_SET(rp->room_flags,  ARENA) &&
	   	   !IS_SET(rp->room_flags,  LORD_ONLY) &&
	   	   !IS_SET(rp->room_flags,  NO_GOTO) &&
	   	   !IS_SET(rp->room_flags,  NO_SUM) &&
	   	   !IS_SET(rp->room_flags,  NO_MAGIC))
			break;

		if ((new_loc+1) == location)
			new_loc += 2;
		if (new_loc >= top)
			new_loc = bottom;
		else
			new_loc++;
		if (new_loc == start) { /* we've looped, bail and goto mob */
			rp = real_roomp (location);
			nlog ("going to mob locaton");
			break;
		}
	}
	if (IS_SET(rp->room_flags,  PRIVATE) ||
	   IS_SET(rp->room_flags,  DEATH) ||
	(IS_SET(rp->room_flags,EVIL_ONLY) && !IS_EVIL(ch)) ||
	(IS_SET(rp->room_flags,NEUTRAL_ONLY) && !IS_NEUTRAL(ch)) ||
	(IS_SET(rp->room_flags,GOOD_ONLY) && !IS_GOOD(ch)) ||
	   IS_SET(rp->room_flags,  ARENA) ||
	   IS_SET(rp->room_flags,  LORD_ONLY) ||
	   IS_SET(rp->room_flags,  NO_GOTO) ||
	   IS_SET(rp->room_flags,  NO_SUM) ||
	   IS_SET(rp->room_flags,  NO_MAGIC))  {
		return ((struct room_data *) NULL);
	}
	if (rp->number != location)
		send_to_char ("You feel yourself lose control of the spell...\n", ch);
	return (rp);
}

void spell_astral_walk(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct room_data *rp;

  assert(ch && victim);

  if (dice(1,8) == 8) {
    send_to_char("You failed.\n", ch);
    return;
  }


  rp = spell_char_to_mob (victim->in_room, get_spell (ch, SPELL_ASTRAL_WALK), ch);

  if (GetMaxLevel(victim) > MAX_MORT ||
      ROOM_FLAG(ch->in_room,  ARENA) ||
      !rp) { 
    send_to_char("You failed.\n", ch);
    return;
  }

    act("$n opens a door to another dimension and steps through!",FALSE,ch,0,0,TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, rp->number);
  act("You are blinded for a moment as $n appears in a flash of light!",FALSE,ch,0,0,TO_ROOM);
  do_look(ch, "",15);
}

void spell_teleport(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
	int to_room;
	extern int top_of_world;      /* ref to the top element of world */
	struct room_data *room;

	assert(ch && victim);

        if (victim != ch) {
           if (saves_spell(victim,SAVING_SPELL)) {
              send_to_char("Your spell has no effect.\n",ch);
              if (IS_NPC(victim)) {
                 if (!victim->specials.fighting)
                    set_fighting(victim, ch);
              } else {
                 send_to_char("You feel strange, but the effect fades.\n",victim);
	      }
              return;
           } else {
             ch = victim;  /* the character (target) is now the victim */
           }
	}

	do {
		to_room = number(0, top_of_world);
		room = real_roomp(to_room);
		if (room) {
		  if (IS_SET(room->room_flags, PRIVATE) ||
                      IS_SET(room->room_flags, ARENA) ||
                      IS_SET(room->room_flags, NO_GOTO))
		    room = 0;
		}

	} while (!room);

        act("$n slowly fade out of existence.", FALSE, ch,0,0,TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, to_room);
        act("$n slowly fade in to existence.", FALSE, ch,0,0,TO_ROOM);

	do_look(ch, "", 0);

	if (IS_SET(real_roomp(to_room)->room_flags, DEATH) &&
	    GetMaxLevel(ch) < LOW_IMMORTAL) {
			death_trap (ch);
        }
}



void spell_bless(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(ch && (victim || obj));
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if (obj) {
    if ( (5*GET_LEVEL(ch) > GET_OBJ_WEIGHT(obj)) &&
	(GET_POS(ch) != POSITION_FIGHTING) &&
	!IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_BLESS);
      act("$p briefly glows.",FALSE,ch,obj,0,TO_CHAR);
    }
  } else {

    if ((GET_POS(victim) != POSITION_FIGHTING) &&
	(!affected_by_spell(victim, SPELL_BLESS))) {

      send_to_char("You feel righteous.\n", victim);
      af.type      = SPELL_BLESS;
      af.duration  = 6;
      af.modifier  = 1;
      af.location  = APPLY_HITROLL;
      init_bits(af.bitvector);
      affect_to_char(victim, &af);

      af.location = APPLY_SAVING_SPELL;
      af.modifier = -1;                 /* Make better */
      affect_to_char(victim, &af);
    }
  }
}



void spell_blindness(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(ch && victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));


  if (saves_spell(victim, SAVING_SPELL) ||
	   affected_by_spell(victim, SPELL_BLINDNESS))
		return;

  act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
  send_to_char("You have been blinded!\n", victim);

  af.type      = SPELL_BLINDNESS;
  af.location  = APPLY_HITROLL;
  af.modifier  = -4;  /* Make hitroll worse */
  af.duration  =  2;
  init_set(af.bitvector,AFF_BLIND);
  affect_to_char(victim, &af);

  af.location = APPLY_ALL_AC;
  af.modifier = 20; /* Make AC Worse! */
  affect_to_char(victim, &af);

  if ((!victim->specials.fighting)&&(victim!=ch))
     set_fighting(victim,ch);

}



void spell_clone(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{

  assert(ch && (victim || obj));
  assert((level >= 0) && (level <= ABS_MAX_LVL));

	send_to_char("Clone is not ready yet.", ch);

  if (obj) {

	} else {
		/* clone_char(victim); */
	}
}



void spell_control_weather(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
   /* Control Weather is not possible here!!! */
   /* Better/Worse can not be transferred     */
}



void spell_create_food(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct obj_data *tmp_obj;

  assert(ch);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  tmp_obj->name = mystrdup("mushroom");
  tmp_obj->short_description = mystrdup("A Magic Mushroom");
  tmp_obj->description = mystrdup("A really delicious looking magic mushroom lies here.");

  tmp_obj->obj_flags.type_flag = ITEM_FOOD;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[0] = 5+level;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 10;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_room(tmp_obj,ch->in_room);

	act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
	act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);
}



void spell_create_water(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
	int water;

  extern struct weather_data weather_info;

  assert(ch && obj);

	if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
		if ((obj->obj_flags.value[2] != LIQ_WATER)
		     && (obj->obj_flags.value[1] != 0)) {

			name_from_drinkcon(obj);
			obj->obj_flags.value[2] = LIQ_SLIME;
			name_to_drinkcon(obj, LIQ_SLIME);

		} else {

			water = 2*level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);

			/* Calculate water it can contain, or water created */
			water = MIN(obj->obj_flags.value[0]-obj->obj_flags.value[1], water);

			if (water > 0) {
			  obj->obj_flags.value[2] = LIQ_WATER;
				obj->obj_flags.value[1] += water;

				weight_change_object(obj, water);

				name_from_drinkcon(obj);
				name_to_drinkcon(obj, LIQ_WATER);
				act("$p is partially filled.", FALSE, ch,obj,0,TO_CHAR);
			}
		}
	}
}



void spell_cure_blind(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

	if (affected_by_spell(victim, SPELL_BLINDNESS)) {
	  affect_from_char(victim, SPELL_BLINDNESS);

	  send_to_char("Your vision returns!\n", victim);
	}
}



void spell_cure_critic(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int healpoints;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  healpoints = dice(4,8)+3;

  if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  send_to_char("You feel better!\n", victim);

  update_pos(victim);
}


void spell_cure_light(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int healpoints;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  healpoints = dice(1,8);

  if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  send_to_char("You feel better!\n", victim);

  update_pos(victim);


}





void spell_curse(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(victim || obj);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if (obj) {
    SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
    SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);

    /* LOWER ATTACK DICE BY -1 */
    if(obj->obj_flags.type_flag == ITEM_WEAPON)
      obj->obj_flags.value[2]--;
		act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
	} else {
    if ( saves_spell(victim, SAVING_SPELL) ||
		   affected_by_spell(victim, SPELL_CURSE))
      return;

    af.type      = SPELL_CURSE;
    af.duration  = 24*7;       /* 7 Days */
    af.modifier  = -1;
    af.location  = APPLY_HITROLL;
    init_set(af.bitvector,AFF_CURSE);
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_PARA;
    af.modifier = 1; /* Make worse */
    affect_to_char(victim, &af);

    act("$n briefly reveal a red aura!", FALSE, victim, 0, 0, TO_ROOM);
    act("You feel very uncomfortable.",FALSE,victim,0,0,TO_CHAR);
    if (IS_NPC(victim) && !victim->specials.fighting)
       set_fighting(victim,ch);
   }
}



void spell_detect_evil(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

	assert(victim);
	assert((level >= 0) && (level <= ABS_MAX_LVL));

	if ( affected_by_spell(victim, SPELL_DETECT_EVIL) )
		return;

  af.type      = SPELL_DETECT_EVIL;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  init_set(af.bitvector,AFF_DETECT_EVIL);

  affect_to_char(victim, &af);

  act("$n's eyes briefly glow white", FALSE, victim, 0, 0, TO_ROOM);
  send_to_char("Your eyes tingle.\n", victim);
}



void spell_detect_invisibility(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(ch && victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if ( affected_by_spell(victim, SPELL_DETECT_INVISIBLE) )
		return;

  af.type      = SPELL_DETECT_INVISIBLE;
  af.duration  = 15;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  init_set(af.bitvector,AFF_DETECT_INVISIBLE);

  affect_to_char(victim, &af);
  act("$n's eyes briefly glow yellow", FALSE, victim, 0, 0, TO_ROOM);
  send_to_char("Your eyes tingle.\n", victim);
}



void spell_detect_magic(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if ( affected_by_spell(victim, SPELL_DETECT_MAGIC) )
		return;

  af.type      = SPELL_DETECT_MAGIC;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  init_set(af.bitvector,AFF_DETECT_MAGIC);

  affect_to_char(victim, &af);
  send_to_char("Your eyes tingle.\n", victim);
}



void spell_detect_poison(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
	assert(ch && (victim || obj));

  if (victim) {
    if (victim == ch)
      if (IS_AFFECTED(victim, AFF_POISON))
        send_to_char("You can sense poison in your blood.\n", ch);
      else
        send_to_char("You feel healthy.\n", ch);
    else
      if (IS_AFFECTED(victim, AFF_POISON)) {
        act("You sense that $E is poisoned.",FALSE,ch,0,victim,TO_CHAR);
      } else {
        act("You sense that $E isn't poisoned",FALSE,ch,0,victim,TO_CHAR);
      }
  } else { /* It's an object */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      if (obj->obj_flags.value[3])
        act("Poisonous fumes are revealed.",FALSE, ch, 0, 0, TO_CHAR);
      else
        send_to_char("It looks very delicious.\n", ch);
    }
  }
}



void spell_enchant_weapon(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
	int i;

	assert(ch && obj);
	assert(MAX_OBJ_AFFECT >= 2);

	if ((GET_ITEM_TYPE(obj) == ITEM_WEAPON) &&
	   !IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC) &&
           !IS_SET(obj->obj_flags.extra_flags, ITEM_MUNDANE)) {

		for (i=0; i < MAX_OBJ_AFFECT; i++)
			if (obj->affected[i].location != APPLY_NONE)
				return;

		SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

		obj->affected[0].location = APPLY_HITROLL;
		obj->affected[0].modifier = 1;
		if (level > 20)
  		   obj->affected[0].modifier += 1;
		if (level > 40)
   		   obj->affected[0].modifier += 1;
		if (level > MAX_MORT)
   		   obj->affected[0].modifier += 1;


		obj->affected[1].location = APPLY_DAMROLL;
		obj->affected[1].modifier = 1;
		if (level > 15)
  		   obj->affected[1].modifier += 1;
		if (level > 30)
   		   obj->affected[1].modifier += 1;
		if (level > MAX_MORT)
   		   obj->affected[1].modifier += 1;

		if (IS_GOOD(ch)) {
			SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
			act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);
		} else if (IS_EVIL(ch)) {
                        SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
                        act("$p glows red.",FALSE,ch,obj,0,TO_CHAR);
                } else {
                        act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
		}
	}
}



void spell_heal(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
	assert(victim);

	spell_cure_blind(level, ch, victim, obj,eff_level);

	GET_HIT(victim) += 150;

	if (GET_HIT(victim) >= hit_limit(victim))
		GET_HIT(victim) = hit_limit(victim)-dice(1,4);

  update_pos( victim );

  send_to_char("A warm feeling fills your body.\n", victim);
}


void spell_invisibility(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

	assert((ch && obj) || victim);

  if (obj) {
    if ( !IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE) ) {
			act("$p turns invisible.",FALSE,ch,obj,0,TO_CHAR);
			act("$p turns invisible.",TRUE,ch,obj,0,TO_ROOM);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
		}
  } else {              /* Then it is a PC | NPC */
		if (!affected_by_spell(victim, SPELL_INVISIBLE)) {

		  act("$n slowly fades out of existence.", TRUE, victim,0,0,TO_ROOM);
  	  send_to_char("You vanish.\n", victim);

	    af.type      = SPELL_INVISIBLE;
    	af.duration  = 24;
    	af.modifier  = -40;
  	  af.location  = APPLY_ALL_AC;
	    init_set(af.bitvector,AFF_INVISIBLE);
  	  affect_to_char(victim, &af);
  	}
    }
}


void spell_locate_object(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct obj_data *i;
  char name[256];
  int j;

	assert(ch);

  strcpy(name,(char *)(void *)obj);

	j=level>>1;


	for (i = object_list; i && (j>0); i = i->next)
    if (isname(name, i->name)) {
      if(i->carried_by) {
	if (strlen(PERS(i->carried_by, ch))>0)
          sendf(ch,"%s carried by %s.\n",i->short_description,PERS(i->carried_by,ch));
      } else if(i->equipped_by) {
	if (strlen(PERS(i->equipped_by, ch))>0)
          sendf(ch,"%s equipped by %s.\n",i->short_description,PERS(i->equipped_by,ch));
      } else if (i->in_obj)
          sendf(ch,"%s in %s.\n",i->short_description, i->in_obj->short_description);
      else {
          sendf(ch,"%s in %s.\n",i->short_description,
		(i->in_room == NOWHERE ? "use but uncertain." : real_roomp(i->in_room)->name));
         j--;
      }
    }

  if(j==0)
    send_to_char("You are very confused.\n",ch);
  if(j==level>>1)
    send_to_char("No such object.\n",ch);
}


void spell_poison(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
	struct affected_type af;

	assert(victim || obj);

  if (victim) {
    if (IS_NPC(ch)) {
     if (!IS_SET(ch->specials.act, ACT_DEADLY)) {
      if(!ImpSaveSpell(victim, SAVING_PARA, 0))    {
       af.type = SPELL_POISON;
       af.duration = 12;
       af.modifier = -2;
       af.location = APPLY_STR;
       init_set(af.bitvector,AFF_POISON);

       affect_join(victim, &af, FALSE, FALSE);

       send_to_char("You feel very sick.\n", victim);
       if (!victim->specials.fighting)
	 set_fighting(victim, ch);
     } else {
       return;
     }
    } else {
      if (!ImpSaveSpell(victim, SAVING_PARA, 0)) {
	act("Deadly poison fills your veins.",TRUE, ch, 0, 0, TO_CHAR);
	damage(ch, victim, MAX(10, GET_HIT(victim)*2), SPELL_POISON);
      } else {
	return;
      }
    }
   } else {
      if(!ImpSaveSpell(victim, SAVING_PARA, 0))    {
       af.type = SPELL_POISON;
       af.duration = 12;
       af.modifier = -2;
       af.location = APPLY_STR;
       init_set(af.bitvector,AFF_POISON);

       affect_join(victim, &af, FALSE, FALSE);

       send_to_char("You feel very sick.\n", victim);
      }
    }
  } else { /* Object poison */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 1;
    }
  }
}


void spell_protection_from_evil(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

	assert(victim);

  if (!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL) ) {
    af.type      = SPELL_PROTECT_FROM_EVIL;
    af.duration  = 24;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    init_set(af.bitvector,AFF_PROTECT_EVIL);
    affect_to_char(victim, &af);
		send_to_char("You have a righteous feeling!\n", victim);
	}
}


void spell_remove_curse(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
	struct obj_data *objp;

	assert(ch && (victim || obj));

	if (obj) {

		if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
			act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);
			CLEAR_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
		}
	} else {      /* Then it is a PC | NPC */
		if (affected_by_spell(victim, SPELL_CURSE) ) {
			act("$n briefly glows red, then blue.",FALSE,victim,0,0,TO_ROOM);
			act("You feel better.",FALSE,victim,0,0,TO_CHAR);
      			affect_from_char(victim, SPELL_CURSE);
    		} else { /* see if player has cursed object */
			for (objp = victim->carrying; objp; objp = objp->next_content) {
				if (IS_SET (objp->obj_flags.extra_flags, ITEM_NODROP)) {
					act("$p briefly glows blue.", TRUE, ch, objp, 0, TO_CHAR);
					act("$p briefly glows blue.", TRUE, victim, objp, 0, TO_CHAR);
					CLEAR_BIT(objp->obj_flags.extra_flags, ITEM_NODROP);
					break;
				}
			}
		}
	}
}


void spell_remove_poison(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{

	assert(ch && (victim || obj));

  if (victim) {
    if(affected_by_spell(victim,SPELL_POISON)) {
      affect_from_char(victim,SPELL_POISON);
      act("A warm feeling runs through your body.",FALSE,victim,0,0,TO_CHAR);
			act("$N looks better.",FALSE,ch,0,victim,TO_ROOM);
    }
	} else {
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 0;
			act("The $p steams briefly.",FALSE,ch,obj,0,TO_CHAR);
		}
	}
}



void spell_fireshield(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  if (ROOM_FLAG(ch->in_room,ARENA)) {
    sendf(ch,"Fireshield is not allowed in the arena.\n");
    return;
  }

  if ((affected_by_spell(victim, SPELL_SANCTUARY)) || 
     (affected_by_spell(victim, SPELL_INVULNERABLE))) {
    sendf(ch,"Fireshield wont work with sanctuary or invulnerable!\n");
    return;
  }

  if (!affected_by_spell(victim, SPELL_FIRESHIELD) ) {

    act("$n is surrounded by a glowing red aura.",TRUE,victim,0,0,TO_ROOM);
		act("You start glowing red.",TRUE,victim,0,0,TO_CHAR);

    af.type      = SPELL_FIRESHIELD;
    af.duration  = dice(2, 10); /* (level<LOW_IMMORTAL) ? 3 : level; */
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    init_set(af.bitvector,AFF_FIRESHIELD);
    affect_to_char(victim, &af);
  }
}

void spell_sanctuary(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  if (IS_NPC (victim) && IS_AFFECTED (victim, AFF_SANCTUARY)) {
	do_say (victim, "Thanks, but I already have sanctuary.", 0);
	return;
  }
  if ((affected_by_spell(victim, SPELL_FIRESHIELD)) ||
     (affected_by_spell(victim, SPELL_INVULNERABLE))) {
    sendf(ch,"Sanctuary wont work with fireshield or invulnerable!\n");
    return;
  }

  if (!affected_by_spell(victim, SPELL_SANCTUARY) ) {

    act("$n is surrounded by a white aura.",TRUE,victim,0,0,TO_ROOM);
		act("You start glowing.",TRUE,victim,0,0,TO_CHAR);

    af.type      = SPELL_SANCTUARY;
    af.duration  = dice(2, 10);  /* (level<LOW_IMMORTAL) ? 3 : level; */
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    init_set(af.bitvector,AFF_SANCTUARY);
    affect_to_char(victim, &af);
  }
}




void spell_strength(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(victim);


  if (!affected_by_spell(victim,SPELL_STRENGTH)) {
     act("You feel stronger.", FALSE, victim,0,0,TO_CHAR);
     act("$n seems stronger!\n",
	  FALSE, victim, 0, 0, TO_ROOM);
     af.type      = SPELL_STRENGTH;
     af.duration  = MIN(50, 2*level);
     if (IS_NPC(victim))
        af.modifier = number(1,6);
     else {

       if (is_fighter(ch))
           af.modifier = number(1,8);
       else if (is_cleric(ch) || is_thief(ch))
           af.modifier = number(1,6);
       else
	 af.modifier = number(1,4);
     }
     af.location  = APPLY_STR;
     init_bits(af.bitvector);
     affect_to_char(victim, &af);
   } else {

  act("Nothing seems to happen.", FALSE, ch,0,0,TO_CHAR);

  }
}



void spell_ventriloquate(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
	/* Not possible!! No argument! */
}



void spell_word_of_recall(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int location;

	assert(victim);

  if (IS_NPC(victim))
     return;

  if (ch && IS_IMMORTAL(victim) && (ch != victim)) return;

  if (IN_ARENA(victim)) return;

  /*  loc_nr = GET_HOME(ch); */

  location = LOAD_ROOM_MORTAL;

  if (!real_roomp(location))    {
    send_to_char("You are completely lost.\n", victim);
    return;
  }

	/* a location has been found. */

  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, location);
  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  do_look(victim, "",15);

}

void spell_charm_monster(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(ch && victim);

  if (victim == ch) {                                                           
    send_to_char("You like yourself even better!\n", ch);                     
    return;                                                                     
  }

  if (IsPerson(victim)) {
    act("Your glib tongue only seems to enrage $N.", 0, ch, NULL, victim,
      TO_CHAR);
    if(IS_NPC(victim))
      hit(victim, ch, TYPE_UNDEFINED);
    return;
  }

  if(IS_AFFECTED(victim, AFF_NOCHARM)) {
    send_to_char("Your victim isn't too charmed at you ... oops!\n", ch);
    if(IS_NPC(victim))
      hit(victim, ch, TYPE_UNDEFINED);
    return;
  }

  if ((GET_LEVEL(ch)+10) < (GET_LEVEL(victim)) || (GET_HIT(victim) >=1000)) {
    sendf(ch,"%s smiles at you for a moment then frowns.\n", GET_NAME (victim));
    return;
    FailCharm(victim,ch);
   }
  
  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
    if (circle_follow(victim, ch)) {
      send_to_char("Sorry, following in circles can not be allowed.\n", ch);
      return;
    }
      if (IsImmune(victim, IMM_CHARM)) {
          FailCharm(victim,ch);
       	  return;
      }
      if (IsResist(victim, IMM_CHARM)) {
         if (saves_spell(victim, SAVING_PARA)) {
          FailCharm(victim,ch);
       	  return;
	 }

         if (saves_spell(victim, SAVING_PARA)) {
          FailCharm(victim,ch);
       	  return;
	 }
       } else {
          if (!IsSusc(victim, IMM_CHARM)) {
	     if (saves_spell(victim, SAVING_PARA)) {
	        FailCharm(victim,ch);
		return;
	     }
	  }
       }

    if(saves_spell(victim, SAVING_PARA)){
      FailCharm(victim, ch);
      return;
    }

    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type      = SPELL_CHARM_PERSON;

    if (GET_INT(victim))
      af.duration  = 24*18/GET_INT(victim);
    else
      af.duration  = 24;

    af.modifier  = 0;
    af.location  = 0;
    init_set(af.bitvector,AFF_CHARM);
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?",FALSE,ch,0,victim,TO_VICT);
  }
}

/* ***************************************************************************
 *                     Not cast-able spells                                  *
 * ************************************************************************* */


void spell_identify(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  char buf[MAX_STRING_LENGTH];
  int i;
  bool found;

  /* Spell Names */
  extern char *spells[];

  /* For Objects */
  extern char *item_types[];
  extern char *extra_bits[];
  extern char *apply_types[];


  assert(ch && (obj || victim));

  if (obj) {
    identify_new(ch, obj, 200); /* Remove these next to lines */
     return; /* To get the old identify back */

    sendf(ch,"You feel informed:\n");
    sendf(ch,"Object '%s', Item type: ", obj->name);
    sprinttype(GET_ITEM_TYPE(obj),item_types,buf);
    sendf(ch,"%s\n",buf);

/*
    if (obj->obj_flags.bitvector) {
      sendf(ch,"Item will give you following abilities:  ");
      sprintbit(obj->obj_flags.bitvector,affected_bits,buf);
      sendf(ch,"%s\n",buf);
    }
*/

    sendf(ch,"Item is: ");
    sprintbit( obj->obj_flags.extra_flags,extra_bits,buf);
    sendf(ch,"%s\n",buf);
    sendf(ch,"Weight: %d, Value: %d\n",
	    obj->obj_flags.weight, obj->obj_flags.cost);

    switch (GET_ITEM_TYPE(obj)) {
      case ITEM_SCROLL :
      case ITEM_POTION :
	sendf(ch, "Level %d spells of:\n",obj->obj_flags.value[0]);
	if (obj->obj_flags.value[1] >= 1) {
	  sprinttype(obj->obj_flags.value[1]-1,spells,buf);
	  sendf(ch,"%s\n",buf);
	}
	if (obj->obj_flags.value[2] >= 1) {
	  sprinttype(obj->obj_flags.value[2]-1,spells,buf);
	  sendf(ch,"%s\n",buf);
	}
	if (obj->obj_flags.value[3] >= 1) {
	  sprinttype(obj->obj_flags.value[3]-1,spells,buf);
	  sendf(ch,"%s\n",buf);
	}
      break;

    case ITEM_WAND :
    case ITEM_STAFF :
      sendf(ch,"Has %d chages, with %d charges left.\n",
	      obj->obj_flags.value[1],
	      obj->obj_flags.value[2]);

      sendf(ch,"Level %d spell of:\n",obj->obj_flags.value[0]);

      if (obj->obj_flags.value[3] >= 1) {
	sprinttype(obj->obj_flags.value[3]-1,spells,buf);
	sendf(ch,"%s\n",buf);
      }
      break;

    case ITEM_WEAPON :
      sendf(ch,"Damage Dice is '%dD%d'\n",
		obj->obj_flags.value[1], obj->obj_flags.value[2]);
      break;

    case ITEM_ARMOR :
      sendf(ch,"AC-apply is %d\n", obj->obj_flags.value[0]);
      break;

    }

    found = FALSE;

    for (i=0;i<MAX_OBJ_AFFECT;i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
	  (obj->affected[i].modifier != 0)) {
	if (!found) {
	  sendf(ch,"Can affect you as:\n");
	  found = TRUE;
	}

	sprinttype(obj->affected[i].location,apply_types,buf);
	sendf(ch,"    Affects : %s By %d\n", buf,obj->affected[i].modifier);
      }
    }

  } else {       /* victim */

    if (!IS_NPC(victim)) {
      sendf(ch,"%d Years,  %d Months,  %d Days,  %d Hours old.\n",
	      age(victim).year, age(victim).month,
	      age(victim).day, age(victim).hours);

      sendf(ch,"Height %dcm  Weight %dpounds \n",
	      GET_HEIGHT(victim), GET_WEIGHT(victim));
      sendf(ch,"Str %d,  Int %d,  Wis %d,  Dex %d,  Con %d\n",
	GET_STR(victim),
	GET_INT(victim),
	GET_WIS(victim),
	GET_DEX(victim),
	GET_CON(victim),
        GET_CHR(victim), GET_LCK(victim));
    } else {
      send_to_char("You learn nothing new.\n", ch);
    }
  }

}


/* ***************************************************************************
 *                     NPC spells..                                          *
 * ************************************************************************* */

void spell_fire_breath(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
	int dam;
	int hpch;

	assert(victim && ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	hpch = GET_MAX_HIT(ch);
	hpch *= level;
	hpch /= GetMaxLevel(ch);
	if(hpch<10) hpch=10;

	dam = hpch;
	dam = MIN(500, dam);

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	damage(ch, victim, dam, SPELL_FIRE_BREATH);

	/* And now for the damage on inventory */
	/* this was commented out since damage() will hurt objects */
/*

       	for (burn=victim->carrying ;
	     burn && (burn->obj_flags.type_flag!=ITEM_SCROLL) &&
	    (burn->obj_flags.type_flag!=ITEM_WAND) &&
	    (burn->obj_flags.type_flag!=ITEM_STAFF) &&
	    (burn->obj_flags.type_flag!=ITEM_BOAT);
	     burn=burn->next_content) {
	     if (!saves_spell(victim, SAVING_BREATH) ) 	{
       		if (burn)  {
			MakeScrap(victim, burn);
       			act("$o burns",0,victim,burn,0,TO_CHAR);
       			extract_obj(burn);
       		}
	     }
	}
*/
}


void spell_frost_breath(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
	int dam;
	int hpch;

	assert(victim && ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	hpch = GET_MAX_HIT(ch);
	hpch *= level;
	hpch /= GetMaxLevel(ch);
	if(hpch<10) hpch=10;

	dam = hpch;
	dam = MIN(400, dam);

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	damage(ch, victim, dam, SPELL_FROST_BREATH);

	/* And now for the damage on inventory */
	/* this is not needed since damage() will hurt objects */
/*
       	for (frozen=victim->carrying ;
       	    frozen && (frozen->obj_flags.type_flag!=ITEM_DRINKCON) &&
	    (frozen->obj_flags.type_flag!=ITEM_POTION);
	    frozen=frozen->next_content) {

       	    if (!saves_spell(victim, SAVING_BREATH) ) {
       	      if (frozen) {
		    act("$o shatters.",0,victim,frozen,0,TO_CHAR);
		    extract_obj(frozen);
	      }
	    }
	}
*/
}


void spell_acid_breath(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
	int dam;
	int hpch;

	assert(victim && ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	hpch = GET_MAX_HIT(ch);
	hpch *= level;
	hpch /= GetMaxLevel(ch);
	if(hpch<10) hpch=10;

	dam = hpch;
	dam = MIN(500, dam);

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	damage(ch, victim, dam, SPELL_ACID_BREATH);

}


void spell_gas_breath(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
	int dam;
	int hpch;

	assert(victim && ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	hpch = GET_MAX_HIT(ch);
	hpch *= level;
	hpch /= GetMaxLevel(ch);
	if(hpch<10) hpch=10;

	dam = hpch;
	dam = MIN(400, dam);

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	damage(ch, victim, dam, SPELL_GAS_BREATH);


}


void spell_lightning_breath(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
	int dam;
	int hpch;

	assert(victim && ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	hpch = GET_MAX_HIT(ch);
	hpch *= level;
	hpch /= GetMaxLevel(ch);
	if(hpch<10) hpch=10;

	dam = hpch;
	dam = MIN(450, dam);

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	damage(ch, victim, dam, SPELL_LIGHTNING_BREATH);


}

void spell_invulnerability(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  if ((affected_by_spell(victim, SPELL_SANCTUARY)) ||
     (affected_by_spell(victim, SPELL_FIRESHIELD))) {
    sendf(ch,"Invulnerability wont work with fireshield or sanctuary!\n");
    return;
  }

  if(!affected_by_spell(victim, AFF_INVULNERABLE))
    {
      act("$n begins to shimmer.",TRUE,victim,0,0,TO_ROOM);
      act("You begin to shimmer.",TRUE,victim,0,0,TO_CHAR);

      af.type   = SPELL_INVULNERABLE;
      af.duration = 4;
      af.modifier = 0;
      af.location = APPLY_NONE;
      init_set(af.bitvector,AFF_INVULNERABLE);
      affect_to_char(victim, &af);
    }
}


/* NEW */

void spell_shriek(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int dam;

  assert(victim && ch);

  dam = dice(ELVL20*4,9);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_SHRIEK);
}


void spell_drain_vigor_light(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int movepoints;
  assert(victim);

  movepoints = dice(1,8);

  GET_MOVE(victim) -= movepoints;

  update_pos(victim);
}

void spell_drain_vigor_serious(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int movepoints;
  assert(victim);

  movepoints = dice(1, 16);

  GET_MOVE(victim) -= movepoints;

  update_pos(victim);
}

void spell_drain_vigor_critic(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int movepoints;
  assert(victim);

  movepoints = dice(1, 42);

  GET_MOVE(victim) -= movepoints;

  update_pos(victim);
}

void spell_wither(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(ch && victim);

  if (saves_spell(victim, SAVING_SPELL) ||
      affected_by_spell(victim, SPELL_WITHER))
    return;

  af.type      = SPELL_WITHER;
  af.location  = APPLY_MOVE;
  af.modifier  = -50;
  af.duration  = 1;
  init_set(af.bitvector,AFF_BERRECV);
  affect_to_char(victim, &af);

  af.location = APPLY_ALL_AC;
  af.modifier = +40; /* Make AC Worse! */
  affect_to_char(victim, &af);
}

void spell_summon_demon(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(ch && victim && obj);

  act("$n gestures, and a black cloud of smoke appears", 1, ch, 0, 0, TO_ROOM);
  act("You gesture, and a black cloud of smoke appears", 1, ch, 0, 0, TO_CHAR);
  act("$p bursts into flames, and disintegrates!", TRUE, ch, obj, 0, TO_ROOM);
  act("$p bursts into flames, and disintegrates!", TRUE, ch, obj, 0, TO_CHAR);

  obj_from_char(obj);
  extract_obj(obj);
  char_to_room(victim, ch->in_room);

  act("With an evil laugh, $N emerges from the vile, black smoke",
      TRUE, ch, 0, victim, TO_NOTVICT);
  if (victim->master)
    stop_follower(victim);

  add_follower(victim, ch);

  af.type      = SPELL_CHARM_PERSON;
  af.duration  = GetMaxLevel(ch);
  af.modifier  = 0;
  af.location  = 0;
  init_set(af.bitvector,AFF_CHARM);

  affect_to_char(victim, &af);

  if (IS_SET(victim->specials.act, ACT_AGGRESSIVE))
    CLEAR_BIT(victim->specials.act, ACT_AGGRESSIVE);

  if (!IS_SET(victim->specials.act, ACT_SENTINEL))
    SET_BIT(victim->specials.act, ACT_SENTINEL);
}

void spell_fury(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  if (IS_NPC(victim) && !IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
    if(!saves_spell(victim, SAVING_SPELL)) {
      act("$N has just goaded $n into a wild rage with a spell!",
	  TRUE, victim, 0, ch, TO_ROOM);
      act("You have just goaded $n into a wild rage with your spell!",
	  TRUE, victim, 0, ch, TO_CHAR);
      SET_BIT(victim->specials.act, ACT_AGGRESSIVE);
    }
  } else {
    if(!saves_spell(victim, SAVING_SPELL)) {
      act("$N has just goaded $n into a wild rage with a spell!",
	TRUE, victim, 0, ch, TO_ROOM);
      act("You have just goaded $n into a wild rage with your spell!",
	  TRUE, victim, 0, ch, TO_CHAR);

      af.type      = SPELL_FURY;
      af.duration  = MAX(3, MIN(10, GetMaxLevel(ch) - GetMaxLevel(victim)));
      af.modifier  = 50;
      af.location  = APPLY_ALL_AC;
      init_set(af.bitvector,AFF_BERSERK);

      affect_to_char(victim, &af);
    }
  }
}

void spell_calm(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{

  if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
    if(!saves_spell(victim, SAVING_SPELL)) {
      act("$N manages to calm down $n, with a soothing spell!",
	  TRUE, ch, 0, victim, TO_ROOM);
      act("You manage to calm down $n, with your soothing spell!",
	  TRUE, ch, 0, victim, TO_VICT);
      CLEAR_BIT(victim->specials.act, ACT_AGGRESSIVE);
    }
  } else
    send_to_char("You failed.\n", ch);
}

void spell_summon_elemental_ii(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  struct char_data *mob;
  struct follow_type *k, *next_dude;
  int i, sum;
  static struct {
    int mob_number;
    char *message;
  } summons[] = { {3050, "$n arrives in a burst of fire."},
		    {3051, "$n forms from beneath your feet."},
		    {3052, "A gust of wind solidifies into $n."},
		    {3053, "A sudden gale forms into $n"}
		};
  assert(ch);

  for (k = ch->followers, i = 0 ; k ; k = next_dude) {
    next_dude = k->next;
    i++;
  }

  if (i > 4) {
    send_to_char("You cannot have more than 5 elementals!\n", ch);
    return;
  }

  sum = number(0,3);
  mob=get_mob(summons[sum].mob_number);
  /*mob = read_mobile(real_mobile(summons[sum].mob_number), REAL);*/
  char_to_room(mob, ch->in_room);
  act(summons[sum].message, TRUE, mob, 0, 0, TO_ROOM);
  if (!number(0,4)) { /* It's out of control */
    act("$N is NOT pleased at being suddenly summoned against his will!",
	TRUE, ch, 0, mob, TO_ROOM);
    act("$N is NOT pleased with you at all!", TRUE, ch, 0, mob, TO_CHAR);
    hit(mob, ch, TYPE_UNDEFINED);
  } else { /* Under control */
    act("$N sulkily says 'Your wish is my command, $n!'",
	TRUE, ch, 0, mob, TO_CHAR);
    act("$N sulkily says 'Your wish is my command, master!'",
	TRUE, ch, 0, mob, TO_ROOM);
    add_follower(mob, ch);
    GET_EXP(mob) = 0;

    af.type      = SPELL_CHARM_PERSON;
    af.duration  = 24;
    af.modifier  = 0;
    af.location  = 0;
    init_set(af.bitvector,AFF_CHARM);
    affect_to_char(mob, &af);
  }
}


void spell_mage_fire(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int dam;

  assert(victim && ch);

  dam = dice(ELVL20*5,9);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_MAGE_FIRE);
}

void spell_firestorm(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct char_data *tch, *next;
  int dam;

  assert(ch);

  dam = (10 * dice(1,7)) + dice(3, 10);

  for(tch = real_roomp(ch->in_room)->people; tch; tch = next) {
    next = tch->next_in_room;

    if(tch == ch)
      continue;

    if (!IS_NPC(tch))
      continue;

    if(!saves_spell(tch, SAVING_SPELL))
      damage(ch, tch, dam, SPELL_FIRESTORM);
    else
      damage(ch, tch, dam/2, SPELL_FIRESTORM);
  }
}


void spell_full_harm(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int dam;
  assert(victim && ch);

  dam = 200;

  if (hit_limit(victim) <= 200)
    dam = hit_limit(victim) - dice(1,8);

  if (GET_HIT(victim) <= 200)
    dam = GET_HIT(victim) - dice(1,8);

  if (GET_HIT(victim) < 10) {
    dam = 0; /* Kill the suffering bastard */
    send_to_char("Your spell can not injure your victim any further...\n", victim); }
  else {
    if ( saves_spell(victim, SAVING_SPELL) )
      dam = dam / 2;
  }
}


void spell_holy_word(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int dam;
  assert(victim && ch);

  dam = 16 * dice(1, 8);

  if (dam < 0)
    dam = 0;
  else {
    if ( saves_spell(victim, SAVING_SPELL) )
      dam =  dam/2;
  }

  damage(ch, victim, dam, SPELL_HOLY_WORD);

}


void spell_unholy_word(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int dam;
  assert(victim && ch);

  dam = 17 * dice(1, 8);

  if (dam < 0)
    dam = 0;
  else {
    if ( saves_spell(victim, SAVING_SPELL) )
      dam =  dam/2;
  }

  damage(ch, victim, dam, SPELL_UNHOLY_WORD);
}

/* spells2.c - Not directly offensive spells */

void spell_translook(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  sh_int target;
  char buf[MAX_STRING_LENGTH];

  target = real_roomp(victim->in_room)->number;

  if (GetMaxLevel(victim) > MAXLVLMORTAL ||
      !(real_roomp(ch->in_room)->zone == real_roomp(victim->in_room)->zone)) {
    send_to_char("You failed.\n", ch);
    return;
  }

  sendf(ch,"You cast your sights far out, into the zone!\n");
  sprintf(buf,"%d look", target);
  do_at(ch, buf, 0);
}


void spell_telelook(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  sh_int target;
  char buf[MAX_STRING_LENGTH];

  target = real_roomp(victim->in_room)->number;

  if (GetMaxLevel(victim) > MAXLVLMORTAL) {
    sendf(ch,"You failed.\n");
    return;
  }

  sendf(ch,"You cast your sights far out, into the realms!\n");
  sprintf(buf, "%d look", target);
  do_at(ch, buf, 0);
}


void spell_teleimage(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  sh_int target;
  struct char_data *tmp_victim, *temp;
  char buf[MAX_STRING_LENGTH], buf1[MAX_STRING_LENGTH];

  target = real_roomp(victim->in_room)->number;

  if (GetMaxLevel(victim) > MAXLVLMORTAL) {
    send_to_char("You failed.\n", ch);
    return;
  }

  strcpy(buf, "You conjure up a cloud which thickens, shimmers, and\n"
  "then ... suddenly goes transparent and shows ...\n");
  strcpy(buf1, "$n conjures up a cloud which thickens, shimmers, and\n"
  "then ... suddenly goes transparent and shows ...\n");
  act(buf, FALSE, ch, 0, 0, TO_CHAR);
  act(buf1, FALSE, ch, 0, 0, TO_ROOM);

  sprintf(buf1, "%d look", target);
  for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
      temp = tmp_victim->next;
      if((ch->in_room == tmp_victim->in_room) && !IS_NPC(tmp_victim))
	do_at(tmp_victim, buf1, 0);
  }
}


void spell_transmove(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct room_data *rp;

  assert(ch && victim);

  rp = spell_char_to_mob (victim->in_room, get_spell (ch, SPELL_TRANSMOVE), ch);

  if (GetMaxLevel(victim) > MAX_MORT ||
      ROOM_FLAG(ch->in_room,  ARENA) || !rp ||
      ROOM_FLAG(rp->number,ARENA)) { 
    	send_to_char("You failed.\n", ch);
      	return;
     }

  act("$n fades out with a soft swoosh",FALSE,ch,0,0,TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, rp->number);
  act("$n slowly fades in, looking a bit disoriented.",FALSE,ch,0,0,TO_ROOM);
  do_look(ch, "",15);
}


void spell_telemove(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct room_data *rp;

  assert(ch && victim);

  if (dice(1,4) == 4) {
    (void) spell_teleport(level, ch, ch, 0,eff_level);
    return;
  }

  rp = spell_char_to_mob (victim->in_room, get_spell (ch, SPELL_TELEMOVE), ch);

  if (GetMaxLevel(victim) > MAX_MORT ||
      ROOM_FLAG(ch->in_room,  ARENA) || !rp ||
      ROOM_FLAG(rp->number,ARENA)) { 
    	send_to_char("You failed.\n", ch);
      	return;
     }

  act("$n fades out with a soft swoosh",FALSE,ch,0,0,TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, rp->number);
  act("$n slowly fades in, looking a bit disoriented.",FALSE,ch,0,0,TO_ROOM);
  do_look(ch, "",15);
}


void spell_vitalize_mana(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int trans;

  trans = ((dice(1, GetMaxLevel(ch))+20)*2);

  act("You feel more energized!", FALSE, ch, 0, 0, TO_CHAR);

  ch->points.mana += trans;
  ch->points.mana = MIN(GET_MANA(ch), GET_MAX_MANA(ch));
  ch->points.move -= 25;

  GET_HIT(ch) -= (dice(2,10)+100);

  if (GET_HIT(ch) <= 0)
    GET_HIT(ch) = 1;

}



void spell_rejuvenate(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  int age;
  assert(victim);

  act("You feel younger.", FALSE, victim, 0, 0, TO_CHAR);
  age = dice(3, 6);

  af.type      = SPELL_REJUVENATE;
  af.duration  = age;
  af.modifier  = -age;

  af.location  = APPLY_AGE;
  init_bits(af.bitvector);

  affect_join(victim, &af, TRUE, FALSE);
}


void spell_age(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  int age;
  assert(victim);

  act("You feel older.", FALSE, victim, 0, 0, TO_CHAR);
  age = dice(1, 5);

  af.type      = SPELL_AGE;
  af.duration  = 24;
  af.modifier  = age;
  af.location  = APPLY_AGE;
  init_bits(af.bitvector);

  affect_join(victim, &af, TRUE, FALSE);
}


void spell_ageing(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  char buf[MAX_INPUT_LENGTH];
  long secs, played;
  time_t  curr_time = time(NULL);
  played = curr_time - ch->player.time.birth;

  assert(victim);

  if(victim->consent != ch) {
    sprintf(buf, "%s has not given %s consent to you.\n",
	    GET_NAME(victim), HSHR(victim));
    send_to_char(buf, ch);
    return;
  }

  secs = (dice(3,8) * SECS_PER_MUD_YEAR);

  victim->player.time.birth = curr_time - (played + secs);
  send_to_char(buf, ch);

  send_to_char("You feel a bit older all of a sudden!\n", victim);

}

void spell_cure_serious(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int healpoints;

  assert(victim);

  healpoints = dice(2, 8)+2;

  if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  send_to_char("You feel better!\n", victim);

  update_pos(victim);
}


void spell_invis_group(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct char_data *tmp_victim;
  struct affected_type af;

  assert(ch);

  for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
       tmp_victim = tmp_victim->next_in_room) {
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim))
      if (!affected_by_spell(tmp_victim, SPELL_INVISIBLE)) {

	act("$n slowly fades out of existence.", TRUE,
	    tmp_victim, 0, 0, TO_ROOM);
	send_to_char("You vanish.\n", tmp_victim);

	af.type      = SPELL_INVISIBLE;
	af.duration  = 24;
	af.modifier  = -40;
	af.location  = APPLY_ALL_AC;
	init_set(af.bitvector,AFF_INVISIBLE);
	affect_to_char(tmp_victim, &af);
      }
  }
}


void spell_heroes_feast(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct char_data *tch;

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (GET_POS(ch) > POSITION_SLEEPING) {
      send_to_char("You partake of a magnificent feast!\n", ch);
      gain_condition(tch,FULL,24);
      gain_condition(tch,THIRST,24);
      if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
	GET_HIT(ch)+=1;
      }
      GET_MOVE(ch) = GET_MAX_MOVE(ch);
    }
  }
}


/*
 void spell_cloak_of_illumination(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int i;

  assert(ch && obj);
  assert(MAX_OBJ_AFFECT >= 2);

  if (!IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      if(obj->affected[i].location != APPLY_NONE)
	return;
    SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

    obj->affected[0].location = APPLY
*/


void spell_full_heal(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  assert(victim);

  spell_cure_blind(level, ch, victim, obj,eff_level);

  GET_HIT(victim) += 300;

  if (GET_HIT(victim) >= hit_limit(victim))
    GET_HIT(victim) = hit_limit(victim)-dice(1,8);

  update_pos(victim);

  send_to_char("You feel fully restored!.\n", victim);
}

void spell_mystic_heal(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  assert(victim);

switch(number(0,5)) {

	case 0:
		GET_HIT(victim) += 350;
		spell_bless(level, ch, victim, obj,eff_level);
		break;
	case 1:
		GET_HIT(victim) += 400;
		spell_cure_blind(level, ch, victim, obj,eff_level);
		break;
	case 2:
		GET_HIT(victim) += 450;
		spell_armor(level, ch, victim, obj,eff_level);
		break;
	case 3:
		GET_HIT(victim) += 400;
		spell_remove_curse(level, ch, victim, obj,eff_level);
		break;
	case 4:
		GET_HIT(victim) += 500;
		spell_weakness(level, ch, victim, obj,eff_level);
		spell_shield(level, ch, victim, obj,eff_level);
		spell_curse(level, ch, victim, obj,eff_level);
		break;
	default:
		GET_HIT(victim) += 400;
   }
  if (GET_HIT(victim) >= hit_limit(victim))                                     
    GET_HIT(victim) = hit_limit(victim)-dice(1,8);

  update_pos(victim);
  send_to_char("You feel... different..\n", victim);
}

void spell_scry(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int i,max;

  max = MIN(50,GetMaxLevel(ch));
  if (!saves_spell(victim, SAVING_PARA)) {
    send_to_char("Gathering your energies to summon the globe of truth,\n"
		 "you peer into it and see that: \n", ch);
    for (i = 0; i < max; i++) {
      switch(number(1, 50)) {
      case 1: sendf(ch,"%s has %d levels.\n",GET_NAME(victim),GetMaxLevel(victim)); break;
      case 2: sendf(ch,"%s has %d XP pts.\n",GET_NAME(victim),GET_EXP(victim)); break;
      case 3: sendf(ch,"%s has %d movement pts.\n",GET_NAME(victim),GET_MOVE(victim)); break;
      case 4: sendf(ch,"%s has %d mana pts.\n",GET_NAME(victim), GET_MANA(victim)); break;
      case 5: sendf(ch,"%s has %d hit pts.\n", GET_NAME(victim), GET_HIT(victim)); break;
      case 6: sendf(ch,"%s has %d gold coins.\n", GET_NAME(victim), GET_GOLD(victim)); break;
      case 7: sendf(ch,"%s has an alignment of %d.\n", GET_NAME(victim), victim->specials.alignment); break;
      case 8: sendf(ch,"%s has a body armor rating of %d.\n",GET_NAME(victim), victim->points.armor[4]);break;
      case 9: sendf(ch,"%s has a to-hit rating of %d.\n", GET_NAME(victim), GET_HITROLL(victim)); break;
      case 10:sendf(ch,"%s has a to-dam rating of %d.\n", GET_NAME(victim), GET_DAMROLL(victim)); break;
      case 11:sendf(ch,"%s has a first saving throw of %d.\n", GET_NAME(victim), ch->specials.apply_saving_throw[0]); break;
      case 12:sendf(ch,"%s has a second saving throw of %d.\n", GET_NAME(victim), ch->specials.apply_saving_throw[1]); break;
      case 13:sendf(ch,"%s has a third saving throw of %d.\n", GET_NAME(victim), ch->specials.apply_saving_throw[2]); break;
      case 14:sendf(ch,"%s has a fourth saving throw of %d.\n", GET_NAME(victim), ch->specials.apply_saving_throw[3]); break;
      case 15:sendf(ch,"%s has a fifth saving throw of %d.\n", GET_NAME(victim), ch->specials.apply_saving_throw[4]); break;
      case 16:sendf(ch,"%s has %d power pts.\n",GET_NAME(victim), GET_POWER(victim)); break;
      default:
	break;
      }
    }
    send_to_char("The globe flickers and vanishes as you tire.\n", ch);
  } else {
    send_to_char("You failed.\n", ch);
  }
}

void spell_nosleep(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  assert(victim);

  if(!IS_AFFECTED(victim, AFF_NOSLEEP)) {

    af.type = SPELL_NOSLEEP;
    af.duration = GET_INT(ch);
    af.modifier = 0;
    af.location = 0;
    init_set(af.bitvector,AFF_NOSLEEP);
    affect_to_char(victim, &af);

    act("$n has protected you against sleepiness.", FALSE,ch,0,victim,TO_VICT);
  }
}


void spell_sleep(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  assert(victim);

  if(IS_AFFECTED(victim, AFF_NOSLEEP)) {
    send_to_char("You failed!\n", ch);
    return;
  }

  if (!CORRUPT(ch,victim)) return;
  if ( !saves_spell(victim, SAVING_SPELL))
    {
      af.type      = SPELL_SLEEP;
      af.duration  = 4;
      af.modifier  = 0;
      af.location  = APPLY_NONE;
      init_set(af.bitvector,AFF_SLEEP);
      affect_join(victim, &af, FALSE, FALSE);

      if (GET_POS(victim)>POSITION_SLEEPING)
	{
	  act("You feel very sleepy ..... zzzzzz",FALSE,victim,0,0,TO_CHAR);
	  act("$n go to sleep.",TRUE,victim,0,0,TO_ROOM);
	  GET_POS(victim)=POSITION_SLEEPING;
	}
      return;
    }
}


void spell_nosummon(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
/*  sh_int target; */
  struct affected_type af;
  assert(victim);

 if(!IS_AFFECTED(victim, AFF_NOSUMMON)) {

  af.type = SPELL_NOSUMMON;
  af.duration = 24;
  af.modifier = 0;
  af.location = APPLY_NONE;
  init_set(af.bitvector,AFF_NOSUMMON);
  affect_to_char(victim, &af);

  act("$n has protected you against summons.", FALSE,ch,0,victim,TO_VICT);
 }
}

void spell_dexterity(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  assert(victim);

  act("You feel more dexterous.", FALSE, victim, 0, 0, TO_CHAR);

  af.type = SPELL_DEXTERITY;
  af.duration = dice(5, 5);
  af.modifier = 1 + (level >= 33);

  af.location = APPLY_DEX;
  init_bits(af.bitvector);

  affect_join(victim, &af, TRUE, FALSE);
}

void spell_summon(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  sh_int target;

  if (GetMaxLevel(victim) > MIN(MAXLVLMORTAL,level+5) ||
      IS_AFFECTED(victim, AFF_NOSUMMON) ||
      CHAR_IN_PRIV_ZONE(ch) ||
      CHAR_IN_SAFE_ZONE(victim) ||
      (IN_ARENA(ch) && !IN_ARENA(victim)) ||
      (IS_NPC(victim) && GET_POS(victim)==POSITION_FIGHTING) ||
      (MOB_FLAG(victim,ACT_NOSUMMON)) ||
      (IS_NPC(victim) && saves_spell(victim, SAVING_SPELL))) {
    send_to_char("You failed.\n",ch);
    return;
  }

  act("$n disappears suddenly.",TRUE,victim,0,0,TO_ROOM);

  target = ch->in_room;
  char_from_room(victim);
  char_to_room(victim,target);

  act("$n arrives suddenly.", TRUE, victim, 0, 0, TO_ROOM);
  act("$n has summoned you!", FALSE, ch, 0, victim, TO_VICT);
  do_look(victim,"",15);
  if(IS_NPC(victim) && !IS_NPC(ch) && !check_peaceful(victim,
      "You find this area too peacful to contemplate violence.")){
    if(!CAN_SEE(victim, ch)){
      act("Even though $N can't see you, it seems to sense your presence.",
        FALSE, ch, NULL, victim, TO_CHAR);
    }
    hit(victim, ch, TYPE_UNDEFINED);
  }
  else if(IS_NPC(victim) && !IS_NPC(ch)){
    act("The fabric of space is ripped asunder and you fall into it.",
      FALSE, ch, NULL, victim, TO_CHAR);
    spell_teleport(IMPLEMENTOR, ch, ch, NULL, IMPLEMENTOR);
  }
}

void spell_charm_person(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(ch && victim);

  if (victim == ch) {
    send_to_char("You like yourself even better!\n", ch);
    return;
  }

  if(!IsPerson(victim)){
    if(IS_NPC(victim)){
      act("$N growls at your feeble magic and attacks!", 0, ch, NULL, victim,
        TO_CHAR);
      hit(victim, ch, TYPE_UNDEFINED);
    }
    else{
      act("$N ignores your feeble attempts at magic.", 0, ch, NULL, victim,
        TO_CHAR);
    }
    return;
  }

  if(IS_AFFECTED(victim, AFF_NOCHARM)) {
    send_to_char("Your victim isn't too charmed at you ... oops!\n", ch);
    if(IS_NPC(victim))
      hit(victim, ch, TYPE_UNDEFINED);
    return;
  }

  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM) &&
      (level >= GetMaxLevel(victim))) {
    if (circle_follow(victim, ch)) {
      send_to_char("Sorry, following in circles can not be allowed.\n", ch);
      return;
    }
      if (IsImmune(victim, IMM_CHARM)) {
          FailCharm(victim,ch);
          return;
      }
      if (IsResist(victim, IMM_CHARM)) {
         if (saves_spell(victim, SAVING_PARA)) {
          FailCharm(victim,ch);
          return;
         }

         if (saves_spell(victim, SAVING_PARA)) {
          FailCharm(victim,ch);
          return;
         }
       } else {
          if (!IsSusc(victim, IMM_CHARM)) {
             if (saves_spell(victim, SAVING_PARA)) {
                FailCharm(victim,ch);
                return;
             }
          }
       }


    if (GetMaxLevel(ch)<GetMaxLevel(victim)){
      FailCharm(victim,ch);
      return;
    }
    if (saves_spell(victim, SAVING_PARA)){
      FailCharm(victim, ch);
      return;
    }

    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type = SPELL_CHARM_PERSON;

    if (GET_INT(victim))
      af.duration  = 48/GET_INT(victim);
    else
      af.duration  = 4;

    af.modifier  = 0;
    af.location  = 0;
    init_set(af.bitvector,AFF_CHARM);
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?",FALSE,ch,0,victim,TO_VICT);
  }
}


void spell_nocharm(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  void stop_follower(struct char_data *ch);

  if(!IS_AFFECTED(victim, AFF_NOCHARM)) {

    af.type = SPELL_NOCHARM;
    af.duration = GET_INT(ch);
    af.modifier = 0;
    af.location = 0;
    init_set(af.bitvector,AFF_NOCHARM);
    affect_to_char(victim, &af);

  act("$n has protected you against being charmed.", FALSE,ch,0,victim,TO_VICT);
  }
}

void spell_sense_life(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  assert(victim);

  if (!affected_by_spell(victim, SPELL_SENSE_LIFE)) {
    send_to_char("Your feel your awareness improve.\n", ch);

    af.type      = SPELL_SENSE_LIFE;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    init_set(af.bitvector,AFF_SENSE_LIFE);
    affect_to_char(victim, &af);
  }
}


void spell_haste(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  assert(victim);

  if (!affected_by_spell(victim, SPELL_HASTE)) {
    send_to_char("You feel the world slow down about you.\n", victim);

    af.type      = SPELL_HASTE;
    af.duration  = 4;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    init_set(af.bitvector,AFF_HASTE);
    affect_to_char(victim, &af);
  }
}

void spell_recharger(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(victim)) {
    send_to_char("You cant recharge from a monster!\n", ch);
    return;
  }

  if(affected_by_spell(victim, SPELL_RECHARGER)) {
    send_to_char("Nothing seems to happen.\n", ch);
    return;
  }

  if(victim->consent != ch) {
    sprintf(buf, "%s has not given %s consent to you.\n",
	    GET_NAME(victim), HSHR(victim));
    send_to_char(buf, ch);
    return;
  }

  else if(victim == ch) {
    send_to_char("How can you recharge yourself, silly!!.\n", ch);
    return;
  }

  else if(GetMaxLevel(victim) < GetMaxLevel(ch) - 40) {
    sprintf(buf, "%s is too low for you to recharge from.\n",
	    GET_NAME(victim));
    send_to_char(buf, ch);
    return;
  }

  send_to_char("You feel drained.\n", victim);
  send_to_char("You feel recharged.\n", ch);

  af.type   = SPELL_RECHARGER;
  af.duration = 48;
  af.modifier = -20;
  af.location = APPLY_HIT;
  init_set(af.bitvector,AFF_RECHARGER);
  affect_to_char(victim, &af);

  ch->points.mana +=victim->points.mana;
  ch->points.mana = MIN(mana_limit(ch), ch->points.mana);
  victim->points.mana = 0;

}

void spell_chilly(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  int dam;

  dam = 13 * dice(1, 8);

  assert(victim && ch);

  if ( !saves_spell(victim, SAVING_SPELL) )
  {
    af.type      = SPELL_CHILLY;
    af.duration  = 12;
    af.modifier  = -3;
    af.location  = APPLY_STR;
    init_bits(af.bitvector);

    affect_join(victim, &af, TRUE, FALSE);
  } else {
    dam >>= 1;
  }
  damage(ch, victim, dam, SPELL_CHILLY);
}

void spell_sunray(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int dam;

  extern struct weather_data weather_info;

  assert(victim && ch);

  dam = dice(5, 15);

  if (OUTSIDE(ch) && (weather_info.sky=SKY_CLOUDLESS) &&
      (weather_info.sunlight=SUN_LIGHT)) {

    if ( saves_spell(victim, SAVING_SPELL) )
      dam >>= 1;

    damage(ch, victim, dam, SPELL_SUNRAY);
  }
}


void spell_metalskin(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(victim);

  if (!affected_by_spell(victim, SPELL_METALSKIN) &&
      !affected_by_spell(victim, SPELL_STONE_SKIN)) {
    af.type      = SPELL_METALSKIN;
    af.duration  = 4;
    af.modifier  = dice(1, 3);
    af.location  = APPLY_ALL_STOPPING;
    init_bits(af.bitvector);
    affect_to_char(victim, &af);
    af.location  = APPLY_DEX;
    af.modifier  = - dice(1, 3);
    affect_to_char(victim, &af);
    af.location  = APPLY_HITROLL;
    af.modifier= - dice(1, 3);
    affect_to_char(victim, &af);
    send_to_char("Your skin takes a metallic complextion.\n", victim);
  }
}

void spell_vampiric_touch(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int dam;

  assert(victim && ch);

  if ( !saves_spell(victim, SAVING_SPELL) ) {
    GET_ALIGNMENT(ch) = MIN(-1000, GET_ALIGNMENT(ch)-200);
    if (GetMaxLevel(victim) <= 2) {
      damage(ch, victim, 10, SPELL_ENERGY_DRAIN); /* Kill the sucker */
    } else {
      /*
      hp = number(level>>1,level)*1;
      GET_MAX_HIT(victim) -= hp;
      */
      dam = dice(5,5);

      send_to_char("You feel your vitality being drained!\n", victim);

      damage(ch, victim, dam, SPELL_VAMPIRIC_TOUCH);
    }
  } else {
    damage(ch, victim, 0, SPELL_VAMPIRIC_TOUCH); /* Miss */
  }
}

void spell_vigorize_light(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int movepoints;
  assert(ch && victim);

  movepoints = dice(3,8)+2;

  if (GET_RACE(ch) == RACE_MUTANT) {
    send_to_char("You lack the ability to do this!\n", ch);
    return;
  }

  if ( (movepoints+GET_MOVE(victim)) > move_limit(victim) )
    GET_MOVE(victim) = move_limit(victim);
  else
    GET_MOVE(victim) += movepoints;

  update_pos(victim);

  send_to_char("You feel a bit more vigorized!\n", victim);
}


void spell_vigorize_serious(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int movepoints;
  assert(ch && victim);

  if (GET_RACE(ch) == RACE_MUTANT) {
    send_to_char("You lack the ability to do this!\n", ch);
    return;
  }

  movepoints = dice(6, 8)+3;

  if ( (movepoints + GET_MOVE(victim)) > move_limit(victim) )
    GET_MOVE(victim) = move_limit(victim);
  else
    GET_MOVE(victim) += movepoints;

  send_to_char("You feel much more vigorized!\n", victim);

  update_pos(victim);
}

void spell_vigorize_critic(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int movepoints;
  assert(ch && victim);

  if (GET_RACE(ch) == RACE_MUTANT) {
    send_to_char("You lack the ability to do this!\n", ch);
    return;
  }

  movepoints = dice(8,8)+6;

  if ( (movepoints + GET_MOVE(victim)) > move_limit(victim) )
    GET_MOVE(victim) = move_limit(victim);
  else
    GET_MOVE(victim) += movepoints;

  send_to_char("You feel vigorized!\n", victim);

  update_pos(victim);
}


void spell_vitality(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  int temp_hp;

  if(!affected_by_spell(victim, SPELL_VITALITY)) {
    send_to_char("You feel vitalized.\n", ch);

      temp_hp = 5 * dice(1,14);

    af.type      = SPELL_VITALITY;
    af.duration  = dice(2,3);
    af.modifier  = temp_hp;
    af.location  = APPLY_HIT;
    init_set(af.bitvector,AFF_VITALITY);

    affect_to_char(victim, &af);
  }
}

void spell_farsee(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  if(!affected_by_spell(victim, SPELL_FARSEE)) {
    act("Your eyes become as sharp as a hawk's.", TRUE, victim, 0, 0, TO_CHAR);

    af.type   = SPELL_FARSEE;
    af.duration = dice(6,6);
    af.modifier = 0;
    af.location = APPLY_NONE;
    init_set(af.bitvector,AFF_FARSEE);

    affect_to_char(victim, &af);
  }
}

void spell_fear(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  char buf[MAX_STRING_LENGTH];

  assert(victim && ch);

  if(!affected_by_spell(victim, AFF_NOFEAR)) {
    if(!saves_spell(victim, SAVING_SPELL)) {
      sprintf(buf, "You scare the bejesus out of %s!\n", GET_NAME(victim));
      send_to_char(buf, ch);
      send_to_char("SHIT! Flee!! Run, man, run!!", victim);
      do_flee(victim,"",0);
  }
  }
  else {
    sprintf(buf, "%s looks at you nonchalantly.\n", GET_NAME(victim));
    send_to_char(buf, ch);
    send_to_char("You experience a wave of fear,  which passes quickly.\n", victim);
  }
}


void spell_heroism(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  assert(victim && ch);

  if(!IS_AFFECTED(victim, AFF_NOFEAR)) {

    af.type = SPELL_NOFEAR;
    af.duration = GET_INT(ch);
    af.modifier = 0;
    af.location = 0;
    init_set(af.bitvector,AFF_NOFEAR);
    affect_to_char(victim, &af);

    act("$n has protected you with a heroism spell!",
FALSE,ch,0,victim,TO_VICT);
  }
  act("Nothing seems to happen to you...", FALSE, ch, 0, victim, TO_VICT);
}


void spell_ice_lance(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int dam;

  assert(victim && ch);

  dam = 20 * dice(1, 8);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_ICE_LANCE);
}

void spell_freeze(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)

{
  int dam;

  assert(victim && ch);

  dam = 21 * dice(1, 8);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_FREEZE);
}

void spell_fire_bolt(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int dam;

  assert(victim && ch);

  dam = 22 * dice(1, 8);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_FIRE_BOLT);
}

void spell_repulsor(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int dam;

  assert(victim && ch);

  dam = 23 * dice(1, 8);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_REPULSOR);
}

void spell_mind_thrust(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int dam;

  assert(victim && ch);

  dam = 24 * dice(1, 8);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_MIND_THRUST);
}

void spell_disruptor(int level,struct char_data *ch,struct char_data *victim,struct obj_data *obj, int eff_level)
{
  int dam;

  assert(victim && ch);

  dam = 25 * dice(1, 8);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_DISRUPTOR);
}


void spell_dispel_invisible(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  assert((ch && obj) || victim);

  if (obj) {
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE) ) {
      act("$p fades into visibility.",FALSE,ch,obj,0,TO_CHAR);
      act("$p fades into visibility.",TRUE,ch,obj,0,TO_ROOM);
      CLEAR_BIT(obj->obj_flags.extra_flags,ITEM_INVISIBLE);
    }
  } else {
    if (affected_by_spell(victim, SPELL_INVISIBLE)) {
      act("$n slowly fades into existance.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You turn visible.\n", ch);
      affect_from_char(ch, SPELL_INVISIBLE);
      clear_bit(ch->specials.affected_by,AFF_INVISIBLE);
    }
  }
}

