/* ************************************************************************
 *  file: act.informative.c , Implementation of commands.  Part of DIKUMUD *
 *  Usage : Informative commands.                                          *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#ifdef USE_MALLOP
#include <malloc.h>
#endif

#include <fcns.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <spells.h>
#include <limits.h>
#include <race.h>
#include <trap.h>
#include <hash.h>
#include <externs.h>

/* Procedures related to 'look' */

#if 0
static void argument_split_2(char *argument, char *first_arg, char *second_arg)
{
  int look_at, found, begin;
  found = begin = 0;

  /* Find first non blank */
  for ( ;*(argument + begin ) == ' ' ; begin++);

  /* Find length of first word */
  for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

    /* Make all letters lower case, AND copy them to first_arg */
    *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
  *(first_arg + look_at) = '\0';
  begin += look_at;

  /* Find first non blank */
  for ( ;*(argument + begin ) == ' ' ; begin++);

  /* Find length of second word */
  for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

    /* Make all letters lower case, AND copy them to second_arg */
    *(second_arg + look_at) = LOWER(*(argument + begin + look_at));
  *(second_arg + look_at)='\0';
  begin += look_at;
}

#endif

static char *find_ex_description(char *word, struct extra_descr_data *list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
    if (isname(word,i->keyword))
      return(i->description);

  return(0);
}


static char *obj_condition(struct char_data *ch, struct obj_data *object)
{
  static char buffer[MAX_STRING_LENGTH];

  *buffer='\0';
  if (GET_ITEM_TYPE(object)==ITEM_COMPONENT)
    sprintf(buffer,"(%d uses)",object->obj_flags.value[0]);
  if (IS_OBJ_STAT(object,ITEM_INVISIBLE)) {
    strcat(buffer,"(invisible)");
  }
  if (IS_OBJ_STAT(object,ITEM_ANTI_GOOD) &&
      IS_AFFECTED(ch,AFF_DETECT_EVIL)) {
    strcat(buffer,"..It glows red!");
  }
  if (IS_OBJ_STAT(object,ITEM_MAGIC) && DETECT_MAGIC(ch)) {
    strcat(buffer,"..It glows blue!");
  }
  if (IS_OBJ_STAT(object,ITEM_GLOW)) {
    strcat(buffer,"..It has a soft glowing aura!");
  }
  if (IS_OBJ_STAT(object,ITEM_HUM)) {
    strcat(buffer,"..It emits a faint humming sound!");
  }
  if (object->obj_flags.type_flag == ITEM_ARMOR) {
    if (object->obj_flags.value[0] <
	(object->obj_flags.value[1] / 4)) {
      strcat(buffer, "..It is falling apart.");
    } else if (object->obj_flags.value[0] <
	       (object->obj_flags.value[1] / 3)) {
      strcat(buffer, "..It is need of much repair.");
    } else if (object->obj_flags.value[0] <
	       (object->obj_flags.value[1] / 2)) {
      strcat(buffer, "..It is in fair condition.");
    } else if  (object->obj_flags.value[0] <
		object->obj_flags.value[1]) {
      strcat(buffer, "..It is in good condition.");
    } else {
      strcat(buffer, "..It is in excellent condition.");
    }
  }
  if (IS_SET (object->obj_flags.wear_flags, ITEM_DAMAGED)) 
      strcat (buffer, "..It is damaged beyond use.");
  return(buffer);
}

static void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode)
{
  char buffer[MAX_STRING_LENGTH];

  *buffer='\0';
  if ((mode == 0) && object->description)
    strcpy(buffer, object->description);
  else if (object->short_description && ((mode == 1) || (mode == 2) ||
					 (mode==3) || (mode == 4)))
    strcpy(buffer,object->short_description);
  else if (mode == 5) {
    if (object->obj_flags.type_flag == ITEM_NOTE)  	{
      if (object->action_description)	 {
	strcpy(buffer, "There is something written upon it:\n\n");
	strcat(buffer, object->action_description);
      }  else {
	act("It's blank.", FALSE, ch,0,0,TO_CHAR);
      }
    } else if((object->obj_flags.type_flag != ITEM_DRINKCON)) {
      strcpy(buffer,"You see nothing special..");
    }  else  { /* ITEM_TYPE == ITEM_DRINKCON */
      strcpy(buffer, "It looks like a drink container.");
    }
  }

  if (mode != 3) {
    strcat(buffer,obj_condition(ch,object));
  }
  strcat(buffer, "\n");
  page_string(ch->desc, buffer, 1);

}

static void show_mult_obj_to_char(struct obj_data *object, struct char_data *ch, int mode, int num)
{
  char buffer[MAX_STRING_LENGTH];
  char tmp[10];

  *buffer='\0';
  *tmp='\0';
  if ((mode == 0) && object->description)
    strcpy(buffer,object->description);
  else 	if (object->short_description && ((mode == 1) || (mode == 2) || 
					  (mode==3) || (mode == 4)))
    strcpy(buffer,object->short_description);
  else if (mode == 5) {
    if (object->obj_flags.type_flag == ITEM_NOTE)  	{
      if (object->action_description)	 {
	strcpy(buffer, "There is something written upon it:\n\n");
	strcat(buffer, object->action_description);
	/*page_string(ch->desc, buffer, 1);*/
      }  else
	act("It's blank.", FALSE, ch,0,0,TO_CHAR);
      return;
    } else if((object->obj_flags.type_flag != ITEM_DRINKCON)) {
      strcpy(buffer,"You see nothing special..");
    }  else  { /* ITEM_TYPE == ITEM_DRINKCON */
      strcpy(buffer, "It looks like a drink container.");
    }
  }

  if (mode != 3) {
    strcat(buffer,obj_condition(ch,object));
  }

  if (num>1) {
    sprintf(tmp,"[%d]", num);
    strcat(buffer, tmp);
  }
  strcat(buffer, "\n");
  page_string(ch->desc, buffer, 1);
}

