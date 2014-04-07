/* **************************************************************************
 *  file: limits.c , Limit and gain control module. Part of: Perilous Realms *
 *  Usage: Procedures controling gain and limit.                             *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information.   *
 ************************************************************************* */

#include <stdio.h>
#include <assert.h>

#include <fcns.h>
#include <limits.h>
#include <utils.h>
#include <spells.h>
#include <comm.h>
#include <race.h>
#include <ticks.h>
#include <externs.h>

int total_objects;
extern int DEBUG;

int calc_speed(struct char_data *ch, int hand)
{
  int base;
  struct obj_data *weapon = NULL;

  if(!hand && IS_WEAPON(ch->equipment[WIELD])){
    weapon = ch->equipment[WIELD];
  }
  else if(hand && IS_WEAPON(ch->equipment[HOLD])){
    weapon = ch->equipment[HOLD];
  }
  if(IS_NPC(ch)){
    base = ch->specials.mult_att * 10;
  }
  else{
    base = 10 + MIN(GET_LEVEL(ch) / (CLASS(ch).speed_level ?
      CLASS(ch).speed_level : 500), CLASS(ch).speed_max);
  }
  base = (PULSE_VIOLENCE * 10) / base;
  if(weapon){
    base = (base * 10) / MAX(1, weapon->obj_flags.value[4]);
  }
  /* dex modifier */
  base = (base * 20) / MAX(1, 10 + GET_DEX(ch));

  return(MAX(1, base));
}

/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{
  
  if (age < 15)
    return(p0);                               /* < 15   */
  else if (age <= 29) 
    return (int) (p1+(((age-15)*(p2-p1))/15));  /* 15..29 */
  else if (age <= 44)
    return (int) (p2+(((age-30)*(p3-p2))/15));  /* 30..44 */
  else if (age <= 59)
    return (int) (p3+(((age-45)*(p4-p3))/15));  /* 45..59 */
  else if (age <= 79)
    return (int) (p4+(((age-60)*(p5-p4))/20));  /* 60..79 */
  else
    return(p6);                               /* >= 80 */
}

int mana_limit(struct char_data *ch)
{
  int mana;

  mana = 100 + 4*MIN(GetMaxLevel(ch),50) +
	       3*(MAX(0,MIN(GetMaxLevel(ch)-50,100))) +
	       2*(MAX(0,GetMaxLevel(ch)-150));

  if(is_cleric(ch) && is_magicu(ch) && !is_thief(ch) && !is_fighter(ch)){
    mana = (mana + mana/10);
    if(IsClass(ch, SCHOLAR)){
      mana = ((mana + mana/10) * 66) / 100;
    }
    else{
      mana = ((mana + mana/10) * 33) / 100;
    }
  }
  else if (IsClass(ch,DRUID))
    mana += (mana*3/10);
  else if (IsClass(ch,CLERIC))
    mana += (mana*4/10);
  else if (is_cleric(ch))
    mana += 0;
  else
    mana = 100;
  return(mana + ch->points.max_mana);
}

/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */
int power_limit(struct char_data *ch)
{
  int mana;

  mana = 100 + 4*MIN(GetMaxLevel(ch),50) +  /* first 50 levels */
	       3*(MAX(0,MIN(GetMaxLevel(ch)-50,100))) + /* next 100 */
	       2*(MAX(0,GetMaxLevel(ch)-150)); /* rest */

  if(is_cleric(ch) && is_magicu(ch) && !is_thief(ch) && !is_fighter(ch)){
    mana = (mana + mana/10);
    if(IsClass(ch, NECROMANCER)){
      mana = ((mana + mana/10) * 66) / 100;
    }
    else{
      mana = ((mana + mana/10) * 33) / 100;
    }
  }
  else if (IsClass(ch,ILLUSIONIST))
    mana += (mana*3/10);
  else if (IsClass(ch,MAGE))
    mana += (mana*4/10);
  else if (is_magicu(ch))
    mana += 0;
  else if (IsClass(ch,BARBARIAN))
    mana=10;
  else mana=100;
  return(mana+ch->points.max_power);
}

