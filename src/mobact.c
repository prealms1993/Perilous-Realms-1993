/* ************************************************************************
*  file: mobact.c , mobile action module.                 part of dikumud *
*  usage: procedures generating 'intelligent' behavior in the mobiles.    *
*  copyright (c) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>

#include <fcns.h>
#include <externs.h>
#include <utils.h>
#include <handler.h>
#include <db.h>
#include <comm.h>
#include <trap.h>
#include <ticks.h>
#include <spells.h>

typedef struct ll {
  struct char_data *p;
  struct ll *next;
} ll;

extern struct char_data *character_list;
extern struct hash_header room_db;
extern struct str_app_type str_app[];
extern int DEBUG;

int mobs_in_game=0;
int mobs_with_special=0;

/* used for evaluating object values */
struct obj_rate {
  int rating;
  struct obj_data *obj;
  int eq_pos;
};

/* act_state:: look at the state of the mobile and decide what to do - SLB */
/* this will probably eventually supercede mobile_activity */
int act_state(struct char_data *ch)
{
  extern shop_data *shop_index;
  shop_data *s;

  /* don't do anything if switched or charmed or delayed */
  if(ch->desc || IS_AFFECTED(ch, AFF_CHARM) || ch->specials.wait){
    if(ch->specials.wait > 0) ch->specials.wait -= PULSE_MOBILE;
    if(ch->specials.wait < 0) ch->specials.wait = 0;
    return(1);
  }
  if(IsHumanoid(ch) && AWAKE(ch)){
    if(ch->specials.fighting){
      if(IS_CITIZEN(ch) && (number(0, 1) == 0)){
        if(number(0, 9) == 0){
          do_shout(ch, "Help! Murderer! Guards!", 0);
        }
        else{
          act("$n shouts 'Help! Murderer! Guards!'", TRUE, ch, 0, 0, TO_ROOM);
        }
        CallForGuard(ch, ch->specials.fighting, 3);
        return(1);
      }
    }
  }
  if(ch->specials.fighting && (GET_POS(ch) == POSITION_FIGHTING)){
    if(act_fight(ch)){
      return(1);
    }
  }
  if(!ch->specials.fighting && (GET_POS(ch) == POSITION_STANDING)){
    if(aid_friends(ch)){
      return(1);
    }
  }
  if (IsHumanoid(ch) && AWAKE(ch)) {
    if(find_items(ch)){
      return(1);
    }
    if(!ch->specials.fighting){
      if(spread_hatred(ch)){
        return(1);
      }
      for(s = shop_index; s; s = s->next){
        if(s->keeper == ch->virtual) break;
      }
      if(!s && discard_items(ch)){
        return(1);
      }
    }
  }
  if(maintain_pos(ch)){
    return(1);
  }
  return(0);
}

int act_fight(struct char_data *ch)
{
  if(GET_POS(ch) == POSITION_SITTING){
    /* can't do much while sitting so ... */
    do_stand(ch, "", 0);
    return(1);
  }
  switch(GET_CLASS(ch)){
    case SCHOLAR:
      return(act_caster(ch));
    case WARRIOR:
      return(0 /*act_fighter(ch)*/);
  }
  return(0);
}

int act_caster(struct char_data *ch)
{
  int spell;

  spell = eval_spells(ch);
  if(spell != -1){
    return(phys_cast(ch, spell, NULL, ""));
  }
  else{
    return(0);
  }
}

int eval_spells(struct char_data *ch)
{
  int spell = -1, highest = 0, rating;
  int i;

  for(i = 0; i < ch->nspells; i++){
    rating = 0;
    if(GET_POS(ch) < spell_info[ch->spells[i].spell_number].minimum_position){
      switch(ch->spells[i].spell_number){
        case SPELL_ARMOR:
          if(!affected_by_spell(ch, SPELL_ARMOR) && ch->specials.fighting){
            rating = 20;
          }
          break;
        case SPELL_TELEPORT:
          if(GET_HIT(ch) < (GET_MAX_HIT(ch) / 10)){
            /* the war is going badly ... time to say sayonara */
            rating = 100;
          }
          break;
        case SPELL_BLESS:
          if(!affected_by_spell(ch, SPELL_BLESS) && ch->specials.fighting){
            rating = 20;
          }
          break;
        case SPELL_BLINDNESS:
          if(ch->specials.fighting){
            if(!affected_by_spell(ch->specials.fighting, SPELL_BLINDNESS)){
              rating = 30;
            }
          }
          break;
        case SPELL_BURNING_HANDS:
          if(ch->specials.fighting) rating = 40;
          break;
        case SPELL_CHILL_TOUCH:
          if(ch->specials.fighting) rating = 35;
          break;
        case SPELL_COLOUR_SPRAY:
          if(ch->specials.fighting) rating = 60;
          break;
        case SPELL_CURE_CRITIC:
          if(GET_HIT(ch) < (GET_MAX_HIT(ch) / 5)) rating = 50;
          break;
        case SPELL_CURE_LIGHT:
          if(GET_HIT(ch) < (GET_MAX_HIT(ch) / 5)) rating = 30;
          break;
        case SPELL_FIREBALL:
          if(ch->specials.fighting) rating = 70;
          break;
        case SPELL_HARM:
          if(ch->specials.fighting) rating = 65;
          break;
        case SPELL_HEAL:
          if(GET_HIT(ch) < (GET_MAX_HIT(ch) / 5)) rating = 90;
          break;
        case SPELL_LIGHTNING_BOLT:
          if(ch->specials.fighting) rating = 50;
          break;
        case SPELL_MAGIC_MISSILE:
          if(ch->specials.fighting) rating = 30;
          break;
        case SPELL_SANCTUARY:
          if(!IS_AFFECTED(ch, AFF_SANCTUARY) && ch->specials.fighting){
            rating = 80;
          }
          break;
        case SPELL_STRENGTH:
          if(!affected_by_spell(ch, SPELL_STRENGTH)){
            rating = 40;
          }
          break;
        case SPELL_CONE_OF_COLD:
          if(ch->specials.fighting) rating = 80;
          break;
        case SPELL_METEOR_SWARM:
          if(ch->specials.fighting) rating = 85;
          break;
        case SPELL_MAGE_FIRE:
          if(ch->specials.fighting) rating = 90;
          break;
      }
      rating = rating + 10 - number(0, 20);
      rating *= ch->spells[i].learned;
    }
    if(rating > highest){
      spell = ch->spells[i].spell_number;
      highest = rating;
    }
  }

  return(spell);
}