static void list_obj_in_room(struct obj_data *list, struct char_data *ch)
{
  struct obj_data *i, *cond_ptr[50];
  int Inventory_Num = 1, num;
  int k, cond_top, cond_tot[50], found=FALSE;

  cond_top = 0;

  for (i=list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      if (cond_top< 50) {
	found = FALSE;
	for (k=0;(k<cond_top&& !found);k++) {
	  if (cond_top>0) {
	    if ((i->virtual == cond_ptr[k]->virtual) &&
		(i->description && cond_ptr[k]->description &&
		 !strcmp(i->description,cond_ptr[k]->description))){
	      cond_tot[k] += 1;
	      found=TRUE;
	    }
	  }
	}
	if (!found) {
	  cond_ptr[cond_top] = i;
	  cond_tot[cond_top] = 1;
	  cond_top+=1;
	}
      } else {
	if (ITEM_TYPE(i,ITEM_TRAP) || (GET_TRAP_CHARGES(i) > 0)) {
       	  num = number(1,100);
       	  if (num < get_skill(ch,SKILL_FIND_TRAP)/10)
	    show_obj_to_char(i,ch,0);
        } else {
	  show_obj_to_char(i,ch,0);
	}
      }
    }
  }

  if (cond_top) {

    for (k=0; k<cond_top; k++) {
      if (ITEM_TYPE(cond_ptr[k],ITEM_TRAP) &&
	  (GET_TRAP_CHARGES(cond_ptr[k]) > 0)) {
	num = number(1,100);
	if (num < get_skill(ch,SKILL_FIND_TRAP)/10)
	  if (cond_tot[k] > 1) {
	    sendf(ch,"[%2d] ",Inventory_Num++);
	    show_mult_obj_to_char(cond_ptr[k],ch,0,cond_tot[k]);
	  } else {
	    show_obj_to_char(cond_ptr[k],ch,0);
	  }
      } else {
	if (cond_tot[k] > 1) {
	  sendf(ch,"[%2d] ",Inventory_Num++);
	  show_mult_obj_to_char(cond_ptr[k],ch,0,cond_tot[k]);
	} else {
	  show_obj_to_char(cond_ptr[k],ch,0);
	}
      }
    }
  }
}

static void list_obj_on_char(struct obj_data *list, struct char_data *ch)
{
  struct obj_data *i, *cond_ptr[50];
  int k, cond_top, cond_tot[50], found=FALSE;

  int Num_Inventory = 1;
  cond_top = 0;

  for (i=list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      if (cond_top< 50) {
	found = FALSE;
        for (k=0;(k<cond_top&& !found);k++) {
          if (cond_top>0) {
            if ((i->virtual == cond_ptr[k]->virtual) &&
	       (i->short_description && cond_ptr[k]->short_description &&
	       (!strcmp(i->short_description,cond_ptr[k]->short_description)))){
	      cond_tot[k] += 1;
	      found=TRUE;
	    }
	  }
	}
	if (!found) {
	  cond_ptr[cond_top] = i;
	  cond_tot[cond_top] = 1;
	  cond_top+=1;
	}
      } else {
	show_obj_to_char(i,ch,2);
      }
    }
  }

  if (cond_top) {
    for (k=0; k<cond_top; k++) {
      sendf(ch,"[%2d] ",Num_Inventory++);
      if (cond_tot[k] > 1) {
	Num_Inventory += cond_tot[k] - 1;
	show_mult_obj_to_char(cond_ptr[k],ch,2,cond_tot[k]);
      } else {
	show_obj_to_char(cond_ptr[k],ch,2);
      }
    }
  }
}

static void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode, bool show)
{
  struct obj_data *i;
  bool found;

  found = FALSE;
  for ( i = list ; i ; i = i->next_content ) {
    if (CAN_SEE_OBJ(ch,i)) {
      sendf(ch,"%s%s\n",i->short_description,obj_condition(ch,i));
      found = TRUE;
    }
  }
  if ((! found) && (show)) send_to_char("Nothing\n", ch);
}



