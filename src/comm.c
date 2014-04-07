/* ************************************************************************
*  file: comm.c , Communication module.                   Part of DIKUMUD *
*  Usage: Communication, central game loop.                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
*  Using *any* part of DikuMud without having read license.doc is         *
*  violating our copyright.
************************************************************************* */

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/file.h>
#include <stdlib.h>
#ifdef USE_MALLOP
/*#include <malloc.h>*/
#endif

#include <fcns.h>
#include <utils.h>
#include <comm.h>
//#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <ticks.h>
#include <externs.h>

#define DFLT_PORT 4000        /* default port */
#define MAX_NAME_LENGTH 15
#define MAX_HOSTNAME   256
#define OPT_USEC 100000      /* time delay corresponding to 4 passes/sec */
int baddoms;
char wizlocked[BADDOMS][32];

int n_no_new;
char no_new[BADDOMS][32];

#define STATE(d) ((d)->connected)

struct connection_data *connection_list, *next_to_process;
int boottime;
int shutdwn = 0;       /* clean shutdown */
int reboot = 0;         /* reboot the game after a shutdown */
int crash_save = 1;     /* save the players on a crash? */
int no_specials = 0;    /* Suppress ass. of special routines */
long Uptime;            /* time that the game has been up */
char *sbrk(int);
char *init_sbrk;
char *db_sbrk;
int purge_days = 90;

int maxdesc, abs_max_con,max_players, max_saved;
int ConvertOnly=FALSE;
unsigned long data_sent=0;
unsigned long meg_sent=0;

event_t *event_queue[DEF_PULSE_PLAYER];

int rcon = -1;
int rmud = -1;
char rmud_buf[MAX_STRING_LENGTH];

/* *********************************************************************
*  main game loop and related stuff				       *
********************************************************************* */

int main(int argc, char **argv) /*do not remove this comment*/
{
  int port;
  char buf[512];
  int pos = 1;
  char *dir;
  
  *rmud_buf = '\0';
#ifdef USE_MALLOP
  mallopt(M_MXFAST,16);
  mallopt(M_NLBLKS,100);
  mallopt(M_GRAIN,16);
#endif
  init_sbrk=sbrk(0);

  port = DFLT_PORT;
  dir = DFLT_DIR;

  WizLock = FALSE;
  AcceptAllPasswords = FALSE;

  baddoms=0;
  while ((pos < argc) && (*(argv[pos]) == '-'))	{
    switch (*(argv[pos] + 1))  {
    case 'c':
	ConvertOnly = TRUE;
	break;
    case 'w':
	WizLock=TRUE;
	break;
    case 'p':
	if (*(argv[pos]+2))
	  purge_days=atoi(argv[pos]+2);
	else if (++pos < argc)
	  purge_days=atoi(argv[pos]);
	else {
	  log("Purge interval expected after -p.");
	  exit(0);
	}
	break;
    case 'b':
      if (*(argv[pos] + 2))
	DEBUG = atoi(argv[pos] + 2);
      else if (++pos < argc)
	DEBUG = atoi(argv[pos]);
      else   	{
	log("DEBUG level expected after -b");
	exit(0);
      }
      break;
	case 'a':
		AcceptAllPasswords = TRUE;
		log("Security bypassed.  Any password is accepted.");
		break;
    case 'n':
      crash_save = 0;
      log("Nice core dump mode.");
      break;
    case 'd':
      if (*(argv[pos] + 2))
	dir = argv[pos] + 2;
      else if (++pos < argc)
	dir = argv[pos];
      else   	{
	log("Directory arg expected after option -d.");
	exit(0);
      }
      break;
    case 's':
      no_specials = 1;
      log("Suppressing assignment of special routines.");
      break;
    default:
      sprintf(buf, "Unknown option -% in argument string.",
	      *(argv[pos] + 1));
      log(buf);
      break;
    }
    pos++;
  }
  
  if (pos < argc)
    if (!isdigit(*argv[pos]))      	{
      fprintf(stderr, "Usage: %s [-l] [-s] [-d pathname] [ port # ]\n", 
	      argv[0]);
      exit(0);
    }  else if ((port = atoi(argv[pos])) <= 1024)  {
      printf("Illegal port #\n");
      exit(0);
    }
  
  Uptime = time(0);
  
  sprintf(buf, "Running game on port %d.", port);
  log(buf);
  
  if (chdir(dir) < 0)	{
    perror("chdir");
    exit(0);
  }
  
  sprintf(buf, "Using %s as data directory.", dir);
  log(buf);
  
  srandom(boottime=time(0));
  run_the_game(port);
  return(0);
}


int unfriendly_domain(char *h,char baddomain[BADDOMS][32],int baddoms)
{
   int i;

   for(i=0;i<baddoms;++i){
      if(strncmp(h,baddomain[i],strlen(baddomain[i]))==0){
         return(1);
      }
   }
   return(0);
}


/* Init sockets, run game, and cleanup sockets */
int run_the_game(int port)
{
  int i;
  int s=0;
  char c;
  FILE *f;
    
  connection_list = NULL;
  
  if (!ConvertOnly) {
    log("Signal trapping.");
    signal_setup();
    
    log("Opening mother connection.");
    s = init_socket(port);
    log("Opening listener connection.");
    rmud = init_socket(REMOTE_PORT);
  }

  /* initialize event queue */
  for(i = 0; i < DEF_PULSE_PLAYER; i++){
    event_queue[i] = NULL;
  }

  tick_reset();

  boot_db();
  max_saved = -1;
  log("Max players: ");
  if ((f = fopen("maxplayers", "r"))) {
    max_saved = 0;
    do{
      c = fgetc(f);
      if(isdigit(c)){
        max_saved *= 10;
        max_saved += c - '0';
      }
    } while(isdigit(c));
    nlog("%d", max_saved);
    fclose(f);
  }
  else {
    log("unlimited");
  }
  n_no_new=0;
  log("No new:");
  if ((f=fopen("no-new","r"))!=NULL) {
	while (fgets(no_new[n_no_new],80,f)) {
	  no_new[n_no_new][strlen(no_new[n_no_new])-1]='\0';
	  log(no_new[n_no_new]);
	  n_no_new++;
	}
	fclose(f);
  }
  log("Locked hosts:");
  if ((f=fopen("wizlocked","r"))!=NULL) {
	while (fgets(wizlocked[baddoms],80,f)) {
	  wizlocked[baddoms][strlen(wizlocked[baddoms])-1]='\0';
	  log(wizlocked[baddoms]);
	  baddoms++;
	}
	fclose(f);
  }
  
  db_sbrk=sbrk(0);
  log("Entering game loop.");
  
  game_loop(s);
  
  close_sockets(s); 
  
  if (reboot)  {
    log("Rebooting.");
    exit(52);           /* what's so great about HHGTTG, anyhow? */
  }
  
  log("Normal termination of game.");
  return(0);
}

