/* ************************************************************************
 *  file: handler.c , Handler module.                      Part of DIKUMUD *
 *  Usage: Various routines for moving about objects/players               *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include <fcns.h>
#include <utils.h>
#include <comm.h>
#include <db.h>
#include <handler.h>
#include <interpreter.h>
#include <spells.h>
#include <externs.h>

void stopAllFromConsenting(struct char_data *ch)
{
  struct connection_data        *curr_desc;

  for (curr_desc = connection_list; curr_desc != NULL;
       curr_desc = curr_desc->next) {
    if (curr_desc->character != NULL) {
      if (curr_desc->character->consent == ch) {
        send_to_char("The person you gave consent to has just quit.\n",
                     curr_desc->character);
        curr_desc->character->consent = NULL;
      }
    }
  }
}


char *fname(char *namelist)
{
  static char holder[255];
  register char *point;
  
  for (point = holder; isalpha(*namelist); namelist++, point++)
    *point = *namelist;
  
  *point = '\0';
  
  return(holder);
}


int split_string(char *str, char *sep, char **argv)
     /* str must be writable */
{
  char	*s;
  int	argc=0;
  
  s = strtok(str, sep);
  if (s)
    argv[argc++] = s;
  else {
    *argv = str;
    return 1;
  }
  
  while  ((s=strtok(NULL, sep))) {
    argv[argc++] = s;
  }
  return argc;
}


int isname(char *str, char *namelist)
{
  char	*argv[30], *xargv[30];
  int	argc, xargc, i,j, exact;
  char	buf[MAX_INPUT_LENGTH], names[MAX_INPUT_LENGTH], *s;
  
  strcpy(buf, str);
  argc = split_string(buf, "- \t\n,", argv);
  
  strcpy(names, namelist);
  xargc = split_string(names, "- \t\n,", xargv);
  
  s = argv[argc-1];
  s += strlen(s);
  if (*(--s) == '.') {
    exact = 1;
    *s = 0;
  } else {
    exact = 0;
  }
  /* the string has now been split into separate words with the '-'
     replaced by string terminators.  pointers to the beginning of
     each word are in argv */
  
  if (exact && argc != xargc)
    return 0;
  
  for (i=0; i<argc; i++) {
    for (j=0; j<xargc; j++) {
      if (0==str_cmp(argv[i],xargv[j])) {
	xargv[j] = NULL;
	break;
      }
    }
    if (j>=xargc)
      return 0;
  }
  
  return 1;
}

void init_string_block(struct string_block *sb)
{
  sb->data = NULL;
  sb->size = 0;
}

int append_to_string_block(struct string_block *sb, char *str)
{
  int	len;

  if (!str) return(0);
  if (sb->size) {
    len=sb->size-1;
    sb->size += strlen(str);
    sb->data = REALLOC(sb->data,sb->size,char);
    strcpy(sb->data+len,str);
  } else {
    sb->size = strlen(str)+1;
    sb->data = MALLOC(sb->size,char);
    strcpy(sb->data,str);
  }
  return(1);
}

void page_string_block(struct string_block *sb, struct char_data *ch)
{
  if (sb->data)
    page_string(ch->desc, sb->data, 1);
}

void destroy_string_block(struct string_block *sb)
{
  FREE(sb->data);
  sb->data = NULL;
}

int good_affect(struct obj_affected_type *affect)
{
  switch(affect->location){
    case APPLY_SEX:
    case APPLY_CLASS:
    case APPLY_SUSC:
      return(0);
    case APPLY_IMMUNE:
    case APPLY_M_IMMUNE:
    case APPLY_EAT_SPELL:
    case APPLY_WEAPON_SPELL:
      return(1);
    case APPLY_ALL_AC:
    case APPLY_SAVING_PARA:
    case APPLY_SAVING_ROD:
    case APPLY_SAVING_PETRI:
    case APPLY_SAVING_BREATH:
    case APPLY_SAVING_SPELL:
    case APPLY_SAVE_ALL:
    case APPLY_FEET_AC:
    case APPLY_LEGS_AC:
    case APPLY_ARMS_AC:
    case APPLY_BODY_AC:
    case APPLY_HEAD_AC:
      return((affect->modifier < 0) ? 1 : 0);
    default:
      return((affect->modifier > 0) ? 1 : 0);
  }

  return(1);
}

