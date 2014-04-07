/* ************************************************************************
*  file: act.obj1.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Commands mainly moving around objects.                         *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

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
#include <externs.h>


#define WHO(from,to) (from==to?"You":GET_NAME(to))
int can_carry(struct char_data *from,struct char_data *ch, struct obj_data *obj)
{
  if ((IS_CARRYING_V(ch) + GET_OBJ_VOLUME(obj)) > CAN_CARRY_V(ch)) {
    sendf(from,"%s : %s can't carry that many items.\n",
	  obj->short_description,WHO(from,ch));
    if (from!=ch)
      sendf(ch,"%s just tried to give you a %s,\n"
	       "but you are carrying too many items.\n",
		GET_NAME(from),obj->short_description);
    return(FALSE);
  }

  if ((IS_CARRYING_M(ch) + obj->obj_flags.weight) > CAN_CARRY_M(ch)) {
    sendf(from,"%s : %s can't carry that much weight.\n",
		obj->short_description,WHO(from,ch));
    if (from!=ch)
      sendf(ch,"%s just tried to give you a %s,\n"
	       "but you are carrying too much weight.\n",
		GET_NAME(from),obj->short_description);
    return(FALSE);
  }

  if ((obj->virtual==25 && from!=ch) || !CAN_WEAR(obj,ITEM_TAKE)) {
    if (from==ch)
      sendf(ch,"%s : You can't take that.\n",obj->short_description);
    else
      sendf(from,"%s : You can't seem to let go of it.\n",
	obj->short_description);
    return(FALSE);
  }

  if (!can_have(ch,obj)) {
    if (ch==from)
      sendf(from,"%s : You aren't skillful enough to use it.\n",
		obj->short_description);
    else
      sendf(from,"%s : %s isn't skillfull enough to use it.\n",
		obj->short_description,GET_NAME(ch));
    return(FALSE);
  }

  return(TRUE);
}
		 
int phys_get(struct char_data *ch, struct obj_data *obj_object, struct obj_data *sub_object) 
{
  char buffer[256];

  if (sub_object) {
    if (!IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
      obj_from_obj(obj_object);
      obj_to_char(obj_object, ch);
      act("You get $p from $P.",0,ch,obj_object,sub_object,TO_CHAR);
      act("$n gets $p from $P.",1,ch,obj_object,sub_object,TO_ROOM);
    }
    else {
      act("$P must be opened first.",1,ch,0,sub_object,TO_CHAR);
      return(0);
    }
  }
  else {
    if (obj_object->in_room == NOWHERE) {
      obj_object->in_room = ch->in_room;
      sprintf(buffer, "OBJ:%s got %s from room -1...", GET_NAME(ch),
        obj_object->name);
      slog(buffer);
    }

    obj_from_room(obj_object);
    obj_to_char(obj_object, ch);
    act("You get $p.", 0, ch, obj_object, 0, TO_CHAR);
    act("$n gets $p.", 1, ch, obj_object, 0, TO_ROOM);
  }

  if((obj_object->obj_flags.type_flag == ITEM_MONEY) && 
      (obj_object->obj_flags.value[0] >= 1)){
    obj_from_char(obj_object);
    sprintf(buffer, "There was %d coins.\n", obj_object->obj_flags.value[0]);
    send_to_char(buffer,ch);
    GET_GOLD(ch) += obj_object->obj_flags.value[0];
    extract_obj(obj_object);
  }

  return(1);
}

int corpse_consent(struct char_data *ch,struct obj_data *o)
{
  extern struct char_data *character_list;
  struct char_data *i;

  if (!o->held_for) return(1);
  if (GET_ITEM_TYPE(o)!=ITEM_CONTAINER) return(1);
  if (str_cmp(GET_NAME(ch),o->held_for)==0) return(1);
  if (GetMaxLevel(ch)>=SILLYLORD) return(1);
  if (IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM))
    if (ch->master) {
      if (IS_NPC(ch->master))
        return(1);
    }
    else
      return(1);
  for (i=character_list; i; i=i->next)
    if (str_cmp(o->held_for,GET_NAME(i))==0) break;
  if (!i || i->consent!=ch) {
    sendf(ch,"You don't their permission.  They must type 'CONSENT %s'\n",GET_NAME(ch));
    if (i) {
      sendf(i,"%s just tried to get something from your corpse.\n",GET_NAME(ch));
      sendf(i,"You must type 'CONSENT %s' if you want them to be able to.\n",GET_NAME(ch));
    }
    return(0);
  }
  return(1);
}

void do_get(struct char_data *ch, char *argument, int cmd)
{
  struct room_data *rp;
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  struct obj_data *container;
  struct obj_data *obj;
  struct obj_data *next_obj;
  bool found = FALSE;
  bool fail  = FALSE;
  int type   = 3;
  char newarg[100];
  int num, p;

  rp=real_roomp(ch->in_room);
  argument_interpreter(argument, arg1, arg2);

  /* get type */
  if (!*arg1) {
    type = 0;
  }
  if (*arg1 && !*arg2) {
    if (!str_cmp(arg1,"all"))
      type = 1;
    else
      type = 2;
  }
  if (*arg1 && *arg2) {
    if (!str_cmp(arg1,"all")) {
      if (!str_cmp(arg2,"all"))
	type = 3;
      else
	type = 4;
    } else {
      if (!str_cmp(arg2,"all"))
	type = 5;
      else
	type = 6;
    }
  }

  switch (type) {
    case 0: { /* get */
	    send_to_char("Get what?\n", ch); 
    } break;
    case 1: { /* get all */
      container = 0;
      found = FALSE;
      fail = FALSE;
      for (obj = rp->contents; obj; obj = next_obj) {
	next_obj = obj->next_content;

	if (CheckForAnyTrap(ch, obj)) 
	    return;
	if (!corpse_consent(ch,obj)) return;

	if (CAN_SEE_OBJ(ch,obj)) {
	  if (!can_carry(ch,ch,obj)) fail=TRUE;
	  else phys_get(ch,obj,container);
        }
      }
      if (found) {
	send_to_char("OK.\n", ch);
      } else {
	if (!fail) send_to_char("You see nothing here.\n", ch);
      }
    } break;
    case 2:{ /* get ??? (something) */
      container = 0;
      found = FALSE;
      fail = FALSE;
      if (getall(arg1,newarg)!=0) {
	 strcpy(arg1,newarg);
	 num = -1;
      } else if ((p = getabunch(arg1,newarg))!=0) {
	 strcpy(arg1,newarg);
	 num = p;
      } else {
	num = 1;
      }

      while (num != 0) {
	obj=get_obj_in_list_vis(ch,arg1,rp->contents);
	if (!obj) {
	  if (found) return;
	  sendf(ch,"You do not see a %s here.\n", arg1);
	  return;
	}

	found=TRUE;
	if (!corpse_consent(ch,obj)) return;
	if (can_carry(ch,ch,obj)) {
	  phys_get(ch,obj,container);
	  found=TRUE;
	} else return;
	if (num>0) num--;
      }
    } break;
    case 3:{ /* get all all */
      send_to_char("You must be joking?!\n", ch);
    } break;
    case 4:{ /* get all ??? */
      found = FALSE;
      fail = FALSE; 
      container = get_obj_vis_accessible(ch, arg2);
      if (!container) {
	sendf(ch,"You do not have or see the %s.\n",arg2);
	return;
      }
      if (!corpse_consent(ch,container)) return;
      if (!ITEM_TYPE(container,ITEM_CONTAINER) &&
	  !ITEM_TYPE(container,ITEM_SPELL_POUCH)) {
	sendf(ch,"The %s is not a container.\n",arg2);
	return;
      }
      for (obj = container->contains; obj; obj = next_obj) {
	next_obj = obj->next_content;
	if (CheckForGetTrap(ch, obj))
	  return;
	if (!can_carry(ch,ch,obj)) fail=TRUE;
	else {
	  phys_get(ch,obj,container);
	  found = TRUE;
	}
      }
      if (!found && !fail)
	sendf(ch,"You do not see anything in %s.\n",
		container->short_description);
      return;
    } break;
    case 5: { 
	sendf(ch,"You can't take a thing from more than one container.\n");
    } break;

    case 6: { /*  take ??? from ???   (is it??) * */
      found = FALSE;
      fail = FALSE;
      container = get_obj_vis_accessible(ch, arg2);
      if (!container) {
	sendf(ch,"You don't have or see the %s\n",arg2);
	return;
      }
      if (!corpse_consent(ch,container)) return;
      if (!ITEM_TYPE(container,ITEM_CONTAINER) &&
	  !ITEM_TYPE(container,ITEM_SPELL_POUCH)) {
	sendf(ch,"The %s is not a container.\n",arg2);
	return;
      }
      if (getall(arg1,newarg)!=0) {
	num = -1;
	strcpy(arg1,newarg);
      } else if ((p = getabunch(arg1,newarg))!=0) {
	num = p;                     
	strcpy(arg1,newarg);
      } else {
	num = 1;
      }
      while (num != 0) {
	obj = get_obj_in_list_vis(ch,arg1,container->contains);
	if (!obj) {
	  if (found) return;
	  sendf(ch,"You don't see that in the %s\n",arg2);
	  return;
	}
	if (CheckForInsideTrap(ch, container))
	  return;
	if (can_carry(ch,ch,obj)) {
	  phys_get(ch,obj,container);
	  found = TRUE;
	} else return;
	if (num>0) num--;
      }
    } break;
  }
}

