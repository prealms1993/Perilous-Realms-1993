/* ************************************************************************
*  file: spec_procs.c , Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <fcns.h>
#include <externs.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <spells.h>
#include <limits.h>
#include <hash.h>
#include <rooms.h>

#undef howmany
#define INQ_SHOUT 1
#define INQ_LOOSE 0

#define SWORD_ANCIENTS 25000
#define MAX_STORAGE 25
/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct time_info_data time_info;
extern struct weather_data weather_info;
extern int top_of_world;
extern struct int_app_type int_app[26];

/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */





/* ********************************************************************
*  Special procedures for rooms                                       *
******************************************************************** */
int dump(struct char_data *ch, int cmd, char *arg) 
{
  struct obj_data *k;
  char buf[100];
  struct char_data *tmp_char;
  int value=0;
  
  char *fname(char *namelist);
  
  for(k = real_roomp(ch->in_room)->contents; k ; k = real_roomp(ch->in_room)->contents)
    {
      sprintf(buf, "The %s vanish in a puff of smoke.\n" ,fname(k->name));
      for(tmp_char = real_roomp(ch->in_room)->people; tmp_char;
	  tmp_char = tmp_char->next_in_room)
	if (CAN_SEE_OBJ(tmp_char, k))
	  send_to_char(buf,tmp_char);
      extract_obj(k);
    }
  
  if(cmd!=60) return(FALSE);
  
  do_drop(ch, arg, cmd);
  
  value = 0;
  
  for(k = real_roomp(ch->in_room)->contents; k ; k = real_roomp(ch->in_room)->contents)
    {
      sprintf(buf, "The %s vanish in a puff of smoke.\n",fname(k->name));
      for(tmp_char = real_roomp(ch->in_room)->people; tmp_char;
	  tmp_char = tmp_char->next_in_room)
	if (CAN_SEE_OBJ(tmp_char, k))
	  send_to_char(buf,tmp_char);
      value+=(MIN(1000,MAX(k->obj_flags.cost/4,1)));
      /*
	value += MAX(1, MIN(50, k->obj_flags.cost/10));
	*/
      extract_obj(k);
    }
  
  if (value) 	{
    act("You are awarded for outstanding performance.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n has been awarded for being a good citizen.", TRUE, ch, 0,0, TO_ROOM);
    
    if (GetMaxLevel(ch) < 3)
      gain_exp(ch, MIN(100,value));
    else
      GET_GOLD(ch) += value;
  }
  return(TRUE);
}

int pet_shops(struct char_data *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH], pet_name[256];
  int pet_room;
  struct char_data *pet;
  
  pet_room = ch->in_room+1;
  
  if (cmd==59) { /* List */
    send_to_char("Available pets are:\n", ch);
    for(pet = real_roomp(pet_room)->people; pet; pet = pet->next_in_room) {
      sprintf(buf, "%8d - %s\n", 24*GET_EXP(pet), pet->player.short_descr);
      send_to_char(buf, ch);
    }
    return(TRUE);
  } else if (cmd==56) { /* Buy */
    
    arg = one_argument(arg, buf);
    only_argument(arg, pet_name);
    /* Pet_Name is for later use when I feel like it */
    
    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("There is no such pet!\n", ch);
      return(TRUE);
    }
    
    if (GET_GOLD(ch) < (GET_EXP(pet)*10)) {
      send_to_char("You don't have enough gold!\n", ch);
      return(TRUE);
    }
    
    GET_GOLD(ch) -= GET_EXP(pet)*10;
    
    pet = get_mob(pet->virtual);
    GET_EXP(pet) = 0;
    set_bit(pet->specials.affected_by, AFF_CHARM);
    
    if (*pet_name) {
      sprintf(buf,"%s %s", pet->player.name, pet_name);
      FREE(pet->player.name);
      pet->player.name = mystrdup(buf);		
      
      sprintf(buf,"%sA small sign on a chain around the neck says 'My Name is %s'\n",
	      pet->player.description, pet_name);
      FREE(pet->player.description);
      pet->player.description = mystrdup(buf);
    }
    
    char_to_room(pet, ch->in_room);
    add_follower(pet, ch);
    
    IS_CARRYING_M(pet) = 0;
    IS_CARRYING_V(pet) = 0;
    
    send_to_char("May you enjoy your pet.\n", ch);
    act("$n bought $N as a pet.",FALSE,ch,0,pet,TO_ROOM);
    
    return(TRUE);
  }
  
  /* All commands except list and buy */
  return(FALSE);
}
//int compare_cost(const struct obj_data **a, const struct obj_data **b)
int compare_cost(const void *p1, const void *p2)
{
	const struct obj_data *a = p1;
	const struct obj_data *b = p2;
	int diff;
	diff = rent_cost2(a,0)-rent_cost2(b,0);
	if (!diff) return(0);
	else return(diff/abs(diff));
}