int hit_limit(struct char_data *ch)
{
  int max;
  
  if (!IS_NPC(ch))
    max = (ch->points.max_hit) +
      (graf(age(ch).year, 2,4,17,14,8,4,3));
  else 
    max = (ch->points.max_hit);
  
  
  /* Class/Level calculations */
  
  /* Skill/Spell calculations */
  if (IS_NPC(ch))
    return (MIN(max, 32000));
  else
    return (MIN(max,(IsClass(ch, BARBARIAN)) ? 5000 : MAX_HIT)); 
  
/*  return (MIN(max,IS_NPC(ch)?32000:MAX_HIT));  */
}


int move_limit(struct char_data *ch)
{
  int max;
  
  if (!IS_NPC(ch))
    max = 100 + age(ch).year + GET_CON(ch) + GET_LEVEL(ch);
  else
    max = ch->points.max_move;
  
  if (GET_RACE(ch) == RACE_DWARF)
    max -= 30;
  else if (GET_RACE(ch) == RACE_ELVEN)
    max += 25;
  else if (GET_RACE(ch)==RACE_GIANT)
    max += 30;

  max += ch->points.max_move;  /* move bonus */
  
  return (MIN(max,MAX_MOVE));
}



/* power point gain pr. game hour */
int power_gain(struct char_data *ch)
{
  int gain;
  
  if(IS_NPC(ch)) {
    /* Neat and fast */
    gain = GET_LEVEL(ch);
  } else {
    gain = graf(age(ch).year, 2,4,6,8,10,12,16);
    
    /* Class calculations */
    
    /* Skill/Spell calculations */
    
    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POSITION_SLEEPING:
      gain += gain;
      break;
    case POSITION_RESTING:
      gain+= (gain>>1);  /* Divide by 2 */
      break;
    case POSITION_SITTING:
      gain += (gain>>2); /* Divide by 4 */
      break;
    }

    if (is_magicu(ch) || is_cleric(ch))
      gain += gain;
  }
  
  if (GET_RACE(ch)==RACE_PIXIE) gain += 5;

  if (IS_AFFECTED(ch,AFF_POISON))
    gain >>= 2;
  
  if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))
    gain >>= 2;
  
  if (ROOM_FLAG(ch->in_room,FAST_MANA)) gain+=gain;
  return (gain);
}


/* manapoint gain pr. game hour */
int mana_gain(struct char_data *ch)
{
  int gain = 2;

  if(GET_POS(ch) == POSITION_PRAYING){
    gain = graf(age(ch).year, 2, 4, 6, 8, 10, 12, 16);
    gain += 4;
    if(IS_ALTAR(ch)){
      gain += 6;
    }
  }

  return(gain);
}

int hit_gain(struct char_data *ch)
     /* Hitpoint gain pr. game hour */
{
  int gain, dam;
  
  if(IS_NPC(ch)) {
    gain = 8;
    /* Neat and fast */
  } else {
    
    if (GET_POS(ch) == POSITION_FIGHTING) {
      gain = 0;
    } else {
      gain = graf(age(ch).year, 2,5,10,18,6,4,2);
    }
    
    /* Class/Level calculations */
    
    /* Skill/Spell calculations */
    
    /* Position calculations    */
    
    switch (GET_POS(ch)) {
    case POSITION_SLEEPING:
      gain += gain>>1;
      break;
    case POSITION_RESTING:
      gain+= gain>>2;
      break;
    case POSITION_SITTING:
      gain += gain>>3;
      break;
    }
    
  }
  if (GET_RACE(ch)==RACE_TROLL)
	gain+=2;
  
  if (GET_RACE(ch) == RACE_DWARF)
    gain += 1;
  
  if (IS_AFFECTED(ch,AFF_POISON))  {
    gain >>= 2;
    dam = 32;
    if (IsClass(ch, BARBARIAN))
      dam = 0;
    if (affected_by_spell(ch, SPELL_SLOW_POISON))
      dam /=4;
    damage(ch, ch, dam, SPELL_POISON);
 }
  
  if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))
    gain >>= 2;
  
  if (ROOM_FLAG(ch->in_room,FAST_HIT)) gain+=gain;
  return (gain);
}



