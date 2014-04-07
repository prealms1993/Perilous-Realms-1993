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

char *sbrk(int);

void do_kills(struct char_data *ch,char *arg, int cmd)
{
  int i;
  index_mem *h;
  char buf[MAX_INPUT_LENGTH];
  struct char_data *v;
  struct string_block sb;

  v=ch;
  one_argument(arg,buf);
  if (GET_LEVEL(ch)>=GREATER_GOD && *buf) {
    for (v=character_list; v; v=v->next)
      if (CAN_SEE(ch,v) && str_cmp(GET_NAME(v),buf)==0) break;
    if (!v) {
	sendf(ch,"Couldn't find %s\n",buf);
	return;
    }
  }
  init_string_block(&sb);
  if (!v->kills)
    sb_printf(&sb,"You haven't killed anything yet!\n");
  else {
    for (i=0; i<MAX_KILL_KEEP; i++)
      if (v->kills[i].vnum>0) {
	h=real_mobp(v->kills[i].vnum);
	sb_printf(&sb,"Name: %-20s Kills: %d\n",
		  h?CAP(h->mob->player.short_descr):"No longer exists",
		  v->kills[i].nkills);
      }
  }
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
}

#if 1
int compare_name(struct connection_data **c,struct connection_data **d)
{
  if (!(*c)->character) {
    if (!(*d)->character) return(0);
    else return(-1);
  } else if (!(*d)->character) return(1);
  return(strcmp((*c)->character->player.name,(*d)->character->player.name));
}

int compare_host(struct connection_data **c, struct connection_data **d)
{
  return(strcmp((*c)->host,(*d)->host));
}

int compare_level(struct connection_data **c, struct connection_data **d)
{
  int diff;
  if (!(*c)->character) {
    if (!(*d)->character) return(0);
    else return(-1);
  } else if (!(*d)->character) return(1);
  if (!(*c)->character->player.level) {
    if (!(*d)->character->player.level) return(0);
    else return(-1);
  } else if (!(*d)->character->player.level) return(1);
  diff = GetMaxLevel((*c)->character)-GetMaxLevel((*d)->character);
  if (!diff) return(0);
  else return(diff/abs(diff));
}

int compare_fd(struct connection_data **c,struct connection_data **d)
{
  int diff;
  diff=(*c)->id-(*d)->id;
  if (!diff) return(0);
  else return(diff/abs(diff));
}

void do_users(struct char_data *ch, char *arg, int cmd)
{
  extern char *connected_types[];
  struct connection_data *d;
  struct connection_data *conns[256];
  struct char_data *c;
  char buf[MAX_INPUT_LENGTH];
  char line[255];
  char host_only[MAX_INPUT_LENGTH];
  int disp_total,by_host;
  static int most=0;
  int t,i;
  int count;
  int ndesc,visible,disconnected,nanny,immortal,get_name,condensed;
  int (*compar)();
  struct string_block sb;

  compar=NULL;
  *host_only='\0';
  condensed=count=get_name=ndesc=visible=disconnected=nanny=immortal=disp_total=by_host=0;
  if (GetMaxLevel(ch)<LOW_IMMORTAL)
    disp_total=1;
  else {
    while (*arg) {
      arg=one_argument(arg,buf);
      switch (*buf) {
	case '-':
	  switch(buf[1]) {
	    case 'n': compar=compare_name; break;
	    case 'l': compar=compare_level; break;
	    case 'h': if (buf[2]) strcpy(host_only,buf+2);
		      compar=compare_host; break;
	    case 'f': compar=compare_fd; break;
	    case 't': disp_total=1; break;
	    case 'i': immortal=1; break;
	    case 'c': condensed=1; break;
#define USERS_HELP "\n\
Usage: users [-c] [-t] [-l] [-h] [-i] [-c]\nExample: users -i -c -h\n\
    -c : condensed\n\
    -l : sorted by level\n\
    -h : sorted by host\n\
    -n : sorted by name\n\
    -i : immortals only\n\
    -t : totals only\n"
	    case '?': 
		sendf(ch,USERS_HELP);
		return;
	  }; break;
	  /* future expansion */
      }
    }
  }

  for (d=connection_list;d;d=d->next) {
    conns[ndesc++]=d;
    if (immortal && (!d->character || !IS_IMMORTAL(d->character))) ndesc--;
  }

  if (compar)
    qsort(conns,ndesc,sizeof(struct connection_data *),compar);

  init_string_block(&sb);
  for (i=0; i<ndesc; i++) {
    d=conns[i];
    c=d->character;
    if (IS_CONN(d)) {
      if (*host_only && !is_abbrev(host_only,d->host)) continue;
      if (CAN_SEE(ch,c)) {
	count++;
	if (disp_total) continue;
	if (condensed)
	  sprintf(line,"%3d%2d:%-12s%4d %-16s%s",
		d->id,c->specials.timer,
		GET_NAME(c),GetMaxLevel(c),
		d->host,count%2?"|":"\n");
        else
	  sprintf(line,"%3d%2d:%-12s%4d %-16s (%4d/%4d) [%7d]\n",
		d->id,c->specials.timer,
		GET_NAME(c),GetMaxLevel(c),
		d->host,
		GET_HIT(c),hit_limit(c),
		c->in_room);
	append_to_string_block(&sb,line);
      }
    } else if (GetMaxLevel(ch)>=SILLYLORD) {
      count++;
      if (d->character)
	if (condensed)
	  sprintf(line,"%3d%2d*%-12s%4d %-16s%s",
		d->id,d->connected,
		GET_NAME(c),GetMaxLevel(c),
		d->host,count%2?"|":"\n");
	else
	  sprintf(line,"%3d%2d*%-12s%4d %-16s %s\n",
		d->id,d->connected,
		GET_NAME(c),GetMaxLevel(c),
		d->host,connected_types[d->connected]);
      else if (condensed)
	  sprintf(line,"%3d  :%-12s%4s %-16s%s",
		d->id,"New Conn","",
		d->host,count%2?"|":"\n");
	else
	  sprintf(line,"%3d  :%-12s%4s %-16s\n",
		d->id,"New Conn","",d->host);
      append_to_string_block(&sb,line);
    }
  }
  t=30+time(0)-boottime;
  if (ndesc>most) most=ndesc;
  sprintf(line,"\nUsers: %d/%d/%d\n",count,most,player_count());
  append_to_string_block(&sb,line);
  sprintf(line,"Running time %d:%02d\n",t/3600,(t%3600)/60);
  append_to_string_block(&sb,line);
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
}

#endif