static void show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
  char buffer[MAX_STRING_LENGTH];
  char *ig;
  int j, found, percent;
  struct obj_data *tmp_obj;

  if (mode == 0) {
    if (!CAN_SEE(ch,i)) {
      if (IS_AFFECTED(ch, AFF_SENSE_LIFE) && !IS_IMMORTAL(i))
        send_to_char("You sense a hidden life form in the room.\n", ch);
      return;
    }

    if (!(i->player.long_descr)||(GET_POS(i) != i->specials.default_pos)){
      /* A player char or a mobile without long descr, or not in default pos. */
      if (!IS_NPC(i)) {
	strcpy(buffer,GET_NAME(i));
	strcat(buffer," ");
	if (GET_TITLE(i))
	  strcat(buffer,GET_TITLE(i));
      } else {
	strcpy(buffer, i->player.short_descr);
	ig=CAP(buffer);
      }

      switch(GET_POS(i)) {
      case POSITION_STUNNED  :
	strcat(buffer," is lying here, stunned."); break;
      case POSITION_INCAP    :
	strcat(buffer," is lying here, incapacitated."); break;
      case POSITION_MORTALLYW:
	strcat(buffer," is lying here, mortally wounded."); break;
      case POSITION_DEAD     :
	strcat(buffer," is lying here, dead."); break;
      case POSITION_STANDING :
	strcat(buffer," is standing here."); break;
      case POSITION_SITTING  :
	strcat(buffer," is sitting here.");  break;
      case POSITION_PRAYING:
	strcat(buffer, " is praying here."); break;
      case POSITION_RESTING  :
	strcat(buffer," is resting here.");  break;
      case POSITION_SLEEPING :
	strcat(buffer," is sleeping here."); break;
      case POSITION_FIGHTING :
	if (i->specials.fighting) {
	  strcat(buffer," is here, fighting ");
	  if (i->specials.fighting == ch)
	    strcat(buffer,"you!");
	  else {
	    if (i->in_room == i->specials.fighting->in_room)
	      if (IS_NPC(i->specials.fighting))
		strcat(buffer, i->specials.fighting->player.short_descr);
	      else
		strcat(buffer, GET_NAME(i->specials.fighting));
	    else
	      strcat(buffer, "someone who has already left.");
	  }
	} else /* NIL fighting pointer */
	  strcat(buffer," is here struggling with thin air.");
	break;
	default : strcat(buffer," is floating here."); break;
      }
    }
    else {
      strcpy(buffer, i->player.long_descr);
      if (buffer[strlen(buffer) - 1] == '\n')
         buffer[strlen(buffer) - 1] = '\0';
    }

    if (!IS_NPC(i))
      if (!i->desc) strcat(buffer," [link dead]");
    if (IS_CITIZEN(i))
      strcat(buffer," (citizen)");
    if (IS_AFFECTED(i,AFF_INVISIBLE))
      strcat(buffer," (invis)");
    if ( IS_AFFECTED(i,AFF_CHARM))
      strcat(buffer," (charmed)");
    if (IS_AFFECTED(ch, AFF_DETECT_EVIL))
      if (IS_EVIL(i))
        strcat(buffer, " (red aura)");

    strcat(buffer,"\n");
    send_to_char(buffer, ch);

    if (IS_AFFECTED(i,AFF_SANCTUARY))
      act("$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
    if (affected_by_spell(i, SPELL_FIRESHIELD))
      act("$n glows with a red light!", FALSE, i, 0, ch, TO_VICT);
    if (IS_AFFECTED(i,AFF_INVULNERABLE))
      act("$n glows with a green light!", FALSE, i, 0, ch, TO_VICT);
  } else if (mode == 1) {

    if (i->player.description)
      send_to_char(i->player.description, ch);
    else {
      act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
    }

    /* Show a character to another */

    if (GET_MAX_HIT(i) > 0)
      percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
    else
      percent = -1; /* How could MAX_HIT be < 1?? */

    if (IS_NPC(i))
      strcpy(buffer, i->player.short_descr);
    else
      strcpy(buffer, GET_NAME(i));

    if (percent >= 100)
      strcat(buffer, " is in an excellent condition.\n");
    else if (percent >= 90)
      strcat(buffer, " has a few scratches.\n");
    else if (percent >= 75)
      strcat(buffer, " has some small wounds and bruises.\n");
    else if (percent >= 50)
      strcat(buffer, " has quite a few wounds.\n");
    else if (percent >= 30)
      strcat(buffer, " has some big nasty wounds and scratches.\n");
    else if (percent >= 15)
      strcat(buffer, " looks pretty hurt.\n");
    else if (percent >= 0)
      strcat(buffer, " is in an awful condition.\n");
    else
      strcat(buffer, " is bleeding awfully from big wounds.\n");

    send_to_char(buffer, ch);

    found = FALSE;
    for (j=0; j< MAX_WEAR; j++) {
      if (i->equipment[j]) {
	if (CAN_SEE_OBJ(ch,i->equipment[j])) {
	  found = TRUE;
	}
      }
    }
    if (found) {
      act("\n$n is using:", FALSE, i, 0, ch, TO_VICT);
      for (j=0; j< MAX_WEAR; j++) {
	if (i->equipment[j]) {
	  if (CAN_SEE_OBJ(ch,i->equipment[j])) {
	    send_to_char(where[j],ch);
	    show_obj_to_char(i->equipment[j],ch,1);
	  }
	}
      }
    }
    if ((IsClass(ch,BARD) || IsClass(ch, THIEF)) && (ch != i) &&
	(!IS_IMMORTAL(ch))){
      found = FALSE;
      send_to_char("\nYou attempt to peek at the inventory:\n", ch);
      for(tmp_obj = i->carrying; tmp_obj;
	  tmp_obj = tmp_obj->next_content) {
	if (CAN_SEE_OBJ(ch, tmp_obj) &&
	    (number(0,MAX_MORT) < GetMaxLevel(ch))) {
	  show_obj_to_char(tmp_obj, ch, 1);
	  found = TRUE;
	}
      }
      if (!found)
	send_to_char("You can't see anything.\n", ch);
    } else if (IS_IMMORTAL(ch)) {
      send_to_char("Inventory:\n",ch);
      for(tmp_obj = i->carrying; tmp_obj;
	  tmp_obj = tmp_obj->next_content) {
	show_obj_to_char(tmp_obj, ch, 1);
	found = TRUE;
      }
      if (!found) {
	send_to_char("Nothing\n",ch);
      }
    }

  } else if (mode == 2) {

    /* Lists inventory */
    act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
    list_obj_on_char(i->carrying,ch);
  }
}


