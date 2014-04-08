/************************************************************************
*  file: actwiz.c , Implementation of commands.           Part of DIKUMUD *
*  Usage : Wizard Commands.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

extern int DEBUG;

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcns.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <spells.h>
#include <limits.h>
#include <externs.h>
#include <crypt.h>

/*   external vars  */

extern struct zone_data *zone_table;
extern int top_of_zone_table;
extern struct hash_header room_db;
extern struct hash_header mob_db;
extern struct hash_header obj_db;
extern struct char_data *character_list;
extern struct connection_data *connection_list;
extern int top_of_objt;
extern struct int_app_type int_app[26];
extern struct wis_app_type wis_app[26];
extern int top_of_world;
extern char *dirs[];
extern skill_entry skill_table[MAX_SKILLS][32];
extern int shutdwn, reboot;

long reboot_time;

unsigned long int *no_rent;
int num_no_rent;

#define NORENT_FILE "norent"

void boot_no_rent()
{
  FILE *f;
  int i;

  num_no_rent=0;
  f=fopen(NORENT_FILE,"r");
  if (!f) return;
  fscanf(f,"%d",&num_no_rent);
  CREATE(no_rent,unsigned long,num_no_rent);
  for (i=0; i<num_no_rent; i++)
    fscanf(f,"%d",&no_rent[i]);
  fclose(f);
}

int is_no_rent(unsigned long vnum)
{
  int i;
  for (i=0; i<num_no_rent; i++)
    if (vnum==no_rent[i]) return(i+1);
  return(0);
}

void do_norent(struct char_data *ch, char *arg, int cmd)
{
  int vnum,i,j;
  FILE *f;
  char buf[MAX_INPUT_LENGTH];

  arg=one_argument(arg,buf);
  vnum=atoi(buf);
  if (!*buf) {
	for (i=0; i<num_no_rent; i++)
	  sendf(ch,"%7d%s",no_rent[i],(i+1)%10==0?"\n":" ");
	sendf(ch,"\n");
	sendf(ch,"%d items explicitly marked no_rent.\n",num_no_rent);
	return;
  }
  j=is_no_rent(vnum);
  if (j) {
	j--;
	num_no_rent--;
	for (i=j; i<num_no_rent-1; i++) no_rent[i]=no_rent[i+1];
	sendf(ch,"Removed %d\n",vnum);
  } else {
	sendf(ch,"Added %d\n",vnum);
	no_rent=REALLOC(no_rent,num_no_rent+1,unsigned long int);
	no_rent[num_no_rent++]=vnum;
  }
  f=fopen(NORENT_FILE,"w+");
  if (!f) {
	sendf(ch,"Couldn't write to %s.\n",NORENT_FILE);
	return;
  }
  fprintf(f,"%d\n",num_no_rent);
  for (i=0; i<num_no_rent; i++)
	fprintf(f,"%d\n",no_rent[i]);
  fclose(f);
}

#define USAGE "skill class skill min-level min-mana max-learn [class restrictions]\n"\
	      "For any of the above, * will refer to the previous value.\n"\
	      "Ex: skill c armor * 6 *\nSets the clerics armor spell to 6"\
	      "mana.\n"
void do_skill(struct char_data *ch, char *arg,int cmd)
{
#if 0
  extern char *spells[];
  char buf[MAX_INPUT_LENGTH];
  int class,skill,min_level,min_mana,max_learn,no_class;
  int i;

  arg=one_argument(arg,buf);
  if (!*buf) { sendf(ch,USAGE); return; }
  if ((class=which_class(ch,buf,1)) < 0) return;
  if (!arg || !*arg) { sendf(ch,USAGE); return; }

  i=0;
  for (;isspace(*arg);arg++);
  while (!isdigit(*arg) && *arg!='*') buf[i++] = *(arg++);
  if (i>0)
    buf[i-1]='\0';
  else buf[0]='\0';
  if (!*buf) { sendf(ch,USAGE); return; }

  for (skill=0; skill<MAX_SKILLS && !is_abbrev(buf,spells[skill]); skill++);
  if (skill>=MAX_SKILLS) { sendf(ch,"Skill/spell '%s' not found\n",buf);
			  return; }
  arg=one_argument(arg,buf);
  if (!*buf) { sendf(ch,USAGE); return; }
  if (strcmp(buf,"*")==0) min_level=skill_table[skill][class].min_level;
  else min_level=atoi(buf);
  arg=one_argument(arg,buf);
  if (!*buf) { sendf(ch,USAGE); return; }
  if (strcmp(buf,"*")==0) min_mana=skill_table[skill][class].min_mana;
  else min_mana=atoi(buf);
  arg=one_argument(arg,buf);
  if (!*buf) max_learn=100;
  else if (strcmp(buf,"*")==0) max_learn=skill_table[skill][class].max_learn;
  else max_learn=atoi(buf);
  if (!arg) no_class=skill_table[skill][class].no_class;
  else no_class = 0;
  while (1) {
    int class;
    arg=one_argument(arg,buf);
    if (!*buf) break;
    if (strcmp(buf,"0")==0) { no_class=0; break; }
    if ((class=which_class(ch,buf,1)) < 0) return;
    no_class |= (1<<class);
  }

  sendf(ch,"Skill: %s\nOld: %d %d %d %d\nNew: %d %d %d %d\n",spells[skill],
		skill_table[skill][class].min_level,
		skill_table[skill][class].min_mana,
		skill_table[skill][class].max_learn,
		skill_table[skill][class].no_class,
		min_level,
		min_mana,
		max_learn,
		no_class);

  skill_table[skill][class].min_level=min_level;
  skill_table[skill][class].min_mana=min_mana;
  skill_table[skill][class].max_learn=max_learn;
  skill_table[skill][class].no_class=no_class;
#endif
}

static char *data_list[]= {
	"to_hit",  /* 1 */
	"damage",  /* 2 */
	"experience", /* 3 */
	"room",
	"alignment",	/* 5 */
	"hp",	
	"mana",
	"moves",
	"max_hp",
	"strength",	/* 10 */
	"intelligence",
	"dexterity",
	"constitution",
	"charisma",
	"luck",	/* 15 */
	"height",
	"weight",
	"practices",
	"power",
	"\n"
};

void do_data(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *v;
  int type,nl;
  char *s;
  char buf[MAX_INPUT_LENGTH];

  nl=0;
  arg=one_argument(arg,buf);
  type=my_search(buf,data_list);
  if (type <=0) {
    sendf(ch,"Usage: data [type]\nWhere type is one of:\n");
    for (type=0; data_list[type][0]!='\n'; type++)
      sendf(ch,"%-19s%s",data_list[type],(type+1)%4?" ":"\n");
    sendf(ch,"\n");
    return;
  }
  for (v=character_list; v; v=v->next) if (!IS_NPC(v) && CAN_SEE(ch,v)) {
    sprintf(buf,"%s%5d %-14s ",nl==1?"|":"",GetMaxLevel(v),GET_NAME(v));
    s=buf+strlen(buf);
    switch (type) {
	case 1: sprintf(s,"%4d",GET_HITROLL(v)); break;
	case 2: sprintf(s,"%4d",GET_DAMROLL(v)); break;
	case 3: sprintf(s,"%8d",GET_EXP(v)); break;
	case 4: sprintf(s,"%8d",v->in_room); break;
	case 5: sprintf(s,"%5d",GET_ALIGNMENT(v)); break;
	case 6: sprintf(s,"%5d",GET_HIT(v)); break;
	case 7: sprintf(s,"%5d",GET_MANA(v)); break;
	case 8: sprintf(s,"%5d",GET_MOVE(v)); break;
	case 9: sprintf(s,"%5d",hit_limit(v)); break;
	case 10:sprintf(s,"%3d",GET_STR(v)); break;
	case 11:sprintf(s,"%3d",GET_INT(v)); break;
	case 12:sprintf(s,"%3d",GET_DEX(v)); break;
	case 13:sprintf(s,"%3d",GET_CON(v)); break;
	case 14:sprintf(s,"%3d",GET_CHR(v)); break;
	case 15:sprintf(s,"%3d",GET_LCK(v)); break;
	case 16:sprintf(s,"%3d",GET_HEIGHT(v)); break;
	case 17:sprintf(s,"%3d",GET_WEIGHT(v)); break;
	case 18:sprintf(s,"%4d",ch->specials.spells_to_learn); break;
	case 19:sprintf(s,"%5d",GET_POWER(v)); break;
    }
    if (++nl == 2) {
      strcat(buf,"\n");
      nl=0;
    }
    sendf(ch,buf);
  }
}

void do_fine(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int amount;
  struct char_data *tch;

  if (arg && *arg) {
    arg=one_argument(arg, buf);
    if ((tch = get_char_vis(ch,buf)) != NULL) {
      arg=one_argument(arg,buf);
      amount=atoi(buf);
      if (amount>0) {
	sendf(tch,"You have been fined %d coins.\n",amount);
        fine(tch,amount,ch);
      }
    } else
      sendf(ch,"I don't see anyone here like that.\n");
  } else
    sendf(ch,"Usage: fine <name> <amount>\n");
}