void affect_modify(struct char_data *ch,byte loc,u32 mod, u32 *bitv, bool add)
{
  int maxabil;
  int i;
  
  
  if (loc == APPLY_IMMUNE) {
    if (add) {
      SET_BIT(ch->specials.immune, mod);
    } else {
      CLEAR_BIT(ch->specials.immune, mod);
    }
  } else if (loc == APPLY_SUSC) {
    if (add) {
      SET_BIT(ch->specials.susc, mod);
    } else {
      CLEAR_BIT(ch->specials.susc, mod);
    }
    
  } else if (loc == APPLY_M_IMMUNE) {
    if (add) {
      SET_BIT(ch->specials.M_immune, mod);
    } else {
      CLEAR_BIT(ch->specials.M_immune, mod);
    }
  } else if (loc == APPLY_SPELL) {
    if (add) {
      set_bit(ch->specials.affected_by, mod);
    } else {
      clear_bit(ch->specials.affected_by, mod);
    }
  } else if (loc == APPLY_WEAPON_SPELL) {
    return;
  } else {
    u32 check=0;
    for (i=0; i<8 && !check; i++)
	check |= bitv[i];
    if (add) {
      if (check)
	for (i=0; i<255; i++)
	  if (is_set(bitv,i))
	    set_bit(ch->specials.affected_by,i);
    } else {
      if (check)
	for (i=0; i<255; i++)
	  if (is_set(bitv,i))
	    clear_bit(ch->specials.affected_by,i);
      	mod = -mod;
    }
  }
  
  maxabil = (IS_NPC(ch) ? 25:18);
  
  switch(loc) {
    case APPLY_NONE: break;
    case APPLY_STR: GET_STR(ch) += mod; break;
    case APPLY_DEX: GET_DEX(ch) += mod; break;
    case APPLY_INT: GET_INT(ch) += mod; break;
    case APPLY_WIS: GET_WIS(ch) += mod; break;
    case APPLY_CON: GET_CON(ch) += mod; break;
    case APPLY_SEX: /* ??? GET_SEX(ch) += mod; */ break;
    case APPLY_CLASS: break;
    case APPLY_LEVEL: break;
    case APPLY_AGE: /* GET_AGE(ch) += mod; */ break;
    case APPLY_CHAR_WEIGHT: GET_WEIGHT(ch) += mod; break;
    case APPLY_CHAR_HEIGHT: GET_HEIGHT(ch) += mod; break;
    case APPLY_MANA: ch->points.max_mana += mod; break;
    case APPLY_POWER: ch->points.max_power += mod; break;
    case APPLY_HIT: ch->points.max_hit += mod; break;
    case APPLY_MOVE: ch->points.max_move += mod; break;
    case APPLY_GOLD: break;
    case APPLY_EXP: break;
    case APPLY_ALL_AC:
      GET_BODY_AC(ch) += mod;
      GET_HEAD_AC(ch) += mod;
      GET_ARMS_AC(ch) += mod;
      GET_LEGS_AC(ch) += mod;
      GET_FEET_AC(ch) += mod;
      break;

    case APPLY_BODY_AC: GET_BODY_AC(ch) +=mod; break;
    case APPLY_HEAD_AC: GET_HEAD_AC(ch) += mod; break;
    case APPLY_ARMS_AC: GET_ARMS_AC(ch) += mod; break;
    case APPLY_LEGS_AC: GET_LEGS_AC(ch) += mod; break;
    case APPLY_FEET_AC: GET_FEET_AC(ch) += mod; break;

    case APPLY_ALL_STOPPING:
      GET_BODY_STOPPING(ch) += mod;
      GET_HEAD_STOPPING(ch) += mod;
      GET_ARMS_STOPPING(ch) += mod;
      GET_LEGS_STOPPING(ch) += mod;
      GET_FEET_STOPPING(ch) += mod;
      break;

    case APPLY_BODY_STOPPING: GET_BODY_STOPPING(ch) +=mod; break;
    case APPLY_HEAD_STOPPING: GET_HEAD_STOPPING(ch) += mod; break;
    case APPLY_ARMS_STOPPING: GET_ARMS_STOPPING(ch) += mod; break;
    case APPLY_LEGS_STOPPING: GET_LEGS_STOPPING(ch) += mod; break;
    case APPLY_FEET_STOPPING: GET_FEET_STOPPING(ch) += mod; break;
    case APPLY_HITROLL: GET_HITROLL(ch) += mod; break;
    case APPLY_DAMROLL: GET_DAMROLL(ch) += mod; break;
    case APPLY_SAVING_PARA: ch->specials.apply_saving_throw[0] += mod; break;
    case APPLY_SAVING_ROD: ch->specials.apply_saving_throw[1] += mod; break;
    case APPLY_SAVING_PETRI: ch->specials.apply_saving_throw[2] += mod; break;
    case APPLY_SAVING_BREATH: ch->specials.apply_saving_throw[3] += mod; break;
    case APPLY_SAVING_SPELL: ch->specials.apply_saving_throw[4] += mod; break;
    case APPLY_SAVE_ALL: {
	for (i=0;i<=4;i++)
	  ch->specials.apply_saving_throw[i] += mod;
      }break;
    case APPLY_HITNDAM:
      GET_HITROLL(ch) += mod;
      GET_DAMROLL(ch) += mod;
      break; 
    case APPLY_WEAPON_SPELL:
    case APPLY_M_IMMUNE:
    case APPLY_IMMUNE:
    case APPLY_SUSC:
    case APPLY_SPELL:
    case APPLY_EAT_SPELL:
      break;
    case APPLY_BACKSTAB: set_skill(ch,SKILL_BACKSTAB,mod); break;
    case APPLY_KICK:	set_skill(ch,SKILL_KICK,mod); break;
    case APPLY_SNEAK:	set_skill(ch,SKILL_SNEAK,mod); break;
    case APPLY_HIDE:	set_skill(ch,SKILL_HIDE,mod); break;
    case APPLY_BASH:	set_skill(ch,SKILL_BASH,mod); break;
    case APPLY_PICK:	set_skill(ch,SKILL_PICK_LOCK,mod); break;
    case APPLY_STEAL:	set_skill(ch,SKILL_STEAL,mod); break;
    case APPLY_TRACK:	set_skill(ch,SKILL_HUNT,mod); break;
      
    default:
      log("Unknown apply adjust attempt (handler.c, affect_modify).");
      log(ch->player.name);
      
      break;
      
    } /* switch */
}