void handle_remote_mud(int inp, int outp, int excp)
{
  char buf[MAX_STRING_LENGTH];

  if (excp) {
    vlog(LOG_URGENT,"Lost mud listener");
    if (rmud != -1)
      rmud = init_socket(REMOTE_PORT);
    if (rcon != -1) {
      close(rcon);
      rcon = -1;
    }
    *rmud_buf = '\0';
    return;
  }
  if (inp) {
    int br;
    br = read(rcon,buf,sizeof(buf)-1);
    if (br<=0) {
      handle_remote_mud(0,0,1);
      return;
    }
    buf[br] = '\0';
    do_commune(NULL,buf,-1);
  }
  if (outp && *rmud_buf) {
    if (write(rcon,rmud_buf,strlen(rmud_buf)) <0) {
      handle_remote_mud(0,0,1);
      return;
    }
    *rmud_buf = '\0';
  }
}

/* Accept new connects, relay commands, and call 'heartbeat-functs' */
int game_loop(int s)
{
#ifdef BSD
  int mask;
#endif
  int delayed, command;
  event_t *event;
  fd_set input_set, output_set, exc_set;
  struct timeval last_time, now, timespent, timeout, null_time;
  static struct timeval opt_time;
  char comm[MAX_INPUT_LENGTH];
  char promptbuf[MAX_STRING_LENGTH];
  struct connection_data *point, *next_point;

  timer_reset();
  
  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  
  opt_time.tv_usec = PULSE_TIMEOUT;	/* Init time values */
  opt_time.tv_sec = 0;

  gettimeofday(&last_time, NULL);
  
  maxdesc = s+3;
  /* !! Change if more needed !! */
#if 1	//2014 - needed to increase beyond 21 connections on cygwin and probably others
  if (max_saved > 15) setdtablesize(max_saved + 64);
#endif
#ifdef _SEQUENT_
  setdtablesize(128);
#endif
  abs_max_con = getdtablesize() - 10; /* we need 10 free file handles */
  if(max_saved == -1){
    max_players = abs_max_con - 2;
  }
  else{
    max_players=MIN(max_saved, abs_max_con - 2);
  }
  nlog("Available connections = %d",abs_max_con);

#ifdef BSD
  mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
    sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
      sigmask(SIGURG) | sigmask(SIGHUP);
#endif
  
  /* Main loop */
  while (!shutdwn)  {
    if (TIMER) SET_TIMER(TIMER_GAME);
    if (DEBUG>9) integrity_check();

    /* Check what's happening out there */
    FD_ZERO(&input_set);
    FD_ZERO(&output_set);
    FD_ZERO(&exc_set);
    FD_SET(s, &input_set);

    if (rmud != -1)
      FD_SET(rmud, &input_set);

    if (rcon != -1) {
      FD_SET(rcon,&output_set);
      FD_SET(rcon,&input_set);
      FD_SET(rcon,&exc_set);
    }

    for (point = connection_list; point; point = point->next)  {
      FD_SET(point->id, &input_set);
      FD_SET(point->id, &exc_set);
      FD_SET(point->id, &output_set);
    }
    
    /* check out the time */
    gettimeofday(&now, NULL);
    timespent = timediff(&now, &last_time);
    timeout = timediff(&opt_time, &timespent);
/*  fprintf(stderr,"%2d:%7d - %2d:%7d - %2d:%7d %2d:%7d\n",
	timespent.tv_sec,timespent.tv_usec,
	timeout.tv_sec,timeout.tv_usec,
	now.tv_sec,now.tv_usec,
	last_time.tv_sec,last_time.tv_usec
); */
    last_time.tv_sec = now.tv_sec + timeout.tv_sec;
    last_time.tv_usec = now.tv_usec + timeout.tv_usec;
    if (last_time.tv_usec >= 10000000) {
      last_time.tv_usec -= 10000000;
      last_time.tv_sec++;
    }
    
#ifdef BSD
    sigsetmask(mask);
#endif
    
    if (select(maxdesc+1, &input_set, &output_set, &exc_set, &null_time) < 0) {
      perror("Select poll");
      return(-1);
    }
    
    if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
      perror("Select sleep");
    }
    
#ifdef BSD
    sigsetmask(0);
#endif
    
    /* Respond to whatever might be happening */
    
    /* New connection? */
    if (FD_ISSET(s, &input_set))
      if (new_connection(s) < 0)
	perror("New connection");

    if ((rmud != -1) && FD_ISSET(rmud, &input_set))
      mud_connect();

    if (rcon != -1)
      handle_remote_mud(FD_ISSET(rcon,&input_set),FD_ISSET(rcon,&output_set),
			FD_ISSET(rcon,&exc_set));
    
    for (point = connection_list; point; point = next_point)  {
      next_point = point->next;   
      if (IS_CONN(point) && number(1,10000)==3)
        do_save(point->character,"",-1);
      if (FD_ISSET(point->id, &exc_set))  {
	FD_CLR(point->id, &input_set);
	FD_CLR(point->id, &output_set);
	close_socket(point);
      } else if (FD_ISSET(point->id, &input_set)) {
	if (process_input(point) < 0)
	  close_socket(point);
      }
    }
   
    /* process events */
    for(event = event_queue[TICS % DEF_PULSE_PLAYER]; event;
        event = event_queue[TICS % DEF_PULSE_PLAYER]){
      event_queue[TICS % DEF_PULSE_PLAYER] = event->next;
      if(event->type != EVT_REMOVED){
        (*event->function)(event);
      }
      FREE(event->args);
      FREE(event);
    }

    /* process_commands; */
    for (point = connection_list; point; point = next_to_process) {
      next_to_process = point->next;

      delayed = 0;

      if(point->character){
	delayed = 1;
	if((--point->character->specials.wait) <= 0){
	  point->character->specials.wait = 0;
	  delayed = 0;
        }
      }
      if(!delayed && get_from_pq(&point->input, comm, 0, &command)){
        if(point->character){
	  point->character->specials.timer = 0;
	  point->character->specials.wait = 1;
        }
	point->need_prompt = 1;
	
	if (point->str)
	  string_add(point, comm);
	else if (point->showstr_point)
	  show_string(point, comm);
	else if(point->character && (point->connected == CON_PLYNG)){
	  if(command != -2)
	    command_process(point->character, command, comm);
        }
        else
	  nanny(point, comm); 

	if (point->str && point->character)
	  SET_BIT(point->character->specials.act, PLR_VEG);
	else if (point->character)
	  CLEAR_BIT(point->character->specials.act, PLR_VEG);
      }
    }
    for (point = connection_list; point; point = next_to_process) {
      next_to_process = point->next;
      if (FD_ISSET(point->id, &output_set) && 
          (point->output.head || (point->need_prompt != 2))){
        if ((point->need_prompt == 0) &&
            MAX(0, (TICS - point->last_tick)) > 3){
          point->need_prompt = 1;
        }

        if ((point->output.head && point->need_prompt == 2) ||
            (point->need_prompt == 1)){
          if (process_output(point) < 0){
            close_socket(point);
            continue;
          }
        }

        if (point->need_prompt == 1){
          point->last_tick = TICS;
          /* give the people some prompts  */
          point->need_prompt = 2;  /* new id means just did prompt */
          if(point->str)
            write_to_id(point->id,"] ",point->term);
          else if(point->connected==CON_PLYNG){
            if(point->showstr_point){
              write_to_id(point->id,
                "1[Press return to continue, q to quit]0 ",point->term);
            }
            else{ 
              make_prompt(point->character, promptbuf);
              if(!(point->character && !IS_NPC(point->character) &&
                  IS_SET(point->character->specials.act, PLR_COMPACT)))
                write_to_id(point->id, "\n",point->term);
              write_to_id(point->id, promptbuf,point->term);
            }
          }
        }
      }
    }
    
    /* handle heartbeat stuff */
    /* Note: pulse now changes every 1/4 sec  */
    
    PULSE++;
    
    if (!(PULSE % PULSE_ZONE)) {
      if (TIMER) SET_TIMER(TIMER_ZONE);
      zone_update();
      if (TIMER) SET_TIMER(TIMER_ZONE);
    }
    
/*  if (!(PULSE % PULSE_MOBILE)) {
      if (TIMER) SET_TIMER(TIMER_MOBILE);
      mobile_activity();
      if (TIMER) SET_TIMER(TIMER_MOBILE);
      integrity_check();
    } */
    
/*  if (!(PULSE % PULSE_RIVER)) {
      if (TIMER) SET_TIMER(TIMER_RIVER);
      down_river(PULSE);
      if (TIMER) SET_TIMER(TIMER_RIVER);
    } */

    if (!(PULSE % PULSE_AUDIO)) {
      if (TIMER) SET_TIMER(TIMER_AUDIO);
      MakeSound(PULSE);
      if (TIMER) SET_TIMER(TIMER_AUDIO);
    }
    
/*  if (!(PULSE % PULSE_TELEPORT)) {
      if (TIMER) SET_TIMER(TIMER_TELEPORT);
      Teleport(PULSE);
      if (TIMER) SET_TIMER(TIMER_TELEPORT);
    } */
    
    if (!(PULSE % PULSE_VIOLENCE)) {
      if (TIMER) SET_TIMER(TIMER_VIOLENCE);
/*    perform_violence(PULSE); */
      perform_special_rooms (PULSE);
      if (TIMER) SET_TIMER(TIMER_VIOLENCE);
    }
    
#if 0
    if (!(PULSE % PULSE_FALL)) {
      if (TIMER) SET_TIMER(TIMER_FALL);
      all_fall_down();
      if (TIMER) SET_TIMER(TIMER_FALL);
    }
      
    if (!(PULSE % PULSE_DROWN)) {
      if (TIMER) SET_TIMER(TIMER_DROWN);
      glug_glug_glug();
      if (TIMER) SET_TIMER(TIMER_DROWN);
    }
#endif
    
     if (!(PULSE % PULSE_PLAYER)) {
      if (DEBUG) vlog(LOG_DEBUG,"tick");
      if (TIMER) {
	event_tick();
        SET_TIMER(TIMER_PLAYER);
        SET_TIMER(TIMER_WEATHER);
        weather_and_time(1);
        SET_TIMER(TIMER_WEATHER);
        SET_TIMER(TIMER_AFFECT);
        affect_update();
        SET_TIMER(TIMER_AFFECT);
        SET_TIMER(TIMER_POINT);
        point_update();
        SET_TIMER(TIMER_POINT);
        SET_TIMER(TIMER_PLAYER);
      } else {
	event_tick();
	weather_and_time(1);
	affect_update();
	point_update();
      }
    }

    if (PULSE >= PULSE_MAX) {
      PULSE = DEF_PULSE;
    }

    TICS++;			/* tics since last checkpoint signal */

    if (TIMER) SET_TIMER(TIMER_GAME);
	if (reboot_time)
		handle_reboot ();
  } /* while !shutdown */

  close_player_log();
  return(1);
}