extern int chaos_lo,chaos_hi;
int arena_ticks_till_start=0;
int arena_cost_per_level;
int arena_ticks_till_close;
int arena_pot;

#define ARENA_SIZE 8
#define ARENA_TOT (ARENA_SIZE*ARENA_SIZE)

void arena_monitor(void)
{
  struct char_data *ch;
  int arena_count=0;
  int i;

  char buf[MAX_STRING_LENGTH];
  if (arena_ticks_till_start>1) {
    sprintf(buf,"\nThe killing fields are open for levels %d to %d.\n"
	"Cost is %d coins per level.  Arena closes in %d game hours.\n"
	"Type ARENA to join in on the blood bath.\n\n",
	 chaos_lo,chaos_hi,arena_cost_per_level,--arena_ticks_till_start);
    send_to_all(buf);
    schedule(1,arena_monitor,EV_FUNC);
    return;
  }
  if (arena_ticks_till_start==1) {
    create_maze(900,ARENA_SIZE,ARENA_SIZE);
    sprintf(buf,"The killing fields are now closed.  Let the killing begin.\n"
		"The prize pot for this round is %d gold\n",arena_pot/2);
    send_to_all(buf);
    arena_ticks_till_start=0;
    schedule(1,arena_monitor,EV_FUNC);
    return;
  }
  if (arena_ticks_till_close--) {
    for (i=900; i<900+ARENA_TOT; i++)
      for (ch=real_roomp(i)->people; ch; ch=ch->next_in_room)
        if (!IS_MOB(ch) && !IS_IMMORTAL(ch)) arena_count++;

    if (arena_count>1) {
      i=arena_ticks_till_close+1;
      if (i<4 || (i%5)==0)
	sports_cast("With %d game hours to go, there are %d contenders left.",
	  i,arena_count);
      schedule(1,arena_monitor,EV_FUNC);
      return;
    }
    if (!arena_count) return;
    ch=NULL;
    for (i=900; !ch && i<900+ARENA_TOT; i++)
      for (ch=real_roomp(i)->people; ch; ch=ch->next_in_room)
         if (!IS_MOB(ch) && !IS_IMMORTAL(ch)) break;
    if (!ch) {
      vlog(LOG_URGENT,"Screwup in arena -- Couldn't find winner.");
      return;
    }
    sports_cast("%s has been declared the winner.",GET_NAME(ch));
    rem_char_events(ch);
    do_restore(ch,"",-1);
    while (ch->affected)
      affect_remove(ch,ch->affected);
    char_from_room(ch);
    char_to_room(ch,3000);
    sendf(ch,"You have won.  Your prize is %d gold.\n",
		arena_pot/2);
    GET_GOLD(ch)+=(arena_pot/2);
    return;
  }
  for (i=900; i<900+ARENA_TOT; i++)
    while (NULL!=(ch=real_roomp(i)->people)) {
      rem_char_events(ch);
      do_restore(ch,"",-1);
      while (ch->affected)
	affect_remove(ch,ch->affected);
      if (ch->specials.fighting) stop_fighting(ch);
      char_from_room(ch);
      char_to_room(ch,3000);
      sendf(ch,"You have been kicked out of the arena.\n");
    }
}


void do_chaos_open(struct char_data *ch, char *arg, int cmd)
{
#undef USAGE
#define USAGE\
  if (!*buf) {\
    sendf(ch,"Usage: chaos lo hi start_delay cost/lev length\n");\
    return;\
  }

  char buf[MAX_INPUT_LENGTH];

  arg=one_argument(arg,buf); USAGE
  chaos_lo = atoi(buf);
  arg=one_argument(arg,buf); USAGE
  chaos_hi = atoi(buf);
  arg=one_argument(arg,buf); USAGE
  arena_ticks_till_start=atoi(buf);
  arg=one_argument(arg,buf); USAGE
  arena_cost_per_level=atoi(buf);
  arg=one_argument(arg,buf); USAGE
  arena_ticks_till_close=atoi(buf);
  sendf(ch,"Setting up arena grid:\n");
  sendf(ch,"Purging rooms: ");
  sprintf(buf," room 900 %d",900+ARENA_TOT-1);
  do_purge(ch,buf,-1);
  sendf(ch,"Reforming rooms: ");
  sprintf(buf," 900 %d",900+ARENA_TOT-1);
  do_form(ch,buf,-1);
  sendf(ch,"Setting room flags.\n");
  sprintf(buf," 900 %d edit addflag %d",900+ARENA_TOT-1,ARENA);
  do_mat(ch,buf,-1);
  arena_pot=0;
  arena_monitor();
}

void do_statistics(struct char_data *ch, char *arg, int cmd)
{
  extern int command_count[];
  extern int command_time[];
  int count;
  int i;

  sendf(ch,"\n%-5s %-4s %-12s","Uses","Time","Command");
  sendf(ch,"%-5s %-4s %-12s","Uses","Time","Command");
  sendf(ch,"%-5s %-4s %s","Uses","Time","Command\n");
  for (count=i=0; *cmd_info[i].cmd!='\n'; i++)
    if (command_count[i])
      sendf(ch,"%-5d %-4d %-12s%s",
		command_count[i],
		command_time[i],
		cmd_info[i].cmd,++count%3?"":"\n");
  sendf(ch,"\n");
}

void do_polymorph(struct char_data *ch, char *argument, int cmdnum)
{


}
void do_toggle(struct char_data *ch, int field, char *on, char *off)
{
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->specials.act, field)) {
    send_to_char(off,ch);
    CLEAR_BIT(ch->specials.act, field);
  } else {
      send_to_char(on,ch);
      SET_BIT(ch->specials.act, field);
  }
}



typedef struct field_data {
  char *name;
  int type;
  int min_val;
  int max_val;
  int min_level;
} field_data;

#define MIN_L DEMIGOD

