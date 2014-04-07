/* ************************************************************************
*  file: reception.c, Special module for Inn's.           Part of DIKUMUD *
*  Usage: Procedures handling saving/loading of player objects            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <sys/time.h>
#include <ctype.h>

#include <fcns.h>
#include <comm.h>
#include <handler.h>
#include <db.h>
#include <interpreter.h>
#include <utils.h>
#include <spells.h>
#include <externs.h>

extern struct room_data *world;


char *name_to_path(char *name)
{
  static char buffer[200];
  char *p;

  sprintf(buffer,"stash/%c/%s",name[0],name);
  p=buffer;
  for (p=buffer; *p; p++)
    if (isupper(*p))
	*p=tolower(*p);
  return(buffer);
}

  
/* ************************************************************************
* Routines used to update object file, upon boot time                     *
************************************************************************* */

void update_obj_file(void)
{
}


/* ************************************************************************
* Routine Receptionist                                                    *
************************************************************************* */


int receptionist(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *recep = 0;
  static sh_int action_tabel[9] = {23,24,36,105,106,109,111,142,147};
  
  if (!ch->desc)
    return(FALSE); /* You've forgot FALSE - NPC couldn't leave */
  
  recep=FindMobInRoomWithFunction(ch->in_room,receptionist);
  
  if (!recep) {
    log("No receptionist.\n");
    exit(1);
  }
  
  if (IS_NPC(ch))
    return(FALSE);
  
  if (cmd != 92) {
    if (!number(0, 30))
      do_action(recep, "", action_tabel[number(0,8)]);
    return(FALSE);
  }
  
  if (!AWAKE(recep)) {
    act("$e isn't able to talk to you...", FALSE, recep, 0, ch, TO_VICT);
    return(TRUE);
  }
  if (!CAN_SEE(recep, ch)) 
    {
      act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0, TO_ROOM);
      return(TRUE);
    }
  
  if (cmd == 92) { /* Rent  */
    if(count_items(ch) > 150){
      act("$n tells you, 'You have too many items to rent.'", FALSE, recep, 0,
        ch, TO_VICT);
      return(TRUE);
    }
    act("$n stores your stuff in the safe, and helps you into your chamber.", FALSE, recep, 0, ch, TO_VICT);
    act("$n helps $N into $S private chamber.",FALSE, recep,0,ch,TO_NOTVICT);
    rent_char(ch);
    ch->specials.was_in_room=ch->in_room;
    extract_char(ch);
  } else {         /* Offer */
    act("$n tells you - Rent is free... for now...",FALSE,recep,0,ch,TO_VICT);
  }
  
  return(TRUE);
}

void take_obj_list(struct char_data *ch,struct obj_data *o,int *amount,struct char_data *give_to)
{
  char buf[MAX_STRING_LENGTH];

  if (*amount<=0 && !o) return;
  if (o->contains)
    take_obj_list(ch,o->contains,amount,give_to);
  else if (o->next_content)
    take_obj_list(ch,o->next_content,amount,give_to);
  else {
    if (o->carried_by)
      obj_from_char(o);
    if (o->in_obj)
      obj_from_obj(o);
    *amount -= o->obj_flags.cost;
    sendf(ch,"Your %s is taken.\n",o->short_description);
    if (give_to)
      sendf(give_to,"You take his %s.\n",o->short_description);
    if (give_to) {
      sprintf(buf,"FINE: %d take from %s",o->virtual,GET_NAME(ch));
      slog(buf);
      obj_to_char(o,give_to);
      o->held_for=mystrdup(GET_NAME(ch));
    } else {
      sprintf(buf,"LINK: %d take from %s",o->virtual,GET_NAME(ch));
      slog(buf);
      extract_obj(o);
    }
  }
}

void fine(struct char_data *ch,int amount,struct char_data *to_whom)
{
  int i;
  struct obj_data *o;
  int fine;

  fine=0;
  if (GET_GOLD(ch)>amount) {
    GET_GOLD(ch)-=amount;
    if (to_whom) GET_GOLD(to_whom)+=amount;
    sendf(ch,"You pay %d coins from your pocket.\n",amount);
    return;
  }
  fine+=GET_GOLD(ch);
  if (to_whom) GET_GOLD(to_whom)+=GET_GOLD(ch);
  GET_GOLD(ch)=0;
  if (fine)
    sendf(ch,"You give the %d coins from your pockets.\n",fine);
  amount-=fine;
  if (GET_BANK(ch)>amount) {
    GET_BANK(ch)-=amount;
    if (to_whom) GET_GOLD(to_whom)+=amount;
    sendf(ch,"You pay %d from your savings account.\n",amount);
    return;
  }
  if (to_whom) GET_GOLD(to_whom)+=GET_BANK(ch);
  if (GET_BANK(ch))
    sendf(ch,"You give the %d coins from you bank account.\n",GET_BANK(ch));
  fine+=GET_BANK(ch);
  amount-=GET_BANK(ch);
  GET_BANK(ch)=0;
  while (ch->carrying && amount>0)
    take_obj_list(ch,ch->carrying,&amount,to_whom);
  for (i=0; i<MAX_WEAR && amount>0; i++) 
    if (ch->equipment[i]) {
      o=unequip_char(ch,i);
      obj_to_char(o,ch);
      while (ch->carrying && amount>0)
	take_obj_list(ch,ch->carrying,&amount,to_whom);
    }
}

