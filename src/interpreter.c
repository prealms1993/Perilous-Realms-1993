 /***************************************************************************
 *  file: Interpreter.c , Command interpreter module.      Part of DIKUMUD *
 *  Usage: Procedures interpreting user command                            *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>

#include <fcns.h>
#include <comm.h>
#include <interpreter.h>
#include <db.h>
#include <utils.h>
#include <limits.h>
#include <inter.h>
#include <race.h>
#include <rooms.h>
#include <hash.h>
#include <ticks.h>
#include <externs.h>
#include <inter.h>

#include <unistd.h>
#include <crypt.h>

#define NOT !
#define AND &&
#define OR ||

#define STATE(d) ((d)->connected)

extern char motd[MAX_STRING_LENGTH];
extern char classhelp[MAX_STRING_LENGTH];
extern racial_adjustment race_adj[];
extern struct char_data *character_list;
extern struct hash_header room_db;
extern struct connection_data *connection_list;

#define MAX_COMMANDS 500
int command_count[MAX_COMMANDS];
int command_time[MAX_COMMANDS];

char echo_on[]  = {IAC, WONT, TELOPT_ECHO, '\n', '\0'};
char echo_off[] = {IAC, WILL, TELOPT_ECHO, '\0'};
int WizLock;
int AcceptAllPasswords;

command_node *command_tree;

static command_node *new_node(int i)
{
  command_node *new;
  CREATE(new,command_node,1);
  new->right = new->left=NULL;
  new->cmd=i;
  return(new);
}

static void insert_cmd(command_node *root, command_node *new)
{
  if (strcmp(cmd_info[root->cmd].cmd,cmd_info[new->cmd].cmd)>0) {
    if (root->right) insert_cmd(root->right,new);
    else root->right=new;
  } else {
    if (root->left) insert_cmd(root->left,new);
    else root->left=new;
  }
}


void build_command_tree()
{
  int i;

  command_tree=new_node(0);
  for (i=1; cmd_info[i].cmd[0]!='\n'; i++)
    insert_cmd(command_tree,new_node(i));
}

char *fill[]=
{ "in",
    "from",
    "with",
    "the",
    "on",
    "at",
    "to",
    "\n"
    };

int norm(int min,int avg,int max)
{
  int i;
  int percent;
  double tick,amt;
  int range;

/* attemp to get a more "normal" random number */
  percent=0;
  for (i=0; i<10; i++) percent+=number(1,100);
  percent/=10;

  if (percent>50)
    range=max-avg;
  else
    range=avg-min;
  percent=percent-50;
  tick=range/50.0;
  amt=tick*percent;
  return(amt+avg);
}

s16 generate_weight(int race)
{
  int i,min,max,avg;
  i=0;
  while (race_adj[i].race!=race && race_adj[i].race) i++;
  if (race_adj[i].race==race) {
    min=race_adj[i].weight[0];
    avg=race_adj[i].weight[1];
    max=race_adj[i].weight[2];
    return(norm(min,avg,max));
  } else return(175);
}

s16 generate_height(int race)
{
  int i,min,max,avg;
  i=0;
  while (race_adj[i].race!=race && race_adj[i].race) i++;
  if (race_adj[i].race==race) {
    min=race_adj[i].height[0];
    avg=race_adj[i].height[1];
    max=race_adj[i].height[2];
    return(norm(min,avg,max));
  }
  else {
    vlog(LOG_URGENT,"Race %d has no ratial norm.",race);
    return(175);
  }
}

int search_block(char *arg, char **list, bool exact)
{
  register int i,l;

  /* Make into lower case, and get length of string */
  for(l=0; *(arg+l); l++)
    *(arg+l)=LOWER(*(arg+l));

  if (exact) {
    for(i=0; **(list+i) != '\n'; i++)
      if (!strcmp(arg, *(list+i)))
	return(i);
  } else {
    if (!l)
      l=1; /* Avoid "" to match the first available string */
    for(i=0; **(list+i) != '\n'; i++)
      if (!strncmp(arg, *(list+i), l))
	return(i);
  }

  return(-1);
}


int find_command(char *cmd)
{
  int diff;
  int i;

  command_node *c;
  c=command_tree;
  while (c) {
    for (i=diff=0; !diff && cmd[i]; i++)
      diff=cmd[i]-cmd_info[c->cmd].cmd[i];
    if (!diff) return(c->cmd);
    if (diff>0) c=c->left;
    else c=c->right;
  }
  return(-1);
}

int old_search_block(char *argument,int begin,int length,char **list,int mode)
{
  int guess, found, search;
  
  
  /* If the word contain 0 letters, then a match is already found */
  found = (length < 1);
  
  guess = 0;
  
  /* Search for a match */
  
  if(mode)
    while ( NOT found AND *(list[guess]) != '\n' )
      {
	found=(length==strlen(list[guess]));
	for(search=0;( search < length AND found );search++)
	  found=(*(argument+begin+search)== *(list[guess]+search));
	guess++;
      } else {
	while ( NOT found AND *(list[guess]) != '\n' ) {
	  found=1;
	  for(search=0;( search < length AND found );search++)
	    found=(*(argument+begin+search)== *(list[guess]+search));
	  guess++;
	}
      }
  
  return ( found ? guess : -1 ); 
}

int command_interpreter(struct char_data *ch, char **argument)
{
  int cmd,i;
  char comd[MAX_INPUT_LENGTH];
  extern int log_all;
  
  if (ch->specials.was_in_room!=NOWHERE) {
    int room;
    room=ch->specials.was_in_room;
    char_from_room(ch);
    char_to_room(ch,room);
    act("$n has returned.\n",TRUE,ch,0,0,TO_ROOM);
    ch->specials.was_in_room=NOWHERE;
  }
  clear_bit(ch->specials.affected_by,AFF_HIDE);
  
  /* Find first non blank */
  for (; isspace(**argument); (*argument)++);
  if (log_all || IS_SET(ch->specials.act,PLR_LOG)) log_action(ch,*argument);
  
  /* Find first word */
  *comd='\0';
  if (**argument=='\'') {
    comd[0]=*((*argument)++);
    comd[1]='\0';
  } else {
    for (i=0; **argument && !isspace(**argument); (*argument)++,i++)
      comd[i]=LOWER(**argument);
    comd[i]='\0';
  }
   
  if (!*comd) return(-2);
  cmd = find_command(comd);
  
  return(cmd);
}