void rem_char_events(struct char_data *ch)
{
  int i;
  event_t *event;

  for(i = 0; i < DEF_PULSE_PLAYER; i++){
    for(event = event_queue[i]; event; event = event->next){
      if((event->ch == ch) || (event->victim == ch)){
        event->type = EVT_REMOVED;
        event->ch = NULL;
        event->victim = NULL;
      }
    }
  }
}

void rem_obj_events(struct obj_data *obj)
{
  int i;
  event_t *event;

  for(i = 0; i < DEF_PULSE_PLAYER; i++){
    for(event = event_queue[i]; event; event = event->next){
      if(event->obj == obj){
         event->type = EVT_REMOVED;
      }
    }
  }
}

void add_event(int delay, int type, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int room, int virtual, char *args, void (*function)(event_t *))
{
  int idx;
  event_t *event;

  /* don't allow events to be added for a mobile after it's death */
  if(ch){
    if((GET_POS(ch) == POSITION_DEAD) && (type != EVT_DEATH)){
      return;
    }
  }
  if(victim){
    if(GET_POS(victim) == POSITION_DEAD){
      return;
    }
  }

  CREATE(event, event_t, 1);
  event->type     = type;
  event->ch       = ch;
  event->victim   = victim;
  event->obj      = obj;
  event->room     = room;
  event->virtual  = virtual;
  event->args     = mystrdup(args);
  event->function = function;

  idx = (TICS + MAX(1, delay)) % DEF_PULSE_PLAYER;
  event->next = event_queue[idx];
  event_queue[idx] = event;
}

void zapper()
{
   int dow,tod,t;

   t=time(0)-18000;
   dow=((t/86400)+5)%7;
      tod=(t%86400)/60;
      if(tod > 500) return;
      if(tod > 410) send_to_all("The game will shut down soon.\n");

      if(tod >= 415){
         shutdwn=1;
         if(getppid() > 1)
            kill(getppid(),9);
      }
}