static void show_mult_char_to_char(struct char_data *i, struct char_data *ch, int mode, int num)
{
  char *ig,buffer[MAX_STRING_LENGTH];
  char tmp[10];
  int j, found, percent;
  struct obj_data *tmp_obj;

  if (mode == 0) {
    if (!CAN_SEE(ch,i)) {
      if (IS_AFFECTED(ch, AFF_SENSE_LIFE)  && !IS_IMMORTAL(i))
	if (num==1)
	  send_to_char("You sense a hidden life form in the room.\n", ch);
	else
	  send_to_char("You sense hidden life forms in the room.\n", ch);
      return;
    }

    if (!(i->player.long_descr)||(GET_POS(i) != i->specials.default_pos)){
      /* A player char or a mobile without long descr, or not in default pos. */
      if (!IS_NPC(i)) {
	strcpy(buffer,GET_NAME(i));
	strcat(buffer," ");
	if (GET_TITLE(i))
	  strcat(buffer,GET_TITLE(i));
      } else {
	strcpy(buffer, i->player.short_descr);
	ig=CAP(buffer);
      }

      if ( IS_AFFECTED(i,AFF_INVISIBLE))
	strcat(buffer," (invisible)");
      if ( IS_AFFECTED(i,AFF_CHARM))
	strcat(buffer," (pet)");

      switch(GET_POS(i)) {
      case POSITION_STUNNED  :
	strcat(buffer," is lying here, stunned."); break;
      case POSITION_INCAP    :
	strcat(buffer," is lying here, incapacitated."); break;
      case POSITION_MORTALLYW:
	strcat(buffer," is lying here, mortally wounded."); break;
      case POSITION_DEAD     :
	strcat(buffer," is lying here, dead."); break;
      case POSITION_STANDING :
	strcat(buffer," is standing here."); break;
      case POSITION_SITTING  :
	strcat(buffer," is sitting here.");  break;
      case POSITION_RESTING  :
	strcat(buffer," is resting here.");  break;
      case POSITION_SLEEPING :
	strcat(buffer," is sleeping here."); break;
      case POSITION_FIGHTING :
	if (i->specials.fighting) {

	  strcat(buffer," is here, fighting ");
	  if (i->specials.fighting == ch)
	    strcat(buffer," YOU!");
	  else {
	    if (i->in_room == i->specials.fighting->in_room)
	      if (IS_NPC(i->specials.fighting))
		strcat(buffer, i->specials.fighting->player.short_descr);
	      else
		strcat(buffer, GET_NAME(i->specials.fighting));
	    else
	      strcat(buffer, "someone who has already left.");
	  }
	} else /* NIL fighting pointer */
	  strcat(buffer," is here struggling with thin air.");
	break;
	default : strcat(buffer," is floating here."); break;
      }
      if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
	if (IS_EVIL(i))
	  strcat(buffer, " (Red Aura)");
      }

      send_to_char(buffer, ch);
      if (num > 1)
	sendf(ch," [%d]", num);
      strcat(buffer,"\n");
    } else {  /* npc with long */

      if (IS_AFFECTED(i,AFF_INVISIBLE))
	strcpy(buffer,"*");
      else
	*buffer = '\0';

      if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
	if (IS_EVIL(i))
	  strcat(buffer, " (Red Aura)");
      }

      strcat(buffer, i->player.long_descr);

      if (num > 1) {
	while (isspace(buffer[strlen(buffer)-1]))
	  buffer[strlen(buffer)-1] ='\0';
	sprintf(tmp," [%d]\n", num);
	strcat(buffer, tmp);
      }

      send_to_char(buffer, ch);
    }

    if (IS_AFFECTED(i,AFF_SANCTUARY))
      act("$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
    if (affected_by_spell(i, SPELL_FIRESHIELD))
      act("$n glows with a red light!", FALSE, i, 0, ch, TO_VICT);

  } else if (mode == 1) {

    if (i->player.description)
      send_to_char(i->player.description, ch);
    else {
      act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
    }

    /* Show a character to another */

    if (GET_MAX_HIT(i) > 0)
      percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
    else
      percent = -1; /* How could MAX_HIT be < 1?? */

    if (IS_NPC(i))
      strcpy(buffer, i->player.short_descr);
    else
      strcpy(buffer, GET_NAME(i));

    if (percent >= 100)
      strcat(buffer, " is in an excellent condition.\n");
    else if (percent >= 90)
      strcat(buffer, " has a few scratches.\n");
    else if (percent >= 75)
      strcat(buffer, " has some small wounds and bruises.\n");
    else if (percent >= 50)
      strcat(buffer, " has quite a few wounds.\n");
    else if (percent >= 30)
      strcat(buffer, " has some big nasty wounds and scratches.\n");
    else if (percent >= 15)
      strcat(buffer, " looks pretty hurt.\n");
    else if (percent >= 0)
      strcat(buffer, " is in an awful condition.\n");
    else
      strcat(buffer, " is bleeding awfully from big wounds.\n");

    send_to_char(buffer, ch);

    found = FALSE;
    for (j=0; j< MAX_WEAR; j++) {
      if (i->equipment[j]) {
	if (CAN_SEE_OBJ(ch,i->equipment[j])) {
	  found = TRUE;
	}
      }
    }
    if (found) {
      act("\n$n is using:", FALSE, i, 0, ch, TO_VICT);
      for (j=0; j< MAX_WEAR; j++) {
	if (i->equipment[j]) {
	  if (CAN_SEE_OBJ(ch,i->equipment[j])) {
	    send_to_char(where[j],ch);
	    show_obj_to_char(i->equipment[j],ch,1);
	  }
	}
      }
    }
    if ((IsClass(ch, THIEF) || IsClass(ch,BARD)) && (ch != i)) {
      found = FALSE;
      send_to_char("\nYou attempt to peek at the inventory:\n", ch);
      for(tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
	if (CAN_SEE_OBJ(ch,tmp_obj)&&(number(0,MAX_MORT) < GetMaxLevel(ch))) {
	  show_obj_to_char(tmp_obj, ch, 1);
	  found = TRUE;
	}
      }
      if (!found)
	send_to_char("You can't see anything.\n", ch);
    }

  } else if (mode == 2) {

    /* Lists inventory */
    act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
    list_obj_on_char(i->carrying,ch);
  }
}


