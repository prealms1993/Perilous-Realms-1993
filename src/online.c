/* ************************************************************************
*  file: online.c                                         Part of DIKUMUD *
*  Usage: online building						  *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************ */


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include <fcns.h>
#include <utils.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <comm.h>
#include <externs.h>
#include <spells.h>
#include <trap.h>
#include <limits.h>

extern struct zone_data *zone_table;
extern int top_of_zone_table;
extern int opposite_dir[];
extern char *spells[];
#define NCH(s) (s?s:"")


char *room_fields[] =
{
	"name",   /* 1 */
	"desc",
	"fs",
	"exit",
	"exdsc",  /* 5 */
        "extra",  /* 6 */
	"riv",    /* 7 */
	"tele", /* 8 */
	"del",
	"bexit",
	"addflag",
	"delflag",
	"\n"
};

/* maximum length for text field x+1 */
int length[] =
{
	15,
	60,
	256,
	240,
	60
};


int room_length[] =
{
	80,
	4096,
        50,
	50,
	512,
	512,
        50,
	100
};

void confirm_exit(struct char_data *ch)
{
}

void do_edit(struct char_data *ch, char *arg, int cmd)
{
  char c;
  static char dir_char[]="neswud";
  int field, dflags, dir, exroom, dkey, rspeed, rdir,
  tele_room, tele_time, tele_look;
  unsigned r_flags;
  int s_type;
  int x;
  char string[512], buf[MAX_STRING_LENGTH];
  char *a;
  struct extra_descr_data *ed, *tmp;
  struct room_data	*rp;
  
  CHK_PLAYER_ZONE(ch,ch->in_room);
  rp = real_roomp(ch->in_room);
  
  if ((IS_NPC(ch)) || (GetMaxLevel(ch)<LOW_IMMORTAL))
    return;
  
  bisect_arg(arg, &field, string);
  
  if (!field)	{
    send_to_char("No field by that name. Try 'help edit'.\n", ch);
    return;
  }
  
  r_flags = -1;
  s_type = -1;
  
  switch(field) {
    
  case 1: ch->desc->str = &rp->name; break;
  case 2: ch->desc->str = &rp->description; break;
  case 3: sscanf(string,"%u %d ",&r_flags,&s_type);
    if ((s_type < 0) || (s_type > MAX_SECTORS)) {
      send_to_char("didn't quite get those, please try again.\n",ch);
      sendf (ch, "flags must be 0 or positive, and sectors must be from 0 to %d\n",MAX_SECTORS);
      send_to_char("edit fs <flags> <sector_type>\n",ch);
      return;
    }
    rp->room_flags = r_flags;
    rp->sector_type = s_type;
    
    if (rp->sector_type == SECT_WATER_NOSWIM) {
      send_to_char("P.S. you need to do speed and flow\n",ch);
      send_to_char("For this river. (set to 0 as default)\n",ch);
      rp->river_speed = 0;
      rp->river_dir = 0;
      return;
    }
    return;
    break;
  case 10:
  case 4:
    a=one_argument(string,buf); c=buf[0];
    a=one_argument(a,buf); dflags=atoi(buf);
    a=one_argument(a,buf); dkey=atoi(buf);
    a=one_argument(a,buf); exroom=OFFSET(ch,atoi(buf));
    if (!*buf) {
      sendf(ch,"You forgot something\n");
      return;
    }

    for (; a && *a && isspace(*a); a++);
    if ((!a || !*a) && field==10 && dflags>0) {
      sendf(ch,"If you are using bexit, and give a door flag, you have to also enter\n"
	   "the extra description keywords.\n");
      return;
    }
      /*sscanf(string,"%c %d %d %d ", &c, &dflags, &dkey, &exroom);*/
    /* check if the exit exists */
    if (c>='0' && c<='5') dir=c-'0';
    else {
      int i;
      for (i=0; i<6; i++) if (c==dir_char[i]) dir=i;
    }
    if ((dir < 0) || (dir > 5)) {
      send_to_char("You need to use n, s, e, w, u, d or a number from 0 to 5\n",ch);
      return;
    }
    /*CHK_PLAYER_ZONE(ch,exroom);*/
    
    if (rp->dir_option[dir]) {
      send_to_char("modifying exit\n",ch);
      if (dflags == 1) rp->dir_option[dir]->exit_info = DOOR_1;
      else if (dflags == 2) rp->dir_option[dir]->exit_info = DOOR_2;
      else if (dflags == 3) rp->dir_option[dir]->exit_info = DOOR_3;
      else if (dflags == 4) rp->dir_option[dir]->exit_info = DOOR_4;
      else if (dflags == 5) rp->dir_option[dir]->exit_info = DOOR_5;
      else if (dflags == 6) rp->dir_option[dir]->exit_info = DOOR_6;
      else if (dflags == 7) rp->dir_option[dir]->exit_info = DOOR_7;
      else
	rp->dir_option[dir]->exit_info = 0;
      
      rp->dir_option[dir]->key = dkey;
      if (real_roomp(exroom) != NULL) {
	rp->dir_option[dir]->to_room = exroom;
      } else {
	send_to_char("Deleting exit.\n",ch);
	FREE(rp->dir_option[dir]);
	rp->dir_option[dir] = 0;
	return;
      }
    } else if (real_roomp(exroom)==NULL) {
      send_to_char("You can only connect an exit to a room that already is there.\n",ch);
      return;
    } else {
      send_to_char("New exit\n",ch);
      CREATE(rp->dir_option[dir], 
	     struct room_direction_data, 1);
      if (dflags == 1) rp->dir_option[dir]->exit_info = DOOR_1;
      else if (dflags == 2) rp->dir_option[dir]->exit_info = DOOR_2;
      else if (dflags == 3) rp->dir_option[dir]->exit_info = DOOR_3;
      else if (dflags == 4) rp->dir_option[dir]->exit_info = DOOR_4;
      else if (dflags == 5) rp->dir_option[dir]->exit_info = DOOR_5;
      else if (dflags == 6) rp->dir_option[dir]->exit_info = DOOR_6;
      else if (dflags == 7) rp->dir_option[dir]->exit_info = DOOR_7;
      else
	rp->dir_option[dir]->exit_info = 0;
      rp->dir_option[dir]->key = dkey;
      rp->dir_option[dir]->to_room = exroom;
    }
    if (field==10) {
      struct room_data *rp;
      int odir;

      CHK_PLAYER_ZONE(ch,exroom);
      rp=real_roomp(exroom);
      odir=opposite_dir[dir];
      if (rp->dir_option[odir]) {
        sendf(ch,"An exit in that direction already exits there.\n");
	return;
      }
      CREATE(rp->dir_option[odir], struct room_direction_data, 1);
      if (dflags == 1) rp->dir_option[odir]->exit_info = DOOR_1;
      else if (dflags == 2) rp->dir_option[odir]->exit_info = DOOR_2;
      else if (dflags == 3) rp->dir_option[odir]->exit_info = DOOR_3;
      else if (dflags == 4) rp->dir_option[odir]->exit_info = DOOR_4;
      else if (dflags == 5) rp->dir_option[odir]->exit_info = DOOR_5;
      else if (dflags == 6) rp->dir_option[odir]->exit_info = DOOR_6;
      else if (dflags == 7) rp->dir_option[odir]->exit_info = DOOR_7;
      else
	rp->dir_option[odir]->exit_info = 0;
      rp->dir_option[odir]->key = dkey;
      rp->dir_option[odir]->to_room = ch->in_room;
      if (rp->dir_option[odir]->exit_info>0)
        rp->dir_option[odir]->keyword=mystrdup(a);
    }

    
    if (rp->dir_option[dir]->exit_info>0) {
      string[0] = 0;
      if (!a || !*a) {
	send_to_char("enter keywords, 1 line only. \n",ch);
	send_to_char("terminate with an @ on the same line.\n",ch);
	ch->desc->str = &rp->dir_option[dir]->keyword; 
	break;
      } else
        rp->dir_option[dir]->keyword=mystrdup(a);
      return;
    } else return;
    
  case 5: dir = -1;
    sscanf(string,"%d", &dir); 
    if ((dir >=0) && (dir <= 5)) {
      send_to_char("Enter text, term. with '@' on a blank line",ch);
      string[0] = 0;
      if (rp->dir_option[dir]) {
	ch->desc->str = &rp->dir_option[dir]->general_description;
      } else {
	CREATE(rp->dir_option[dir], 
	       struct room_direction_data, 1);
	ch->desc->str = &rp->dir_option[dir]->general_description;
      }
    } else {
      send_to_char("Illegal direction\n",ch);
      send_to_char("Must enter 0-5.I will ask for text.\n",ch);
      return;		  
    }
    break;
  case 6: 
    /* 
      extra descriptions 
      */
    if (!*string)  	{
      send_to_char("You have to supply a keyword.\n", ch);
      return;
    }
    /* try to locate extra description */
    for (ed = rp->ex_description; ; ed = ed->next)
      if (!ed) {
	CREATE(ed , struct extra_descr_data, 1);
	ed->next = rp->ex_description;
	rp->ex_description = ed;
	CREATE(ed->keyword, char, strlen(string) + 1);
	strcpy(ed->keyword, string);
	ed->description = 0;
	ch->desc->str = &ed->description;
	send_to_char("New field.\n", ch);
	break;
      }  else if (!str_cmp(ed->keyword, string)) {
	/* the field exists */
	FREE(ed->description);
	ed->description = 0;
	ch->desc->str = &ed->description;
	send_to_char( "Modifying description.\n", ch);
	break;
      }
    ch->desc->max_str = MAX_STRING_LENGTH;
    return; 
    break;
    
  case 7:
    /*  this is where the river stuff will go */
    rspeed = 0; rdir = 0;
    sscanf(string,"%d %d ",&rspeed,&rdir);
    if ((rdir>= 0) && (rdir <= 5)) {
      rp->river_speed = rspeed;
      rp->river_dir = rdir;
    } else {
      send_to_char("Illegal dir. : edit riv <speed> <dir>\n",ch);
    }
    return;

  case 8:
    /*  this is where the teleport stuff will go */
    tele_room = -1; tele_time = -1; tele_look = -1;
    sscanf(string,"%d %d %d",&tele_time,&tele_room,&tele_look);
    tele_room=OFFSET(ch,tele_room);
    if (tele_room < 0 || tele_time < 0 || tele_look < 0) {
      send_to_char(" edit tele <time> <room_nr> <look-flag>\n", ch);
      return;
      break;
    } else {
	real_roomp(ch->in_room)->tele_time = tele_time;
	real_roomp(ch->in_room)->tele_targ = tele_room;
	real_roomp(ch->in_room)->tele_look = tele_look;
        return;
	break;
    }
    
  case 9: 
    /*
      deletion
      */
      if (!*string)  	{
	send_to_char("You must supply a field name.\n", ch);
	return;
      }
      /* try to locate field */
      for (ed = rp->ex_description; ; ed = ed->next)
	if (!ed) {
	  send_to_char("No field with that keyword.\n", ch);
	  return;
	} else if (!str_cmp(ed->keyword, string)) {
	  FREE(ed->keyword);
	  FREE(ed->description);
	  
	  /* delete the entry in the desr list */						
	  if (ed == rp->ex_description)
	    rp->ex_description = ed->next;
	  else {
	    for(tmp = rp->ex_description; tmp->next != ed; 
		tmp = tmp->next);
	    tmp->next = ed->next;
	  }
	  FREE(ed);
	  
	  send_to_char("Field deleted.\n", ch);
	  return;
	}
      break;				
  case 11:
   rp->room_flags |= atoi(string);
   return;
   break;
  case 12:
   x=atoi(string);
   if (rp->room_flags & x == x) rp->room_flags -=x;
   return;
   break;
    
  default:
    send_to_char("I'm so confused :-)\n",ch);
    return;
    break;
  }
  
  FREE(*ch->desc->str);
  
  if (*string) {   /* there was a string in the argument array */ 
    if (strlen(string) > room_length[field - 1])	{
      char *tmp;
      send_to_char("String too long - truncated.\n", ch);
/*      *(string + length[field - 1]) = '\0';*/
      tmp = string + length[field - 1];
      *tmp = '\0';
    }
    CREATE(*ch->desc->str, char, strlen(string) + 1);
    strcpy(*ch->desc->str, string);
    ch->desc->str = 0;
    send_to_char("Ok.\n", ch);
  } else {  /* there was no string. enter string mode */
    send_to_char("Enter string. terminate with '@'.\n", ch);
    *ch->desc->str = 0;
    ch->desc->max_str = room_length[field - 1];
  }
  
}