void do_exits(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char buf[MAX_STRING_LENGTH];
  static char *exits[] =
    {
      "North",
      "East ",
      "South",
      "West ",
      "Up   ",
      "Down "
      };
  struct room_direction_data	*exitdata;

  *buf = '\0';

  for (door = 0; door <= 5; door++) {
    exitdata = EXIT(ch,door);
    if (exitdata) {
      if (!real_roomp(exitdata->to_room)) {
	/* don't print unless immortal */
	if (IS_IMMORTAL(ch)) {
	  sprintf(buf + strlen(buf), "%s[%d] - swirling chaos of #%d\n",
	    	  exits[door], door, exitdata->to_room);
        }
      } else if (exitdata->to_room != NOWHERE &&
		 (!IS_SET(exitdata->exit_info, EX_CLOSED) ||
		  IS_IMMORTAL(ch))) {
	  if (IS_DARK(exitdata->to_room) && !IS_IMMORTAL(ch)) {
	    sprintf(buf + strlen(buf), "%s - Too dark to tell\n", exits[door]);
	  } else if (IS_IMMORTAL(ch)) {
	    sprintf(buf+strlen(buf),"%s[%d] - [%d]%s\n",
		exits[door], door, exitdata->to_room,
		real_roomp(exitdata->to_room)->name);
	  } else {
	    sprintf(buf + strlen(buf), "%s - %s", exits[door],
		  real_roomp(exitdata->to_room)->name);
	    if (IS_SET(exitdata->exit_info, EX_CLOSED))
	      strcat(buf, " (closed)");
	    strcat(buf, "\n");
          }
      }
    }
  }

  send_to_char("Obvious exits:\n", ch);

  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char("None.\n", ch);
}


void do_score(struct char_data *ch, char *argument, int cmd)
{
  struct time_info_data playing_time;

  struct time_info_data real_time_passed(time_t t2, time_t t1);

  sendf(ch, "You are %d years old.", GET_AGE(ch));

  if ((age(ch).month == 0) && (age(ch).day == 0))
    send_to_char(" It's your birthday today.",ch);
  send_to_char("\n", ch);

  if (!IS_IMMORTAL(ch) && (!IS_NPC(ch))) {
    if (GET_COND(ch,DRUNK)>10)
      send_to_char("You are intoxicated.\n", ch);
    if (GET_COND(ch,FULL)<2)
      send_to_char("You are hungry...\n", ch);
    if (GET_COND(ch,THIRST)<2)
      send_to_char("You are thirsty...\n", ch);
  }

  sendf(ch,"You have %d(%d) hit, %d(%d) mana, %d(%d) power, and %d(%d) move points.\n",
	  GET_HIT(ch),GET_MAX_HIT(ch),
	  GET_MANA(ch),GET_MAX_MANA(ch),
	  GET_POWER(ch),GET_MAX_POWER(ch),
	  GET_MOVE(ch),GET_MAX_MOVE(ch));

  sendf(ch,"Your alignment (-1000 : +1000): %d\n", GET_ALIGNMENT(ch));
  sendf(ch,"You have scored %d exp, and have %d gold coins.\n",
	  GET_EXP(ch),GET_GOLD(ch));

  sendf(ch, "You are a level %d %s.\n",GET_LEVEL(ch),CLASS(ch).class_name);
  if (!IS_IMMORTAL(ch)) {
    if (CAN_ADVANCE(ch))
      sendf(ch,"You have enough experience to advance a level.\n");
    else
      sendf(ch,"You need %ld XP to advance a level.\n",
	  EXP_NEEDED(GetMaxLevel(ch)+1,GET_CLASS(ch))-GET_EXP(ch));
  }

  sendf(ch,"You are: %s %s\n", GET_NAME(ch), GET_TITLE(ch));

  playing_time = real_time_passed((time(0)-ch->player.time.logon) +
				  ch->player.time.played, 0);
  sendf(ch,"You have been playing for %d days and %d hours.\n",
	  playing_time.day,
	  playing_time.hours);

  switch(GET_POS(ch)) {
  case POSITION_DEAD :
    send_to_char("You are DEAD!\n", ch); break;
  case POSITION_MORTALLYW :
    send_to_char("You are mortally wounded! You should seek help!\n",ch);
    break;
  case POSITION_INCAP :
    send_to_char("You are incapacitated, slowly fading away\n", ch); break;
  case POSITION_STUNNED :
    send_to_char("You are stunned! You can't move\n", ch); break;
  case POSITION_SLEEPING :
    send_to_char("You are sleeping.\n",ch); break;
  case POSITION_RESTING  :
    send_to_char("You are resting.\n",ch); break;
  case POSITION_PRAYING:
    send_to_char("You are praying.\n", ch); break;
  case POSITION_SITTING  :
    send_to_char("You are sitting.\n",ch); break;
  case POSITION_FIGHTING :
    if (ch->specials.fighting)
      act("You are fighting $N.\n", FALSE, ch, 0,
	  ch->specials.fighting, TO_CHAR);
    else {
      send_to_char("You are fighting thin air.\n", ch);
      GET_POS(ch)=POSITION_STANDING;
    }
    break;
  case POSITION_STANDING :
    send_to_char("You are standing.\n",ch); break;
    default :
      send_to_char("You are floating.\n",ch); break;
  }
  if(IS_AWAY(ch)){
    sendf(ch, "You are away: %s\n", ch->specials.away);
  }
}