static void list_char_in_room(struct char_data *list, struct char_data *ch)
{
  struct char_data *i, *cond_ptr[50];
  int k, cond_top, cond_tot[50], found=FALSE;

  cond_top = 0;

  for (i=list; i; i = i->next_in_room) {
    if (ch!=i && (CAN_SEE(ch,i) || IS_AFFECTED(ch,AFF_SENSE_LIFE))) {
      if (cond_top< 50) {
	found = FALSE;
	if (IS_NPC(i)) {
	  for (k=0;(k<cond_top&& !found);k++) {
	    if (cond_top>0) {
	      if (i->virtual == cond_ptr[k]->virtual &&
		  (GET_POS(i) == GET_POS(cond_ptr[k])) &&
		  (i->specials.affected_by==cond_ptr[k]->specials.affected_by) &&
		  (i->specials.fighting == cond_ptr[k]->specials.fighting) &&
		  (i->player.short_descr && cond_ptr[k]->player.short_descr &&
		   0==strcmp(i->player.short_descr,cond_ptr[k]->player.short_descr))) {
		cond_tot[k] += 1;
		found=TRUE;
	      }
	    }
	  }
	}
	if (!found) {
	  cond_ptr[cond_top] = i;
	  cond_tot[cond_top] = 1;
	  cond_top+=1;
	}
      } else {
	show_char_to_char(i,ch,0);
      }
    }
  }

  if (cond_top) {
    for (k=0; k<cond_top; k++) {
      if (cond_tot[k] > 1) {
	show_mult_char_to_char(cond_ptr[k],ch,0,cond_tot[k]);
      } else {
	show_char_to_char(cond_ptr[k],ch,0);
      }
    }
  }
}


#if 0
static void list_char_to_char(struct char_data *list, struct char_data *ch, int mode)
{
  struct char_data *i;

  for (i = list; i ; i = i->next_in_room) {
    if (ch!=i && (IS_AFFECTED(ch,AFF_SENSE_LIFE) || CAN_SEE(ch,i)))
      show_char_to_char(i,ch,0);
  }
}

#endif


