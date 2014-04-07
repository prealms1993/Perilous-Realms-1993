/************************************************************************
*  file: actwiz.c , Implementation of commands.           Part of DIKUMUD *
*  Usage : Wizard Commands.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <structs.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <spells.h>
#include <limits.h>
#include <fcns.h>
#include <ticks.h>
#include <externs.h>

#include <write.h>


typedef struct area_node {
  char *name;
  int offset;
  struct area_node *next;
} area_node;

area_node *area_list=NULL;

void add_area(char *name, int offset)
{
  area_node *p,*new;

  CREATE(new,area_node,1);
  new->name=mystrdup(name);
  new->offset=offset;
  new->next=NULL;

  if (DEBUG)
    vlog(LOG_DEBUG,"Adding area '%s' at %d",
		name,offset);
  if (!area_list) {
    area_list=new;
    return;
  }
  if (area_list->offset>offset) {
    new->next=area_list;
    area_list=new;
    return;
  }
  for (p=area_list; p->next; p=p->next)
    if (p->next->offset>offset) break;
  new->next=p->next;
  p->next=new;
}

void boot_areas()
{
  FILE *f;
  char *p,*q;
  char s[255];

  f=fopen("area.list","r");
  if (!f) {
    vlog(LOG_URGENT,"Couldn't open area.list");
    return;
  }
  while (fgets(s,sizeof(s),f)) {
    if (s[0]!='#') continue;
    p=s+8; /* skip #define */
    q=strchr(p,' ');
    *q='\0'; q++;
    add_area(p,atoi(q));
  }
  fclose(f);
}

void print_bitv(FILE *f,char **bits,ulong bitv)
{
  int nprinted=0;
  ulong i;

  for (i=0; bitv; i++)
    if (IS_SET(bitv,(1<<i))) {
      bitv -= (1<<i);
      fprintf(f,"%s%s",nprinted++==0?"":", ",bits[i]);
    }
}


char *room_number(int base,int room)
{
  area_node *a;
  static char buf[MAX_STRING_LENGTH];

  if (!area_list || !area_list->next)
    sprintf(buf,"%d",room-base);
  else {
    a=area_list;
    while (a->next && a->next->offset<=room)
	a=a->next;
    if (a->offset==base)
      sprintf(buf,"%d",room-base);
    else
      sprintf(buf,"%s:%d",a->name,room-a->offset);
  }
  return(buf);
}

void write_string(FILE *f,char *s)
{
  int i;
  i=strlen(s);
  if (i>70) fputc('\n',f); else fputc(' ',f);
  if (isspace(s[0]) || s[0]=='"') fputc('"',f);
  fprintf(f,"%s",s);
  if ((s[i-1]=='\n' && s[i-2]=='\n') || s[i-1]=='"')
    fputc('"',f);
}

void write_string1(FILE *f, char *s)
{
  int i;
  char buf[MAX_STRING_LENGTH];
  strcpy(buf,s);
  s=buf;
  for (;isspace(*s); s++);
  i=strlen(s)-1;
  while (i>0 && isspace(s[i])) i--;
  s[i+1]='\0';
  if (s[0]=='"') fputc('"',f);
  fprintf(f,"%s",s);
  if (s[i]=='"') fputc('"',f);
}

void write_room(FILE *f,int base,int room)
{
  struct room_data *rp;
  int i;

  rp=real_roomp(room);
  if (!rp) return;
  room -= base;
  
  fprintf(f,"\n%d\n{\n  name { %s }\n  desc {",
	room,rp->name);
  write_string(f,rp->description);
  fprintf(f,"  }\n");
  if (rp->sector_type)
    fprintf(f,"  sector { %s }\n",sector_types[rp->sector_type]);
  if (rp->river_dir<=5 && rp->river_speed>0)
    fprintf(f,"  river { %c, %d }\n",dirs[rp->river_dir][0],rp->river_speed);
  if (rp->ex_description) {
    struct extra_descr_data *ex;
    fprintf(f,"  extra {\n");
    for (ex=rp->ex_description; ex; ex=ex->next) {
      fprintf(f,"\tkeywords { ");
      write_string1(f,ex->keyword);
      fprintf(f," }\n\tdesc { ");
      write_string(f,ex->description);
      fprintf(f,"\t}\n");
    }
    fprintf(f,"  }\n");
  }
  if (rp->room_flags) {
    fprintf(f,"  flags { ");
    print_bitv(f,room_bits,rp->room_flags);
    fprintf(f," }\n");
  }
  if (real_roomp(rp->tele_targ) && rp->tele_time>0 && rp->tele_targ>0)
    fprintf(f,"  tele { %d, %d, %s }\n",rp->tele_time,rp->tele_look,
		room_number(base,rp->tele_targ));
  for (i=0; i<6; i++)
    if (rp->dir_option[i]) break;
  if (i<6) {
    fprintf(f,"  exits {\n");
    for (i=0; i<6; i++) {
      struct room_direction_data *d;
      if (!rp->dir_option[i]) continue;
      d=rp->dir_option[i];
      fprintf(f,"\tto { %c, %s }\n",
	dirs[i][0],
	room_number(base,d->to_room));
      if (d->key>0)
	fprintf(f,"\t  key { %d }\n",d->key);
      if (d->exit_info) {
        fprintf(f,"\t  info { ");
	print_bitv(f,exit_bits,d->exit_info);
	fprintf(f," }\n");
      }
      if (d->keyword && *d->keyword) {
	fprintf(f,"\t  keywords { ");
	write_string1(f,d->keyword);
	fprintf(f," }\n");
      }
      if (d->general_description && *d->general_description) {
	fprintf(f,"\t  desc { ");
	write_string(f,d->general_description);
	fprintf(f,"}\n");
      }
    }
    fprintf(f,"  }\n");
  }

  fprintf(f,"}\n");
}