void do_time(struct char_data *ch, char *argument, int cmd)
{
  char *suf;
  int weekday, day;

  weekday = ((35*time_info.month)+time_info.day+1) % 7;/* 35 days in a month */
  sendf(ch, "It is %d o'clock %s, on %s.\n",
	  ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am"),weekdays[weekday] );


  day = time_info.day + 1;   /* day in [1..35] */

  if (day == 1)
    suf = "st";
  else if (day == 2)
    suf = "nd";
  else if (day == 3)
    suf = "rd";
  else if (day < 20)
    suf = "th";
  else if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";

  sendf(ch, "The %d%s Day of the %s, Year %d.\n",
	  day,
	  suf,
	  month_name[time_info.month],
	  time_info.year);
}


void do_weather(struct char_data *ch, char *argument, int cmd)
{
  static char *sky_look[4]= {
    "cloudless",
    "cloudy",
    "rainy",
    "lit by flashes of lightning"};

  if (OUTSIDE(ch))
    sendf(ch,"The sky is %s and %s.\n",
      sky_look[weather_info.sky],
      (weather_info.change >=0 ? 
	     "you feel a warm wind from south" :
	     "your foot tells you bad weather is due"));
  else
    send_to_char("You have no feeling about the weather at all.\n", ch);
}


void do_help(struct char_data *ch, char *argument, int cmd)
{
  int chk, bot, top, mid, minlen;
  char buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];
  char *p;


  if (!ch->desc)
    return;

  for(;isspace(*argument); argument++)  ;


  if (*argument) {
      if (!help_index) {
	  send_to_char("No help available.\n", ch);
	  return;
	}
      bot = 0;
      top = top_of_helpt;

      for (;;) {
	  mid = (bot + top) / 2;
	  minlen = strlen(argument);

	  if (!(chk = strn_cmp(argument, help_index[mid].keyword, minlen))) {
	      fseek(help_fl, help_index[mid].pos, 0);
	      *buffer = '\0';
	      for (;;) {
		  fgets(buf, 90, help_fl);
	          if (strlen(buffer)+strlen(buf)>MAX_STRING_LENGTH) {
			vlog(LOG_URGENT,"HELP: %s",argument);
			return;
		  }
		  strcat(buf,"");
		  if (*buf == '#')
		    break;
		  if (*buf == '!' && *(buf+1)=='>')
			if (GetMaxLevel(ch)<atoi(buf+2)) break;
			else continue;
		  if (*buf == '!' && *(buf+1)=='<')
			if (GetMaxLevel(ch)>atoi(buf+2)) break;
			else continue;
		  if (*buf=='>' && GetMaxLevel(ch)>=atoi(buf+1)) {
			p=strchr(buf,':');
			if (p++) strcat(buffer,p);
			else vlog(LOG_URGENT,"Error in help '>': %s",argument);
		  } else if (*buf=='<' && GetMaxLevel(ch)<=atoi(buf+1)) {
			p=strchr(buf,':');
			if (p++) strcat(buffer,p);
			else vlog(LOG_URGENT,"Error in help '<': %s",argument);
		  } else if (*buf!='>' && *buf!='<')
			strcat(buffer, buf);
		}
	      page_string(ch->desc, buffer, 1);
	      return;
	    }
	  else if (bot >= top) {
	    send_to_char("There is no help on that word.\n", ch);
	    return;
	  }
	  else if (chk > 0)
	    bot = ++mid;
	  else
	    top = --mid;
	}
    }

  send_to_char(help, ch);
}

void do_listhelp(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH*5];
  int no, i;
  char temp[132];

  if (IS_NPC(ch))
    return;

  send_to_char("The following help topics are available:\n\n", ch);

  *buf = '\0';

  for (no = 1, i = 0; i<top_of_helpt; i++) {
      strcpy(temp,help_index[i].keyword);
      temp[10]='\0';
      sprintf(buf + strlen(buf), "%-10s ", temp);
      if (!(no % 7))
	strcat(buf, "\n");
      no++;
  }
  strcat(buf, "\n");
  page_string(ch->desc, buf, 1);
}

void do_listcommands(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int no, i;

  if (IS_NPC(ch))
    return;

  send_to_char("The following commands are available:\n\n", ch);

  *buf = '\0';

  for (no = 1, i = 0; *cmd_info[i].cmd != '\n'; i++)
    if (GetMaxLevel(ch) >= cmd_info[i].minimum_level) {
      sprintf(buf + strlen(buf), "%-10s", cmd_info[i].cmd);
      if (!(no % 7))
	strcat(buf, "\n");
      no++;
    }
  strcat(buf, "\n");
  page_string(ch->desc, buf, 1);
}

void do_wizhelp(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int no, i;

  if (IS_NPC(ch))
    return;

  send_to_char("The following privileged comands are available:\n\n", ch);

  *buf = '\0';

  for (no = 1, i = 0; *cmd_info[i].cmd != '\n'; i++)
    if ((GetMaxLevel(ch) >= cmd_info[i].minimum_level) &&
	(cmd_info[i].minimum_level >= LOW_IMMORTAL)) 	{
      sprintf(buf + strlen(buf), "%-10s", cmd_info[i].cmd);
      if (!(no % 7))
	strcat(buf, "\n");
      no++;
    }
  strcat(buf, "\n");
  page_string(ch->desc, buf, 1);
}


char *select_flags(struct char_data *ch, struct char_data *victim)
{
  static char final[256];
  char buf[256];
  char buf2[256];

  final[0] = '\0';
  buf[0] = '\0';
  if(IS_AWAY(victim)){
    strcpy(buf, "away");
  }
  if(IS_IMMORTAL(ch)){
    if(victim->specials.invis_level){
      sprintf(buf2, "%sinvis %d", *buf ? ", " : "",
	victim->specials.invis_level);
      strcat(buf, buf2);
    }
    if(IS_SET(victim->specials.act, PLR_NOSHOUT)){
      sprintf(buf2, "%snoshout", *buf ? ", " : "");
      strcat(buf, buf2);
    }
    if(IS_SET(victim->specials.act, PLR_NOTELL)){
      sprintf(buf2, "%snotell", *buf ? ", " : "");
      strcat(buf, buf2);
    }
    if(IS_SET(victim->specials.act, PLR_SILENCE)){
      sprintf(buf2, "%ssilenced", *buf ? ", " : "");
      strcat(buf, buf2);
    }
    if(IS_SET(victim->specials.act, PLR_VEG)){
      sprintf(buf2, "%swriting", *buf ? ", " : "");
      strcat(buf, buf2);
    }
  }
  if(*buf){
    sprintf(final, " (%s)", buf);
  }
  return(final);
}

char *header_types[] = {
  "Ma",
  "Cl",
  "Wa",
  "Th",
  "Ra",
  "Pa",
  "As",
  "Ni",
  "Ba",
  "Ca",
  "Av",
  "B ",
  "Dr",
  "Il",
};

char *guild_header_types[] = {
  "PR",
  "DT",
};

char *race_header_types[] = {
  "None",
  "Hu",
  "El",
  "Dw",
  "Hb",
  "Gn",
  "Re",
  "Sp",
  "Ly",
  "Dr",
  "Un",
  "Or",
  "In",
  "Ar",
  "Di",
  "Fi",
  "Bi",
  "Gi",
  "Pr",
  "Ps",
  "Sl",
  "Dm",
  "Sn",
  "Hv",
  "Te",
  "Vg",
  "El",
  "Pl",
  "De",
  "Gh",
  "Go",
  "Tr",
  "Ve",
  "Mf",
  "Pr",
  "En",
  "36",
  "37",
  "38",
  "39",
  "Py",
  "La",
  "Sa",
  "Ty",
  "44",
  "Mu",
  "Hr",
  "He",
  "Hl",
  "Ho",
  "Og",
  "Pi",
  "Li",
  "\n"
};