/* maintain_pos :: make mobile position react to environs in non critical */
/*   times                                                          - SLB */
int maintain_pos(struct char_data *ch)
{
  switch(ch->specials.default_pos){
    case POSITION_STANDING:
      switch(GET_POS(ch)){
        case POSITION_SLEEPING:
          if(!IS_AFFECTED(ch, AFF_SLEEP)){
            do_wake(ch, "", 0);
            return(1);
          }
          break;
        case POSITION_RESTING:
          do_stand(ch, "", 0);
          return(1);
        case POSITION_SITTING:
          do_stand(ch, "", 0);
          return(1);
      }
      break;
  }
  return(0);
}

/* aid friends :: find like v-nums in the room and help 'em out */
int aid_friends(struct char_data *ch)
{
  struct char_data *other;

  for(other = real_roomp(ch->in_room)->people; other;
      other = other->next_in_room){
    if((ch != other) && other->specials.fighting &&
         (ch->virtual == other->virtual) &&
         CAN_FIGHT(ch, other->specials.fighting)){
       act("$n enters the fray.", FALSE, ch, NULL, NULL, TO_ROOM);
       hit(ch, other->specials.fighting, TYPE_UNDEFINED);
       return(1);
    }
  }

  return(0);
}

/* spread_hatred :: find those hatreds and pass 'em on - SLB */
int spread_hatred(struct char_data *ch)
{
  struct char_data *other;
  struct char_list *hatred;
  char buf[MAX_STRING_LENGTH];

  for(other = real_roomp(ch->in_room)->people; other;
      other = other->next_in_room){
    if((IsHumanoid(other) && IS_NPC(other) && AWAKE(other) &&
        CAN_SEE(ch, other) && ((IS_POLICE(ch) || IS_CITIZEN(ch)) &&
        (IS_POLICE(other) || IS_CITIZEN(other)))) ||
        (ch->virtual == other->virtual)) {
      for(hatred = ch->hates.clist; hatred; hatred = hatred->next){
        if(hatred->op_ch){
          if(!Hates(other, hatred->op_ch)){
            act("$n whispers something to $N.", FALSE, ch, NULL, other,
              TO_NOTVICT);
            sprintf(buf, "$n whispers to you, 'Kill %s on sight!'",
              hatred->name);
            AddHated(other, hatred->op_ch);
            return(1);
          }
        }
      }
    }
  }
  return(0);
}

/* uniq_cont:: find the unique string for an object in a contents list - SLB */
char *uniq_cont(struct char_data *ch, struct obj_data *list,
  struct obj_data *obj)
{
  int i;
  struct obj_data *curr;
  static char s[MAX_STRING_LENGTH];

  for(i = 0, curr = list; curr; curr = curr->next_content){
    if(isname(fname(obj->name), curr->name) && CAN_SEE_OBJ(ch, curr)){
      i++;
      if(curr == obj){
        break;
      }
    }
  }

  sprintf(s, "%d.%s", i, fname(obj->name));

  return(s);
}

/* uniq_equip:: find the unique string for an object in equipment - SLB */
char *uniq_equip(struct char_data *ch, int eq_pos)
{
  int i, j;
  static char s[MAX_STRING_LENGTH];

  for(i = 0, j = 0; j < MAX_WEAR; j++){
    if(ch->equipment[j]){
      if(isname(fname(ch->equipment[eq_pos]->name), ch->equipment[j]->name) &&
          CAN_SEE_OBJ(ch, ch->equipment[j])){
        i++;
        if(eq_pos == j){
          break;
        }
      }
    }
  }

  sprintf(s, "%d.%s", i, fname(ch->equipment[eq_pos]->name));

  return(s);
}