typedef struct log_info {
  char *type;
  int min_level;
} log_info;

static log_info log_bits[] = {
{ "connection", REAL_IMMORT },
{ "mortalspy",	SILLYLORD },
{ "immortalspy", SILLYLORD },
{ "urgent", SILLYLORD },
{ "debug", IMPLEMENTOR },
{ "reject", REAL_IMMORT },
{ "misc", REAL_IMMORT },
{ "death", REAL_IMMORT },
{ "", IMPLEMENTOR+1 }
};

void do_viewlog(struct char_data *ch, char *arg, int cmd)
{
  int i,count=0;
  char buf[MAX_STRING_LENGTH];

  arg=one_argument(arg,buf);
  for (i=0; *log_bits[i].type; i++)
    if (is_abbrev(buf,log_bits[i].type)) break;

  if (!*buf || !*log_bits[i].type) {
    sendf(ch,"Usage: view bit\nWhere bit is one of:\n");
    for (i=0; *log_bits[i].type; i++)
      if (GET_LEVEL(ch)>=log_bits[i].min_level)
	sendf(ch,"%s%-18s%s",
		LOG(ch,(1<<i))?"+":"-",
		log_bits[i].type,
		(++count)%4?" ":"\n");
      sendf(ch,"\nNote: +=Set now, -=Not set now.  Don't include in view command.\n");
    return;
  }
  if (LOG(ch,1<<i)) {
    CLEAR_BIT(LOG_BITS(ch),(1<<i));
    sendf(ch,"Log bit cleared.\n");
    return;
  }
  SET_BIT(LOG_BITS(ch),(1<<i));
  sendf(ch,"Log bit set.\n");
}



void do_instazone(struct char_data *ch, char *argument, int cmdnum)
{
  char cmd, c, buf[MAX_STRING_LENGTH];
  int i, start_room, end_room, j, arg1, arg2, arg3;
  struct char_data *p;
  struct obj_data *o;
  struct room_data *room;
  FILE *fp;
  
  
  if (IS_NPC(ch))
    return;
  
  /*
   *   read in parameters (room #s)
   */
  start_room = -1; end_room = -1;
  sscanf(argument, "%d%c%d", &start_room, &c, &end_room);
  
  if ((start_room == -1) || (end_room == -1)) {
    send_to_char("Instazone <start_room> <end_room>\n", ch);
    return;
  }
  
  fp = (FILE *)MakeZoneFile(ch);
  
  if (!fp) {
    send_to_char("Couldn't make file.. try again later\n", ch);
    return;
  }
  
  for (i = start_room; i<=end_room; i++) {
    room = real_roomp(i);
    if (room) {
      
      /*
       *  first write out monsters
       */
      for (p = room->people; p; p = p->next_in_room) {
	if (IS_NPC(p)) {
	  cmd = 'M';
	  arg1 = MobVnum(p);
	  arg2 = real_mobp(p->virtual)->count;
	  arg3 = i;
	  Zwrite(fp, cmd, 0, arg1, arg2, arg3, p->player.short_descr);
	  for (j = 0; j<MAX_WEAR; j++) {
	    if (p->equipment[j]) {
	      if (real_objp(p->equipment[j]->virtual)) {
		cmd = 'E'; 
		arg1 = p->equipment[j]->virtual;
		arg2 = real_objp(p->equipment[j]->virtual)->count;
		arg3 = j;
		strcpy(buf, p->equipment[j]->short_description);
		Zwrite(fp, cmd,1,arg1, arg2, arg3, 
		       buf);
		RecZwriteObj(fp, p->equipment[j]);
	      }
	    }
	  }
	  for (o = p->carrying; o; o=o->next_content) {
	      cmd = 'G';
	      arg1 = o->virtual;
	      arg2 = real_objp(o->virtual)->count;
	      arg3 = 0;
              strcpy(buf, o->short_description);
	      Zwrite(fp, cmd, 1, arg1, arg2, arg3, buf);
	      RecZwriteObj(fp, o);
	  }
	}
      }
      /*
       *  write out objects in rooms
       */
      for (o = room->contents; o; o= o->next_content) {
	  cmd = 'O';
	  arg1 = o->virtual;
	  arg2 = real_objp(o->virtual)->count;
	  arg3 = i;
          strcpy(buf, o->short_description);
	  Zwrite(fp, cmd, 0, arg1, arg2, arg3, buf);
	  RecZwriteObj(fp, o);
      }
      /*
       *  lastly.. doors
       */
      
      for (j = 0; j < 6; j++) {
	/*
	 *  if there is an door type exit, write it.
	 */
	if (room->dir_option[j]) {  /* is a door */
	  if (room->dir_option[j]->exit_info) {
	    cmd = 'D';
	    arg1 = i ;
	    arg2 = j;
	    arg3 = 0;
	    if (IS_SET(room->dir_option[j]->exit_info, EX_CLOSED)) {
	      arg3 = 1;
	    }
	    if (IS_SET(room->dir_option[j]->exit_info, EX_LOCKED)) {
	      arg3 = 2;
	    }
	    Zwrite(fp, cmd, 0, arg1, arg2, arg3, room->name);
	  }
	}
      }
    }
  }
  fclose(fp);
}