typedef struct obj_field_data {
	char *fieldname;
	int type; /* 0 = numeric, 1=string 2=affect 3=extra*/
	slong min,max;
} obj_field_data;

const struct obj_field_data obj_fields[] = {
/*	field name            type    min       max */
{	"value1", /* 0 */	0,	0,	255 },
{	"value2", /* 1 */	0,	0,	255 },
{	"value3", /* 2 */	0,	0,	255 },
{	"value4", /* 3 */	0,	0,	255 },
{	"type",   /* 4 */	0,	1,	25 },
{	"wear",   /* 5 */	0,	0,	65535 },
{	"xflag",  /* 6 */	0,	0,	65535 },
{	"weight", /* 7 */	0,	0,	4000000 },
{	"cost",   /* 8 */	0,	0,	1000000 },
{	"rent",   /* 9 */	0,	0,	1000000 },
{	"timer",  /* 10 */	0,	0,	7000000 },
{	"bits",   /* 11 */	0,	0,	MAX_SLONG },
{	"aff1",   /* 12 */	2,      0,      0 },
{	"aff2",   /* 13 */	2,	0,	0 },
{	"aff3",   /* 14 */	2,	0,	0 },
{	"aff4",   /* 15 */	2,	0,	0 },
{	"aff5",   /* 16 */	2,	0,	0 },
{	"name",   /* 17 */	1,	1,	80 },
{	"long",   /* 18 */	1,	1,	255 },
{	"short",  /* 19 */	1,	1,	160 },
{	"action", /* 20 */	1,	1,	80 },
{	"extra",  /* 21 */	3,	0,	0 },
{	"" ,	-1,	0,	0 }
};

const struct obj_field_data mob_fields[]= {
{	"name",    /* 0 */	1,	1,	80 },
{	"short",   /* 1 */	1,	1,	80 },
{	"long",    /* 2 */	1,	1,	80 },
{	"descr",   /* 3 */	1,	1,	80 },
{	"act",	   /* 4 */	0,	0,	MAX_SLONG },
{	"affected",/* 5 */	0,	0,	MAX_SLONG },
{	"align",   /* 6 */	0,  -1000,	1000 },
{	"level",   /* 7 */	0,	1,	ABS_MAX_LVL },
{	"tohit",   /* 8 */	0,	0,	20 },
{	"armor",   /* 9 */	0,    -20,	20 },
{	"hp",	   /* 10 */	4,	0,	0 },
{	"damage",  /* 11 */	4,	0,	0 },
{	"gold",    /* 12 */	0,	0,	10000000 },
{	"exp",     /* 13 */	0,	0,	1000000 },
{	"race",    /* 14 */	0,	0,	30 },
{	"pos",     /* 15 */	0,	0,	POSITION_STANDING },
{	"dpos",    /* 16 */	0,	0,	POSITION_STANDING },
{	"sex",     /* 17 */	0,	0,	3 },
{	"immune",  /* 18 */	0,	0,	MAX_SLONG },
{	"mimmune", /* 19 */	0,	0,	MAX_SLONG },
{	"susc",    /* 20 */	0,	0,	MAX_SLONG },
{	"sound",   /* 21 */	1,	1,	80 },
{	"distant", /* 22 */	1,	1,	80 },
{	"str",     /* 23 */	0,	1,	25 },
{	"astr",    /* 24 */	0,	0,	100 },
{	"int",     /* 25 */ 	0,	1,	25 },
{	"wis",     /* 26 */	0,	1,	25 },
{	"dex",     /* 27 */	0,	1,	25 },
{	"con",     /* 28 */	0,	1,	25 },
{	"height",  /* 29 */	0,	0,	255 },
{	"weight",  /* 30 */	0,	0,	255 },
{	"class",   /* 40 */     1,      1,      MAX_SLONG },
{	"", -1, 0, 0 }
};



static int obj_search_field(char *arg,int length,const struct obj_field_data obj_fields[])
{
  int i;

  i=0;
  while (*obj_fields[i].fieldname)
    if (is_abbrev(arg,obj_fields[i].fieldname)) return(i);
    else i++;
  return(-1);
}