char *home_header_types[] = {
  "--",
  "--",
  "--",
  "--",
  "--",
  "--",
  "--",
  "--",
  "--",
};

void do_finger(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char buf[MAX_STRING_LENGTH], *p;
  int loaded = 0;

  for(; isspace(*argument); argument++);

  if(!*argument){
    sendf(ch, "Finger who?\n");
    return;
  }

  for(p = argument; isalpha(*p); p++);
  if(isspace(*p)){
    *p = '\0';
  }

  if(*p){
    sendf(ch, "No such player.\n");
    return;
  }

  for(victim = character_list; victim; victim = victim->next){
    if(!str_cmp(victim->player.name, argument) && !IS_NPC(victim)){
      break;
    }
  }
  if(!victim){
    CREATE(victim, struct char_data, 1);
    clear_char(victim);
    if(new_load_char(victim, argument) == -1){
      sendf(ch, "No such player.\n");
      return;
    }
    loaded = 1;
  }
#define ESTR(s, t)  ((s) ? ((*(s)) ? (s) : t) : t)
  if(GET_LEVEL(ch) >= SILLYLORD){
    if(IS_AWAY(victim)){
      sendf(ch, "%s is away %s.\n", GET_NAME(victim), victim->specials.away);
    }
    cftime(buf, "%a %b %e %T %Y", &victim->player.time.logon);
    sendf(ch, "Full Name: %s\nE-Mail: %s\n%s %s from %s.\nPlan:\n%s\n",
      ESTR(victim->player.full_name, victim->player.name),
      ESTR(victim->player.email, "<none>"),
      (!loaded && CAN_SEE(ch, victim)) ? "On since" : "Last logon on", buf,
      ESTR(victim->player.last_site, "nowhere"),
      ESTR(victim->player.plan, "No plan."));
  }
  else if(GET_LEVEL(ch) >= IMMORTAL){
    if(IS_AWAY(victim)){
      sendf(ch, "%s is away %s.\n", GET_NAME(victim), victim->specials.away);
    }
    cftime(buf, "%a %b %e %T %Y", &victim->player.time.logon);
    sendf(ch, "Full Name: %s\n%s %s from %s.\nPlan:\n%s\n",
      ESTR(victim->player.full_name, victim->player.name),
      (!loaded && CAN_SEE(ch, victim)) ? "On since" : "Last logon on", buf,
      ESTR(victim->player.last_site, "nowhere"),
      ESTR(victim->player.plan, "No plan."));
  }
  else if(GET_LEVEL(victim) < IMMORTAL){
    if(IS_AWAY(victim)){
      sendf(ch, "%s is away %s.\n", GET_NAME(victim), victim->specials.away);
    }
    cftime(buf, "%a %b %e %T %Y", &victim->player.time.logon);
    sendf(ch, "Full Name: %s\n%s %s.\nPlan:\n%s\n",
      ESTR(victim->player.full_name, victim->player.name),
      (!loaded && CAN_SEE(ch, victim)) ? "On since" : "Last logon on", buf,
      ESTR(victim->player.plan, "No plan."));
  }
  else{
    if(IS_AWAY(victim)){
      sendf(ch, "%s is away %s.\n", GET_NAME(victim), victim->specials.away);
    }
    sendf(ch, "Full Name: %s\nPlan:\n%s\n",
      ESTR(victim->player.full_name, victim->player.name),
      ESTR(victim->player.plan, "No plan."));
  }
  if(loaded){
    free_char(victim);
  }
}

void do_who(struct char_data *ch, char *argument, int cmd)
{
  struct connection_data *d;
  struct char_data *vic,*looking_for=0;
  char buf[MAX_STRING_LENGTH],cls[MAX_STRING_LENGTH],class[80];
  int players=0,total=0,min=0;

  struct string_block	sb;
    
  total = players = 0;
  init_string_block(&sb);

  for(;isspace(*argument);argument++);

  if (!strcmp (argument, "arena") || !strcmp (argument, "a")) {
      append_to_string_block(&sb, 
              "  []===================================[]\n"
              "  ||  Participants in the Blood Bath   ||\n"
              "  []===================================[]\n");
  } else if (!strcmp (argument, "corrupt") || !strcmp (argument, "c")) {
      append_to_string_block(&sb, 
              "[]===============================[]\n"
              "||  Corrupt People in the Realm  ||\n"
              "[]===============================[]\n");
  } else if (!strcmp (argument, "lawful") || !strcmp (argument, "l")) {
      append_to_string_block(&sb, 
              "[]==============================[]\n"
              "||  Lawful People in the Realm  ||\n"
              "[]==============================[]\n");
  } else {
      if(isdigit(*argument) || !argument || !*argument) {
	if (!argument || !*argument)
		min = 1;
	else
		min = atoi(argument);
      	append_to_string_block(&sb,
              "[]===================================[]\n"
              "||   Denizens of Perilous Realms     ||\n"
              "[]===================================[]\n"
              "  Ra Lev Class Name\n"
              " ------------------\n");
	/* not numeric, is it a name ? */
      } else {
	min = -1;
	looking_for = get_char_vis(ch,argument);
	if (!looking_for || IS_NPC(looking_for)) {
	  sendf(ch,"%s is not on or visible.\n", argument);
	  destroy_string_block(&sb);
	  return;
	}
	if (!looking_for->desc) {
	  sendf(ch,"%s is currently link dead.\n",GET_NAME(looking_for));
	  destroy_string_block(&sb);
	  return;
	}
	*argument='\0';
      }
  }

  for (d = connection_list; d; d = d->next) {
    if (*argument == 'a') {
      if (IS_CONN(d) && IN_CONTEST(d->character) && CAN_SEE(ch,d->character)) {
	if (d->original)   /* If switched */
	  sprintf(buf, "%-20s", d->original->player.name);
	else
	  sprintf(buf, "%-20s", d->character->player.name);
	append_to_string_block(&sb, buf);
	if(GetMaxLevel(d->character) <= MAX_MORT)
	     ++players;
	++total;
	if (!(total%2))
	  append_to_string_block(&sb, "\n");
      }
    } else if ((looking_for && d->character==looking_for) ||
	(!looking_for &&
	 IS_CONN(d) && CAN_SEE(ch,d->character) &&
	 GET_LEVEL(d->character)>=min)) {
      if (*argument=='l' || *argument=='c') {
	if (!IS_SET(d->character->specials.act,PLR_CORRUPT)) continue;
	if (GET_TRUE_ALIGN(d->character)==1000 && *argument=='c') continue;
	if (GET_TRUE_ALIGN(d->character)==-1000 && *argument=='l') continue;
      }
      ++total;
      if (d->original) { /* If switched */            
	vic=d->original;
      } else {
	vic=d->character;
      }    
      switch(GetMaxLevel(vic)){    
	case IMMORTAL:		strcpy(cls," [ Immortal  ]"); break;
	case REAL_IMMORT:	strcpy(cls," [ M Builder ]"); break;
        case CREATOR:		strcpy(cls," [  Builder  ]"); break;
        case SAINT:		strcpy(cls," [ G Builder ]"); break;
        case DEMIGOD:		strcpy(cls," [  Demigod  ]"); break;
        case LESSER_GOD:	strcpy(cls," [Lesser God ]"); break;
        case GOD:		strcpy(cls," [    God    ]"); break;
        case GREATER_GOD:	strcpy(cls," [Greater God]"); break;
        case SILLYLORD:		strcpy(cls," [ Asst. Imp ]"); break;
        case IMPLEMENTOR:	strcpy(cls," [    Imp    ]"); break;
        default: {
	  strcpy(class,CLASS(vic).abbrev);
  
          sprintf(cls," [ %s-%-3d %2s ]",
              race_header_types[GET_RACE(vic)],
              GetMaxLevel(vic),
              class);
          ++players;
        } break;
      }
      sprintf(buf,"%s %s %s%s\n", cls, GET_NAME(vic), vic->player.title,
	select_flags(ch, vic));
      append_to_string_block(&sb, buf);
      if(looking_for){
        if(vic->player.email && (GET_LEVEL(ch) >= 2009)){
          sprintf(buf,"%s is %s.\n", GET_NAME(vic), vic->player.email);
          append_to_string_block(&sb, buf);
        }
        if(IS_AWAY(vic)){
          sprintf(buf, "%s is away %s.\n", GET_NAME(vic),
            vic->specials.away);
          append_to_string_block(&sb, buf);
        }
      }
    }
  }
  if (!looking_for)
    sb_printf(&sb,"\n         Total Players: %d\n         Total Immorts: %d\n",players,total-players);
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
}

