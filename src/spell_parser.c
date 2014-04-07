/* ************************************************************************
 *  file: spell_parser.c , Basic routines and parsing      Part of DIKUMUD *
 *  Usage : Interpreter of spells                                          *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <stdio.h>
#include <assert.h>

#include <fcns.h>
#include <utils.h>
#include <comm.h>
#include <db.h>
#include <interpreter.h>
#include <spells.h>
#include <handler.h>
#include <ticks.h>
#include <externs.h>
#include "spell_func.h"

extern char *spell_wear_off_msg[];


struct obj_data *SpellComponent(struct char_data *ch, int vnum)
/* locate an object either 1) held, 2) In inventory, or 3) in spell pouch */
{
  struct obj_data *obj;
  if (ch->equipment[POUCH])
    for (obj=ch->equipment[POUCH]->contains; obj; obj=obj->next_content)
      if (obj->virtual == vnum) return(obj);
  if (ch->equipment[HOLD])
    if (ch->equipment[HOLD]->virtual==vnum) return(ch->equipment[HOLD]);
  for (obj=ch->carrying; obj; obj=obj->next_content)
    if (obj->virtual == vnum) return(obj);
  return(0);
}

int is_spell_component(struct char_data *ch, int vnum)
{
  spell_entry *sp;
  int i;
  if (IS_NPC(ch)) return(0);
  for (sp=CLASS(ch).spells; sp; sp=sp->next)
    for (i=0; i<sp->n_spell_components; i++)
      if (vnum==sp->components[i]) return(1);
  return(0);
}

int has_components(struct char_data *ch, int spell)
{
  spell_entry *sp;
  int i;
  
  sp=find_spell_entry(ch,spell);
  if (!sp) return(0);

  for (i=0; i<sp->n_spell_components; i++)
    if (!SpellComponent(ch,sp->components[i])) return(0);

  return(1);
}

void use_components(struct char_data *ch, int spell)
{
  spell_entry *sp;
  struct obj_data *obj;
  int i;
  
  sp=find_spell_entry(ch,spell);
  if (!sp) return;

  for (i=0; i<sp->n_spell_components; i++) {
    obj=SpellComponent(ch,sp->components[i]);
    if (!obj) continue;
    if (GET_ITEM_TYPE(obj)==ITEM_COMPONENT) {
      if (obj->obj_flags.value[0]>1) {
	obj->obj_flags.value[0]--;
	sendf(ch,"You use some of %s in your spell.\n",obj->short_description);
	continue;
      }
    }
    if (obj->eq_pos>0 && obj->eq_pos<=MAX_WEAR)
      obj=unequip_char(ch,obj->eq_pos);
    if (obj->carried_by)
	obj_from_char(obj);
    if (obj->in_obj)
	obj_from_obj(obj);
    sendf(ch,"You use %s in your spell.\n",obj->short_description);
    extract_obj(obj);
  }
}


#include "spell_func.h"
extern char *spells[];
extern char *songs[];

void affect_update ()
{
  static struct affected_type *af, *next_af_dude;
  static struct char_data *i;

  for (i = character_list; i; i = i->next)
    for (af = i->affected; af; af = next_af_dude) {
      next_af_dude = af->next;
      if (af->duration >= 1)
	      af->duration--;
      else {
	if ((af->type > 0) && (af->type <= 69)) /* It must be a spell */ {
	  if (!af->next||(af->next->type!=af->type)||(af->next->duration>0)) {
	    if (*spell_wear_off_msg[af->type])
	      sendf(i,"%s\n",spell_wear_off_msg[af->type]);
	  }
	} else if (af->type>=FIRST_BREATH_WEAPON&&af->type<=LAST_BREATH_WEAPON){
	  extern funcp bweapons[];
	  bweapons[af->type-FIRST_BREATH_WEAPON](-af->modifier/2, i, "",
					   SPELL_TYPE_SPELL, i, 0);
	  if (!i->affected) /* oops, you're dead :) */
	    break;
	}
	affect_remove(i, af);
      }
    }
}