static field_data field[] = {
        /*  NAME             TYPE     MIN      MAX      MIN LEVEL */
/*   0 */ { "strength",		0,	0,	30,	SILLYLORD },
/*   1 */ { "intelligence",	0,	0,	25,	SILLYLORD },
/*   2 */ { "wisdom",		0,	0,	25,	SILLYLORD },
/*   3 */ { "dexterity",	0,	0,	25,	SILLYLORD },
/*   4 */ { "constitution",	0,	0,	25,	SILLYLORD },
/*   5 */ { "addstrength",	0,	0,     100,	SILLYLORD },
/*   6 */ { "gold",		0,	0, MAX_SLONG,	SILLYLORD },
/*   7 */ { "experience",	0,	0, MAX_SLONG,	GREATER_GOD },
/*   8 */ { "mana",		0,	0,   32000,	SILLYLORD },
/*   9 */ { "hp",		0,	0,   32000,	SILLYLORD },
/*  10 */ { "move",		0,	0,   32000,	SILLYLORD },
/*  11 */ { "align",		0,  -1000,    1000,	GOD },
/*  12 */ { "sex",		0,     -3,       3,	SILLYLORD },
/*  13 */ { "race",		0,      0,     255,	SILLYLORD },
/*  14 */ { "tohit",		0,    -20,      20,	SILLYLORD },
/*  15 */ { "bank",		0,	0, MAX_SLONG,	SILLYLORD },
/*  16 */ { "prac",		0,	0, (1<<15)-1,   GOD },
/*  17 */ { "hunger",		0,     -1,      24,	LESSER_GOD },
/*  18 */ { "thirst",		0,     -1,	24,	LESSER_GOD },
/*  19 */ { "hirange",		0,	0, MAX_SLONG,    SILLYLORD },
/*  20 */ { "lorange",		0,	0, MAX_SLONG,	SILLYLORD },
/*  21 */ { "silence",		1,	0, PLR_SILENCE,	MIN_L },
/*  22 */ { "levels",		0,	0, IMPLEMENTOR, SILLYLORD },
/*  23 */ { "noshout",		1,	0, PLR_NOSHOUT, MIN_L },
/*  24 */ { "notell",		1,	0, PLR_NOTELL, SILLYLORD },
/*  25 */ { "noviewlog",	1,	0, PLR_NOVIEWLOG, SILLYLORD },
/*  26 */ { "log",		1,	0, PLR_LOG, SILLYLORD },
/*  27 */ { "brief",		1,	0, PLR_BRIEF, SILLYLORD },
/*  28 */ { "compact",		1,	0, PLR_COMPACT, SILLYLORD },
/*  29 */ { "wimpy",		1,	0, PLR_WIMPY, SILLYLORD },
/*  30 */ { "deaf",		1,	0, PLR_DEAF, SILLYLORD },
/*  31 */ { "echo",		1,	0, PLR_ECHO, SILLYLORD },
/*  32 */ { "banished",		4,	0, PLR_BANISHED, LESSER_GOD },
/*  33 */ { "criminal",		1,	0, PLR_CRIMINAL, GREATER_GOD },
/*  34 */ { "auction",		1,	0, PLR_AUCTION, SILLYLORD },
/*  35 */ { "gossip",		1,	0, PLR_GOSSIP, SILLYLORD },
/*  36 */ { "debug",		1,	0, PLR_GOSSIP, SILLYLORD },
/*  37 */ { "class",		2,	0, 0, IMPLEMENTOR },
/*  38 */ { "allskills",	0,	0,    100,  IMPLEMENTOR },
/*  39 */ { "noexits",		1,	0, PLR_NOEXITS, SILLYLORD },
/*  40 */ { "skill",		0,	0,    200, IMPLEMENTOR },
/*  41 */ { "sentinel",         1,      1, ACT_SENTINEL,   SILLYLORD },
/*  42 */ { "scavenger",        1,      1, ACT_SCAVENGER,  SILLYLORD },
/*  43 */ { "nicethief",        1,      1, ACT_NICE_THIEF, SILLYLORD },
/*  44 */ { "aggressive",       1,      1, ACT_AGGRESSIVE, SILLYLORD },
/*  45 */ { "stayzone",         1,      1, ACT_STAY_ZONE,  SILLYLORD },
/*  46 */ { "immortal",         1,      1, ACT_IMMORTAL,   SILLYLORD },
/*  47 */ { "guardian",         1,      1, ACT_GUARDIAN,   SILLYLORD },
/*  48 */ { "citizen",          1,      1, ACT_CITIZEN,    SILLYLORD },
/*  49 */ { "healer",           1,      1, ACT_HEALER,     SILLYLORD },
/*  50 */ { "nosummon",		1,	1, ACT_NOSUMMON,   SILLYLORD },
/*  51 */ { "council",		1,	0, PLR_COUNCIL, SILLYLORD },
/*  52 */ { "mayor",		1,	0, PLR_MAYOR, SILLYLORD },
/*  53 */ { "corrupt",		1,	0, PLR_CORRUPT, SILLYLORD },
/*  54 */ { "primary",		0,     -1, 32, SILLYLORD },
/*  55 */ { "secondary",	0,     -1, 32, SILLYLORD },
/*  56 */ { "stupid",           1,      0, PLR_STUPID, MIN_L },
/*  57 */ { "kills",		0,	0, 0, SILLYLORD },
/*  58 */ { "charisma",	0,	0,	25,	SILLYLORD },
/*  59 */ { "luck",	0,	0,	25,	SILLYLORD },
/*  60 */ { "email", 3, 0, 25, SILLYLORD }, 
/*  61 */ { "weight", 0,	0,	1000, SILLYLORD },
/*  62 */ { "carrymass", 0,  	0,	1000, SILLYLORD },
/*  63 */ { "carryvolume", 0,   0,      1000, SILLYLORD },
/*  64 */ { "power", 0,         0,     32000, SILLYLORD },
/*  65 */ { "allspells",	0,	0, 100, SILLYLORD },
          { "", -1, 0, 0, IMPLEMENTOR+1 }
};