/* find_best:: find the best item in a list of items - SLB */
void find_best(struct obj_data *list, struct char_data *ch,
  struct obj_rate *best)
{
  int a, b, new_rate, old_rate, eq_pos;
  struct obj_data *obj;

  best->rating = 0;
  best->obj = NULL;
  best->eq_pos = -1;
  for (obj = list; obj; obj = obj->next_content) {
    if(IS_CORPSE(obj)){
      new_rate = rate_list(obj->contains, ch);
      if(new_rate > best->rating){
        best->rating = new_rate;
        best->obj = obj;
        best->eq_pos = -1;
      }
    }
    else{
      switch (obj->obj_flags.type_flag) {
        case ITEM_ARMOR:
          new_rate = rate_armor(obj, ch);
          old_rate = 0;
          eq_pos = -1;
          /* gander at the wear bits to see where it goes */
          if (CAN_WEAR(obj, ITEM_WEAR_SHIELD)) {
            old_rate = rate_armor(ch->equipment[WEAR_SHIELD], ch);
            if(ch->equipment[WEAR_SHIELD])
              eq_pos = WEAR_SHIELD;
          }
          else if (CAN_WEAR(obj, ITEM_WEAR_FINGER)) {
            a = rate_armor(ch->equipment[WEAR_FINGER_R], ch);
            b = rate_armor(ch->equipment[WEAR_FINGER_L], ch);
            if(a < b){
              old_rate = a;
              if(ch->equipment[WEAR_FINGER_R])
                eq_pos = WEAR_FINGER_R;
            }
            else{
              old_rate = b;
              if(ch->equipment[WEAR_FINGER_L])
                eq_pos = WEAR_FINGER_L;
            }
          }
          else if (CAN_WEAR(obj, ITEM_WEAR_NECK)) {
            a = rate_armor(ch->equipment[WEAR_NECK_1], ch);
            b = rate_armor(ch->equipment[WEAR_NECK_2], ch);
            if(a < b){
              old_rate = a;
              if(ch->equipment[WEAR_NECK_1])
                eq_pos = WEAR_NECK_1;
            }
            else{
              old_rate = b;
              if(ch->equipment[WEAR_NECK_2])
                eq_pos = WEAR_NECK_2;
            }
          }
          else if (CAN_WEAR(obj, ITEM_WEAR_WRIST)) {
            a = rate_armor(ch->equipment[WEAR_WRIST_R], ch);
            b = rate_armor(ch->equipment[WEAR_WRIST_L], ch);
            if(a < b){
              old_rate = a;
              if(ch->equipment[WEAR_WRIST_R])
                eq_pos = WEAR_WRIST_R;
            }
            else{
              old_rate = b;
              if(ch->equipment[WEAR_WRIST_L])
                eq_pos = WEAR_WRIST_L;
            }
          }
          else if (CAN_WEAR(obj, ITEM_WEAR_ARMS)) {
            old_rate = rate_armor(ch->equipment[WEAR_ARMS], ch);
            if(ch->equipment[WEAR_ARMS])
              eq_pos = WEAR_ARMS;
          }
          else if (CAN_WEAR(obj, ITEM_WEAR_HANDS)) {
            old_rate = rate_armor(ch->equipment[WEAR_HANDS], ch);
            if(ch->equipment[WEAR_HANDS])
              eq_pos = WEAR_HANDS;
          }
          else if (CAN_WEAR(obj, ITEM_WEAR_ABOUT)) {
            old_rate = rate_armor(ch->equipment[WEAR_ABOUT], ch);
            if(ch->equipment[WEAR_ABOUT])
              eq_pos = WEAR_ABOUT;
          }
          else if (CAN_WEAR(obj, ITEM_WEAR_WAISTE)) {
            old_rate = rate_armor(ch->equipment[WEAR_WAISTE], ch);
            if(ch->equipment[WEAR_WAISTE])
              eq_pos = WEAR_WAISTE;
          }
          else if (CAN_WEAR(obj, ITEM_WEAR_FEET)) {
            old_rate = rate_armor(ch->equipment[WEAR_FEET], ch);
            if(ch->equipment[WEAR_FEET])
              eq_pos = WEAR_FEET;
          }
          else if (CAN_WEAR(obj, ITEM_WEAR_LEGS)) {
            old_rate = rate_armor(ch->equipment[WEAR_LEGS], ch);
            if(ch->equipment[WEAR_LEGS])
              eq_pos = WEAR_LEGS;
          }
          else if (CAN_WEAR(obj, ITEM_WEAR_HEAD)) {
            old_rate = rate_armor(ch->equipment[WEAR_HEAD], ch);
            if(ch->equipment[WEAR_HEAD])
              eq_pos = WEAR_HEAD;
          }
          else if (CAN_WEAR(obj, ITEM_WEAR_BODY)) {
            old_rate = rate_armor(ch->equipment[WEAR_BODY], ch);
            if(ch->equipment[WEAR_BODY])
              eq_pos = WEAR_BODY;
          }
          if (((new_rate > (old_rate + 10)) || !old_rate) &&
              (new_rate > best->rating)) {
            /* bias towards old armor a bit */
            best->rating = new_rate;
            best->obj = obj;
            best->eq_pos = eq_pos;
          }
          break;
        case ITEM_WEAPON:
          new_rate = rate_weapon(obj, ch);
          old_rate = rate_weapon(ch->equipment[WIELD], ch);
          if (((new_rate > (old_rate + 30)) || !old_rate) &&
              (new_rate > best->rating)) {
            /* bias towards old weapon a bit */
            best->rating = new_rate;
            best->obj = obj;
            if(ch->equipment[WIELD])
              best->eq_pos = WIELD;
            else
              best->eq_pos = -1;
          }
          break;
      }
    }
  }
}