void do_rload(struct char_data *ch, char *arg, int cmd)
{
  char filename[MAX_INPUT_LENGTH];
  FILE *f;
  short int id;
  int count;

  if (IS_NPC(ch)) return;
  if (GetMaxLevel(ch) < IMMORTAL) return;

  for (;isspace(*arg); arg++);

  sprintf(filename,"Area/%s",GET_NAME(ch));
  if (*arg && GetMaxLevel(ch)>=SILLYLORD) {
    sendf(ch,"Searching for '%s'\n",arg);
    sprintf(filename,"Area/%s",arg);
  }
  f=fopen(filename,"rb");
  if (!f) {
    sendf(ch,"Can't find your file.\n");
    return;
  }
  fread(&id,sizeof(id),1,f);
  if (id!=255) {
    sendf(ch,"Magic number not there... %d!=255\n",id);
  }
  count=0;
  while (read_one_room(f)) count++;
  fclose(f);
  sendf(ch,"%d rooms were loaded.\n",count);
}


void do_rsave(struct char_data *ch, char *arg, int cmd)
{
  FILE *f;
  int start,end,i;
  char filename[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  char *p;

  if (IS_NPC(ch)) return;
  if (GET_LEVEL(ch) < IMMORTAL) return;
  start = ch->build_lo;
  end = ch->build_hi;
  for (;isspace(*arg); arg++);
  sprintf(filename,"Area/%s",GET_NAME(ch));
  if (*arg && GET_LEVEL(ch)>=SILLYLORD) {
    strcpy(filename,"Area/");
    p=filename+strlen(filename);
    while (*arg && !isspace(*arg)) *(p++) = *(arg++);
    *p='\0';
    if (!*arg) {
      sendf(ch,"Usage: rsave Filename lo_number hi_number\n");
      return;
    }
    arg=one_argument(arg,buf);
    start=atoi(buf);
    arg=one_argument(arg,buf);
    end=atoi(buf);
    if (!*buf || end<start) {
      sendf(ch,"Usage: rsave Filename lo_number hi_number\n");
      return;
    }
  }
  if (end==0) {
    sendf(ch,"You don't have a range yet.\n");
    return;
  }
  sprintf(buf,"mv %s %s.bak",filename,filename);
  system(buf);
  f=fopen(filename,"w+b");
  if (!f) {
    vlog(LOG_URGENT,"Couldn't write to %s",filename);
    return;
  }
  for (i=start; i<=end; i++)
    bin_write_room(f,i);
  fclose(f);
  sendf(ch,"Ok.\n");
  if (GET_LEVEL(ch)>=SILLYLORD)
    sendf(ch,"%s was saved\n",filename);
}

void save_world(struct char_data *ch,char *arg,int cmd)
{
  FILE *f,*world;
  char buf[255];
  area_node *a;
  int i;

  world=fopen("WORLD/world","w+");
  if (!world) {
    vlog(LOG_URGENT,"WORLD/world could not be opened... bleah!");
    return;
  }
  fprintf(world,"#include <area.list>\n\n");
  for (a=area_list; a && a->next; a=a->next) {
    nlog("Saving %s (%d-%d)",a->name,a->offset,a->next->offset-1);
    sprintf(buf,"WORLD/%s",a->name);
    f=fopen(buf,"w+");
    if (!f) {
      nlog("Couldn't write to %s\n",buf);
      continue;
    }
    fprintf(world,"#offset %s\n#include <%s>\n\n",a->name,a->name);
    for (i=a->offset; i<a->next->offset; i++)
      write_room(f,a->offset,i);
    fclose(f);
  }
  fclose(world);
}

void do_at(struct char_data *ch, char *argument, int cmd)
{
  char command[MAX_INPUT_LENGTH], loc_str[MAX_INPUT_LENGTH], *p;
  int loc_nr, location, original_loc, cmd_no;
  struct char_data *target_mob;
  struct obj_data *target_obj;
  
  if (IS_NPC(ch))
    return;
  
  half_chop(argument, loc_str, command);
  if (!*loc_str)
    {
      send_to_char("You must supply a room number or a name.\n", ch);
      return;
    }
  
  
  if (isdigit(*loc_str)) {
      loc_nr = OFFSET(ch,atoi(loc_str));
      if (NULL==real_roomp(loc_nr)) {
	send_to_char("No room exists with that number.\n", ch);
	return;
      }
      location = loc_nr;
    } else if ((target_mob = get_char_vis(ch, loc_str))) {
      location = target_mob->in_room;
    } else if ((target_obj=get_obj_vis_world(ch, loc_str, NULL)))
      if (target_obj->in_room != NOWHERE)
	location = target_obj->in_room;
      else
	{
	  send_to_char("The object is not available.\n", ch);
	  return;
	}
    else
      {
	send_to_char("No such creature or object around.\n", ch);
	return;
      }
  
  /* a location has been found. */

  if ((GetMaxLevel(ch)<IMPLEMENTOR && ROOM_FLAG(location,NO_GOTO)) ||
      (GetMaxLevel(ch)<SILLYLORD   && ROOM_FLAG(location,LORD_ONLY))) {
    send_to_char("Can't go there, so you can't at there!\n",ch);
    return;
  }
  
  if (GET_LEVEL(ch)<SAINT && GET_LEVEL(ch)>MAX_MORT) {
    CHK_PLAYER_ZONE(ch,location);
  }
  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);
  p = command;
  cmd_no = command_interpreter(ch, &p);
  command_process(ch, cmd_no, p);

  /* check if the guy's still there */
  for (target_mob = real_roomp(location)->people; target_mob; target_mob =
       target_mob->next_in_room)
    if (ch == target_mob) {
      char_from_room(ch);
      char_to_room(ch, original_loc);
    }
}