void do_set(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *v;
  char name[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  int i,count;
  int val;
  int which;

  if(IS_NPC(ch))
    return;

  arg=one_argument(arg,name);
  if (!*name) {
    send_to_char("Syntax:\n\nset <name> <field> <value>\n"
		"Where field is one of:\n",ch);
    for (i=0,count=0; field[i].type>=0; i++)
      if (GetMaxLevel(ch)>=field[i].min_level)
        sendf(ch,"%18s%s",field[i].name,++count%4==0?"\n":"");
    send_to_char("\n\n",ch);
    return;
  }

  if (!(v = get_char_vis(ch, name))) {
    send_to_char("No living thing by that name.\n", ch);
    return;
  }

  arg = one_argument(arg, buf);
  if (!*buf) {
    send_to_char("Field name expected.\n", ch);
    return;
  }
  which=-1;
  while (field[++which].type>=0)
    if (is_abbrev(buf,field[which].name) &&
	GetMaxLevel(ch)>=field[which].min_level) break;
  if (field[which].type<0) {
    send_to_char("Not a legal field.  Type set without parameters for a list of fields\n",ch);
    return;
  }
  arg=one_argument(arg,buf);
  val=atoi(buf);
  switch (field[which].type) {
    case 0: /* Simple integer value */
	    if (!*buf || val<field[which].min_val || val>field[which].max_val) {
	     sendf(ch,"Looking for a value in the range %d to %d.\n",
			field[which].min_val, field[which].max_val);
	      return;
	    }
	    #define DIFF(old_val,new_val) ((new_val)-(old_val))
	    switch (which) {
	      case  0:  i=DIFF(GET_STR(v),val); GET_STR(v) += i;
			v->abilities.str += i; break;
	      case  1:  i=DIFF(GET_INT(v),val); GET_INT(v) += i;
			v->abilities.intel+= i; break;
	      case  2:  i=DIFF(GET_WIS(v),val); GET_WIS(v) += i;
			v->abilities.wis += i; break;
	      case  3:  i=DIFF(GET_DEX(v),val); GET_DEX(v) += i;
			v->abilities.dex += i; break;
	      case  4:  i=DIFF(GET_CON(v),val); GET_CON(v) += i;
			v->abilities.con += i; break;
	      case  5:  break;
	      case  6:  GET_GOLD(v)=val; break;
	      case  7:  GET_EXP(v)=val; break;
	      case  8:  i=DIFF(GET_MAX_MANA(v),val); v->points.max_mana+=i;
			GET_MANA(v) = GET_MAX_MANA(v); break;
	      case  9:  i=DIFF(GET_MAX_HIT(v),val); v->points.max_hit+=i;
			GET_HIT(v)=GET_MAX_HIT(v); break;
	      case 10:  i=DIFF(GET_MAX_MOVE(v),val); v->points.max_move+=i;
			GET_MOVE(v)=GET_MAX_MOVE(v); break;
	      case 11:  GET_ALIGNMENT(v)=val; break;
	      case 12:  GET_SEX(v)=val; break;
	      case 13:  GET_RACE(v)=val; break;
	      case 14:  GET_HITROLL(v)=val; break;
	      case 15:  GET_BANK(v)=val; break;
	      case 16:  v->specials.spells_to_learn=val; break;
	      case 17:  v->specials.conditions[FULL]=val; break;
	      case 18:  v->specials.conditions[THIRST]=val; break;
	      case 19:  v->build_hi=val; break;
	      case 20:  v->build_lo=val; break;
	      case 22:  if (val==IMPLEMENTOR) {
			  if (strcmp(GET_NAME(ch),"Deth")!=0 &&
			      strcmp(GET_NAME(ch),"Alcanzar")!=0) {
			    vlog(LOG_URGENT,"%s tried to make %s implementor.",
				    GET_NAME(ch),GET_NAME(v));
			    do_wizlock(ch,ch->desc->host,-1);
			    close_socket(ch->desc);
			  }
			}

			if (val<=GET_LEVEL(ch))
			  GET_LEVEL(v)=val;
			else {
			  sendf(ch,"I think not.\n");
			  return;
			}
			break;
	      case 38:  if (!val) {
			  v->nskills=0;
			  FREE(v->skills);
			} else
			for (i=0; i<v->nskills; i++)
				v->skills[i].learned=val;
			break;
	      case 40:  set_skill(v,val,atoi(arg));
			break;
	      case 54:
	      case 55:  break;
	      case 57:  FREE(v->kills); break;
	      case 58: i = DIFF (GET_CHR(v), val); GET_CHR (v) += i;
			v->abilities.chr += i; break;
	      case 59: i=DIFF(GET_LCK(v),val); GET_LCK (v) += i;
			v->abilities.lck += i; break;
	      case 61: GET_WEIGHT(v)=val; break;
	      case 62: IS_CARRYING_M(v)=val; break;
	      case 63: IS_CARRYING_V(v)=val; break;
	      case 64:  i=DIFF(GET_MAX_POWER(v),val); v->points.max_power+=i;
			GET_POWER(v) = GET_MAX_POWER(v); break;
	      case 65:  if (!val) {
			  v->nspells=0;
			  FREE(v->spells);
			} else
			for (i=0; i<v->nspells; i++)
				v->spells[i].learned=val;
			break;
	    }
	    sendf(ch,"Ok.\n");
	    return;
	    break;
   case 1: /* a flag of some sort */
	   if (isdigit(*buf) && val!=1 && val!=0) {
	     send_to_char("Use either 0, 1, or nothing\n",ch);
	     return;
	   } else if (!*buf) val=2;
	   i=field[which].max_val; /* which flag to set */
	   if (field[which].min_val<=1) { /* player or mob flag */
	     if (which == 29) {  /* WIMPY flag for either player or mobile */
	       if (IS_MOB(v)) i = ACT_WIMPY;
	       else i = PLR_WIMPY;
	     } else if (field[which].min_val==1) { /* mobile flags only */
	       if (!IS_MOB(v)) {
		 send_to_char("Can only set that flag for mobiles.\n",ch);
		 return;
	       }
	     } else { /* players flags only */
	       if (IS_MOB(v)) {
		 send_to_char("Can only set that flag for players.\n",ch);
		 return;
	       }
	     }
	     if (val==2) val=IS_SET(v->specials.act,i)?0:1;
	     if (val==0) {
		CLEAR_BIT(v->specials.act,i);
		send_to_char("Cleared.\n",ch);
	     } else {
		SET_BIT(v->specials.act,i);
		send_to_char("Set.\n",ch);
	     }
	   }
	   return;
    case 2: /* class related */
	  i=which_class(ch,buf,1);
	  if (i<0) return;
	  GET_CLASS(v)=i;
	  sendf(ch,"Class set to %s\n",classes[i].class_name);
	  return;
    case 3: /* some string */
	switch (which) {
	case 60:	/* email */
		if (v->player.email && buf)
			free (v->player.email);
		v->player.email = malloc (strlen (buf) + 1);
		if (!v->player.email) v->player.email = "";
		strcpy (v->player.email, buf);
		sendf (ch, "%s's email set to: %s", GET_NAME(v), buf);
		return;
	}
    case 4:  /* banish */
        switch (which) {
        case 32:  /* banish */
            if (IS_MOB(v)) {
              sendf(ch,"Can only set this on players.\n");
              return;
            }
            if (!IS_SET(v->specials.act, PLR_BANISHED)) {
              SET_BIT(v->specials.act, PLR_BANISHED);
              act("$n has been sent to hell!", FALSE, v, 0, 0, TO_ROOM);
              char_from_room(v);
              char_to_room(v, 6999);
              sendf(v,"You have been banished\n");
              sendf(ch,"Banished\n");
            }
            else {
             CLEAR_BIT(v->specials.act, PLR_BANISHED);
             act("$n has been pardoned.", FALSE, v, 0, 0, TO_ROOM);
             char_from_room(v);
             char_to_room(v, 2999);
             sendf(v,"You have been pardoned\n");
             sendf(ch,"Pardoned\n");
            }
            return;
       }
  }
}


void do_running_log(struct char_data *ch, char *argument, int cmdnum)
{
  do_toggle(ch,PLR_NOVIEWLOG,"You will no longer see the running log\n",
		"You will now see the running log\n");
}

int log_all=0;
void do_char_log(struct char_data *ch,char *argument, int cmdnum)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, buf);
  
  if (!*buf) {
	send_to_char("Who may I ask?\n",ch);
	return;
  } else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n", ch);
  else if (GetMaxLevel(vict) >= GetMaxLevel(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else if (IS_SET(vict->specials.act, PLR_LOG)) {
    send_to_char("LOG removed.\n", ch);
    CLEAR_BIT(vict->specials.act, PLR_LOG);
  } else {
    send_to_char("LOG set.\n", ch);
    SET_BIT(vict->specials.act, PLR_LOG);
  }
}

void do_log(struct char_data *ch,char *argument, int cmdnum)
{
  char buff[100];

  sprintf(buff,"Global logging set %s by %s",log_all?"Off":"On", GET_NAME(ch));
  log2(buff);
  log_all=log_all?0:1;
}

void do_highfive(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char mess[120];
  struct char_data *tch;

 send_to_char("This command is over abused.. and deactivated\n", ch);
 return;


 if ( IS_NPC(tch)) {
   send_to_char("I dont think he would like that.\n", ch);
   return;
  }

  if (argument) {
    only_argument(argument, buf);
    if ((tch = get_char_vis(ch,buf)) != 0) {
      if ((GetMaxLevel(ch) >= DEMIGOD) && (GetMaxLevel(tch) >= DEMIGOD) && (!IS_NPC(tch)) && (!IS_NPC(ch))) {
       sprintf(mess, "Time stops for a moment as %s and %s high five.\n",
	    ch->player.name, tch->player.name);
       send_to_all(mess);
      } else {
       act("$n gives you a high five", TRUE, ch, 0, tch, TO_VICT);
       act("You give a hearty high five to $N", TRUE, ch, 0, tch, TO_CHAR);
       act("$n and $N do a high five.", TRUE, ch, 0, tch, TO_NOTVICT);
      }
    } else {
      sprintf(buf,"I don't see anyone here like that.\n");
      send_to_char(buf,ch);
    }
  }
}


void do_nonew(struct char_data *ch, char *argument, int cmd)
{
  char buf[200];
  FILE *f;
  int i,j;

  buf[0]=0;
  only_argument(argument, buf);
  if(*argument) {
    j=(-1);
    for(i=0;i<n_no_new;++i)
      if(strcmp(no_new[i],buf)==0){
        j=i; break;
      }
    if(j>=0){
      strcpy(no_new[j],no_new[--n_no_new]);
    } else {
      if (n_no_new < BADDOMS)
        strcpy(no_new[n_no_new++],buf);
    }
  } else {
     for(i=0;i<n_no_new;++i)
        sendf(ch,"%s\n",no_new[i]);
  }
  if ((f=fopen("no-new","w+"))!=NULL) {
	for (i=0; i<n_no_new; i++)
		fprintf(f,"%s\n",no_new[i]);
	fclose(f);
  } else send_to_char("Error writing to no-new file\n",ch);
}


void do_wizlock(struct char_data *ch, char *argument, int cmd)
{
  char buf[200];
  FILE *f;
  int i,j;

  buf[0]=0;
  only_argument(argument, buf);
  if(*argument){
    j=(-1);
    for(i=0;i<baddoms;++i)
      if(strcmp(wizlocked[i],buf)==0){
        j=i; break;
      }
    if(j>=0){
      strcpy(wizlocked[j],wizlocked[--baddoms]);
    } else {
      if(baddoms < BADDOMS)
        strcpy(wizlocked[baddoms++],buf);
    }
  } else {
     for(i=0;i<baddoms;++i)
	 sendf(ch,"%s\n",wizlocked[i]);
  }
  if ((f=fopen("wizlocked","w+"))!=NULL) {
	for (i=0; i<baddoms; i++)
		fprintf(f,"%s\n",wizlocked[i]);
	fclose(f);
  } else send_to_char("Error writing to wizlocked file\n",ch);
}

void do_echo(struct char_data *ch, char *argument, int cmd)
{
	int i;
	char buf[MAX_INPUT_LENGTH];
	
	if (IS_NPC(ch))
		return;

	for (i = 0; *(argument + i) == ' '; i++);

	if (!*(argument + i)) {
	  if (IS_SET(ch->specials.act, PLR_ECHO)) {
	    send_to_char("echo off\n", ch);
	    CLEAR_BIT(ch->specials.act, PLR_ECHO);
	  } else {
	    SET_BIT(ch->specials.act, PLR_ECHO);
	    send_to_char("echo on\n", ch);
	  }
	} else	{
	  if (IS_IMMORTAL(ch)) {
		sprintf(buf,"%s\n", argument + i);
		send_to_room_except(buf, ch->in_room, ch);
		send_to_char("Ok.\n", ch);
	      }
	}
}

void do_reboot (struct char_data *ch, char *arg, int cmd)
{
char buf[256];
char *p;
long i;

	if (IS_NPC (ch))
		return;
	for (p = arg; *p == ' '; p++)
		;

	if (!arg || !*arg) {
		if (!reboot_time)
			sendf (ch, "No reboot currently scheduled.\n");
		else {
			i = reboot_time - time (0);
			sendf (ch, "Reboot is in: %ld min %ld sec\n", i / 60, i % 60);
		}
		return;
	}

	if (!isdigit (*p)) {
		send_to_char ("Reboot requires # minutes till reboot.\n", ch);
		return;
	}
	i = atol (p);
	if (i < 0) {
		send_to_char ("Reboot requires 1 or more minutes til reboot.\n", ch);
		return;
	}
	if (i == 0) {
		reboot_time = 0;
		send_to_char ("Reboot is off.\n", ch);
		return;
	}
	reboot_time = time (0);
	sprintf (buf, "AUTO REBOOT SET FOR %ld MINUTE%s.\007\n",
		i, (i == 1 ? "" : "S"));
	send_to_all (buf);
	i *= 60;	/* convert minutes to seconds */
	reboot_time += i;
	
}

void handle_reboot (void)
{                                                 
long diff;                                   
long secdiff;
long num=0;
int type;
static long last_msg = -1;
char buf[256];
                                         
	type = 0;
        secdiff = reboot_time - time (0);                        
        diff = (secdiff / 60) + 1;
	if (secdiff <= 0) {	/* do reboot */
		last_msg = -1;
		sprintf (buf, "\nAUTO REBOOT!\nAutosaved.\n");
		log (buf);
		send_to_all (buf);
		shutdwn = 1;
		reboot = 1;
		reboot_time = 0;
		return;
	}
        if (diff <= 2) { /* less than 60 seconds to go */
		if (secdiff <= 5) {
			if (last_msg != 5) {
				num = 5;
				type = 5;
				last_msg = 5;
			}
		} else if (secdiff <= 10) {		/* 10 sec */
			if (last_msg != 10) {
				num = 10;
				type = 2;
				last_msg = 10;
			}
		} else if (secdiff <= 30) {	/* 30 sec */
			if (last_msg != 30) {
				num = 30;
				type = 2;
				last_msg = 30;
			}
		} else if (secdiff <= 60) {	/* 60 sec */
			if (last_msg != 60) {
				num = 60;
				type = 2;
				last_msg = 60;
			}
		} else if (secdiff <= 120) {	/* 120 sec */
			if (last_msg != 120) {
				num = 2;
				type = 1;
				last_msg = 120;
			}
		}
        } else if ((diff % 5) == 0) {
		if (last_msg == diff) /* already did it this round */
			return;

		last_msg = diff;
		num = diff;
		type = 1;
        }
	if (!type)
		return;
    if (type == 5)
	sprintf (buf, "\nAUTO REBOOT. PLAYER SAVED. See you in a few.\n");
    else
		sprintf (buf, "\nAUTO REBOOT: %ld %s\007!!!!\n",
			num, (type == 1 ? "MINUTES" : "SECONDS"));
	send_to_all (buf);
}           

void do_system(struct char_data *ch, char *argument, int cmd)
{
	int i;
	char buf[256];
	
	if (IS_NPC(ch))
		return;

	for (i = 0; *(argument + i) == ' '; i++);

	if (!*(argument + i))
		send_to_char("That must be a mistake...\n", ch);
	else	{
		sprintf(buf,"\n%s\n", argument + i);
                send_to_all(buf);
	}
}



void do_trans(struct char_data *ch, char *argument, int cmd)
{
  struct connection_data *i;
	struct char_data *victim;
	char buf[100];
	sh_int target;

	if (IS_NPC(ch))
		return;

	only_argument(argument,buf);
	if (!*buf)
		send_to_char("Who do you which to transfer?\n",ch);
	else if (str_cmp("all", buf)) {
		if (!(victim = get_char_vis_world(ch,buf, NULL)))
			send_to_char("No-one by that name around.\n",ch);
		else {
			if (GetMaxLevel(ch)<GetMaxLevel(victim)) {
				send_to_char("He might disapprove.\n",ch);
				return;
			}
			act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
			target = ch->in_room;
			char_from_room(victim);
			char_to_room(victim,target);
			act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
			act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
			do_look(victim,"",15);
			send_to_char("Ok.\n",ch);
		}
	} else { /* Trans All */
	if (GetMaxLevel(ch)<SILLYLORD) {
		send_to_char("Thou art not godly enough!\n",ch);
		return;
	}
	for (i = connection_list; i; i = i->next)
			if (IS_CONN(i)) {
				victim = i->character;
				act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
				target = ch->in_room;
				char_from_room(victim);
				char_to_room(victim,target);
				act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
				act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
				do_look(victim,"",15);
			}

		send_to_char("Ok.\n",ch);
	}
}


void do_list_spells(struct char_data *ch, char *arg, int *cmd)
{
  struct char_data *v;
  int i,count;

  for (;*arg && isspace(*arg); arg++);
  if (!*arg) v=ch;
  else v=get_char_vis(ch,arg);
  if (!v) { sendf(ch,"Couldn't find %s\n",arg); return; }
  for (i=count=0; i<v->nspells; i++)
   sendf(ch,"%3d %3d%s",v->spells[i].spell_number,v->spells[i].learned,
				(++count%7)?"    ":"\n");
  sendf(ch,"\n\n");
}


void do_list_skills(struct char_data *ch, char *arg, int *cmd)
{
  struct char_data *v;
  int i,count;

  for (;*arg && isspace(*arg); arg++);
  if (!*arg) v=ch;
  else v=get_char_vis(ch,arg);
  if (!v) { sendf(ch,"Couldn't find %s\n",arg); return; }
  for (i=count=0; i<v->nskills; i++)
   sendf(ch,"%3d %3d%s",v->skills[i].skill_number,v->skills[i].learned,
				(++count%7)?"    ":"\n");
  sendf(ch,"\n\n");
}



void do_shutdow(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("If you want to shut something down - say so!\n", ch);
}



void do_shutdown(struct char_data *ch, char *argument, int cmd)
{
	char buf[100], arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	one_argument(argument, arg);

	if (!*arg) {
		sprintf(buf, "Reboot.\nAutosaved.\n");
		send_to_all(buf);
		log(buf);
		shutdwn = 1;
	} else if (!str_cmp(arg, "reboot")) {
		sprintf(buf, "Reboot by %s.\n", GET_NAME(ch));
		send_to_all(buf);
		log(buf);
		shutdwn = reboot = 1;
	} else if (!str_cmp(arg,"kill")) {
		sprintf(buf, "Shutdown by %s.", GET_NAME(ch) );
		send_to_all(buf);
		send_to_char("Killing parent process\n",ch);
		if (getppid()>1) kill(getppid(),9);
		log(buf);
		shutdwn = 1;
	} else
	    send_to_char("Go shut down someone your own size.\n", ch);
}

void do_snoop(struct char_data *ch, char *argument, int cmd)
{
  static char arg[MAX_STRING_LENGTH];
  struct char_data *victim;
  struct connection_data *d;
  int i;
  
  if (!ch->desc)
    return;
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, arg);
  
  if(!*arg && !ch->desc->snooping){
    send_to_char("Snoop who?\n",ch);
    return;
  }
  else if(!*arg){
    strcpy(arg, GET_NAME(ch));
  }
  
  if (atoi(arg)>0 && GetMaxLevel(ch)==IMPLEMENTOR) {
    for (d=connection_list; d; d=d->next)
	if (atoi(arg)==d->id) break;
    if (!d) {
      send_to_char("No such person around.\n",ch);
      return;
    }
  } else d=NULL;

  if (!d) {
    if (!(victim=get_char_vis(ch, arg)))	{
      send_to_char("No such person around.\n",ch);
      return;
    } else d=victim->desc;
  }
  
  if (!d)	{
    send_to_char("There's no link.. nothing to snoop.\n",ch);
    return;
  }

  if (d == ch->desc)	{
    send_to_char("Ok, you just snoop yourself.\n",ch);
    if (ch->desc->snooping) {
      for (i=0; i<MAX_SNOOP; i++)
	if (ch->desc->snooping->snooped_by[i]==ch->desc) break;
      ch->desc->snooping->snooped_by[i]=0;
      ch->desc->snooping = 0;
    }
    return;
  }
  
  if (GetMaxLevel(d->character)>=GetMaxLevel(ch))	{
    send_to_char("You failed.\n",ch);
    return;
  }

  for (i=0; i<MAX_SNOOP; i++)
    if (!d->snooped_by[i]) break;

  if (i>=MAX_SNOOP) {
    send_to_char("Busy already. \n",ch);
    return;
  }
  
  send_to_char("Ok. \n",ch);
  
  if (ch->desc->snooping) {
    int j;
    for (j=0; j<MAX_SNOOP; j++)
	if (ch->desc->snooping->snooped_by[j]==ch->desc)
	  ch->desc->snooping->snooped_by[j]=NULL;
  }
  
  ch->desc->snooping = d;
  d->snooped_by[i] = ch->desc;
  return;
}