int CAN_DROP(struct char_data *ch, struct obj_data *obj)
{
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) return(0);
  if (obj->virtual==25) return(0);
  return(1);
}

int phys_drop(struct char_data *ch, struct obj_data *obj)
{
  if (!CAN_DROP(ch,obj)) {
    sendf(ch, "You can't seem to get rid of the %s\n",
      obj->short_description);
    return(0);
  }
  sendf(ch, "You drop %s.\n", obj->short_description);
  act("$n drops $p.", 1, ch, obj, 0, TO_ROOM);
  obj_from_char(obj);
  if (!IS_IMMORTAL(ch))
    obj->obj_flags.timer2=DROP_TICKS;
  obj_to_room(obj,ch->in_room);
  return(1);
}

void do_drop(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct obj_data *obj;
  struct obj_data *next_obj;
  int found=0;
  int num;

  num=1;
  only_argument(argument,arg);
  if (strchr(arg,'*')) {
    num=atoi(arg);
    if (num==0 && !strncmp(arg,"all",3)) num=32000;
    if (num<=0) {
      sendf(ch,"Try a positive number before the *!\n");
      return;
    }
    strcpy(buf,strchr(arg,'*')+1);
    strcpy(arg,buf);
  }
  if (!strchr(arg,'.')) {
    if (atoi(arg)) {
      num=atoi(arg);
      if (num<=0) {
	sendf(ch,"Try a positive number!\n");
	return;
      }
      if (!strchr(arg,' ')) {
	sendf(ch,"Drop %d what's?\n",num);
	return;
      }
      strcpy(buf,strchr(arg,' ')+1);
      strcpy(arg,buf);
    }
  }

  if (!str_cmp(arg,"coin") || !str_cmp(arg,"coins")) {
    if (num>GET_GOLD(ch)) {
      sendf(ch,"Ha! And where will that come from?\n");
      return;
    }
    if (num<25) {
      sendf(ch,"You can't drop less than 25 coins.\n");
      return;
    }
    if (GET_LEVEL(ch)>MAX_MORT && GET_LEVEL(ch)<SILLYLORD) {
      vlog(LOG_URGENT,"%s just tried to drop %d coins.",GET_NAME(ch),num);
      return;
    }
    sendf(ch,"Ok.\n");
    act("$n drops some gold.", FALSE, ch, 0, 0, TO_ROOM);
    obj = create_money(num);
    obj_to_room(obj,ch->in_room);
    GET_GOLD(ch)-=num;
    return;
  }

  if (!*arg) {
    sendf(ch,"What would you like to drop?\n");
    return;
  }

  if (!str_cmp(arg,"all")) {
    if (!ch->carrying) {
      sendf(ch,"You aren't carrying anything!\n");
      return;
    }
    for (obj = ch->carrying; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if (CAN_DROP(ch,obj)) {
	sendf(ch,"%s: Ok\n",
		CAN_SEE_OBJ(ch,obj)?obj->short_description:"something");
	act("$n drops $p.", 1, ch, obj, 0, TO_ROOM);
	obj_from_char(obj);
	if (!IS_IMMORTAL(ch))
	  obj->obj_flags.timer2=DROP_TICKS;
	obj_to_room(obj,ch->in_room);
      } else
	sendf(ch,"%s: You can't seem to drop this.\n",
		 obj->short_description);
    }
    return;
  } /* drop all */

  while (num != 0) {
    obj = get_obj_in_list_vis(ch, arg, ch->carrying);
    if (obj) found=1; /* for drop 10*object */
    if (!obj) {
      if (!found) sendf(ch,"You don't seem to have a %s\n",arg);
      return;
    }
    phys_drop(ch, obj);
    num--;
  }
}

