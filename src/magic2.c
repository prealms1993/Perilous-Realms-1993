/* ************************************************************************
*  file: magic2.c , Implementation of (new)spells.        Part of DIKUMUD *
*  Usage : The actual effect of magic.                                    *
************************************************************************* */

#include <stdio.h>
#include <assert.h>

#include <fcns.h>
#include <externs.h>
#include <utils.h>
#include <comm.h>
#include <spells.h>
#include <handler.h>
#include <limits.h>
#include <db.h>
#include <race.h>


#define ELVL20 MIN(eff_level,20)

int max_follow(struct char_data *ch)
{
  return(10);
}

/*
  cleric spells
*/

/*
 **   requires the sacrifice of 150k coins, victim loses a con point, and
 **   caster is knocked down to 1 hp, 1 mp, 1 mana, and sits for a LONG
 **   time (if a pc)
 */

void spell_resurrection(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
#if 0
  struct affected_type af;
  struct obj_data *obj_object, *next_obj;
  FILE *fl;

  if (!obj) return;


  if (IS_CORPSE(obj)) {

    if (obj->char_vnum) {  /* corpse is a npc */

      victim = get_mob(obj->char_vnum);
      char_to_room(victim, ch->in_room);
      GET_GOLD(victim)=0;
      GET_EXP(victim)=0;
      GET_HIT(victim)=1;
      GET_POS(victim)=POSITION_STUNNED;

      act("With mystic power, $n resurrects a corpse.", TRUE, ch,
	  0, 0, TO_ROOM);
      act("$N slowly rises from the ground.", FALSE, ch, 0, victim, TO_ROOM);

      /*
	should be charmed and follower ch
	*/

      if (IsImmune(victim, IMM_CHARM) || IsResist(victim, IMM_CHARM)) {
 	act("$n says 'Thank you'", FALSE, ch, 0, victim, TO_ROOM);
      } else {
        af.type      = SPELL_CHARM_PERSON;
        af.duration  = 36;
        af.modifier  = 0;
        af.location  = 0;
        init_set(af.bitvector,AFF_CHARM);

        affect_to_char(victim, &af);

       	add_follower(victim, ch);
      }

      IS_CARRYING_M(victim) = 0;
      IS_CARRYING_V(victim) = 0;

      /*
	take all from corpse, and give to person
	*/

      for (obj_object=obj->contains; obj_object; obj_object=next_obj) {
	next_obj = obj_object->next_content;
	obj_from_obj(obj_object);
	obj_to_char(obj_object, victim);
      }

      /*
	get rid of corpse
	*/
      extract_obj(obj);


    } else {          /* corpse is a pc  */

      if (GET_GOLD(ch) < 150000) {
	send_to_char("The gods are not happy with your sacrifice.\n",ch);
	return;
      } else {
	GET_GOLD(ch) -= 150000;
      }




      fl = fopen(PLAYER_FILE, "r+");
      if (!fl) {
	perror("player file");
	exit(1);
      }
      fseek(fl, obj->char_f_pos * sizeof(struct char_file_u), 0);
      fread(&st, sizeof(struct char_file_u), 1, fl);
      /*
       **   this is a serious kludge, and must be changed before multiple
       **   languages can be implemented
       */
      if (!st.talks[2] && st.abilities.con > 3) {
	st.points.exp *= 2;
	st.talks[2] = TRUE;
	st.abilities.con -= 1;
	act("A clear bell rings throughout the heavens",
	    TRUE, ch, 0, 0, TO_CHAR);
	act("A ghostly spirit smiles, and says 'Thank you'",
	    TRUE, ch, 0, 0, TO_CHAR);
	act("A clear bell rings throughout the heavens",
	    TRUE, ch, 0, 0, TO_ROOM);
	act("A ghostly spirit smiles, and says 'Thank you'",
	    TRUE, ch, 0, 0, TO_ROOM);
	act("$p dissappears in the blink of an eye.",
	    TRUE, ch, obj, 0, TO_ROOM);
	act("$p dissappears in the blink of an eye.",
	    TRUE, ch, obj, 0, TO_ROOM);
	GET_MANA(ch) = 0;
	GET_POWER(ch) = 0;
	GET_MOVE(ch) = 0;
	GET_HIT(ch) = 1;
	GET_POS(ch) = POSITION_STUNNED;
	act("$n collapses from the effort!",TRUE, ch, 0, 0, TO_ROOM);
	send_to_char("You collapse from the effort\n",ch);
        fseek(fl, obj->char_f_pos * sizeof(struct char_file_u), 0);
	fwrite(&st, sizeof(struct char_file_u), 1, fl);
	ObjFromCorpse(obj);

      } else {
	send_to_char
	  ("The body does not have the strength to be recreated.\n", ch);
      }
      fclose(fl);
    }
  }
#endif
}

void spell_cause_light(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(1,8);

  damage(ch, victim, dam, SPELL_CAUSE_LIGHT);

}

void spell_cause_critical(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(3,8) + 3;

  damage(ch, victim, dam, SPELL_CAUSE_CRITICAL);

}

void spell_cause_serious(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(2,8) + 2;

  damage(ch, victim, dam, SPELL_CAUSE_SERIOUS);

}

#if 0
 void spell_cure_serious(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(2,8)+2;

  if ( (dam + GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += dam;

  send_to_char("You feel better!\n", victim);

  update_pos(victim);

}
#endif

void spell_mana(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level,4);
  dam = MAX(dam, level*2);

  if (GET_MANA(ch)+dam > GET_MAX_MANA(ch))
    GET_MANA(ch) = GET_MAX_MANA(ch);
  else
    GET_MANA(ch) += dam;

}

void spell_second_wind(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(2, 8)+level;

  if ( (dam + GET_MOVE(victim)) > move_limit(victim) )
    GET_MOVE(victim) = move_limit(victim);
  else
    GET_MOVE(victim) += dam;

  send_to_char("You feel less tired\n", victim);

}


void spell_flamestrike(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(6,8);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_FLAMESTRIKE);

}