/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void affect_total(struct char_data *ch)
{
  struct affected_type *af;
  int i,j;
  
  for(i=0; i<MAX_WEAR; i++) {
    if (ch->equipment[i])
      for(j=0; j<MAX_OBJ_AFFECT; j++)
        if (!(IS_WEAPON(ch->equipment[i]) && (i == HOLD) &&
	    good_affect(&ch->equipment[i]->affected[j])))
	  affect_modify(ch, ch->equipment[i]->affected[j].location,
		        ch->equipment[i]->affected[j].modifier,
		        ch->equipment[i]->obj_flags.bitvector, FALSE);
  }
  
  for(af = ch->affected; af; af=af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);
  
  ch->tmpabilities = ch->abilities;
  
  for(i=0; i<MAX_WEAR; i++) {
    if (ch->equipment[i])
      for(j=0; j<MAX_OBJ_AFFECT; j++)
        if (!(IS_WEAPON(ch->equipment[i]) && (i == HOLD) &&
	    good_affect(&ch->equipment[i]->affected[j])))
	  affect_modify(ch, ch->equipment[i]->affected[j].location,
		        ch->equipment[i]->affected[j].modifier,
		        ch->equipment[i]->obj_flags.bitvector, TRUE);
  }
  
  
  for(af = ch->affected; af; af=af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
  
  /* Make certain values are between 0..25, not < 0 and not > 25! */
  
  i = (IS_NPC(ch) ? 25 :18);
  
  GET_DEX(ch) = MAX(0,MIN(GET_DEX(ch), MAX(i,GET_RDEX(ch))));
  GET_INT(ch) = MAX(0,MIN(GET_INT(ch), MAX(i,GET_RINT(ch))));
  GET_WIS(ch) = MAX(0,MIN(GET_WIS(ch), MAX(i,GET_RWIS(ch))));
  GET_CON(ch) = MAX(0,MIN(GET_CON(ch), MAX(i,GET_RWIS(ch))));
  GET_STR(ch) = MAX(0,MIN(GET_STR(ch), MAX(30,GET_RSTR(ch))));
}



/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char( struct char_data *ch, struct affected_type *af )
{
  struct affected_type *affected_alloc;
  
  if (IsClass(ch,BARBARIAN) && af->type==SPELL_POISON) return;
  CREATE(affected_alloc, struct affected_type, 1);
  
  *affected_alloc = *af;
  affected_alloc->next = ch->affected;
  ch->affected = affected_alloc;
  
  affect_modify(ch, af->location, af->modifier,
		af->bitvector, TRUE);
  affect_total(ch);
}



/* Remove an affected_type structure from a char (called when duration
   reaches zero). Pointer *af must never be NIL! Frees mem and calls 
   affect_location_apply                                                */
void affect_remove( struct char_data *ch, struct affected_type *af )
{
  struct affected_type *hjp;
  
  assert(ch->affected);
  
  affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);
  
  /* remove structure *af from linked list */
  
  if (ch->affected == af) {
    /* remove head of list */
    ch->affected = af->next;
  } else {
    
    for(hjp = ch->affected; (hjp->next) && (hjp->next != af); hjp = hjp->next);
    
    if (hjp->next != af) {
      log("Could not locate affected_type in ch->affected. (handler.c, affect_remove)");
      return;
    }
    hjp->next = af->next; /* skip the af element */
  }
  
  free ( af );
  
  affect_total(ch);
}



/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char( struct char_data *ch, short skill)
{
  struct affected_type *hjp;
  
  for(hjp = ch->affected; hjp; hjp = hjp->next)
    if (hjp->type == skill)
      affect_remove( ch, hjp );
  
}



/* Return if a char is affected by a spell (SPELL_XXX), NULL indicates 
   not affected                                                        */
bool affected_by_spell( struct char_data *ch, short skill )
{
  struct affected_type *hjp;
  
  		for (hjp = ch->affected; hjp; hjp = hjp->next)
    			if ( hjp->type == skill )
      				return( TRUE );
	return (FALSE);
}



void affect_join( struct char_data *ch, struct affected_type *af, bool avg_dur, bool avg_mod )
{
  struct affected_type *hjp;
  bool found = FALSE;
  
  	for (hjp = ch->affected; !found && hjp; hjp = hjp->next) {
    		if ( hjp->type == af->type ) {
      			af->duration += hjp->duration;
      			if (avg_dur)
				af->duration /= 2;
      			af->modifier += hjp->modifier;
      			if (avg_mod)
				af->modifier /= 2;
      
      			affect_remove(ch, hjp);
      			affect_to_char(ch, af);
      			found = TRUE;
    		}
  	}
  	if (!found)
    		affect_to_char(ch, af);
  }

/* move a player out of a room */
void char_from_room(struct char_data *ch)
{
  struct char_data *i;
  struct room_data *rp;
  
  if (ch->in_room == NOWHERE) {
    vlog(LOG_DEBUG,"NOWHERE extracting char from room (handler.c, char_from_room)");
    return;
  }
  
  if (ch->equipment[WEAR_LIGHT])
    if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
      if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2] && real_roomp(ch->in_room)->light>=0)
	real_roomp(ch->in_room)->light--;
  
  rp = real_roomp(ch->in_room);
  if (rp==NULL) {
    vlog(LOG_URGENT,"ERROR: char_from_room: %s was not in a valid room (%d)",
	    (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr),
	    ch->in_room);
    return;
  }
  
  if (ch == rp->people)  /* head of list */
    rp->people = ch->next_in_room;
  
  else {   /* locate the previous element */
    for (i = rp->people; i && i->next_in_room != ch; i = i->next_in_room)
      ;
    if (i)
      i->next_in_room = ch->next_in_room;
    else {
      vlog(LOG_URGENT,"SHIT, %s was not in people list of his room %d!",
	      (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr),
	      ch->in_room);
    }
  }
  
  ch->in_room = NOWHERE;
  ch->next_in_room = 0;
}