void update_warehouse(struct char_data *ch)
{
  int days;
  char buf[MAX_STRING_LENGTH];
  int total;
  int cost;
  int i,count,now;
  struct obj_data *obj[MAX_STORAGE],*o;

  if (!ch->warehouse) return;
  *buf='\0';
  now=time(0);
  for (i=count=0,o=ch->warehouse; o && count<MAX_STORAGE; o=o->next_content,i++)
    if (now-o->time_stamp>24*60*60)
      obj[count++]=o;
  sendf(ch,"You have %d items in storage.\n",i);
  if (!count) return;
  qsort(obj, count, sizeof(struct obj_data *), compare_cost);
  for (total = i = 0; i<count; i++) {
    days = (now-obj[i]->time_stamp)/(24*60*60);
    cost=rent_cost(ch,obj[i],0)*days;
    if (GET_BANK(ch)<cost) {
      if (GET_GOLD(ch)<cost) {
	sprintf(buf+strlen(buf),"You can't afford to keep your %s.\n",
		obj[i]->short_description);
	obj_from_warehouse(obj[i]);
	obj_to_char(obj[i],InsuranceGuy());
        obj[i]->held_for=mystrdup(GET_NAME(ch));
      } else GET_GOLD(ch) -= cost;
    } else GET_BANK(ch)-=cost;
    if (obj[i]->in_warehouse) {
      total+=cost;
      obj[i]->time_stamp += (days*24*60*60);
      sprintf(buf+strlen(buf),"You pay %d for %d day%s rent.\n",
	cost,days,days>1?"s":"");
    }
  }
  sprintf(buf+strlen(buf),"\nThat's a total of %d coins.\n",total);
  sendf(ch,buf);
}