void do_linkdead(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *i;
  struct room_data *rp;
  int count;

  for (count=0, i=character_list; i; i=i->next)
    if (!IS_NPC(i) && !i->desc) {
      rp=real_roomp(i->in_room);
      sendf(ch,"[%2d] %-25s [%4d] %s\n",++count,GET_NAME(i),i->in_room,
			rp?rp->name:"Nowhere");
    }
  if (!count) sendf(ch,"Amazingly, no one is linkdead\n");
}

void do_credits(struct char_data *ch, char *argument, int cmd)
{
  page_string(ch->desc, credits, 0);
}

void do_oldnews(struct char_data *ch, char *argument, int cmd)
{
  page_string(ch->desc, oldnews, 0);
}

void do_motd(struct char_data *ch, char *argument, int cmd)
{
  page_string(ch->desc, motd, 0);
}

void do_news(struct char_data *ch, char *argument, int cmd)
{
  page_string(ch->desc, news, 0);
}


void do_donations(struct char_data *ch, char *argument, int cmd)
{
  page_string(ch->desc, donations, 0);
}

void do_info(struct char_data *ch, char *argument, int cmd)
{
  page_string(ch->desc, info, 0);
}


void do_wizlist(struct char_data *ch, char *argument, int cmd)
{
  page_string(ch->desc, wizlist, 0);
}


static void describe_object(int count,struct obj_data *obj,struct string_block *sb)
{
  struct char_data *ch;
  struct obj_data *o;
  char buf[MAX_STRING_LENGTH];

  o=obj;
  sprintf(buf,"[%3d] %-30s ",count,o->short_description);
  if (real_roomp(o->in_room)) {
    sprintf(buf+strlen(buf),"%-25s [%5d]\n","",o->in_room);
    append_to_string_block(sb,buf);
    return;
  }
/* go out of whatever objects we are in */
  while (o->in_obj)
    o=o->in_obj;

  ch=o->carried_by;
  if (!ch) ch=o->equipped_by;

  if (ch) {
    sprintf(buf+strlen(buf),"%-25s [%5d]\n",GET_NAME(ch),
		ch->in_room);
    append_to_string_block(sb,buf);
    return;
  } else if (real_roomp(o->in_room)) {
    sprintf(buf+strlen(buf),"%-25s [%5d]\n","(in object)",o->in_room);
    append_to_string_block(sb,buf);
    return;
  } else {
    sprintf(buf+strlen(buf),"ERROR -- Not in a real room!\n");
    append_to_string_block(sb,buf);
    return;
  }
}

static void describe_char(int count,struct char_data *ch,struct string_block *sb)
{
  char buf[MAX_STRING_LENGTH];
  char room_name[255];
  struct room_data *rp;

  rp=real_roomp(ch->in_room);

  if (rp)
    strcpy(room_name,rp->name);
  else
    strcpy(room_name,"ERROR!?!?!?!?");
  room_name[20]='\0';
  sprintf(buf,"[%3d] %-30s %-20s [%5d]\n",
		count,
		GET_NAME(ch),
		room_name,
		ch->in_room);
  append_to_string_block(sb,buf);
}

void do_where(struct char_data *ch,char *arg,int cmd)
{
  int number=0;
  struct string_block sb;
  struct char_data *c;
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  struct obj_data *obj;
  char *p;
  int count=0;

#define OWHERE 173
#define PWHERE 318
#define MWHERE 319
#define RWHERE -1

  only_argument(arg,buf);
  if (!*buf && cmd!=PWHERE) {
	sendf(ch,"What you looking for?\n");
	return;
  }
  number=atoi(buf);
  if (number) {
    p=buf;
    while (isdigit(*p)) p++;
    if (*p!='.') {
      sendf(ch,"If you are gonna use a number, use #.name\n");
      return;
    }
    p++; /* get rid of period */
    strcpy(buf1,p);
    strcpy(buf,buf1);
	sendf(ch,"looking for %d %s\n",number,buf);
  }

  init_string_block(&sb);
  if (cmd==OWHERE) {
    for (obj=object_list; obj;  obj=obj->next) {
      if (str_str(obj->name,buf)) {
	if (number==1) {
	  describe_object(++count,obj,&sb);
	  break;
	} else if (number) number--;
	else describe_object(++count,obj,&sb);
      }
    }
  } else if (cmd==MWHERE) {
    for (c=character_list; c; c=c->next) {
      if (IS_MOB(c) && str_str(c->player.name,buf) && CAN_SEE(ch,c)) {
	if (number==1) {
	  describe_char(++count,c,&sb);
	  break;
	} else if (number) number--;
	else describe_char(++count,c,&sb);
      }
    }
  } else if (cmd==PWHERE) {
    for (c=character_list; c; c=c->next) {
      if (!IS_MOB(c) && (!*buf || str_str(c->player.name,buf)) 
		&& CAN_SEE(ch,c)) {
	if (number==1) {
	  describe_char(++count,c,&sb);
	  break;
	} else if (number) number--;
	else describe_char(++count,c,&sb);
      }
    }
  } else if (cmd==RWHERE) {
  } else {
	sendf(ch,"Mixup.. you shouldn't be here. (cmd=%d)\n",cmd);
	return;
  }
  if (!sb.data || !*sb.data)
    send_to_char("Couldn't find any such thing.\n", ch);
  else
    page_string_block(&sb, ch);
  destroy_string_block(&sb);
}