void do_put(struct char_data *ch, char *argument, int cmd)
{
  char buffer[256];
  char arg1[128];
  char arg2[128];
  struct obj_data *obj_object;
  struct obj_data *sub_object;
  struct char_data *tmp_char;
  int bits;
  char newarg[100];
  int num, p;
  
  argument_interpreter(argument, arg1, arg2);
  
  if (!*arg1) {
    sendf(ch,"Put what in what?\n");
    return;
  }
  if (!*arg2) {
    sendf(ch,"Put %s in what?\n",arg1);
    return;
  }
  if (getall(arg1,newarg)!=0) {
    num = -1;
    strcpy(arg1,newarg);
  } else if ((p = getabunch(arg1,newarg))!=0) {
    num = p;                     
    strcpy(arg1,newarg);
  } else {
    num = 1;  
  }
  
  if (!strcmp(arg1,"all")) {
    send_to_char("sorry, you can't do that (yet)\n",ch);
    return;
  }
  while (num != 0) {
    bits = generic_find(arg1, FIND_OBJ_INV, ch, &tmp_char, &obj_object);
    if (!obj_object) {
      if ((num>0) || (num==-1)) {
	sendf(ch,"You don't have the %s.\n",arg1);
	return;
      }
      return;
    }
    bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &tmp_char, &sub_object);
    if (!sub_object) {
      sendf(ch,"You don't have the %s.\n",arg2);
      return;
    }
    if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER ||
	GET_ITEM_TYPE(sub_object) == ITEM_SPELL_POUCH) {
      if (IS_SET(obj_object->obj_flags.extra_flags, ITEM_NODROP)) {
	send_to_char("You can not put a cursed item in a container.\n", ch);
	return;
      }
      if (obj_object->virtual == 25) {
       send_to_char("You cannot put an insurance token in a container.\n",ch);
       return;
      }
      if (GET_ITEM_TYPE(sub_object) == ITEM_SPELL_POUCH) {
	if (!is_spell_component(ch,obj_object->virtual)) {
	  sendf(ch,"That isn't a spell component!\n");
	  return;
	}
      }
      if (!IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
	if (obj_object == sub_object) {
	  send_to_char("You attempt to fold it into itself, but fail.\n", ch);
	  return;
	}
/* new check [should eventually redo the value stuff] */
        if(sub_object->obj_flags.value[0] > GET_OBJ_VOLUME(obj_object)){
/*	if (((sub_object->obj_flags.weight) + 
	     (obj_object->obj_flags.weight)) <
	    (sub_object->obj_flags.value[0])) { */
	  act("You put $p in $P",TRUE, ch, obj_object, sub_object, TO_CHAR);
	  if (bits==FIND_OBJ_INV) {
	    obj_from_char(obj_object);
/* make up for above line - No! This is taken care of by obj_to_obj now */
/* as it should be!                                               - SLB */
/*	    IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(obj_object); */
	    obj_to_obj(obj_object, sub_object);
	  } else {
	    obj_from_room(obj_object);
	    obj_to_obj(obj_object, sub_object);
	  }
	  
	  act("$n puts $p in $P",TRUE, ch, obj_object, sub_object, TO_ROOM);
	  num--;
	} else {
	  send_to_char("It won't fit.\n", ch);
	  num = 0;
	}
      } else {
	send_to_char("It seems to be closed.\n", ch);
	num = 0;
      }
    } else {
      sprintf(buffer,"%s is not a container.\n", sub_object->short_description);
      send_to_char(buffer, ch);
      num = 0;
    }
  }
}




