/************************************************************************
*  file: actwiz.c , Implementation of commands.           Part of DIKUMUD *
*  Usage : Wizard Commands.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

extern int DEBUG;

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <fcns.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <spells.h>
#include <limits.h>
#include <externs.h>

void stat_room(struct char_data *ch,int room)
{
  struct room_data *rm;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct extra_descr_data *desc;
  struct char_data *k;
  struct obj_data *j;
  int i;

  rm = real_roomp(room);
  sendf(ch,"Room name: %s, Of zone : %d. V-Number : %d\n",
	  rm->name, rm->zone, rm->number, ch->in_room);
  
  sprinttype(rm->sector_type,sector_types,buf2);
  sendf(ch,"Sector type : %s ", buf2);
  sendf(ch,"Special procedure: ",(rm->funct) ? "Exists\n" : "No\n");
  sendf(ch,"Room flags: ");
  sprintbit((long) rm->room_flags,room_bits,buf);
  sendf(ch,"%s\n",buf);
  sendf(ch,"Description:\n%s",rm->description);
  sendf(ch,"Extra description keywords:%s\n",rm->ex_description?"":" None");
  if (rm->ex_description)
    for (desc = rm->ex_description; desc; desc = desc->next)
      sendf(ch,"%s\n",desc->keyword);
  
  sendf(ch,"------- Chars present -------\n");
  for (k = rm->people; k; k = k->next_in_room)
    if (CAN_SEE(ch,k))
      sendf(ch,"%s (%s)\n",GET_NAME(k), IS_MOB(k)?"MOB":"PC");

  sendf(ch,"--------- Contents ---------\n");
  for (j = rm->contents; j; j = j->next_content)
    sendf(ch,"%s (%d)\n",j->name,j->virtual);

  sendf(ch,"------- Exits defined -------\n");
  for (i = 0; i <= 5; i++) {
    if (rm->dir_option[i]) {
      sprintbit(rm->dir_option[i]->exit_info,exit_bits,buf);
      sendf(ch,"Dir:%-5s To: %7d Key: %5d Bits: %s\n",
		dirs[i],
		rm->dir_option[i]->to_room,
		rm->dir_option[i]->key,
		buf);
      if (rm->dir_option[i]->keyword) 
	sendf(ch,"Keyword: %s\n", rm->dir_option[i]->keyword);
      if (rm->dir_option[i]->general_description)
	sendf(ch,"Description: %s\n",rm->dir_option[i]->general_description);
    }
  }
}


void stat_char(struct char_data *ch,struct char_data *k)
{
  int i,i2;
  struct obj_data *j;
  struct follow_type *fol;
  struct affected_type *aff;
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  switch (k->player.sex) {
    case SEX_NEUTRAL : strcpy(buf,"NEUTRAL-SEX"); break;
    case SEX_MALE    : strcpy(buf,"MALE"); break;
    case SEX_FEMALE  : strcpy(buf,"FEMALE"); break;
    default : strcpy(buf,"ILLEGAL-SEX!!");
  }
 
  sendf(ch,"%s%s",buf,IS_MOB(k)?"MOB":"PC");
  sendf(ch," Name: %s    Class: %s   Race: %s   Room(was)[%d(%d)]",
	  GET_NAME(k), CLASS(k).class_name, race_list[GET_RACE(k)],
	  k->in_room,k->specials.was_in_room);

  if (IS_MOB(k)) sendf(ch,"VNum [%d]", k->virtual);
  sendf(ch,"\n");

  if (k->player.short_descr)
    sendf(ch,"Short description: %s\n", k->player.short_descr);
  if (!IS_MOB(k) && k->player.title) 
    sendf(ch,"Title: %s\n",k->player.title);
  if (k->player.long_descr)
    sendf(ch,"Long description: ",k->player.long_descr);
  

  sendf (ch,
  "Level [%d]   XP[%d]    Alignment[%d]\n",
    GET_LEVEL(k),
    GET_EXP (k),
    GET_ALIGNMENT(k));
  sendf(ch,"Coins: [%d], Bank: [%d], Hitroll: [%d], Damroll: [%d]\n",
     GET_GOLD(k), GET_BANK(k), k->points.hitroll, k->points.damroll);

  strcpy(buf1,asctime(localtime(&k->player.time.birth))+4);
  strcpy(buf2,asctime(localtime(&k->player.time.logon))+11);

  strcpy(buf,buf1+18); /* position of the year */
  buf[2]='\0'; /* cut off newline */
  strcpy(buf1+7,buf);
  buf1[6]=',';
  buf2[5]='\0';

  sendf(ch,"Birth: [%s] Logon: [%s] Played[%ld]secs\n", 
	  buf1,
	  buf2,
	  k->player.time.played);
  
  sendf(ch,"Age: %dy,%dm,%dd,%dh  Height:%d Weight:%d\n",
	  age(k).year, age(k).month, age(k).day, age(k).hours,
  	  GET_HEIGHT(k), GET_WEIGHT(k));

  sendf(ch,"Str:[%d]  Int:[%d]  Wis:[%d]  Dex:[%d]  Con:[%d]  Cha:[%d]  Lck:[%d]\n",
	  GET_STR(k),
	  GET_INT(k),
	  GET_WIS(k),
	  GET_DEX(k),
	  GET_CON(k),
	    GET_CHR (k),
	    GET_LCK (k));
  
  sendf(ch,"Mana p.:[%d/%d (%d) +%d]  Power p.:[%d/%d (%d) +%d]  Hit p.:[%d/%d+%d]  Move p.:[%d/%d+%d]\n",
	  GET_MANA(k),mana_limit(k),ch->points.max_mana,mana_gain(k),
	  GET_POWER(k),power_limit(k),ch->points.max_power,power_gain(k),
	  GET_HIT(k),hit_limit(k),hit_gain(k),
	  GET_MOVE(k),move_limit(k),move_gain(k) );

  sendf(ch,"AC/SP[F:%d/%d L:%d/%d A:%d/%d B:%d/%d H:%d/%d]\n",
     GET_FEET_AC(k),GET_FEET_STOPPING(k),GET_LEGS_AC(k),
     GET_LEGS_STOPPING(k),GET_ARMS_AC(k),GET_ARMS_STOPPING(k),
     GET_BODY_AC(k),GET_BODY_STOPPING(k),GET_HEAD_AC(k),
     GET_HEAD_STOPPING(k));

  
  sprinttype(GET_POS(k),position_types,buf2);
  sprintf(buf,"Position: %s",buf2);
  if (k->specials.fighting)
	sprintf(buf+strlen(buf)," Fighting: %s",GET_NAME(k->specials.fighting));
  if (k->desc) {
    sprinttype(k->desc->connected,connected_types,buf2);
    strcat(buf," Connected: ");
    strcat(buf,buf2);
  }
  sendf(ch,"%s\n",buf);
  sendf(ch, "Power casting penalty: %d, Stealth penalty: %d\n",
    power_penalty(k), stealth_penalty(k));