void do_look(struct char_data *ch, char *argument, int cmd)
{
  char buffer[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int keyword_no, res;
  int j, bits, temp;
  bool found;
  struct obj_data *tmp_object, *found_object;
  struct char_data *tmp_char;
  char *tmp_desc;
  static char *keywords[]= {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "in",
    "at",
    "",  /* Look at '' case */
    "room",
    "\n" };

  if (!ch->desc)
    return;

  if (GET_POS(ch) < POSITION_SLEEPING)
    send_to_char("You can't see anything but stars!\n", ch);
  else if (GET_POS(ch) == POSITION_SLEEPING)
    send_to_char("You can't see anything, you're sleeping!\n", ch);
  else if ( IS_AFFECTED(ch, AFF_BLIND) )
    send_to_char("You can't see a damn thing, you're blinded!\n", ch);
  else if  ((IS_DARK(ch->in_room)) &&
   (!IS_IMMORTAL(ch)) && (!IS_AFFECTED(ch, AFF_TRUE_SIGHT))) {
    send_to_char("It is very dark in here...\n", ch);
    if (IS_AFFECTED(ch, AFF_INFRAVISION)) {
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
    }
  } else {

    only_argument(argument, arg1);

    if (0==strn_cmp(arg1,"at",1) && isspace(arg1[2])) {
      only_argument(argument+3, arg2);
      keyword_no = 7;
    } else if (0==strn_cmp(arg1,"in",1) && isspace(arg1[2])) {
      only_argument(argument+3, arg2);
      keyword_no = 6;
    } else {
      keyword_no = search_block(arg1, keywords, FALSE);
    }

    if ((keyword_no == -1) && *arg1) {
      keyword_no = 7;
      only_argument(argument, arg2);
    }


    found = FALSE;
    tmp_object = 0;
    tmp_char	 = 0;
    tmp_desc	 = 0;

    switch(keyword_no) {
      /* look <dir> */
    case 0 :
    case 1 :
    case 2 :
    case 3 :
    case 4 :
    case 5 : {
      struct room_direction_data	*exitp;
      exitp = EXIT(ch, keyword_no);
      if (exitp) {
	if (exitp->general_description) {
	  send_to_char(exitp-> general_description, ch);
	} else {
	  send_to_char("You see nothing special.\n", ch);
	}

	if (IS_SET(exitp->exit_info, EX_CLOSED) && (exitp->keyword)) {
	   if ((strcmp(fname(exitp->keyword), "secret")) && (!IS_SET(exitp->exit_info, EX_SECRET)))
	      sendf(ch, "The %s is closed.\n", fname(exitp->keyword));
	 } else {
	   if (IS_SET(exitp->exit_info, EX_ISDOOR) && exitp->keyword)
	     sendf(ch, "The %s is open.\n", fname(exitp->keyword));
	 }
      } else {
	send_to_char("You see nothing special.\n", ch);
      }
      if (IS_AFFECTED(ch, AFF_SCRYING) || IS_IMMORTAL(ch)) {
	struct room_data	*rp;
	sendf(ch,"You look %swards.\n", dirs[keyword_no]);
	if (!exitp || !exitp->to_room) {
	  send_to_char("You see nothing special.",ch);
	  return;
	}
	rp = real_roomp(exitp->to_room);
	if (!rp) {
	  send_to_char("You see swirling chaos.\n", ch);
	} else if(exitp) {
	  sprintf(buffer, "%d look", exitp->to_room);
	  do_at(ch, buffer, 0);
	} else {
	  send_to_char("You see nothing special.\n", ch);
	}
      }
    }
      break;

      /* look 'in'	*/
    case 6: {
      if (*arg2) {
	/* Item carried */
	bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
			    FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

	if (bits) { /* Found something */
	  if (GET_ITEM_TYPE(tmp_object)== ITEM_DRINKCON) 	{
	    if (tmp_object->obj_flags.value[1] <= 0) {
	      act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
	    } else {
	      temp=((tmp_object->obj_flags.value[1]*3)/tmp_object->obj_flags.value[0]);
	      sendf(ch,"It's %sfull of a %s liquid.\n",
		      fullness[temp],color_liquid[tmp_object->obj_flags.value[2]]);
	    }
	  } else if (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER || 
		GET_ITEM_TYPE(tmp_object) == ITEM_SPELL_POUCH) {
	    if (!IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED)) {
	      send_to_char(fname(tmp_object->name), ch);
	      switch (bits) {
	      case FIND_OBJ_INV :
		send_to_char(" (carried) : \n", ch);
		break;
	      case FIND_OBJ_ROOM :
		send_to_char(" (here) : \n", ch);
		break;
	      case FIND_OBJ_EQUIP :
		send_to_char(" (used) : \n", ch);
		break;
	      }
	      list_obj_to_char(tmp_object->contains, ch, 2, TRUE);
	    } else
	      send_to_char("It is closed.\n", ch);
	  } else {
	    send_to_char("That is not a container.\n", ch);
	  }
	} else { /* wrong argument */
	  send_to_char("You do not see that item here.\n", ch);
	}
      } else { /* no argument */
	send_to_char("Look in what?!\n", ch);
      }
    }
      break;

      /* look 'at'	*/
    case 7 : {
      if (*arg2) {
	bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
			    FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);
	if (tmp_char) {
	  show_char_to_char(tmp_char, ch, 1);
	  if (ch != tmp_char) {
	    act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
	    act("$n looks at $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
	  }
	  return;
	}
	/*
	  Search for Extra Descriptions in room and items
	  */

	/* Extra description in room?? */
	if (!found) {
	  tmp_desc = find_ex_description(arg2,
					 real_roomp(ch->in_room)->ex_description);
	  if (tmp_desc) {
	    page_string(ch->desc, tmp_desc, 0);
	    return;
	  }
	}

	/* extra descriptions in items */

	/* Equipment Used */
	if (!found) {
	  for (j = 0; j< MAX_WEAR && !found; j++) {
	    if (ch->equipment[j]) {
	      if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
		tmp_desc = find_ex_description(arg2,
					       ch->equipment[j]->ex_description);
		if (tmp_desc) {
		  page_string(ch->desc, tmp_desc, 1);
		  found = TRUE;
		}
	      }
	    }
	  }
	}
	/* In inventory */
	if (!found) {
	  for(tmp_object = ch->carrying;
	      tmp_object && !found;
	      tmp_object = tmp_object->next_content) {
	    if CAN_SEE_OBJ(ch, tmp_object) {
	      tmp_desc = find_ex_description(arg2,
					     tmp_object->ex_description);
	      if (tmp_desc) {
		page_string(ch->desc, tmp_desc, 1);
		found = TRUE;
	      }
	    }
	  }
	}
	/* Object In room */

	if (!found) {
	  for(tmp_object = real_roomp(ch->in_room)->contents;
	      tmp_object && !found;
	      tmp_object = tmp_object->next_content) {
	    if CAN_SEE_OBJ(ch, tmp_object) {
	      tmp_desc = find_ex_description(arg2,
					     tmp_object->ex_description);
	      if (tmp_desc) {
		page_string(ch->desc, tmp_desc, 1);
		found = TRUE;
	      }
	    }
	  }
	}
	/* wrong argument */
	if (bits) { /* If an object was found */
	  if (!found)
	    show_obj_to_char(found_object, ch, 5);
	  /* Show no-description */
	  else
	    show_obj_to_char(found_object, ch, 6);
	  /* Find hum, glow etc */
	} else if (!found) {
	  send_to_char("You do not see that here.\n", ch);
	}
      } else {
	/* no argument */
	send_to_char("Look at what?\n", ch);
      }
    }
      break;

      /* look ''		*/
    case 8 : {
      send_to_char(real_roomp(ch->in_room)->name, ch);
      send_to_char("\n", ch);

      if (!IS_SET(ch->specials.act, PLR_BRIEF))
	send_to_char(real_roomp(ch->in_room)->description, ch);

      if (!IS_SET(ch->specials.act, PLR_NOEXITS) &&
	  !IS_SET(real_roomp(ch->in_room)->room_flags, NOSHOWEXIT)) {
	struct room_data *rp;
	int i,count;
	rp=real_roomp(ch->in_room);
	sendf(ch,"Exits:");
        for (count=i=0; i<6; i++) 
	  if (rp->dir_option[i]) {
	    if (rp->dir_option[i]->to_room <= 0) continue;
	    if (IS_SET(rp->dir_option[i]->exit_info, EX_CLOSED)) continue;
	    sendf(ch,"%s %s",count++?",":"",dirs[i]);
	  }
	if (!count) sendf(ch," None.");
	sendf(ch,"\n");
      }

      if (!IS_NPC(ch)) {
	if (IS_SET(ch->specials.act, PLR_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      CLEAR_BIT(ch->specials.act, PLR_HUNTING);
	    }
	  } else {
	    ch->hunt_dist = 0;
	    CLEAR_BIT(ch->specials.act, PLR_HUNTING);
	  }
	}
      } else {
	if (IS_SET(ch->specials.act, ACT_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      CLEAR_BIT(ch->specials.act, ACT_HUNTING);
	    }
	  } else {
	    ch->hunt_dist = 0;
	    CLEAR_BIT(ch->specials.act, ACT_HUNTING);
	  }
	}
      }

      list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
      list_char_in_room(real_roomp(ch->in_room)->people, ch);

    }
      break;

      /* wrong arg	*/
    case -1 :
      send_to_char("Sorry, I didn't understand that!\n", ch);
      break;

      /* look 'room' */
    case 9 : {
	char buf[MAX_STRING_LENGTH];
	struct room_data *rm;

      rm=real_roomp(ch->in_room);
      send_to_char(rm->name, ch);
      buf[0]='[';
      sprintbit((long) rm->room_flags,room_bits,buf+1);
      if (buf[strlen(buf)-1]==' ') buf[strlen(buf)-1]=']';
      else strcat(buf,"]");
      if (IS_IMMORTAL(ch)) send_to_char(buf,ch);
      send_to_char("\n", ch);
      send_to_char(rm->description, ch);

      if (!IS_NPC(ch)) {
	if (IS_SET(ch->specials.act, PLR_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      CLEAR_BIT(ch->specials.act, PLR_HUNTING);
	    }
	  } else {
	    ch->hunt_dist = 0;
	    CLEAR_BIT(ch->specials.act, PLR_HUNTING);
	  }
	}
      } else {
	if (IS_SET(ch->specials.act, ACT_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      CLEAR_BIT(ch->specials.act, ACT_HUNTING);
	    }
	  } else {
	    ch->hunt_dist = 0;
	    CLEAR_BIT(ch->specials.act, ACT_HUNTING);
	  }
	}
      }

      list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
      list_char_in_room(real_roomp(ch->in_room)->people, ch);

    }
      break;
    }
  }
}