int get_from_q(struct txt_q *queue, char *dest,int fold)
{
	struct txt_block *tmp;
	char buf[20];
	int length;

 	/* Q empty? */
	if (!queue->head)
		return(0);

	length=0;
	while (queue->head) {
	  tmp = queue->head;
	  if (length && length+tmp->length+20>MAX_STRING_LENGTH)
	    return(1);

	  if (tmp->copies==0) {
	    strcpy(dest+length, tmp->text);
	    length += tmp->length;
	  } else {
	    sprintf(buf,"(%d*)",tmp->copies+1);
	    sprintf(dest+length,"%s%s",buf,tmp->text);
	    length+= tmp->length;
	    length+=strlen(buf);
	  }

	  queue->head = queue->head->next;

	  FREE(tmp->text);
	  FREE(tmp);
	  if (!fold) return(1);
        }
	return(1);
}

int get_from_pq(struct txt_q *queue, char *dest,int fold, int *command)
{
	struct txt_block *tmp;
	char buf[20];
	int length;

 	/* Q empty? */
	if (!queue->head)
		return(0);

	length=0;
	while (queue->head) {
	  tmp = queue->head;
	  *command = tmp->command;
	  if (length && length+tmp->length+20>MAX_STRING_LENGTH)
	    return(1);

	  if (tmp->copies==0) {
	    strcpy(dest+length, tmp->text);
	    length += tmp->length;
	  } else {
	    sprintf(buf,"(%d*)",tmp->copies+1);
	    sprintf(dest+length,"%s%s",buf,tmp->text);
	    length+= tmp->length;
	    length+=strlen(buf);
	  }

	  queue->head = queue->head->next;

	  FREE(tmp->text);
	  FREE(tmp);
	  if (!fold) return(1);
        }
	return(1);
}

void write_to_pq(char *txt, struct txt_q *queue, int priority, int command)
{
  struct txt_block *new, *prev, *curr, *next;
  int i;

  if(!queue){
    vlog(LOG_URGENT, "Output message to non-existant queue");
    return;
  }

  i = strlen(txt);

  if (i>=MAX_STRING_LENGTH) {
    i=MAX_STRING_LENGTH-1;
    vlog(LOG_URGENT,"String truncated: write_to_q");
  }

  CREATE(new, struct txt_block, 1);
  CREATE(new->text, char, i + 1);
  new->copies=0;
  new->length=i;

  strncpy(new->text, txt,i);
  new->text[i]='\0';

  new->command = command;
  if((command < 0) || !priority){
    new->priority = 0;
  }
  else{
    new->priority = cmd_info[command].priority;
  }

  for(prev = NULL, curr = queue->head; curr; curr = curr->next){
    if(new->priority > curr->priority) break;
    prev = curr;
  }

  new->next = curr;
  if(IS_SET(cmd_info[command].flags, CMDF_FLUSH) && priority){
    /* get rid of other commands */
    for(curr = new->next; curr; curr = next){
      next = curr->next;
      FREE(curr->text);
      FREE(curr);
    }
    new->next = NULL;
  }
  if(prev){
    prev->next = new;
  }
  else{
    queue->head = new;
  }
  if(!new->next){
    queue->tail = new;
  }
}

void write_to_q(const char *txt, struct txt_q *queue,int fold)
{
	struct txt_block *new;
	struct txt_block *p;
	int i;

        if (!queue) {
	  vlog(LOG_URGENT,"Output message to non-existant queue");
	  return;
	}

	i = strlen(txt);

	if (i>=MAX_STRING_LENGTH) {
	  i=MAX_STRING_LENGTH-1;
	  vlog(LOG_URGENT,"String truncated: write_to_q");
	}

/* if doing string folding, try to match with something already there */
	if (fold && i>5 && queue->head && txt[i-1]=='\n') {
	  for (p=queue->head; p; p=p->next) {
	    if (p->length==i)
	      if (strcmp(p->text,txt)==0) {
		p->copies++;
		return;
	      }
	  }
        }

	CREATE(new, struct txt_block, 1);
	CREATE(new->text, char, i + 1);
	new->copies=0;
	new->length=i;

	strncpy(new->text, txt,i);
	new->text[i]='\0';

	/* Q empty? */
	if (!queue->head)  {
		new->next = NULL;
		queue->head = queue->tail = new;
	} else	{
		queue->tail->next = new;
		queue->tail = new;
		new->next = NULL;
	}
}

struct timeval timediff(struct timeval *a, struct timeval *b)
{
  struct timeval result;

  result.tv_usec = a->tv_usec - b->tv_usec;
  result.tv_sec = a->tv_sec - b->tv_sec;
  if(result.tv_usec < 0){
    result.tv_usec += 1000000;
    result.tv_sec--;
  }
  if(result.tv_sec < 0){
    result.tv_usec = 0;
    result.tv_sec = 0;
  }

  return(result);
}

/* Empty the queues before closing connection */
void flush_queues(struct connection_data *d)
{
	char dummy[MAX_STRING_LENGTH];

	while (get_from_q(&d->output, dummy,1));
	while (get_from_q(&d->input, dummy,1));
}






/* ******************************************************************
*  socket handling							 *
****************************************************************** */




int init_socket(int port)
{
  int s;
  char *opt;
  char hostname[MAX_HOSTNAME+1];
  struct sockaddr_in sa;
  struct hostent *hp;
  struct linger ld;

  bzero(&sa, sizeof(struct sockaddr_in));
  gethostname(hostname, MAX_HOSTNAME);
  hp = gethostbyname(hostname);
  if (hp == NULL) {
    printf("hostname: %s\n", hostname);
    perror("gethostbyname");
    exit(1);
  }
  sa.sin_family = hp->h_addrtype;
  sa.sin_port	= htons(port);
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    perror("Init-socket");
    exit(1);
  }
  if (setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char *)&opt,sizeof(opt)) < 0) {
    perror ("setsockopt REUSEADDR");
    exit (1);
  }

  ld.l_onoff = 0;
  ld.l_linger = 0;
  if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *)&ld, sizeof(ld)) < 0) {
    perror("setsockopt LINGER");
    exit(1);
  }
  if (bind(s, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
    perror("bind");
    close(s);
    exit(1);
  }
  listen(s, 3);
  if (s>maxdesc) maxdesc=s+1;
  return(s);
}



/* print an internet host address prettily */
static void printhost(struct in_addr *addr,char * buf)
{
#if 0
    sprintf(buf,"%3d.%3d.%3d.%3d",
            (addr->s_addr & 0xff000000) >> 24,
            (addr->s_addr & 0xff0000) >> 16,
            (addr->s_addr & 0xff00) >> 8,
            (addr->s_addr & 0xff));
#else
    sprintf(buf,"%3d.%3d.%3d.%3d",
            (addr->s_addr & 0xff),
            (addr->s_addr & 0xff00) >> 8,
            (addr->s_addr & 0xff0000) >> 16,
            (addr->s_addr & 0xff000000) >> 24);
#endif
}