void command_process(struct char_data *ch, int cmd, char *argument)
{
  extern int no_specials;

  if(cmd >= 0){
    if(IS_SET(cmd_info[cmd].flags, CMDF_DISABLED)){
      sendf(ch, "That command has been temporarily disabled.\n");
      return;
    }
    else if(GetMaxLevel(ch) < cmd_info[cmd].minimum_level){
      sendf(ch, "Pardon?\n");
      return;
    }
  }

  if ( cmd>=0 && (cmd_info[cmd].command_pointer != 0))	{
    if ((!IS_AFFECTED(ch, AFF_PARALYSIS)) || (cmd_info[cmd].minimum_position <= POSITION_STUNNED)) {
      if( GET_POS(ch) < cmd_info[cmd].minimum_position ) {
	switch(GET_POS(ch))
	  {
	  case POSITION_DEAD:
	    send_to_char("Lie still; you are DEAD!!! :-( \n", ch);
	    break;
	  case POSITION_INCAP:
	  case POSITION_MORTALLYW:
	    send_to_char(
		"You are in a pretty bad shape, unable to do anything!\n",ch);
	    break;
	    
	  case POSITION_STUNNED:
	    send_to_char(
		 "All you can do right now, is think about the stars!\n", ch);
	    break;
	  case POSITION_SLEEPING:
	    send_to_char("In your dreams, or what?\n", ch);
	    break;
	  case POSITION_RESTING:
	    send_to_char("Nah... You feel too relaxed to do that..\n", ch);
	    break;
	  case POSITION_SITTING:
	    send_to_char("Maybe you should get on your feet first?\n",ch);
	    break;
	  case POSITION_FIGHTING:
	    send_to_char("No way! You are fighting for your life!\n", ch);
	    break;
	  }
      } else {
	time_t start,stop;

	command_count[cmd]++;
	start=time(0);
	if (!no_specials && special(ch, cmd+1, argument))
	  return;  

	((*cmd_info[cmd].command_pointer)
	 (ch, argument, cmd+1));
	stop=time(0);
	command_time[cmd] += (stop-start);
      }
      return;
    } else {
      send_to_char(" You are paralyzed, you can't do much of anything!\n",ch);
      return;
    }
  }
  if ( cmd>=0 && (cmd_info[cmd].command_pointer == 0))
    send_to_char( "Sorry, but that command has yet to be implemented...\n",
		 ch);
  else 
    send_to_char("Pardon? \n", ch);
}

void argument_interpreter(char *argument,char *first_arg,char *second_arg )
{
  int look_at, found, begin;
  
  found = begin = 0;
  
  do
    {
      /* Find first non blank */
      for ( ;*(argument + begin ) == ' ' ; begin++);
      
      /* Find length of first word */
      for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)
	
	/* Make all letters lower case,
	   AND copy them to first_arg */
	*(first_arg + look_at) =
	  LOWER(*(argument + begin + look_at));
      
      *(first_arg + look_at)='\0';
      begin += look_at;
      
    }
  while( fill_word(first_arg));
  
  do
    {
      /* Find first non blank */
      for ( ;*(argument + begin ) == ' ' ; begin++);
      
      /* Find length of first word */
      for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)
	
	/* Make all letters lower case,
	   AND copy them to second_arg */
	*(second_arg + look_at) =
	  LOWER(*(argument + begin + look_at));
      
      *(second_arg + look_at)='\0';
      begin += look_at;
      
    }
  while( fill_word(second_arg));
}

int is_number(char *str)
{
  int look_at;
  
  if(*str=='\0')
    return(0);
  
  for(look_at=0;*(str+look_at) != '\0';look_at++)
    if((*(str+look_at)<'0')||(*(str+look_at)>'9'))
      return(0);
  return(1);
}

/* find the first sub-argument of a string, return pointer to first char in
   primary argument, following the sub-arg			            */
char *one_argument(char *argument, char *first_arg )
{
  int found, begin, look_at;

  found = begin = 0;

  do
    {
      /* Find first non blank */
      for ( ;isspace(*(argument + begin)); begin++);

      /* Find length of first word */
      for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

	/* Make all letters lower case,
	   AND copy them to first_arg */
	*(first_arg + look_at) =
	  LOWER(*(argument + begin + look_at));
      
      *(first_arg + look_at)='\0';
      begin += look_at;
    }
  while (fill_word(first_arg));
  
  return(argument+begin);
}

char *last_argument(char *argument, char *last_arg)
{
    int begin, end;
    for(begin = strlen(argument); (begin > 0) &&
        ((*(argument+begin) == ' ') || (*(argument+begin) == '\0')); begin--);
   
    if(begin <= 0) {
       *last_arg = '\0';
       return(0);
    }

    for(end = begin; *(argument+end) != ' '; end--);

    strncpy(last_arg,argument+end+1,begin-end);
    *(last_arg+begin-end) = '\0';
    return(last_arg);

}


void only_argument(char *argument, char *dest)
{
  while (*argument && isspace(*argument))
    argument++;
  strcpy(dest, argument);
}




int fill_word(char *argument)
{
  return ( search_block(argument,fill,TRUE) >= 0);
}





/* determine if a given string is an abbreviation of another */
int is_abbrev(char *arg1, char *arg2)
{
  if (!*arg1)
    return(0);
  
  for (; *arg1; arg1++, arg2++)
    if (LOWER(*arg1) != LOWER(*arg2))
      return(0);
  
  return(1);
}




/* return first 'word' plus trailing substring of input string */
void half_chop(char *string, char *arg1, char *arg2)
{
  for (; isspace(*string); string++);
  
  for (; !isspace(*arg1 = *string) && *string; string++, arg1++);
  
  *arg1 = '\0';
  
  for (; isspace(*string); string++);

  for (; (*arg2 = *string); string++, arg2++);
}



int special(struct char_data *ch, int cmd, char *arg)
{
  register struct obj_data *i;
  register struct char_data *k;
  int j;

  if (ch->in_room == NOWHERE || !real_roomp(ch->in_room)) {
    char_to_room(ch, LOAD_ROOM_MORTAL);
    return(1);
  }

  /* special in room? */
  if (real_roomp(ch->in_room)->funct)
    if ((*real_roomp(ch->in_room)->funct)(ch, cmd, arg))
      return(1);

  /* special in equipment list? */
  for (j = 0; j < MAX_WEAR; j++)
    if (ch->equipment[j] && real_objp(ch->equipment[j]->virtual))
      if (real_objp(ch->equipment[j]->virtual)->func)
	if ((*real_objp(ch->equipment[j]->virtual)->func)(ch, cmd, arg))
	  return(1);

  /* special in inventory? */
  for (i = ch->carrying; i; i = i->next_content)
    if (real_objp(i->virtual))
      if (real_objp(i->virtual)->func)
	if ((*real_objp(i->virtual)->func)(ch, cmd, arg))
	  return(1);


  /* special in mobile present? */
  for (k = real_roomp(ch->in_room)->people; k; k = k->next_in_room)
    if (IS_MOB(k) && real_mobp(k->virtual))
      if (real_mobp(k->virtual)->func)
	if ((*real_mobp(k->virtual)->func)(ch, cmd, arg))
	  return(1);


  /* special in object present? */
  for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content)
    if (real_objp(i->virtual))
      if (real_objp(i->virtual)->func)
	if ((*real_objp(i->virtual)->func)(ch, cmd, arg))
	  return(1);


  return(0);
}

/* *************************************************************************
 *  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
 ************************************************************************* */


#if 0
/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char *name)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)	{
    if (!str_cmp(player_table[i].name, name))
      return(i);
  }

  return(-1);
}
#endif