void do_oedit(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  struct extra_descr_data *ex;
  int field;
  slong v1,v2;
  struct obj_data *o;

  if (IS_NPC(ch)) return;
  arg=one_argument(arg,buf);
  o=ch->build.obj_build;
  if (!o) {
	send_to_char("Use ocreate first.\n",ch);
	return;
  }

  if (!*buf) {
	int i;
	i=0;
	send_to_char("Legal fields:\n",ch);
	while (*obj_fields[i].fieldname) {
	  sprintf(buf,"%-19s",obj_fields[i++].fieldname);
	  send_to_char(buf,ch);
	  if (i%4==0) send_to_char("\n",ch);
	}
	return;
  }
 
  if ((field=obj_search_field(buf,strlen(buf),obj_fields))<0) {
	int i;

	i=0;
	send_to_char("Unknown field\n",ch);
	send_to_char("Legal fields:\n",ch);
	while (*obj_fields[i].fieldname) {
	  sprintf(buf,"%-19s",obj_fields[i++].fieldname);
	  send_to_char(buf,ch);
	  if (i%4==0) send_to_char("\n",ch);
	}
	return;
  }

  while (*arg && isspace(*arg)) arg++;

  switch (obj_fields[field].type) {
    case 0:
	arg=one_argument(arg,buf);
	if (!*buf || !isdigit(*buf)) {
		send_to_char("expecting number\n",ch);
		return;
	}
	v1=atol(buf);
	if (v1<obj_fields[field].min) {
		sprintf(buf,"Minimum value is %ld.\n",obj_fields[field].min);
		send_to_char(buf,ch);
		return;
	}
	if (v1>obj_fields[field].max) {
		sprintf(buf,"Maximum value is %ld.\n",obj_fields[field].max);
		send_to_char(buf,ch);
		return;
	}
	switch (field) {
	  case 0: case 1: case 2: case 3:
		o->obj_flags.value[field]=v1;
		break;
	  case 4: o->obj_flags.type_flag=v1; break;
	  case 5: o->obj_flags.wear_flags=v1; break;
	  case 6: o->obj_flags.extra_flags=v1; break;
	  case 7: o->obj_flags.weight=v1; break;
	  case 8: o->obj_flags.cost=v1; break;
	  case 9: o->obj_flags.cost_per_day=v1; break;
	  case 10: o->obj_flags.timer=v1; break;
	  case 11: break;
	  default:
		send_to_char("Error! please report immediately\n",ch);
	}
	break;
    case 1:
	switch (field) {
	  case 17: ch->desc->str= &o->name; break;
	  case 18: ch->desc->str= &o->description; break;
	  case 19: ch->desc->str= &o->short_description; break;
	  case 20: ch->desc->str= &o->action_description; break;
	  default: send_to_char("Error! please report immediately\n",ch);
		  return;
	}
	if (*arg) {
		strcpy(buf,arg);
		buf[obj_fields[field].max]='\0';
    		CREATE(*ch->desc->str, char, strlen(buf) + 1);
		strcpy(*ch->desc->str,buf);
		ch->desc->str=NULL;
		if (strlen(buf)>obj_fields[field].max)
			send_to_char("Too long, trucated.\n",ch);
	} else {
		ch->desc->max_str=obj_fields[field].max;
		*ch->desc->str='\0';
	}
        break;
    case 2:
	if (!*arg) {
		send_to_char("Expecting Location and Modifier\n",ch);
		return;
	}
	sscanf(arg,"%ld %ld",&v1,&v2);
	o->affected[field-12].location=v1;
	o->affected[field-12].modifier=v2;
    break;
    case 3:
	if (!*arg) {
		send_to_char("Need the extra description key words\n",ch);
		return;
	}
	CREATE(ex,struct extra_descr_data,1);
	strcpy(buf,arg);
	buf[80]='\0';
	CREATE(ex->keyword,char,strlen(buf)+1);
	strcpy(ex->keyword,buf);
	ex->next=o->ex_description;
	o->ex_description=ex;
	ex->description=NULL;
	ch->desc->str=&ex->description;
	ch->desc->max_str=255;
  }
  send_to_char("Ok.\n",ch);
}

void do_osave(struct char_data *ch, char *arg, int cmd)
{
  char filename[MAX_STRING_LENGTH];
  struct obj_data *o;
  int i;
  struct extra_descr_data *xtra;
  FILE *f;

  o=ch->build.obj_build;
  if (!o) {
	send_to_char("You aren't working on an object!\n",ch);
	return;
  }
  sprintf(filename,"Object/%ld",o->virtual);
  if ((f=fopen(filename,"w+"))==NULL) {
	send_to_char("Uh-Oh... can't save!\n",ch);
	log(filename);
	return;
  }
  fprintf(f,"#%ld\n%s~\n%s~\n%s~\n%s~\n",
	o->virtual,
	NCH(o->name),
	NCH(o->short_description),
	NCH(o->description),
	NCH(o->action_description));
  fprintf(f,"%ld %lu %lu\n%ld %ld %ld %ld\n%d %d %d\n",
	o->obj_flags.type_flag,
	o->obj_flags.extra_flags,
	o->obj_flags.wear_flags,
	o->obj_flags.value[0],
	o->obj_flags.value[1],
	o->obj_flags.value[2],
	o->obj_flags.value[3],
	o->obj_flags.weight,
	o->obj_flags.cost,
	o->obj_flags.cost_per_day);
  if (o->ex_description)
    for (xtra=o->ex_description; xtra; xtra=xtra->next)
	fprintf(f,"E\n%s~\n%s~\n",
	  xtra->keyword,
	  xtra->description);
  for(i=0; i<MAX_OBJ_AFFECT; i++)
	if (o->affected[i].location)
		fprintf(f,"A\n%d %d\n",
		  o->affected[i].location,
		  o->affected[i].modifier);
  fclose(f);
  send_to_char("Ok.\n",ch);
}

char *one_number(char *arg,slong *tmp)
{
  char buf[100];
  char *p;
  p=one_argument(arg,buf);
  *tmp=atol(buf);
  return(p);
}

void read_one_object(FILE *obj_f,struct obj_data *obj)
{
  slong tmp,ok;
  struct extra_descr_data *desc;
  int i;
  char chk[MAX_STRING_LENGTH];
  char *p;

  fscanf(obj_f,"#%ld\n",&obj->virtual);
  obj->name = fread_string(obj_f);
  obj->short_description = fread_string(obj_f);
  obj->description = fread_string(obj_f);
  obj->action_description = fread_string(obj_f);

  /* *** numeric data *** */

  ok=fgets(chk,100,obj_f)!=NULL;
  p=one_number(chk,&tmp); obj->obj_flags.type_flag = tmp;
  p=one_number(p,&tmp);   obj->obj_flags.extra_flags = tmp;
  p=one_number(p,&tmp);   obj->obj_flags.wear_flags = tmp;

  ok=fgets(chk,100,obj_f)!=NULL;
  p=one_number(chk,&tmp); obj->obj_flags.value[0] = tmp;
  p=one_number(p,&tmp);   obj->obj_flags.value[1] = tmp;
  p=one_number(p,&tmp);   obj->obj_flags.value[2] = tmp;
  p=one_number(p,&tmp);   obj->obj_flags.value[3] = tmp;

  ok=fgets(chk,100,obj_f)!=NULL;
  p=one_number(chk,&tmp); obj->obj_flags.weight = tmp;
  p=one_number(p,&tmp);   obj->obj_flags.cost = tmp;
  p=one_number(p,&tmp);   obj->obj_flags.cost_per_day = tmp;

  /* *** extra descriptions *** */

  obj->ex_description = 0;

  ok=fgets(chk,100,obj_f)!=NULL;
  while (ok && *chk == 'E')     {
    CREATE(desc, struct extra_descr_data, 1);

    desc->keyword = fread_string(obj_f);
    desc->description = fread_string(obj_f);

    desc->next = obj->ex_description;
    obj->ex_description = desc;
    ok=fgets(chk,100,obj_f)!=NULL;
  }

  i=0;
  while (ok && i<MAX_OBJ_AFFECT && *chk=='A') {
    fscanf(obj_f, " %ld ", &tmp);   obj->affected[i].location = tmp;
    fscanf(obj_f, " %ld \n", &tmp); obj->affected[i++].modifier = tmp;
    ok=fgets(chk,100,obj_f)!=NULL;
  }

  for (;(i < MAX_OBJ_AFFECT);i++)       {
    obj->affected[i].location = APPLY_NONE;
    obj->affected[i].modifier = 0;
  }

  obj->in_room = NOWHERE;
  obj->next_content = 0;
  obj->carried_by = 0;
  obj->equipped_by = 0;
  obj->in_obj = 0;
  obj->contains = 0;

}

void do_oload(struct char_data *ch, char *arg, int cmd)
{
  FILE *f;
  struct obj_data *obj;
  extern struct obj_data *object_list;
  slong vnum;

  obj=ch->build.obj_build;

  if (*arg) {
	char buf[100];

	vnum=atoi(arg);
	CHK_PLAYER_ZONE(ch,vnum);

	sprintf(buf,"Loading object #%ld\n",vnum);
	send_to_char(buf,ch);
        sprintf(buf,"Object/%ld",vnum);
        if ((f=fopen(buf,"r"))!=NULL) {
            CREATE(obj,struct obj_data,1);
	    bzero(obj,sizeof(struct obj_data));
            clear_object(obj);
            read_one_object(f,obj);
	    obj->obj_flags.wear_flags |= ITEM_CONSTRUCTION;
            fclose(f);
            send_to_char("Loaded.\n",ch);
	} else {
	    send_to_char("Object doesn't exist.\n",ch);
	    return;
	}
  }
  if (!obj) {
	send_to_char("Nothing in your workspace.  Either specify a vnum or"
			" use ocreate.\nMake sure you osave first!\n",ch);
	return;
  }
  obj->next=object_list;
  object_list=obj;
  if (obj==ch->build.obj_build) ch->build.obj_build=NULL;
  obj_to_char(obj,ch);
  send_to_char("Done.\n",ch);
}


