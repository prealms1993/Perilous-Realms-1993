/* ************************************************************************
*  file: act.obj2.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Commands mainly using objects.                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include <fcns.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <spells.h>
#include <externs.h>

void weight_change_object(struct obj_data *obj, int weight)
{
  struct obj_data *tmp_obj;
  struct char_data *tmp_ch;

  if ((tmp_ch = obj->carried_by)) {
    obj_from_char(obj);
    GET_OBJ_MASS(obj) += weight;
    obj_to_char(obj, tmp_ch);
  } else if ((tmp_obj = obj->in_obj)) {
    obj_from_obj(obj);
    GET_OBJ_MASS(obj) += weight;
    obj_to_obj(obj, tmp_obj);
  } else {
    GET_OBJ_MASS(obj) += weight;
  }
}

void name_from_drinkcon(struct obj_data *obj)
{
  int i;
  char *new_name;

  for(i=0; (*((obj->name)+i)!=' ') && (*((obj->name)+i)!='\0'); i++)  ;

  if (*((obj->name)+i)==' ') {
    new_name=mystrdup((obj->name)+i+1);
    FREE(obj->name);
    obj->name=new_name;
  }
}



void name_to_drinkcon(struct obj_data *obj,int type)
{
  char *new_name;
  extern char *drinknames[];

  CREATE(new_name,char,strlen(obj->name)+strlen(drinknames[type])+2);
  sprintf(new_name,"%s %s",drinknames[type],obj->name);
  FREE(obj->name);
  obj->name=new_name;
}


void do_fill(struct char_data *ch, char *arg, int cmd)
{
  struct obj_data *from, *to;
  int can_hold;
  char from_name[MAX_INPUT_LENGTH],to_name[MAX_INPUT_LENGTH];

  arg=one_argument(arg,to_name);
  arg=one_argument(arg,from_name);
  if (is_abbrev("from",from_name)) arg=one_argument(arg,from_name);
  if (!*to_name || !*from_name) {
    send_to_char("Syntax: fill object {from} object\nExample:\n"
		 "  fill barrel fountain\n  fill cup from barrel\n",ch);
  }
  if (!(to=get_obj_in_list_vis(ch,to_name,ch->carrying))) {
    sendf(ch,"You don't have a %s to fill.\n",to_name);
    return;
  }
  if (!(from=get_obj_in_list_vis(ch,from_name,real_roomp(ch->in_room)->contents)))
  if (!(from=get_obj_in_list_vis(ch,from_name,ch->carrying))) {
    sendf(ch,"You don't see a %s to fill from.\n",from_name);
    return;
  }
  if (to->obj_flags.type_flag!=ITEM_DRINKCON) {
    sendf(ch,"%s isn't a container\n",to_name);
    return;
  }
  if (from->obj_flags.type_flag!=ITEM_DRINKCON) {
    sendf(ch,"%s isn't a container\n",from_name);
    return;
  }
  if (to->obj_flags.value[2]!=from->obj_flags.value[2]) {
    send_to_char("Sorry, can't mix container types yet.\n",ch);
    return;
  }
  if (from->obj_flags.value[1]==0) {
    sendf(ch,"The %s is already empty.\n",from_name);
    return;
  }
  if (to->obj_flags.value[1]==to->obj_flags.value[0]) {
    sendf(ch,"The %s is already full.\n",to_name);
    return;
  }
  can_hold=to->obj_flags.value[0]-to->obj_flags.value[1];
  if (can_hold>from->obj_flags.value[1])
    can_hold=from->obj_flags.value[1];
  to->obj_flags.value[1]+=can_hold;
/*  if (to->obj_flags.value[0] > 20) - SLB */
     weight_change_object(to, can_hold * 50);
/*   if (from->obj_flags.value[0]>20) - SLB */
     weight_change_object(from, -(can_hold * 50));
  if (to->obj_flags.value[1]==to->obj_flags.value[0])
    sendf(ch,"The %s is now full.\n",to_name);
  else
    sendf(ch,"You emptied the %s.\n",from_name);
  act("$n fills $p from $P\n",TRUE,ch,to,from,TO_ROOM);
}