static int ok;
static void is_mob_name(int key,index_mem *h,char *name)
{
  if (!h) return;	/* being passed a null mob somewhere */
  if (!ok) return;
  if (isname(name,h->mob->player.name)) ok=0;
}

int new_name_ok(char *name)
{
  FILE *f;
  char name1[20];
  char buf[100];
  int i;

  ok=1;
  hash_iterate(&mob_db,is_mob_name,name);

  f=fopen(BAD_NAMES_FILE,"r");
  if (!f) return(ok);

  for (i=0; i<=strlen(name); i++) name1[i]=LOWER(name[i]);
  while (ok && fgets(buf,100,f)!=NULL) {
    buf[strlen(buf)-1]='\0';
    if (strstr(name1,buf)!=NULL) ok=0;
  }
  fclose(f);
  if (search_block(name, fill, TRUE) != -1) ok = 0;
  if (!str_cmp(name, "self") || !str_cmp(name, "me") || !str_cmp(name, "myself")) ok = 0;
  if (strlen(name)>10) ok=0;
  return(ok);
}

int valid_passwd(char *passwd, char *name)
{
  int spec_char = FALSE;
  int length = 0;
  char *s;

  if(str_str(passwd, name)){
    return(FALSE);
  }

  for(s = passwd; *s; s++){
    length++;
    if(!isalpha(*s)){
      spec_char = TRUE;
    }
  }

  return((length >= 6) && spec_char);
}

int _parse_name(char *arg, char *name)
{
  int i;

  /* skip whitespaces */
  for (; isspace(*arg); arg++);

  for (i = 0; (*name = *arg); arg++, i++, name++)  {
    if ((*arg <0) || !isalpha(*arg) || i > 15)
      return(1);
  }

  if (!i)
    return(1);
  return(0);
}

/********************/
/* States for nanny */
/********************/

void pre_CONFIRM_GET(struct connection_data *d)
{
  SEND_TO_Q("Are you sure? ",d);
  STATE(d)= CON_CONFIRM_GET;
}

void state_CONFIRM_GET(struct connection_data *d, char *arg)
{
  for (;isspace(*arg); arg++);
  if (!*arg)
	  STATE(d)=CON_CONFIRM_GET;
  else if (*arg=='Y' || *arg=='y') {
	  STATE(d)=CON_PLYNG;
	  d->character->act_ptr=1;
	  ((funcp)d->character->user_data)(d->character);
  } else if (*arg=='N' || *arg=='n') {
	  STATE(d)=CON_PLYNG;
	  d->character->act_ptr=0;
	  ((funcp)d->character->user_data)(d->character);
  } else {
	  SEND_TO_Q("Please respond with Y or N\nAre you sure? ",d);
	  STATE(d)=CON_CONFIRM_GET;
  }
}

void pre_QRACE(struct connection_data *d)
{
  SEND_TO_Q("\n\nChoose A Race:\n", d);
  SEND_TO_Q(
" 1. Human          2. Elf\n"
" 3. Dwarf          4. Half-Elf\n"
" 5. Half-Orc       6. Pixie\n"
" 7. Lizardman      8. Troll\n"
" 9. Ogre          10. Giant\n"
"11. Halfling      12. Gnome\n",d);
  SEND_TO_Q("For help type '?'. \n RACE:  ", d);
  STATE(d) = CON_QRACE;
}

void state_QRACE(struct connection_data *d, char *arg)
{
  static int races[] = { RACE_HUMAN, RACE_ELVEN, RACE_DWARF, RACE_H_ELF,
			RACE_H_ORC, RACE_PIXIE, RACE_LIZARDMAN, RACE_TROLL,
			RACE_OGRE, RACE_GIANT, RACE_HALFLING, RACE_GNOME };
  int race;

  race = -1;
  for (; isspace(*arg); arg++)  ;
  race = atoi(arg);
  if (*arg=='?') {
	SEND_TO_Q(RACEHELP,d);
	pre_QRACE(d);
	return;
  }
  if (race<1 || race>12) {
    SEND_TO_Q("\nThat's not a race.\n", d);
    pre_QRACE(d);
    return;
  }
  pre_QSEX(d);
  GET_RACE(d->character)=races[race-1];
}

void pre_NME(struct connection_data *d)
{
  SEND_TO_Q("By what name do you want to be known? ", d);
  STATE(d)=CON_NME;
}

void state_NME(struct connection_data *d, char *arg)
{
  char tmp_name[20];
  char buf[MAX_STRING_LENGTH];
  struct connection_data *k;

  if(strstr(arg, "Snowy rules")){
    SEND_TO_Q("\n\nNo god-damned Tintin users allowed! Get a real client!\n\nPress ENTER to leave.\n", d);
    STATE(d) = CON_WIZLOCK;
    return;
  }

  for (; isspace(*arg); arg++)  ;
  if (!*arg) {
    close_socket(d);
    return;
  }

  if (!d->character) {
    CREATE(d->character, struct char_data, 1);
    clear_char(d->character);
    d->character->desc = d;
  }
  if (_parse_name(arg, tmp_name)) {
    SEND_TO_Q("Illegal name, please try another.\n", d);
    pre_NME(d);
    return;
  }

    /* Check if already playing */
  for(k=connection_list; k; k = k->next) {
    if ((k->character != d->character) && k->character) {
      if (k->original) {
	if (GET_NAME(k->original) && (k->connected != CON_PLYNG) &&
	  (str_cmp(GET_NAME(k->original), tmp_name) == 0)) {
	    SEND_TO_Q("Already playing, cannot connect\n", d);
	    pre_NME(d);
	    return;
	}
      } else {
	if (GET_NAME(k->character) && (k->connected != CON_PLYNG) &&
	    (str_cmp(GET_NAME(k->character), tmp_name) == 0)) {
	    SEND_TO_Q("Already playing, cannot connect\n", d);
	    pre_NME(d);
	    return;
	}
      }
    }
  }

  if ((new_load_char(d->character, tmp_name)) > -1)  {
    strcpy(d->pwd, d->character->player.pw);
    pre_PWDNRM(d);
  } else {                           /* player unknown gotta make a new */
    if (!new_name_ok(tmp_name)) {
      SEND_TO_Q("Illegal name, please try another.\n", d);
      pre_NME(d);
      return;
    } else if (unfriendly_domain(d->host,no_new,n_no_new)) {
      vlog(LOG_REJECT,"New player reject from %s",d->host);
      alarm(5); /*
      write_to_id(d->id,"\n\n\7\7\7\7\7\7\7\7Because of morons from your site, new players are banned from your site.\n\n",d->term);
*/
      write_to_id(d->id,"\n\n\7\7\7\7\7\7\7\7Perilous Realms has temporarly gone back to a testing only/registration mode\nuntil we can get some problems fixed and major new changes settled. If you\nfeel you MUST play feel free to mail accounts@odin.wosc.osshe.edu\n\n-Deth Imp Perilous Realms\n\n",d->term);
      alarm(0);
      close_socket(d);
    } else if (!WizLock) {
      if (!allow_desc(player_count(), d)) {
	vlog(LOG_REJECT,"Game Full Reject %s<new>[%s]",tmp_name,d->host);
	write_to_id(d->id, toomany, d->term);
	close_socket(d);
	return;
      }
      CREATE(GET_NAME(d->character), char, strlen(tmp_name) + 1);
      strcpy(GET_NAME(d->character), CAP(tmp_name));
      sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
      SEND_TO_Q(buf, d);
      STATE(d) = CON_NMECNF;
    } else {
      sprintf(buf, "Sorry, %s, no new characters at this time\n",
		tmp_name);
      SEND_TO_Q(buf,d);
      STATE(d) = CON_WIZLOCK;
    }
  }
}