void mud_connect()
{
  struct sockaddr_in isa;
  int i;
  int t;
  int s;
  char buf[100];

  alarm(30);

  s=rmud;
  i = sizeof(isa);
  if (getsockname(s, (struct sockaddr *)&isa, &i) == -1) {
    perror("getsockname()");
    return;
  }

  if ((t = accept(s, (struct sockaddr *)&isa, &i)) < 0) {
    alarm(0);
    perror("Accept");
    return;
  }
  nonblock(t);

  printhost(&isa.sin_addr, buf);
  vlog(LOG_URGENT, "New mud connection from [%s] fd=%d.", buf,t);
  alarm(0);

  rcon = t;
  if (rcon>maxdesc) maxdesc=rcon+1;
  close(rmud);
  rmud = -1;
}


int new_id(int s,char *from)
{
  struct sockaddr_in isa;
  int i;
  int t;
  char buf[100];

  alarm(30);

  i = sizeof(isa);
  if (getsockname(s, (struct sockaddr *)&isa, &i) == -1) {
    perror("getsockname()");
    return(-1);
  }

  if ((t = accept(s, (struct sockaddr *)&isa, &i)) < 0) {
    alarm(0);
    perror("Accept");
    return(-1);
  }
  nonblock(t);

  printhost(&isa.sin_addr, from);
  sprintf(buf, "New connection from [%s].", from);
  slog(buf);
  alarm(0);

  return(t);
}

int same_addr(int ip1[4], int ip2[4])
{
  int i;

  for(i = 0; i < 4; i++){
    if((ip1[i] != ip2[i]) && (ip1[i] != 255) && (ip2[i] != 255)){
      return(0);
    }
  }

  return(1);
}

int allow_desc(int cnt, struct connection_data *desc)
{
  int allow = FALSE;
  FILE *fd;
  char buf[100];
  int ip1[4], ip2[4];

  desc->flags = 0;
  /* find if host should be allowed -- override */
  if ((fd = fopen (ALLOW_SITES_ALWAYS, "r")) != NULL) {
    while (fgets (buf, 99, fd)) {
      buf[strlen (buf) - 1] = '\0';
      sscanf(buf, "%d.%d.%d.%d", &ip1[0], &ip1[1], &ip1[2], &ip1[3]);
      sscanf(desc->host, "%d.%d.%d.%d", &ip2[0], &ip2[1], &ip2[2], &ip2[3]);
      if (same_addr(ip1, ip2)) {
	allow = TRUE;
	break;
      }
    }
    fclose (fd);
  }

  if (allow && (cnt < abs_max_con)) {
    desc->flags |= CONNF_PRIV;
    return(TRUE);
  }

  if (GET_LEVEL(desc->character)>=LOW_IMMORTAL)
    return(TRUE);

  if (cnt <= max_players) return(TRUE);

  return (FALSE);
}

int new_connection(int s)
{
  int desc;
  struct connection_data *newd;
  int i;
  char from[255];

  if ((desc = new_id(s,from)) < 0)
    return (-1);
  

  CREATE(newd, struct connection_data, 1);

#if 0
  /* find info */
  size = sizeof(sock);
  if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0)	{
    perror("getpeername");
    *newd->host = '\0';
  }
  else {
    printhost(&sock.sin_addr, newd->host);
  }
#endif
  strcpy(newd->host,from);

#if 0
/* done in interpreter.c now */
  /* count the number on */
  for (i = 0, d = connection_list; d; d = d->next){
    if ((d->character && (GET_LEVEL(d->character) < IMMORTAL) &&
	!(d->flags & CONNF_PRIV)) ||
	(!d->character)){
      i++;
    }
  }

  if (!allow_desc(i, newd)) {
    vlog(LOG_REJECT,"Game Full Reject from %s",newd->host);
    write_to_id(desc, toomany, TERM_OTHER);
    alarm(5);
    close(desc);
    alarm(0);
    return(0);
  }
#endif

  if (desc > maxdesc)
    maxdesc = desc;

  if (unfriendly_domain(newd->host,wizlocked,baddoms)) {
    vlog(LOG_REJECT,"Reject from %s",newd->host);
    write_to_id(desc,"\n\n\7\7\7\7\7\7\7\7Sorry, the game is unavailable from your site.\7\7\7\7\7\n",TERM_OTHER);
    /* free (newd->host); */
    free (newd);
    alarm(5);
    close (desc);
    alarm(0);
    return(0);
  }
  /* init desc data */
  newd->id = desc;
  newd->connected  = CON_TERM;
  newd->wait = 1;
  *newd->buf = '\0';
  newd->str = 0;
  newd->showstr_head = 0;
  newd->showstr_point = 0;
  *newd->last_input= '\0';
  newd->output.head = NULL;
  newd->input.head = NULL;
  newd->next = connection_list;
  newd->character = 0;
  newd->original = 0;

  newd->snooping = NULL;
  for (i=0; i<MAX_SNOOP; i++)
    newd->snooped_by[i] = NULL;

  /* prepend to list */

  connection_list = newd;

  pre_TERM(newd);

  return(0);
}
	




int process_output(struct connection_data *t)
{
  char i[MAX_STRING_LENGTH + 1];
  int j;

  if ((t->need_prompt == 2) && !t->connected){
    t->need_prompt = 0;
    if (write_to_id(t->id, "\n",t->term) < 0)
      return(-1);
  }

  /* Cycle thru output queue */
  while (get_from_q(&t->output, i,1)) {  
    for (j=0; j<MAX_SNOOP; j++)
      if (t->snooped_by[j]) {
	SEND_TO_Q("% ",t->snooped_by[j]);
	SEND_TO_Q(i,t->snooped_by[j]);
      }
      if (write_to_id(t->id, i,t->term))
	return(-1);
  }
  
  return(1);
}