void do_levels(struct char_data *ch, char *argument, int cmd)
{
  int i;
  int class;
  int lo,hi;
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))	{
    send_to_char("You ain't nothin' but a hound-dog.\n", ch);
    return;
  }

  *buf = '\0';
  for (;isspace(*argument);argument++);

  if (!*argument) {
    sendf(ch,"Usage: levels lo [hi [class]]\nEx: levels 1 100 ma\n");
    return;
  }
  argument=one_argument(argument,buf);

  for (;isspace(*argument);argument++);
  lo=atoi(buf);
  if (lo<=0) {
    send_to_char("Need a number greater than zero!\n",ch);
    return;
  }
  if (*argument) {
    argument=one_argument(argument,buf);
    hi=atoi(buf);
    if (hi<lo) {
      send_to_char("Second parameter must be greater than the first!\n",ch);
      return;
    }
  } else hi=lo;

  for (;argument && isspace(*argument);argument++);
  if (!argument || !*argument) class=GET_CLASS(ch);
  else class=which_class(ch,argument,1);
  if (class<0) return;

  if (hi>500) {
    send_to_char("There are only 500 levels of mortality!\n",ch);
    return;
  }

  for (i=lo; i<=hi; i++) {
    sprintf(buf,"%5d: %lu-%lu\n",i,EXP_NEEDED(i,class),EXP_NEEDED(i+1,class));
    send_to_char(buf,ch);
  }
}

void set_mock(struct char_data *ch, struct mock_char_data *mock_char)
{
  mock_char->thaco = THACO(ch);
  mock_char->thaco -= str_app[GET_STR(ch)].tohit;
  mock_char->thaco -= GET_HITROLL(ch);

  mock_char->ac = ch->points.armor[LOCATION_BODY]/10;

  mock_char->hp = GET_HIT(ch);
  mock_char->dam = str_app[GET_STR(ch)].todam;
  mock_char->dam += GET_DAMROLL(ch);
  if(!ch->equipment[WIELD]) {
        mock_char->dam += (ch->specials.damnodice * ch->specials.damsizedice/2);
  } else 
        mock_char->dam += (ch->equipment[WIELD]->obj_flags.value[1] * 
                ch->equipment[WIELD]->obj_flags.value[2]/2);
  mock_char->dam = MAX(mock_char->dam,1);
  if (IS_MOB(ch))
    mock_char->dam *= MAX(1,ch->specials.mult_att);
  else if (is_fighter(ch)) {
     if(GetMaxLevel(ch)>12)
       mock_char->dam *= 2;
  }

  mock_char->hp = GET_HIT(ch);
}


void do_consider(struct char_data *ch, char *argument, int cmd)
{

  struct mock_char_data mock_char;
  struct mock_char_data mock_victim;
  struct char_data *victim;
  char name[256];
  int diff;
  int scale;
  double char_roundsr, victim_roundsr, temp, temp2;


  only_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Consider killing who?\n", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Easy! Very easy indeed!\n", ch);
    return;
  }

  set_mock(ch,&mock_char);
  set_mock(victim,&mock_victim);

  mock_char.dam -= MAX(1,victim->points.stopping[LOCATION_BODY]/2);
  mock_victim.dam -= MAX(1,ch->points.stopping[LOCATION_BODY]/2);

  if (mock_char.dam<=0) mock_char.dam=1;
  if (mock_victim.dam<=0) mock_victim.dam=1;
  char_roundsr = (double)mock_victim.hp/(double)mock_char.dam;
  victim_roundsr = (double)mock_char.hp/(double)mock_victim.dam;

  temp2 = (20.0-MIN(19,MAX(mock_char.thaco-mock_victim.ac,0))*1.0)/20.0;
  temp  = (20.0-MIN(19,MAX(mock_victim.thaco-mock_char.ac,0))*1.0)/20.0;

  victim_roundsr /= temp;
  char_roundsr /= temp2;

  diff = (int)(char_roundsr-victim_roundsr);

  if(GetMaxLevel(ch) > IMMORTAL || IS_SET(ch->specials.act,PLR_DEBUG)) {
       sendf(ch,"Character: AC: %d   hp: %d   dam: %d   rnd: %d  thaco: %d  thaco%: %g\n",
             mock_char.ac,mock_char.hp,mock_char.dam,(int)char_roundsr, mock_char.thaco,temp2);
       sendf(ch,"Mobile   : AC: %d   hp: %d   dam: %d   rnd: %d  thaco: %d  thaco%: %g\n",
             mock_victim.ac,mock_victim.hp,mock_victim.dam,(int)victim_roundsr, mock_victim.thaco,temp);
       sendf(ch,"Difficulty rating is %d.\n\n",diff);
  }

  if (GetMaxLevel(ch)<=2) scale=5;
  else if (GetMaxLevel(ch)<=10) scale=7;
  else if (GetMaxLevel(ch)<=50) scale=10;
  else if (GetMaxLevel(ch)<=100) scale=20;
  else scale=25;
  if(diff < -30)
    send_to_char("Too easy to be believed.\n", ch);
  else if(diff < -5*scale)
    send_to_char("Not a problem.\n", ch);
  else if(diff < -4*scale)
    send_to_char("Rather easy.\n",ch);
  else if(diff < -3*scale) 
    send_to_char("Easy.\n", ch);
  else if(diff < -2*scale) 
    send_to_char("Fairly easy.\n", ch);
  else if(diff < -1*scale)
    send_to_char("You're a bit more skillful.\n",ch);
  else if(diff < 0)
    send_to_char("The perfect match!\n", ch);
  else if(diff < 1*scale) 
    send_to_char("You would need some luck!\n", ch);
  else if(diff < 2*scale)
    send_to_char("You would need a lot of luck!\n", ch);
  else if(diff < 3*scale)
    send_to_char("You would need a lot of luck and great equipment!\n", ch);
  else if(diff < 4*scale)
    send_to_char("You probably wouldn't survive", ch);
  else if(diff < 5*scale)
    send_to_char("Are you crazy?  Is that your problem?\n", ch);
  else
    send_to_char("You ARE mad!\n", ch);

  diff = HeightClass(ch) - HeightClass(victim) + 6;
  switch(diff) {
    case 0:
    act("You are merely an ant compared to $N", TRUE,ch,0,victim,TO_CHAR);
    break;
    case 1:
    act("You are tiny compared to $N", TRUE,ch,0,victim,TO_CHAR);
    break;
    case 2:
    act("$N's is much taller than you are.",TRUE,ch,0,victim,TO_CHAR);
    break;
    case 3:
    act("$N is several hands bigger than you.",TRUE,ch,0,victim,TO_CHAR);
    break;
    case 4:
    act("$N is a good bit taller than you.",TRUE,ch,0,victim,TO_CHAR);
    break;
    case 5:
    act("$N is a bit taller than you. ",TRUE,ch,0,victim,TO_CHAR);
    break;
    case 6:
    act("You and $N are exactly the same height!",TRUE,ch,0,victim,TO_CHAR);
    break;
    case 7:
    act("You're a bit taller than $N.",TRUE,ch,0,victim,TO_CHAR);
    break;
    case 8:
    act("$N is several hands shorter than you.",TRUE,ch,0,victim,TO_CHAR);
    break;
    case 9:
    act("$N's is much shorter than you are.",TRUE,ch,0,victim,TO_CHAR);
    break;
    case 10:
    act("Your size dwarfs $N.",TRUE,ch,0,victim,TO_CHAR);
    break;
    case 11:
    act("You tower above $N.",TRUE,ch,0,victim,TO_CHAR);
    case 12:
    act("You tower above $N.",TRUE,ch,0,victim,TO_CHAR);
    break;
  }
}