void do_give(struct char_data *ch, char *argument, int cmd)
{
  char obj_name[MAX_INPUT_LENGTH], vict_name[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH], newarg[MAX_INPUT_LENGTH];
  int amount, num, p;
  int found, fail, rate = 0;
  struct char_data *vict;
  struct obj_data *obj;
  
  argument=one_argument(argument,obj_name);
  if(is_number(obj_name))	{
    amount = atoi(obj_name);
    argument=one_argument(argument, arg);
    if (str_cmp("coins",arg) && str_cmp("coin",arg))      	{
      send_to_char("Sorry, you can't do that (yet)...\n",ch);
      return;
    }
    if(amount<0)	{
      send_to_char("Sorry, you can't do that!\n",ch);
      return;
    }
    if((GET_GOLD(ch)<amount) && ((IS_NPC(ch) || (GetMaxLevel(ch)<SILLYLORD)))) {
      send_to_char("You haven't got that many coins!\n",ch);
      return;
    }

    argument=one_argument(argument, vict_name);

    if(!*vict_name)	{
      send_to_char("To who?\n",ch);
      return;
    }

    if (!(vict = get_char_room_vis(ch, vict_name)))	{
      send_to_char("To who?\n",ch);
      return;
    }

    send_to_char("Ok.\n",ch);
    sprintf(buf,"%s gives you %d gold coins.\n",PERS(ch,vict),amount);
    send_to_char(buf,vict);
    act("$n gives some gold to $N.", 1, ch, 0, vict, TO_NOTVICT);
    if (IS_NPC(ch) || (GetMaxLevel(ch) < DEMIGOD))
      GET_GOLD(ch)-=amount;
    GET_GOLD(vict)+=amount;
    if (amount > 2000000) /* hmmm */ {
      vlog(LOG_MISC,"%s gave %d coins to %s",
		 GET_NAME(ch), amount, GET_NAME(vict));
    }
      
    return;
  }
  
  argument=one_argument(argument, vict_name);
  
  
  if (!*obj_name || !*vict_name)	{
    send_to_char("Give what to who?\n", ch);
    return;
  }
  /* &&&& */
  if (getall(obj_name,newarg)!=0) {
    num = -1;
    strcpy(obj_name,newarg);
  } else if ((p = getabunch(obj_name,newarg))!=0) {
    num = p;                     
    strcpy(obj_name,newarg);
  } else {
    num = 1;  
  }
  
  found=fail=FALSE;
  vict = get_char_room_vis(ch,vict_name);
  if (!vict) {
    send_to_char("No one by that name around here.\n", ch);
    return;
  }
  if (vict==ch) {
    sendf(ch,"You can't give to yourself!\n");
    return;
  }
  while (num != 0) {
    obj = get_obj_in_list_vis(ch, obj_name, ch->carrying);
    if (!obj) {
      if (found) return;
      sendf(ch,"You do not seem to have anything like that.\n");
      return;
    }
    if (!can_carry(ch,vict,obj)) return;
    else found=TRUE;

    if(IS_NPC(vict) && AWAKE(vict) && IsHumanoid(vict) &&
        (IS_AFFECTED(vict, AFF_CHARM) ? IS_NPC(vict->master) : 1)){
      switch(obj->obj_flags.type_flag){
        case ITEM_ARMOR:
          rate = rate_armor(obj, vict);
          break;
        case ITEM_WEAPON:
          rate = rate_weapon(obj, vict);
          break;
      }
      if(!rate || !CAN_SEE_OBJ(vict, obj)){
        act("$N refuses your offer politely.", FALSE, ch, obj, vict,
          TO_CHAR);
        act("You refuse $n's offer of $p.", FALSE, ch, obj, vict, TO_VICT);
        return;
      }
    }
    obj_from_char(obj);
    obj_to_char(obj, vict);
    act("$n gives $p to $N.", 1, ch, obj, vict, TO_NOTVICT);
    act("$n gives you $p.", 0, ch, obj, vict, TO_VICT);
    act("You give $p to $N", 0, ch, obj, vict, TO_CHAR);
    
    if (num > 0) num--;
  }
}