/* discard_items:: look at carried items and determine if any should be */
/*   dropped                                                       -SLB */
int discard_items(struct char_data *ch)
{
  int a, b, new_rate;
  struct obj_rate worst;
  struct obj_data *obj;

  worst.rating = 0;
  worst.obj = NULL;
  for (obj = ch->carrying; obj; obj = obj->next_content) {
    switch (obj->obj_flags.type_flag) {
      case ITEM_ARMOR:
        new_rate = rate_armor(obj, ch);
        /* gander at the wear bits to see where it goes */
        if (CAN_WEAR(obj, ITEM_WEAR_SHIELD)) {
          new_rate -= rate_armor(ch->equipment[WEAR_SHIELD], ch);
        }
        else if (CAN_WEAR(obj, ITEM_WEAR_FINGER)) {
          a = rate_armor(ch->equipment[WEAR_FINGER_R], ch);
          b = rate_armor(ch->equipment[WEAR_FINGER_L], ch);
          if(a < b){
            new_rate -= a;
          }
          else{
            new_rate -= b;
          }
        }
        else if (CAN_WEAR(obj, ITEM_WEAR_NECK)) {
          a = rate_armor(ch->equipment[WEAR_NECK_1], ch);
          b = rate_armor(ch->equipment[WEAR_NECK_1], ch);
          if(a < b){
            new_rate -= a;
          }
          else{
            new_rate -= b;
          }
        }
        else if (CAN_WEAR(obj, ITEM_WEAR_WRIST)) {
          a = rate_armor(ch->equipment[WEAR_NECK_1], ch);
          b = rate_armor(ch->equipment[WEAR_NECK_1], ch);
          if(a < b){
            new_rate -= a;
          }
          else{
            new_rate -= b;
          }
        }
        else if (CAN_WEAR(obj, ITEM_WEAR_ARMS)) {
          new_rate -= rate_armor(ch->equipment[WEAR_ARMS], ch);
        }
        else if (CAN_WEAR(obj, ITEM_WEAR_HANDS)) {
          new_rate -= rate_armor(ch->equipment[WEAR_HANDS], ch);
        }
        else if (CAN_WEAR(obj, ITEM_WEAR_ABOUT)) {
          new_rate -= rate_armor(ch->equipment[WEAR_ABOUT], ch);
        }
        else if (CAN_WEAR(obj, ITEM_WEAR_WAISTE)) {
          new_rate -= rate_armor(ch->equipment[WEAR_WAISTE], ch);
        }
        else if (CAN_WEAR(obj, ITEM_WEAR_FEET)) {
          new_rate -= rate_armor(ch->equipment[WEAR_FEET], ch);
        }
        else if (CAN_WEAR(obj, ITEM_WEAR_LEGS)) {
          new_rate -= rate_armor(ch->equipment[WEAR_LEGS], ch);
        }
        else if (CAN_WEAR(obj, ITEM_WEAR_HEAD)) {
          new_rate -= rate_armor(ch->equipment[WEAR_HEAD], ch);
        }
        else if (CAN_WEAR(obj, ITEM_WEAR_BODY)) {
          new_rate -= rate_armor(ch->equipment[WEAR_BODY], ch);
        }
        if(new_rate < worst.rating){
          worst.rating = new_rate;
          worst.obj = obj;
        }
        break;
      case ITEM_WEAPON:
        new_rate = rate_weapon(obj, ch);
        new_rate -= rate_weapon(ch->equipment[WIELD], ch);
        if(new_rate < worst.rating){
          worst.rating = new_rate;
          worst.obj = obj;
        }
        break;
    }
  }

  if(worst.rating < 0){
    switch(worst.obj->obj_flags.type_flag){
      case ITEM_LIGHT:
      case ITEM_OTHER:
      case ITEM_TRASH:
      case ITEM_TRAP:
      case ITEM_NOTE:
      case ITEM_DRINKCON:
      case ITEM_FOOD:
      case ITEM_PEN:
      case ITEM_BOAT:
        return(phys_junk(ch, worst.obj));
      default:
        return(phys_drop(ch, worst.obj));
    }
  }
  return(0);
}