void clone_char (struct char_data *ch)
{
	struct char_data *clone;
	struct affected_type *af;
	int i;

	CREATE(clone, struct char_data, 1);


	clear_char(clone);       /* Clear EVERYTHING! (ASSUMES CORRECT) */

	clone->player    = ch->player;
	clone->abilities = ch->abilities;

	for (i=0; i<5; i++)
		clone->specials.apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

	for (af=ch->affected; af; af = af->next)
		affect_to_char(clone, af);

	for (i=0; i<3; i++)
		GET_COND(clone,i) = GET_COND(ch, i);

	clone->points = ch->points;

	CREATE(clone->skills,struct char_skill_data,ch->nskills);
	for (i=0; i<ch->nskills; i++)
		clone->skills[i] = ch->skills[i];

	clone->specials = ch->specials;
	clone->specials.fighting = 0;

	GET_NAME(clone) = mystrdup(GET_NAME(ch));

	clone->player.short_descr =	mystrdup(ch->player.short_descr);

	clone->player.long_descr = mystrdup(ch->player.long_descr);

	clone->player.description = 0;
	/* REMEMBER EXTRA DESCRIPTIONS */

	GET_TITLE(clone) = mystrdup(GET_TITLE(ch));

	clone->virtual=ch->virtual;

	if (IS_NPC(clone))
		real_mobp(clone->virtual)->count++;
	else { /* Make PC's into NPC's */
		clone->virtual = 1;
		SET_BIT(clone->specials.act, ACT_ISNPC);
	}

	clone->desc = 0;
	clone->followers = 0;
	clone->master = 0;

	clone->next = character_list;
	character_list = clone;

	char_to_room(clone, ch->in_room);
}



void clone_obj( struct obj_data *obj )
{
	struct obj_data *clone;

	CREATE(clone, struct obj_data, 1);

	*clone = *obj;

	clone->name               = mystrdup(obj->name);
	clone->description        = mystrdup(obj->description);
	clone->short_description  = mystrdup(obj->short_description);
	clone->action_description = mystrdup(obj->action_description);
	clone->ex_description     = 0;

	/* REMEMBER EXTRA DESCRIPTIONS */
	clone->carried_by         = 0;
	clone->equipped_by        = 0;
	clone->in_obj             = 0;
	clone->contains           = 0;
	clone->next_content       = 0;
	clone->next               = 0;

	/* VIRKER IKKE ENDNU */
}



int count_followers (struct char_data *ch)
{
	struct follow_type *k;
	int cnt = 0;

	for(k=ch->followers; k; k=k->next, cnt++)
		;

	return(cnt);
}