#if 0
void do_spells(struct char_data *ch, char *argument, int cmd)
{
  int spl, i;
  char buf[16384];
  int max_l;
  int c;

  if (IS_NPC(ch))    {
    send_to_char("You ain't nothin' but a hound-dog.\n", ch);
    return;
  }

  argument=one_argument(argument,buf);
  if ((c=which_class(ch,buf,1))<0) return;
  one_argument(argument,buf);
  if (*buf && atoi(buf)>0) max_l=atoi(buf); else max_l=IMPLEMENTOR;
  *buf=0;

  for (i = 0; i < MAX_EXIST_SPELL; i++) {
    if (skill_table[i][c].min_level>0 && skill_table[i][c].min_level<=max_l)
      sprintf(buf + strlen(buf),
	      "[%2d] %-20s  Lv: %4d Mana: %3d Max:%3d\n",
	      i+1, spells[i], skill_table[i][c].min_level,
	      skill_table[i][c].min_mana, skill_table[i][c].max_learn);
  }
  strcat(buf, "\n");
  page_string(ch->desc, buf, 1);
}
#endif

void do_world(struct char_data *ch, char *argument, int cmd)
{
  extern int num_players;
  extern unsigned long meg_sent,data_sent;
  long ct, ot;
  char *tmstr, *otmstr;

  ot = Uptime;
  otmstr = asctime(localtime(&ot));
  *(otmstr + strlen(otmstr) - 1) = '\0';
  sendf(ch, "Start time was: %s (PST)\n", otmstr);

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  sendf(ch, "Current time is: %s (PST)\n", tmstr);
  sendf(ch,"Total number of rooms in world: %d\n",room_db.klistlen);
  sendf(ch,"Total number of zones in world: %d\n\n",top_of_zone_table+1);
  sendf(ch,"MOBS:\n  Total distinct: %d\n",mob_db.klistlen);
  sendf(ch,"  Total undistinct: %d\n",mobs_in_game);
  sendf(ch,"  With special procedures: %d\n",mobs_with_special);
  sendf(ch,"OBJS:\n  Total distinct: %d\n",obj_db.klistlen);
  sendf(ch,"  Total undistinct: %d\n",total_objects);
  if (GetMaxLevel(ch)>=SILLYLORD) {
#define K_DIFF(p1,p2) (abs((unsigned long)(p1)-(unsigned long)(p2))/1024),\
			(abs((unsigned long)(p1)-(unsigned long)(p2)))
#ifdef USE_MALLOP
    struct mallinfo m;
    m=mallinfo();
    sendf(ch,"MEMORY:\n");
    sendf(ch,"  Max : %5dk (%d)\n",K_DIFF(m.arena,0));
    sendf(ch,"  Free: %5dk (%d)\n",K_DIFF(m.fsmblks+m.fordblks,0));
    sendf(ch,"  Used: %5dk (%d)\n",K_DIFF(m.usmblks+m.uordblks,0));
#endif
    sendf(ch,"DATA:\n");
    sendf(ch,"  Sent: %dM %dk\n",meg_sent,data_sent/1024);
  }
  sendf(ch,"\nTotal number of registered players: %d\n",num_players);
}

char *value_stat(int value)
{
  if(value < 3){
    return("extremely low");
  }
  else if(value < 5){
    return("very low");
  }
  else if(value < 8){
    return("low");
  }
  else if(value < 10){
    return("below average");
  }
  else if(value < 12){
    return("average");
  }
  else if(value < 14){
    return("above average");
  }
  else if(value < 17){
    return("high");
  }
  else if(value < 19){
    return("very high");
  }
  else{
    return("extremely high");
  }
}