/* find_items:: find items and see if any are worth having - SLB */
int find_items(struct char_data *ch)
{
  struct obj_rate best, best_inv, best_ground;
  int best_loc;
  char buf[MAX_STRING_LENGTH];

  /* check our inventory */
  find_best(ch->carrying, ch, &best_inv);
  /* check the ground */
  find_best(real_roomp(ch->in_room)->contents, ch, &best_ground);

  if(best_inv.rating > best_ground.rating){
    best = best_inv;
    best_loc = 0;
  }
  else{
    best = best_ground;
    best_loc = 1;
  }

  if (best.rating) {
    if(best.obj){
      if(IS_CORPSE(best.obj) && (number(0, 9) == 0)){
        if(best_loc){
          sprintf(buf, "all %s",
            uniq_cont(ch, real_roomp(ch->in_room)->contents, best.obj));
          do_get(ch, buf, 0);
        }
        else{
          sprintf(buf, "all %s", uniq_cont(ch, ch->carrying, best.obj));
          do_get(ch, buf, 0);
        }
        return(1);
      }
      switch (best.obj->obj_flags.type_flag) {
        case ITEM_ARMOR:
          if (best_loc) {
            /* it's on the ground, so get it */
            return(phys_get(ch, best.obj, NULL));
          }
          else if (best.eq_pos != -1) {
            /* we should remove armor */
            return(phys_remove(ch, best.eq_pos));
          }
          else {
            /* no equipment worn there, so wear */
            return(phys_wear(ch, best.obj));
          }
          return(1);
        case ITEM_WEAPON:
          if (best_loc) {
            /* it's on the ground, so get it */
            return(phys_get(ch, best.obj, NULL));
          }
          else if (best.eq_pos != -1) {
            /* remove the old weapon */
            return(phys_remove(ch, WIELD));
          }
          else {
            /* nothing wielded, so use */
            return(phys_wield(ch, best.obj));
          }
          return(1);
      }
    }
  }
  return(0);
}

/* rate_list:: rate a list of objects - SLB */
int rate_list(struct obj_data *list, struct char_data *ch)
{
  int total = 0;
  struct obj_data *obj;

  for(obj = list; obj; obj = obj->next_content){
    switch(obj->obj_flags.type_flag){
      case ITEM_ARMOR:
        total += rate_armor(obj, ch);
        break;
      case ITEM_WEAPON:
        total += rate_weapon(obj, ch);
        break;
    }
  }

  return(total);
}

/* rate_effects:: rate the magic effects on an item - SLB */
int rate_effects(struct obj_data *obj, struct char_data *ch)
{
  int i, total = 0;

  for(i = 0; i < MAX_OBJ_AFFECT; i++){
    switch(obj->affected[i].location){
      case APPLY_STR:
      case APPLY_DEX:
      case APPLY_INT:
      case APPLY_WIS:
      case APPLY_CON:
        total += obj->affected[i].modifier * 30;
        break;
      case APPLY_LEVEL:
        total += obj->affected[i].modifier * 100;
        break;
      case APPLY_AGE:
        total -= 100;
        break;
      case APPLY_HIT:
        total += (100 * obj->affected[i].modifier) / GET_MAX_HIT(ch);
        break;
      case APPLY_MANA:
        total += (100 * obj->affected[i].modifier) / GET_MAX_MANA(ch);
        break;
      case APPLY_POWER:
        total += (100 * obj->affected[i].modifier) / GET_MAX_POWER(ch);
        break;
      case APPLY_MOVE:
        total += (100 * obj->affected[i].modifier) / GET_MAX_MOVE(ch);
        break;
      case APPLY_ALL_AC:
        total += obj->affected[i].modifier * -20;
        break;
      case APPLY_HITROLL:
        total += obj->affected[i].modifier;
        break;
      case APPLY_DAMROLL:
        total += obj->affected[i].modifier * 10;
        break;
      case APPLY_IMMUNE:
        total += 50;
        break;
      case APPLY_SUSC:
        total -= 50;
        break;
      case APPLY_M_IMMUNE:
        total += 100;
        break;
      case APPLY_HITNDAM:
        total += obj->affected[i].modifier * 11;
        break;
    }
  }

  return(total);
}

/* rate_weapon:: rate the value of a weapon for a mobile - SLB */
int rate_weapon(struct obj_data *obj, struct char_data *ch)
{
  if (!obj)
    return(0);
  else if(ch->specials.fighting ? affected_by_spell(ch->specials.fighting, SPELL_STONE_SKIN) : 0)
    return(0);
  else if(!can_carry(ch, ch, obj) ||
      ((GET_OBJ_WEIGHT(obj) > str_app[GET_STR(ch)].wield_w) &&
      (obj != ch->equipment[WIELD])))
    return(0);
  else if((obj->obj_flags.type_flag != ITEM_WEAPON) || !CAN_SEE_OBJ(ch, obj) ||
      !IsHumanoid(ch))
    return(0);
  else if((((((obj->obj_flags.value[2] + 1) * obj->obj_flags.value[1] * 9) >>
      1) + (10 - obj->obj_flags.value[4]) * 4 + rate_effects(obj, ch)) <
      (((ch->specials.damsizedice + 1) * ch->specials.damnodice * 9) >> 1)) &&
      (obj != ch->equipment[WIELD]))
    return(0);
  else
    return((((obj->obj_flags.value[2] + 1) * obj->obj_flags.value[1] * 9) >>
      1) + (10 - obj->obj_flags.value[4]) * 4 + rate_effects(obj, ch));
}

