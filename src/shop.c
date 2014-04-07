/* ************************************************************************
*  file: shop.c , Shop module.                            Part of DIKUMUD *
*  Usage: Procedures handling shops and shopkeepers.                      *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>

#include <fcns.h>
#include <comm.h>
#include <handler.h>
#include <db.h>
#include <interpreter.h>
#include <utils.h>
#include <externs.h>

shop_data *shop_index;

static int is_ok(struct char_data *keeper, struct char_data *ch, shop_data *s)
{
  int is_open;
  int i;

  if (!s->n_hours) is_open=1;
  else is_open=0;
  for (i=0; i<s->n_hours && !is_open; i++)
    if (time_info.hours>=s->open[i] && time_info.hours<s->close[i])
      is_open=1;
  if (!is_open) {
    do_say(keeper,"Sorry, we are closed, but come back later.",17);
    return(FALSE);
  }
  if (!CAN_SEE(keeper,ch)) {
    do_say(keeper, "I don't trade with someone I can't see!",17);
    return(FALSE);
  }
  return(TRUE);
}

static int trade_with(struct obj_data *item, shop_data *s)
{
  int i;
  if (item->obj_flags.cost < 1) return(FALSE);

  for (i=0; i<s->n_type; i++)
    if ((unsigned)s->type[i]==(unsigned)item->obj_flags.type_flag)
      return(TRUE);
  for (i=0; i<s->n_vnum; i++)
    if (s->vnum[i]==item->virtual)
      return(TRUE);
  return(FALSE);
}

static int shop_producing(int virtual, shop_data *s)
{
  int i;

  for (i=0; i<s->n_producing; i++)
    if (s->producing[i]==virtual)
      return(TRUE);
  return(FALSE);
}

static void shopping_buy(char *arg, struct char_data *ch, struct char_data *keeper, shop_data *s)
{
  char name[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  int virtual;
  int cost;
  int num;
  char *p;
  int count,total;
  struct obj_data *obj;

  if(!(is_ok(keeper,ch,s)))
	  return;
  
  only_argument(arg, name);
  if(!*name) {
    sprintf(buf, "%s what do you want to buy??", GET_NAME(ch));
    do_tell(keeper,buf,19);
    return;
  }
  num=atoi(name);
  p=strchr(name,'*');
  if (!num || !p) {
    p=name;
    num=1;
  } else p++;

  obj=get_obj_in_list_vis(ch,p,keeper->carrying);
  if (!obj) {
    sprintf(buf,"%s %s",GET_NAME(ch), s->no_such_item1);
    do_tell(keeper,buf,19);
    return;
  }
  
  if (obj->obj_flags.cost <= 0) {
    sprintf(buf,"%s %s",GET_NAME(ch), s->no_such_item1);
    do_tell(keeper,buf,19);
    extract_obj(obj);
    return;
  }

  if ((IS_CARRYING_M(ch) + GET_OBJ_MASS(obj)) > CAN_CARRY_M(ch)) {
    sendf(ch,"%s : You can't carry that much weight.\n",fname(obj->name));
    return;
  }

  if ((IS_CARRYING_V(ch) + GET_OBJ_VOLUME(obj)) > CAN_CARRY_V(ch)) {
    sendf(ch,"%s : You can't carry that much volume.\n", fname(obj->name));
    return;
  }

  act("$n buys $p.", FALSE, ch, obj, 0, TO_ROOM);
  total=count=0;

  strcpy(name,obj->short_description);
  virtual = obj->virtual;
  while (num-- > 0) {
    if (GetMaxLevel(ch)<DEMIGOD) {
      cost = (int)(obj->obj_flags.cost*s->profit_sell);
      total += cost;
      if (GET_GOLD(ch)<cost) {
	sprintf(buf,"%s %s",GET_NAME(ch), s->missing_cash2);
	do_tell(keeper,buf,19);

	switch (s->temper1) {
	  case 0: do_action(keeper,GET_NAME(ch),30); return;
	  case 1: do_emote(keeper,"grins happily",36); return;
	}
	return;
      }
      
      GET_GOLD(ch) -= cost;
      GET_GOLD(keeper) += cost;
    }
    count++;

    if (shop_producing(virtual,s)) {
      obj = get_obj(obj->virtual);
      obj_to_char(obj,ch);
    } else {
      obj_from_char(obj);
      obj_to_char(obj,ch);
      for (obj=keeper->carrying; obj; obj=obj->next_content)
	if (obj->virtual==virtual) break;
      if (!obj) {
	sendf(ch,"Just ran out of those.\n");
	break;
      }
    }
  }
  if (!s->message_sell)
    vlog(LOG_URGENT,"shop %d has no sell message",s->index);
  else
    sendf(ch,s->message_sell,total);
  sendf(ch,"\n");
  sendf(ch,"You now have %s (*%d).\n",name,count);
}

static void shopping_sell(char *arg, struct char_data *ch, struct char_data *keeper,shop_data *s,int value_only)
{
  char name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  int cost,virtual;
  struct obj_data *obj;

  if (!is_ok(keeper,ch,s))
    return;

  only_argument(arg,name);

  if (!*name) {
    sprintf(buf,"%s What do you want to sell??",GET_NAME(ch));
    do_tell(keeper,buf,19);
    return;
  }

  obj=get_obj_in_list_vis(ch,name,ch->carrying);
  if (!obj) {
    sprintf(buf,"%s %s",GET_NAME(ch),s->no_such_item2);
    do_tell(keeper,buf,19);
    return;
  }

  virtual=obj->virtual;
  if (!trade_with(obj,s)) {
    sprintf(buf,"%s %s",GET_NAME(ch), s->do_not_buy);
    do_tell(keeper,buf,19);
    return;
  }

  cost = obj->obj_flags.cost;

  if (ITEM_TYPE(obj,ITEM_WAND) || ITEM_TYPE(obj,ITEM_STAFF)) {
    if (obj->obj_flags.value[1]) {
      cost = (int)cost * (float)(obj->obj_flags.value[2] /
			 (float) obj->obj_flags.value[1]);
    } else {
      cost = 0;
    }
  } else if (ITEM_TYPE(obj,ITEM_ARMOR)) {
    if (obj->obj_flags.value[1]) {
       cost = (int)cost * (float)(obj->obj_flags.value[0] /
		       (float)(obj->obj_flags.value[1]));
    } else {
      cost = 0;
    }
    obj->obj_flags.cost = cost;
  }

  if (GET_GOLD(keeper)<(int)(cost*s->profit_buy)) {
    sprintf(buf,"%s %s",GET_NAME(ch),s->missing_cash1);
    do_tell(keeper,buf,19);
    return;
  }
  cost = (int)(cost*s->profit_buy);
  if (value_only) {
    sendf(ch,"I'd give you %d coins for that.\n",cost);
    return;
  }

  act("$n sells $p.", FALSE, ch, obj, 0, TO_ROOM);

  if (s->message_buy)
    sendf(ch,s->message_buy,cost);
  else
    vlog(LOG_URGENT,"Shop %d has no buy message",s->index);
  sendf(ch,"\n");
  sendf(ch,"The shopkeeper now has %s\n",obj->short_description);
  GET_GOLD(keeper) -= cost;
  GET_GOLD(ch) += cost;
  obj_from_char(obj);
  obj_to_char(obj,keeper);
  return;
}

static void shopping_list( char *arg, struct char_data *ch, struct char_data *keeper, shop_data *s)
{
  struct string_block sb;
  struct obj_data *obj;
  extern char *drinks[];
  int found_obj=0;

  if (!is_ok(keeper,ch,s))
    return;

  init_string_block(&sb);
  sb_printf(&sb,"You can buy:\n");
  for (obj=keeper->carrying; obj; obj = obj->next_content)
    if (CAN_SEE_OBJ(ch,obj) && obj->obj_flags.cost>0) {
      found_obj = TRUE; 
      if (obj->obj_flags.type_flag != ITEM_DRINKCON) 
        sb_printf(&sb, "%s for %d gold coins.\n",
		      obj->short_description,
		      (int)(obj->obj_flags.cost*s->profit_sell));
      else {
	if (obj->obj_flags.value[1])
	  sb_printf(&sb,"%s of %s",
			obj->short_description,
			drinks[obj->obj_flags.value[2]]);
	else
	  sb_printf(&sb,"%s",obj->short_description);
	sb_printf(&sb, " for %d gold coins.\n",
			(int)(obj->obj_flags.cost*s->profit_sell));
      }
    }

  if (!found_obj)
    sb_printf(&sb,"Nothing!\n");
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
}

static void shopping_kill(char *arg, struct char_data *ch,struct char_data *keeper, shop_data *s)
{
  switch(s->temper2) {
    case 0: sendf(ch,"Don't ever try that again!"); return;
    case 1: sendf(ch,"Scram - midget!"); return;
  }
}


int shop_keeper(struct char_data *ch, int cmd, char *arg)
{
  char argm[MAX_INPUT_LENGTH];
  struct char_data *keeper;
  shop_data *s;


  keeper = FindMobInRoomWithFunction(ch->in_room,shop_keeper);
  if (!keeper) {
	vlog(LOG_URGENT,"Serious problem with shop_keeper in room %d",
		ch->in_room);
	return(FALSE);
  }

  for (s=shop_index; s; s=s->next)
    if (s->keeper == keeper->virtual) break;
  if (!s) {
    vlog(LOG_URGENT,"Couldn't find shop for keeper %d",keeper->virtual);
    return(FALSE);
  }

  if (!cmd)
    return(0);

  if (ch->in_room == s->in_room) {
    switch (cmd) {
	case 56: shopping_buy(arg,ch,keeper,s); break;
	case 57: shopping_sell(arg,ch,keeper,s,0); break;
	case 58: shopping_sell(arg,ch,keeper,s,1); break; /*value*/
	case 59: shopping_list(arg,ch,keeper,s); break;
	case 25: /* kill or hit */
	case 70:
	  only_argument(arg, argm);

	  if (keeper == get_char_room(argm,ch->in_room))
	    shopping_kill(arg,ch,keeper,s);
	  else return(FALSE);
	  break;
  	case 84:
	case 207:
	case 172:
	  act("$N tells you 'No magic here - kid!'.",FALSE,ch,0,keeper,TO_CHAR);
	  break;
	default:
	  return(FALSE);
    }
    return(TRUE);
  } else if (DEBUG)
     vlog(LOG_DEBUG,"shopkeeper (shop:%d, Vnum:%d) not in his room: %d != %d.",
		s->index,
		keeper->virtual,
		ch->in_room, 
		s->in_room);
  return(FALSE);
}


void assign_the_shopkeepers()
{
  shop_data *s;
  index_mem *h;

  for (s=shop_index; s; s=s->next) {
    h=real_mobp(s->keeper);
    if (h) h->func = shop_keeper;
    else nlog("shop: %d, keeper %d does not exist",s->index,s->keeper);
  }
}

void shop_give_inventory(struct char_data *ch)
{
  shop_data *s;
  struct obj_data *obj;
  int i;

  if (real_mobp(ch->virtual)->func!=shop_keeper) return;
  for (s=shop_index; s; s=s->next)
    if (s->keeper==ch->virtual) break;
  if (!s) {
    vlog(LOG_URGENT,"Problem giving inventory to %d",ch->virtual);
    return;
  }

  vlog(LOG_DEBUG,"Giving items to keeper %d (shop %d)",s->keeper,s->index);
  for (i=0; i<s->n_producing; i++) {
    obj=get_obj(s->producing[i]);
    if (!obj)
	vlog(LOG_URGENT,"Shop(%d): object %d does not exist.",
		s->index,s->producing[i]);
    else
	obj_to_char(obj,ch);
  }
}