int player_count()
{
  struct connection_data *c;
  int i;
  /* count the number on */
  for (i = 0, c = connection_list; c; c = c->next){
    if ((c->character && (GET_LEVEL(c->character) < IMMORTAL) &&
	!(c->flags & CONNF_PRIV)) ||
	(!c->character)) {
      i++;
    }
  }
  return(i);
}


void pre_PWDNRM(struct connection_data *d)
{
  SEND_TO_Q("Password: ", d);
  write(d->id, echo_off, 4);
  STATE(d) = CON_PWDNRM;
}

void state_PWDNRM(struct connection_data *d, char *arg)
{
  struct char_data *tmp_ch;
  char buf[MAX_STRING_LENGTH];

  write(d->id, echo_on, 5);
  for (; isspace(*arg); arg++);
  if (!*arg) {
    close_socket(d);
    return;
  }
  //2014.  Any password is treated as okay.
  if (AcceptAllPasswords) {
    SEND_TO_Q("Password check bypassed.  Any password accepted.\n", d);
  } else
  if (strncmp(crypt(arg, d->pwd), d->pwd, 10)) 	{
    SEND_TO_Q("Wrong password.\n", d);
    pre_PWDNRM(d);
    return;
  }
  if (!allow_desc(player_count(), d)) {
    vlog(LOG_REJECT,"Game Full Reject %s[%s]",GET_NAME(d->character),d->host);
    write_to_id(d->id, toomany, d->term);
    close_socket(d);
    return;
  }


  if (WizLock && !IS_IMMORTAL(d->character)) {
	SEND_TO_Q("Only immortals are allowed on at this time.\n",d);
	STATE(d)=CON_WIZLOCK;
	return;
  }
  for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
    if ((!str_cmp(GET_NAME(d->character), GET_NAME(tmp_ch)) &&
	!IS_NPC(tmp_ch)) || (IS_NPC(tmp_ch) && tmp_ch->orig &&
	!str_cmp(GET_NAME(d->character),GET_NAME(tmp_ch->orig)))) {

      if(tmp_ch->desc){
        close_socket(tmp_ch->desc);
        SEND_TO_Q("Reconnected into already playing character.\n", d);
      }
      else{
        SEND_TO_Q("Reconnecting.\n", d);
      }
      free_char(d->character);
      tmp_ch->desc = d;
      d->character = tmp_ch;
      tmp_ch->specials.timer = 0;
      if (tmp_ch->orig) {
	tmp_ch->desc->original = tmp_ch->orig;
	tmp_ch->orig = 0;
      }
      STATE(d) = CON_PLYNG;

      act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
      if (WAS_IN_ROOM(d)!=NOWHERE) {
	char_from_room(d->character);
	char_to_room(d->character,WAS_IN_ROOM(d));
	WAS_IN_ROOM(d)=NOWHERE;
      }
      sprintf(buf, "%s[%s] has reconnected.",
				      GET_NAME(d->character),d->host);
      if (GetMaxLevel(d->character)<IMPLEMENTOR) vlog(LOG_CONNECTION,buf);
      else log2(buf);
      return;
    }

  if(time(0) > (d->character->player.time.password + SECS_PER_REAL_DAY * 30)){
    pre_PWDEXP(d);
    return;
  }

  sprintf(buf, "%s[%s] has connected.", GET_NAME(d->character), d->host);
  if (d->character && GetMaxLevel(d->character)<IMPLEMENTOR) 
	vlog(LOG_CONNECTION,buf);
  else log2(buf);

  if (is_abbrev("137.229",d->host)) {
    vlog(LOG_URGENT,"WARNING: %s has connected from a possibly illegal site please watch them!", GET_NAME(d->character));
  }

  pre_RMOTD(d);
}

void pre_PWDEXP(struct connection_data *d)
{
  SEND_TO_Q("Your password has expired, please choose a new one.\nPassword: ", d);
  write(d->id, echo_off, 4);
  STATE(d) = CON_PWDEXP;
}

void state_PWDEXP(struct connection_data *d, char *arg)
{
  write(d->id, echo_on, 5);

  for (; isspace(*arg); arg++);

  if(!strncmp(d->character->player.pw, crypt(arg, d->character->player.name), 10)){
    SEND_TO_Q("You must select a new password.\n", d);
    pre_PWDEXP(d);
    return;
  }
  if (!valid_passwd(arg, d->character->player.name)) {
    SEND_TO_Q("Password must be at least six characters including one non-letter.\n", d);
    pre_PWDEXP(d);
    return;
  }

  strncpy(d->pwd, crypt(arg, d->character->player.name), 10);
  *(d->pwd + 10) = '\0';
  SEND_TO_Q("Please retype password: ", d);
  write(d->id, echo_off, 4);
  STATE(d) = CON_PWDECNF;
}

void state_PWDECNF(struct connection_data *d, char *arg)
{
  write(d->id, echo_on, 5);
  /* skip whitespaces */
  for (; isspace(*arg); arg++);

  if (strncmp(crypt(arg, d->pwd), d->pwd, 10)) {
    SEND_TO_Q("Passwords don't match.\n", d);
    pre_PWDEXP(d);
  }
  else{
    FREE(d->character->player.pw);
    d->character->player.pw = mystrdup(d->pwd);
    d->character->player.time.password = time(0);
    save_char(d->character, NOWHERE);
    pre_RMOTD(d);
  }
}

void pre_RMOTD(struct connection_data *d)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  if(d->character->player.last_site){
    cftime(buf2, "%a %b %e %T %Y", &(d->character->player.time.logon));
    sprintf(buf, "\nYou last logged in on %s from %s.\n", buf2,
      d->character->player.last_site);
    SEND_TO_Q(buf, d);
  }
  else{
    SEND_TO_Q("\nThis is the first time you've logged in.\n", d);
  }
  FREE(d->character->player.last_site);
  d->character->player.last_site = mystrdup(d->host);
  if(d->character->player.time.motd < motd_time){
    d->character->player.time.motd = motd_time;
    SEND_TO_Q(motd, d);
  }
  save_char(d->character, NOWHERE);
  SEND_TO_Q("\n[-return-]", d);
  STATE(d)=CON_RMOTD;
}