void do_ocreate(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  slong vnum;

  if (IS_NPC(ch)) return;

  arg=one_argument(arg,buf);
  if (!*buf) {
	send_to_char("Syntax: ocreate new|old|one vnum [copy_vnum]\n",ch);
	return;
  }
  if (is_abbrev(buf,"one") && GetMaxLevel(ch)<SILLYLORD) {
	send_to_char("Thou art not godly enough.\n",ch);
	return;
  }
  if (!is_abbrev(buf,"new") && !is_abbrev(buf,"old") && !is_abbrev(buf,"one")) {
	send_to_char("Type ocreate without any parameters for help.\n",ch);
	return;
  }
  arg=one_argument(arg,buf2);
  if (!*buf2) {
	send_to_char("Need vnum.\n",ch);
	return;
  }
  vnum=atol(buf2);
  CHK_PLAYER_ZONE(ch,vnum);
  if (!is_abbrev(buf,"one")) {
    if (!real_objp(vnum)) {
	if (is_abbrev(buf,"new")) {
	  CREATE(ch->build.obj_build,struct obj_data,1);
	  bzero(ch->build.obj_build,sizeof(struct obj_data));
	  clear_object(ch->build.obj_build);
	  ch->build.obj_build->virtual=vnum;
	  ch->desc->connected=CON_OBJECT;
	  ch->build.state=1;
	  send_to_char("You can type QUIT at any prompt to abort.\n",ch);
	  send_to_char("At most prompts, you can type ? for help.\n\n",ch);
	  send_to_char("Name list: ",ch);
	  /* send_to_char("Ok.  Now use oedit.\n",ch); */
	} else {
	  FILE *f;

	  sprintf(buf,"Object/%ld",vnum);
	  if ((f=fopen(buf,"r"))!=NULL) {
	    CREATE(ch->build.obj_build,struct obj_data,1);
	    bzero(ch->build.obj_build,sizeof(struct obj_data));
	    clear_object(ch->build.obj_build);
	    read_one_object(f,ch->build.obj_build);
	    fclose(f);
	    send_to_char("Ok.  Now use oedit\n",ch);
	  } else {
	    send_to_char("Object doesn't exists yet.\n",ch);
	    return;
	  }
	}
    } else {
	send_to_char("That object already exists in the world file.\n",ch);
	return;
    }
  } else {
	char *b;
	if (vnum && !strchr(buf2,'.'))
	  return;
	else b=vnum?(char *)(strchr(buf2,'.')+1):buf2;
	ch->build.obj_build=get_obj_vis_world(ch,b,vnum?&vnum:NULL);
	if (!ch->build.obj_build)
		send_to_char("Couldn't find that.\n",ch);
	else
	  send_to_char("Ok.\n",ch);
  }
}