void do_goto(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int loc_nr, location, i;
  struct char_data *target_mob, *pers, *v;
  struct obj_data *target_obj;
  struct room_data *rp;
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, buf);
  if (!*buf)	{
    send_to_char("You must supply a room number or a name.\n", ch);
    return;
  }
  
  if (isdigit(*buf) && NULL==index(buf, '.'))	{
    loc_nr = OFFSET(ch,atoi(buf));
    if (NULL==real_roomp(loc_nr)) {
      if (GetMaxLevel(ch)<REAL_IMMORT || loc_nr < 0) {
	send_to_char("No room exists with that number.\n", ch);
	return;
      } else {
	CHK_PLAYER_ZONE(ch,loc_nr);
	send_to_char("You form order out of chaos.\n", ch);
	allocate_room(loc_nr);
	rp = real_roomp(loc_nr);
	bzero(rp, sizeof(*rp));
	rp->number = loc_nr;
	if (top_of_zone_table >= 0) {
	  int	zone;
	  
	  for (zone=0;
	       rp->number > zone_table[zone].top && zone<=top_of_zone_table;
	       zone++)
	    ;
	  if (zone > top_of_zone_table) {
	    fprintf(stderr, "Room %d is outside of any zone.\n", rp->number);
	    zone--;
	  }
	  rp->zone = zone;
	}
	sprintf(buf, "%d", loc_nr);
	rp->name = (char *)mystrdup(buf);
	rp->description = (char *)mystrdup("Empty\n");
      }
    }
    location = loc_nr;
  }
  else if ((target_mob = get_char_vis_world(ch, buf, NULL)))
    location = target_mob->in_room;
  else if ((target_obj=get_obj_vis_world(ch, buf, NULL)))
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else   	{
      send_to_char("The object is not available.\n", ch);
      send_to_char("Try where #.object to nail its room number.\n", ch);
      return;
    }
  else	{
    send_to_char("No such creature or object around.\n", ch);
    return;
  }
  
  /* a location has been found. */
  
  
  if (!real_roomp(location)) {
    vlog(LOG_URGENT,"Massive error in do_goto. Everyone Off NOW.");
    return;
  }
  
  if ((GetMaxLevel(ch)<IMPLEMENTOR && ROOM_FLAG(location,NO_GOTO)) ||
      (GetMaxLevel(ch)<SILLYLORD   && ROOM_FLAG(location,LORD_ONLY))) {
	send_to_char("Sorry, but you aren't allowed there!\n",ch);
	return;
  }
  if (IS_SET(real_roomp(location)->room_flags, PRIVATE) && 
	GetMaxLevel(ch)<IMPLEMENTOR) {
    for (i = 0, pers = real_roomp(location)->people; pers; pers =
	 pers->next_in_room, i++);
    if (i > 1)
      {
	send_to_char(
	    "There's a private conversation going on in that room.\n", ch);
	return;
      }
  }
  
  if (IS_SET(ch->specials.act, PLR_STEALTH)) {
    for (v = real_roomp(ch->in_room)->people; v; v= v->next_in_room) {
      if ((ch != v) && (GetMaxLevel(v) >= IMPLEMENTOR)) {
	act("$n disappears in a puff of smoke.", FALSE, ch, 0, v, TO_VICT);
      }
    }
  } else {
    act("$n disappears in a puff of smoke.", FALSE, ch, 0, 0, TO_ROOM);
  }
  
  if (ch->specials.fighting)
    stop_fighting(ch);
  char_from_room(ch);
  char_to_room(ch, location);
  
  if (IS_SET(ch->specials.act, PLR_STEALTH)) {
    for (v = real_roomp(ch->in_room)->people; v; v= v->next_in_room) {
      if ((ch != v) && (GetMaxLevel(v) >= IMPLEMENTOR)) {
	act("$n appears with an ear-splitting bang.", FALSE, ch, 0,v,TO_VICT);
      }
    }
  } else {
    act("$n appears with an ear-splitting bang.", FALSE, ch, 0,0,TO_ROOM);
  }
  do_look(ch, "",15);
}


static void purge_one_room(int rnum, struct room_data *rp, int *range)
{
  struct char_data	*ch;
  struct obj_data	*obj;

  if (rnum==0 ||		/* purge the void?  I think not */
      rnum < range[0] || rnum > range[1])
    return;

  while (rp->people) {
    ch = rp->people;
    send_to_char("A god strikes the heavens making the ground around you erupt into a\n", ch);
    send_to_char("fluid fountain boiling into the ether.  All that's left is the Void.\n", ch);
    char_from_room(ch);
    char_to_room(ch, 0);	/* send character to the void */
    do_look(ch, "", 15);
    act("$n tumbles into the Void.", TRUE, ch, 0, 0, TO_ROOM);
  }

  while (rp->contents) {
    obj = rp->contents;
    obj_from_room(obj);
    obj_to_room(obj, 0);	/* send item to the void */
  }

  completely_cleanout_room(rp); /* clear out the pointers */
  
  hash_remove(&room_db, rnum);	/* remove it from the database */
}