int bank (struct char_data *ch, int cmd, char *arg)
{
  int per_day;
  int per_hour;
  int per_minute;
  int total;
  int time_spent;
  int count=0;
  struct obj_data *obj;
  static char buf[256];
  int money;
  
  if (cmd!=219 && cmd!=220 && cmd!=221 && cmd!=59)
    return(FALSE);

  for (; isspace(*arg); arg++);
  money = atoi(arg);
  if (money<0) {
    sendf(ch,"Go away, you bother me.\n");
    return(TRUE);
  }
  
  if (strchr(arg,'.') || strchr(arg,'*') || cmd==59)
    money=-1;
  if (isdigit(*arg) && money==0) {
    sendf(ch,"If you have some business to do, lets do it!\n");
    return(TRUE);
  }
  if (money<0) money=0;
  /*deposit*/
  if (cmd==219) {
    if (money > GET_GOLD(ch)) {
      send_to_char("You don't have enough for that!\n", ch);
      return(TRUE);
    } else if (money>0) {
      send_to_char("Thank you.\n",ch);
      GET_GOLD(ch) = GET_GOLD(ch) - money;
      GET_BANK(ch) = GET_BANK(ch) + money;
      sendf(ch,"Your balance is %d.\n", GET_BANK(ch));
      if (GET_BANK(ch) > 2000000) {
	vlog(LOG_MISC,"%s has %d coins in the bank.", 
		GET_NAME(ch), GET_BANK(ch));
      }
      return(TRUE);
    }
    for (count=0,obj=ch->warehouse; obj; obj=obj->next_content)
      count++;
    if (count>=MAX_STORAGE) {
      sendf(ch,"You have too many items in storage already.\n");
      return(TRUE);
    }
    obj=get_obj_in_list_vis(ch,arg,ch->carrying);
    if (!obj) {
      sendf(ch,"You don't seem to have that on you.\n");
      return(TRUE);
    }
    if (ITEM_TYPE(obj,ITEM_SPELL_POUCH) || ITEM_TYPE(obj,ITEM_TRASH) ||
	ITEM_TYPE(obj,ITEM_CONTAINER) || ITEM_TYPE(obj,ITEM_KEY) ||
	obj->contains || ITEM_TYPE(obj,ITEM_TRAP) || !CAN_DROP(ch,obj)) {
      sendf(ch,"Sorry, but bank policy forbids me from storing that for you.\n");
      return(TRUE);
    }
    money=rent_cost(ch,obj,0);
    if (money<500) {
      sendf(ch,"That isn't worth storing.\n");
      return(TRUE);
    }
    sendf(ch,"It will cost you %d coins per day to store that.\n",money);
    if (GET_GOLD(ch)+GET_BANK(ch)<money) {
      sendf(ch,"You can't even afford to store this for one day!\n");
      return(TRUE);
    }
    obj_from_char(obj);
    obj_to_warehouse(obj,ch);
    sendf(ch,"Ok.\n");
    return(TRUE);
  }
    /*withdraw*/
  if (cmd==220) {
    if (money > GET_BANK(ch)) {
      send_to_char("You don't have enough in the bank for that!\n", ch);
      return(TRUE);
    } else if (money) {
      send_to_char("Thank you.\n",ch);
      GET_GOLD(ch) = GET_GOLD(ch) + money;
      GET_BANK(ch) = GET_BANK(ch) - money;
      sendf(ch,"Your balance is %d.\n", GET_BANK(ch));
      return(TRUE);
    }
    obj=get_obj_in_list(arg,ch->warehouse);
    if (!obj) {
      sendf(ch,"We aren't storing that for you!\n");
      return(TRUE);
    }
    if (!can_carry(ch,ch,obj)) return(TRUE);
    time_spent=(time(0)-obj->time_stamp)/60; /* time in warehouse in min */
    per_day=rent_cost(ch,obj,0);
    per_hour=per_day/24;
    per_minute=per_hour/60;
    total=time_spent/(60*24);
    if (total>1) {
      time_spent -= (total*60*24);
      total *= per_day;
    }
    if (time_spent>60) {
      total += ((time_spent/60)*per_hour);
      time_spent = time_spent % 60;
    }
    total += (time_spent+1)*per_minute;
    sendf(ch,"It will cost you %d coins to retrieve the %s\n",
		total,obj->short_description);
    if (total>GET_GOLD(ch)) {
      sendf(ch,"You don't have that much money on you.  Withdraw some.\n");
      return(TRUE);
    }
    GET_GOLD(ch) -= total;
    obj_from_warehouse(obj);
    obj_to_char(obj,ch);
    sendf(ch,"You now have your %s back.\n",obj->short_description);
    return(TRUE);
  }
  if (cmd == 221) {
    sprintf(buf,"Your balance is %d.\n", GET_BANK(ch));
    send_to_char(buf, ch);
    return(TRUE);
  }
  if (cmd==59) { /* list */
    if (!ch->warehouse) {
      sendf(ch,"You have nothing in storage here!\n");
      return(TRUE);
    }
    for (count=0,obj=ch->warehouse; obj; obj=obj->next_content)
      sendf(ch,"[%2d] %7d %s\n",
	    ++count,rent_cost(ch,obj,0),obj->short_description);
    return(TRUE);
  }
  return(FALSE);
}



/* Idea of the LockSmith is functionally similar to the Pet Shop */
/* The problem here is that each key must somehow be associated  */
/* with a certain player. My idea is that the players name will  */
/* appear as the another Extra description keyword, prefixed     */
/* by the words 'item_for_' and followed by the player name.     */
/* The (keys) must all be stored in a room which is (virtually)  */
/* adjacent to the room of the lock smith.                       */