void state_RMOTD(struct connection_data *d,char *arg)
{
  char buf[MAX_STRING_LENGTH];

  SEND_TO_Q(MENU, d);
  STATE(d) = CON_SLCT;
  if (WizLock) {
    if (d->character && GetMaxLevel(d->character) < LOW_IMMORTAL) {
      printf(buf, "Sorry, the game is locked up for repair\n");
      SEND_TO_Q(buf,d);
      STATE(d) = CON_WIZLOCK;
    }
  }
}

void state_NMECNF(struct connection_data *d,char *arg)
{
  for (; isspace(*arg); arg++);

  if (*arg == 'y' || *arg == 'Y')	{
    SEND_TO_Q("New character.\n", d);

    pre_PWDGET(d);
  } else 	{
    if (*arg == 'n' || *arg == 'N') {
      SEND_TO_Q("Ok, what IS it, then? ", d);
      FREE(GET_NAME(d->character));
      STATE(d) = CON_NME;
    }
    else SEND_TO_Q("Please type Yes or No? ", d);
  }
}

void pre_PWDGET(struct connection_data *d)
{
  char buf[MAX_STRING_LENGTH];

  sprintf(buf, "Give me a password for %s: ",GET_NAME(d->character));

  SEND_TO_Q(buf, d);
  write(d->id, echo_off, 4);
  STATE(d) = CON_PWDGET;
}

void state_PWDGET(struct connection_data *d, char *arg)
{
  write(d->id, echo_on, 5);

  for (; isspace(*arg); arg++);

  if (!valid_passwd(arg, d->character->player.name)) {
    SEND_TO_Q("Password must be at least six characters including one non-letter.\nPassword: ", d);
    write(d->id, echo_off, 4);
    return;
  }

  strncpy(d->pwd, crypt(arg, d->character->player.name), 10);
  *(d->pwd + 10) = '\0';
  SEND_TO_Q("Please retype password: ", d);
  write(d->id, echo_off, 4);
  d->character->player.pw=MALLOC(strlen(d->pwd)+1,char);
  strcpy(d->character->player.pw,d->pwd);
  STATE(d) = CON_PWDCNF;
}

void pre_PWDCNF(struct connection_data *d)
{
  SEND_TO_Q("Please retype password: ",d);
  write(d->id, echo_off, 4);
  STATE(d) = CON_PWDGET;
}

void state_PWDCNF(struct connection_data *d, char *arg)
{
  write(d->id, echo_on, 5);
  /* skip whitespaces */
  for (; isspace(*arg); arg++);

  if (strncmp(crypt(arg, d->pwd), d->pwd, 10)) {
    SEND_TO_Q("Passwords don't match.\n", d);
    pre_PWDGET(d);
  }
  else{
    d->character->player.time.password = time(0);
    pre_QSTATS(d);
  }
}

void pre_QSEX(struct connection_data *d)
{
  SEND_TO_Q("\nAre you a Male or a Female? ", d);
  STATE(d) = CON_QSEX;
}

void state_QSEX(struct connection_data *d, char *arg)
{
    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    switch (*arg) {
      case 'm': case 'M': d->character->player.sex = SEX_MALE;	 break;
      case 'f': case 'F': d->character->player.sex = SEX_FEMALE; break;
      default:
	SEND_TO_Q("That's not a sex..\n", d);
	SEND_TO_Q("What IS your sex? :", d);
	return;
    }
    pre_QCLASS(d);
}

static int can_be(struct char_data *ch,class_entry *c)
{
  int j;

  if (  GET_STR(ch)>=c->min[0] &&
	GET_INT(ch)>=c->min[1] &&
	GET_WIS(ch)>=c->min[2] &&
	GET_DEX(ch)>=c->min[3] &&
	GET_CON(ch)>=c->min[4] &&
	GET_CHR(ch)>=c->min[5] &&
	GET_LCK(ch)>=c->min[6]) { /* ok.. their abilities check out :) */
    for (j=0; j<c->nraces; j++)
	if (GET_RACE(ch)==c->races[j]) return(1);
  }
  return(0);
}

void pre_QCLASS(struct connection_data *d)
{
  int i,count;
  char buf[MAX_STRING_LENGTH];
  class_entry *c;
  struct char_data *ch;
  int can_be_something;

  SEND_TO_Q("\nSelect a class ('*' denotes classes available to you):\n",d);
  ch=d->character;
  count=0;
  can_be_something=0;
  for (i=0; i<MAX_DEF_CLASS; i++) {
    c=classes+i;
    sprintf(buf,"%2d. %s %-20s",++count,
	   can_be(d->character,c)?"*": " ",c->class_name);
    if (!can_be_something) can_be_something=can_be(ch,c);
    SEND_TO_Q(buf,d);
    if ((i+1)%3==0) SEND_TO_Q("\n", d);
  }
  SEND_TO_Q("\n", d);
  if (!can_be_something) {
    SEND_TO_Q("Oops.. Looks like you can't be anything with your race and those stats.\nYou gotta start over now!\n",d);
    pre_QSTATS(d);
    return;
  }
  SEND_TO_Q("\nClass :", d);
  STATE(d) = CON_QCLASS;
}

void state_QCLASS(struct connection_data *d, char *arg)
{
  class_entry *c;
  int i;

  for (; isspace(*arg); arg++);
  d->character->player.class = 0;
  if (*arg=='h' || *arg=='H') {
      send_to_char(CLASS_HELP,d->character);
      STATE(d)=CON_QCLASS;
      return;
  }
  i=atoi(arg)-1;
  if (i>=0 && i<MAX_DEF_CLASS && can_be(d->character,classes+i)) {
    d->character->player.class=i;
    c=classes+i;
    GET_STR(d->character)=MIN(c->max[0],GET_STR(d->character));
    GET_INT(d->character)=MIN(c->max[1],GET_INT(d->character));
    GET_WIS(d->character)=MIN(c->max[2],GET_WIS(d->character));
    GET_DEX(d->character)=MIN(c->max[3],GET_DEX(d->character));
    GET_CON(d->character)=MIN(c->max[3],GET_CON(d->character));
    GET_CHR(d->character)=MIN(c->max[3],GET_CHR(d->character));
    GET_LCK(d->character)=MIN(c->max[3],GET_LCK(d->character));
/* Racial Adjustments go here*/
    i= 0;
    while (race_adj[i].race!=GET_RACE(d->character)) i++;
    GET_STR(d->character) += race_adj[i].adj[0];
    GET_INT(d->character) += race_adj[i].adj[1];
    GET_WIS(d->character) += race_adj[i].adj[2];
    GET_DEX(d->character) += race_adj[i].adj[3];
    GET_CON(d->character) += race_adj[i].adj[4];
    GET_CHR(d->character) += race_adj[i].adj[5];
    GET_LCK(d->character) += race_adj[i].adj[6];
/* End of Racial Adjustments */
    d->character->abilities=d->character->tmpabilities;
    pre_QALIGN(d);
  } else {
    SEND_TO_Q("That class isn't available with you Race/Abilities.\n",d);
    pre_QCLASS(d);
  }
}