/* rate_armor:: rate the value of a piece of armor for a mobile - SLB */
/*    will become more complex, which is why it isn't a macro or */
/*    somethin'                                                  */
int rate_armor(struct obj_data *obj, struct char_data *ch)
{
  if (!obj)
    return(0);
  if ((obj->obj_flags.type_flag != ITEM_ARMOR) || !CAN_SEE_OBJ(ch, obj) ||
      !IsHumanoid(ch) || !CAN_CARRY_OBJ(ch, obj))
    return(0);
  else
    return(9 * obj->obj_flags.value[1] + obj->obj_flags.value[0] +
      5 * obj->obj_flags.value[3] + rate_effects(obj, ch));
}

void mobile_guardian(struct char_data *ch)
{
  
  if (ch->in_room > -1) {
    if ((!ch->master) || (!IS_AFFECTED(ch, AFF_CHARM)))
      return;
    if (ch->master->specials.fighting) {
      if (!SameRace(ch->master->specials.fighting, ch)) {
	if (IsHumanoid(ch)) {
	  act("$n screams 'I must protect my master!'", 
	      FALSE, ch, 0, 0, TO_ROOM);
	} else {
	  act("$n growls angrily!", 
	      FALSE, ch, 0, 0, TO_ROOM);
	}
	if (CAN_SEE(ch, ch->master->specials.fighting))
	  hit(ch, ch->master->specials.fighting,0);	  
      }
    }
  }
}
  
void mobile_wander(struct char_data *ch)
{
  int door;
  struct room_direction_data *exitp;
  struct room_data *rp;
      
  door=number(0,15);
  exitp=EXIT(ch,door);

  if (GET_POS(ch) != POSITION_STANDING || door>5 || !exit_ok(exitp,&rp) ||
      IS_SET(rp->room_flags, NO_MOB | DEATH))
    return;
      
    if (IsHumanoid(ch) ? CAN_GO_HUMAN(ch, door) : CAN_GO(ch, door)) {
      if (ch->specials.last_direction == door) {
	ch->specials.last_direction = -1;
      } else {
	if (!IS_SET(ch->specials.act, ACT_STAY_ZONE) ||
	    (rp->zone==real_roomp(ch->in_room)->zone)) {
	  ch->specials.last_direction = door;
	  go_direction(ch, door);
	}
      }
    }
}
  
void MobHunt(struct char_data *ch)
{
  int res, k;
  
  if (ch->persist <= 0) {
    res = choose_exit(ch->in_room, ch->old_room, 2000);
    if((res < 0) && IsHumanoid(ch)){
      res = choose_exit(ch->in_room, ch->old_room, -2000);
    }
    if (res > -1) {
      go_direction(ch, res);
    } else {
      if (ch->specials.hunting) {
	if (ch->specials.hunting->in_room == ch->in_room) {
	  if (Hates(ch, ch->specials.hunting) && 
	      (!IS_AFFECTED(ch->specials.hunting, AFF_HIDE))) {
	    if (check_peaceful(ch, "You'd love to tear your quarry to bits, but you just CAN'T\n")) {
	      act("$n fumes at $N", TRUE, ch, 0,
		  ch->specials.hunting, TO_ROOM); 
	    } else {
	      if (IsHumanoid(ch)) {
		act("$n screams 'Time to die, $N'", 
		    TRUE, ch, 0, ch->specials.hunting, TO_ROOM); 
	      } else if (IsAnimal(ch)) {
		act("$n growls.", TRUE, ch, 0, 0, TO_ROOM);
	      }
	      hit(ch,ch->specials.hunting,0);
	      return;
	    }
	  }
	}
      }
      REMOVE_BIT(ch->specials.act, ACT_HUNTING);
      ch->specials.hunting = 0;
      ch->hunt_dist = 0;
    }
  } else if (ch->specials.hunting) {
    if (ch->hunt_dist <= 50) 
      ch->hunt_dist = 100;
    for (k=1;k<=1 && ch->specials.hunting; k++) {
      ch->persist -= 1;
      res = dir_track(ch, ch->specials.hunting);
      if (res!= -1) {
	go_direction(ch, res);
      } else {
	ch->persist = 0;
        REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	ch->specials.hunting = 0;
	ch->hunt_dist = 0;
      }
    }
  } else {
    ch->persist = 0;
    REMOVE_BIT(ch->specials.act, ACT_HUNTING);
  }	       
}
  