/* place a character in a room */
void char_to_room(struct char_data *ch, int room)
{
  /*void raw_kill(struct char_data *ch);*/
  struct room_data *rp;
  
  rp = real_roomp(room);
  if (!rp) {
    room = 0;
    rp = real_roomp(room);
    if (!rp) {
      vlog(LOG_URGENT,"void doesn't exist");
      exit(0);
    }
  }
  ch->next_in_room = rp->people;
  rp->people = ch;
  ch->in_room = room;
  
  if (ch->equipment[WEAR_LIGHT])
    if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
      if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2]) /* Light is ON */
	rp->light++;

  if(rp->tele_time){
    add_event(rp->tele_time, EVT_TELEPORT, ch, NULL, NULL, room, 0, NULL,
      event_teleport);
  }
  if(rp->river_speed && ((rp->sector_type == SECT_WATER_SWIM) ||
      (rp->sector_type == SECT_WATER_NOSWIM) ||
      (rp->sector_type == SECT_UNDERWATER))){
    add_event(rp->river_speed, EVT_RIVER, ch, NULL, NULL, room, 0, NULL,
      event_river);
  }
}

void obj_to_warehouse(struct obj_data *obj, struct char_data *ch)
{
  obj->next_content=ch->warehouse;
  ch->warehouse=obj;
  obj->time_stamp=time(0);
  obj->carried_by = NULL;
  obj->in_obj = NULL;
  obj->obj_flags.timer2 = 0;
  obj->in_room=NOWHERE;
  obj->in_warehouse=ch;
}

void obj_from_warehouse(struct obj_data *obj)
{
  struct obj_data *o;
  struct char_data *ch;
  if (!obj || !obj->in_warehouse) return;
  ch=obj->in_warehouse;
  if (ch->warehouse==obj)
    ch->warehouse=ch->warehouse->next_content;
  else {
    for (o=ch->warehouse; o->next_content; o=o->next_content)
      if (o->next_content==obj) break;
    if (!o->next_content) return;
    o->next_content=obj->next_content;
  }
  obj->time_stamp=0;
  obj->carried_by = NULL;
  obj->in_obj = NULL;
  obj->obj_flags.timer2 = 0;
  obj->in_room=NOWHERE;
  obj->in_warehouse=NULL;
}

/* give an object to a char   */
void obj_to_char(struct obj_data *object, struct char_data *ch)
{
  object->next_content = ch->carrying;
  object->obj_flags.timer2  = 0;
  ch->carrying = object;
  object->carried_by = ch;
  object->in_room = NOWHERE;
  IS_CARRYING_M(ch) += GET_OBJ_MASS(object);
  IS_CARRYING_V(ch) += GET_OBJ_VOLUME(object);
}

/* take an object from a char */
void obj_from_char(struct obj_data *object)
{
  struct obj_data *tmp;
  
  if (!object) {
    object = 0;
    return;
  }
  
  
  if (!object->carried_by) {
    object = 0;
    return;
  }
  
  if (!object->carried_by->carrying) {
    object = 0;
    return;
  }
  
  if (object->carried_by->carrying == object)   /* head of list */
    object->carried_by->carrying = object->next_content;
  
  else
    {
      for (tmp = object->carried_by->carrying; 
	   tmp && (tmp->next_content != object); 
	   tmp = tmp->next_content); /* locate previous */
      
      if (!tmp) {
	object = 0;
	return;
      }
      
      tmp->next_content = object->next_content;
    }
  
  IS_CARRYING_M(object->carried_by) -= GET_OBJ_MASS(object);
  IS_CARRYING_V(object->carried_by) -= GET_OBJ_VOLUME(object);
  object->carried_by = 0;
  object->equipped_by = 0; /* should be unnecessary, but, why risk it */
  object->next_content = 0;
}

/* Return the effect of a piece of armor in position eq_pos 
 * Nope...go ahead and set the ac to the proper value */
int apply_ac(struct char_data *ch, int eq_pos, int state)
{
  int num=ch->equipment[eq_pos]->obj_flags.value[0];
  int num2=-ch->equipment[eq_pos]->obj_flags.value[3];

  assert(ch->equipment[eq_pos]);
  
  if(state) {
    num = -num;
    num2 = -num2;
  }

  if (!(GET_ITEM_TYPE(ch->equipment[eq_pos]) == ITEM_ARMOR))
    return 0;
  
  switch (eq_pos) {
    
  case WEAR_BODY:
  case WEAR_WAISTE:
  case WEAR_ABOUT:
    GET_BODY_AC(ch) += num;
    GET_BODY_STOPPING(ch) += num2;
  break;
  case WEAR_NECK_1:
  case WEAR_NECK_2:
  case WEAR_HEAD:
    GET_HEAD_AC(ch) += num;
    GET_HEAD_STOPPING(ch) += num2;
  break;
  case WEAR_LEGS:
    GET_LEGS_AC(ch) += num;
    GET_LEGS_STOPPING(ch) += num2;
  break;
  case WEAR_FEET:
    GET_FEET_AC(ch) += num;
    GET_FEET_STOPPING(ch) += num2;
  break;
  case WEAR_HANDS:
  case WEAR_ARMS:
  case WEAR_WRIST_L:
  case WEAR_WRIST_R:
    GET_ARMS_AC(ch) += num;
    GET_ARMS_STOPPING(ch) += num2;
  break;
  case WEAR_SHIELD:
    GET_BODY_AC(ch) += num;
    GET_BODY_STOPPING(ch) += num2;
    GET_HEAD_AC(ch) += num;
    GET_HEAD_STOPPING(ch) += num2;
  break;
  }
  return 0;
}