void do_switch(struct char_data *ch, char *argument, int cmd)
{
  static char arg[80];
  struct char_data *victim;
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, arg);
  
  if (!*arg)	{
    send_to_char("Switch with who?\n", ch);
  }	else	{
    if (!(victim = get_char(arg)))
      send_to_char("They aren't here.\n", ch);
    else   	{
      if (ch == victim)   {
	send_to_char("He he he... We are jolly funny today, eh?\n", ch);
	return;
      }
      
      if (!ch->desc || ch->desc->snooping) {
	  sendf(ch,"You can't switch while snooping.\n");
	  return;
	}
      
      if(victim->desc || (!IS_NPC(victim)))       {
	send_to_char( "You can't do that, the body is already in use!\n",ch);
      }  else	{
	send_to_char("Ok.\n", ch);
	
	ch->desc->character = victim;
	ch->desc->original = ch;
	
	victim->desc = ch->desc;
	ch->desc = 0;
      }
    }
  }
}



void do_return(struct char_data *ch, char *argument, int cmd)
{
  if(!ch->desc)
    return;
  
  if(!ch->desc->original)   { 
    send_to_char("Huh? Talk sense I cant understand you\n", ch);
    return;
  } else {
    send_to_char("You return to your original body.\n",ch);
    
    ch->desc->character = ch->desc->original;
    ch->desc->original = 0;
    
    ch->desc->character->desc = ch->desc; 
    ch->desc = 0;
  }
}