void MobScavenge(struct char_data *ch)
{
  struct obj_data *best_obj, *obj;
  int max;
  
  if ((real_roomp(ch->in_room))->contents && !number(0,5)) {
    for (max = 1,best_obj = 0,obj = (real_roomp(ch->in_room))->contents;
	 obj; obj = obj->next_content) {
      if (CAN_GET_OBJ(ch, obj)) {
	if (obj->obj_flags.cost > max) {
	  best_obj = obj;
	  max = obj->obj_flags.cost;
	}
      }
    } /* for */
    
    if (best_obj) {
      if (CheckForAnyTrap(ch, best_obj))
	return;
      
      obj_from_room(best_obj);
      obj_to_char(best_obj, ch);
      act("$n gets $p.",FALSE,ch,best_obj,0,TO_ROOM);
    }
  }
}
  
void mobile_healer(struct char_data *ch)
{
  if (GET_HIT(ch) < GET_MAX_HIT(ch) / 2) {
    if (GET_POS(ch) > POSITION_SLEEPING) {
      vlog(LOG_MISC,"Healing mobile: %s in room %d with hp %d/%d", GET_NAME(ch),ch->in_room,
	   GET_HIT(ch),GET_MAX_HIT(ch));
      GET_HIT(ch) += 200;
      if (GET_HIT(ch) >= GET_MAX_HIT(ch))
        GET_HIT(ch) = GET_MAX_HIT(ch) - dice(1,8);
      update_pos(ch);
     act("$n pulls out a pill and instantly swallows it.",FALSE,ch,0,0,TO_ROOM);
    }
  }

  return;
}

static void mobile_act(struct char_data *ch)
{
  struct char_data *tmp_ch;
  int k;
  extern int no_specials;
      
  if (ch && IS_MOB(ch)) {
/* some status checking for errors */
    if ((ch->in_room != NOWHERE) && !hash_find(&room_db,ch->in_room)) {
      vlog(LOG_URGENT,"Char not in correct room.  Moving %s to 0.",
		ch->player.name);
      char_from_room(ch);
      char_to_room(ch, 0);
    }
/* check for special procs */
    if (IS_SET(ch->specials.act, ACT_SPEC) && !no_specials) {
      if (!real_mobp(ch->virtual)->func) {
	if (DEBUG) vlog(LOG_DEBUG,"mobact: non-existant mob func for %s",
		ch->player.name);
	REMOVE_BIT(ch->specials.act, ACT_SPEC);
        mobs_with_special--;
      } else {
	if ((*real_mobp(ch->virtual)->func) (ch, 0, ""))
	  return;
      }
    }

    /* new bit which makes mobiles smarter - SLB */
    if(act_state(ch)) return;
      
/* Added this code so that mobs can heal themselves after so many ticks */
/* pass by.  Done 3 times per tick.  (Zucan 10/28)                      */
    if (IS_SET(ch->specials.act, ACT_HEALER))
      if (!(PULSE % PULSE_HEALER))
	mobile_healer(ch);

/* check to see if the monster is possessed */
    if (AWAKE(ch) && (!ch->specials.fighting) && (!ch->desc)) {
      /*AssistFriend(ch);*/

      if (IS_SET(ch->specials.act, ACT_SCAVENGER) && !IsHumanoid(ch))
	MobScavenge(ch);

      if (IS_SET(ch->specials.act, ACT_HUNTING))
	MobHunt(ch);
      else if ((!IS_SET(ch->specials.act, ACT_SENTINEL)))
	mobile_wander(ch);

      if (GET_HIT(ch) > (GET_MAX_HIT(ch)/2)) {
	if (IS_SET(ch->specials.act, ACT_HATEFUL)) {
	  tmp_ch = FindAHatee(ch);
	  if (tmp_ch) {
	    if (check_peaceful(ch, "You ask your mortal enemy to step outside to settle matters.\n"))
	      act("$n growls '$N, would you care to step outside where we can settle this?'", TRUE, ch, 0, tmp_ch, TO_ROOM);
	    else {
	      if (IsHumanoid(ch))
		act("$n screams 'I'm gonna kill you!'", TRUE, ch, 0, 0, TO_ROOM); 
	      else if (IsAnimal(ch))
		act("$n growls", TRUE, ch, 0, 0, TO_ROOM);
	      hit(ch,tmp_ch,0);
              return;
	    }
	  }
	}
	if (!ch->specials.fighting && GET_POS(ch)!=POSITION_DEAD)
	  if (IS_SET(ch->specials.act, ACT_AFRAID))
	    if ((tmp_ch = FindAFearee(ch))!= NULL)
	      do_flee(ch, "", 0);
      } else { 
	if (IS_SET(ch->specials.act, ACT_AFRAID) && GET_POS(ch)!=POSITION_DEAD) {
	  if ((tmp_ch = FindAFearee(ch))!= NULL) {
	    do_flee(ch, "", 0);
	  } else {
	    if (IS_SET(ch->specials.act, ACT_HATEFUL)) {
	      tmp_ch = FindAHatee(ch);
	      if (tmp_ch) {
		if (check_peaceful(ch, "You ask your mortal enemy to step outside to settle matters.\n")) {
		  act("$n growls '$N, would you care to step outside where we can settle this?'", TRUE, ch, 0, tmp_ch, TO_ROOM);
		} else {
		  if (IsHumanoid(ch)) {
		    act("$n screams 'I'm gonna get you!'", TRUE, ch, 0, 0, TO_ROOM); 
		  } else if (IsAnimal(ch)) {
		    act("$n growls", TRUE, ch, 0, 0, TO_ROOM);
		  }
		  hit(ch,tmp_ch,0);
                  return;
		}
	      }
	    }	 		
	  }
	}
      }
      if (IS_SET(ch->specials.act,ACT_AGGRESSIVE)) {
	for (k=0;k<=5;k++) {
	  tmp_ch = FindVictim(ch);
	  if (tmp_ch) {
	    if (check_peaceful(ch, "You can't seem to exercise your violent tendencies.\n")) {
	      act("$n growls impotently", TRUE, ch, 0, 0, TO_ROOM);
	    } else {
              hit(ch, tmp_ch, 0);
              return;
            }
	    k = 10;
	  }
	}
      }
      if (IS_SET(ch->specials.act, ACT_GUARDIAN)) {
	mobile_guardian(ch);
      }
    } /* If AWAKE(ch)   */
  }   /* If IS_MOB(ch)  */
}
  
  
  