void do_form(struct char_data *ch,char *arg, int cmd)
{
  struct room_data *rp;
  slong loc_nr;
  slong hi,lo;
  char buf[MAX_STRING_LENGTH];

  if (!*arg) {
	send_to_char("Syntax: form low_room# hi_room#\n",ch);
	return;
  }
  sscanf(arg,"%ld %ld",&lo,&hi);
  if (hi<lo || hi<0 || lo<0 || hi>2000000000 || lo>2000000000 || hi-lo>200) {
	send_to_char("Maximum of 200 rooms at once!\n",ch);
	return;
  }
  
  for (loc_nr=lo; loc_nr<=hi; loc_nr++) {
    if (NULL==real_roomp(loc_nr)) {
      allocate_room(loc_nr);
      rp = real_roomp(loc_nr);
      bzero(rp, sizeof(*rp));
      rp->number = loc_nr;
      if (top_of_zone_table >= 0) {
        int   zone;

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
      if (cmd==-1) {
	rp->name = (char *)mystrdup("The killing fields.");
	rp->description = (char *)mystrdup("You know you are going to die.\n");
      } else {
	sprintf(buf, "%d", loc_nr);
	rp->name = (char *)mystrdup(buf);
	rp->description = (char *)mystrdup("Empty\n");
      }
    }
  }
  send_to_char("You form a lot of order out of a little chaos\n",ch);
}

void do_olook(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct obj_data  *j=0;
  struct extra_descr_data *desc;
  int i;
  
  extern char *item_types[];
  extern char *wear_bits[];
  extern char *extra_bits[];
  extern char *drinks[];
  extern char *apply_types[];
  
  if (IS_NPC(ch)) return;
  if (!ch->build.obj_build) {
	send_to_char("You aren't building anything.\n",ch);
	return;
  }
  j=ch->build.obj_build;
  sprintf(buf,  "VNum: %ld  Name: %s\n"
		"Short: %s\n"
		"Long:\n%s\n"
		"Action: %s\nItem type:",
		j->virtual,
		NCH(j->name),
		NCH(j->short_description),
		NCH(j->description),
		NCH(j->action_description));

      sprinttype(GET_ITEM_TYPE(j),item_types,buf2);
      strcat(buf,buf2); strcat(buf,"\n");
      send_to_char(buf, ch);
      if(j->ex_description){
	strcpy(buf, "Extra description keyword(s):\n----------\n");
	for (desc = j->ex_description; desc; desc = desc->next) {
	  strcat(buf, desc->keyword);
	  strcat(buf, "\n");
	}
	strcat(buf, "----------\n");
	send_to_char(buf, ch);
      } else {
	strcpy(buf,"Extra description keyword(s): None\n");
	send_to_char(buf, ch);
      }
      
      send_to_char("Can be worn on :", ch);
      sprintbit(j->obj_flags.wear_flags,wear_bits,buf);
      strcat(buf,"\n");
      send_to_char(buf, ch);
      
      
      send_to_char("Extra flags: ", ch);
      sprintbit(j->obj_flags.extra_flags,extra_bits,buf);
      strcat(buf,"\n");
      send_to_char(buf,ch);
      
      sprintf(buf,"Weight: %d, Value: %d, Cost/day: %d, Timer: %d\n",
	      j->obj_flags.weight,j->obj_flags.cost,
	      j->obj_flags.cost_per_day,  j->obj_flags.timer);
      send_to_char(buf, ch);
      
      strcpy(buf,"In room: ");
      if (j->in_room == NOWHERE)
	strcat(buf,"Nowhere");
      else {
	sprintf(buf2,"%d",j->in_room);
	strcat(buf,buf2);
      }
      strcat(buf," ,In object: ");
      strcat(buf, (!j->in_obj ? "None" : fname(j->in_obj->name)));
      switch (j->obj_flags.type_flag) {
      case ITEM_LIGHT : 
	sprintf(buf, "Color : [%d]\nType : [%d]\nHours : [%d]",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2]);
	break;
      case ITEM_SCROLL : 
	sprintf(buf, "Spells : %d, %d, %d, %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3] );
	break;
      case ITEM_WAND : 
	sprintf(buf, "Spell : %d\nMana : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1]);
	break;
      case ITEM_STAFF : 
	sprintf(buf, "Spell : %d\nMana : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1]);
	break;
      case ITEM_WEAPON :
	sprintf(buf, "Tohit : %d\nTodam : %dD%d\nType : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
      case ITEM_FIREWEAPON : 
	sprintf(buf, "Tohit : %d\nTodam : %dD%d\nType : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
      case ITEM_MISSILE : 
	sprintf(buf, "Tohit : %d\nTodam : %d\nType : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[3]);
	break;
      case ITEM_ARMOR :
	sprintf(buf, "AC-apply : [%d]\nFull Strength : [%d]",
		j->obj_flags.value[0],
		j->obj_flags.value[1]);
	
	break;
      case ITEM_POTION : 
	sprintf(buf, "Spells : %d, %d, %d, %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]); 
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
      
      send_to_char("\nCan affect char :\n", ch);
      for (i=0;i<MAX_OBJ_AFFECT;i++) {
	sprinttype(j->affected[i].location,apply_types,buf2);
	sprintf(buf,"    Affects : %s By %d\n", buf2,j->affected[i].modifier);
	send_to_char(buf, ch);
      }			
      return;
}


void do_medit(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int field;
  slong v1;
  struct char_data *m;

  if (IS_NPC(ch)) return;
  arg=one_argument(arg,buf);
  m=ch->build.mob_build;
  if (!m) {
	send_to_char("Use mcreate first.\n",ch);
	return;
  }

  if (!*buf) {
	int i;
	i=0;
	send_to_char("Legal fields:\n",ch);
	while (*mob_fields[i].fieldname) {
	  sprintf(buf,"%-19s",mob_fields[i++].fieldname);
	  send_to_char(buf,ch);
	  if (i%4==0) send_to_char("\n",ch);
	}
	send_to_char("\n",ch);
	return;
  }
 
  if ((field=obj_search_field(buf,strlen(buf),mob_fields))<0) {
	int i;

	i=0;
	send_to_char("Unknown field.\n",ch);
	send_to_char("Legal fields:\n",ch);
	while (*obj_fields[i].fieldname) {
	  sprintf(buf,"%-19s",mob_fields[i++].fieldname);
	  send_to_char(buf,ch);
	  if (i%4==0) send_to_char("\n",ch);
	}
	return;
  }

  while (*arg && isspace(*arg)) arg++;

  switch (mob_fields[field].type) {
    case 0:
	arg=one_argument(arg,buf);
	if (!*buf || !isdigit(*buf)) {
		send_to_char("expecting number\n",ch);
		return;
	}
	v1=atol(buf);
	if (v1<mob_fields[field].min) {
		sprintf(buf,"Minimum value is %ld.\n",mob_fields[field].min);
		send_to_char(buf,ch);
		return;
	}
	if (v1>mob_fields[field].max) {
		sprintf(buf,"Maximum value is %ld.\n",mob_fields[field].max);
		send_to_char(buf,ch);
		return;
	}
	switch (field) {
	  case 4: m->specials.act=v1; break;
	  case 5: break;
	  case 6: m->specials.alignment=v1; break;
	  case 7: SET_LEVEL(m,v1); break;
	  case 8: m->points.hitroll=v1; break;
	  case 9: m->points.armor[4]=v1; break;
	  case 12: m->points.gold=v1; break;
	  case 13: GET_EXP(m)=v1; break;
	  case 14: GET_RACE(m)=v1; break;
	  case 15: m->specials.position=v1; break;
	  case 16: m->specials.default_pos=v1; break;
	  case 17: m->player.sex=v1; break;
	  case 18: m->specials.immune=v1; break;
	  case 19: m->specials.M_immune=v1; break;
	  case 20: m->specials.susc=v1; break;
	  case 23: GET_STR(m)=v1; break;
	  case 24: break;
	  case 25: GET_INT(m)=v1; break;
	  case 26: GET_WIS(m)=v1; break;
	  case 27: GET_DEX(m)=v1; break;
	  case 28: GET_CON(m)=v1; break;
	  case 29: GET_HEIGHT(m)=v1; break;
	  case 30: GET_WEIGHT(m)=v1; break;
	  case 31: m->player.class=v1;
	  default:
		send_to_char("Error! please report immediately\n",ch);
	}
	break;
    case 1:
	switch (field) {
	  case 0: ch->desc->str= &m->player.name; break;
	  case 1: ch->desc->str= &m->player.short_descr; break;
	  case 2: ch->desc->str= &m->player.long_descr; break;
	  case 3: ch->desc->str= &m->player.description; break;
	  case 21: ch->desc->str= &m->player.sounds; break;
	  case 22: ch->desc->str= &m->player.distant_snds; break;
	  default: send_to_char("Error! please report immediately\n",ch);
		  return;
	}
	if (*arg) {
		strcpy(buf,arg);
		buf[mob_fields[field].max]='\0';
    		CREATE(*ch->desc->str, char, strlen(buf) + 1);
		strcpy(*ch->desc->str,buf);
		ch->desc->str=NULL;
		if (strlen(buf)>mob_fields[field].max)
			send_to_char("Too long, trucated.\n",ch);
	} else {
		ch->desc->max_str=mob_fields[field].max;
		*ch->desc->str='\0';
	}
        break;
    case 2:
    case 3:
	send_to_char("ERROR:Please report\n",ch);
        break;
    case 4: {
	int dice, dsize;

	while (*arg && *arg==' ') arg++;
	if (!isdigit(*arg)) {
	  send_to_char("Expecting something in the format of ndx or n x\n",ch);
	  return;
	}
	dice=atoi(arg);
	while (*arg && isdigit(*arg)) arg++;
	if (*arg=='d') arg++;
	while (*arg && *arg==' ') arg++;
	if (!isdigit(*arg)) {
	  send_to_char("Expecting something in the format of ndx or n x\n",ch);
	  return;
	}
	dsize=atoi(arg);

	switch (field) {
	  case 10: ch->build.hpd=dice; ch->build.hps=dsize; break;
	  case 11: m->specials.damnodice=dice;
		   m->specials.damsizedice=dsize;
		   break;
	}
	break;
     }
  }
  send_to_char("Ok.\n",ch);
}

void do_mcreate(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  slong vnum;

  if (IS_NPC(ch)) return;

  arg=one_argument(arg,buf);
  if (!*buf) {
	send_to_char("Syntax: mcreate new|old|one vnum [copy_vnum]\n",ch);
	return;
  }
  if (is_abbrev(buf,"one") && GetMaxLevel(ch)<SILLYLORD) {
	send_to_char("Thou art not godly enough.\n",ch);
	return;
  }
  if (!is_abbrev(buf,"new") && !is_abbrev(buf,"old") && !is_abbrev(buf,"one")) {
	send_to_char("Type mcreate without any parameters for help.\n",ch);
	return;
  }
  arg=one_argument(arg,buf2);
  if (!*buf2) {
	send_to_char("Need vnum.\n",ch);
	return;
  }
  vnum=atol(buf2);
  CHK_PLAYER_ZONE(ch,vnum);
  if (!is_abbrev(buf,"one")) {
    if (!real_mobp(vnum)) {
	if (is_abbrev(buf,"new")) {
	  CREATE(ch->build.mob_build,struct char_data,1);
	  bzero(ch->build.mob_build,sizeof(struct char_data));
	  clear_char(ch->build.mob_build);
	  ch->build.m_virtual=vnum;
          ch->desc->connected=CON_MOB;
          ch->build.state=1;
          send_to_char("You can type QUIT at any prompt to abort.\n",ch);
          send_to_char("At most prompts, you can type ? for help.\n\n",ch);
          send_to_char("Name list: ",ch);
	  /*send_to_char("Ok.  Now use medit.\n",ch); */
	} else {
	  FILE *f;

	  sprintf(buf,"Mobile/%ld",vnum);
	  if ((f=fopen(buf,"r"))!=NULL) {
	    CREATE(ch->build.mob_build,struct char_data,1);
	    bzero(ch->build.mob_build,sizeof(struct char_data));
	    clear_char(ch->build.mob_build);
/*
	    read_one_mobile(f,ch->mob_build);
*/
	    fclose(f);
	    send_to_char("Ok.  Now use medit\n",ch);
	  } else {
	    send_to_char("Mobile doesn't exists yet.\n",ch);
	    return;
	  }
	}
    } else {
	send_to_char("That mobile already exists in the world file.\n",ch);
	return;
    }
  } else {
	char *b;
	if (vnum && !strchr(buf2,'.'))
	  return;
	else b=vnum?(char *)(strchr(buf2,'.')+1):buf2;
	ch->build.mob_build=get_char_vis_world(ch,b,vnum?&vnum:NULL);
	if (!ch->build.mob_build)
		send_to_char("Couldn't find that.\n",ch);
	else
	  send_to_char("Ok.\n",ch);
  }
}

void do_msave(struct char_data *ch, char *arg, int cmd)
{
  char filename[MAX_STRING_LENGTH];
  struct char_data *m;
  FILE *f;

  m=ch->build.mob_build;
  if (!m) {
        send_to_char("You aren't working on an object!\n",ch);
        return;
  }
  return;
  sprintf(filename,"Mobile/%ld",ch->build.m_virtual);
  if ((f=fopen(filename,"w+"))==NULL) {
        send_to_char("Uh-Oh... can't save!\n",ch);
        log(filename);
        return;
  }
  fclose(f);
}

void do_mat(struct char_data *ch,char *arg, int cmd)
{
  slong loc_nr;
  slong hi,lo;
  char buf[MAX_STRING_LENGTH];
  if (!*arg) {
	send_to_char("Syntax: mat low_room# hi_room# cmd\n",ch);
	return;
  }
  arg=one_argument(arg,buf);
  lo=OFFSET(ch,atol(buf));
  arg=one_argument(arg,buf);
  hi=OFFSET(ch,atol(buf));
  if (cmd!=-1) {
    if (lo==0) lo=OFFSET(ch,1)-1;
    if (hi==0) hi=OFFSET(ch,1)-1;

    if (hi<lo || hi<0 || lo<0 || hi>2000000000 || lo>2000000000 || hi-lo>200) {
      send_to_char("Maximum of 200 rooms at once!\n",ch);
      return;
    }
    CHK_PLAYER_ZONE(ch,lo);
    CHK_PLAYER_ZONE(ch,hi);
  }
  for (loc_nr=lo; loc_nr<=hi; loc_nr++) {
    if (real_roomp(loc_nr)) {
	sprintf(buf,"%d %s",loc_nr,arg);
	do_at(ch,buf,cmd);
    }
  }
}


/* Oset.c is Copyright (C) 1992 by Dan Brumleve.  Ignorance or removal  *
 * of this friendly reminder is punishable by death by slooow torture  */

struct oset_field_data {
    char *set[4];
    char *description[4];
};


struct oset_field_data oset_field[26] = {
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"duration", "", "", ""},
      {"(duration of light.  -1 for permanent)", "", "", ""} },
    { {"level", "spell1", "spell2", "spell3"},
      {"(level of spells)", "(first spell)", "(second)", "(third)"} },
    { {"level", "max-charges", "charges", "spell"},
      {"(level of spell)", "(maximum charges)", "(charges left)", "(spell)"} },
    { {"level", "max-charges", "charges", "spell"},
      {"(level of spell)", "(maximum charges)", "(charges left)", "(spell)"} },
    { {"damage", "wtype", "", ""},
      {"(weapon damage, in the form: XdY)", "(weapon type)", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"points", "", "", ""},
      {"(armor points)", "", "", ""} },
    { {"level", "spell1", "spell2", "spell3"},
      {"(level of spells)", "(first spell)", "(second)", "(third)"} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"capacity", "flags", "key", ""},
      {"(container's capacity)", "(container flags)", "(opening key)", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"capacity", "amount", "liquid", "poisoned"},
      {"(capacity)", "(amount left)", "(liquid)", "(poisoned?)"} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"amount", "poisoned", "", ""},
      {"(number of hours restored)", "(poisoned?)", "", ""} },
    { {"amount", "", "", ""},
      {"(amount of money)", "", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"entrance", "exit", "front", "flags"},
      {"(entrance room)", "(exit dir)", "(front room)", "(wagon flags)"} },
    { {"damage", "poison", "", ""},
      {"(dart damage, in the form: XdY)", "(amount of poison)", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} }
};
    

void set_oedesc(struct char_data *ch, struct obj_data *obj, char *keywds) 
{
    struct extra_descr_data *tmp, *newdesc;

    if (!*keywds) {
	send_to_char("You must give a list of keywords.\n", ch);
	return;
    }
    for (tmp = obj->ex_description; tmp; tmp = tmp->next) {
	if (!str_cmp(tmp->keyword, keywds)) {
	    break;
	}
    }

    CREATE(newdesc, struct extra_descr_data, 1);
    newdesc->description = (char *)0;

    if (!tmp) {
	newdesc->next = obj->ex_description;
	obj->ex_description = newdesc;
	newdesc->keyword = mystrdup(keywds);
    } else {
	send_to_char("The old description was: \n\n", ch);
	send_to_char(tmp->description, ch);
	FREE(newdesc);
	newdesc = tmp;
    }

    FREE(newdesc->description);

    send_to_char("Enter a new description.  Terminate with a '~'\n", ch);
    ch->desc->str = &newdesc->description;
    newdesc->description = 0;
    ch->desc->max_str = 1000;
}

void set_otype(struct char_data *ch, struct obj_data *obj, char *arg) 
{
    int type, i;

#if 0
    extern int oval_defaults[24][4];
#endif

    static char *obj_type[]=
    {
	"light",
	"scroll",
	"wand",
	"staff",
	"weapon",
	"fireweapon",
	"missile",
	"treasure",
	"armor",
	"potion",
	"worn",
	"other",
	"trash",
	"trap",
	"container",
	"note",
	"drinkcon",
	"key",
	"food",
	"money",
	"pen",
	"boat",
	"wagon",
        "dart",
	"\n"
    };

    if (!*arg) {
        send_to_char("The following types are available:\n\n", ch);
        for (i = 0; *obj_type[i] != '\n'; i++) {
            send_to_char(obj_type[i], ch);
            send_to_char("\n", ch);
        }
        return;
    }

    type = old_search_block(arg, 0, strlen(arg), obj_type, 0);
    if (type == -1) {
	send_to_char("That obj type does not exist.\n", ch);
	return;
    }

    obj->obj_flags.type_flag = type;

#if 0
    for (i = 0; i < 4; i++)
	obj->obj_flags.value[i] = oval_defaults[type][i];
#endif

    send_to_char("Done.\n", ch);
}

void set_oaffect(struct char_data *ch, struct obj_data *obj, char *arg, int a)
{
    char buf1[256], buf2[256];
    int type, mod, i;

    static char *oaffects[]=
    {
	"strength",
	"dexterity",
	"intelligence",
	"wisdom",
	"constitution",
	"",
	"",
	"",
	"age",
	"weight",
	"height",
	"mana points",
	"hit points",
	"movement points",
	"",
	"",
	"armor",
	"offense",
	"damage",
	"",
	"",
	"",
	"",
	"",
	"sneak",
	"hide",
	"listen",
	"climb",
	"defense",
	"\n"
    };

    if (!*arg) {
        send_to_char("The following affects are available:\n\n", ch);
        send_to_char("none\n", ch);
        for (i = 0; *oaffects[i] != '\n'; i++) {
            send_to_char(oaffects[i], ch);
            send_to_char("\n", ch);
        }
        return;
    }

    if (!str_cmp(arg, "none")) {
        obj->affected[a].location = 0;
        obj->affected[a].modifier = 0;
        send_to_char("Done.\n", ch);
        return;
    }

    arg = one_argument(arg, buf1);
    arg = one_argument(arg, buf2);

    type = old_search_block(buf1, 0, strlen(buf1), oaffects, 0);
    mod = atoi(buf2);

    if (type == -1) {
	send_to_char("That affect does not exist.\n", ch);
	return;
    }
    if (!mod) {
	send_to_char("You must specify an amount to affect it by.\n", ch);
	return;
    }
    if ((mod > 100) || (mod < -100)) {
	send_to_char("That object affect is too high.\n", ch);
	return;
    }
    obj->affected[a].location = type;
    obj->affected[a].modifier = mod;
    send_to_char("Done.\n", ch);
}


void do_oset(struct char_data *ch, char *argument, int cmd)
{
    char arg2[256], arg3[256];
    char buf[256], buf2[256];
    int i, j;
    int val, dice, sides;
    int spaces;
    struct obj_data *obj;
    struct extra_descr_data *tmpexd;

    char *generic_field[] = {
        "name",
        "sdesc",
        "ldesc",
        "desc",
        "edesc",
        "type",
        "aff1",
        "aff2",
        "weight",
        "cost",
        "storage",
        "limit",
        "\n"
    };

    char *generic_desc[] = {
        "(\"sword long spiked\")",
        "(\"a spiked long sword\")",
        "(\"You see a spiked long sword here.\")",
        "(object description)",
        "(object extra description)",
        "(object type)",
        "(first affect)",
        "(second affect)",
        "(object weight)",
        "(object cost)",
        "(object storage cost)",
        "(object limit)",
        "\n"
    };

    extern char *drinknames[];

/*
    argument = one_argument(argument, arg1);
*/
    half_chop(argument, arg2, arg3);

/*
    obj = get_obj_vis(ch, arg1);
*/
	obj=ch->build.obj_build;
    if (!obj) {
	send_to_char("That object does not exist.\n", ch);
	return;
    }

    if (!*arg2) {
        for (i =  0; *generic_field[i] != '\n'; i++) {
            spaces = 15 - strlen(generic_field[i]);
	    buf2[0]='\0';
            for (j = 0; j < spaces; j++) 
                strcat(buf2, " ");
            sprintf(buf, "%s%s%s\n", generic_field[i], buf2, generic_desc[i]);
            send_to_char(buf, ch);
        }
        for (i = 0; i < 4; i++) {
            if (strcmp(oset_field[obj->obj_flags.type_flag].set[i], "")) {
                spaces = 15 - strlen(oset_field[obj->obj_flags.type_flag].set[i]);
		buf2[0]='\0';
                for (j = 0; j < spaces; j++)
                    strcat(buf2, " ");
                sprintf(buf, "%s%s%s\n", oset_field[obj->obj_flags.type_flag].set[i], buf2, oset_field[obj->obj_flags.type_flag].description[i]);
                send_to_char(buf, ch);
            }
        }
        return;
    }

    for (i = 0; *generic_field[i] != '\n'; i++)
	if (is_abbrev(arg2,generic_field[i])) break;

    switch (i) {
    case 0: /* name */
        for (tmpexd = obj->ex_description; tmpexd; tmpexd = tmpexd->next)
            if (!str_cmp(tmpexd->keyword, obj->name))
	        break;

        if (tmpexd) {
            FREE( tmpexd->keyword);
            tmpexd->keyword = mystrdup(arg3);
        }
        FREE(obj->name);
        obj->name = mystrdup(arg3);
        send_to_char("Done.\n", ch);
        return;
    case 1: /* sdesc */
        FREE(obj->short_description);
        obj->short_description = mystrdup(arg3);
        send_to_char("Done.\n", ch);
        return;
    case 2: /* ldesc */
        FREE(obj->description);
        obj->description = mystrdup(arg3);
        send_to_char("Done.\n", ch);
        return;
    case 3: /* desc */
        sprintf(buf, " %s", obj->name);
        set_oedesc(ch, obj, buf);
        return;
    case 4: /* edesc */
        set_oedesc(ch, obj, arg3);
        return;
    case 5: /* type */
        set_otype(ch, obj, arg3);
        return;
    case 6: /* aff1 */
        set_oaffect(ch, obj, arg3, 0);
        return;
    case 7: /* aff2 */
        set_oaffect(ch, obj, arg3, 1);
        return;
    case 8: /* weight */
        if ((atoi(arg3) < 0) || (atoi(arg3) > 100000)) {
            send_to_char("That weight is invalid.\n", ch);
            return;
        }
        obj->obj_flags.weight = atoi(arg3);
        send_to_char("Done.\n", ch);
        return;
    case 9: /* cost */
        if ((atoi(arg3) < 0) || (atoi(arg3) > 100000)) {
            send_to_char("That cost is invalid.\n", ch);
            return;
        }
        obj->obj_flags.cost = atoi(arg3);
        send_to_char("Done.\n", ch);
        return;
    case 10: /* storage */
        if ((atoi(arg3) < 0) || (atoi(arg3) > 10000)) {
            send_to_char("That storage cost is invalid.\n", ch);
            return;
        }
        obj->obj_flags.cost_per_day = atoi(arg3);
        send_to_char("Done.\n", ch);
        return;
    case 11: /* limit */
	send_to_char("Not implemented.\n",ch);
        return;
    }

    for (i = 0; i < 4; i++) 
	if (is_abbrev(arg2,oset_field[obj->obj_flags.type_flag].set[i])) break;
    if (i == 4) {
        send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n", ch);
        return;
    }

    switch (obj->obj_flags.type_flag) {
    case ITEM_LIGHT:
        switch (i) {
        case 0: /* duration */
            if (!*arg3) {
                send_to_char("ITEM_LIGHT: DURATION\n\nThe light's duration is the amount of time (in game hours) that the\nlight will stay on.  Use a value of -1 for an infinite duration.\n", ch);
                return;
            } 
            if ((atoi(arg3) > 100000) || (atoi(arg3) < -1)) {
                send_to_char("That light duration is invalid.\n", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n", ch);
            break;
        }
        return;
    case ITEM_SCROLL:
        switch (i) {
        case 0: /* level */
            if (!*arg3) {
                send_to_char("ITEM_SCROLL: LEVEL\n\nThis value represents the level of the spells contained in the scroll.\nIt must be a number between 1 and 20.\n", ch);
                return;
            }
            if ((atoi(arg3) < 1) || (atoi(arg3) > 20)) {
                send_to_char("That spell level is invalid.\n", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        case 1: /* spell */
        case 2:
        case 3:
            if (!*arg3) {
                send_to_char("ITEM_SCROLL: SPELL\n\nThis value determines which spell is performed by reciting the scroll.\nYou need not use the spell number, but simply the name:\nOSET <scroll> SPELL# FIREBALL\n", ch);
                return;
            }
	    val = (old_search_block(arg3, 0, strlen(arg3), spells, 0));
	    if (val == -1) {
	        send_to_char("There is no spell of that name.\n", ch);
	        return;
	    }
	    val--;
#if 0
	    if (!IS_SPELL(val)) {
	        send_to_char("That's a SKILL, not a SPELL.\n", ch);
	        return;
	    }
#endif
            obj->obj_flags.value[i] = val;
            send_to_char("Done.\n", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n", ch);
            break;
        }
        return;
    case ITEM_WAND:
        switch (i) {
        case 0: /* level */
            if (!*arg3) {
                send_to_char("ITEM_WAND: LEVEL\n\nThis value represents the level of the spell contained in the wand.\nIt must be a number between 1 and 20.\n", ch);
                return;
            }
            if ((atoi(arg3) < 1) || (atoi(arg3) > 20)) {
                send_to_char("That spell level is invalid.\n", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        case 1: /* max-charges */
            if (!*arg3) {
                send_to_char("ITEM_WAND: MAX-CHARGES\n\nThe number of \"charges\" that a wand has is the amount of times that it\ncan be used.  In the future a \"recharge\" spell will probably be made, which\nis probably the reason that a \"max-charges\" value was created in the first\nplace.  Just set \"max-charges\" and \"charges\" to the same thing for now,\nunless you have something weird in mind. ;-)\n", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 100)) {
                send_to_char("That max-charges value is invalid.\n", ch);
                return;
            }
            obj->obj_flags.value[1] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        case 2: /* charges */
            if (!*arg3) {
                send_to_char("ITEM_WAND: CHARGES\n\nThe number of \"charges\" that a wand has is the amount of times that it\ncan be used.  In the future a \"recharge\" spell will probably be made, which\nis probably the reason that a \"max-charges\" value was created in the first\nplace.  Just set \"max-charges\" and \"charges\" to the same thing for now,\nunless you have something weird in mind. ;-)\n", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 100)) {
                send_to_char("That charges value is invalid.\n", ch);
                return;
            }
            obj->obj_flags.value[2] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        case 3: /* spell */
            if (!*arg3) {
                send_to_char("ITEM_WAND: SPELL\n\nThis value determines which spell is performed by using the wand.\nYou need not use the spell number, but simply the name:\nOSET <wand> SPELL# FIREBALL\n", ch);
                return;
            }
	    val = (old_search_block(arg3, 0, strlen(arg3), spells, 0));
	    if (val == -1) {
	        send_to_char("There is no spell of that name.\n", ch);
	        return;
	    }
	    val--;
#if 0
	    if (!IS_SPELL(val)) {
	        send_to_char("That's a SKILL, not a SPELL.\n", ch);
	        return;
	    }
#endif
            obj->obj_flags.value[3] = val;
            send_to_char("Done.\n", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n", ch);
            break;
        }
        return;
    case ITEM_STAFF:
        switch (i) {
        case 0: /* level */
            if (!*arg3) {
                send_to_char("ITEM_STAFF: LEVEL\n\nThis value represents the level of the spell contained in the staff.\nIt must be a number between 1 and 20.\n", ch);
                return;
            }
            if ((atoi(arg3) < 1) || (atoi(arg3) > 20)) {
                send_to_char("That spell level is invalid.\n", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        case 1: /* max-charges */
            if (!*arg3) {
                send_to_char("ITEM_STAFF: MAX-CHARGES\n\nThe number of \"charges\" that a staff has is the amount of times that it\ncan be used.  In the future a \"recharge\" spell will probably be made, which\nis probably the reason that a \"max-charges\" value was created in the first\nplace.  Just set \"max-charges\" and \"charges\" to the same thing for now,\nunless you have something weird in mind. ;-)\n", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 100)) {
                send_to_char("That max-charges value is invalid.\n", ch);
                return;
            }
            obj->obj_flags.value[1] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        case 2: /* charges */
            if (!*arg3) {
                send_to_char("ITEM_STAFF: CHARGES\n\nThe number of \"charges\" that a staff has is the amount of times that it\ncan be used.  In the future a \"recharge\" spell will probably be made, which\nis probably the reason that a \"max-charges\" value was created in the first\nplace.  Just set \"max-charges\" and \"charges\" to the same thing for now,\nunless you have something weird in mind. ;-)\n", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 100)) {
                send_to_char("That charges value is invalid.\n", ch);
                return;
            }
            obj->obj_flags.value[2] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        case 3: /* spell */
            if (!*arg3) {
                send_to_char("ITEM_STAFF: SPELL\n\nThis value determines which spell is performed by using the staff.\nYou need not use the spell number, but simply the name:\nOSET <staff> SPELL# FIREBALL\n", ch);
                return;
            }
	    val = (old_search_block(arg3, 0, strlen(arg3), spells, 0));
	    if (val == -1) {
	        send_to_char("There is no spell of that name.\n", ch);
	        return;
	    }
	    val--;
#if 0
	    if (!IS_SPELL(val)) {
	        send_to_char("That's a SKILL, not a SPELL.\n", ch);
	        return;
	    }
#endif
            obj->obj_flags.value[3] = val;
            send_to_char("Done.\n", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n", ch);
            break;
        }
        return;
    case ITEM_WEAPON:
        switch (i) {
        case 0:
            if (!*arg3) {
                send_to_char("ITEM_WEAPON: DAMAGE\n\nWeapon damage *must* be given in the form \"XdY\", where X and Y are the\ntwo damage dice.  A die with Y sides is rolled X times, and added up for the\ntotal damage.  Other bonuses such as character strength also apply to\ndamage.  Damage should range from around 1d4 at a minimum to 5d5 or so as a\nmaximum.  If you wish to create a weapon which is an exception to the rules,\ntalk to an Overlord about it.\n", ch);
                return;
            }
            sscanf(arg3, "%dd%d", &dice, &sides);
            if ((dice < 1) || (dice > 20) || (sides < 1) || (sides > 20)) {
                send_to_char("That weapon damage is invalid.\n", ch);
                return;
            }
            obj->obj_flags.value[1] = dice;
            obj->obj_flags.value[2] = sides;
            send_to_char("Done.\n", ch);
            break;
        case 1: /* wtype */
            if (!*arg3) {
                send_to_char("ITEM_WEAPON: WTYPE\n\nThe weapon type determines the attack type used by the weapon.\nYou may set it to (p)iercing, (s)lashing, (b)ludgeoning.\n", ch);
                return;
            if ((*arg3 == 'p') || (*arg3 == 'P')) 
               obj->obj_flags.value[3] = TYPE_PIERCE;
            else if ((*arg3 == 's') || (*arg3 == 'S')) 
               obj->obj_flags.value[3] = TYPE_SLASH;
            else if ((*arg3 == 'b') || (*arg3 == 'B')) 
               obj->obj_flags.value[3] = TYPE_BLUDGEON;
            else {
                send_to_char("That is not a valid weapon type.\n", ch);
                return;
            }
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n", ch);
            break;
        }
        return;
    case ITEM_ARMOR:
        switch (i) {
        case 0: /* points */
            if (!*arg3) {
                send_to_char("ITEM_ARMOR: POINTS\n\nArmor points are directly proportional to the amount of damage absorbed\nby a blow from a weapon on that position.  If the armor points value is\ndivisible by ten, the armor will absord points / 10 damage points.  If it is\nin between multiples of ten, the amount will be semi-random.  For instance,\nsay a helmet of 44 AP is hit for 10 damage.  There will be a 40% chance of\nthe armor absorbing 5 damage points, and a 60% chance of it absorbing 4.\n", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 100)) {
                send_to_char("That armor points value is invalid.\n", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n", ch);
            break;
        }
        return;
    case ITEM_POTION:
        switch (i) {
        case 0: /* level */
            if (!*arg3) {
                send_to_char("ITEM_POTION: LEVEL\n\nThis value represents the level of the spells contained in the potion.\nIt must be a number between 1 and 20.\n", ch);
                return;
            }
            if ((atoi(arg3) < 1) || (atoi(arg3) > 20)) {
                send_to_char("That spell level is invalid.\n", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        case 1: /* spell */
        case 2:
        case 3:
            if (!*arg3) {
                send_to_char("ITEM_POTION: SPELL\n\nThis value determines which spell is performed by quaffing the potion.\nYou need not use the spell number, but simply the name:\nOSET <potion> SPELL# FIREBALL\n", ch);
                return;
            }
	    val = (old_search_block(arg3, 0, strlen(arg3), spells, 0));
	    if (val == -1) {
	        send_to_char("There is no spell of that name.\n", ch);
	        return;
	    }
	    val--;
#if 0
	    if (!IS_SPELL(val)) {
	        send_to_char("That's a SKILL, not a SPELL.\n", ch);
	        return;
	    }
#endif
            obj->obj_flags.value[i] = val;
            send_to_char("Done.\n", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n", ch);
            break;
        }
        return;
    case ITEM_CONTAINER:
        switch (i) {
        case 0: /* capacity */
            if (!*arg3) {
                send_to_char("ITEM_CONTAINER: CAPACITY\n\nThis value determines the maximum weight that the container can hold.\n", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 10000)) {
                send_to_char("That capacity is invalid.\n", ch);
                return; 
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        case 1: /* flags */
            if (!*arg3) {
                send_to_char("ITEM_CONTAINER: FLAGS\n\nYou may set the following flags: (c)losable, (p)ickproof, and (t)rapped.\n", ch);
                return;
            }
            switch (*arg3) {
            case 'c':
            case 'C':
                if (IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE)) {
                    REMOVE_BIT(obj->obj_flags.value[1], CONT_CLOSEABLE);
                    send_to_char("You have removed the 'closable' flag\n", ch);    
                } else {
                    SET_BIT(obj->obj_flags.value[1], CONT_CLOSEABLE);
                    send_to_char("You have set the 'closable' flag\n", ch);  
                }
                break;
            case 'p':
            case 'P':
                if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF)) {
                    REMOVE_BIT(obj->obj_flags.value[1], CONT_PICKPROOF);
                    send_to_char("You have removed the 'pickproof' flag\n", ch);    
                } else {
                    SET_BIT(obj->obj_flags.value[1], CONT_PICKPROOF);
                    send_to_char("You have set the 'pickproof' flag\n", ch);  
                }
                break;
            case 't':
            case 'T':
#if 0
                if (IS_SET(obj->obj_flags.value[1], CONT_TRAPPED)) {
                    REMOVE_BIT(obj->obj_flags.value[1], CONT_TRAPPED);
                    send_to_char("You have removed the 'trapped' flag\n", ch);
                } else {
                    SET_BIT(obj->obj_flags.value[1], CONT_TRAPPED);
                    send_to_char("You have set the 'trapped' flag\n", ch);  
                }
#endif
                break;
            default:
                send_to_char("That container flag is invalid.\n", ch);
                break;
            }
            break;
        case 2: /* key */
            if (!*arg3) {
                send_to_char("ITEM_CONTAINER: KEY\n\nThis value is the virtual object number for a key which can open the\ncontainer.\n", ch);
                return;
            }
            if (GetMaxLevel(ch)<SILLYLORD) {
		CHK_PLAYER_ZONE(ch,atoi(arg3));
	    }
            obj->obj_flags.value[2] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n", ch);
            break;
        }
    case ITEM_DRINKCON:
        switch (i) {
        case 0: /* capacity */
            if (!*arg3) {
                send_to_char("ITEM_DRINKCON: CAPACITY\n\nThis value represents the amount of liquid that the drink container\ncan hold.\n", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 10000)) {
                send_to_char("That capacity is invalid.\n", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        case 1:
            if (!*arg3) {
               send_to_char("ITEM_DRINKCON: AMOUNT\n\nThis is the amount of liquid left in the container.\n", ch);
               return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 10000)) {
                send_to_char("That amount is invalid.\n", ch);
                return;
            }
            if (atoi(arg3) > obj->obj_flags.value[0]) {
                send_to_char("The amount of liquid can't be more than the liquid capacity.\n", ch);
                return;
            }
            obj->obj_flags.value[1] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        case 2:
            if (!*arg3) {
                send_to_char("ITEM_DRINKCON: TYPE\n\nThere are a variety of different drink types you can use.  Read HELP\nITEM_DRINKCON for a list of them all.\n", ch);
                return;
            }
            for (i = 0; i < 16; i++)
		if (is_abbrev(arg3, drinknames[i])) break;
            if (i == 16) {
                send_to_char("There is no such drink.  See HELP ITEM_DRINKCON for a list.\n", ch);
                return;
            }
            obj->obj_flags.value[2] = i;
            send_to_char("Done.\n", ch);
            break;
        case 3:
            if (!*arg3) {
                send_to_char("ITEM_DRINKCON: POISONED\n\nSet this value to TRUE if you want the drink to be poisoned.\n", ch);
                return;
            }
            if (is_abbrev(arg3,"true")) {
                obj->obj_flags.value[3] = TRUE;
                send_to_char("Done.\n", ch);
            } else if (is_abbrev(arg3,"false")) {
                obj->obj_flags.value[3] = FALSE;
                send_to_char("Done.\n", ch);
            } else 
                send_to_char("The poisoned setting can be set to either TRUE or FALSE.\n", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n", ch);
            break;
        }
        return;
    case ITEM_FOOD:
        switch (i) {
        case 0:
            if (!*arg3) {
                send_to_char("ITEM_FOOD: AMOUNT\n\nThis value represents the number of game hours which the food will fill\nyou for.\n", ch);
                return;
            }
            if ((atoi(arg3) > 24) || (atoi(arg3) < 1)) {
                send_to_char("That amount is invalid.\n", ch);
                return;
            }
            obj->obj_flags.value[i] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        case 1:
            if (!*arg3) {
                send_to_char("ITEM_FOOD: POISONED\n\nSet this value to TRUE if you want the drink to be poisoned.\n", ch);
                return;
            }
            if (is_abbrev(arg3,"true")) {
                obj->obj_flags.value[3] = TRUE;
                send_to_char("Done.\n", ch);
            } else if (is_abbrev(arg3,"false")) {
                obj->obj_flags.value[3] = FALSE;
                send_to_char("Done.\n", ch);
            } else 
                send_to_char("The poisoned setting can be set to either TRUE or FALSE.\n", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n", ch);
            break;
        }
        return;
    case ITEM_MONEY:
        switch (i) {
        case 0:
            if (!*arg3) {
                send_to_char("ITEM_MONEY: AMOUNT\n\nThe amount of gold contained in a \"gold object.\"\n", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 1000000)) {
                send_to_char("That amount is invalid.\n", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n", ch);
            break;
        }
        return;
      }
  }
}
#if 0
    case ITEM_WAGON:
        switch (i) {
        case 0: /* entrance */
            if (!*arg3) {
                send_to_char("ITEM_WAGON: ENTRANCE\n\nThe entrance to the wagon is the room which characters are put into\nwhen they enter.\n", ch);
                return;
            }
            if (real_room(atoi(arg3)) == -1) {
                send_to_char("That room doesn't exist.\n", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        case 1: /* exit */
            if (!*arg3) {
                send_to_char("ITEM_WAGON: EXIT\n\nThis value is the direction which leads out of the entrance room to the\noutside.\n", ch);
                return;
            }
            if (*arg3 == 'n') dir = 0;
            else if (*arg3 == 'e') dir = 1;
            else if (*arg3 == 's') dir = 2;
            else if (*arg3 == 'w') dir = 3;
            else if (*arg3 == 'u') dir = 4;
            else if (*arg3 == 'd') dir = 5;
            else {
                send_to_char("You must specify a valid direction (neswud).\n", ch);
                return;
            }
            if (!world[real_room(obj->obj_flags.value[0])].dir_option[dir]) {
                send_to_char("There is no exit leading out of the wagon in that direction.\n", ch);
                return;
            }
            obj->obj_flags.value[1] = dir;
            break;
        case 2: /* front */
            if (!*arg3) {
                send_to_char("ITEM_WAGON: FRONT\n\nThe front of the wagon is the room number from which commands to the\nlizard leading the wagon can be issued.\n", ch);
                return;
            }
            if (real_room(atoi(arg3)) == -1) {
                send_to_char("That room doesn't exist.\n", ch);
                return;
            }
            obj->obj_flags.value[2] = atoi(arg3);
            send_to_char("Done.\n", ch);
            break;
        case 3:
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n", ch);
            break;
        }
        return;
    }
}
#endif