/* clean a room of all mobiles and objects */
void do_purge(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;
  
  char name[100];
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, name);
  
  if (GetMaxLevel(ch)==IMPLEMENTOR && str_cmp(name,"allmob")==0) {
    for (vict=character_list; vict; vict=next_v) {
	next_v=vict->next;
	if (IS_MOB(vict)) {
		act("$n is smited by the hand of god.\n",TRUE,vict,0,0,TO_ROOM);
		extract_char(vict);
	}
    }
    sendf(ch,"So be it.\n");
    return;
  }
  if (GetMaxLevel(ch)==IMPLEMENTOR && str_cmp(name,"allobj")==0) {
    while (object_list) {
      extract_obj(object_list);
    }
    return;
  }
  if (*name) {  /* argument supplied. destroy single object or char */
      if ((vict = get_char_room_vis(ch, name)))	{
	if (!IS_NPC(vict) && (GetMaxLevel(ch)<IMPLEMENTOR)) {
	  send_to_char("I'm sorry, Dave.  I can't let you do that.\n", ch);
	  return;
	}
	
	act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);
	
	if (IS_NPC(vict)) {
	  extract_char(vict);
	} else {
	  if (vict->desc) {
	    strip_char(vict);
	    extract_char(vict);
	    close_socket(vict->desc);
	  } else {
	    save_char(vict,NOWHERE);
	    extract_char(vict);
	  }
	}
      } else if ((obj = get_obj_in_list_vis
		         (ch, name, real_roomp(ch->in_room)->contents))) {
	act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
	extract_obj(obj);
      } else	{
	argument = one_argument(argument, name);
	if (0==str_cmp("room", name)) {
	  int	range[2];
	  if (GetMaxLevel(ch)<IMPLEMENTOR && cmd!=-1) {
	    send_to_char("I'm sorry, Dave.  I can't let you do that.\n", ch);
	    return;
	  }
	  argument = one_argument(argument,name);
	  if (!isdigit(*name)) {
	    send_to_char("purge room start [end]",ch);
	    return;
	  }
	  range[0] = atoi(name);
	  argument = one_argument(argument,name);
	  if (isdigit(*name))
	    range[1] = atoi(name);
	  else
	    range[1] = range[0];
	  
	  if (range[0]==0 || range[1]==0) {
	    send_to_char("usage: purge room start [end]\n", ch);
	    return;
	  }
	  hash_iterate(&room_db, purge_one_room, range);
	} else {
	  send_to_char("I don't see that here.\n", ch);
	  return;
	}
      }
      
      send_to_char("Ok.\n", ch);
    }  else {   /* no argument. clean out the room */
      if (GetMaxLevel(ch) < DEMIGOD)
	return;
      if (IS_NPC(ch))	{
	send_to_char("You would only kill yourself..\n", ch);
	return;
      }
      
      act("$n gestures... You are surrounded by thousands of tiny scrubbing bubbles!", 
	  FALSE, ch, 0, 0, TO_ROOM);
      send_to_room("The world seems a little cleaner.\n", ch->in_room);

      for (vict = real_roomp(ch->in_room)->people; vict; vict = next_v) {
	next_v = vict->next_in_room;
	if (IS_NPC(vict)) {
	  extract_char(vict);
        }
      }
      
      for (obj = real_roomp(ch->in_room)->contents; obj; obj = next_o) {
	next_o = obj->next_content;
	extract_obj(obj);
      }
    }
}

static void print_room(int rnum, struct room_data *rp, struct string_block *sb)
{
  char	buf[MAX_STRING_LENGTH];
  int	dink,bits, scan;
  extern char *room_bits[];
  extern char *sector_types[];

  sprintf(buf, "%5d %4d %-12s %s", rp->number, rnum,
	  sector_types[rp->sector_type], rp->name);
  strcat(buf, " [");

  dink=0;
  for (bits=rp->room_flags, scan=0; bits; scan++) {
    if (bits & (1<<scan)) {
      if (dink)
	strcat(buf, " ");
      strcat(buf, room_bits[scan]);
      dink=1;
      bits ^= (1<<scan);
    }
  }
  strcat(buf, "]\n");

  append_to_string_block(sb, buf);
}

static void print_exits(int rnum, struct room_data *rp, struct string_block *sb)
{
  char	buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  register int i, count;
  int len = 0;

  sprintf(buf, "%5d ", rp->number); /*,rnum*/

  for (count=i=0; i<6; i++) {
    if (rp->dir_option[i]) {
      if (rp->dir_option[i]->to_room <= 0) continue;
      else if (!real_roomp(rp->dir_option[i]->to_room))
	sprintf(buf2, "%s %c-NONE", count++?",":"",dirs[i][0]);
      else if (IS_SET(rp->dir_option[i]->exit_info,EX_CLOSED))
	sprintf(buf2, "%s %c-%d*", count++?",":"",dirs[i][0],
		rp->dir_option[i]->to_room);
      else sprintf(buf2, "%s %c-%d", count++?",":"",dirs[i][0],
		   rp->dir_option[i]->to_room);
      len += strlen(buf2);
      strcat(buf, buf2);
    }
  }

  for (i = 0; i < 80; i++) buf2[i] = ' ';
  if (len < 38) buf2[38-len] = '\0';
  else buf2[2] = '\0';
  strcat(buf, buf2);
  strcpy(buf2, rp->name);
  if (strlen(buf2) > 28) buf2[28] = '\0';
  strcat(buf, buf2);

  if (IS_SET(rp->room_flags, NO_MOB)) strcat(buf," [NO_MOB]");
  strcat(buf, "\n");

  append_to_string_block(sb, buf);
}