void equip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
  int j;
  
  assert(pos>=0 && pos<MAX_WEAR);
  assert(!(ch->equipment[pos]));
  
  if (obj->carried_by) {
    log("EQUIP: Obj is carried_by when equip.");
    return;
  }
  
  if (obj->in_room!=NOWHERE) {
    log("EQUIP: Obj is in_room when equip.");
    return;
  }
  
  if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
    if (ch->in_room != NOWHERE) {
      
      act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
      act("$n is zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_ROOM);
      obj_to_room(obj, ch->in_room);
      return;
    } else {
      log("ch->in_room = NOWHERE when equipping char.");
    }
  }
  
  ch->equipment[pos] = obj;
  obj->equipped_by = ch;
  obj->eq_pos = pos;
  
  if (GET_ITEM_TYPE(obj) == ITEM_ARMOR && (pos != WEAR_FINGER_L || pos != WEAR_FINGER_R))
    apply_ac(ch,pos,TRUE);
 
  for(j=0; j<MAX_OBJ_AFFECT; j++)
    if (!(IS_WEAPON(obj) && (pos == HOLD) && good_affect(&obj->affected[j])))
      affect_modify(ch, obj->affected[j].location,
		    obj->affected[j].modifier,
		    obj->obj_flags.bitvector, TRUE);
  
  affect_total(ch);
}



struct obj_data *unequip_char(struct char_data *ch, int pos)
{
  int j;
  struct obj_data *obj;
  
  assert(pos>=0 && pos<MAX_WEAR);
  assert(ch->equipment[pos]);
  
  obj = ch->equipment[pos];
  if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
    apply_ac(ch,pos,FALSE);
  
  ch->equipment[pos] = 0;
  obj->equipped_by = NULL;
  obj->eq_pos = -1;
  
  for(j=0; j<MAX_OBJ_AFFECT; j++)
    if (!(IS_WEAPON(obj) && (pos == HOLD) && good_affect(&obj->affected[j])))
      affect_modify(ch, obj->affected[j].location,
		    obj->affected[j].modifier,
		    obj->obj_flags.bitvector, FALSE);
  
  affect_total(ch);
  
  return(obj);
}


int get_number(char **name)
{
  
  int i;
  char *ppos;
  char number[MAX_INPUT_LENGTH];
  
  *number='\0';
  if ((ppos = (char *)index(*name, '.')) && ppos[1]) {
    *ppos++ = '\0';
    strcpy(number,*name);
    strcpy(*name, ppos);
    
    for(i=0; *(number+i); i++)
      if (!isdigit(*(number+i)))
	return(0);
    
    return(atoi(number));
  }
  
  return(1);
}


/* Search a given list for an object, and return a pointer to that object */
struct obj_data *get_obj_in_list(char *name, struct obj_data *list)
{
  struct obj_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  /*
    need ---
    special handlers for 2*thing, all.thing
    
    should be built into each command (get, put, buy)
    
    if (getall(name) == tRUE) {
    while *(i = getobj_in_list()) != NULL) {
    blah
    blah
    blah
    }
    } else if ((p = getabunch(name)) != NULL) {
    while (p > 0) {
    i = get_obj_in_list();
    blah
    blah
    blah
    p--;
    }
    }
    */
  
  
  if (!(number = get_number(&tmp)))
    return(0);
  
  for (i = list, j = 1; i && (j <= number); i = i->next_content)
    if (isname(tmp, i->name)) {
      if (j == number) 
	return(i);
      j++;
    }
  
  return(0);
}



/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list)
{
  struct obj_data *i;
  
  for (i = list; i; i = i->next_content)
    if (i->virtual == num) 
      return(i);
  
  return(0);
}





#if 0
/*search the entire world for an object, and return a pointer  */
static struct obj_data *get_obj(char *name)
{
  struct obj_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  for (i = object_list, j = 1; i && (j <= number); i = i->next)
    if (isname(tmp, i->name)) {
      if (j == number)
	return(i);
      j++;
    }
  
  return(0);
}
#endif





/*search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(int nr)
{
  struct obj_data *i;
  
  for (i = object_list; i; i = i->next)
    if (i->virtual == nr) 
      return(i);
  
  return(0);
}





/* search a room for a char, and return a pointer if found..  */
struct char_data *get_char_room(char *name, int room)
{
  struct char_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  for (i = real_roomp(room)->people, j = 1; i && (j <= number); i = i->next_in_room)
    if (isname(tmp, GET_NAME(i))) {
      if (j == number)
        return(i);
      j++;
    }
  
  return(0);
}





/* search all over the world for a char, and return a pointer if found */
struct char_data *get_char(char *name)
{
  struct char_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  for (i = character_list, j = 1; i && (j <= number); i = i->next)
    if (isname(tmp, GET_NAME(i))) {
      if (j == number)
	return(i);
      j++;
    }
  
  return(0);
}



/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(int nr)
{
  struct char_data *i;
  
  for (i = character_list; i; i = i->next)
    if (i->virtual == nr)
      return(i);
  
  return(0);
}




/* put an object in a room */
void obj_to_room(struct obj_data *object, int room)
{
  
  struct room_data *rp;

  rp=real_roomp(room);
  if (!rp) {
	nlog("obj_to_room: %d %s",room,object->short_description);
	rp = real_roomp(4);
	room=4;
  }
  
  object->next_content = rp->contents;
  rp->contents = object;
  object->in_room = room;
  object->carried_by = 0;
  object->equipped_by = 0; /* should be unnecessary */
}