/* end of look */




void do_read(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
    struct obj_data *obj_object;  /* the potential book          */
    struct char_data *dummy;

/* Is it a book? */

   one_argument(argument,buf);

   generic_find(buf, FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
        FIND_OBJ_INV | FIND_CHAR_ROOM, ch, &dummy, &obj_object);

#if 0
   if(obj_object && GET_ITEM_TYPE(obj_object)==ITEM_OTHER) {

      for (extra = obj_object->ex_description; extra;
          extra = extra->next) {
          if(atoi(extra->keyword)==obj_object->obj_flags.value[0])
            break;
      }

      if(!extra)
         log("Error in do_read: book page does not exist");

      file_to_q(extra->description, ch->desc, TRUE);
      act("$n reads from $p",FALSE,ch,obj_object,0,TO_ROOM);
      return;
  } else {
#endif
      /* This is just for now - To be changed later.! */
      sprintf(buf,"at %s",argument);
      do_look(ch,buf,15);
#if 0
  }
#endif
}

int hash_string(char *s)
{
  char *p;
  int value = 0;

  for(p = s; *p; p++){
    value = value << 1;
    value = ~value;
    value += *p;
  }
  return((value < 0) ? -value : value);
}

void do_examine(struct char_data *ch, char *argument, int cmd)
{
  int gweight; /* tenth pounds */
  int bits, i, hash;
  char name[MAX_STRING_LENGTH];
  struct char_data *tmp_ch;
  struct obj_data *obj;

  one_argument(argument, name);

  if(!*name){
    sendf(ch, "Examine what?\n");
    return;
  }

  bits = generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM |
    FIND_OBJ_EQUIP, ch, &tmp_ch, &obj);

  if(!obj){
    sendf(ch, "You can't find it.\n");
    return;
  }

  strcpy(name, fname(obj->name));
  strcat(name, GET_NAME(ch));
  hash = hash_string(name) % 50;

  gweight = GET_OBJ_MASS(obj) / 45;
  if(gweight < 15){
    sendf(ch, "The %s is mostly %s and weighs %s\n",
      fname(obj->name), material_types[obj->material], (gweight < 9) ?
      "less than 1 lb." : "about 1 lb.");
  }
  else{
    sendf(ch, "The %s is mostly %s and weighs about %d lbs.\n",
      fname(obj->name), material_types[obj->material], (gweight + 5) / 10);
  }
  if((50 + hash) < get_skill(ch, SKILL_APPRAISE)){
    sendf(ch, "You estimate the value to be about %d coins.\n",
      (obj->obj_flags.cost * (hash + 75)) / 100);
  }
  if(IS_OBJ_STAT(obj, ITEM_MAGIC)){
    for(i = 0; i < MAX_OBJ_AFFECT; i++){
      if(obj->affected[i].location != APPLY_NONE){
        if((50 + hash) < (get_skill(ch, SKILL_EXAMINE_WARD) +
	    power_penalty(ch) - materials[obj->material].iron)){
          switch(obj->affected[i].location){
	    case APPLY_STR:
	      sendf(ch, "You sense a ward which %s the strength of the user.\n",
	        (obj->affected[i].modifier < 0) ? "decreases" : "increases");
              break;
	    case APPLY_DEX:
	      sendf(ch, "You sense a ward which %s the dexterity of the user.\n",
	        (obj->affected[i].modifier < 0) ? "decreases" : "increases");
              break;
	    case APPLY_INT:
	      sendf(ch, "You sense a ward which %s the intelligence of the user.\n",
	        (obj->affected[i].modifier < 0) ? "decreases" : "increases");
              break;
	    case APPLY_WIS:
	      sendf(ch, "You sense a ward which %s the wisdom of the user.\n",
	        (obj->affected[i].modifier < 0) ? "decreases" : "increases");
              break;
	    case APPLY_CON:
	      sendf(ch, "You sense a ward which %s the constitution of the user.\n",
	        (obj->affected[i].modifier < 0) ? "decreases" : "increases");
              break;
	    case APPLY_DAMROLL:
	    case APPLY_HITROLL:
	    case APPLY_HITNDAM:
	      sendf(ch, "You sense a ward which %s the fighting prowess of the user.\n",
	        (obj->affected[i].modifier < 0) ? "decreases" : "increases");
              break;
            case APPLY_ALL_AC:
	    case APPLY_FEET_AC:
	    case APPLY_LEGS_AC:
	    case APPLY_ARMS_AC:
	    case APPLY_BODY_AC:
	    case APPLY_HEAD_AC:
	      sendf(ch, "You sense a ward which %s the armor of the user.\n",
	        (obj->affected[i].modifier > 0) ? "decreases" : "increases");
              break;
            case APPLY_SAVING_PARA:
	    case APPLY_SAVING_ROD:
	    case APPLY_SAVING_PETRI:
	    case APPLY_SAVING_BREATH:
	    case APPLY_SAVING_SPELL:
	    case APPLY_SAVE_ALL:
	      sendf(ch, "You sense a ward which %s the users resistance.\n",
	        (obj->affected[i].modifier > 0) ? "decreases" : "increases");
              break;
          }
        }
      }
    }
  }
}