static void print_death_room(int rnum, struct room_data *rp, struct string_block *sb)
{
  if (rp && rp->room_flags&DEATH)
    print_room(rnum, rp, sb);
}

static void print_death_exits(int rnum, struct room_data *rp, struct string_block *sb)
{
  if (rp && rp->room_flags&DEATH)
    print_exits(rnum, rp, sb);
}

static void print_private_room(int rnum, struct room_data *rp, struct string_block *sb)
{
return;
/*
  if (rp && rp->room_flags&PRIVATE)
    print_room(rnum, rp, sb);
*/
}

static void print_private_exits(int rnum, struct room_data *rp, struct string_block *sb)
{
return;
/*
  if (rp && rp->room_flags&PRIVATE)
    print_exits(rnum, rp, sb);
*/
}

struct show_room_zone_struct {
  int	blank;
  int	startblank, lastblank;
  int	bottom, top;
  struct string_block	*sb;
};

static void show_room_zone(int rnum, struct room_data *rp,
			   struct show_room_zone_struct *srzs)
{
  char buf[MAX_STRING_LENGTH];

  if (!rp || rp->number<srzs->bottom || rp->number>srzs->top)
    return; /* optimize later*/
  
  if (srzs->blank && (srzs->lastblank+1 != rp->number) ) {
    sprintf(buf, "rooms %d-%d are blank\n", srzs->startblank, srzs->lastblank);
    append_to_string_block(srzs->sb, buf);
    srzs->blank = 0;
  }

  if (1==sscanf(rp->name, "%d", &srzs->lastblank) &&
      srzs->lastblank==rp->number) {
    if (!srzs->blank) {
      srzs->startblank = srzs->lastblank;
      srzs->blank = 1;
    }
    return;
  } else if (srzs->blank) {
    sprintf(buf, "rooms %d-%d are blank\n", srzs->startblank, srzs->lastblank);
    append_to_string_block(srzs->sb, buf);
    srzs->blank = 0;
  }
  
  print_room(rnum, rp, srzs->sb);
}

static void show_exits_zone(int rnum, struct room_data *rp,
			   struct show_room_zone_struct *srzs)
{
  char buf[MAX_STRING_LENGTH];

  if (!rp || rp->number<srzs->bottom || rp->number>srzs->top)
    return; /* optimize later*/
  
  if (srzs->blank && (srzs->lastblank+1 != rp->number) ) {
    sprintf(buf, "rooms %d-%d are blank\n", srzs->startblank, srzs->lastblank);
    append_to_string_block(srzs->sb, buf);
    srzs->blank = 0;
  }

  if (1==sscanf(rp->name, "%d", &srzs->lastblank) &&
      srzs->lastblank==rp->number) {
    if (!srzs->blank) {
      srzs->startblank = srzs->lastblank;
      srzs->blank = 1;
    }
    return;
  } else if (srzs->blank) {
    sprintf(buf, "rooms %d-%d are blank\n", srzs->startblank, srzs->lastblank);
    append_to_string_block(srzs->sb, buf);
    srzs->blank = 0;
  }
  
  print_exits(rnum, rp, srzs->sb);
}

void do_zone(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int which_zone,reset_time;

  argument=one_argument(argument,buf);
  which_zone=atoi(buf);

  argument=one_argument(argument,buf);
  reset_time=atoi(buf);

  zone_table[which_zone].lifespan=reset_time;
}


static int range_lo, range_hi;
static struct hash_header *db;
static char search_name[MAX_STRING_LENGTH];
static void print_index_data(int vnum, struct index_mem *h, struct string_block *sb)
{
  char buf[MAX_STRING_LENGTH];

  if (vnum<range_lo || vnum>range_hi) return;
  if (db==&mob_db)
    sprintf(buf,"%7d %5d %s\n",vnum,h->count,h->mob->player.name);
  else
    sprintf(buf,"%7d %5d %s\n",vnum,h->count,h->obj->name);
  append_to_string_block(sb,buf);
}

static void print_index_data_name(int vnum, struct index_mem *h, struct string_block *sb)
{
  char buf[MAX_STRING_LENGTH];
  if (db==&mob_db) {
    if (strstr(h->mob->player.name,search_name)) {
      sprintf(buf,"%7d %5d %s\n",vnum,h->count,h->mob->player.name);
      append_to_string_block(sb,buf);
    }
  } else if (strstr(h->obj->name,search_name)) {
    sprintf(buf,"%7d %5d %s\n",vnum,h->count,h->obj->name);
    append_to_string_block(sb,buf);
  }
}