void do_drink(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  struct obj_data *temp;
  struct affected_type af;
  int amount;

  only_argument(argument,buf);

  if (!(temp=get_obj_in_list_vis(ch,buf,real_roomp(ch->in_room)->contents)))
    if (!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
      act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
      return;
  }

  if (!ITEM_TYPE(temp,ITEM_DRINKCON)) {
    act("You can't drink from that!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

/* The pig is drunk */
  if ((GET_COND(ch,DRUNK)>15)&&(GET_COND(ch,THIRST)>0)) {
    act("You're just sloshed.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n is looks really drunk.", TRUE, ch, 0, 0, TO_ROOM);
    return;
  }

/* Stomach full */
  if ((GET_COND(ch,FULL)>20)&&(GET_COND(ch,THIRST)>0)) {
    act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if (VALUE1(temp)<=0) {
    act("It's empty already.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  sprintf(buf,"$n drinks %s from $p",drinks[VALUE2(temp)]);
  act(buf, TRUE, ch, temp, 0, TO_ROOM);
  sendf(ch,"You drink the %s\n",drinks[VALUE2(temp)]);

  if (drink_aff[VALUE2(temp)][DRUNK] > 0 )
    amount = (25-GET_COND(ch,THIRST))/drink_aff[VALUE2(temp)][DRUNK];
  else
    amount = number(3,10);

  amount = MIN(amount,VALUE1(temp));

/* Subtract amount */
/*  if (temp->obj_flags.value[0] > 20)  always do it with refined vals - SLB */
     weight_change_object(temp, -(amount * 50));  

  gain_condition(ch,DRUNK,drink_aff[VALUE2(temp)][DRUNK]*amount/4);
  gain_condition(ch,FULL,drink_aff[VALUE2(temp)][FULL]*amount/4);
  gain_condition(ch,THIRST,drink_aff[VALUE2(temp)][THIRST]*amount/4);

  if (GET_COND(ch,DRUNK)>10)
    act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);

  if (GET_COND(ch,THIRST)>20)
    act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);

  if (GET_COND(ch,FULL)>20)
    act("You are full.",FALSE,ch,0,0,TO_CHAR);

/* The shit was poisoned ? */
  if (VALUE3(temp)) {
    act("Oops, it tasted rather strange ?!!?",FALSE,ch,0,0,TO_CHAR);
    act("$n chokes and utters some strange sounds.",TRUE,ch,0,0,TO_ROOM);
    af.type = SPELL_POISON;
    af.duration = amount*3;
    af.modifier = 0;
    af.location = APPLY_NONE;
    init_set(af.bitvector,AFF_POISON);
    affect_join(ch,&af, FALSE, FALSE);
  }

/* empty the container, and no longer poison. */
  VALUE1(temp) -= amount;
  if (VALUE1(temp)<=0) {
    if (VALUE0(temp) < 20) {  
      extract_obj(temp);  /* get rid of it */
      return;
    }
    VALUE2(temp)=0;
    VALUE3(temp)=0;
    name_from_drinkcon(temp);
  }
}



void do_eat(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int j, num;
  struct obj_data *temp;
  struct affected_type af;

  one_argument(argument,buf);

  if (!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
    act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if (!ITEM_TYPE(temp,ITEM_FOOD) && GET_LEVEL(ch) < DEMIGOD) {
    act("Your stomach refuses to eat that!?!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

/* Stomach full */
  if (GET_COND(ch,FULL)>20) {
    act("You are to full to eat more!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  act("$n eats $p",TRUE,ch,temp,0,TO_ROOM);
  act("You eat the $o.",FALSE,ch,temp,0,TO_CHAR);

  gain_condition(ch,FULL,VALUE0(temp));

  if (GET_COND(ch,FULL)>20)
    act("You are full.",FALSE,ch,0,0,TO_CHAR);

/* hit 'em with the spell (if any) */
  for (j=0; j<MAX_OBJ_AFFECT; j++)
    if (temp->affected[j].location == APPLY_EAT_SPELL) {
      num = temp->affected[j].modifier;
      if (!spell_info[num].spell_pointer)
        vlog(LOG_URGENT,"Vnum %d: EAT_SPELL %d", temp->virtual, num);
      else
	((*spell_info[num].spell_pointer)(6, ch, "", SPELL_TYPE_POTION, ch, 0,6));
    }

  if (VALUE3(temp) && (GetMaxLevel(ch) < LOW_IMMORTAL)) {
   act("That tasted rather strange !!",FALSE,ch,0,0,TO_CHAR);
   act("$n coughs and utters some strange sounds.",FALSE,ch,0,0,TO_ROOM);

    af.type = SPELL_POISON;
    af.duration = temp->obj_flags.value[0]*2;
    af.modifier = 0;
    af.location = APPLY_NONE;
    init_set(af.bitvector,AFF_POISON);
    affect_join(ch,&af, FALSE, FALSE);
  }
  extract_obj(temp);
}


void do_pour(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *from_obj;
  struct obj_data *to_obj;
  int temp;

  argument_interpreter(argument, arg1, arg2);

/* No arguments */
  if (!*arg1) {
    act("What do you want to pour from?",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if (!(from_obj = get_obj_in_list_vis(ch,arg1,ch->carrying)))	{
    act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if (from_obj->obj_flags.type_flag!=ITEM_DRINKCON)	{
    act("You can't pour from that!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if (VALUE1(from_obj)<=0) {
    act("The $p is empty.",FALSE, ch,from_obj, 0,TO_CHAR);
    return;
  }

  if (!*arg2)	{
    act("Where do you want it? Out or in what?",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if (!str_cmp(arg2,"out")) {
    act("$n empties $p", TRUE, ch,from_obj,0,TO_ROOM);
    act("You empty the $p.", FALSE, ch,from_obj,0,TO_CHAR);

    weight_change_object(from_obj, -(from_obj->obj_flags.value[1] * 50));

    from_obj->obj_flags.value[1]=0;
    from_obj->obj_flags.value[2]=0;
    from_obj->obj_flags.value[3]=0;
    name_from_drinkcon(from_obj);
    
    return;
  }

  if (!(to_obj = get_obj_in_list_vis(ch,arg2,ch->carrying)))  {
    act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if (!ITEM_TYPE(to_obj,ITEM_DRINKCON)) {
    act("You can't pour anything into that.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if (VALUE1(to_obj) && VALUE2(to_obj)!=VALUE2(from_obj)) {
    act("There is already another liquid in it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if (VALUE1(to_obj)>=VALUE0(to_obj)) {
    act("There is no room for more.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  sendf(ch,"You pour the %s into the %s.",drinks[VALUE2(from_obj)],arg2);

  /* New alias */
  if (VALUE1(to_obj)==0)
    name_to_drinkcon(to_obj,VALUE2(from_obj));

  /* First same type liq. */
  VALUE2(to_obj)=VALUE2(from_obj);

  temp = VALUE1(from_obj);
  VALUE1(from_obj)=0;
  VALUE1(to_obj) += temp;
  temp = VALUE1(to_obj)-VALUE0(to_obj);

  if (temp>0)
    VALUE1(from_obj)=temp;
  else
    name_from_drinkcon(from_obj);

  if (VALUE1(from_obj) > VALUE0(from_obj))
    VALUE1(from_obj)=VALUE0(from_obj);

  /* Then the poison boogie */
  VALUE3(to_obj)=VALUE3(to_obj) || VALUE3(from_obj);
}

void do_sip(struct char_data *ch, char *argument, int cmd)
{
	struct affected_type af;
	char arg[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	struct obj_data *temp;

	one_argument(argument,arg);

	if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))
	{
		act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
		return;
	}

	if(temp->obj_flags.type_flag!=ITEM_DRINKCON)
	{
		act("You can't sip from that!",FALSE,ch,0,0,TO_CHAR);
		return;
	}

	if(GET_COND(ch,DRUNK)>10) /* The pig is drunk ! */
	{
		act("You simply fail to reach your mouth!",FALSE,ch,0,0,TO_CHAR);
		act("$n tries to sip, but fails!",TRUE,ch,0,0,TO_ROOM);
		return;
	}

	if(!temp->obj_flags.value[1])  /* Empty */
	{
		act("But there is nothing in it?",FALSE,ch,0,0,TO_CHAR);
		return;
	}

	act("$n sips from the $o",TRUE,ch,temp,0,TO_ROOM);
	sprintf(buf,"It tastes like %s.\n",drinks[temp->obj_flags.value[2]]);
	send_to_char(buf,ch);

	gain_condition(ch,DRUNK,(int)(drink_aff[temp->obj_flags.value[2]][DRUNK]/4));

	gain_condition(ch,FULL,(int)(drink_aff[temp->obj_flags.value[2]][FULL]/4));

	gain_condition(ch,THIRST,(int)(drink_aff[temp->obj_flags.value[2]][THIRST]/4));

	weight_change_object(temp, -50);  /* Subtract one unit */

	if(GET_COND(ch,DRUNK)>10)
		act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);

	if(GET_COND(ch,THIRST)>20)
		act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);

	if(GET_COND(ch,FULL)>20)
		act("You are full.",FALSE,ch,0,0,TO_CHAR);

	if(temp->obj_flags.value[3]&&!IS_AFFECTED(ch,AFF_POISON)) /* The shit was poisoned ! */
	{
		act("But it also had a strange taste!",FALSE,ch,0,0,TO_CHAR);

		af.type = SPELL_POISON;
		af.duration = 3;
		af.modifier = 0;
		af.location = APPLY_NONE;
		init_set(af.bitvector,AFF_POISON);
		affect_to_char(ch,&af);
	}

	temp->obj_flags.value[1]--;

	if(!temp->obj_flags.value[1])  /* The last bit */
	{
		temp->obj_flags.value[2]=0;
		temp->obj_flags.value[3]=0;
		name_from_drinkcon(temp);
	}

	return;

}


void do_taste(struct char_data *ch, char *argument, int cmd)
{
	struct affected_type af;
	char arg[80];
	struct obj_data *temp;

	one_argument(argument,arg);

	if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))
	{
		act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
		return;
	}

	if(temp->obj_flags.type_flag==ITEM_DRINKCON)
	{
		do_sip(ch,argument,0);
		return;
	}

	if(!(temp->obj_flags.type_flag==ITEM_FOOD))
	{
		act("Taste that?!? Your stomach refuses!",FALSE,ch,0,0,TO_CHAR);
		return;
	}

	act("$n tastes the $o", FALSE, ch, temp, 0, TO_ROOM);
	act("You taste the $o", FALSE, ch, temp, 0, TO_CHAR);

	gain_condition(ch,FULL,1);

	if(GET_COND(ch,FULL)>20)
		act("You are full.",FALSE,ch,0,0,TO_CHAR);

	if(temp->obj_flags.value[3]&&!IS_AFFECTED(ch,AFF_POISON)) /* The shit was poisoned ! */
	{
		act("Ooups, it did not taste good at all!",FALSE,ch,0,0,TO_CHAR);

		af.type = SPELL_POISON;
		af.duration = 2;
		af.modifier = 0;
		af.location = APPLY_NONE;
		init_set(af.bitvector,AFF_POISON);
		affect_to_char(ch,&af);
	}

	temp->obj_flags.value[0]--;

	if(!temp->obj_flags.value[0])	/* Nothing left */
	{
		act("There is nothing left now.",FALSE,ch,0,0,TO_CHAR);
		extract_obj(temp);
	}

	return;

}



/* functions related to wear */

char *perform_wear_messages[] = {
    "$n lights $p and holds it.",
    "$n wears $p on $s finger.",
    "$n wears $p around $s neck.",
    "$n wears $p on $s body.",
    "$n wears $p on $s head.",
    "$n wears $p on $s legs.",
    "$n wears $p on $s feet.",
    "$n wears $p on $s hands.",
    "$n wears $p on $s arms.",
    "$n wears $p about $s body.",
    "$n wears $p about $s waist.",
    "$n wears $p around $s wrist.",
    "$n wields $p.",
    "$n grabs $p.",
    "$n starts using $p as shield.",
    "$n starts using $p as a spell pouch."
};

void perform_wear(struct char_data *ch, struct obj_data *obj_object, int keyword)
{
  act(perform_wear_messages[keyword], FALSE, ch, obj_object,0,TO_ROOM);
}


int wear(struct char_data *ch, struct obj_data *obj, int keyword)
{
  char buffer[MAX_STRING_LENGTH];
  
  if (!IS_IMMORTAL(ch)) {
    if (IS_MAGIC(obj) && IsClass(ch,BARBARIAN)) {
	send_to_char("You get an uncomfortable feeling.\n",ch);
	send_to_char("Maybe its magic... Maybe you should destroy it!\n",ch);
	return(0);
    }
  if (!can_have(ch, obj)) { /* check lvl limits */ 
    sprintf(buffer,"You can't figure out how to use %s\n", obj->short_description);
    send_to_char(buffer, ch);
    send_to_char("Try again later when your wiser and have more experience.\n",ch);
    return(0);
  } /* UNCOMMENT THIS WHEN ANTI ITEMS ARE FIXED
  if ((is_fighter(ch) && IS_SET(obj->obj_flags.extra_flags,ITEM_ANTI_FIGHTER))
     || (is_cleric(ch) && IS_SET(obj->obj_flags.extra_flags,ITEM_ANTI_CLERIC))
     || (is_magicu(ch) && IS_SET(obj->obj_flags.extra_flags,ITEM_ANTI_MAGE))
     || (is_thief(ch) && IS_SET(obj->obj_flags.extra_flags,ITEM_ANTI_THIEF)))
    if (!IS_NPC(ch)) {
      send_to_char("You are forbidden to do that.\n", ch);
      return;
    } */
  }

  if (!IsHumanoid(ch)) {
    if ((keyword != 13) || (!HasHands(ch))) {
      send_to_char("You can't wear things!\n",ch);
      return(0);
    }
  }
  
  if (!IS_NPC(ch) && IS_SET(obj->obj_flags.wear_flags,ITEM_CONSTRUCTION)
       && !IS_IMMORTAL(ch)) {
    send_to_char("That item is flagged as a contruction item\n"
		"You aren't allowed to use it.\n",ch);
    vlog(LOG_URGENT, "%s just tried to use a %s -- CONTRUCTION (%d)",
		GET_NAME(ch),obj->name, obj->virtual);
    return(0);
  }

  if (IS_SET (obj->obj_flags.wear_flags, ITEM_DAMAGED)) {
	if (!IS_NPC(ch))
		send_to_char ("You can not use a damaged item.\n", ch);
  return(0);
  }

  switch(keyword) {
  case 0: {  /* LIGHT SOURCE */
    if (ch->equipment[WEAR_LIGHT]){
      send_to_char("You are already holding a light source.\n", ch);
      return(0);
    }
    else {
      send_to_char("Ok.\n", ch);
      perform_wear(ch,obj,keyword);
      obj_from_char(obj);
      equip_char(ch,obj, WEAR_LIGHT);
      if (obj->obj_flags.value[2])
	real_roomp(ch->in_room)->light++;
    }
  } break;
    
  case 1: {
    if (CAN_WEAR(obj,ITEM_WEAR_FINGER)) {
      if ((ch->equipment[WEAR_FINGER_L]) && (ch->equipment[WEAR_FINGER_R])) {
	send_to_char("You are already wearing something on your fingers.\n",
          ch);
        return(0);
      } else {
	perform_wear(ch,obj,keyword);
	if (ch->equipment[WEAR_FINGER_L]) {
	  sprintf(buffer, "You put %s on your right finger.\n",	obj->short_description);
	  send_to_char(buffer, ch);
	  obj_from_char(obj);
	  equip_char(ch, obj, WEAR_FINGER_R);
	} else {
	  sprintf(buffer, "You put %s on your left finger.\n", obj->short_description);
	  send_to_char(buffer, ch);
	  obj_from_char(obj);
	  equip_char(ch, obj, WEAR_FINGER_L);
	}
      }
    } else {
      send_to_char("You can't wear that on your finger.\n", ch);
      return(0);
    }
  } break;
  case 2: {
    if (CAN_WEAR(obj,ITEM_WEAR_NECK)) {
      if ((ch->equipment[WEAR_NECK_1]) && (ch->equipment[WEAR_NECK_2])) {
	send_to_char("You can't wear any more around your neck.\n", ch);
        return(0);
      } else {
	send_to_char("OK.\n", ch);
	perform_wear(ch,obj,keyword);
	if (ch->equipment[WEAR_NECK_1]) {
	  obj_from_char(obj);
	  equip_char(ch, obj, WEAR_NECK_2);
	} else {
	  obj_from_char(obj);
	  equip_char(ch, obj, WEAR_NECK_1);
	}
      }
    } else {
      send_to_char("You can't wear that around your neck.\n", ch);
      return(0);
    }
  } break;
  case 3: {
    if (CAN_WEAR(obj,ITEM_WEAR_BODY)) {
      if (ch->equipment[WEAR_BODY]) {
	send_to_char("You already wear something on your body.\n", ch);
        return(0);
      } else {
	send_to_char("OK.\n", ch);
	perform_wear(ch,obj,keyword);
	obj_from_char(obj);
	equip_char(ch,  obj, WEAR_BODY);
      }
    } else {
      send_to_char("You can't wear that on your body.\n", ch);
      return(0);
    }
  } break;
  case 4: {
    if (CAN_WEAR(obj,ITEM_WEAR_HEAD)) {
      if (ch->equipment[WEAR_HEAD]) {
	send_to_char("You already wear something on your head.\n", ch);
        return(0);
      } else {
	send_to_char("OK.\n", ch);
	perform_wear(ch,obj,keyword);
	obj_from_char(obj);
	equip_char(ch, obj, WEAR_HEAD);
      }
    } else {
      send_to_char("You can't wear that on your head.\n", ch);
      return(0);
    }
  } break;
  case 5: {
    if (CAN_WEAR(obj,ITEM_WEAR_LEGS)) {
      if (ch->equipment[WEAR_LEGS]) {
	send_to_char("You already wear something on your legs.\n", ch);
        return(0);
      } else {
	send_to_char("OK.\n", ch);
	perform_wear(ch,obj,keyword);
	obj_from_char(obj);
	equip_char(ch, obj, WEAR_LEGS);
      }
    } else {
      send_to_char("You can't wear that on your legs.\n", ch);
      return(0);
    }
  } break;
  case 6: {
    if (CAN_WEAR(obj,ITEM_WEAR_FEET)) {
      if (ch->equipment[WEAR_FEET]) {
	send_to_char("You already wear something on your feet.\n", ch);
        return(0);
      } else {
	send_to_char("OK.\n", ch);
	perform_wear(ch,obj,keyword);
	obj_from_char(obj);
	equip_char(ch, obj, WEAR_FEET);
      }
    } else {
      send_to_char("You can't wear that on your feet.\n", ch);
      return(0);
    }
  } break;
  case 7: {
    if (CAN_WEAR(obj,ITEM_WEAR_HANDS)) {
      if (ch->equipment[WEAR_HANDS]) {
	send_to_char("You already wear something on your hands.\n", ch);
        return(0);
      } else {
	send_to_char("OK.\n", ch);
	perform_wear(ch,obj,keyword);
	obj_from_char(obj);
	equip_char(ch, obj, WEAR_HANDS);
      }
    } else {
      send_to_char("You can't wear that on your hands.\n", ch);
      return(0);
    }
  } break;
  case 8: {
    if (CAN_WEAR(obj,ITEM_WEAR_ARMS)) {
      if (ch->equipment[WEAR_ARMS]) {
	send_to_char("You already wear something on your arms.\n", ch);
        return(0);
      } else {
	send_to_char("OK.\n", ch);
	perform_wear(ch,obj,keyword);
	obj_from_char(obj);
	equip_char(ch, obj, WEAR_ARMS);
      }
    } else {
      send_to_char("You can't wear that on your arms.\n", ch);
      return(0);
    }
  } break;
  case 9: {
    if (CAN_WEAR(obj,ITEM_WEAR_ABOUT)) {
      if (ch->equipment[WEAR_ABOUT]) {
	send_to_char("You already wear something about your body.\n", ch);
        return(0);
      } else {
	send_to_char("OK.\n", ch);
	perform_wear(ch,obj,keyword);
	obj_from_char(obj);
	equip_char(ch, obj, WEAR_ABOUT);
      }
    } else {
      send_to_char("You can't wear that about your body.\n", ch);
      return(0);
    }
  } break;
  case 10: {
    if (CAN_WEAR(obj,ITEM_WEAR_WAISTE)) {
      if (ch->equipment[WEAR_WAISTE]) {
	send_to_char("You already wear something about your waiste.\n", ch);
        return(0);
      } else {
	send_to_char("OK.\n", ch);
	perform_wear(ch,obj,keyword);
	obj_from_char(obj);
	equip_char(ch,  obj, WEAR_WAISTE);
      }
    } else {
      send_to_char("You can't wear that about your waist.\n", ch);
      return(0);
    }
  } break;
  case 11: {
    if (CAN_WEAR(obj,ITEM_WEAR_WRIST)) {
      if ((ch->equipment[WEAR_WRIST_L]) && (ch->equipment[WEAR_WRIST_R])) {
	send_to_char(
          "You already wear something around both your wrists.\n", ch);
        return(0);
      } else {
	perform_wear(ch,obj,keyword);
	obj_from_char(obj);
	if (ch->equipment[WEAR_WRIST_L]) {
	  sprintf(buffer, "You wear the %s around your right wrist.\n", obj->short_description);
	  send_to_char(buffer, ch);
	  equip_char(ch,  obj, WEAR_WRIST_R);
	} else {
	  sprintf(buffer, "You wear the %s around your left wrist.\n", 	obj->short_description);
	  send_to_char(buffer, ch);
	  equip_char(ch, obj, WEAR_WRIST_L);
	}
      }
    } else {
      send_to_char("You can't wear that around your wrist.\n", ch);
      return(0);
    }
  } break;
    
  case 12:
    if (CAN_WEAR(obj,ITEM_WIELD)) {
      if (ch->equipment[WIELD]) {
	send_to_char("You are already wielding something.\n", ch);
        return(0);
      } else {
	if (GET_OBJ_WEIGHT(obj) >
	    str_app[GET_STR(ch)].wield_w) {
	  send_to_char("It is too heavy for you to use.\n",ch);
          return(0);
        }
	else if(IS_TWO_HANDED(obj) &&
	    (ch->equipment[HOLD] || ch->equipment[WEAR_SHIELD])){
          if(ch->equipment[HOLD])
	    act("You need both hands free to wield $p.", FALSE, ch, obj, NULL,
	      TO_CHAR);
          else
	    act("You can't wield a two-handed weapon while wearing a shield.",
	      FALSE, ch, NULL, NULL, TO_CHAR);
          return(0);
        }
        else if(IS_WEAPON(ch->equipment[HOLD]) ? (GET_OBJ_LENGTH(obj) <= GET_OBJ_LENGTH(ch->equipment[HOLD])) : 0){
	  act("You can't balance $p with the weapon in your other hand.",
	    FALSE, ch, obj, NULL, TO_CHAR);
          return(0);
        }
	else {
	  send_to_char("OK.\n", ch);
	  perform_wear(ch,obj,keyword);
	  obj_from_char(obj);
	  equip_char(ch, obj, WIELD);
	}
      }
    } else {
      send_to_char("You can't wield that.\n", ch);
      return(0);
    }
    break;
    
  case 13:
    if (CAN_WEAR(obj,ITEM_HOLD) || (CAN_WEAR(obj, ITEM_WIELD) && (obj->obj_flags.type_flag == ITEM_WEAPON))) {
      if (ch->equipment[HOLD]) {
	send_to_char("You are already holding something.\n", ch);
        return(0);
      }
      else if ((GET_OBJ_WEIGHT(obj) + 2) > str_app[GET_STR(ch)].wield_w) {
        send_to_char("It is too heavy for you to use.\n",ch);
        return(0);
      }
      else if(IS_TWO_HANDED(obj)){
	act("You can't hold $p in only one hand.", FALSE, ch, obj, NULL,
	  TO_CHAR);
        return(0);
      }
      else if(IS_TWO_HANDED(ch->equipment[WIELD])){
	act("You already need both hands to wield $p.", FALSE, ch,
	  ch->equipment[WIELD], NULL, TO_CHAR);
        return(0);
      }
      else if(ch->equipment[WEAR_SHIELD] && IS_WEAPON(obj)){
	act("You can't wear a shield and hold a weapon in the same hand.",
	  FALSE, ch, NULL, NULL, TO_CHAR);
        return(0);
      }
      else if((IS_WEAPON(ch->equipment[WIELD]) ? ((GET_OBJ_LENGTH(obj) >
	  GET_OBJ_LENGTH(ch->equipment[WIELD])) || (GET_OBJ_LENGTH(obj) >
	  76)) : 0)){
	act("You can't balance $p with the weapon in your other hand.",
	  FALSE, ch, obj, NULL, TO_CHAR);
        return(0);
      }
      else {
	send_to_char("OK.\n", ch);
	perform_wear(ch,obj,keyword);
	obj_from_char(obj);
	equip_char(ch, obj, HOLD);
      }
    } else {
      send_to_char("You can't hold this.\n", ch);
      return(0);
    }
    break;
  case 14: {
    if (CAN_WEAR(obj,ITEM_WEAR_SHIELD)) {
      if ((ch->equipment[WEAR_SHIELD])) {
	send_to_char("You are already using a shield\n", ch);
        return(0);
      }
      else if(IS_TWO_HANDED(ch->equipment[WIELD])){
	act("You can't wield $p and use a shield both.", FALSE, ch,
	  ch->equipment[WIELD], NULL, TO_CHAR);
        return(0);
      }
      else if(IS_WEAPON(ch->equipment[HOLD])){
        act("You can't hold a weapon and a shield in the same hand.", FALSE,
	  ch, NULL, NULL, TO_CHAR);
        return(0);
      }
      else {
	perform_wear(ch,obj,keyword);
	sprintf(buffer, "You start using the %s.\n", obj->short_description);
	send_to_char(buffer, ch);
	obj_from_char(obj);
	equip_char(ch, obj, WEAR_SHIELD);
      }
    } else {
      send_to_char("You can't use that as a shield.\n", ch);
      return(0);
    }
  } break;
  case 15: {
    if (CAN_WEAR(obj,ITEM_POUCH)) {
      if ((ch->equipment[POUCH])) {
	send_to_char("You are already using a spell pouch\n", ch);
        return(0);
      } else {
	perform_wear(ch,obj,keyword);
	sprintf(buffer, "You start using the %s.\n", obj->short_description);
	send_to_char(buffer, ch);
	obj_from_char(obj);
	equip_char(ch, obj, POUCH);
      }
    } else {
      send_to_char("You can't use that as a pouch.\n", ch);
      return(0);
    }
  } break;
  case -1: {
    sprintf(buffer,"Wear %s where?.\n", obj->short_description);
    send_to_char(buffer, ch);
    return(0);
  } break;
  case -2: {
    sprintf(buffer,"You can't wear %s.\n", obj->short_description);
    send_to_char(buffer, ch);
    return(0);
  } break;
  default: {
    vlog(LOG_URGENT,"Unknown type called in wear.");
    return(0);
  } break;
  }

  return(1);
}

int phys_wear(struct char_data *ch, struct obj_data *obj)
{
  int keyword;

  keyword = -2;

  if (CAN_WEAR(obj,ITEM_WEAR_SHIELD)) keyword = 14;
  if (CAN_WEAR(obj,ITEM_WEAR_FINGER)) keyword = 1;
  if (CAN_WEAR(obj,ITEM_WEAR_NECK)) keyword = 2;
  if (CAN_WEAR(obj,ITEM_WEAR_WRIST)) keyword = 11;
  if (CAN_WEAR(obj,ITEM_WEAR_WAISTE)) keyword = 10;
  if (CAN_WEAR(obj,ITEM_WEAR_ARMS)) keyword = 8;
  if (CAN_WEAR(obj,ITEM_WEAR_HANDS)) keyword = 7;
  if (CAN_WEAR(obj,ITEM_WEAR_FEET)) keyword = 6;
  if (CAN_WEAR(obj,ITEM_WEAR_LEGS)) keyword = 5;
  if (CAN_WEAR(obj,ITEM_WEAR_ABOUT)) keyword = 9;
  if (CAN_WEAR(obj,ITEM_WEAR_HEAD)) keyword = 4;
  if (CAN_WEAR(obj,ITEM_WEAR_BODY)) keyword = 3;
  if (CAN_WEAR(obj,ITEM_WIELD)) keyword = 12;
  if (CAN_WEAR(obj,ITEM_HOLD)) keyword = 13;
  if (CAN_WEAR(obj,ITEM_POUCH)) keyword=15;

  if(can_have(ch, obj)){ /* check lvl limits */
    return(wear(ch, obj, keyword));
  }

  return(0);
}

void do_wear(struct char_data *ch, char *argument, int cmd)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[256];
   char buffer[MAX_INPUT_LENGTH];
   struct obj_data *obj, *next_obj;
   int keyword;
   static char *keywords[] = {
	"finger",
	"neck",
	"body",
	"head",
	"legs",
	"feet",
	"hands",
	"arms",
	"about",
	"waist",
	"wrist",
	"shield",
	"pouch",
	"\n"
};

	argument_interpreter(argument, arg1, arg2);
	if (*arg1) {
            if (!strcmp(arg1,"all")) {
                
	       for (obj = ch->carrying; obj; obj = next_obj) {
	          next_obj = obj->next_content;
                  phys_wear(ch, obj);
		}

            } else {
		obj = get_obj_in_list_vis(ch, arg1, ch->carrying);
		if (obj) {
			if (*arg2) {
				keyword = search_block(arg2, keywords, FALSE); /* Partial Match */
				if (keyword == -1) {
					sprintf(buf, "%s is an unknown body location.\n", arg2);
					send_to_char(buf, ch);
				} else {
					wear(ch, obj, keyword+1);
				}
			} else {
                          phys_wear(ch, obj);
			}
		} else {
			sprintf(buffer, "You do not seem to have the '%s'.\n",arg1);
			send_to_char(buffer,ch);
		}
	    }
	} else {
		send_to_char("Wear what?\n", ch);
	}
}

int phys_wield(struct char_data *ch, struct obj_data *obj)
{
  return(wear(ch, obj, 12));
}

void do_wield(struct char_data *ch, char *argument, int cmd)
{
char arg1[MAX_STRING_LENGTH];
char arg2[MAX_STRING_LENGTH];
char buffer[MAX_STRING_LENGTH];
struct obj_data *obj_object;
int keyword = 12;

	argument_interpreter(argument, arg1, arg2);
	if (*arg1) {
		obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
		if (obj_object) {
			wear(ch, obj_object, keyword);
		} else {
			sprintf(buffer, "You do not seem to have the '%s'.\n",arg1);
			send_to_char(buffer,ch);
		}
	} else {
		send_to_char("Wield what?\n", ch);
	}
}


void do_grab(struct char_data *ch, char *argument, int cmd)
{
	char arg1[128];
	char arg2[128];
	char buffer[256];
	struct obj_data *obj_object;

	argument_interpreter(argument, arg1, arg2);

	if (*arg1) {
		obj_object = get_obj_in_list(arg1, ch->carrying);
		if (obj_object) {
			if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
				wear(ch, obj_object, WEAR_LIGHT);
			else
				wear(ch, obj_object, 13);
		} else {
			sprintf(buffer, "You do not seem to have the '%s'.\n",arg1);
			send_to_char(buffer,ch);
		}
	} else {
		send_to_char("Hold what?\n", ch);
	}
}

static struct obj_data *get_object_in_equip_vis(struct char_data *ch, char *arg, struct obj_data *equipment[], int *j)
{
  for ((*j) = 0; (*j) < MAX_WEAR ; (*j)++)
    if (equipment[(*j)])
      if (CAN_SEE_OBJ(ch,equipment[(*j)]))
        if (isname(arg, equipment[(*j)]->name))
          return(equipment[(*j)]);

  return (0);
}

int phys_remove(struct char_data *ch, int eq_pos)
{
  int to_ground = 0;
  struct obj_data *obj;

  if (!ch->equipment[eq_pos]) {
    vlog(LOG_DEBUG, "phys_remove() called with invalid equipment location");
    return(0);
  }
  if ((CAN_CARRY_V(ch) < (IS_CARRYING_V(ch) +
      GET_OBJ_VOLUME(ch->equipment[eq_pos]))) ||
      (CAN_CARRY_M(ch) < (IS_CARRYING_M(ch) +
      GET_OBJ_MASS(ch->equipment[eq_pos])))) {
    to_ground = 1;
  }
  if ((obj = unequip_char(ch, eq_pos)) != NULL) {
    if(to_ground)
      obj_to_room(obj, ch->in_room);
    else
      obj_to_char(obj, ch);
    if (obj->obj_flags.type_flag == ITEM_LIGHT)
      if (obj->obj_flags.value[2] && real_roomp(ch->in_room)->light >= 0)
        real_roomp(ch->in_room)->light--;
    if(to_ground){
      act("You stop using $p and drop it because you can't carry it.", FALSE,
        ch, obj, 0, TO_CHAR);
      act("$n stops using $p and drops it.", TRUE, ch, obj, 0, TO_ROOM);
    }
    else{
      act("You stop using $p.", FALSE, ch, obj, 0, TO_CHAR);
      act("$n stops using $p.", TRUE, ch, obj, 0, TO_ROOM);
    }
    return(1);
  }
  vlog(LOG_DEBUG, "phys_remove() exited in unexpected state");
  return(0);
}

void do_remove(struct char_data *ch, char *argument, int cmd)
{
	char arg1[128],*T,*P;
	char buffer[256];
	int Rem_List[20],Num_Equip;
	struct obj_data *obj_object;
	int j;

	one_argument(argument, arg1);

  if (*arg1) {
    if (!strcmp(arg1,"all")) {
      for (j=0;j<MAX_WEAR;j++) {
        if (ch->equipment[j]) {
          if ((IS_CARRYING_V(ch) + GET_OBJ_VOLUME(ch->equipment[j])) <=
              CAN_CARRY_V(ch))
            phys_remove(ch, j);
        } else {
          break;
        }
      }
    }
    else if (isdigit(arg1[0])) {
/* PAT-PAT-PAT */
  
/* Make a list of item numbers for stuff to remove */

      for (Num_Equip = j=0;j<MAX_WEAR;j++)
        if (ch->equipment[j])
          if ((IS_CARRYING_V(ch) + GET_OBJ_VOLUME(ch->equipment[j])) <=
              CAN_CARRY_V(ch))
            Rem_List[Num_Equip++] = j;

	    T = arg1;

	    while (isdigit(*T) && (*T != '\0'))	    {
		P = T;
	        if (strchr(T,','))		{
		     P = strchr(T,',');
		     *P = '\0';
		}
		if (atoi(T) > 0 && atoi(T) <= Num_Equip)		{
			j = Rem_List[atoi(T) - 1];
        if (ch->equipment[j])
          if ((IS_CARRYING_V(ch) + GET_OBJ_VOLUME(ch->equipment[j])) <=
              CAN_CARRY_V(ch))
            phys_remove(ch, j);
		     else 		     {
                       j = MAX_WEAR;
                     }
	        } else 	{
		   sprintf(buffer,"You dont seem to have the %s\n",T);
		   send_to_char(buffer,ch);
		}
		if (T != P) T = P + 1;
		else *T = '\0';
	    }
	  }  else {
		obj_object = get_object_in_equip_vis(ch, arg1, ch->equipment, &j);
		if (obj_object) {
                  phys_remove(ch, j);
		} else {
			send_to_char("You are not using it.\n", ch);
		}
	    }
	}
	else {
		send_to_char("Remove what?\n", ch);
	  }
}