void pre_FINAL(struct connection_data *d)
{
  SEND_TO_Q("\n\nKeep this character? ",d);
  STATE(d)=CON_FINAL;
}

void state_FINAL(struct connection_data *d,char *arg)
{
  if (*arg=='Y' || *arg=='y') {
    init_char(d->character);
    create_entry(GET_NAME(d->character));
	save_char(d->character, NOWHERE);
    pre_RMOTD(d);
  } else if (*arg=='N' || *arg=='n') {
    SEND_TO_Q("Have it your way.  So Long.\n",d);
    STATE(d)=CON_WIZLOCK;
  } else {
    SEND_TO_Q("A simple Yes or No will do.\n",d);
    pre_FINAL(d);
  }
}

void state_DELETE_ME(struct connection_data *d, char *arg)
{
  char name[MAX_INPUT_LENGTH];
  if (!str_cmp(arg,"delete me")) {
    strcpy(name,GET_NAME(d->character));
    close_socket(d);
    unlink(name_to_path(name));
    vlog(LOG_CONNECTION|LOG_MISC|LOG_DEATH,"%s deleted self",name);
  } else {
    SEND_TO_Q("Aborted.\n",d);
    SEND_TO_Q(MENU, d);
    STATE(d)=CON_SLCT;
  }
}

void menu_delete(struct connection_data *d, char *arg)
{
  SEND_TO_Q("This will permanently delete you.  To continue type\ndelete me\n> ",d);
  STATE(d)=CON_DELETE_ME;
}

void menu_regen(struct connection_data *d,char *arg)
{
  if (GET_LEVEL(d->character)<1) {
    SEND_TO_Q("Try entering the game first.\n",d);
    SEND_TO_Q(MENU,d);
    return;
  }
  SEND_TO_Q("\n\nThis will put you in regeration mode while you are out of the game.\nWhen you come back, you will have regained 22 hp per real minute and 40 mana\nper real minute.  If you stay away for 90 minutes, you will be fully healed\nwhen you return.  The cost of this is 10 coins per hp healed and 5 coins per\nmana point restored.\n\nIs this what you would like to do? ",d);
  STATE(d)=CON_REGEN;
}

void state_REGEN(struct connection_data *d, char *arg)
{
  for (;isspace(*arg);arg++);
  if (LOWER(*arg)=='y') {
    SET_BIT(d->character->specials.act,PLR_REGEN);
    d->character->needs_saved=1;
    close_socket(d);
    return;
  }
  SEND_TO_Q("So be it.\n",d);
  STATE(d)=CON_SLCT;
  SEND_TO_Q(MENU,d);
}

void menu_enter(struct connection_data *d, char *arg)
{
  d->character->needs_saved=1;
  reset_char(d->character);

  d->character->player.pw=
		REALLOC(d->character->player.pw,strlen(d->pwd)+1,char);
  strcpy(d->character->player.pw,d->pwd);

  send_to_char(WELC_MESSG, d->character);

  d->character->next = character_list;
  character_list = d->character;

  if (WAS_IN_ROOM(d)== NOWHERE || !real_roomp(WAS_IN_ROOM(d))) {
    if (GetMaxLevel(d->character) >= IMPLEMENTOR)
      char_to_room(d->character,LOAD_ROOM_IMPLEMENTOR);
    else if (GetMaxLevel(d->character) >= SILLYLORD)
      char_to_room(d->character,LOAD_ROOM_SILLYLORD);
    else if (GetMaxLevel(d->character) >= LOW_IMMORTAL)
      char_to_room(d->character, LOAD_ROOM_IMMORTAL);
    else if (IS_SET(d->character->specials.act,PLR_BANISHED)) {
      /*nlog("Player starting as BANISHED: %s",GET_NAME(d->character));*/
      char_to_room(d->character, LOAD_ROOM_BANISHED);
    } else
      char_to_room(d->character, LOAD_ROOM_MORTAL);
  } else {
    if (real_roomp(WAS_IN_ROOM(d))) {
      char_to_room(d->character, WAS_IN_ROOM(d));
      WAS_IN_ROOM(d)=NOWHERE;
    } else
      char_to_room(d->character, LOAD_ROOM_MORTAL);
  }

  act("$n has entered the game.",TRUE, d->character, 0, 0, TO_ROOM);
  STATE(d) = CON_PLYNG;
  if (!GetMaxLevel(d->character)) {
    vlog(LOG_CONNECTION,"New startup: %s",GET_NAME(d->character));
    do_start(d->character);
  } 
  do_look(d->character, "",15);

  if (IS_SET(d->character->specials.act,PLR_REGEN)) {
    struct char_data *ch;
    int time_out;
    int hp_heal,mana_heal,power_heal;
    int old_bank;

    ch=d->character;
    CLEAR_BIT(ch->specials.act,PLR_REGEN);
    hp_heal=MAX(0,hit_limit(ch)-GET_HIT(ch));
    mana_heal=MAX(0,mana_limit(ch)-GET_MANA(ch));
    power_heal=MAX(0,power_limit(ch)-GET_POWER(ch));
    time_out=(time(0)- ch->player.time.logon)/60;
    if (time_out<90) {
      if (22*time_out<hp_heal) hp_heal=22*time_out;
      if (40*time_out<mana_heal) mana_heal=40*time_out;
      if (40*time_out<power_heal) power_heal=40*time_out;
    }
    old_bank=GET_BANK(ch);
    GET_GOLD(ch) += GET_BANK(ch);
    if (GET_GOLD(ch)<10*hp_heal)
      hp_heal=GET_GOLD(ch)/10;
    sendf(ch,"You healed %d hp in regeneration mode.\n",hp_heal);
    GET_HIT(ch) += hp_heal;
    GET_GOLD(ch) -= (hp_heal*10);
    if (GET_GOLD(ch)<5*mana_heal)
      mana_heal=GET_GOLD(ch)/5;
    sendf(ch,"You regained %d mana points.\n",mana_heal);
    GET_MANA(ch) += mana_heal;
    GET_GOLD(ch) -= (mana_heal*5);
    if (GET_GOLD(ch)<5*power_heal)
      power_heal=GET_GOLD(ch)/5;
    sendf(ch,"You regained %d power points.\n",power_heal);
    GET_POWER(ch) += power_heal;
    GET_GOLD(ch) -= (power_heal*5);
    sendf(ch,"The total cost to you was %d coins.\n",hp_heal*10+mana_heal*5+power_heal*5);
    if (GET_GOLD(ch)<old_bank) {
      sendf(ch,"%d coins were taken from your bank account since you didn't have\nenough money on hand.\n",abs(GET_GOLD(ch)-old_bank));
      GET_BANK(ch)=GET_GOLD(ch);
      GET_GOLD(ch)=0;
    } else {
      GET_BANK(ch)=old_bank;
      GET_GOLD(ch)-=old_bank;
    }
    GET_BANK(ch)=MAX(0,GET_BANK(ch));
    GET_GOLD(ch)=MAX(0,GET_GOLD(ch));
  }

  d->character->player.time.logon = time(0);

  if (GET_GOLD(d->character)>(1<<30))
    GET_GOLD(d->character)=0;

  if (d->character->mail_list) {
    struct obj_data *obj;
    mail_item *m, *n;
    for (m=d->character->mail_list; m; m=n) {
      n=m->next;
      obj=get_obj(NOTE_NUMBER);
      obj->action_description=m->mail;
      obj_to_char(obj,d->character);
      FREE(m);
    }
    d->character->mail_list=0;
    act("A mail daemon runs up to $n and gives $m a note.",TRUE,d->character,0,0,TO_ROOM);
    sendf(d->character,"\nA mail daemon runs up to you and gives you a note.\n\n");
  }
  update_warehouse(d->character);
  do_save(d->character,"",15);
}