void do_show(struct char_data *ch, char *argument, int cmd)
{
  int	zone;
  char buf[MAX_STRING_LENGTH], zonenum[MAX_INPUT_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  struct string_block	sb;
  
  if (IS_NPC(ch))
    return;
  
  argument = one_argument(argument, buf);
  

  if (is_abbrev(buf,"shop")) {
    shop_data *s;
    int index;
    int i;

    index=atoi(argument);
    if (!index) {
      i=0;
      for (s=shop_index; s; s=s->next)
	sendf(ch,"%5d. Keeper: %7d Room: %d\n",s->index,s->keeper,s->in_room);
      return;
    }
    for (s=shop_index; s; s=s->next)
      if (index==s->index) break;
    if (!s) {
      sendf(ch,"Shop not found.\n");
      return;
    }
    sendf(ch,"Keeper: %d  Room: %d\n",s->keeper,s->in_room);
    sendf(ch,"Produces %d items: %s",s->n_producing,s->n_producing?"":"None");
    for (i=0; i<s->n_producing; i++)
      sendf(ch,"%s%d",i?", ":"",s->producing[i]);
    sendf(ch,"\n");
    sendf(ch,"Trades %d types: %s",s->n_type,s->n_type?"":"None");
    for (i=0; i<s->n_type; i++)
	sendf(ch,"%s%s",i?", ":"",item_types[s->type[i]]);
    sendf(ch,"\n");
    sendf(ch,"Buys %d items: %s",s->n_vnum,s->n_vnum?"":"None");
    for (i=0; i<s->n_vnum; i++)
      sendf(ch,"%s%d",i?", ":"",s->vnum[i]);
    sendf(ch,"\n");
    sendf(ch,"Hours:%s\n",s->n_hours?"":" Always open");
    for (i=0; i<s->n_hours; i++)
      sendf(ch,"  %d - %d\n",s->open[i],s->close[i]);
    return;
  }

  if(is_abbrev(buf, "events")){
    int i, any = 0;
    struct event_t *event;

    for(i = 0; i < (WAIT_SEC * SECS_PER_MUD_HOUR); i++){
      for(event = event_queue[i]; event; event = event->next){
	any = 1;
        switch(event->type){
	  case EVT_REMOVED:
	    sendf(ch, "[%3d] EVT_REMOVED\n", i);
	    break;
          case EVT_SPELL:
	    sendf(ch, "[%3d] EVT_SPELL <%s, ch: %s, args: %s>\n", i,
	      spells[event->virtual], GET_NAME(event->ch), *event->args ?
	      event->args : "(nil)");
            break;
          case EVT_STRIKE:
	    sendf(ch, "[%3d] EVT_STRIKE <ch: %s, victim: %s>\n", i,
	      GET_NAME(event->ch), event->ch->specials.fighting ?
	      GET_NAME(event->ch->specials.fighting) : "(nil)");
            break;
          case EVT_MOBILE:
            sendf(ch, "[%3d] EVT_MOBILE <ch: %s>\n", i, GET_NAME(event->ch));
            break;
          default:
	    sendf(ch, "[%3d] unknown event type\n", i);
	    break;
        }
      }
    }
    if(!any){
      sendf(ch, "No events pending.\n");
    }
    return;
  }

  if (is_abbrev(buf,"class")) {
	int class,i;
	class_entry *c;
    if (GET_LEVEL(ch)<IMPLEMENTOR) return;
    for (;argument && isspace(*argument); argument++);
    if (!argument || !*argument) {
	sendf(ch,"Show class abbrev\n");
	return;
    }
    class=which_class(ch,argument,1);
    if (class<0) return;
    c=classes+class;
    sendf(ch,"Index: %d  Name: %s\n",c->index,c->class_name);
    sendf(ch,"Min: %2d %2d %2d %2d %2d %2d %2d\n",
      c->min[0],c->min[1],c->min[2],c->min[3],c->min[4],c->min[5],c->min[6]);
    sendf(ch,"Max: %2d %2d %2d %2d %2d %2d %2d\n",
      c->max[0],c->max[1],c->max[2],c->max[3],c->max[4],c->max[5],c->max[6]);
    sendf(ch,"Races:");
    for (i=0; i<c->nraces; i++)
	sendf(ch,"%s ",race_list[c->races[i]]);
    sendf(ch,"\n");
    return;
  }

  init_string_block(&sb);
  if (is_abbrev(buf, "zones")) {
    struct zone_data	*zd;
    int	bottom=0;
    append_to_string_block(&sb, "# Zone   name                                lifespan age     rooms     reset\n");

    for(zone=0; zone<=top_of_zone_table; zone++) {
      char	*mode;

      zd = zone_table+zone;
      switch(zd->reset_mode) {
      case 0: mode = "never";      break;
      case 1: mode = "ifempty";    break;
      case 2: mode = "always";     break;
      default: mode = "!unknown!"; break;
      }
      sprintf(buf,"%4d %-40s %4dm %4dm %6d-%-6d %s\n", zone, zd->name,
	      zd->lifespan, zd->age, bottom, zd->top, mode);
      append_to_string_block(&sb, buf);
      bottom = zd->top+1;
    }
  } else if ((is_abbrev(buf, "objects") && (db=&obj_db))  ||
	     (is_abbrev(buf, "mobiles") && (db=&mob_db))) {
    
    only_argument(argument,search_name);
    argument=one_argument(argument,buf1);
    if (!*buf1) {
	sendf(ch,"show obj|mob lo_vnum hi_vnum\n");
	destroy_string_block(&sb);
	return;
    } else range_lo=atoi(buf1);
    append_to_string_block(&sb, "VNUM    count names\n");
    if (range_lo!=0 || isdigit(*buf1)) {
      argument=one_argument(argument,buf1);
      if (!*buf1) {
	  sendf(ch,"sho obj|mob lo_vnum hi_vnum\n");
	  destroy_string_block(&sb);
	  return;
      } else range_hi=atoi(buf1);
      hash_iterate(db,print_index_data,&sb);
    } else {
      sendf(ch,"Searching for '%s'\n",search_name);
      hash_iterate(db,print_index_data_name,&sb);
    }
  } else if (is_abbrev(buf, "rooms")) {
    only_argument(argument, zonenum);
    
    append_to_string_block(&sb, "VNUM  rnum type         name [BITS]\n");

    if (is_abbrev(zonenum, "death")) {
      hash_iterate(&room_db, print_death_room, &sb);
      
    } else if (is_abbrev(zonenum, "private")) {
      hash_iterate(&room_db, print_private_room, &sb);

    } else if (1!=sscanf(zonenum,"%i", &zone) ||
	       zone<0 || zone>top_of_zone_table) {
      append_to_string_block(&sb, "I need a zone number with this command\n");

    } else {
      struct show_room_zone_struct	srzs;
      
      srzs.bottom = zone ? (zone_table[zone-1].top+1) : 0;
      srzs.top = zone_table[zone].top;
      
      srzs.blank = 0;
      srzs.sb = &sb;
      hash_iterate(&room_db, show_room_zone, &srzs);
	
      if (srzs.blank) {
	sprintf(buf, "rooms %d-%d are blank\n", srzs.startblank,
		srzs.lastblank);
	append_to_string_block(&sb, buf);
	srzs.blank = 0;
      }
    }
  } else if (is_abbrev(buf, "exits")) {
    only_argument(argument, zonenum);
    
    sprintf(buf1, " VNUM  exits%32sname [BITS]\n"," ");
    append_to_string_block(&sb, buf1);

    if (is_abbrev(zonenum, "death")) {
      hash_iterate(&room_db, print_death_exits, &sb);
      
    } else if (is_abbrev(zonenum, "private")) {
      hash_iterate(&room_db, print_private_exits, &sb);

    } else if (1!=sscanf(zonenum,"%i", &zone) ||
	       zone<0 || zone>top_of_zone_table) {
      append_to_string_block(&sb, "I need a zone number with this command\n");

    } else {
      struct show_room_zone_struct	srzs;
      
      srzs.bottom = zone ? (zone_table[zone-1].top+1) : 0;
      srzs.top = zone_table[zone].top;
      
      srzs.blank = 0;
      srzs.sb = &sb;
      hash_iterate(&room_db, show_exits_zone, &srzs);
	
      if (srzs.blank) {
	sprintf(buf, "rooms %d-%d are blank\n", srzs.startblank,
		srzs.lastblank);
	append_to_string_block(&sb, buf);
	srzs.blank = 0;
      }
    }
  } else {
    append_to_string_block(&sb,"Usage:\nshow zones\nshow (objects|mobiles) (zone#|name)\nshow rooms (zone#|death|private)\nshow exits (zone#|death|private)\nshow events\n");
  }
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
}

void do_offset(struct char_data *ch, char *arg, int cmd)
{
  ch->build.offset=atoi(arg);
  sendf(ch,"Building offset set to %d\n",ch->build.offset);
}

void do_foreach(struct char_data *ch,char *arg, int cmd)
{
  int command;
  struct char_data *c;
  char buf[MAX_STRING_LENGTH], *p;
  for (;isspace(*arg);arg++);
  for (c=character_list; c; c=c->next)
    if (c!=ch && !IS_NPC(c)) {
	sprintf(buf,arg,GET_NAME(c));
	p = buf;
	command = command_interpreter(ch,&p);
	command_process(ch, command, p);
    }
}

void bin_write_room(FILE *fp,int virtual)
{
  short int t;
  u8 u8;
  int i;
  struct room_data *rp;

  rp=real_roomp(virtual);
  if (!rp) return;

  ID(255);
  WriteF(virtual);
  Write(1,rp->sector_type);
  if (rp->river_speed>0 && rp->river_dir<6) {
    ID(2);
    WriteF(rp->river_dir);
    WriteF(rp->river_speed);
  }
  if (rp->tele_time>0 && rp->tele_targ>0 && real_roomp(rp->tele_targ)) {
    ID(3);
    WriteF(rp->tele_time);
    WriteF(rp->tele_look);
    WriteF(rp->tele_targ);
  }
  WriteS(4,rp->name);
  WriteS(5,rp->description);
  if (rp->ex_description) {
    struct extra_descr_data *ex;

    ID(6);
    for (ex=rp->ex_description; ex; ex=ex->next) {
      if (!ex->keyword || !ex->description) continue;
      WriteS(1,ex->keyword);
      WriteS(2,ex->description);
    }
    ID(254);
  }
  for (i=0; i<6; i++)
    if (rp->dir_option[i]) break;
  if (i<6) {
    struct room_direction_data *d;
    ID(7);
    for (; i<6; i++) {
      d=rp->dir_option[i];
      if (!d) continue;
      ID(1);
      u8=i;
      WriteF(u8);
      WriteF(d->to_room);
      if (d->general_description) WriteS(2,d->general_description);
      if (d->keyword) WriteS(3,d->keyword);
      if (d->exit_info) Write(4,d->exit_info);
      if (d->key>0) Write(5,d->key);
    }
    ID(254);
  }
  Write(9,rp->room_flags);
}

void do_oldload(struct char_data *ch, char *arg, int cmd)
{
  char buf[255];
  short int id;
  FILE *f;

  sprintf(buf,"Area/%s",GET_NAME(ch));
  f=fopen(buf,"rb");
  if (!f) {
    sendf(ch,"You have no old area.\n");
    return;
  }
  fread(&id,sizeof(id),1,f);
  fclose(f);
  if (id==255) {
    sendf(ch,"You have already saved in the new format.\n");
    return;
  }
  RoomLoad(ch,-1,-1,GET_NAME(ch));
}