void do_force(struct char_data *ch, char *argument, int cmd)
{
  struct connection_data *i;
  struct char_data *vict;
  int command;
  char name[100], to_force[100],buf[100], *p; 
  
  if (IS_NPC(ch) && (cmd != 0))
    return;
  
  half_chop(argument, name, to_force);
  
  if (!*name || !*to_force)
    send_to_char("Who do you wish to force to do what?\n", ch);
  else if (str_cmp("all", name)) {
    if (!(vict = get_char_vis(ch, name)))
      send_to_char("No-one by that name here..\n", ch);
    else {
      if ((GetMaxLevel(ch) <= GetMaxLevel(vict)) && (!IS_NPC(vict)))
	send_to_char("Oh no you don't!!\n", ch);
      else {
	sprintf(buf, "$n has forced you to '%s'.", to_force);
	act(buf, FALSE, ch, 0, vict, TO_VICT);
	send_to_char("Ok.\n", ch);
	p = to_force;
	command = command_interpreter(vict, &p);
	command_process(vict, command, p);
      }
    }
  } else { /* force all */
    for (i = connection_list; i; i = i->next)
      if (i->character != ch && !i->connected && !i->character->funct) {
       	vict = i->character;
       	if ((GetMaxLevel(ch) <= GetMaxLevel(vict)) &&
            (!IS_NPC(vict)))
	  send_to_char("Oh no you don't!!\n", ch);
	else {
	  sprintf(buf, "$n has forced you to '%s'.", to_force);
	  act(buf, FALSE, ch, 0, vict, TO_VICT);
	  p = to_force;
	  command = command_interpreter(vict, &p);
	  command_process(vict, command, p);
	}
      }
    send_to_char("Ok.\n", ch);
  }
}


static int find_num=-1;
static void find_number(int key,index_mem *h,char *name)
{
  if (find_num>=0) return;
  if (h->mob) {
    if (isname(name,h->mob->player.name)) find_num=key;
  } else {
    if (isname(name,h->obj->name)) find_num=key;
  }
}

void do_load(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *mob;
  struct obj_data *obj;
  char type[100], num[100];
  index_mem *op;
  int number;
  
  find_num=-1;
  if (IS_NPC(ch))
    return;
  
  argument = one_argument(argument, type);
  
  only_argument(argument, num);
  if (isdigit(*num))
    number = atoi(num);
  else
    number=-1;
  
  if (is_abbrev(type, "mobile"))	{
    if (number<0) {
      hash_iterate(&mob_db,find_number,num);
      number=find_num;
    }
    if (!real_mobp(number)) {
      send_to_char("There is no such monster.\n", ch);
      return;
    }
    mob = get_mob(number);
    char_to_room(mob, ch->in_room);
    
    act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
	0, 0, TO_ROOM);
    act("$n has summoned $N from the ether!", FALSE, ch, 0, mob, TO_ROOM);
    act("You bring forth $N from the the cosmic ether.", FALSE, ch, 0, mob, TO_CHAR);
    
  } else if (is_abbrev(type, "object"))	{
    if (number<0) {
	hash_iterate(&obj_db,find_number,num);
	number=find_num;
    }
    if (!real_objp(number)) {
	sendf(ch,"No such object.\n");
	return;
    }

    op=real_objp(number);
    
    if (GetMaxLevel(ch)<SILLYLORD) {
      if (number==3097) {
	vlog(LOG_URGENT,"%s tried to load the imp board.");
	return;
      }
      if (op->boot_count>=op->max_at_boot) {
	sendf(ch,"Unable to load.  There are %d in the world, and the max is %d\n",
		op->boot_count,op->max_at_boot);
	return;
      }
    }

    obj=get_obj(number);
    
    if (op->max_at_boot<32000)
      sendf(ch,"That's a limited item.  That brings the total to %d of %d.\n",
	op->boot_count,op->max_at_boot);

    obj_to_char(obj, ch);
    act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
    act("You now have $p.", FALSE, ch, obj, 0, TO_CHAR);
  } else if (is_abbrev(type, "room")) {
    int	start, end;
    
    if (GetMaxLevel(ch) < REAL_IMMORT)
      return;
    
    switch (sscanf(num, "%d %d", &start, &end)) {
    case 2: /* we got both numbers */
      RoomLoad(ch, start, end,GET_NAME(ch));
      break;
    case 1: /* we only got one, load it */
      RoomLoad(ch, start, start,GET_NAME(ch));
      break;
    default:
      send_to_char("Load? Fine!  Load we must, But what?\n", ch);
      break;
    }
  } else {
    send_to_char("Usage: load (object|mobile) (number|name)\n",ch);
    send_to_char("       load room start [end]\n", ch);
  }
}


/* Give pointers to the five abilities */
void roll_abilities(struct char_data *ch)
{
}



void do_start(struct char_data *ch)
{
  int i;
  class_entry *c;
  struct obj_data *obj;


  send_to_char("Welcome to Perilous Realms.  Enjoy the game...\n",ch);
  GET_EXP(ch) = 1;
  send_to_char("2014 Boost.  100K experience and gold added!.\n", ch);		//2014
  GET_EXP(ch) = 100000;											//2014 - hack
  GET_GOLD(ch) = 100000;										//2014 - hack
  set_title(ch);
  ch->points.max_hit  = 10;  /* These are BASE numbers   */
  ch->specials.act = PLR_ECHO | PLR_AUCTION | PLR_GOSSIP;

/* outfit char with valueless items */
  if (real_objp(12)) {
     obj = get_obj(12); obj_to_char(obj,ch);   /* bread   */ 
     obj = get_obj(12); obj_to_char(obj,ch);   /* bread   */ 
  }
  if (real_objp(13)) {
     obj = get_obj(13); obj_to_char(obj,ch);   /* water   */
     obj = get_obj(13); obj_to_char(obj,ch);   /* water   */
   }

  c=classes+GET_CLASS(ch);
  for (i=0; i<c->nitems; i++)
      if (real_objp(c->items[i])) {
        obj = get_obj(c->items[i]); obj_to_char(obj,ch); 
      }
  ch->prompt=mystrdup("Perilous Realms (type HELP PROMPT) -> ");

  set_skill(ch, c->proficiency + PROF_BASE, 90);

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_POWER(ch) = GET_MAX_POWER(ch);
  ch->points.max_move += GET_CON(ch) + number(1,20) - 9;
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  GET_COND(ch,THIRST) = 24;
  GET_COND(ch,FULL) = 24;
  GET_COND(ch,DRUNK) = 0;


  ch->player.time.played = 0;
  ch->player.time.logon = time(0);
  advance_level(ch);
  do_save(ch,"",0);
}


void do_advance(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	char name[100], level[100];
	int adv, newlevel;

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, name);

	if (*name)	{
		if (!(victim = get_char_room_vis(ch, name)))		{
			send_to_char("That player is not here.\n", ch);
			return;
		}
	} else {
		send_to_char("Advance who?\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		send_to_char("NO! Not on NPC's.\n", ch);
		return;
	}

	argument = one_argument(argument, level);

	if (!*level) {
		send_to_char("You must supply a level number.\n", ch);
		return;
	}
	if (!isdigit(*level)) {
	  send_to_char("second argument must be a positive integer.\n",ch);
	  return;
	}
	if ((newlevel = atoi(level)) < GET_LEVEL(victim)) {
	  send_to_char("Can't dimish a players status (yet).\n", ch);
	  return;
	}
	adv = newlevel - GET_LEVEL(victim);

	if (newlevel>GET_LEVEL(ch) || newlevel>=IMPLEMENTOR) {
	  send_to_char("Thou art not godly enough.\n", ch);
	  return;
	}
	if (adv<=0) {
	  sendf(ch,"Try making them a higher level ;)\n");
	  return;
	}

	send_to_char("You feel generous.\n", ch);
  act("$n makes some strange gestures.\nA strange feeling comes upon you,\
\nLike a giant hand, light comes down from\nabove, grabbing your \
body, that begins\nto pulse with coloured lights from inside.\nYo\
ur head seems to be filled with daemons\nfrom another plane as your\
 body dissolves\ninto the elements of time and space itself.\nSudde\
nly a silent explosion of light snaps\nyou back to reality. You fee\
l slightly\ndifferent.",FALSE,ch,0,victim,TO_VICT);
	
	
  if (GET_LEVEL(victim) == 0) {
    do_start(victim);
  } else {
    if (GET_LEVEL(victim) < IMPLEMENTOR) {
      gain_exp_regardless(victim, EXP_NEEDED(GET_LEVEL(victim)+adv,
			GET_CLASS(victim)) -GET_EXP(victim));
      send_to_char("Character is now advanced.\n", ch);
    } else {
      send_to_char("Some idiot just tried to advance your level.\n",
			  victim);
      send_to_char("IMPOSSIBLE! IDIOTIC!\n", ch);
    }
  }
}