int move_gain(struct char_data *ch)
     /* move gain pr. game hour */
{
  int gain;
  
  if(IS_NPC(ch)) {
    return(GET_LEVEL(ch));
    /* Neat and fast */
  } else {
    if (GET_POS(ch) != POSITION_FIGHTING)
      gain = 5 + GET_CON(ch);
    else
      gain = 0;
    
    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POSITION_SLEEPING:
      gain += (gain>>1); /* Divide by 2 */
      break;
    case POSITION_RESTING:
      gain+= (gain>>2);  /* Divide by 4 */
      break;
    case POSITION_SITTING:
      gain += (gain>>3); /* Divide by 8 */
      break;
    }
  }
  
  
  if (GET_RACE(ch) == RACE_ELVEN)
    gain += 4;
  if (GET_RACE(ch) == RACE_GIANT)
    gain += 8;
  
  if (IS_AFFECTED(ch,AFF_POISON))
    gain >>= 2;
  
  if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))
    gain >>= 2;

  if (ROOM_FLAG(ch->in_room,FAST_MOVE)) gain+=gain;
  
  return (gain);
}



/* Gain maximum in various points */
void advance_level(struct char_data *ch)
{
  int prac;
  int add_hp, h;
  
  extern struct wis_app_type wis_app[];
  extern struct int_app_type int_app[];
  extern struct con_app_type con_app[];

  if (!CAN_ADVANCE(ch)) {
    /*  they can't advance here */
	if (GET_EXP(ch)<EXP_NEEDED(GET_LEVEL(ch),GET_CLASS(ch))+1)
		log("not enough xp");
    log("Bad advance_level");
    return;
  }
  /* check to see if gained to this level in another class already */

  SET_LEVEL(ch, GET_LEVEL(ch)+1);

  if (is_fighter(ch))
     add_hp = con_app[GET_CON(ch)].hitp;
  else 
     add_hp = MIN(con_app[GET_CON(ch)].hitp,2);
    

  h=dice(CLASS(ch).hp[0],CLASS(ch).hp[1])+CLASS(ch).hp[2];

  if (GET_HIT(ch)>1000) h /= 2;

  if (GET_LEVEL(ch)<= 5)
     h++;
  h+=add_hp;

  ch->points.max_hit += MAX(1, h);

  prac = MAX(2,wis_app[GET_WIS(ch)].bonus);
  prac *= (MAX(10,int_app[GET_INT(ch)].learn));
  prac = MAX(50,prac);

  if (GET_LEVEL(ch)>ch->player.max_level) {
    ch->specials.spells_to_learn += prac;
    ch->player.max_level=GET_LEVEL(ch);
  }
}	



/* Lose in various points */

void drop_level(struct char_data *ch)
{
  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
    return;
  if (GetMaxLevel(ch) <= 1)
    return;

/* loose some hp */
  ch->points.max_hit -= (ch->points.max_hit/(GET_LEVEL(ch)+1));
  
  SET_LEVEL(ch,GET_LEVEL(ch)-1);
  
  if (ch->points.max_hit < 1)
    ch->points.max_hit = 1;
  if (CAN_ADVANCE(ch)) GET_EXP(ch)=EXP_NEEDED(GET_LEVEL(ch)+1,GET_CLASS(ch));
  sendf(ch,"You lose a level.\n");
}	



void set_title(struct char_data *ch)
{
  
  char buf[256];
  
  if (!GET_TITLE(ch)) {
    strcpy(buf,"says, \"I haven't used the TITLE command yet!\"");
    CREATE(GET_TITLE(ch),char,strlen(buf)+1);
    strcpy(GET_TITLE(ch),buf);
  }
}