int rent_cost(struct char_data *ch, struct obj_data *o,int threshold)
{
  if (!o) return(0);
  if (o->obj_flags.cost_per_day>=threshold && threshold>0)
    sendf(ch,"%s : %d\n",o->short_description, o->obj_flags.cost_per_day);
  return(o->obj_flags.cost_per_day +
		(o->in_obj?rent_cost(ch,o->next_content,threshold):0) +
		rent_cost(ch,o->contains,threshold));
}

//2014 - hacked/ duplicate of rent_cost() to allow const structure when ch is going to be null
int rent_cost2(const struct obj_data *o, int threshold)
{
	if (!o) return(0);
	return(o->obj_flags.cost_per_day +
		(o->in_obj ? rent_cost2(o->next_content, threshold) : 0) +
		rent_cost2(o->contains, threshold));
}




int real_cost(struct char_data *ch,struct obj_data *o,int threshold)
{
  if (!o) return(0);
  if (o->obj_flags.cost>=threshold && threshold>0)
    sendf(ch,"%s : %d\n",o->short_description, o->obj_flags.cost);
  return(o->obj_flags.cost +
		(o->in_obj?real_cost(ch,o->next_content,threshold):0) +
		real_cost(ch,o->contains,threshold));
}

int value_of_char_equip(struct char_data *ch,int threshold)
{
  int val,i;

  val=real_cost(ch,ch->carrying,threshold);
  for (i=0; i<MAX_WEAR; i++)
    val+=real_cost(ch,ch->equipment[i],threshold);
  return(val);
}

int amount_of_tax(struct char_data *ch)
{
  return(value_of_char_equip(ch,0)/1000 +
	 (GET_BANK(ch)+GET_GOLD(ch)/100));
}

int count_items(struct char_data *ch)
{
  int i, count = 0;

  for(i = 0; i < MAX_WEAR; i++){
    count += count_rentables(ch->equipment[i]);
  }

  count += count_rentables(ch->carrying);
  count += count_rentables(ch->warehouse);

  return(count);
}

int count_rentables(struct obj_data *list)
{
  int count = 0;
  struct obj_data *o;

  for(o = list; o; o = o->next_content){
    if((o->obj_flags.cost_per_day >= 0) &&
        (o->obj_flags.type_flag != ITEM_TRASH) &&
        !IS_SET(o->obj_flags.wear_flags, ITEM_CONSTRUCTION) &&
        !is_no_rent(o->virtual)){
      count++;
      count += count_rentables(o->contains);
    }
  }

  return(count);
}

struct obj_data *next_junk(struct obj_data *obj)
{
  struct obj_data *o;

  if (!obj) return(0);
  if (obj->obj_flags.cost_per_day<0) return(obj);
  if (obj->obj_flags.type_flag==ITEM_TRASH) return(obj);
  if (IS_SET(obj->obj_flags.wear_flags,ITEM_CONSTRUCTION)) return(obj);
  if (is_no_rent(obj->virtual)) return(obj);
  if (obj->contains) {
    o=next_junk(obj->contains);
    if (o) return(o);
  }
  if (obj->next_content) return(next_junk(obj->next_content));
  return(0);
}

void remove_junk(struct char_data *ch, int do_msg)
{
  int i;
  struct obj_data *o;
  for (i=0; i<MAX_WEAR; i++) {
    while ((o=next_junk(ch->equipment[i]))!=NULL) {
    if (do_msg)
		sendf(ch,"%s disintegrates.\n",o->short_description);
	if (o->equipped_by) {
	  o=unequip_char(ch,i);
	  if (o->carried_by) obj_from_char(o);
	  extract_obj(o);
	} else if (o->in_obj) {
	  obj_from_obj(o);
	  if (o->carried_by) obj_from_char(o);
	  extract_obj(o);
	} else extract_obj(o);
    }
  }
  while ((o=next_junk(ch->carrying))!=NULL) {
	if (do_msg)
    	sendf(ch,"%s disintegrates.\n",o->short_description);
    if (o->in_obj) obj_from_obj(o);
    if (o->carried_by) obj_from_char(o);
    extract_obj(o);
  }
}

void rent_char(struct char_data *ch)
{
  remove_junk(ch, 1);
  save_char(ch,ch->in_room);
}