int write_to_id(int desc, char *txt,int term)
{
  int sofar, thisround, total;
  char buf[MAX_STRING_LENGTH*4];
  int args;
  char *p,*q;
  
  for (p=buf; *txt; txt++) {
    if (*txt==3) {
      txt++;
      if (termlist[term].color[*txt-'0']) {
	strcpy(p,termlist[term].color[*txt-'0']);
	p+=strlen(termlist[term].color[*txt-'0']);
      }
    } else if (*txt=='\n' && *(txt+1)!='\r') {
      *(p++) = '\r';
      *(p++) = '\n';
    } else if (*txt==1) { /* control-A means use next char as termcap */
      txt++;
      q=termlist[term].cap[(*txt)-1];
      args=0;
      while (q && *q) {
	while (*q && *q!='#')
	  *(p++) = *(q++);
	if (*q=='#') {
	  q++;
	  sprintf(p,"%d",txt[*q-'0']);
	  if (*q-'0' > args) args = *q-'0';
	  p += strlen(p);
	  q++;
	}
      }
      txt+=args;
    } else (*p++) = *txt;
  }
  *p='\0';
  total = strlen(buf);
  sofar = 0;
  
  do {
    thisround = write(desc, buf + sofar, total - sofar);
    if (thisround < 0) {
      if (errno == EWOULDBLOCK)
	break;
      perror("Write to socket");
      return(-1);
    }
    sofar += thisround;
  } while (sofar < total);

  data_sent+=sofar;
  if (data_sent>1024*1000) {
    data_sent -= (1024*1000);
    meg_sent += 1;
  }
  return(0);
}