/* Take an object from a room */
void obj_from_room(struct obj_data *object)
{
  struct obj_data *i;
  
  /* remove object from room */
  
  if (object == real_roomp(object->in_room)->contents)  /* head of list */
    real_roomp(object->in_room)->contents = object->next_content;
  
  else     /* locate previous element in list */
    {
      for (i = real_roomp(object->in_room)->contents; i && 
	   (i->next_content != object); i = i->next_content);
      if (!i)
	nlog("Object not in room %d (Vnum %d)",
			object->in_room,object->virtual);
      else
	i->next_content = object->next_content;
    }
  
  object->in_room = NOWHERE;
  object->next_content = 0;
}

/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to)
{
  int flex;
  struct obj_data *tmp_obj;
  
  obj->next_content = obj_to->contains;
  obj_to->contains = obj;
  obj->in_obj = obj_to;
  obj->carried_by = 0;

  for(flex = 1, tmp_obj = obj->in_obj; tmp_obj; tmp_obj = tmp_obj->in_obj){
    tmp_obj->obj_flags.weight += GET_OBJ_WEIGHT(obj);
    GET_OBJ_MASS(tmp_obj) += GET_OBJ_MASS(obj);
    if(IS_OBJ_STAT(tmp_obj, ITEM_FLEXIBLE) && flex)
      GET_OBJ_VOLUME(tmp_obj) += GET_OBJ_VOLUME(obj);
    else
      flex = 0;
    if(tmp_obj->obj_flags.type_flag == ITEM_CONTAINER){
      tmp_obj->obj_flags.value[0] -= GET_OBJ_VOLUME(obj);
    }
    if(tmp_obj->carried_by){
      IS_CARRYING_M(tmp_obj->carried_by) += GET_OBJ_MASS(obj);
    }
  }
}

/* remove an object from an object */
void obj_from_obj(struct obj_data *obj)
{
  int flex;
  struct obj_data *tmp, *obj_from;
  
  if (obj->in_obj) {
    obj_from = obj->in_obj;
    if (obj == obj_from->contains)   /* head of list */
      obj_from->contains = obj->next_content;
    else {
      for (tmp = obj_from->contains; 
	   tmp && (tmp->next_content != obj);
	   tmp = tmp->next_content); /* locate previous */
      
      if (!tmp) {
	perror("Fatal error in object structures.");
	abort();
      }
      
      tmp->next_content = obj->next_content;
    }
        
    /* Subtract weight from containers container */
    for(flex = 1, tmp = obj->in_obj; tmp->in_obj; tmp = tmp->in_obj){
      tmp->obj_flags.weight -= GET_OBJ_WEIGHT(obj);
      GET_OBJ_MASS(tmp) -= GET_OBJ_MASS(obj);
      if(IS_OBJ_STAT(tmp, ITEM_FLEXIBLE) && flex)
        GET_OBJ_VOLUME(tmp) -= GET_OBJ_VOLUME(obj);
      else
        flex = 0;
      if(tmp->obj_flags.type_flag == ITEM_CONTAINER){
        tmp->obj_flags.value[0] += GET_OBJ_VOLUME(obj);
      }
    }

    tmp->obj_flags.weight -= GET_OBJ_WEIGHT(obj);
    GET_OBJ_MASS(tmp) -= GET_OBJ_MASS(obj);
    if(IS_OBJ_STAT(tmp, ITEM_FLEXIBLE) && flex)
      GET_OBJ_VOLUME(tmp) -= GET_OBJ_VOLUME(obj);
    if(tmp->obj_flags.type_flag == ITEM_CONTAINER){
      tmp->obj_flags.value[0] += GET_OBJ_VOLUME(obj);
    }

    /* Subtract weight from char that carries the object */
    if (tmp->carried_by){
      IS_CARRYING_M(tmp->carried_by) -= GET_OBJ_MASS(obj);
    }
    obj->in_obj = 0;
    obj->next_content = 0;
  } else {
    perror("Trying to object from object when in no object.");
    abort();
  }
}

/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data *list, struct char_data *ch)
{
  if (list) {
    object_list_new_owner(list->contains, ch);
    object_list_new_owner(list->next_content, ch);
    list->carried_by = ch;
  }
}

/* Extract an object from the world */
void extract_obj(struct obj_data *obj)
{
  struct obj_data *temp1, *temp2;
  extern int db_boot;

  rem_obj_events(obj);

  if (obj->in_room != NOWHERE)
    obj_from_room(obj);
  else if (obj->in_warehouse)
    obj_from_warehouse(obj);
  else if (obj->carried_by)
    obj_from_char(obj);
  else if (obj->equipped_by) {
    if (obj->eq_pos < 0) {
      obj->equipped_by->equipment[obj->eq_pos] = 0;
    } 
  } else if(obj->in_obj)	{
    temp1 = obj->in_obj;
    if(temp1->contains == obj)   /* head of list */
      temp1->contains = obj->next_content;
    else		{
      for( temp2 = temp1->contains ;
	  temp2 && (temp2->next_content != obj);
	  temp2 = temp2->next_content );
      
      if(temp2) {
	temp2->next_content =
	  obj->next_content; 
      }
    }
  }
  
  while (obj->contains)
    extract_obj(obj->contains);
  /* leaves nothing ! */
  
  if (object_list == obj )       /* head of list */
    object_list = obj->next;
  else {
    for(temp1 = object_list; 
	temp1 && (temp1->next != obj);
	temp1 = temp1->next);
    
    if (temp1)
      temp1->next = obj->next;
  }
  
  if (real_objp(obj->virtual) && !db_boot) real_objp(obj->virtual)->count--;
  free_obj(obj);
}