void do_attribute(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type *aff;

  sendf(ch,"You are %d years and %d months, %d cms, and you weigh %d lbs.\n",
	  GET_AGE(ch), age(ch).month,
	  ch->player.height,
	  ch->player.weight);
  sendf(ch, "You have %d lbs. of carried weight and can carry %d lbs. total.\n", IS_CARRYING_M(ch) / 454, CAN_CARRY_M(ch) / 454);
  sendf(ch, "You have %d pints of carried volume and can carry %d pints total.\n", IS_CARRYING_V(ch) / 237, CAN_CARRY_V(ch) / 237);

  sendf(ch,"You have choosen to be %s.\n",
    GET_TRUE_ALIGN(ch)>0?"Good":GET_TRUE_ALIGN(ch)<0?"Evil":"Neutral");

  sendf(ch,"\n"
	"Current Protection Information (ArmorClass/StoppingPower):\n"
	"     Head: (%-3d/%-3d)   Body: (%-3d/%-3d)\n"
	"     Arms: (%-3d/%-3d)   Legs: (%-3d/%-3d)\n"
	"     Feet: (%-3d/%-3d)   (Good: Low AC, High Stopping)\n\n",
   GET_HEAD_AC(ch),GET_HEAD_STOPPING(ch),GET_BODY_AC(ch),GET_BODY_STOPPING(ch),
   GET_ARMS_AC(ch),GET_ARMS_STOPPING(ch),GET_LEGS_AC(ch),GET_LEGS_STOPPING(ch),
   GET_FEET_AC(ch),GET_FEET_STOPPING(ch));

  send_to_char("Saving vs.\n",ch);
  sendf(ch,"  Paralyzation:  %2d     Breath:  %2d     Rods:  %2d\n"
	   "  Petrification: %2d     Spells:  %2d     (Good: Low Saving)\n",
    saving_throw_val(ch,SAVING_PARA),
    saving_throw_val(ch,SAVING_BREATH),
    saving_throw_val(ch,SAVING_ROD),
    saving_throw_val(ch,SAVING_PETRI),
    saving_throw_val(ch,SAVING_SPELL));

/*  sendf(ch, "Str: %-14s   |   Int: %-14s   |   Wis: %-14s\n",
    value_stat(GET_STR(ch)), value_stat(GET_INT(ch)), value_stat(GET_WIS(ch)));
  sendf(ch, "Dex: %-14s   |   Con: %-14s   |   Cha: %-14s\n",
    value_stat(GET_DEX(ch)), value_stat(GET_CON(ch)), value_stat(GET_CHR(ch)));
  sendf(ch, "Lck: %-14s\n", value_stat(GET_LCK(ch)));
*/
  sendf(ch,"You have %d Str, %d Int, %d Wis, %d Dex, %d Con, %d Cha, %d Lck\n",
	    GET_STR(ch), GET_INT(ch), GET_WIS(ch), GET_DEX(ch), GET_CON(ch), GET_CHR (ch), GET_LCK (ch));

  sendf(ch,"Your hitroll and damroll are %d and %d respectively.\n",
	  GET_HITROLL(ch), GET_DAMROLL(ch));
  if (IsClass(ch,ASSASSIN))
    sendf(ch,"You base chance of assasination is %d.\n",
		get_skill(ch,SKILL_ASSASINATE));

  /*
  **   by popular demand -- affected stuff
  */
  if (ch->affected) {
    send_to_char("\nAffecting Spells:\n--------------\n", ch);
    for(aff = ch->affected; aff; aff = aff->next) {
      switch(aff->type-1) {
      case SKILL_SNEAK:
      case SPELL_POISON:
      case SPELL_CURSE:
	break;
      default:
	if (aff->type<9999)
	  sendf(ch,"Spell : '%s'\n",spells[aff->type]);
	else
	  sendf(ch,"Skill : '%s'\n",skills[aff->type-10000]);
	break;
      }
    }
  }
  if (GetMaxLevel(ch)>=REAL_IMMORT) {
    if (ch->build_hi && ch->build_lo)
      sendf(ch,"Your building range is from %ld to %ld\n",
		ch->build_lo,ch->build_hi);
    else
      send_to_char("You have not been assigned a building range yet.\n",ch);
  }
}


#if 0
void do_users(struct char_data *ch, char *argument, int cmd)
{
  char line[256];
  struct connection_data *d;
  struct char_data *i;
  struct room_data *rp;
  int m=0,n=0,flag,t;
  static most=0;
  int immortals=0;
  int link_dead=0;

  one_argument(argument,line);
  flag=((GetMaxLevel(ch) < IMMORTAL) || (strcmp("-t",line)==0));
  if (!flag) {
    if (!strcmp("-i",line)) immortals = 1;
    if (!strcmp("-l",line)) link_dead = 1;
  }

  if (link_dead) {
    line[0] = 0;
    for (i=character_list; i; i=i->next) {
      if (!IS_NPC(i) && !i->desc) {
        rp=real_roomp(i->in_room);
	sprintf(line+strlen(line),"%2d:", i->specials.timer);
	sprintf(line+strlen(line),"%-14s%2d ",GET_NAME(i),GetMaxLevel(i));
	/*sprintf(line+strlen(line),"%-15s",i->desc->host);*/
	if (!(n%2)) strcat(line,"|");
	else {
	   strcat(line,"\n");
	   send_to_char(line, ch);
	   line[0] = 0;
	}
	++n;
      }
    }
  } else {
    line[0]=0;
    for (d=connection_list;d;d=d->next) {
      ++m;
      if(flag) continue;
      if (d->original) {
        if(!CAN_SEE(ch,d->original)) continue;
        if (GetMaxLevel(d->original) < IMMORTAL && immortals) continue;
        sprintf(line+strlen(line),"%3d%2d:",d->id,
            d->original->specials.timer);
        sprintf(line+strlen(line), "%-14s%2d ",
          GET_NAME(d->original),GetMaxLevel(d->original));
      } else if (d->character) {
	if (GetMaxLevel(ch)!=IMMORTAL)
	  if(!CAN_SEE(ch,d->character)) continue;
        if (GetMaxLevel(d->character) < IMMORTAL && immortals) continue;
        sprintf(line+strlen(line),"%3d%2d:",d->id,
            d->character->specials.timer);
        sprintf(line+strlen(line), "%-12s%4d ",
          (IS_CONN(d)) ? GET_NAME(d->character) : "Not in game",
          GetMaxLevel(d->character));
      } else
        sprintf(line+strlen(line), "%3d%9s%10s ",
          d->id,"  UNDEF  ",connected_types[d->connected]);
      sprintf(line+strlen(line),"%-15s",d->host);
      if(!(n%2)){
        strcat(line,"|");
      } else {
        strcat(line,"\n");
        send_to_char(line, ch);
        line[0]=0;
      }
      ++n;
    }
    if((!flag)&&(n%2)){
      strcat(line,"\n");
      send_to_char(line, ch);
    }
    if(m > most) most=m;
    sprintf(line,"%s%d/%d active connections\n",
      (n%2) ? "\n" : "",m,most);
    send_to_char(line,ch);
    t=30+time(0)-boottime;
    sprintf(line,"Running time %d:%02d\n",t/3600,(t%3600)/60);
    send_to_char(line,ch);
  }
}
#endif

void do_notell(struct char_data *ch, char *argument, int cmd)
{
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->specials.act, PLR_NOTELL)) {
    send_to_char("You can now hear tells again.\n", ch);
    CLEAR_BIT(ch->specials.act, PLR_NOTELL);
  } else {
    send_to_char("From now on, you won't hear tells.\n", ch);
    SET_BIT(ch->specials.act, PLR_NOTELL);
  }
}