void gain_exp(struct char_data *ch, slong gain)
{
  if (((IS_NPC(ch)) && (IS_AFFECTED(ch, AFF_CHARM))) || (IS_IMMORTAL(ch)))
    return;
  if (gain > 0) {
    gain = MIN(MAX_XP_GAIN(ch), gain);
    GET_EXP(ch)+=gain;

  if (!IS_NPC(ch) && HAS_EXP(ch, GET_LEVEL(ch) + 3)){
    GET_EXP(ch) = EXP_NEEDED(GET_LEVEL(ch) + 3, GET_CLASS(ch));
    sendf(ch, "You must gain at a guild before you can earn more experience!\n");
  }
  else if (!IS_NPC(ch) && CAN_ADVANCE(ch))
      sendf(ch,"You have gained enough for a level!\n");
  } else {
    if (GET_EXP(ch)> -1*gain)
      GET_EXP(ch)+=gain;
    else GET_EXP(ch)=1;
    while (!HAS_EXP(ch,GET_LEVEL(ch)) && GET_LEVEL(ch)>1) drop_level(ch);
  }
}


void gain_exp_regardless(struct char_data *ch, slong gain)
{
  int i;
  bool is_altered = FALSE;
  
  /*save_char(ch,NOWHERE);*/
  if (!IS_NPC(ch)) {
    if (gain > 0) {
      GET_EXP(ch) += gain;

      for (i=0;(i<ABS_MAX_LVL) && (EXP_NEEDED(i,GET_CLASS(ch))
		<= GET_EXP(ch)); i++) {
	if (i > GET_LEVEL(ch)) {
	  send_to_char("You raise a level\n", ch);
	  advance_level(ch);
	  is_altered = TRUE;
	}
      }
    }
    if (gain < 0) 
      GET_EXP(ch) += gain;
    if (GET_EXP(ch) < 0)
      GET_EXP(ch) = 0;
  }
}

void gain_condition(struct char_data *ch,int condition,int value)
{
  bool intoxicated;
  
  if(GET_COND(ch, condition)==-1) /* No change */
    return;
  
  intoxicated=(GET_COND(ch, DRUNK) > 0);
  
  GET_COND(ch, condition)  += value;
  
  GET_COND(ch,condition) = MAX(0,GET_COND(ch,condition));
  GET_COND(ch,condition) = MIN(24,GET_COND(ch,condition));
  
  if(GET_COND(ch,condition))
    return;
  
  switch(condition){
  case FULL :
    {
      send_to_char("You are hungry.\n",ch);
      return;
    }
  case THIRST :
    {
      send_to_char("You are thirsty.\n",ch);
      return;
    }
  case DRUNK :
    {
      if(intoxicated)
	send_to_char("You are now sober.\n",ch);
      return;
    }
    default : break;
  }
  
}


void check_idling(struct char_data *ch)
{


  ++ch->specials.timer;
  if (ch->specials.timer == 5)
    do_save(ch,"",-1);
  else if (ch->specials.timer==20) {
    if (ch->specials.was_in_room == NOWHERE && ch->in_room != NOWHERE) {
      ch->specials.was_in_room = ch->in_room;
      if (ch->specials.fighting) {
	stop_fighting(ch->specials.fighting);
	stop_fighting(ch);
      }
      act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You have been idle, and are pulled into a void.\n", ch);
      char_from_room(ch);
      char_to_room(ch, 1);  /* Into room number 1 */
    }
  } else if (ch->specials.timer > 60)  {
    char_from_room(ch);
    char_to_room(ch,ch->specials.was_in_room);
    act("Uh-oh... $n has just been arrested for vagrancy!\n",FALSE,ch,0,0,TO_ROOM);
    sendf(ch,"You have been arrested for vagrancy.\n");
/*    vlog(LOG_MISC,"LINK: %s arrested fine=%d",
	GET_NAME(ch),1000*GetMaxLevel(ch));
    fine(ch,1000*GetMaxLevel(ch),NULL); */
    ch->specials.was_in_room=NOWHERE;

    if (ch->desc) {
	extract_char(ch);
	close_socket(ch->desc);
    } else {
	save_char(ch,NOWHERE);
	extract_char(ch);
    }
  }
}