void spell_dispel_good(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  assert(ch && victim);
   assert((level >= 1) && (level<=ABS_MAX_LVL));

  if (IsExtraPlanar(victim)) {
	if (IS_GOOD(ch)) {
       	    victim = ch;
	} else if (IS_EVIL(victim)) {
            act("Evil protects $N.", FALSE, ch, 0, victim, TO_CHAR);
  	    return;
	}

        if (!saves_spell(victim, SAVING_SPELL) ) {
	    act("$n forces $N from this plane.",TRUE,ch,0,victim,TO_NOTVICT);
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

void spell_turn(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int diff,i;

  assert(ch && victim);
  assert((level >= 1) && (level<=ABS_MAX_LVL));

  if (IsUndead(victim)) {

    diff = level - GET_LEVEL(victim);
    if (diff <= 0) {
      act("You are powerless to affect $N", TRUE, ch, 0, victim, TO_CHAR);
      return;
    } else {
      for (i = 1; i <= diff; i++) {
        if (!saves_spell(victim, SAVING_SPELL) ) {
	    act("$n forces $N from this room.",TRUE,ch,0,victim,TO_NOTVICT);
	    act("You force $N from this room.", TRUE, ch, 0, victim, TO_CHAR);
	    act("$n forces you from this room.", TRUE, ch, 0, victim,TO_VICT);
	    do_flee(victim,"",0);
	    break;
	}
      }
      if (i < diff) {
	act("You laugh at $n.", TRUE, ch, 0, victim, TO_VICT);
	act("$N laughs at $n.", TRUE, ch, 0, victim, TO_NOTVICT);
	act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
      }
    }
  } else {
	act("$n just tried to turn you, what a moron!", TRUE, ch, 0, victim, TO_VICT);
	act("$N thinks $n is really strange.", TRUE, ch, 0, victim, TO_NOTVICT);
	act("Um... $N isn't undead...", TRUE, ch, 0, victim, TO_CHAR);
  }
}

void spell_remove_paralysis(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{

  assert(ch && victim);

  if (affected_by_spell(victim,SPELL_PARALYSIS)) {
      affect_from_char(victim,SPELL_PARALYSIS);
      act("A warm feeling runs through your body.",FALSE,victim,0,0,TO_CHAR);
      act("$N looks better.",FALSE,ch,0,victim,TO_ROOM);
  }

}






void spell_succor(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct obj_data *o;

  o = get_obj(3052);
  obj_to_char(o,ch);

  act("$n waves $s hand, and creates $p", TRUE, ch, o, 0, TO_ROOM);
  act("You wave your hand and create $p.", TRUE, ch, o, 0, TO_CHAR);

}

void spell_detect_charm(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{

}

void spell_true_seeing(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(victim && ch);

  if (!IS_AFFECTED(victim, AFF_TRUE_SIGHT)) {
    if (ch != victim) {
       send_to_char("Your eyes glow silver for a moment.\n", victim);
       act("$n's eyes take on a silvery hue.\n", FALSE, victim, 0, 0, TO_ROOM);
    } else {
       send_to_char("Your eyes glow silver.\n", ch);
       act("$n's eyes glow silver.\n", FALSE, ch, 0, 0, TO_ROOM);
    }

    af.type      = SPELL_TRUE_SIGHT;
    af.duration  = 2+level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    init_set(af.bitvector,AFF_TRUE_SIGHT);
    affect_to_char(victim, &af);
  } else {
    send_to_char("Nothing seems to happen\n", ch);
  }
}




/*
   magic user spells
*/

void spell_poly_self(int level, struct char_data *ch, struct char_data *mob, struct obj_data *obj, int eff_level)
{
#if 0
  /*
   *  Check to make sure that there is no snooping going on.
   */
   for (i=0; i<2; i++)
   if (!ch->desc || ch->desc->snoop[i].snoop_by || ch->desc->snoop[i].snooping) {
      send_to_char("Godly interference prevents the spell from working.", ch);
      return;
   }
#endif

  /*
   * Put mobile in correct room
   */

  char_to_room(mob, ch->in_room);

  SwitchStuff(ch, mob);

  /*
   *  move char to storage
   */

  act("$n's flesh melts and flows into the shape of $N",
      TRUE, ch, 0, mob, TO_ROOM);

  act("Your flesh melts and flows into the shape of $N",
      TRUE, ch, 0, mob, TO_CHAR);

  char_from_room(ch);
  char_to_room(ch, 3);

  /*
   *  switch caster into mobile
   */

  ch->desc->character = mob;
  ch->desc->original = ch;

  mob->desc = ch->desc;
  ch->desc = 0;

  SET_BIT(mob->specials.act, ACT_POLYSELF);

}

void spell_minor_create(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{

  assert(ch && obj);

  act("$n claps $s hands together.", TRUE, ch, 0, 0, TO_ROOM);
  act("You clap your hands together.", TRUE, ch, 0, 0, TO_CHAR);
  act("In a flash of light, $p appears.", TRUE, ch, obj, 0, TO_ROOM);
  act("In a flash of light, $p appears.", TRUE, ch, obj, 0, TO_CHAR);

  obj_to_room(obj, ch->in_room);

}


void spell_stone_skin(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(ch);

  if (!affected_by_spell(ch, SPELL_STONE_SKIN)) {
    act("$n's skin turns grey and granite-like.", TRUE, ch, 0, 0, TO_ROOM);
    act("Your skin turns to a stone-like substance.", TRUE, ch, 0, 0, TO_CHAR);

    af.type      = SPELL_STONE_SKIN;
    af.duration  = level;
    af.modifier  = -10;
    af.location  = APPLY_ALL_AC;
    init_bits(af.bitvector);
    affect_to_char(ch, &af);
    af.modifier  = 3;
    af.location  = APPLY_ALL_STOPPING;
    affect_to_char(ch,&af);

    /* resistance to piercing weapons */

    af.type      = SPELL_STONE_SKIN;
    af.duration  = level;
    af.modifier  = 32;
    af.location  = APPLY_IMMUNE;
    init_bits(af.bitvector);
    affect_to_char(ch, &af);
  }
}



void spell_infravision(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(victim && ch);

  if (!IS_AFFECTED(victim, AFF_INFRAVISION)) {
    if (ch != victim) {
       send_to_char("Your eyes glow red.\n", victim);
       act("$n's eyes glow red.\n", FALSE, victim, 0, 0, TO_ROOM);
    } else {
       send_to_char("Your eyes glow red.\n", ch);
       act("$n's eyes glow red.\n", FALSE, ch, 0, 0, TO_ROOM);
    }

    af.type      = SPELL_INFRAVISION;
    af.duration  = 4+level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    init_set(af.bitvector,AFF_INFRAVISION);
    affect_to_char(victim, &af);

  }
}

void spell_shield(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(victim && ch);

  if (!affected_by_spell(victim, SPELL_SHIELD)) {
    act("$N is surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_NOTVICT);
    if (ch != victim) {
       act("$N is surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_CHAR);
       act("You are surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_VICT);
     } else {
       act("You are surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_CHAR);
     }

    af.type      = SPELL_SHIELD;
    af.duration  = 4+level;
    af.modifier  = -7;
    af.location  = APPLY_ALL_AC;
    init_bits(af.bitvector);
    affect_to_char(victim, &af);
  }
}

void spell_weakness(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  float modifier;

  assert(ch && victim);

  if (!affected_by_spell(victim,SPELL_WEAKNESS))
     if (!saves_spell(victim, SAVING_SPELL)) {
        modifier = (577.0 - level)/100.0;
        act("You feel weaker.", FALSE, victim,0,0,TO_VICT);
        act("$n seems weaker.", FALSE, victim, 0, 0, TO_ROOM);

        af.type      = SPELL_WEAKNESS;
        af.duration  = (int) level/10;
        af.modifier  = (int) 0 - (victim->abilities.str * modifier);
        af.location  = APPLY_STR;
        init_bits(af.bitvector);

        affect_to_char(victim, &af);
      }
}

#if 0
 void spell_invis_group(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct char_data *tmp_victim, *temp;
  struct affected_type af;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

   for ( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
	tmp_victim = tmp_victim->next_in_room) {
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim))
         if (in_group(ch,tmp_victim)) {
       	    if (!affected_by_spell(tmp_victim, SPELL_INVISIBLE)) {

	       act("$n slowly fades out of existence.", TRUE, tmp_victim,0,0,TO_ROOM);
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
}
#endif


void spell_acid_blast(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{

  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(ELVL20*2,8);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_ACID_BLAST);

}

void spell_cone_of_cold(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{

  int dam;
  struct char_data *tmp_victim, *temp;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(ELVL20*3,8);

  send_to_char("A cone of freezing air fans out before you\n", ch);
  act("$n sends a cone of ice shooting from the fingertips!\n",
	  FALSE, ch, 0, 0, TO_ROOM);

   for ( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
	tmp_victim = temp) {
	temp = tmp_victim->next_in_room;
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
         if ((GetMaxLevel(tmp_victim)>LOW_IMMORTAL) && (!IS_NPC(tmp_victim)))
            return;
         if (!in_group(ch, tmp_victim)) {
            act("You are chilled to the bone!\n",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
            if ( saves_spell(tmp_victim, SAVING_SPELL) )
                dam >>= 1;
   	    damage(ch, tmp_victim, dam, SPELL_CONE_OF_COLD);
	 } else {
            act("You are able to avoid the cone!\n", FALSE, ch, 0, tmp_victim, TO_VICT);
	  }
       }
    }
}

void spell_ice_storm(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;
  struct char_data *tmp_victim, *temp;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(ELVL20*2,10);

  send_to_char("You conjure a storm of ice\n", ch);
  act("$n conjures an ice storm!\n",
	  FALSE, ch, 0, 0, TO_ROOM);

   for ( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
	tmp_victim = temp) {
	temp = tmp_victim->next_in_room;
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
         if ((GetMaxLevel(tmp_victim)>LOW_IMMORTAL) && (!IS_NPC(tmp_victim)))
            return;
         if (!in_group(ch, tmp_victim)) {
            act("You are blasted by the storm\n",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
            if ( saves_spell(tmp_victim, SAVING_SPELL) )
                dam >>= 1;
   	    damage(ch, tmp_victim, dam, SPELL_ICE_STORM);
	 } else {
            act("You are able to dodge the storm!\n",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
	  }
       }
    }
}


void spell_poison_cloud(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{



}

void spell_chain_lightn(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int lev = ELVL20;
  struct char_data *t, *next;
  damage(ch, victim, dice(lev,12), SPELL_LIGHTNING_BOLT);

  lev = lev/2;

  for (t = real_roomp(ch->in_room)->people; t; t=next) {
    next = t->next_in_room;
    if (!in_group(ch, t) && t != victim && !IS_IMMORTAL(t)) {
      damage(ch, t, dice(lev,12), SPELL_LIGHTNING_BOLT);
      lev--;
    }
  }
}


void spell_major_create(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
}

void spell_summon_obj(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
}

void spell_sending(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
}

void spell_meteor_swarm(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(ELVL20*5,9);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_METEOR_SWARM);

}

void spell_Create_Monster(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;
  struct char_data *mob;
  int rnum;

   /* load in a monster of the correct type, determined by
      level of the spell */

/* really simple to start out with */

   if (level <= 5) {
      rnum = number(1,10)+200;
      mob = get_mob(rnum);
   } else if (level <= 7) {
      rnum = number(1,10)+210;
      mob = get_mob(rnum);
   } else if (level <= 9) {
      rnum = number(1,10)+220;
      mob = get_mob(rnum);
   } else if (level <= 11) {
      rnum = number(1,10)+230;
      mob = get_mob(rnum);
   } else if (level <= 13) {
      rnum = number(1,10)+240;
      mob = get_mob(rnum);
   } else if (level <= 15) {
      rnum = 251;
      mob = get_mob(rnum);
   } else {
      rnum = 261;
      mob = get_mob(rnum);
   }


    char_to_room(mob, ch->in_room);

    act("$n waves $s hand, and $N appears!", TRUE, ch, 0, mob, TO_ROOM);
    act("You wave your hand, and $N appears!", TRUE, ch, 0, mob, TO_CHAR);

   /* charm them for a while */
    if (mob->master)
      stop_follower(mob);

    add_follower(mob, ch);

    af.type      = SPELL_CHARM_PERSON;

    if (GET_INT(mob))
      af.duration  = 24*18/GET_INT(mob);
    else
      af.duration  = 24*18;

    af.modifier  = 0;
    af.location  = 0;
    init_set(af.bitvector,AFF_CHARM);
    affect_to_char(mob, &af);


/*
  adjust the bits...
*/

/*
 get rid of aggressive, add sentinel
*/

  if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
    CLEAR_BIT(mob->specials.act, ACT_AGGRESSIVE);
  }
  if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
    SET_BIT(mob->specials.act, ACT_SENTINEL);
  }


}




/*
   either
*/

void spell_light(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{

/*
   creates a ball of light in the hands.
*/
  struct obj_data *tmp_obj;

  assert(ch);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  tmp_obj->name = mystrdup("ball light");
  tmp_obj->short_description = mystrdup("A ball of light");
  tmp_obj->description = mystrdup("There is a ball of light on the ground here.");

  tmp_obj->obj_flags.type_flag = ITEM_LIGHT;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[2] = 24+level;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 10;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_char(tmp_obj,ch);

  act("$n twiddles $s thumbs and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("You twiddle your thumbs and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);

}

void spell_fly(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(ch && victim);

  act("You feel lighter than air!", TRUE, ch, 0, victim, TO_VICT);
  if (victim != ch) {
     act("$N's feet rise off the ground.", TRUE, ch, 0, victim, TO_CHAR);
   } else {
     send_to_char("Your feet rise up off the ground.", ch);
   }
  act("$N's feet rise off the ground.", TRUE, ch, 0, victim, TO_NOTVICT);

    af.type      = SPELL_FLY;
    af.duration  = GetMaxLevel(ch)+3;
    af.modifier  = 0;
    af.location  = 0;
    init_set(af.bitvector,AFF_FLYING);
    affect_to_char(victim, &af);
}

void spell_refresh(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(5,4)+level;
  dam = MAX(dam,20);

  if ( (dam + GET_MOVE(victim)) > move_limit(victim) )
    GET_MOVE(victim) = move_limit(victim);
  else
    GET_MOVE(victim) += dam;

  send_to_char("You feel less tired\n", victim);

}


void spell_water_breath(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{

  struct affected_type af;

  assert(ch && victim);

  act("You feel fishy!", TRUE, ch, 0, victim, TO_VICT);
  if (victim != ch) {
     act("$N makes a face like a fish.", TRUE, ch, 0, victim, TO_CHAR);
   }
  act("$N makes a face like a fish.", TRUE, ch, 0, victim, TO_NOTVICT);

    af.type      = SPELL_WATER_BREATH;
    af.duration  = GET_LEVEL(ch)+3;
    af.modifier  = 0;
    af.location  = 0;
    init_set(af.bitvector,AFF_WATERBREATH);
    affect_to_char(victim, &af);


}



void spell_cont_light(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
/*
   creates a ball of light in the hands.
*/
  struct obj_data *tmp_obj;

  assert(ch);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  tmp_obj->name = mystrdup("ball light");
  tmp_obj->short_description = mystrdup("A bright ball of light");
  tmp_obj->description = mystrdup("There is a bright ball of light on the ground here.");

  tmp_obj->obj_flags.type_flag = ITEM_LIGHT;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[2] = -1;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 40;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_char(tmp_obj,ch);

  act("$n twiddles $s thumbs and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("You twiddle your thumbs and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);

}

void spell_animate_dead(int level, struct char_data *ch, struct char_data *victim, struct obj_data *corpse,int eff_level)
{
	struct char_data *mob;
	struct obj_data *obj_object, *next_obj;
	char buf[MAX_STRING_LENGTH];
	int r_num=100; /* virtual # for zombie */
/*
 some sort of check for corpse hood
*/
	if (!corpse_consent(ch,corpse)) return;
        if ((GET_ITEM_TYPE(corpse)!=ITEM_CONTAINER)||
	    (!corpse->obj_flags.value[3])) {
	  send_to_char("The magic fails abruptly!\n",ch);
	  return;
	}

        mob = get_mob(r_num);
      	char_to_room(mob, ch->in_room);

       	act("With mystic power, $n animates a corpse.", TRUE, ch,
			0, 0, TO_ROOM);
       	act("$N slowly rises from the ground.", FALSE, ch, 0, mob, TO_ROOM);

/*
  zombie should be charmed and follower ch
*/

       	set_bit(mob->specials.affected_by, AFF_CHARM);
	GET_EXP(mob) = 10*GET_LEVEL(ch);
       	add_follower(mob, ch);
       	IS_CARRYING_M(mob) = 0;
       	IS_CARRYING_V(mob) = 0;

/*
        mob->killer = obj->killer;
*/
/*
  take all from corpse, and give to zombie
*/

       	for (obj_object=corpse->contains; obj_object; obj_object=next_obj) {
     	    next_obj = obj_object->next_content;
       	    obj_from_obj(obj_object);
	    obj_to_char(obj_object, mob);
        }

/*
   set up descriptions and such
*/
    sprintf(buf,"%s is here, slowly animating\n",corpse->short_description);
      mob->player.long_descr = mystrdup(buf);

/*
  set up hitpoints
*/

	mob->points.max_hit = dice(((level/4)+1),6);
	mob->points.hit = (int)(mob->points.max_hit/2);

    	mob->player.level = ch->player.level;
	mob->player.sex = 0;

	GET_RACE(mob) = RACE_UNDEAD;
	mob->player.class = ch->player.class;

/*
  get rid of corpse
*/
	extract_obj(corpse);

}

void spell_know_alignment(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
   int ap;
   char buf[200], name[100];

   assert(victim && ch);

   if (IS_NPC(victim))
      strcpy(name,victim->player.short_descr);
   else
      strcpy(name,GET_NAME(victim));

   ap = GET_ALIGNMENT(victim);

   if (ap > 700)
      sprintf(buf,"%s has an aura as white as the driven snow.\n",name);
   else if (ap > 350)
      sprintf(buf, "%s is of excellent moral character.\n",name);
   else if (ap > 100)
      sprintf(buf, "%s is often kind and thoughtful.\n",name);
   else if (ap > 25)
      sprintf(buf, "%s isn't a bad sort...\n",name);
   else if (ap > -25)
      sprintf(buf, "%s doesn't seem to have a firm moral commitment\n",name);
   else if (ap > -100)
    sprintf(buf, "%s isn't the worst you've come across\n",name);
   else if (ap > -350)
    sprintf(buf, "%s could be a little nicer, but who couldn't?\n",name);
   else if (ap > -700)
    sprintf(buf, "%s probably just had a bad childhood\n",name);
   else
     sprintf(buf,"I'd rather just not say anything at all about %s\n",name);

   send_to_char(buf,ch);

}

void spell_dispel_magic(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
   int yes=0;

   assert(ch && victim);

/* gets rid of infravision, invisibility, detect, etc */

   if (GetMaxLevel(victim)<=GetMaxLevel(ch))
      yes = TRUE;
   else
     yes = FALSE;

    if (affected_by_spell(victim,SPELL_INVISIBLE))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
         affect_from_char(victim,SPELL_INVISIBLE);
         send_to_char("You feel exposed.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_DETECT_INVISIBLE))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
         affect_from_char(victim,SPELL_DETECT_INVISIBLE);
         send_to_char("You feel less perceptive.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_DETECT_EVIL))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
         affect_from_char(victim,SPELL_DETECT_EVIL);
         send_to_char("You feel less morally alert.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_DETECT_MAGIC))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_DETECT_MAGIC);
        send_to_char("You stop noticing the magic in your life.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_SENSE_LIFE))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_SENSE_LIFE);
        send_to_char("You feel less in touch with living things.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_SANCTUARY)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_SANCTUARY);
        send_to_char("You don't feel so invulnerable anymore.\n",victim);
        act("The white glow around $n's body fades.",FALSE,victim,0,0,TO_ROOM);
      }
      /*
       *  aggressive Act.
       */
      if ((victim->attackers < 6) && (!victim->specials.fighting) &&
	    (IS_NPC(victim))) {
	  set_fighting(victim, ch);
	}
    }
    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
	clear_bit(victim->specials.affected_by, AFF_SANCTUARY);
	send_to_char("You don't feel so invulnerable anymore.\n",victim);
	act("The white glow around $n's body fades.",FALSE,victim,0,0,TO_ROOM);      }
      /*
       *  aggressive Act.
       */
      if ((victim->attackers < 6) && (!victim->specials.fighting) &&
	  (IS_NPC(victim))) {
	set_fighting(victim, ch);
      }
    }
    if (affected_by_spell(victim,SPELL_PROTECT_FROM_EVIL))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_PROTECT_FROM_EVIL);
        send_to_char("You feel less morally protected.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_INFRAVISION))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_INFRAVISION);
        send_to_char("Your sight grows dimmer.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_SLEEP))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_SLEEP);
        send_to_char("You don't feel so tired.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_CHARM_PERSON))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_CHARM_PERSON);
        send_to_char("You feel less enthused about your master.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_WEAKNESS))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_WEAKNESS);
        send_to_char("You don't feel so weak.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_STRENGTH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_STRENGTH);
        send_to_char("You don't feel so strong.\n",victim);
    }

    if (affected_by_spell(victim,SPELL_ARMOR))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_ARMOR);
        send_to_char("You don't feel so well protected.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_DETECT_POISON))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_DETECT_POISON);
        send_to_char("You don't feel so sensitive to fumes.\n",victim);
    }

    if (affected_by_spell(victim,SPELL_BLESS))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_BLESS);
        send_to_char("You don't feel so blessed.\n",victim);
    }

    if (affected_by_spell(victim,SPELL_FLY))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FLY);
        send_to_char("You don't feel lighter than air anymore.\n",victim);
    }

    if (affected_by_spell(victim,SPELL_WATER_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_WATER_BREATH);
        send_to_char("You don't feel so fishy anymore.\n",victim);
    }

    if (affected_by_spell(victim,SPELL_FIRE_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FIRE_BREATH);
        send_to_char("You don't feel so fiery anymore.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_LIGHTNING_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_LIGHTNING_BREATH);
        send_to_char("You don't feel so electric anymore.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_GAS_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_GAS_BREATH);
        send_to_char("You don't have gas anymore.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_FROST_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FROST_BREATH);
        send_to_char("You don't feel so frosty anymore.\n",victim);
    }
    if (affected_by_spell(victim,SPELL_FIRESHIELD))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FIRESHIELD);
        send_to_char("You don't feel so firey anymore.\n",victim);
	act("The red glow around $n's body fades.", TRUE, ch, 0, 0, TO_ROOM);
    }
    if (affected_by_spell(victim,SPELL_FAERIE_FIRE))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FAERIE_FIRE);
        send_to_char("You don't feel so pink anymore.\n",victim);
	act("The pink glow around $n's body fades.", TRUE, ch, 0, 0, TO_ROOM);
    }


   if (level == IMPLEMENTOR)  {
    if (affected_by_spell(victim,SPELL_BLINDNESS)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_BLINDNESS);
        send_to_char("Your vision returns.\n",victim);
      }
    }
    if (affected_by_spell(victim,SPELL_PARALYSIS)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_PARALYSIS);
        send_to_char("You feel freedom of movement.\n",victim);
      }
    }
    if (affected_by_spell(victim,SPELL_POISON)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_POISON);
      }
    }
   }

}



void spell_paralyze(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(victim);

 if(GET_POS(victim)==POSITION_FIGHTING)
  {
   sendf(ch,"No Way! You cant paralize someone fighting!\n");
   return;
  }


  if (!IS_AFFECTED(victim, AFF_PARALYSIS)) {
    if (IsImmune(victim, IMM_HOLD)) {
      FailPara(victim, ch);
      return;
    }
    if (IsResist(victim, IMM_HOLD)) {
      if (saves_spell(victim, SAVING_PARA)) {
	FailPara(victim, ch);
	return;
      }
      if (saves_spell(victim, SAVING_PARA)) {
	FailPara(victim, ch);
	return;
      }
    } else if (!IsSusc(victim, IMM_HOLD)) {
      if (saves_spell(victim, SAVING_PARA)) {
	FailPara(victim, ch);
	return;
      }
    }

    af.type      = SPELL_PARALYSIS;
    af.duration  = 2;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    init_set(af.bitvector,AFF_PARALYSIS);
    affect_join(victim, &af, FALSE, FALSE);

    act("Your limbs freeze in place",FALSE,victim,0,0,TO_CHAR);
    act("$n is paralyzed!",TRUE,victim,0,0,TO_ROOM);
    GET_POS(victim)=POSITION_STUNNED;

  } else {
    send_to_char("Someone tries to paralyze you AGAIN!\n",victim);
  }
}

#if 0
void spell_fear(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(victim && ch);

  if (GetMaxLevel(ch) >= GetMaxLevel(victim)-2) {
     if ( !saves_spell(victim, SAVING_SPELL))  {

/*
        af.type      = SPELL_FEAR;
        af.duration  = 4;
        af.modifier  = 0;
        af.location  = APPLY_NONE;
	init_bits(af.bitvector);

        affect_join(victim, &af, FALSE, FALSE);
*/
	do_flee(victim, "", 0);

      } else {
	send_to_char("You feel afraid, but the effect fades.\n",victim);
	return;
      }
   }
}
#endif

void spell_prot_align_group(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
}

#if 0
 void spell_calm(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  assert(ch && victim);
/*
   removes aggressive bit from monsters
*/
  if (IS_NPC(victim)) {
     if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
       if (!saves_spell(victim, SAVING_PARA)) {
          CLEAR_BIT(victim->specials.act, ACT_AGGRESSIVE);
	} else {
	  FailCalm(victim, ch);
	}
     } else {
       send_to_char("You feel calm\n", victim);
     }
  } else {
    send_to_char("You feel calm.\n", victim);
  }
}
#endif


void spell_conjure_elemental(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  /*
   *   victim, in this case, is the elemental
   *   object could be the sacrificial object
   */

   assert(ch && victim && obj);

   /*
   ** objects:
   **     fire  : red stone
   **     water : pale blue stone
   **     earth : grey stone
   **     air   : clear stone
   */

   act("$n gestures, and a cloud of smoke appears", TRUE, ch, 0, 0, TO_ROOM);
   act("$n gestures, and a cloud of smoke appears", TRUE, ch, 0, 0, TO_CHAR);
   act("$p explodes with a loud BANG!", TRUE, ch, obj, 0, TO_ROOM);
   act("$p explodes with a loud BANG!", TRUE, ch, obj, 0, TO_CHAR);
   obj_from_char(obj);
   extract_obj(obj);
   char_to_room(victim, ch->in_room);
   act("Out of the smoke, $N emerges", TRUE, ch, 0, victim, TO_NOTVICT);

   /* charm them for a while */
    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type      = SPELL_CHARM_PERSON;
    af.duration  = 48;
    af.modifier  = 0;
    af.location  = 0;
    init_set(af.bitvector,AFF_CHARM);

    affect_to_char(victim, &af);

}

void spell_faerie_fire (int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(ch && victim);

  if (affected_by_spell(victim, SPELL_FAERIE_FIRE)) {
    send_to_char("Nothing new seems to happen",ch);
    return;
  }

     act("$n points at $N.", TRUE, ch, 0, victim, TO_ROOM);
     act("You point at $N.", TRUE, ch, 0, victim, TO_CHAR);
     act("$N is surrounded by a pink outline", TRUE, ch, 0, victim, TO_ROOM);
     act("$N is surrounded by a pink outline", TRUE, ch, 0, victim, TO_CHAR);

    af.type      = SPELL_FAERIE_FIRE;
    af.duration  = 6;
    af.modifier  = 10;
    af.location  = APPLY_ARMOR;
    init_bits(af.bitvector);

    affect_to_char(victim, &af);

}

void spell_faerie_fog (int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
   struct char_data *tmp_victim, *temp;

  assert(ch);

  act("$n snaps $s fingers, and a cloud of purple smoke billows forth",
      TRUE, ch, 0, 0, TO_ROOM);
  act("You snap your fingers, and a cloud of purple smoke billows forth",
      TRUE, ch, 0, 0, TO_CHAR);


   for ( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
	tmp_victim = temp) {
	temp = tmp_victim->next_in_room;
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
         if (IS_IMMORTAL(tmp_victim))
            break;
         if (!in_group(ch, tmp_victim)) {
	   if (IS_AFFECTED(tmp_victim, AFF_INVISIBLE)) {
            if ( saves_spell(tmp_victim, SAVING_SPELL) ) {
	      clear_bit(tmp_victim->specials.affected_by, AFF_INVISIBLE);
	      act("$n is briefly revealed, but dissapears again.",
		  TRUE, tmp_victim, 0, 0, TO_ROOM);
	      act("You are briefly revealed, but dissapear again.",
		  TRUE, tmp_victim, 0, 0, TO_CHAR);
	      set_bit(tmp_victim->specials.affected_by, AFF_INVISIBLE);
	    } else {
	      clear_bit(tmp_victim->specials.affected_by, AFF_INVISIBLE);
	      act("$n is revealed!",
		  TRUE, tmp_victim, 0, 0, TO_ROOM);
	      act("You are revealed!",
		  TRUE, tmp_victim, 0, 0, TO_CHAR);
	    }
	   }
	 }
       }
    }
}



void spell_cacaodemon(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;

  assert(ch && victim && obj);

   act("$n gestures, and a black cloud of smoke appears", TRUE, ch, 0, 0, TO_ROOM);
   act("$n gestures, and a black cloud of smoke appears", TRUE, ch, 0, 0, TO_CHAR);
   act("$p bursts into flame and disintegrates!", TRUE, ch, obj, 0, TO_ROOM);
   act("$p bursts into flame and disintegrates!", TRUE, ch, obj, 0, TO_CHAR);
   obj_from_char(obj);
   extract_obj(obj);
   char_to_room(victim, ch->in_room);

   act("With an evil laugh, $N emerges from the smoke", TRUE, ch, 0, victim, TO_NOTVICT);

   /* charm them for a while */
    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type      = SPELL_CHARM_PERSON;
    af.duration  = 48;
    af.modifier  = 0;
    af.location  = 0;
    init_set(af.bitvector,AFF_CHARM);

    affect_to_char(victim, &af);

    if (IS_SET(victim->specials.act, ACT_AGGRESSIVE))
      CLEAR_BIT(victim->specials.act, ACT_AGGRESSIVE);

    if (!IS_SET(victim->specials.act, ACT_SENTINEL))
      SET_BIT(victim->specials.act, ACT_SENTINEL);

}

/*
 neither
*/

void spell_improved_identify(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
}



void spell_geyser(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int dam;

  struct char_data *tmp_victim, *temp;

  if (ch->in_room<0)
    return;
	dam =  dice(level,3);

  act("The Geyser erupts in a huge column of steam!\n",
	  FALSE, ch, 0, 0, TO_ROOM);


   for(tmp_victim = real_roomp(ch->in_room)->people; tmp_victim; tmp_victim = temp) {
      temp = tmp_victim->next_in_room;
      if ((ch != tmp_victim) && (ch->in_room == tmp_victim->in_room)) {
            if ((GetMaxLevel(tmp_victim)<LOW_IMMORTAL)||(IS_NPC(tmp_victim))) {
      	       damage(ch, tmp_victim, dam, SPELL_GEYSER);
               act("You are seared by the boiling water!!\n",
                   FALSE, ch, 0, tmp_victim, TO_VICT);
	    } else {
               act("You are almost seared by the boiling water!!\n",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
	    }
      }
    }
}



void spell_green_slime(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
	int dam;
	int hpch;

	assert(victim && ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL));

	hpch = GET_MAX_HIT(ch);
	if(hpch<10) hpch=10;

	dam = (int)(hpch/10);

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	send_to_char("You are attacked by green slime!\n",victim);

	damage(ch, victim, dam, SPELL_GREEN_SLIME);

}


struct char_data *find_random_mob(struct char_data *ch)
{
  struct char_data *i;
  int count=0;

  for (i=character_list; i; i=i->next)
    if (IS_MOB(i) && CAN_SEE(ch,i)) count++;
  count=number(1,count);
  for (i=character_list; i; i=i->next)
    if (IS_MOB(i) && CAN_SEE(ch,i))
	if (!--count) return(i);
  return(0);
}

#define PORTAL 31
 
void spell_portal(int level, struct char_data *ch, struct char_data *tmp_ch, struct obj_data *obj, int eff_level)
 {
  /* create a magic portal */
  struct obj_data *tmp_obj;
  struct extra_descr_data *ed;
  struct room_data *rp;
  char buf[512];
 
  assert(ch);
  assert((level >= 0) && (level <= ABS_MAX_LVL));
 
  if (number(1,100)<20 || IS_IMMORTAL(tmp_ch)) /*tmp_ch=find_random_mob(ch);*/
  if (ROOM_FLAG(tmp_ch->in_room,NO_GOTO) ||
	ROOM_FLAG(tmp_ch->in_room,LORD_ONLY) ||
	ROOM_FLAG(tmp_ch->in_room,PRIVATE) ||
        (IN_ARENA(ch) && !IN_ARENA(tmp_ch)) ||
        (IN_ARENA(tmp_ch) && !IN_ARENA(ch)) ||
	ROOM_FLAG(tmp_ch->in_room,ARENA) ||
        ROOM_FLAG(tmp_ch->in_room,PEACEFUL) ||
	ROOM_FLAG(tmp_ch->in_room,NO_MAGIC) ||
        IS_AFFECTED(tmp_ch, AFF_NOSUMMON) ||
        IS_IMMORTAL(tmp_ch)) {
	sendf(ch,"You failed.\n");
	return;
  }

  if ((IS_NPC(ch)) || (IS_NPC(tmp_ch))) {
    sendf(ch,"Sorry portal is player to player only now.\n");
    return;
  }

  if (!tmp_ch) {
    sendf(ch,"Oops... someone messed something up.\n");
    return;
  }

  rp = real_roomp(tmp_ch->in_room);
  tmp_obj = get_obj(PORTAL);
  if (!rp || !tmp_obj) {
    send_to_char("The magic fails\n", ch);
    return;
  }
 
  sprintf(buf, "Through the mists of the portal, you can faintly see %s", rp->name);
 
  CREATE(ed , struct extra_descr_data, 1);
  ed->next = tmp_obj->ex_description;
  tmp_obj->ex_description = ed;
  CREATE(ed->keyword, char, strlen(tmp_obj->name) + 1);
  strcpy(ed->keyword, tmp_obj->name);
  ed->description = mystrdup(buf);
 
  tmp_obj->obj_flags.timer2 = level/100;
  tmp_obj->obj_flags.value[1] = tmp_ch->in_room;
 
  obj_to_room(tmp_obj,ch->in_room);
 
  act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);
 
 }

void spell_pword_kill(int  level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
 
  if (GET_MAX_HIT(victim) <= 120) {
    damage(ch, victim, GET_MAX_HIT(victim)*12, SPELL_PWORD_KILL);
  } else {
    send_to_char("They are too powerful to destroy this way\n", ch);
  }
 
}
 
void spell_pword_blind(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
 
  if (GET_MAX_HIT(victim) <= 100) {
    set_bit(victim->specials.affected_by, AFF_BLIND);
  } else {
    send_to_char("They are too powerful to blind this way\n", ch);
  }
 
}
 
 
void spell_scare(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  if (GetMaxLevel(victim) <= 5)
    do_flee(victim, "\0", 0);
}
 
#define KITTEN  3090
#define PUPPY   3091
#define BEAGLE  3092
#define ROTT    3093
#define WOLF    3094
#define GOLEM  17007
void spell_familiar(int level, struct char_data *ch, struct char_data **victim, struct obj_data *obj, int eff_level)
{
 
  struct affected_type af;
  struct char_data *f;
 
  if (affected_by_spell(ch, SPELL_FAMILIAR)) {
    send_to_char("You can't have more than 1 familiar per day\n",ch);
    return;
  }
 
  /*
    depending on the level, one of the pet shop kids.
    */
 
  if (level < 2)
    f = get_mob(KITTEN);
  else if (level < 4)
    f = get_mob(PUPPY);
  else if (level < 6)
    f = get_mob(BEAGLE);
  else if (level < 8)
    f = get_mob(ROTT);
  else if (level < 12)
    f = get_mob(WOLF);
  else if (level < 30)
    f = get_mob(WOLF);
  else
   f = get_mob(GOLEM);
 
  char_to_room(f, ch->in_room);
 
 
  af.type      = SPELL_FAMILIAR;
  af.duration  = 24;
  af.modifier  = -1;
  af.location  = APPLY_ARMOR;
  init_bits(af.bitvector);
  affect_to_char(ch, &af);
 
  act("$n appears in a flash of light!\n", FALSE, f, 0, 0, TO_ROOM);
 
  set_bit(f->specials.affected_by, AFF_CHARM);
  GET_EXP(f) = 0;
  add_follower(f, ch);
  IS_CARRYING_M(f) = 0;
  IS_CARRYING_V(f) = 0;
 
  *victim = f;
 
}
 
void spell_aid(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  /* combo bless, cure light woundsish */
  struct affected_type af;
 
  if (affected_by_spell(victim, SPELL_AID)) {
    send_to_char("Already in effect\n", ch);
    return;
  }
 
  GET_HIT(victim)+=number(1,8);
 
  update_pos(victim);
 
  act("$n looks aided", FALSE, victim, 0, 0, TO_ROOM);
  send_to_char("You feel better!\n", victim);
 
  af.type      = SPELL_AID;
  af.duration  = 10;
  af.modifier  = 1;
  af.location  = APPLY_HITROLL;
  init_bits(af.bitvector);
  affect_to_char(victim, &af);
 
}
 
void spell_slow_poison(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;
 
  assert(ch && victim);
 
  if (affected_by_spell(victim, SPELL_POISON)) {
 
    act("$n seems to fade slightly.", TRUE, victim,0,0,TO_ROOM);
    send_to_char("You feel a bit better!.\n", victim);
 
    af.type      = SPELL_SLOW_POISON;
    af.duration  = 24;
    af.modifier  = 0;
    af.location  = 0;
    init_bits(af.bitvector);
    affect_to_char(victim, &af);
  }
}
 
void spell_gust_of_wind(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct char_data *tmp_victim, *temp;
 
  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));
 
  send_to_char("You wave your hands, and a gust of wind boils forth!\n",
	       ch);
  act("$n sends a gust of wind towards you!\n",
	  FALSE, ch, 0, 0, TO_ROOM);
 
  for ( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
       tmp_victim = temp ) {
    temp = tmp_victim->next_in_room;
    if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if ((GetMaxLevel(tmp_victim)>LOW_IMMORTAL) &&(!IS_NPC(tmp_victim)))
	return;
      if (!in_group(ch, tmp_victim)) {
	if ( saves_spell(tmp_victim, SAVING_SPELL) )
	  continue;
	GET_POS(tmp_victim) = POSITION_SITTING;
      } else {
	act("You are able to avoid the swirling gust\n",
	    FALSE, ch, 0, tmp_victim, TO_VICT);
      }
    }
  }
}
 
void spell_warp_weapon(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
 
  assert(ch && (victim || obj));
 
  if (!obj) {
    if (!victim->equipment[WIELD]) {
      act("$N doesn't have a weapon wielded!", FALSE, ch, 0,
	  victim, TO_CHAR);
      return;
    }
    obj = victim->equipment[WIELD];
  }
 
  act("$p is warped and twisted by the power of the spell", FALSE,
      ch, obj, 0, TO_CHAR);
  act("$p is warped and twisted by the power of the spell", FALSE,
      ch, obj, 0, TO_ROOM);
  DamageOneItem(victim, BLOW_DAMAGE, obj);
 
  if (IS_NPC(victim))
    if (!victim->specials.fighting)
      set_fighting(victim,ch);
}
 