#if 0
  strcpy(buf,"Default position: ");
  sprinttype((k->specials.default_pos),position_types,buf2);
  strcat(buf, buf2);
#endif
  if (IS_NPC(k)) 	{
    strcpy(buf,"NPC flags: ");
    sprintbit(k->specials.act,action_bits,buf2);
  }  else	{
    strcpy(buf,"PC flags: ");
    sprintbit(k->specials.act,player_bits,buf2);
  }
  
  strcat(buf, buf2);
  
  sendf(ch,"%s Timer[%d]\n",buf,k->specials.timer);
  
  if (IS_MOB(k) && real_mobp(k->virtual)->func)
    sendf(ch,"Has special procedure.\n");
  
  if (IS_NPC(k))
    sendf(ch,"NPC Bare Hand Damage %dd%d.\n",
	    k->specials.damnodice, k->specials.damsizedice);
  
  sendf(ch,"Carried mass: %d g, Carried volume: %d cu. cm, Delay: %d\n",
	  IS_CARRYING_M(k),
	  IS_CARRYING_V(k),
	  k->specials.wait);
 
  for (i=0,j=k->carrying;j;j=j->next_content,i++);
  sprintf(buf,"Items in inventory: %d, ",i);
  
  for(i=0,i2=0;i<MAX_WEAR;i++)
    if (k->equipment[i]) i2++;
  sendf(ch,"%sItems in equipment: %d\n",buf,i2);
  
  sendf(ch,"Apply saving throws: [%d] [%d] [%d] [%d] [%d]\n",
	  k->specials.apply_saving_throw[0],
	  k->specials.apply_saving_throw[1],
	  k->specials.apply_saving_throw[2],
	  k->specials.apply_saving_throw[3],
	  k->specials.apply_saving_throw[4]);
  
  sendf(ch,"Thirst: %d, Hunger: %d, Drunk: %d\n",
	  k->specials.conditions[THIRST],
	  k->specials.conditions[FULL],
	  k->specials.conditions[DRUNK]);
  
  if (k->desc)
  	sendf (ch, "Descriptor[%d], IP#: %s\n",
	  	k->desc->id, k->desc->host);
 if (GET_LEVEL (ch) >= SILLYLORD)
	sendf (ch, "Email Address: %s\n",
		(k->player.email ? k->player.email : "<none>"));
  
  if (k->master)
    sendf(ch,"Master is '%s'\n",GET_NAME(k->master));

  if (k->followers) send_to_char("Followers are:\n", ch);
  for (fol=k->followers; fol; fol = fol->next)
    act("    $N", FALSE, ch, 0, fol->follower, TO_CHAR);
  
  if (k->specials.M_immune) {
    sprintbit(k->specials.M_immune, immunity_names, buf);
    sendf(ch,"Immune to: %s\n",buf);
  }
  if (k->specials.immune) {
    sprintbit(k->specials.immune, immunity_names, buf);
    sendf(ch,"Resistant to: %s\n",buf);
  }
  if (k->specials.susc) {
    sprintbit(k->specials.susc, immunity_names, buf);
    sendf(ch,"Susceptible to:",buf);
  }
  
  /* Showing the bitvector */
  sendf(ch,"Affected by: ");
  for (i2=i=0; i<255; i++)
    if (is_set(k->specials.affected_by,i))
	sendf(ch,"%s%s",(i2++)?", ":"",affected_bits[i]);
  if (!i2) sendf(ch,"Nothing\n"); else sendf(ch,"\n");

  if (IS_IMMORTAL(k))
   sendf(ch,"Character zone: %ld to %ld\n",k->build_lo,k->build_hi);
  
  /* Routine to show what spells a char is affected by */
  if (k->affected) {
    send_to_char("\nAffecting Spells:\n--------------\n", ch);
    for(aff = k->affected; aff; aff = aff->next) {
      sendf(ch,"%-25s: %10d : %s by %d : ",
		    spells[aff->type],
		    aff->duration,
		    apply_types[aff->location],
		    aff->modifier);
      for (i2=i=0; i<255; i++)
	if (is_set(aff->bitvector,i))
	    sendf(ch,"%s%s",(i2++)?", ":"",affected_bits[i]);
      if (!i2) sendf(ch,"Nothing\n"); else sendf(ch,"\n");
    }
  }
}