/* Update both PC's & NPC's and objects*/
void point_update( void )
{	
  struct char_data *i, *next_dude;
  struct obj_data *j,*next_thing;
  int count=0;
  
  /* characters */
  for (i = character_list; i; i = next_dude) {
    next_dude = i->next;
    if (GET_POS(i) >= POSITION_STUNNED) {
      GET_HIT(i)  = MIN(GET_HIT(i)  + hit_gain(i),  hit_limit(i));
      GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
      GET_POWER(i) = MIN(GET_POWER(i) + power_gain(i), power_limit(i));
      GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
      if (GET_POS(i) == POSITION_STUNNED) 
         update_pos( i );
    } else if (GET_POS(i) == POSITION_INCAP)
      /* do nothing */  damage(i, i, 0, TYPE_SUFFERING);
    else if (!IS_NPC(i) && (GET_POS(i) == POSITION_MORTALLYW))
      damage(i, i, 1, TYPE_SUFFERING);
    if (!IS_NPC(i))	{
      update_char_objects(i);
      if (GetMaxLevel(i) < DEMIGOD)
	check_idling(i);
    }
    gain_condition(i,FULL,-1);
    gain_condition(i,DRUNK,-1);
    gain_condition(i,THIRST,-1);
  } /* for */
  
  /* objects */
  for(j = object_list; j ; j = next_thing){
    next_thing = j->next; /* Next in object list */
    count++;

    if (j->obj_flags.timer2>0) {
      j->obj_flags.timer2--;
      if (!j->obj_flags.timer2) {
	if (j->carried_by) {
	  int room;
	  room=j->carried_by->in_room;
	  obj_from_char(j);
	  obj_to_room(j,room);
	} else if (j->in_obj) {
	  obj_from_obj(j);
	  extract_obj(j);
	  continue;
	}
	if (real_roomp(j->in_room)->people) {
	  act("$p phases out of existance.",
		TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
	  act("$p phases out of existance.",
		TRUE, real_roomp(j->in_room)->people, j, 0, TO_CHAR);
	}
	obj_from_room(j);
	extract_obj(j);
	continue;
      }
    }

    /* If this is a corpse */
    if ( (GET_ITEM_TYPE(j) == ITEM_CONTAINER) && 
	(j->obj_flags.value[3]) ) {
      /* timer count down */
      if (j->obj_flags.timer > 0) j->obj_flags.timer--;
      
      if (!j->obj_flags.timer) {
	if (j->carried_by)
	  act("$p biodegrades in your hands.", 
	      FALSE, j->carried_by, j, 0, TO_CHAR);
	else if ((j->in_room != NOWHERE) && 
		 (real_roomp(j->in_room)->people)){
	  act("$p dissolves into a fertile soil.",
	      TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
	  act("$p dissolves into a fertile soil.",
	      TRUE, real_roomp(j->in_room)->people, j, 0, TO_CHAR);
	}	
	ObjFromCorpse(j);
      }
    }
  }

  total_objects=count;
}


int ObjFromCorpse( struct obj_data *c)
{
  struct obj_data *jj, *next_thing;

        for(jj = c->contains; jj; jj = next_thing) {
	  next_thing = jj->next_content; /* Next in inventory */
	  if (jj->in_obj) {
	     obj_from_obj(jj);
	     if (c->in_obj)
	       obj_to_obj(jj,c->in_obj);
	     else if (c->carried_by)
	       obj_to_room(jj,c->carried_by->in_room);
	     else if (c->in_room != NOWHERE)
	       obj_to_room(jj,c->in_room);
	     else
	       assert(FALSE);
	   } else {
	     /*hmm..  it isn't in the object it says it is in.
	     **  deal with the memory lossage
	     */
	     c->contains = 0;
	     extract_obj(c);
             log("Memory lost in ObjFromCorpse.");
	     return(TRUE);
	   }
	}
       	extract_obj(c);
        return(TRUE);
}