void do_reroll(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char buf[100];
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument,buf);
  if (!*buf)
    send_to_char("Who do you wish to reroll?\n",ch);
  else
    if(!(victim = get_char(buf)))
      send_to_char("No-one by that name in the world.\n",ch);
    else {
      send_to_char("Rerolled...\n", ch);
      roll_abilities(victim);
    }
}


void do_restore(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char buf[100];
  
  only_argument(argument,buf);
  if (!*buf && cmd!= -1)
    send_to_char("Who do you wish to restore?\n",ch);
  else
    if(!(victim = get_char(buf)) && cmd!= -1)
      send_to_char("No-one by that name in the world.\n",ch);
    else {
      if (cmd== -1) victim=ch;
      GET_MANA(victim) = GET_MAX_MANA(victim);
      GET_POWER(victim) = GET_MAX_POWER(victim);
      GET_HIT(victim) = GET_MAX_HIT(victim);
      GET_MOVE(victim) = GET_MAX_MOVE(victim);
      
      if (IS_NPC(victim))
	return;
      
      if (!IS_IMMORTAL(victim)) {
	GET_COND(victim,THIRST) = 24;
	GET_COND(victim,FULL) = 24;
      } else {
	GET_COND(victim,THIRST) = -1;
	GET_COND(victim,FULL) = -1;
      }
      
      if (IsClass(victim,BUILDER) || IsClass(victim,CLASS_IMMORTAL)) {
	spell_entry *sp; skill_entry *sk;
	FREE(victim->skills);
	FREE(victim->spells);
	victim->nskills=0; victim->nskills=0;
	for (sp=CLASS(victim).spells; sp; sp=sp->next)
	  set_spell(victim,sp->num,100);
	for (sk=CLASS(victim).skills; sk; sk=sk->next)
	  set_skill(victim,sk->num,100);
      }
      
      if (GetMaxLevel(victim) >= GOD) {
	victim->abilities.intel = 25;
	victim->abilities.wis = 25;
	victim->abilities.dex = 25;
	victim->abilities.str = 30;
	victim->abilities.con = 25;
	victim->tmpabilities = victim->abilities;
      }
      update_pos( victim );
      send_to_char("Done.\n", ch);
      if (cmd>=0)
      act("You have been fully healed by $N!", FALSE, victim, 0, ch, TO_CHAR);
    }
}