static void update_object( struct obj_data *obj, int use)
{
  if (obj->obj_flags.timer > 0)	obj->obj_flags.timer -= use;
  if (obj->contains) update_object(obj->contains, use);
  if (obj->next_content) 
    if (obj->next_content != obj)
      update_object(obj->next_content, use);
}

void update_char_objects( struct char_data *ch )
{
  
  int i;
  
  if (ch->equipment[WEAR_LIGHT])
    if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
      if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2] > 0)
	(ch->equipment[WEAR_LIGHT]->obj_flags.value[2])--;
  
  for(i = 0;i < MAX_WEAR;i++) 
    if (ch->equipment[i])
      update_object(ch->equipment[i],2);
  
  if (ch->carrying) update_object(ch->carrying,1);
}

/* Extract a ch completely from the world */
void extract_char(struct char_data *ch)
{
  struct char_data *k;
  struct connection_data *t_desc;
  int was_in;
  
  if (!IS_NPC(ch) && !ch->desc)	{
    for (t_desc = connection_list; t_desc; t_desc = t_desc->next)
      if (t_desc->original==ch)
	do_return(t_desc->character, "", 0);
  }
 
  if (ch->in_room == NOWHERE) {
    log("NOWHERE extracting char. (handler.c, extract_char)");
    char_to_room(ch, 0);  /* 4 == all purpose store */
  }
  
  if (ch->followers || ch->master)
    die_follower(ch);
  
  if (ch->specials.fighting)
    stop_fighting(ch);
  
  was_in = ch->in_room;
  char_from_room(ch);
  
  rem_char_events(ch);

  if (IS_NPC(ch)) {
    for (k=character_list; k; k=k->next) {
      if (k->specials.hunting)
	if (k->specials.hunting == ch) {
	  k->specials.hunting = 0;
	}
      if (Hates(k, ch)) {
	RemHated(k, ch);
      }
      if (Fears(k, ch)) {
	RemFeared(k, ch);
      }
    }          
  } else {
    for (k=character_list; k; k=k->next) {
      if (k->specials.hunting)
	if (k->specials.hunting == ch) {
	  k->specials.hunting = 0;
	}
      if (Hates(k, ch)) {
	ZeroHatred(k, ch);
      }
      if (Fears(k, ch)) {
	ZeroFeared(k, ch);
      }
    }
    
  }
  /* pull the char from the list */
  
  if (ch == character_list)  
    character_list = ch->next;
  else {
      for(k = character_list; (k) && (k->next != ch); k = k->next);
      if(k) {
	k->next = ch->next;
      } else {
	log("Trying to remove ?? from character_list. (handler.c, extract_char)");
      }
    }
  
  
  if (ch->desc)	{
    if (ch->desc->original)
      do_return(ch, "", 0);
  }
  
  ch->in_room=NOWHERE;
  if (IS_NPC(ch)) 	{
    mobs_in_game--;
    if(IS_SET(ch->specials.act, ACT_SPEC)) mobs_with_special--;
    GET_POS(ch)=POSITION_DEAD;
    if (ch->virtual>0) real_mobp(ch->virtual)->count--;
    FreeHates(ch);
    FreeFears(ch);
    free_char(ch);
  } else if (ch->desc) {
    ch->desc->connected = CON_SLCT;
    SEND_TO_Q(MENU, ch->desc);
  } else if (!ch->desc) {
    free_char(ch);
  }
}


/* ***********************************************************************
   Here follows high-level versions of some earlier routines, ie functionst
   which incorporate the actual player-data.
   *********************************************************************** */


struct char_data *get_char_room_vis(struct char_data *ch, char *name)
{
  struct char_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  if(!str_cmp(name, "self") || !str_cmp(name, "me") || !str_cmp(name, "myself")){
    return(ch);
  }
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  for (i = real_roomp(ch->in_room)->people, j = 1; 
       i && (j <= number); i = i->next_in_room)
    if (isname(tmp, GET_NAME(i)))
      if (CAN_SEE(ch, i))	{
	if (j == number) 
	  return(i);
	j++;
      }
  
  return(0);
}


/* get a character from anywhere in the world, doesn't care much about
   being in the same room... */
struct char_data *get_char_vis_world(struct char_data *ch, char *name, slong *count)
     
{
  struct char_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  if(!str_cmp(name, "self") || !str_cmp(name, "me") || !str_cmp(name, "myself")){
    return(ch);
  }
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  j = count ? *count : 1;
  for (i = character_list; i && (j <= number); i = i->next)
    if (isname(tmp, GET_NAME(i)))
      if (CAN_SEE(ch, i))	{
	if (j == number)
	  return(i);
	j++;
      }
  if (count) *count = j;
  return 0;
}

struct char_data *get_char_vis(struct char_data *ch, char *name)
{
  struct char_data *i;

  if(!str_cmp(name, "self") || !str_cmp(name, "me") || !str_cmp(name, "myself")){
    return(ch);
  }
  
  /* check location */
  if ((i = get_char_room_vis(ch, name)))
    return(i);
  
  return(get_char_vis_world(ch,name, NULL));
}






struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, struct obj_data *list)
{
  struct obj_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  for (i = list, j = 1; i && (j <= number); i = i->next_content)
    if (isname(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i)) {
	if (j == number)
	  return(i);
	j++;
      }
  return(0);
}



struct obj_data *get_obj_vis_world(struct char_data *ch, char *name, slong *count)
{
  struct obj_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  j = count ? *count : 1;
  