void do_inventory(struct char_data *ch, char *argument, int cmd)
{
  send_to_char("You are carrying:\n", ch);
  if (!ch->carrying)
	send_to_char("Nothing.\n",ch);
  else
	list_obj_on_char(ch->carrying, ch);
}


void do_equipment(struct char_data *ch, char *argument, int cmd)
{
  int j,Worn_Index;
  bool found;

  send_to_char("You are using:\n", ch);
  found = FALSE;
/*  for (Worn_Index = j = 0; j < FORM(ch).nlocs; j++) {
    if (ch->locations[j].worn) {
      Worn_Index++;
      sprintf(buf, "<%s>", location_text[ch->locations[j].type]);
      sendf(ch, "[%d] %-19s", Worn_Index, buf);
      found = TRUE;
      if (CAN_SEE_OBJ(ch, ch->locations[j].worn)) {
        show_obj_to_char(ch->locations[j].worn, ch, 1);
      }
      else{
        send_to_char("something\n", ch);
      }
    }
  }
*/
  for (Worn_Index = j=0; j< MAX_WEAR; j++) {
    if (ch->equipment[j]) {
      Worn_Index++;
      sendf(ch,"[%d] %s",Worn_Index,where[j]);
      if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
	show_obj_to_char(ch->equipment[j],ch,1);
	found = TRUE;
      } else {
	send_to_char("Something.\n",ch);
	found = TRUE;
      }
    }
  }
  if(!found) {
    send_to_char(" Nothing.\n", ch);
  }
}