int SameRace( struct char_data *ch1, struct char_data *ch2)
{
  
  if ((!ch1) || (!ch2))
    return(FALSE);
  
  if (ch1 == ch2)
    return(TRUE);
  
  if (IS_NPC(ch1) && (IS_NPC(ch2)))
    if (ch1->virtual==ch2->virtual);
      return (TRUE);
  
  if (in_group(ch1,ch2))
    return(TRUE);
  
  if (GET_RACE(ch1) == GET_RACE(ch2)) {
    return(TRUE);
  }
  
  return(FALSE);
}

int AssistFriend( struct char_data *ch)
{
  struct char_data *damsel, *targ, *tmp_ch;
  int t, found;
      
      
  damsel = 0;
  targ = 0;
      
  if (check_peaceful(ch, ""))
    return(0);
      
/* find the people who are fighting */
      
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch; tmp_ch=tmp_ch->next_in_room) {
    if (CAN_SEE(ch,tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY)) {
	if (IS_NPC(tmp_ch) && (SameRace(tmp_ch,ch))) {
	  if (tmp_ch->specials.fighting)
	    damsel = tmp_ch;
	}
      }
    }
  }
      
  if (damsel) {
/* check if the people in the room are fighting. */
    found = FALSE;
    for (t=1; t<=8 && !found;t++) {
      targ = FindAnyVictim(damsel);
      if (targ) {
	if (targ->specials.fighting)
	  if (SameRace(targ->specials.fighting, ch))
	    found = TRUE;
      }
    }
    if (targ)
      if (targ->in_room == ch->in_room) {
	if (!IS_AFFECTED(ch, AFF_CHARM) || ch->master != targ) {
	  hit(ch,targ,0);
	}
      }
  }
  return(1);
}

int mobile_alive(struct char_data *mob)
{
  struct char_data *ch;

  for(ch = character_list; ch; ch = ch->next){
    if(mob == ch) return(1);
  }

  return(0);
}

void event_mobile(event_t *event)
{
  if(!event->ch->desc){
    mobile_act(event->ch);
  }
  if(event->ch->specials.fighting){
    add_event(PULSE_VIOLENCE, EVT_MOBILE, event->ch, NULL, NULL, -1, 0, NULL, event_mobile);
  }
  else{
    add_event(PULSE_MOBILE, EVT_MOBILE, event->ch, NULL, NULL, -1, 0, NULL, event_mobile);
  }
}

void for_each_char(void (*to_do)(struct char_data *))
{
  struct char_data *ch;
  int i,count;
  struct char_data **list;

  for (count=0,ch=character_list; ch; ch=ch->next) count++;
  CREATE(list,struct char_data *,count);
  for (i=0,ch=character_list; i<count; i++,ch=ch->next)
    list[i]=ch;

  for (i=0; i<count; i++){
    for (ch=character_list; ch && ch!=list[i]; ch=ch->next);
    if (ch==list[i])
      to_do(list[i]);
  }
  FREE(list);

#if 0
  ll *to_process,*p;

  to_process=NULL;
  for (ch=character_list; ch; ch=ch->next) {
    CREATE(p,ll,1);
    p->next=to_process;
    p->p=ch;
    to_process=p;
  }
  while (to_process) {
    for (ch=character_list; ch && ch!=to_process->p; ch=ch->next) ;
    if (ch) {
	if (valid_player(ch)) to_do(ch);
	else vlog(LOG_DEBUG,"Serious problem avoided in for_each_char");
    } else vlog(LOG_DEBUG,"Avoided a problem in for_each_char");
    p=to_process;
    to_process=p->next;
    FREE(p);
  }
#endif
}

void mobile_activity(void)
{
  for_each_char(mobile_act);
}