  /* ok.. no luck yet. scan the entire obj list   */
  for (i = object_list; i && (j <= number); i = i->next)
    if (isname(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i)) {
	if (j == number)
	  return(i);
	j++;
      }
  if (count) *count = j;
  return(0);
}

/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data *ch, char *name)
{
  struct obj_data *i;
  
  /* scan items carried */
  if ((i = get_obj_in_list_vis(ch, name, ch->carrying)))
    return(i);
  
  /* scan room */
  if ((i = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents)))
    return(i);
  
  return(get_obj_vis_world(ch, name, NULL));
}

struct obj_data *get_obj_vis_accessible(struct char_data *ch, char *name)
{
  struct obj_data *i;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  
  strcpy(tmpname,name);
  tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
  
  /* scan items carried */
  for (i = ch->carrying, j=1; i && j<=number; i = i->next_content)
    if (isname(tmp, i->name) && CAN_SEE_OBJ(ch, i))
      if (j == number)
	return(i);
      else
	j++;
  for (i = real_roomp(ch->in_room)->contents; i && j<=number; i = i->next_content)
    if (isname(tmp, i->name) && CAN_SEE_OBJ(ch, i))
      if (j==number)
	return(i);
      else
	j++;
  return 0;
}




struct obj_data *create_money( int amount )
{
  struct obj_data *obj;
  struct extra_descr_data *new_descr;
  char buf[MAX_STRING_LENGTH];
  
  if (amount<=0) {
    vlog(LOG_URGENT,"ERROR: Try to create negative money.");
    amount=1;
  }
  
  CREATE(obj, struct obj_data, 1);
  CREATE(new_descr, struct extra_descr_data, 1);
  clear_object(obj);
  
  if(amount==1)
    {
      obj->name = mystrdup("coin gold");
      obj->short_description = mystrdup("a gold coin");
      obj->description = mystrdup("One miserable gold coin.");
      
      new_descr->keyword = mystrdup("coin gold");
      new_descr->description = mystrdup("One miserable gold coin.");
    }
  else
    {
      obj->name = mystrdup("coins gold");
      obj->short_description = mystrdup("gold coins");
      obj->description = mystrdup("A pile of gold coins.");
      
      new_descr->keyword = mystrdup("coins gold");
      if(amount<10) {
	sprintf(buf,"There is %d coins.",amount);
	new_descr->description = mystrdup(buf);
      } 
      else if (amount<100) {
	sprintf(buf,"There is about %d coins",10*(amount/10));
	new_descr->description = mystrdup(buf);
      }
      else if (amount<1000) {
	sprintf(buf,"It looks like something round %d coins",100*(amount/100));
	new_descr->description = mystrdup(buf);
      }
      else if (amount<100000) {
	sprintf(buf,"You guess there is %d coins",1000*((amount/1000)+ number(0,(amount/1000))));
	new_descr->description = mystrdup(buf);
      }
      else 
	new_descr->description = mystrdup("There is A LOT of coins");			
    }
  
  new_descr->next = 0;
  obj->ex_description = new_descr;
  
  obj->obj_flags.type_flag = ITEM_MONEY;
  obj->obj_flags.wear_flags = ITEM_TAKE;
  obj->obj_flags.value[0] = amount;
  obj->obj_flags.cost = amount;
  
  obj->next = object_list;
  object_list = obj;
  
  return(obj);
}

struct obj_data *get_obj_vis_equ(struct char_data *ch, char *name)
{
  int i;

  for(i=0; i < MAX_WEAR; i++){
    if(ch->equipment[i] && (isname(name, ch->equipment[i]->name)) && CAN_SEE_OBJ(ch, ch->equipment[i])){
      return(ch->equipment[i]);
    }
  }
  return(NULL);
}

/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int generic_find(char *arg, int bitvector, struct char_data *ch, struct char_data **tar_ch, struct obj_data **tar_obj)
{
  static char *ignore[] = {
    "the",
    "in",
    "on",
    "at",
    "\n" };
  
  int i;
  char name[256];
  bool found;
  
  found = FALSE;
  
  *tar_ch=NULL; *tar_obj=NULL;
  /* Eliminate spaces and "ignore" words */
  while (*arg && !found) {
    
    for(; *arg == ' '; arg++)   ;
    
    for(i=0; (name[i] = *(arg+i)) && (name[i]!=' '); i++)   ;
    name[i] = 0;
    arg+=i;
    if (search_block(name, ignore, TRUE) > -1)
      found = TRUE;
    
  }
  
  if (!name[0])
    return(0);
  
  *tar_ch  = 0;
  *tar_obj = 0;
  
  if (IS_SET(bitvector, FIND_CHAR_ROOM)) {      /* Find person in room */
    if ((*tar_ch = get_char_room_vis(ch, name))) {
      return(FIND_CHAR_ROOM);
    }
  }
  
  if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
    if ((*tar_ch = get_char_vis(ch, name))) {
      return(FIND_CHAR_WORLD);
    }
  }
  
  if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
    if ((*tar_obj = get_obj_vis_equ(ch, name))) {
      return(FIND_OBJ_EQUIP);
    }
  }
  
  if (IS_SET(bitvector, FIND_OBJ_INV)) {
    if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
      if ((*tar_obj = get_obj_vis_accessible(ch, name))) {
	return(FIND_OBJ_INV);
      }
    } else {
      if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
	return(FIND_OBJ_INV);
      }
    }
  }
  
  if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
    if ((*tar_obj = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents))) {
      return(FIND_OBJ_ROOM);
    }
  }
  
  if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
    if ((*tar_obj = get_obj_vis(ch, name))) {
      return(FIND_OBJ_WORLD);
    }
  }
  
  return(0);
}