int process_input(struct connection_data *t)
{
  int sofar, thisround, begin, squelch, i, k, flag,x, command;
  char tmp[MAX_INPUT_LENGTH+2], buffer[MAX_INPUT_LENGTH + 60], *p;
  
  sofar = 0;
  flag = 0;
  begin = strlen(t->buf);
  
  /* Read in some stuff */
  do  {
    if ((thisround = read(t->id, t->buf + begin + sofar, 
			  MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0) {
      sofar += thisround;
    } else {
      if (thisround < 0) {
	if (errno != EWOULDBLOCK) {
	  perror("Read1 - ERROR");
	  return(-1);
	} else {
	  break;
	}
      } else {
	slog("EOF encountered on socket read.");
	return(-1);
      }
    }
  } while (!ISNEWL(*(t->buf + begin + sofar - 1)));	
  
  *(t->buf + begin + sofar) = 0;
  
  /* if no newline is contained in input, return without proc'ing */
  for (i = begin; !ISNEWL(*(t->buf + i)); i++)
    if (!*(t->buf + i))
      return(0);
  
  /* input contains 1 or more newlines; process the stuff */
  for (i = 0, k = 0; *(t->buf + i);)	{
    if (!ISNEWL(*(t->buf + i)) && !(flag=(k>=(MAX_INPUT_LENGTH - 2))))
      if (*(t->buf + i) == '\b') {	 /* backspace */
	if (k) { /* more than one char ? */
	  if (*(tmp + --k) == '$')
	    k--;				
	  i++;
	} else {
	  i++;  /* no or just one char.. Skip backsp */
	}
      } else {
	if (isascii(*(t->buf + i)) && isprint(*(t->buf + i))) {
	  /* 
	    trans char, double for '$' (printf)	
	    */
	  if ((*(tmp + k) = *(t->buf + i)) == '$')
	    *(tmp + ++k) = '$';
	  k++;
	  i++;
	} else {
	  i++;
	}
      } else 	{
	*(tmp + k) = 0;
	if((*tmp == '!') && (t->connected == CON_PLYNG) && !t->showstr_point &&
            !t->str)
	  strcpy(tmp,t->last_input);
	else
	  strcpy(t->last_input,tmp);

      if(t->character && (t->connected == CON_PLYNG)){
	if(t->str || t->showstr_point){
	  write_to_pq(tmp, &t->input, 0, -2);
        }
	else{
          p = tmp;
	  command = command_interpreter(t->character, &p);
	  write_to_pq(p, &t->input, (!IS_NPC(t->character) ?
	    IS_SET(t->character->specials.act, PLR_PRIORITY) : 0), command);
        }
      }
      else{
	write_to_q(tmp, &t->input, 0);
      }
	for (x=0; x<2; x++)
	  if (t->snooped_by[x]) {
	    SEND_TO_Q("% ",t->snooped_by[x]);
	    SEND_TO_Q(tmp,t->snooped_by[x]);
	    SEND_TO_Q("\n",t->snooped_by[x]);
	  }
	
	if (flag) {
	  sprintf(buffer, 
		  "Line too long. Truncated to:\n%s\n", tmp);
	  if (write_to_id(t->id, buffer,t->term) < 0)
	    return(-1);
	  
	  /* skip the rest of the line */
	  for (; !ISNEWL(*(t->buf + i)); i++);
	}
	
	/* find end of entry */
	for (; ISNEWL(*(t->buf + i)); i++);
	
	/* squelch the entry from the buffer */
	for (squelch = 0;; squelch++)
	  if ((*(t->buf + squelch) = 
	       *(t->buf + i + squelch)) == '\0')
	    break;
	k = 0;
	i = 0;
      }
  }
  return(1);
}




void close_sockets(int s)
{
	struct char_data *ch;

	log("Closing all sockets.");

	for (ch=character_list; ch; ch=ch->next)
		if (!IS_NPC(ch)) save_char(ch,NOWHERE);
	while (connection_list)
		close_socket(connection_list);

	close(s);
}





void close_socket(struct connection_data *d)
{
  struct connection_data *tmp;
  char buf[100];
  int i;

  
  if (!d) { vlog(LOG_URGENT,"Close socket null"); return;}
  
  if (d->character && d->character->funct)
    d->character->funct(d->character);
  /* Forget snooping */
  if (d->snooping)
    for (i=0; i<MAX_SNOOP; i++)
	if (d->snooping->snooped_by[i]==d)
		d->snooping->snooped_by[i]=NULL;

  for (i=0; i<MAX_SNOOP; i++)
    if (d->snooped_by[i]) {
	SEND_TO_Q("Your victim has left the game.\n",d->snooped_by[i]);
	d->snooped_by[i]->snooping=NULL;
    }
  
  if (d->character)
    if (IS_CONN(d)) {
      save_char(d->character,NOWHERE);
      act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
      vlog(LOG_CONNECTION,"Closing link to: %s.",
		ck_null(GET_NAME(d->character)));
      if (IS_NPC(d->character)) {
	if (d->character->desc)
	  d->character->orig = d->character->desc->original;
      }
      d->character->desc = 0;
    } else {
      sprintf(buf, "Losing player: %s.", ck_null(GET_NAME(d->character)));
      if (d->character && GetMaxLevel(d->character)<IMPLEMENTOR)
		vlog(LOG_CONNECTION,buf);
      else log2(buf);
      if (d->character->needs_saved) {
        d->character->player.time.played +=
		 (time(0)-d->character->player.time.logon);
	d->character->player.time.logon=time(0); /* used if they heal */
	save_char(d->character,WAS_IN_ROOM(d));
      }
      free_char(d->character);
  }
  else vlog(LOG_CONNECTION,"Losing connection without char.");
  flush_queues(d);

  errno=0;

  alarm(5);
  close(d->id);
  alarm(0);

  if (d->id == maxdesc)
    --maxdesc;
  
  if (next_to_process == d)		/* to avoid crashing the process loop */
    next_to_process = next_to_process->next;   
  
  if (d == connection_list) /* this is the head of the list */
    connection_list = connection_list->next;
  else  /* This is somewhere inside the list */
    {
      /* Locate the previous element */
      for (tmp = connection_list; (tmp->next != d) && tmp; 
	   tmp = tmp->next);
      
      tmp->next = d->next;
    }
  if (d->showstr_head)
    FREE(d->showstr_head);
  FREE(d);
}





void nonblock(int s)
{
	if (fcntl(s, F_SETFL, FNDELAY) == -1)
	{
		perror("Noblock");
	}
	if (fcntl(s, F_SETFL, O_NDELAY) == -1)
	{
		perror("Noblock1");
	}
}




/* ****************************************************************
*	Public routines for system-to-player-communication	  *
**************************************************************** */



void send_to_char(const char *messg, struct char_data *ch)
{
  if (ch) {
    if (ch->desc && messg && !IS_SET(ch->specials.act,PLR_VEG))
       	write_to_q(messg, &ch->desc->output,1);
  }
}


void save_all()
{
	struct connection_data *i;

       	for (i = connection_list; i; i = i->next)
	  if (i->character) {
	    remove_junk(i->character,0);
       	    save_char(i->character,NOWHERE);
	  }
}

void send_to_all(char *messg)
{
	struct connection_data *i;

	if (messg)
	  for (i = connection_list; i; i = i->next)
	    if (!i->connected)
		write_to_q(messg, &i->output,0);
}


void send_to_outdoor(char *messg)
{
	struct connection_data *i;

	if (messg)
	  for (i = connection_list; i; i = i->next)
	    if (!i->connected && i->character &&
			!IS_SET(i->character->specials.act,PLR_VEG))
		if (OUTSIDE(i->character))
		  write_to_q(messg, &i->output,0);
}


void send_to_desert(char *messg)
{
  struct connection_data *i;
  struct room_data *rp;

  if (messg) {
    for (i = connection_list; i; i = i->next) {
      if (!i->connected && i->character && !IS_SET(i->character->specials.act,PLR_VEG)) {
	if (OUTSIDE(i->character)) {
	  if ((rp = real_roomp(i->character->in_room))!=NULL) {
	    if (rp->sector_type == SECT_DESERT) {
	      write_to_q(messg, &i->output,0);
	    }
	  }
	}
      }
    }
  }
 }

void send_to_out_other(char *messg)
 {
  struct connection_data *i;
  struct room_data *rp;

  if (messg) {
    for (i = connection_list; i; i = i->next) {
      if (!i->connected && i->character && !IS_SET(i->character->specials.act,PLR_VEG)) {
	if (OUTSIDE(i->character)) {
	  if ((rp = real_roomp(i->character->in_room))!=NULL) {
	    if (rp->sector_type != SECT_DESERT && rp->sector_type != SECT_ARCTIC) {
	      write_to_q(messg, &i->output,0);
	    }
	  }
	}
      }
    }
  }
 }


void send_to_arctic(char *messg)
 {
  struct connection_data *i;
  struct room_data *rp;

  if (messg) {
    for (i = connection_list; i; i = i->next) {
      if (!i->connected && i->character && !IS_SET(i->character->specials.act,PLR_VEG)) {
	if (OUTSIDE(i->character)) {
	  if ((rp = real_roomp(i->character->in_room))!=NULL) {
	    if (rp->sector_type == SECT_ARCTIC) {
	      write_to_q(messg, &i->output,0);
	    }
	  }
	}
      }
    }
  }
 }

void send_to_except(char *messg, struct char_data *ch)
{
	struct connection_data *i;

	if (messg)
	  for (i = connection_list; i; i = i->next)
	    if (ch->desc != i && !i->connected)
		write_to_q(messg, &i->output,1);
}



void send_to_room(char *messg, int room)
{
	struct char_data *i;

	if (messg)
	  for (i = real_roomp(room)->people; i; i = i->next_in_room)
	    if (i->desc && !IS_SET(i->specials.act,PLR_VEG))
		write_to_q(messg, &i->desc->output,1);
}




void send_to_room_except(char *messg, int room, struct char_data *ch)
{
	struct char_data *i;

	if (messg)
	  for (i = real_roomp(room)->people; i; i = i->next_in_room)
	    if (i != ch && i->desc && !IS_SET(i->specials.act,
				PLR_VEG))
		write_to_q(messg, &i->desc->output,1);
}

void send_to_room_except_two (char *messg, int room, struct char_data *ch1, struct char_data *ch2)
{
	struct char_data *i;

	if (messg)
	  for (i = real_roomp(room)->people; i; i = i->next_in_room)
		  if (i != ch1 && i != ch2 && i->desc)
			  write_to_q(messg, &i->desc->output,1);
}



/* higher-level communication */


void act(char *str, int hide_invisible, struct char_data *ch,struct obj_data *obj, void *vict_obj, int type)
{
	register char *strp, *point, *i=NULL;
	struct char_data *to;
	char buf[MAX_STRING_LENGTH];
	char work[MAX_STRING_LENGTH];

	if (!str)
		return;
	if (!*str)
		return;

	if (type == TO_VICT)
		to = (struct char_data *) vict_obj;
	else if (type == TO_CHAR)
		to = ch;
        else if (!real_roomp(ch->in_room)) {
		vlog(LOG_URGENT,"act:%s[%d]",str,ch->in_room);
		return;
	} else to = real_roomp(ch->in_room)->people;

	for (; to; to = to->next_in_room)	{
       	  if (to->desc && ((to != ch) || (type == TO_CHAR)) &&  
	       	(CAN_SEE(to, ch) || !hide_invisible) && AWAKE(to) &&
	       	!((type == TO_NOTVICT)&&(to==(struct char_data *) vict_obj))){
	    for (strp = str, point = buf;;)
	      if (*strp == '$') {
		switch (*(++strp)) {
		  case 'c': i = work; work[0]=3; work[1]= *(++strp); work[2]=0;
			    break;
		  case 'n': i = PERS(ch, to); break;
		  case 'N': i = PERS((struct char_data *) vict_obj, to); break;
		  case 'm': i = HMHR(ch); break;
		  case 'M': i = HMHR((struct char_data *) vict_obj); break;
		  case 's': i = HSHR(ch); break;
		  case 'S': i = HSHR((struct char_data *) vict_obj); break;
		  case 'e': i = HSSH(ch); break;
		  case 'E': i = HSSH((struct char_data *) vict_obj); break;
		  case 'o': i = OBJN(obj, to); break;
		  case 'O': i = OBJN((struct obj_data *) vict_obj, to); break;
		  case 'p': i = OBJS(obj, to); break;
		  case 'P': i = OBJS((struct obj_data *) vict_obj, to); break;
		  case 'a': i = SANA(obj); break;
		  case 'A': i = SANA((struct obj_data *) vict_obj); break;
		  case 'T': i = (char *) vict_obj; break;
		  case 'F': i = fname((char *) vict_obj); break;
		  case '$': i = "$"; break;
		  default:
		    vlog(LOG_DEBUG,"Illegal $-code (%c):'%s'",*(strp-1),str);
		    break;
		}

		while ((*point = *(i++)))
		  ++point;

		++strp;

	      }	else if (!(*(point++) = *(strp++)))
		break;

	      *(--point) = '\n';
	      *(++point) = '\0';

	      if (!IS_SET(to->specials.act,PLR_VEG))
		write_to_q(CAP(buf), &to->desc->output,1);
	    }
	    if ((type == TO_VICT) || (type == TO_CHAR))
		    return;
	}
}

#define PERCENT(ch) ((int)(GET_HIT(ch)*100/GET_MAX_HIT(ch)))

void make_prompt(struct char_data *ch,char *b)
{
  char buf[MAX_STRING_LENGTH];
  char *p;
  int i;

  if (!ch->prompt || !*ch->prompt) {
	strcpy(b,"0> ");
	return;
  }
  strcpy(buf,"0");
  i=2;
  p=ch->prompt;
  while (*p) {
    if (*p!='%' && *p!='\\') {
      buf[i++]= *p;
        buf[i]='\0';
      p++;
    } else if (*p=='\\') {
	p++;
	switch (*p) {
	  case '\0': break;
	  case 'n' : strcat(buf,"\n"); break;
	  case 'r' : strcat(buf,""); break;
	  case '\\' : strcat(buf,"\\"); break;
	  case 'e' : strcat(buf,"\x1b"); break;
	  default:
		buf[i]='?'; buf[i+1]='\0';
        }
        if (*p) {
	  p++;
	  i=strlen(buf);
        }
    } else {
      p++;
      switch (*p) {
	case '\0': break;
	case 'h': sprintf(buf+i,"%d",GET_HIT(ch)); break;
	case 'H': sprintf(buf+i,"%d",GET_MAX_HIT(ch)); break;
	case 'r': if (IS_IMMORTAL(ch))
			sprintf(buf+i,"%ld",ch->in_room);
		  break;
	case 'm': sprintf(buf+i,"%d",GET_MANA(ch)); break;
	case 'M': sprintf(buf+i,"%d",GET_MAX_MANA(ch)); break;
	case 'p': sprintf(buf+i,"%d",GET_POWER(ch)); break;
	case 'P': sprintf(buf+i,"%d",GET_MAX_POWER(ch)); break;
	case 'v': sprintf(buf+i,"%d",GET_MOVE(ch)); break;
	case 'V': sprintf(buf+i,"%d",GET_MAX_MOVE(ch)); break;
	case 'o': if (ch->specials.fighting && GET_MAX_HIT (ch) >= 0)
		    strcat(buf,CONDITION(PERCENT(ch->specials.fighting)));
		  else sprintf(buf+i,"0");
		  break;
	case 'O': if (ch->specials.fighting)
			strcat(buf,fname(GET_NAME(ch->specials.fighting)));
		  else strcat(buf,"None");
		  break;
	case 'i': if (IS_AFFECTED(ch,AFF_INVISIBLE)) strcat(buf,"*");
		  break;
	case 'I': if (IS_IMMORTAL(ch))
			sprintf(buf+i,"%d",ch->specials.invis_level);
		  break;
	case 'l': if (ch->master && IS_AFFECTED(ch,AFF_GROUP))
			sprintf(buf+i,"%d",PERCENT(ch->master));
		  else strcat(buf,"0");
		  break;
	case 'L': if (ch->master) strcat(buf,GET_NAME(ch->master));
		  else strcat(buf,"None");
		  break;
	case 'x': if (!IS_IMMORTAL(ch))
			sprintf(buf+i,"%ld",GET_EXP(ch)); break;
	case 'X': if (!IS_IMMORTAL(ch))
		    sprintf(buf+i,"%ld",
		      EXP_NEEDED(GetMaxLevel(ch)+1,GET_CLASS(ch))-GET_EXP(ch));
		  break;
	case 'g': sprintf(buf+i,"%ld",GET_GOLD(ch)); break;
	case 'G': sprintf(buf+i,"%ld",GET_BANK(ch)); break;
	case '%': buf[i]='%'; buf[i+1]='\0'; break;
	default:
		buf[i]='?'; buf[i+1]='\0';
      }
      if (*p) {
	p++;
	i=strlen(buf);
      }
    }
  }
  buf[MAX_STRING_LENGTH-1]='\0'; /* just in case */
  strcpy(b,buf);
}

void do_open_remote(struct char_data *ch, char *arg, int cmd)
{
  struct sockaddr_in addr;
  struct hostent *hp;


  for (; isspace(*arg); arg++) ;
  if (!*arg) {
    if (rcon != -1) {
      close(rcon);
      rcon = -1;
      rmud = init_socket(REMOTE_PORT);
      sendf(ch,"Connection closed.\n");
      return;
    }
    sendf(ch,"Usage: remote machine\n");
    return;
  }
  if (rcon != -1) {
    sendf(ch,"Channel should be open.\n");
    return;
  }

  bzero((char *)&addr, sizeof(addr));
  addr.sin_port = htons(REMOTE_PORT);

  if (!isdigit(*arg)) {
    if (!(hp = gethostbyname(arg))) {
      sendf(ch,"Unknown host: %s\n",arg);
      return;
    } 
    bcopy(hp->h_addr, (char *)&addr.sin_addr, hp->h_length);
    addr.sin_family = hp->h_addrtype;
  } else {
    addr.sin_addr.s_addr=inet_addr(arg);
    addr.sin_family = AF_INET;
  }

  if ((rcon = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    sendf(ch,"Couldn't connect.\n");
    rcon = -1;
    return;
  }

  if (connect(rcon, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    sendf(ch,"Couldn't connect.\n");
    rcon = -1;
    perror("connect");
    return;
  }

  close(rmud);
  rmud = -1;
  sendf(ch,"Connection successful.\n");
  if (rcon>maxdesc) maxdesc = rcon+1;
}