void menu_newdesc(struct connection_data *d, char *arg)
{
  SEND_TO_Q("Enter a text you'd like others to see when they look at you.\n", d);
  SEND_TO_Q("Terminate with an '@'.\n", d);
  if (d->character->player.description)	{
      SEND_TO_Q("Old description :\n", d);
      SEND_TO_Q(d->character->player.description, d);
      FREE(d->character->player.description);
      d->character->player.description = 0;
    }
  d->str =
    &d->character->player.description;
  d->max_str = 240;
  STATE(d) = CON_EXDSCR;
}

void menu_readstory(struct connection_data *d, char *arg)
{
  SEND_TO_Q(STORY, d);
  pre_RMOTD(d);
}

void menu_newpasswd(struct connection_data *d, char *arg)
{
  SEND_TO_Q("Enter a new password: ", d);
  write(d->id, echo_off, 4);

  STATE(d) = CON_PWDNEW;
}

void menu_finger(struct connection_data *d)
{
  SEND_TO_Q("Enter your full name: ", d);
  STATE(d) = CON_FULLNAME;
}

void state_FULLNAME(struct connection_data *d, char *arg)
{
  for(; isspace(*arg); arg++);
  FREE(d->character->player.full_name);
  d->character->player.full_name = mystrdup(arg);
  if(d->character->player.email ? !*d->character->player.email : 1){
    SEND_TO_Q("E-Mail address: ", d);
    STATE(d) = CON_EMAIL;
  }
  else{
    SEND_TO_Q("Enter your plan.\n", d);
    SEND_TO_Q("Terminate with an '@':\n", d);
    if(d->character->player.plan ? *d->character->player.plan : 0){
      SEND_TO_Q("Old plan:\n", d);
      SEND_TO_Q(d->character->player.plan, d);
      FREE(d->character->player.plan);
    }
    d->str = &d->character->player.plan;
    d->max_str = 512;
    STATE(d) = CON_EXDSCR;
  }
}

void state_EMAIL(struct connection_data *d, char *arg)
{
  for(; isspace(*arg); arg++);
  FREE(d->character->player.email);
  d->character->player.email = mystrdup(arg);
  SEND_TO_Q("Enter your plan.\n", d);
  SEND_TO_Q("Terminate with an '@':\n", d);
  if(d->character->player.plan ? *d->character->player.plan : 0){
    SEND_TO_Q("Old plan:\n", d);
    SEND_TO_Q(d->character->player.plan, d);
    FREE(d->character->player.plan);
  }
  d->str = &d->character->player.plan;
  d->max_str = 512;
  STATE(d) = CON_EXDSCR;
}

void state_SLCT(struct connection_data *d, char *arg)
{
  for (; isspace(*arg); arg++);
  switch (*arg) {
    case '0': close_socket(d);      break;
    case '1': menu_enter(d,arg);     break;
    case '2': menu_newdesc(d,arg);   break;
    case '3': menu_readstory(d,arg); break;
    case '4': menu_newpasswd(d,arg); break;
    case '5': menu_delete(d,arg);    break;
    case '6': menu_regen(d,arg);     break;
    case '7': menu_finger(d);        break;
    default:
      SEND_TO_Q("Wrong option.\n", d);
      SEND_TO_Q(MENU, d);
  }
}

void state_PWDNEW(struct connection_data *d, char *arg)
{
  for (; isspace(*arg); arg++);

  write(d->id, echo_on, 5);
  if (!valid_passwd(arg, d->character->player.name)) {
    SEND_TO_Q("Password must be at least six characters including one non-letter.\n", d);
    SEND_TO_Q("Password: ", d);
    write(d->id, echo_off, 4);
    return;
  }

  strncpy(d->pwd, crypt(arg, d->character->player.name), 10);
  *(d->pwd + 10) = '\0';

  pre_PWDCNF(d);
  STATE(d)=CON_PWDNCNF;
}

void state_PWDNCNF(struct connection_data *d, char *arg)
{

  write(d->id, echo_on, 5);
  for (; isspace(*arg); arg++);

  if (strncmp(crypt(arg, d->pwd), d->pwd, 10))      {
    SEND_TO_Q("Passwords don't match.\n", d);
    pre_PWDCNF(d);
    STATE(d)=CON_PWDNEW;
    return;
  }

  SEND_TO_Q("\nDone.\n", d);
  d->character->player.time.password = time(0);
  save_char(d->character, NOWHERE);
  SEND_TO_Q(MENU, d);
  STATE(d) = CON_SLCT;
}

void pre_QALIGN(struct connection_data *d)
{
  SEND_TO_Q("\nWould you like to be G)ood, N)uetral, or E)vil? ",d);
  STATE(d)=CON_QALIGN;
}

void state_QALIGN(struct connection_data *d,char *arg)
{
  int align;
  int v;

  for (; isspace(*arg); arg++);
  switch(*arg) {
	case 'G': case 'g': align=1000;   v=ALIGN_G; break;
	case 'E': case 'e': align= -1000; v=ALIGN_E; break;
	case 'N': case 'n': align=0;	  v=ALIGN_N; break;
	case '?': SEND_TO_Q(ALIGN_HELP,d); return;
	default: SEND_TO_Q("You must choose one!\n",d);
		pre_QALIGN(d);
		return;
  }
  if (CLASS(d->character).alignment & v) {
    GET_ALIGNMENT(d->character)=align;
    GET_TRUE_ALIGN(d->character)=align;
    pre_FINAL(d);
  } else {
    SEND_TO_Q("That alignment is not allowed with your class.\n",d);
    pre_QALIGN(d);
  }
}