void do_noshout(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, buf);
  
  if (!*buf)
    if (IS_SET(ch->specials.act, PLR_NOSHOUT))		{
      send_to_char("You can now hear shouts again.\n", ch);
      CLEAR_BIT(ch->specials.act, PLR_NOSHOUT);
    }		else		{
      send_to_char("From now on, you won't hear shouts.\n", ch);
      SET_BIT(ch->specials.act, PLR_NOSHOUT);
    }
  else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n", ch);
  else if (GetMaxLevel(vict) >= GetMaxLevel(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else if (IS_SET(vict->specials.act, PLR_NOSHOUT) &&
	   (GetMaxLevel(ch) >= SAINT))	{
    send_to_char("You can shout again.\n", vict);
    send_to_char("NOSHOUT removed.\n", ch);
    CLEAR_BIT(vict->specials.act, PLR_NOSHOUT);
  }	else if (GetMaxLevel(ch) >= SAINT)	{
    send_to_char("The gods take away your ability to shout!\n", vict);
    send_to_char("NOSHOUT set.\n", ch);
    SET_BIT(vict->specials.act, PLR_NOSHOUT);
  } else {
    send_to_char("Sorry, you can't do that\n",ch);
  }
}


void do_nohassle(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, buf);
  
  if (!*buf)
    if (IS_SET(ch->specials.act, PLR_NOHASSLE))
      {
	send_to_char("You can now be hassled again.\n", ch);
	CLEAR_BIT(ch->specials.act, PLR_NOHASSLE);
      }
    else
      {
	send_to_char("From now on, you won't be hassled.\n", ch);
	SET_BIT(ch->specials.act, PLR_NOHASSLE);
      }
  else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n", ch);
  else if (GetMaxLevel(vict) > GetMaxLevel(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else
    send_to_char("The implementor won't let you set this on mortals...\n",ch);
  
}


void do_stealth(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, buf);
  
  if (!*buf)
    if (IS_SET(ch->specials.act, PLR_STEALTH))
      {
	send_to_char("STEALTH mode OFF.\n", ch);
	CLEAR_BIT(ch->specials.act, PLR_STEALTH);
      }
    else
      {
	send_to_char("STEALTH mode ON.\n", ch);
	SET_BIT(ch->specials.act, PLR_STEALTH);
      }
  else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n", ch);
  else if (GetMaxLevel(vict) > GetMaxLevel(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else
    send_to_char("The implementor won't let you set this on mortals...\n",ch);

}

void do_debug(struct char_data *ch, char *argument, int cmd)
{
  char	arg[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int	i;

#ifdef MALLOC_DEBUG
  malloc_dump(1);
#endif
  i=0;
  one_argument(argument, arg);
  i = atoi(arg);
  if (!isdigit(*arg)) {
    if (!(vict = get_char_vis(ch, arg))) {
	sendf(ch,"Couldn't find %s\n",arg);
	return;
    }
    if (IS_MOB(vict)) {
      send_to_char("Not on a beast!\n",ch);
      return;
    }
    if (IS_SET(vict->specials.act,PLR_DEBUG)) {
      CLEAR_BIT(vict->specials.act,PLR_DEBUG);
      sendf(ch,"Debug flag removed from %s\n",GET_NAME(vict));
      return;
    } else {
      SET_BIT(vict->specials.act,PLR_DEBUG);
      sendf(ch,"Debug flag set on %s\n",GET_NAME(vict));
      return;
    }
  }

  DEBUG=i;
  if (i) {
    sprintf(arg, "debug level set to %d\n", i);
    send_to_char(arg, ch);
  }
  else send_to_char("Debugging turned off\n",ch);
}

void do_invis(struct char_data *ch, char *argument, int cmd)
{
  char	buf[MAX_STRING_LENGTH];
  int	level;
  
  one_argument(argument, buf);

  if (isdigit(*buf)) {
    level=atoi(buf);
    if (level<0) level=0;
    else if (level>GetMaxLevel(ch)) level=GetMaxLevel(ch);
    ch->specials.invis_level = level;
    sprintf(buf,"Invis level set to %d.\n", level);
    send_to_char(buf, ch);
  } else {
    if (ch->specials.invis_level>0) {
      ch->specials.invis_level = 0;
      send_to_char("You are now totally visible.\n",ch);
    } else {
      ch->specials.invis_level = GetMaxLevel(ch);
      send_to_char("You are now invisible to all lower levels.\n",ch);
    }
  }
}

void do_noaffect(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;
  one_argument(argument, buf);
  if (!*buf){
    send_to_char("Remove affects from whom?\n", ch);
    return;
  } else {
    if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
      send_to_char("Couldn't find any such creature.\n", ch);
    else if (IS_NPC(vict))
      send_to_char("Can't do that to a beast.\n", ch);
    else if (GetMaxLevel(vict) > GetMaxLevel(ch))
      act("$E might object to that.. better not.",0,ch,0,vict,TO_CHAR);
    else{
      send_to_char("You are normal again.\n", vict);
      while (vict->affected)
	affect_remove(vict,vict->affected);
    }
  }
  send_to_char("Ok.\n",ch);
}

void do_file(struct char_data *ch, char *arg, int cmd)
{
  char buf[100],buf2[100];
  struct stat statbuf;

  extern char classhelp[MAX_STRING_LENGTH];
  extern char news[MAX_STRING_LENGTH];
  extern char donations[MAX_STRING_LENGTH];
  extern char oldnews[MAX_STRING_LENGTH];
  extern char credits[MAX_STRING_LENGTH];
  extern char motd[MAX_STRING_LENGTH];
  extern char help[MAX_STRING_LENGTH];
  extern FILE *help_fl;
  extern char info[MAX_STRING_LENGTH];
  extern int top_of_helpt;
  extern struct help_index_element *help_index;


  one_argument(arg,buf);

  if (!strcmp(buf,"shops")) {
    shop_data *s,*next;
    for (s=shop_index; s; s=next) {
      next=s->next;
      FREE(s->producing);
      FREE(s->type);
      FREE(s->vnum);
      FREE(s->no_such_item1);
      FREE(s->no_such_item2);
      FREE(s->missing_cash1);
      FREE(s->missing_cash2);
      FREE(s->do_not_buy);
      FREE(s->message_buy);
      FREE(s->message_sell);
      FREE(s->open);
      FREE(s->close);
      FREE(s);
    }
    boot_shp();
    assign_the_shopkeepers();
  } else if (!strcmp(buf,"news")) {
    file_to_string(NEWS_FILE,news);
    file_to_string(OLD_NEWS_FILE,oldnews);
  } else if(!strcmp(buf,"credits")) file_to_string(CREDITS_FILE,credits);
    else if(!strcmp(buf,"donations")) file_to_string(DONATIONS_FILE,donations);
  else if(!strcmp(buf,"motd")){
    file_to_string(MOTD_FILE,motd);
    stat(MOTD_FILE, &statbuf);
    motd_time = statbuf.st_mtime;
  }
  else if(!strcmp(buf,"help")) file_to_string(HELP_PAGE_FILE,help);
  else if(!strcmp(buf,"info")) file_to_string(INFO_FILE,info);
  else if (!strcmp(buf,"welcome")) file_to_string(WELCOME_FILE,welcome);
  else if (!strcmp(buf,"classes")) boot_classes();
  else if (!strcmp(buf,"info_classes")) file_to_string(CLASS_INFO_FILE,classhelp);
  else if (!strcmp(buf,"termcap")) reload_terms();
  else if (!strcmp(buf, TOOMANY_FILE)) file_to_string(TOOMANY_FILE, toomany);
  else if (!strcmp(buf,"table")) {
    fclose(help_fl);
    FREE(help_index);
    top_of_helpt=0;
    if (!(help_fl=fopen(HELP_KWRD_FILE,"r")))
	log("Could not open help file.");
    else
	help_index=build_help_index(help_fl,&top_of_helpt);
  } else {
      sprintf(buf2,"%sUsage:\nfile <welcome | donations | oldnews | news | credits | motd | help "
	      "| info_classes | info | table | classes>\n",*buf?"Illegal filename. ":"");	
      send_to_char(buf2,ch);
      return;
    }

  send_to_char("Ok.\n",ch);
}


void do_police(struct char_data *ch, char *argument, int cmd)
{
  char name[MAX_INPUT_LENGTH];
  struct connection_data *d,*next;
  int fd=0;

  one_argument(argument, name);
  if(!*argument) {
    sendf(ch,"Usage: police [fd | Name] | [-host]\n");
    return;
  }
  if (isdigit(*name))
    fd=atoi(argument);
  else if (*name=='-') {
    if (!name[1]) return;
    for (d=connection_list; d; d=next) {
      next=d->next;
      if (is_abbrev(name+1,d->host)) {
	if (!IS_CONN(d) || GET_LEVEL(d->character) < GET_LEVEL(ch)) {
	  sendf(ch,"Booting %d\n",d->id);
	  close_socket(d);
	}
      }
    }
    return;
  } else {
    for (d=connection_list; d; d=d->next)
      if (d->character)
	if (str_cmp(GET_NAME(d->character),name)==0)
	  break;
    if (d) fd=d->id;
  }

  for (d=connection_list; d; d=d->next) {
    if (fd==d->id) {
      if (IS_CONN(d)) {
	if (GET_LEVEL(d->character) < GET_LEVEL(ch)) {
	  extract_char(d->character);
          close_socket(d);
	  return;
        }
      } else {
        close_socket(d);
	return;
      }
    }
  }
}

void do_passwd(struct char_data *ch, char *arg, int cmd)
{
  char player_name[MAX_INPUT_LENGTH],password[MAX_INPUT_LENGTH];
  char buf[20];
  struct char_data *tmp;

  if (IS_NPC(ch)) return;
  arg=one_argument(arg,player_name);
  if (!*player_name) {
	send_to_char("But who?\n",ch);
	return;
  }
  arg=one_argument(arg,password);
  if (!*password) {
	send_to_char("What password?\n",ch);
	return;
  }
  if (strlen(password)>10) {
	send_to_char("Too Long.\n",ch);
	return;
  }

  CREATE(tmp,struct char_data,1);
  if (new_load_char(tmp,player_name)<0) {
	send_to_char("Couldn't find them.\n",ch);
	FREE(tmp);
	return;
  }
  strncpy(buf,(char *)crypt(password,tmp->player.name),10);
  *(buf+10)='\0';
  tmp->player.pw=REALLOC(tmp->player.pw,strlen(buf)+1,char);
  strcpy(tmp->player.pw,buf);
  save_char(tmp,tmp->in_room);
  free_char(tmp);
}

void do_clean(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *v;
  int i;

  if (IS_MOB(ch)) return;
  if (cmd==-1) v=ch;
  else if (!(v = get_char_vis(ch, arg))) {
    send_to_char("No living thing by that name.\n", ch);
    return;
  }
  while (v->affected) affect_remove(v,v->affected);
  if (cmd!=-1) FREE(v->kills);
  for (i=0; i<6; i++) {
    v->points.armor[i]=100;
    v->points.stopping[i]=0;
  }
  GET_STR(v)=GET_RSTR(v);
  GET_INT(v)=GET_RINT(v);
  GET_WIS(v)=GET_RWIS(v);
  GET_DEX(v)=GET_RDEX(v);
  GET_CON(v)=GET_RCON(v);
  GET_CHR(v)=GET_RCHR(v);
  GET_LCK(v)=GET_RLCK(v);
  GET_HITROLL(v)=0;
  GET_DAMROLL(v)=0;
  GET_POS(v)=POSITION_STANDING;
  for (i=0; i<5; i++)
    v->specials.apply_saving_throw[i]=0;
  init_bits(v->specials.affected_by);
}

void do_linkload(struct char_data *ch, char *arg, int cmd)
{
  char player_name[MAX_INPUT_LENGTH];
  struct char_data *tmp;

  if (IS_NPC(ch)) return;
  arg=one_argument(arg,player_name);
  if (!*player_name) {
	send_to_char("But who?\n",ch);
	return;
  }
  CREATE(tmp,struct char_data,1);
  if (new_load_char(tmp,player_name)<0) {
	send_to_char("Couldn't find them.\n",ch);
	FREE(tmp);
	return;
  }
  tmp->next=character_list;
  character_list=tmp;
  tmp->specials.timer=0;
  char_to_room(tmp,ch->in_room);
  act("$n has been link-loaded.\n",FALSE,tmp,0,0,TO_ROOM);
}

//this is used to cause a crash and exit (hopefully an external script is restarting the server).  Suppress the warning about it.
#pragma GCC diagnostic ignored "-Wdiv-by-zero"
void do_crash(struct char_data *ch, char *arg, int cmd)
{
  int x;
  x = ch->player.level/0;
  sendf(ch, "%d\n", x);
}
#pragma GCC diagnostic warning "-Wdiv-by-zero"

void do_trashcan(struct char_data *ch, char *arg, int cmd)
{
  FILE *f;
  char name[MAX_INPUT_LENGTH];
  int i;

  one_argument(arg,name);
  for (i=0; i<=strlen(name); i++) name[i]=LOWER(name[i]);
  if (strlen(name)<3 || strlen(name)>10 || !new_name_ok(name)) {
	sendf(ch,"Bad name not accepted -- ");
	if (strlen(name)<3) sendf(ch,"Too short.\n");
	else if (strlen(name)>10) sendf(ch,"Too long.\n");
	else sendf(ch,"Already disabled.\n");
	return;
  }
  f=fopen(BAD_NAMES_FILE,"a+");
  if (!f) {
    sendf(ch,"Can't open %s for writing.\n",BAD_NAMES_FILE);
    return;
  }
  fprintf(f,"%s\n",name);
  fclose(f);
  sendf(ch,"'%s' was added.\n",name);
}

void do_maxplayers(struct char_data *ch, char *arg, int cmd)
{
  char name[MAX_INPUT_LENGTH];
  FILE *f;
  int i;
  extern int abs_max_con,max_players;

  one_argument(arg,name);
  if (*name && GetMaxLevel(ch) >= SILLYLORD) {
    if (isdigit(*name)) {
      i = atoi(name);
      if (i > abs_max_con) {
	sendf(ch, "Can not set to %d because max is %d.\n", i, abs_max_con);
      } else {
	max_players=i;
	sendf(ch, "Maxplayers now set to: %d\n", max_players);
        if ((f = fopen("maxplayers", "w"))) {
          fprintf(f, "%d\n", max_players);
          fclose(f);
        }
      }
    } else {
      sendf(ch, "Try a positive number or 0 to lockout players.\n");
    }
  } else 
    sendf(ch, "Maxplayers: %d\n", max_players);
}

void do_setall(struct char_data *ch, char *arg,int cmd)
{
  struct char_data *i;
  char buf[MAX_STRING_LENGTH];
  if (IS_MOB(ch)) return;

  for (i=character_list; i; i=i->next)
    if (!IS_MOB(i)) {
	sprintf(buf,"%s %s",GET_NAME(i),arg);
	do_set(ch,buf,cmd);
    }
}