void stat_obj(struct char_data*ch,struct obj_data *j)
{
  struct extra_descr_data *desc;
  struct obj_data *j2;
  int i;
  int found;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  sprinttype(GET_ITEM_TYPE(j),item_types,buf);
  sprinttype(GET_OBJ_MATERIAL(j), material_types, buf2);
  sendf(ch,"Object name: [%s], V-number: [%d] Item type: %s Material: %s\n",
	  j->name, j->virtual, buf, buf2);

  if (j->short_description)
    sendf(ch,"Short description: %s\n",j->short_description);
  if (j->description)
    sendf(ch,"Long description:\n%s\n",j->description);

  if (j->ex_description) {
    sendf(ch,"Extra description keyword(s):\n----------\n");
    for (desc = j->ex_description; desc; desc = desc->next)
      sendf(ch,"%s\n",desc->keyword);
    sendf(ch,"----------\n");
  }
  
  sprintbit(j->obj_flags.wear_flags,wear_bits,buf);
  sendf(ch,"Can be worn on :\n",buf);
  
  sprintbit(j->obj_flags.extra_flags,extra_bits,buf);
  sendf(ch,"Extra flags: %s\n", buf);
  
  sendf(ch,"Mass: %d g (%d.%1d #), Volume: %d cm^3, Length: %d cm, Value: %d, Cost/day: %d, Timer: %d\n",
	  j->obj_flags.mass, j->obj_flags.mass / 454, ((j->obj_flags.mass * 10) / 454) % 10,  j->obj_flags.volume, j->obj_flags.length, j->obj_flags.cost,
	  j->obj_flags.cost_per_day, j->obj_flags.timer);
  
  if (real_roomp(j->in_room))
    sendf(ch,"In room: %d\n",j->in_room);
  else if (j->in_obj)
    sendf(ch,"In obj: %s\n",j->in_obj->name);
  else if (j->carried_by)
    sendf(ch,"Carried by: %s\n",GET_NAME(j->carried_by));

  switch (j->obj_flags.type_flag) {
  case ITEM_LIGHT : 
    sprintf(buf, "Colour : [%d]\nType : [%d]\nHours : [%d]",
	    j->obj_flags.value[0],
	    j->obj_flags.value[1],
	    j->obj_flags.value[2]);
    break;
  case ITEM_SCROLL : 
  case ITEM_POTION : 
    sprintf(buf, "Spells(%d): %s, %s, %s",
	    j->obj_flags.value[0],
	    (j->obj_flags.value[1] ? spells[j->obj_flags.value[1]] : "0"),
	    (j->obj_flags.value[2] ? spells[j->obj_flags.value[2]] : "0"),
	    (j->obj_flags.value[3] ? spells[j->obj_flags.value[3]] : "0"));
    break;
  case ITEM_STAFF : 
  case ITEM_WAND : 
    sprintf(buf, "Spell(%d): %s\nCharges: %d/%d",
	    j->obj_flags.value[0],
	    (j->obj_flags.value[3] ? spells[j->obj_flags.value[3]] : "0"),
	    j->obj_flags.value[2],
	    j->obj_flags.value[1]);
    break;
  case ITEM_WEAPON :
  case ITEM_FIREWEAPON : 
    sprintf(buf, "Weapon: [%s], Damage: %dd%d, Damage Type: [%s], Speed: %d\n",
	    weapon_types[j->obj_flags.value[0]],
	    j->obj_flags.value[1],
	    j->obj_flags.value[2],
	    damage_types[j->obj_flags.value[3]],
	    j->obj_flags.value[4]);
    break;
  case ITEM_MISSILE : 
    sprintf(buf, "Tohit : %d\nTodam : %d\nType : %s",
	    j->obj_flags.value[0],
	    j->obj_flags.value[1],
	    damage_types[j->obj_flags.value[3]]);
    break;
  case ITEM_ARMOR :
    sprintf(buf, "AC-modifier : [%d]\nStopping Power : [%d]\nFull Strength : [%d]",
	    j->obj_flags.value[0],
	    j->obj_flags.value[3],
	    j->obj_flags.value[1]);
    
    break;
  case ITEM_TRAP :
    sprintf(buf, "level: %d, att type: %d, damage class: %d, charges: %d", 
	    j->obj_flags.value[0],
	    j->obj_flags.value[1],
	    j->obj_flags.value[2],
	    j->obj_flags.value[3]);
    break;
  case ITEM_SPELL_POUCH :
  case ITEM_CONTAINER :
    sprintf(buf, "Max-contains : %d\nLocktype : %d\nCorpse : %s",
	    j->obj_flags.value[0],
	    j->obj_flags.value[1],
	    j->obj_flags.value[3]?"Yes":"No");
    break;
  case ITEM_DRINKCON :
    sprinttype(j->obj_flags.value[2],drinks,buf2);
    sprintf(buf, "Max-contains : %d\nContains : %d\nPoisoned : %d\nLiquid : %s",
	    j->obj_flags.value[0],
	    j->obj_flags.value[1],
	    j->obj_flags.value[3],
	    buf2);
    break;
  case ITEM_NOTE :
    sprintf(buf, "Tounge : %d",
	    j->obj_flags.value[0]);
    break;
  case ITEM_KEY :
    sprintf(buf, "Keytype : %d",
	    j->obj_flags.value[0]);
    break;
  case ITEM_FOOD :
    sprintf(buf, "Makes full : %d\nPoisoned : %d",
	    j->obj_flags.value[0],
	    j->obj_flags.value[3]);
    break;
    default :
      sprintf(buf,"Values 0-3 : [%d] [%d] [%d] [%d]",
	      j->obj_flags.value[0],
	      j->obj_flags.value[1],
	      j->obj_flags.value[2],
	      j->obj_flags.value[3]);
    break;
  }
  send_to_char(buf, ch);
  
  strcpy(buf,"\nEquipment Status: ");
  if (!j->carried_by)
    strcat(buf,"NONE");
  else {
    found = FALSE;
    for (i=0;i < MAX_WEAR;i++) {
      if (j->carried_by->equipment[i] == j) {
	sprinttype(i,equipment_types,buf2);
	strcat(buf,buf2);
	found = TRUE;
      }
    }
    if (!found)
      strcat(buf,"Inventory");
  }
  sendf(ch,"%s\n",buf);

  if (j->min_level) sendf(ch,"Equipment Level: %d\n", j->min_level);
  
  if (real_objp(j->virtual) && real_objp(j->virtual)->func)
    sendf(ch,"Has special procedure.\n");
  
  strcpy(buf,"Contains :\n");
  found = FALSE;
  for(j2=j->contains;j2;j2 = j2->next_content) {
    strcat(buf,fname(j2->name));
    strcat(buf,"\n");
    found = TRUE;
  }
  if (!found)
    strcpy(buf,"Contains : Nothing\n");
  send_to_char(buf, ch);
  if (real_objp(j->virtual) && real_objp(j->virtual)->max_at_boot<32000) {
    sendf(ch,"Limit: [%d] Current [%d]\n",real_objp(j->virtual)->max_at_boot,
			real_objp(j->virtual)->boot_count);
  }
  
  send_to_char("Can affect char :\n", ch);
  for (i=0;i<MAX_OBJ_AFFECT;i++) 
    if (j->affected[i].location!=APPLY_NONE) {
    sprinttype(j->affected[i].location,apply_types,buf2);
    sendf(ch,"    Affects : %s By %d\n", buf2,j->affected[i].modifier);
  }			
}