void pre_QSTATS(struct connection_data *d)
{
#define MAX_CHOICES 5
#define NUM_STATS 7
  int stats[MAX_CHOICES][NUM_STATS];
  int rols[4];
  int i,j,k;
  char buf[MAX_STRING_LENGTH];

  for (i=0; i<MAX_CHOICES; i++)
    for (j=0; j<NUM_STATS; j++) {
      for (k=0; k<4; k++)
	rols[k]=dice(1,6);
      stats[i][j]=rols[0]+rols[1]+rols[2]+rols[3]-
		MIN(MIN(rols[0],rols[1]),MIN(rols[2],rols[3]));
    }
  SEND_TO_Q("\nYou can choose from any of the following sets of abilities:\n",d);
  for (i=0; i<MAX_CHOICES; i++) {
    sprintf(buf,"%2d. Str: %2d, Int: %2d, Wis: %2d, Dex: %2d, Con: %2d Chr: %2d Lck: %2d\n",
	i+1,stats[i][0],stats[i][1],stats[i][2],stats[i][3],
	    stats[i][4],stats[i][5],stats[i][6]);
    SEND_TO_Q(buf,d);
  }
  SEND_TO_Q("Choice? [Enter=Roll again, H for help on classes, Q to quit from the mud] ",d);
  if (d->character->user_data) FREE(d->character->user_data);
  CREATE(d->character->user_data,int,MAX_CHOICES*NUM_STATS);
  memcpy(d->character->user_data,stats,MAX_CHOICES*NUM_STATS*sizeof(int));
  STATE(d)=CON_QSTATS;
}

void state_QSTATS(struct connection_data *d,char *arg)
{
  int i;
  int stats[MAX_CHOICES][NUM_STATS];
  char buf[MAX_STRING_LENGTH];

  for (; isspace(*arg); arg++);
  if (*arg=='Q' || *arg=='q') {
	SEND_TO_Q("See ya round!\n",d);
	STATE(d)=CON_WIZLOCK;
	return;
  } else if (*arg=='H' || *arg=='h') {
	SEND_TO_Q(classhelp, d);
    memcpy(stats,d->character->user_data,MAX_CHOICES*NUM_STATS*sizeof(int));
  SEND_TO_Q("\nYou can choose from any of the following sets of abilities:\n",d);
  for (i=0; i<MAX_CHOICES; i++) {
    sprintf(buf,"%2d. Str: %2d, Int: %2d, Wis: %2d, Dex: %2d, Con: %2d Chr: %2d Lck: %2d\n",
	i+1,stats[i][0],stats[i][1],stats[i][2],stats[i][3],
	    stats[i][4],stats[i][5],stats[i][6]);
    SEND_TO_Q(buf,d);
  }
  SEND_TO_Q("Choice? [Enter=Roll again, H for help on classes, Q to quit from the mud] ",d);
  STATE(d)=CON_QSTATS;
	return;
  }
  if (atoi(arg)<1 || atoi(arg)>MAX_CHOICES) pre_QSTATS(d);
  else {
    memcpy(stats,d->character->user_data,MAX_CHOICES*NUM_STATS*sizeof(int));
    i=atoi(arg)-1;
    GET_STR(d->character)=stats[i][0];
    GET_INT(d->character)=stats[i][1];
    GET_WIS(d->character)=stats[i][2];
    GET_DEX(d->character)=stats[i][3];
    GET_CON(d->character)=stats[i][4];
    GET_CHR(d->character)=stats[i][5];
    GET_LCK(d->character)=stats[i][6];
    d->character->abilities=d->character->tmpabilities;
    FREE(d->character->user_data);
    d->character->user_data=NULL;
    pre_QRACE(d);
  }
}

void do_terminal(struct char_data *ch, char *arg)
{
  int i;

  if (!ch->desc) return;
  for (;isspace(*arg) && *arg; arg++);
  for (i=0; termlist[i].name; i++)
    if (str_cmp(arg,termlist[i].name)==0) break;
  if (!termlist[i].name) {
    sendf(ch,"Choices are:\n");
    for (i=0; termlist[i].name; i++)
      sendf(ch,"\t%s\n",termlist[i].name);
    sendf(ch,"\nYour terminal type is currently: %s\n",
		termlist[ch->desc->term].name);
    return;
  }
  ch->desc->term=i;
  sendf(ch,"Your terminal type is now: %s\n",termlist[i].name);
}

void pre_TERM(struct connection_data *d)
{
  char buf[255];

  sprintf(buf,"Term type? (?=list, Enter=%s) ",termlist[0].name);
  SEND_TO_Q(buf,d);
  STATE(d)=CON_TERM;
}

void state_TERM(struct connection_data *d, char *arg)
{
  int i;

  for (;isspace(*arg) && *arg; arg++);
  for (i=0; termlist[i].name; i++)
    if (str_cmp(termlist[i].name,arg)==0) break;
  if (!*arg) i=0;
  if (!termlist[i].name) {
    SEND_TO_Q("Choices are:\n",d);
    for (i=0; termlist[i].name; i++) {
      SEND_TO_Q("\t",d);
      SEND_TO_Q(termlist[i].name,d);
      SEND_TO_Q("\n",d);
    }
    SEND_TO_Q("\n",d);
    pre_TERM(d);
    return;
  }
  d->term=i;
  SEND_TO_Q(welcome,d);
  pre_NME(d);
}

/* deal with newcomers and other non-playing sockets */
void nanny(struct connection_data *d, char *arg)
{
  switch (STATE(d)) {
    case CON_CONFIRM_PROMPT: pre_CONFIRM_GET(d);     break;
    case CON_CONFIRM_GET:  state_CONFIRM_GET(d,arg); break;
    case CON_QALIGN:	   state_QALIGN(d,arg);	     break;
    case CON_QSTATS:	   state_QSTATS(d,arg);	     break;
    case CON_QRACE:        state_QRACE(d,arg);       break;
    case CON_NME:	   state_NME(d,arg);         break;
    case CON_NMECNF:	   state_NMECNF(d,arg);      break;
    case CON_PWDNRM: 	   state_PWDNRM(d,arg);      break;
    case CON_PWDGET:	   state_PWDGET(d,arg);      break;
    case CON_PWDCNF:	   state_PWDCNF(d,arg);      break;
    case CON_QSEX:	   state_QSEX(d,arg);        break;
    case CON_QCLASS : 	   state_QCLASS(d,arg);      break;
    case CON_RMOTD: 	   state_RMOTD(d,arg);       break;
    case CON_WIZLOCK:      close_socket(d);          break;
    case CON_SLCT:         state_SLCT(d,arg);	     break;
    case CON_PWDNEW:	   state_PWDNEW(d,arg);	     break;
    case CON_PWDNCNF:      state_PWDNCNF(d,arg);     break;
    case CON_FINAL:        state_FINAL(d,arg);       break;
    case CON_OBJECT:       oedit_prompt(d->character,arg); break;
    case CON_MOB:	   medit_prompt(d->character,arg); break;
    case CON_DELETE_ME:    state_DELETE_ME(d,arg);   break;
    case CON_REGEN:        state_REGEN(d,arg);       break;
    case CON_PWDEXP:       state_PWDEXP(d, arg);     break;
    case CON_PWDECNF:      state_PWDECNF(d, arg);    break;
    case CON_FULLNAME:     state_FULLNAME(d, arg);   break;
    case CON_EMAIL:        state_EMAIL(d, arg);      break;
    case CON_TERM:         state_TERM(d,arg);	     break;
    default:
      vlog(LOG_URGENT,"Nanny: illegal state of con'ness");
      abort();
  }
}