/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow (struct char_data *ch, struct char_data *victim)
{
	struct char_data *k;

	for(k=victim; k; k=k->master) {
		if (k == ch)
			return(TRUE);
	}

	return(FALSE);
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower ( struct char_data *ch)
{
	struct follow_type *j, *k;

	if (!ch->master) return;

	if (IS_AFFECTED(ch, AFF_CHARM)) {
		act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
		act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
		act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
		if (affected_by_spell(ch, SPELL_CHARM_PERSON))
			affect_from_char(ch, SPELL_CHARM_PERSON);
	} else {
		act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
           if (!IS_SET(ch->specials.act,PLR_STEALTH)) {
		act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
		act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
	   }
	}

	if (ch->master->followers->follower == ch) { /* Head of follower-list? */
		k = ch->master->followers;
		ch->master->followers = k->next;
		FREE(k);
	} else { /* locate follower who is not head of list */

		for(k = ch->master->followers; k->next && k->next->follower!=ch; k=k->next)  ;

		if (k->next) {
		   j = k->next;
		   k->next = j->next;
		   FREE(j);
		}
	}

	ch->master = 0;
	clear_bit(ch->specials.affected_by,AFF_CHARM);
	clear_bit(ch->specials.affected_by,AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower ( struct char_data *ch)
{
	struct follow_type *j, *k;

	if (ch->master)
		stop_follower(ch);

	for (k=ch->followers; k; k=j) {
		j = k->next;
		stop_follower(k->follower);
	}
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower (struct char_data *ch,struct char_data *leader)
{
	struct follow_type *k;

	assert(!ch->master);

	ch->master = leader;

	CREATE(k, struct follow_type, 1);

	k->follower = ch;
	k->next = leader->followers;
	leader->followers = k;

        
	act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
        if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
	   act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
	   act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
	}
}

void say_spell ( struct char_data *ch, int si)
{
	char buf[MAX_STRING_LENGTH], splwd[MAX_BUF_LENGTH];
	char buf2[MAX_STRING_LENGTH];

	int j, offs;
	struct char_data *temp_char;


	struct syllable {
		char org[10];
		char new[10];
	};

	static struct syllable syls[] = {
	{ " ", " " },
	{ "ar", "abra"   },
	{ "au", "kada"    },
	{ "bless", "fido" },
  { "blind", "nose" },
  { "bur", "mosa" },
	{ "cu", "judi" },
        { "ca", "jedi" },
	{ "de", "oculo"},
	{ "en", "unso" },
	{ "light", "dies" },
	{ "lo", "hi" },
	{ "mor", "zak" },
	{ "move", "sido" },
  { "ness", "lacri" },
  { "ning", "illa" },
	{ "per", "duda" },
	{ "ra", "gru"   },
  { "re", "candus" },
	{ "son", "sabru" },
        { "se",  "or"},
  { "tect", "infra" },
	{ "tri", "cula" },
	{ "ven", "nofo" },
	{"a", "a"},{"b","b"},{"c","q"},{"d","e"},{"e","z"},{"f","y"},{"g","o"},
	{"h", "p"},{"i","u"},{"j","y"},{"k","t"},{"l","r"},{"m","w"},{"n","i"},
	{"o", "a"},{"p","s"},{"q","d"},{"r","f"},{"s","g"},{"t","h"},{"u","j"},
	{"v", "z"},{"w","x"},{"x","n"},{"y","l"},{"z","k"}, {"",""}
	};



	strcpy(buf, "");
	strcpy(splwd, spells[si]);

	offs = 0;

	while(*(splwd+offs)) {
		for(j=0; *(syls[j].org); j++)
			if (strncmp(syls[j].org, splwd+offs, strlen(syls[j].org))==0) {
				strcat(buf, syls[j].new);
				if (strlen(syls[j].org))
					offs+=strlen(syls[j].org);
				else
					++offs;
			}
	}


	sprintf(buf2,"$n says, '%s'", buf);
	sprintf(buf, "$n mumbles, '%s'", spells[si]);

	for(temp_char = real_roomp(ch->in_room)->people;
		temp_char;
		temp_char = temp_char->next_in_room)
		if(temp_char != ch) {
/*
**  Remove-For-Multi-Class
*/
			if (ch->player.class == temp_char->player.class)
				act(buf, FALSE, ch, 0, temp_char, TO_VICT);
			else
				act(buf2, FALSE, ch, 0, temp_char, TO_VICT);

		}

}


int saving_throw_val(struct char_data *ch,int save_type)
{
  class_entry *c;
  int save;
  /* Negative apply_saving_throw makes saving throw better! */

  c=classes+GET_CLASS(ch);
  if (!IS_NPC(ch))
    save=MAX(c->saves[save_type]-GET_LEVEL(ch)/c->decrease[save_type],
		c->minsave[save_type]);
  else save=100-GET_LEVEL(ch);

  save += ch->specials.apply_saving_throw[save_type];

  return(MAX(2,MIN(100,save)));
}

bool saves_spell ( struct char_data *ch, sh_int save_type)
{
	return(number(1,100)>=saving_throw_val(ch,save_type));
}

bool ImpSaveSpell ( struct char_data *ch, sh_int save_type, int mod)
{
        /* Positive mod is better for save */
	return(number(1,100)>=saving_throw_val(ch,save_type)-mod);
}



char *skip_spaces ( char *string)
{
	for(;*string && (*string)==' ';string++);

	return(string);
}



/* Assumes that *argument does start with first letter of chopped string */

void do_cast( struct char_data *ch, char *argument, int cmd)
{
  char *p;
  char spell_name[MAX_INPUT_LENGTH];
  spell_entry *sp;
  int spl;

  if (IS_NPC(ch))
    return;

  if (!IsHumanoid(ch)) {
    send_to_char("Sorry, you don't have the right form for that.\n",ch);
    return;
  }

/*  if ((IsClass(ch,PALADIN) && !IS_GOOD(ch)) ||
      (IsClass(ch,AVENGER) && !IS_EVIL(ch))) {
    sendf(ch,"Your god prevents you from using your magic until you get back onto the\npath of %s.\n",IsClass(ch,PALADIN)?"goodness":"evil");
    return;
  } */
   
  if (!IS_IMMORTAL(ch) && ROOM_FLAG(ch->in_room,NO_MAGIC)) {
    sendf(ch,"There is something about this area that disappates your magical energy.\n");
    return;
  }
  argument = skip_spaces(argument);
  
  /* If there is no chars in argument */
  if (!(*argument)) {
    if(cmd == 129)
      send_to_char("Sing what, praytell?\n", ch);
    else
      send_to_char("Cast which what where?\n", ch);
    return;
  }
  
  if (*argument != '\'') {
    if(cmd == 129)
      sendf(ch,"You must enclose your balard within the proper frames: '\n");
    else
      send_to_char("Magic must always be enclosed by the holy magic symbols : '\n",ch);
    return;
  }
  
  /* Locate the last quote && lowercase the magic words (if any) */
  
  
  p=spell_name;
  for (argument++; *argument && *argument!= '\''; argument++,p++)
    *p = LOWER(*argument);
  *p ='\0';
  
  if (*argument != '\'') {
    if(cmd == 129)
      sendf(ch,"You must enclose your balard within the proper frames: '\n");
    else
      send_to_char("Magic must always be enclosed by the holy magic symbols : '\n",ch);
    return;
  }
  
  argument++;
  argument=skip_spaces(argument);

  for (sp=CLASS(ch).spells; sp; sp=sp->next)
    if (is_abbrev(spell_name,spells[sp->num])) break;

  if (sp)
    spl = sp->num;
  else{
    switch (number(1,5)){
    case 1: send_to_char("Bylle Grylle Grop Gryf???\n", ch); break;
    case 2: send_to_char("Olle Bolle Snop Snyf?\n",ch); break;
    case 3: send_to_char("Olle Grylle Bolle Bylle?!?\n",ch); break;
    case 4: send_to_char("Gryffe Olle Gnyffe Snop???\n",ch); break;
    default: send_to_char("Bolle Snylle Gryf Bylle?!!?\n",ch); break;
    }
    return;
  }

  phys_cast(ch, spl, NULL, argument);
}

int phys_cast(struct char_data *ch, int spl, struct char_data *vict, char * argument)
{
  spell_entry *sp;
  int i, tmp, delay;
  int mana_needed;
  int effective_level;
  int hc;

  for (sp = CLASS(ch).spells; sp; sp = sp->next)
    if (sp->num == spl) break;

  hc=has_components(ch,spl);
  if (!hc) {
    sendf(ch,"You attempt to draw on your mana store to make up for missing reagents.\n");
  }
  
  if (!spell_info[spl].spell_pointer) {
    sendf(ch,"That magic is unimplemented/disabled.\n");
    return(0);
  }
  if (GET_POS(ch) < spell_info[spl].minimum_position) {
    switch(GET_POS(ch)) {
    case POSITION_SLEEPING :
      send_to_char("You dream about great magical powers.\n", ch);
      break;
    case POSITION_RESTING :
      send_to_char("You can't concentrate enough while resting.\n",ch);
      break;
    case POSITION_SITTING :
      send_to_char("You can't do this sitting!\n", ch);
      break;
    case POSITION_FIGHTING :
      send_to_char("Impossible! You can't concentrate enough!.\n", ch);
      break;
    default:
      send_to_char("It seems like you're in pretty bad shape!\n",ch);
      break;
    } /* Switch */
    return(0);
  }
      
  if (sp->min_level>GetMaxLevel(ch)) {
    sendf(ch,"You lack the experience needed to cast this spell.\n");
    return(0);
  }
      
  i=GET_LEVEL(ch)-sp->min_level;
  effective_level=i+1;
  i=MIN(4,i);
  mana_needed = sp->mana;

  if (mana_needed<0) mana_needed= -mana_needed;
  else {
    mana_needed *= (5-i);
    if (mana_needed>200) mana_needed=200;
  }

  if (sp->min_level<=10)
    mana_needed=MAX(sp->mana,100/(MAX(1,2+GetMaxLevel(ch)-sp->min_level)));

  if (!hc) mana_needed*=(1+sp->difficulty);

  if (GetMaxLevel(ch) < LOW_IMMORTAL) {
    if(sp->source == SOURCE_MANA){
      if(((GET_TRUE_ALIGN(ch) == -1000) && !IS_EVIL(ch)) ||
          ((GET_TRUE_ALIGN(ch) == 0) && !IS_NEUTRAL(ch)) ||
          ((GET_TRUE_ALIGN(ch) == 1000) && !IS_GOOD(ch))){
        sendf(ch, "You sense your deity is displeased with you.\n");
        return(0);
      }
      else if(GET_MANA(ch) < mana_needed) {
        sendf(ch,"You feel too out of touch with your deity.\n");
        return(0);
      }
    }
    else{
      if(GET_POWER(ch) < mana_needed) {
        sendf(ch,"You can't summon enough energy to cast the spell.\n");
        return(0);
      }
    }
  }

  if((GET_LEVEL(ch) >= IMMORTAL) && !IS_NPC(ch)){
     delay = 1;
  }
  else{
     delay = (PULSE_VIOLENCE * spell_info[spl].beats) / 4;
  }

  tmp = number(1,101);
  if (tmp > (get_spell(ch, spl) +
      (sp->source == SOURCE_POWER ? power_penalty(ch) : 0))) {
    if(tmp <= (get_spell(ch, spl)) && (sp->source == SOURCE_POWER))
      sendf(ch, "Your focus is warped by the presence of iron and your power wasted.\n");
    else
      send_to_char("You lost your concentration!\n", ch);
    if(sp->source == SOURCE_MANA){
      GET_MANA(ch) -= (mana_needed/2);
    }
    else{
      GET_POWER(ch) -= (mana_needed/2);
    }
    if (get_spell(ch,sp->num)>=sp->max_at_guild && sp->max_at_guild &&
	get_spell(ch,sp->num)<sp->max_learn) {
      if (number(1,100)>get_spell(ch,sp->num)) {
	sendf(ch,"In failure, you learn better the nuances of this spell.\n");
	set_spell(ch,sp->num,1);
      }
    }
    return(1);
  }

  WAIT_STATE(ch, delay);

  add_event(delay, EVT_SPELL, ch, NULL, NULL, -1, spl, argument, event_spell);
  if(IS_SET(spell_info[spl].flags, SPLF_VERBAL)){
    sendf(ch, "You begin muttering under your breath.\n");
    act("$n begins muttering beneath $s breath.", TRUE, ch, NULL, NULL,
      TO_ROOM);
    say_spell(ch, spl);
  }
  else{
    if(sp->source == SOURCE_POWER){
      sendf(ch, "You begin to shape the flows of power in your mind.\n");
    }
    else{
      sendf(ch, "You concentrate upon becoming one with your deity.\n");
    }
  }

  return(1);
}

void event_spell(event_t *event)
{
  int target_ok = FALSE;
  int mana_needed;
  struct char_data *victim = NULL;
  struct obj_data *obj = NULL;
  spell_entry *spell;

  if(!(spell = find_spell_entry(event->ch, event->virtual))){
    return;
  }

  if(IS_SET(spell_info[event->virtual].targets, TAR_VIOLENT) &&
      check_peaceful(event->ch, "Impolite magic is banned here.")){
    return;
  }

  if(IS_SET(spell_info[event->virtual].targets, TAR_CHAR_ROOM)){
    if((victim = get_char_room_vis(event->ch, event->args))){
      target_ok = TRUE;
    }
  }
  else if(IS_SET(spell_info[event->virtual].targets, TAR_CHAR_WORLD)){
    if((victim = get_char_vis(event->ch, event->args))){
      target_ok = TRUE;
    }
  }
  if(IS_SET(spell_info[event->virtual].targets, TAR_FIGHT_VICT) && (!victim) && event->ch->specials.fighting){
    victim = event->ch->specials.fighting;
    target_ok = TRUE;
  }
  if(IS_SET(spell_info[event->virtual].targets, TAR_SELF_ONLY) && (victim != event->ch)){
    if(!victim){
      victim = event->ch;
      target_ok = TRUE;
    }
    else{
      target_ok = FALSE;
    }
  }
  else if(IS_SET(spell_info[event->virtual].targets, TAR_SELF_NONO) && (victim == event->ch)){
    target_ok = FALSE;
  }

  if(IS_SET(spell_info[event->virtual].targets, TAR_OBJ_INV)){
    if((obj = get_obj_in_list_vis(event->ch, event->args, event->ch->carrying))){
      target_ok = TRUE;
    }
  }
  if(IS_SET(spell_info[event->virtual].targets, TAR_OBJ_ROOM) && !obj){
    if((obj = get_obj_in_list_vis(event->ch, event->args, real_roomp(event->ch->in_room)->contents))){
      target_ok = TRUE;
    }
  }
  if(IS_SET(spell_info[event->virtual].targets, TAR_OBJ_EQUIP) && !obj){
    if((obj = get_obj_vis_equ(event->ch, event->args))){
      target_ok = TRUE;
    }
  }
  if(IS_SET(spell_info[event->virtual].targets, TAR_OBJ_WORLD) && !obj){
    if((obj = get_obj_vis(event->ch, event->args))){
      target_ok = TRUE;
    }
  }
  if(IS_SET(spell_info[event->virtual].targets, TAR_IGNORE)){
    target_ok = TRUE;
  }

  if(!target_ok){
    if(IS_SET(spell_info[event->virtual].targets, TAR_CHAR_ROOM)){
      sendf(event->ch, "You don't see anyone around by that name.\n");
      return;
    }
    else if(IS_SET(spell_info[event->virtual].targets, TAR_CHAR_WORLD)){
      sendf(event->ch, "Nobody in the world by that name.\n");
      return;
    }
    else if(IS_SET(spell_info[event->virtual].targets, TAR_OBJ_INV)){
      sendf(event->ch, "You are not carrying anything like that.\n");
      return;
    }
    else if(IS_SET(spell_info[event->virtual].targets, TAR_OBJ_ROOM)){
      sendf(event->ch, "Nothing here by that name.\n");
      return;
    }
    else if(IS_SET(spell_info[event->virtual].targets, TAR_OBJ_EQUIP)){
      sendf(event->ch, "You are not wearing anything like that.\n");
      return;
    }
    else if(IS_SET(spell_info[event->virtual].targets, TAR_OBJ_WORLD)){
      sendf(event->ch, "Nothing at all by that name.\n");
      return;
    }
  }

  mana_needed = spell->mana;

  if (mana_needed<0) mana_needed= -mana_needed;
  else {
    mana_needed *= (5 - (MIN(4, GET_LEVEL(event->ch) - spell->min_level)));
    if (mana_needed>200) mana_needed=200;
  }

  if(spell->min_level<=10)
    mana_needed=MAX(spell->mana,100/(MAX(1,2+GET_LEVEL(event->ch)-spell->min_level)));

  if(has_components(event->ch, event->virtual))
    use_components(event->ch, event->virtual);
  else
    mana_needed *= (1 + spell->difficulty);

  if(spell->source == SOURCE_MANA){
    GET_MANA(event->ch) -= mana_needed;
  }
  else{
    GET_POWER(event->ch) -= mana_needed;
  }
  ((*spell_info[event->virtual].spell_pointer)(GET_LEVEL(event->ch),
    event->ch, event->args, SPELL_TYPE_SPELL, victim, obj,
    GET_LEVEL(event->ch) - spell->min_level + 1));
}