void do_stat(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char *p;
  struct char_data *c;
  struct obj_data *obj;
  int number;

#define OSTAT 74
#define MSTAT 320
#define PSTAT 321
#define RSTAT 322

  if (IS_NPC(ch))
    return;

  if (cmd==RSTAT) {
    stat_room(ch,ch->in_room);
    return;
  }

  only_argument(argument, arg1);

  /* no argument */
  if (!*arg1) {
    send_to_char("Stats on who or what?\n",ch);
    return;
  }

  number=atoi(arg1);
  if (number) {
    p=arg1;
    while (isdigit(*p)) p++;
    if (*p!='.') {
      sendf(ch,"If you are gonna use a number, use #.name\n");
      return;
    }
    p++; /* get rid of period */
    strcpy(buf,p);
    strcpy(arg1,buf);
  }

  if (cmd==OSTAT) {
    for (obj=object_list; obj;  obj=obj->next) {
      if (str_str(obj->name,arg1)) {
        if (number<=1) {
          stat_obj(ch,obj);
          return;
        } else number--;
      }
    }
  } else if (cmd==MSTAT) {
    for (c=character_list; c; c=c->next) {
      if (IS_MOB(c) && str_str(c->player.name,arg1) && CAN_SEE(ch,c)) {
        if (number<=1) {
	  stat_char(ch,c);
	  return;
        } else number--;
      }
    }
  } else if (cmd==PSTAT) {
    for (c=character_list; c; c=c->next) {
      if (!IS_MOB(c) && str_str(c->player.name,arg1) && CAN_SEE(ch,c)) {
        if (number<=1) {
	  stat_char(ch,c);
	  return;
        } else number--;
      }
    }
  } else {
    sendf(ch,"Error: cmd=%d\n",cmd);
    return;
  }
  sendf(ch,"Couldn't find %s\n",arg1);
}