int pray_for_items(struct char_data *ch, int cmd, char *arg)
{
  char buf[256];
  int key_room, gold;
  bool found;
  struct obj_data *tmp_obj, *obj;
  struct extra_descr_data *ext;
  
  if (cmd != 176) /* You must pray to get the stuff */
    return FALSE;
  
  key_room = 1+ch->in_room;
  
  strcpy(buf, "item_for_");
  strcat(buf, GET_NAME(ch));
  
  gold = 0;
  found = FALSE;
  
  for (tmp_obj = real_roomp(key_room)->contents; tmp_obj; tmp_obj = tmp_obj->next_content)
    for(ext = tmp_obj->ex_description; ext; ext = ext->next)
      if (str_cmp(buf, ext->keyword) == 0) {
	if (gold == 0) {
	  gold = 1;
	  act("$n kneels and at the altar and chants a prayer to Odin.",
	      FALSE, ch, 0, 0, TO_ROOM);
	  act("You notice a faint light in Odin's eye.",
	      FALSE, ch, 0, 0, TO_CHAR);
	}
	obj=get_obj(tmp_obj->virtual);
        obj_to_room(obj, ch->in_room);
	act("$p slowly fades into existence.",FALSE,ch,obj,0,TO_ROOM);
	act("$p slowly fades into existence.",FALSE,ch,obj,0,TO_CHAR);
        gold += obj->obj_flags.cost;
        found = TRUE;
      }
  
  
  if (found) {
    GET_GOLD(ch) -= gold;
    GET_GOLD(ch) = MAX(0, GET_GOLD(ch));
    return TRUE;
  }
  
  return FALSE;
}


int hospital(struct char_data *ch, int cmd, char *arg)
/* Special procedure Room for hospital
 * By Adam Caldwell
 * acaldwel@bigbird.cs.ohiou.edu
 */
{
	char buf[MAX_STRING_LENGTH];
	int choice;
	unsigned long cost=0;

	if (IS_NPC(ch)) return(FALSE);
	if (cmd==59) { /* List */
		send_to_char("Available cures are:\n", ch);
		send_to_char("1. Hit point restoration (50/hp + 1000/lev)\n",ch);
		send_to_char("2. Mana restoration (100*level/ mana pt)\n",ch);
		send_to_char("3. Power restoration (100*level/ power pt)\n", ch);
		return(TRUE);
	} else if (cmd==56) { /* Buy */
		arg = one_argument(arg, buf);
		choice = atoi(buf);
		if (choice<1 || choice>3) {
			send_to_char("You can't buy that here\n",ch);
			return(TRUE);
		}
		if ((GET_HIT(ch)>=hit_limit(ch) && choice==1) || (GET_MANA(ch)>=mana_limit(ch) && choice==2) || (GET_POWER(ch)>=power_limit(ch) && choice==3)) {
		  send_to_char("You don't need it!\n",ch);
		  send_to_char("Aren't you glad this isn't Judy Mud?\nOtherwise you would have been charged anyways\n",ch);
		  return(TRUE);
		}
		switch (choice) {
		  case 1: cost=GetMaxLevel(ch)*1000+(hit_limit(ch)-GET_HIT(ch))*50; break;
		  case 2: cost=GetMaxLevel(ch)*100*(mana_limit(ch)-GET_MANA(ch)); break;
		  case 3: cost=GetMaxLevel(ch)*100*(power_limit(ch)-GET_POWER(ch)); break;
		}
		sprintf(buf,"Its gonna cost you %d gold.\n",cost);
		send_to_char(buf,ch);

		if (GET_GOLD(ch) < cost){
			send_to_char("You don't have enough gold!\n", ch);
			return(TRUE);
		}

		GET_GOLD(ch) -= cost;

		switch (choice) {
		case 1:	GET_HIT(ch)=hit_limit(ch);
		   	send_to_char("A warm fealing fills your body.\n", ch);
			act("$n looks healthier.",FALSE,ch,0,ch,TO_ROOM);
			break;
		case 2: GET_MANA(ch)=mana_limit(ch);
		   	send_to_char("Energy fills your body.\n", ch);
			act("$n sizzles with energy.",FALSE,ch,0,ch,TO_ROOM);
			break;
		case 3: GET_POWER(ch)=power_limit(ch);
		   	send_to_char("Energy fills your body.\n", ch);
			act("$n sizzles with energy.",FALSE,ch,0,ch,TO_ROOM);
			break;
		}
		return(TRUE);
	}

	/* All commands except list and buy */
	return(FALSE);
}

# define BOUNCE_AMT 	16000
int bounce(struct char_data *ch,int cmd, char *arg)
{
  if (!IS_NPC(ch) && cmd==22 && GET_EXP(ch)<BOUNCE_AMT) {
    GET_EXP(ch)=BOUNCE_AMT;
    sendf(ch,"That's all it takes.. check your experience...\n");
    act("$n impresses the gods with $s bounciness.",FALSE,ch,0,0,TO_ROOM);
    return(TRUE);
  } else return(FALSE);
}
