/**************************************************************************
*  file: db.c , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars, booting world, resetting etc.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
/*#include <bsd/unistd.h>*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>


#include <fcns.h>
#include <ticks.h>
#include <externs.h>
#include <utils.h>
#include <db.h>
#include <comm.h>
#include <handler.h>
#include <limits.h>
#include <race.h>
#include <hash.h>
#include <rooms.h>
extern int ConvertOnly;



#define ZCAT "/usr/ucb/zcat"
#define EXT "Z"
#define NEW_ZONE_SYSTEM

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

  extern int db_boot;
int top_of_world = 0;                 /* ref to the top element of world */
struct hash_header	room_db;
struct hash_header	mob_db;
struct hash_header	obj_db;
struct obj_data  *object_list = 0;    /* the global linked list of obj's */
struct char_data *character_list = 0; /* global l-list of chars          */

struct zone_data *zone_table;         /* table of reset data             */
int top_of_zone_table = 0;
struct message_list fight_messages[MAX_MESSAGES]; /* fighting messages   */

int num_players = 0;

extern DEBUG;
extern struct connection_data *connection_list;


char credits[MAX_STRING_LENGTH];      /* the Credits List                */
char news[MAX_STRING_LENGTH];	      /* the news                        */
char oldnews[MAX_STRING_LENGTH];      /* the news                        */
char donations[MAX_STRING_LENGTH];    /* the info on donations           */
char motd[MAX_STRING_LENGTH];         /* the messages of today           */
time_t motd_time;
char help[MAX_STRING_LENGTH];         /* the main help page              */
char info[MAX_STRING_LENGTH];         /* the info text                   */
char wizlist[MAX_STRING_LENGTH];      /* the wizlist                     */
char classhelp[MAX_STRING_LENGTH];	/* the pre-roll class help */
char welcome[MAX_STRING_LENGTH];
char toomany[MAX_STRING_LENGTH];

FILE *obj_f,                          /* obj prototypes                  */
     *help_fl;                        /* file for help texts (HELP <kwd>)*/

struct help_index_element *help_index = 0;

int top_of_objt = 0;                  /* top of object index table       */
int top_of_helpt;                     /* top of help index table         */

struct time_info_data time_info;	/* the infomation about the time   */
struct weather_data weather_info;	/* the infomation about the weather */






/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */


/* body of the booting system */
void boot_db(void)
{
	int i;
        struct stat buf;
	extern int no_specials;

	log("Boot db -- BEGIN.");
	init_terms();
	boot_areas();

	if (!ConvertOnly) {
	  build_command_tree();
          log("booting forms");
          boot_forms();
	  log("booting classes");
	  boot_classes();
	  log("Resetting the game time:");
	  reset_time();
	  open_player_log();
	  log("Loading fight messages.");
	  load_messages();

	  log("Loading social messages.");
	  boot_social_messages();

	  log("Loading pose messages.");
	  boot_pose_messages();

	  log("Reading newsfile, credits, help-page, info and motd.");
	  file_to_string(WELCOME_FILE, welcome);
	  file_to_string(NEWS_FILE, news);
	  file_to_string(OLD_NEWS_FILE, oldnews);
          file_to_string(DONATIONS_FILE, donations);
	  file_to_string(CREDITS_FILE, credits);
	  file_to_string(MOTD_FILE, motd);
	  file_to_string(HELP_PAGE_FILE, help);
	  file_to_string(INFO_FILE, info);
	  file_to_string(WIZLIST_FILE, wizlist);
	  file_to_string(CLASS_INFO_FILE, classhelp);
          file_to_string(TOOMANY_FILE, toomany);

          stat(MOTD_FILE, &buf);
          motd_time = buf.st_mtime;

	  log("Opening help files.");


	  if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
	    log("   Could not open help file.");
	  else 
	    help_index = build_help_index(help_fl, &top_of_helpt);

	  log("Loading zone table.");
	  boot_zones();

	  log("Loading rooms.");
	  boot_no_rent();
#if 1
	  boot_world();
#else
	  old_boot_world();
	  save_world();
#endif
	  log("Mob file.");
	  boot_mob();
	}

	boot_obj();
	log("Renumbering zone table.");
	renum_zone_table();

	log("Scanning player files.");
	boot_players();
	fflush(stdout);
	if (ConvertOnly) exit(0);


	log("Assigning function pointers:");
	if (!no_specials)	{
		log("   Mobiles.");
		assign_mobiles();
		log("   Objects.");
		assign_objects();
		log("   Room.");
		assign_rooms();
	}

	fprintf(stderr,"------> Performing Boot Time Reset <-----\n");
	for (i = 0; i <= top_of_zone_table; i++)	{
	  char	*s;
	  int	d,e;
	  s = zone_table[i].name;
	  d = (i ? (zone_table[i - 1].top + 1) : 0);
	  e = zone_table[i].top;
	  fprintf(stderr, "%8d-%8d %s\n", d, e,s);
	  reset_zone(i);
	}
	fprintf(stderr,"------> Boot Time Reset Complete <-----\n");

	reset_q.head = reset_q.tail = 0;

	log("Boot db -- DONE.");
}


/* reset the time in the game from file */
void reset_time(void)
{
	char buf[MAX_STRING_LENGTH];
        extern unsigned char moontype;
	long beginning_of_time = 650336715;



	struct time_info_data mud_time_passed(time_t t2, time_t t1);

	time_info = mud_time_passed(time(0), beginning_of_time);

        moontype = time_info.day;

	if (time_info.hours <5) weather_info.sunlight=SUN_DARK;
	else if (time_info.hours<7) weather_info.sunlight = SUN_RISE;
	else if (time_info.hours<19) weather_info.sunlight = SUN_LIGHT;
	else if (time_info.hours<21) weather_info.sunlight = SUN_SET;
	else weather_info.sunlight = SUN_DARK;

	sprintf(buf,"   Current Gametime: %dH %dD %dM %dY.",
	        time_info.hours, time_info.day,
	        time_info.month, time_info.year);
	log(buf);

	weather_info.pressure = 960;
	if ((time_info.month>=7)&&(time_info.month<=12))
		weather_info.pressure += dice(1,50);
	else
		weather_info.pressure += dice(1,80);

	weather_info.change = 0;

	if (weather_info.pressure<=980) {
           if ((time_info.month>=3) && (time_info.month<=14))
		weather_info.sky = SKY_LIGHTNING;
           else
                weather_info.sky = SKY_LIGHTNING;
	} else if (weather_info.pressure<=1000) {
           if ((time_info.month>=3) && (time_info.month<=14))
		weather_info.sky = SKY_RAINING;
           else
		weather_info.sky = SKY_RAINING;
	} else if (weather_info.pressure<=1020) {
		weather_info.sky = SKY_CLOUDY;
	} else {
            weather_info.sky = SKY_CLOUDLESS;
	}
}


void boot_players(void)
{
#define NLEVELS (IMPLEMENTOR-IMMORTAL+1)
  static char *headers[] = {
	"Guest Immortals",
	"Minor Builders",
	"Builders",
	"Greater Builders",
	"Demigods",
	"Lesser Gods",
	"Gods",
	"Greater Gods",
	"Assistants to the Implementor",
	"Implementors"
  };

  FILE *f;
  FILE *imm;
  char s[100];
  char list[NLEVELS][MAX_STRING_LENGTH];
  int length[NLEVELS];
  int i;
  int now,time_frame;
  extern int purge_days;
  struct char_data *ch;
  extern int db_boot;

  now=time(0);
  if (purge_days < (365 * 32)) {
	  nlog("Retro assist - increased purge interval to %d years", 32);
	  purge_days = (365 * 32);	//2014 - Increased this to 32 years to avoid purging really old characters
  }
  time_frame = purge_days * 60 * 60 * 24;

  nlog("Purge interval is %d days",purge_days);

  for (i=0; i<NLEVELS; i++) {
	list[i][0]='\0';
	length[i]=0;
  }

  if (!(f=fopen(PLAYER_FILE,"r"))) {
    log("No player files.");
    return;
  }
  imm=fopen("IMMORTALS","w+");
  db_boot=1;
  while (fgets(s,100,f)) {
    CREATE(ch,struct char_data,1);
    clear_char(ch);
    s[strlen(s)-1]='\0';
    if (new_load_char(ch,s)>0) {
      num_players++;
      if (num_players % 500 ==0) nlog("  %d processed",num_players);

      if (GET_GOLD(ch)+GET_BANK(ch)>5000000)
	nlog("  %s has %d gold",GET_NAME(ch),GET_GOLD(ch)+GET_BANK(ch));

      if (IS_IMMORTAL(ch)) {
        int i;

        fprintf(imm,"%-20s %4d %7d %7d\n",ch->player.name,GetMaxLevel(ch),
                ch->build_lo, ch->build_hi);

	i=GetMaxLevel(ch)-IMMORTAL;
	if (strlen(GET_NAME(ch))+length[i]>75) {
	  strcat(list[i],"\n");
	  length[i]=0;
	}
	strcat(list[i],GET_NAME(ch));
	strcat(list[i],", ");
	length[i]+=(strlen(GET_NAME(ch))+2);
      } else {
	if (ch->player.time.logon - now > time_frame || GetMaxLevel(ch)<1) {
	  nlog("Purging %s",GET_NAME(ch));
	  delete_entry(ch,GET_NAME(ch),-1);
	}
      }
	
      special_boot_check(ch);
      free_char(ch);
    }
  }
  fclose(f);
  fclose(imm);
  db_boot=0;
  strcpy(wizlist,"");
  for (i=NLEVELS-1; i>=0; i--) 
    if (list[i][0]!='\0') {
	strcat(wizlist,headers[i]);
	strcat(wizlist,"\n");
	list[i][strlen(list[i])-2]='\0';
	strcat(wizlist,list[i]);
	strcat(wizlist,"\n\n");
    }
  nlog("  There are %d players",num_players);
}

void cleanout_room(struct room_data *rp)
{
  int	i;
  struct extra_descr_data *exptr, *nptr;

  FREE(rp->name);
  FREE(rp->description);
  for (i=0; i<6; i++)
    if (rp->dir_option[i]) {
      FREE(rp->dir_option[i]->general_description);
      FREE(rp->dir_option[i]->keyword);
      FREE(rp->dir_option[i]);
    }

  for (exptr=rp->ex_description; exptr; exptr = nptr) {
    nptr = exptr->next;
    FREE(exptr->keyword);
    FREE(exptr->description);
    FREE(exptr);
  }
  rp->ex_description=NULL;
}

void completely_cleanout_room(struct room_data *rp)
{
  struct char_data	*ch;
  struct obj_data	*obj;
  
  while (rp->people) {
    ch = rp->people;
    act("The hand of god sweeps across the land and you are swept into the Void.", FALSE, NULL, NULL, NULL, TO_VICT);
    char_from_room(ch);
    char_to_room(ch, 0);	/* send character to the void */
  }
  
  while (rp->contents) {
    obj = rp->contents;
    obj_from_room(obj);
    obj_to_room(obj, 0);	/* send item to the void */
  }
  
  cleanout_room(rp);
}
    
void load_one_room(FILE *fl, struct room_data *rp)
{
  char chk[50];

  int	tmp;
  int	ltmp;
  struct extra_descr_data *new_descr;
  
  if (!rp) {
    vlog(LOG_URGENT,"load_one_room called with null pointer!");
    perror("load_one_room");
    return;
  }
  rp->name = fread_string(fl);
  rp->description = fread_string(fl);
  
  if (top_of_zone_table >= 0) {
    int	zone;
    fscanf(fl, " %*d ");
    
    /* OBS: Assumes ordering of input rooms */
    
    for (zone=0;
	 rp->number > zone_table[zone].top && zone<=top_of_zone_table;
	 zone++)
      ;
    if (zone > top_of_zone_table) {
      fprintf(stderr, "Room %d is outside of any zone.\n", rp->number);
      exit(0);
    }
    rp->zone = zone;
  }
  fscanf(fl, " %ld ", &ltmp); rp->room_flags = ltmp;
  fscanf(fl, " %d ", &tmp); rp->sector_type = tmp;
  
  if (tmp == -1) { 
    fscanf(fl, " %d", &tmp); rp->tele_time = tmp;
    fscanf(fl, " %d", &tmp); rp->tele_targ = tmp;
    fscanf(fl, " %d", &tmp); rp->tele_look = tmp;
    fscanf(fl, " %d", &tmp); rp->sector_type = tmp;
  } else {
    rp->tele_time = 0;
    rp->tele_targ = 0;
    rp->tele_look = 0;
  }
  
  if (tmp == 7)  { /* river */
    /* read direction and rate of flow */
    fscanf(fl, " %d ", &tmp);
    rp->river_speed = tmp;
    fscanf(fl, " %d ", &tmp);
    rp->river_dir = tmp;
  } 

  
  rp->funct = 0;
  rp->light = 0; /* Zero light sources */
  
  for (tmp = 0; tmp <= 5; tmp++)
    rp->dir_option[tmp] = 0;
  
  rp->ex_description = 0;
  
  while (1==fscanf(fl, " %s \n", chk)) {
    switch (*chk) {
    case 'A':  /* Room affect */
      break;
    case 'D':
      setup_dir(fl, rp->number, atoi(chk + 1));
      break;
    case 'E': /* extra description field */
      CREATE(new_descr,struct extra_descr_data,1);
      new_descr->keyword = fread_string(fl);
      new_descr->description = fread_string(fl);
      new_descr->next = rp->ex_description;
      rp->ex_description = new_descr;
      new_descr=NULL;
      break;
    case 'S':	/* end of current room */
      return;
    default:
      vlog(LOG_URGENT,"unknown auxiliary code `%s' in room load of #%d",
	      chk, rp->number);
      break;
    }
  }
}

/* load the rooms */
void boot_world(void)
{
  FILE *fp;
  short int id;
  
  init_hash_table(&room_db, sizeof(struct room_data), 8192);
  character_list = 0;
  object_list = 0;
  
  fp = fopen(NEW_WORLD_FILE, "rb");
  if (!fp) {
    perror("fopen");
    log("boot_world: could not open world file.");
    exit(0);
  }
  
  fread(&id,sizeof(id),1,fp);
  while (read_one_room(fp)) ;
  fclose(fp);
}

  

/* load the rooms */
void old_boot_world(void)
{
  FILE *fl;
  slong virtual_nr;
  struct room_data	*rp;
  
  init_hash_table(&room_db, sizeof(struct room_data), 4192);
  character_list = 0;
  object_list = 0;
  
#ifdef COMPRESSED_WORLD
  sprintf(line, "%s %s.%s", ZCAT, WORLD_FILE, EXT);
  fl = popen(line, "rb"); 
#else
  fl=fopen(WORLD_FILE,"rb");
#endif
  if (!fl) {
    perror("fopen");
    log("boot_world: could not open world file.");
    exit(0);
  }
  
  while (1==fscanf(fl, " #%ld\n", &virtual_nr)) {
    allocate_room(virtual_nr);
    rp = real_roomp(virtual_nr);
    bzero(rp, sizeof(*rp));
    rp->number = virtual_nr;
    load_one_room(fl, rp);
  }
  
  fclose(fl);
}





void allocate_room(int room_number)
{
  struct room_data *rp;
  if (room_number>top_of_world)
    top_of_world = room_number;
  if ((rp=real_roomp(room_number))) {
    cleanout_room(rp);
    return;
  }
  hash_find_or_create(&room_db, room_number);
  rp=real_roomp(room_number);
  bzero(rp,sizeof(struct room_data));
}






/* read direction data */
void setup_dir(FILE *fl, int room, int dir)
{
	int tmp;
	struct room_data	*rp;

	rp = real_roomp(room);
	CREATE(rp->dir_option[dir], 
		struct room_direction_data, 1);

	rp->dir_option[dir]->general_description =
		fread_string(fl);
	rp->dir_option[dir]->keyword = fread_string(fl);

	fscanf(fl, " %d ", &tmp);
	if (tmp == 1) rp->dir_option[dir]->exit_info = DOOR_1;
	else if (tmp == 2) rp->dir_option[dir]->exit_info = DOOR_2;
	else if (tmp == 3) rp->dir_option[dir]->exit_info = DOOR_3;
	else if (tmp == 4) rp->dir_option[dir]->exit_info = DOOR_4;
	else if (tmp == 5) rp->dir_option[dir]->exit_info = DOOR_5;
	else if (tmp == 6) rp->dir_option[dir]->exit_info = DOOR_6;
	else if (tmp == 7) rp->dir_option[dir]->exit_info = DOOR_7;
	else
       	   rp->dir_option[dir]->exit_info = 0;
 
	fscanf(fl, " %d ", &tmp);
	rp->dir_option[dir]->key = tmp;

	fscanf(fl, " %d ", &tmp);
	rp->dir_option[dir]->to_room = tmp;
}


void renum_zone_table(void)
{
  int zone, comm;
  struct reset_com *cmd;
  
  for (zone = 0; zone <= top_of_zone_table; zone++)
    for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++) {
      switch((cmd = zone_table[zone].cmd +comm)->command) {
      case 'M':
	if (!real_mobp(cmd->arg1) || !real_roomp(cmd->arg3)) {
	  nlog("M: %d %d %d %d",cmd->arg1,cmd->arg2,cmd->arg3,cmd->line);
	  cmd->line = -1;
	}
	break;
      case 'O':
	if (!real_objp(cmd->arg1) || !real_roomp(cmd->arg3)) {
	  nlog("O: %d %d %d %d",cmd->arg1,cmd->arg2,cmd->arg3,cmd->line);
	  cmd->line = -1;
	}
	break;
      case 'G':
	if (!real_objp(cmd->arg1)) {
	  nlog("G: %d %d %d %d",cmd->arg1,cmd->arg2,cmd->arg3,cmd->line);
	  cmd->line = -1;
	}
	break;
      case 'E':
	if (!real_objp(cmd->arg1)) {
	  nlog("E: %d %d %d %d",cmd->arg1,cmd->arg2,cmd->arg3,cmd->line);
	  cmd->line = -1;
	}
	break;
      case 'P':
	if (!real_objp(cmd->arg1) || !real_objp(cmd->arg3)) {
	  nlog("P: %d %d %d %d",cmd->arg1,cmd->arg2,cmd->arg3,cmd->line);
	  cmd->line = -1;
	}
	break;					
      case 'D':
	if (!real_roomp(cmd->arg1)) {
	  nlog("D: %d %d %d %d",cmd->arg1,cmd->arg2,cmd->arg3,cmd->line);
	  cmd->line = -1;
	}
	break;
      }
    }
}


#define MAX_ZONES 125

/* load the zone table and command tables */
void boot_zones(void)
{
	FILE *fl;
	int line = 1;
	int zon = 0, cmd_no = 0, expand, tmp;
	char *check, buf[81];

	if (!(fl = fopen(ZONE_FILE, "r")))	{
		perror("boot_zones");
		exit(0);
	}

	CREATE(zone_table, struct zone_data, MAX_ZONES);

	for (;;)	{
		fscanf(fl, " #%*d\n");
		line += 2;
		check = fread_string(fl);

		if (*check == '$')
			break;		/* end of file */
		if (zon >= MAX_ZONES) {
			fprintf(stderr, "Too many zones.  Edit db.c and recompile.\nSearch for MAX_ZONES\n");
			exit(1);
		}

		zone_table[zon].name = check;
		fscanf(fl, " %ld ", &zone_table[zon].top);
		fscanf(fl, " %d ", &zone_table[zon].lifespan);
		fscanf(fl, " %d ", &zone_table[zon].reset_mode);
		line++;

		/* read the command table */

		cmd_no = 0;

		for (expand = 1;;)     	{
			if (expand)
			if (!cmd_no)
				CREATE(zone_table[zon].cmd, struct reset_com, 1);
			else
			if (!(zone_table[zon].cmd =
				(struct reset_com *) realloc(zone_table[zon].cmd,
				(cmd_no + 1) * sizeof(struct reset_com))))
			{
				perror("reset command load");
				exit(0);
			}

			expand = 1;

			line++;
			zone_table[zon].cmd[cmd_no].line = line;
			fscanf(fl, " "); /* skip blanks */
			fscanf(fl, "%c",
				&zone_table[zon].cmd[cmd_no].command);

			if (zone_table[zon].cmd[cmd_no].command == 'S')
				break;

			if (zone_table[zon].cmd[cmd_no].command == '*')	{
				expand = 0;
				fgets(buf, 80, fl); /* skip command */
				continue;
			}

			fscanf(fl, " %d %ld %ld",
				&tmp,
				&zone_table[zon].cmd[cmd_no].arg1,
				&zone_table[zon].cmd[cmd_no].arg2);

			zone_table[zon].cmd[cmd_no].if_flag = tmp;

			if (zone_table[zon].cmd[cmd_no].command == 'M' ||
				zone_table[zon].cmd[cmd_no].command == 'O' ||
				zone_table[zon].cmd[cmd_no].command == 'E' ||
				zone_table[zon].cmd[cmd_no].command == 'P' ||
				zone_table[zon].cmd[cmd_no].command == 'D')
				fscanf(fl, " %ld", &zone_table[zon].cmd[cmd_no].arg3);

			fgets(buf, 80, fl);	/* read comment */

			cmd_no++;
		}
		zon++;
	}
	zone_table = REALLOC(zone_table, zon*sizeof(struct zone_data), struct zone_data);
	top_of_zone_table = --zon;
	FREE(check);
	fclose(fl);
}


/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*********************************************************************** */

#define _Read(v) fread(&v,sizeof(v),1,fp)

#define Read(v) fread(&v,sizeof(v),1,fp); break

#define ReadSt(v,type)  fread(&v,sizeof(type),1,fp); break

#define ReadSS(v)       fread(&temp,sizeof(temp),1,fp); fread(&v,temp,1,fp);\
                        break

#define _ReadSD(v)      fread(&t1,sizeof(t1),1,fp); v=MALLOC(t1+1,char);\
                        fread(v,t1,1,fp)

#define ReadSD(v)       _ReadSD(v); break

#define _ReadAD(v,n,type) _Read(temp); n=temp / sizeof(type);\
                        v=MALLOC(temp,type); fread(v,temp,1,fp)

#define ReadAD(v,n,type) _ReadAD(v,n,type); break

#define ReadAS(v,n,type) _Read(temp); n=temp / sizeof(type);\
                        fread(v,temp,1,fp); break

#define FIX(s) {int i; i=strlen(s); if (s[i-1]!='\n') {s[i]='\n';  s[i+1]='\0'; } }

/* returns 0 if no room could be loaded */
int read_one_room(FILE *fp)
{
  short int id,t1;
  struct room_data *rp;
  int virtual;

  _Read(virtual);
  allocate_room(virtual);
  rp=real_roomp(virtual);
  rp->number=virtual;

  if (top_of_zone_table >= 0) {
    int zone;
    for (zone=0; zone<=top_of_zone_table; zone++)
	if (virtual <= zone_table[zone].top) break;
    if (zone > top_of_zone_table) {
      fprintf(stderr, "Room %d is outside of any zone.\n", rp->number);
      exit(0);
    }
    rp->zone = zone;
  }


  while (_Read(id)>0) {
    switch(id) {
	case 255: return(1);
	case 1: Read(rp->sector_type);
	case 2: _Read(rp->river_dir);
		Read(rp->river_speed);
	case 3: _Read(rp->tele_time);
		_Read(rp->tele_look);
		Read(rp->tele_targ);
	case 4: ReadSD(rp->name);
	case 5: _ReadSD(rp->description);
		FIX(rp->description);
		break;
	case 6: {
		struct extra_descr_data *ex;
		ex=NULL;
		while (1) {
		  _Read(id);
		  if (id==254) break;
		  if (!ex) {
			CREATE(ex,struct extra_descr_data,1);
			ex->next=rp->ex_description;
			rp->ex_description=ex;
			ex->description=NULL;
			ex->keyword=NULL;
		  }
		  switch (id) {
			case 1: _ReadSD(ex->keyword);
				if (ex->description) ex=NULL;
				break;
			case 2: _ReadSD(ex->description);
				FIX(rp->description);
				if (ex->keyword) ex=NULL;
				break;
		  }
		 }
		} break;
	case 7: {
		  struct room_direction_data *d;
		  u8 dir;
		  d=NULL;
		  while (1) {
		    _Read(id);
		    if (id==254) break;
		    if (id!=1 && !d) {
		      vlog(LOG_URGENT,"Error in exits virtual=%d id %d",virtual,id);
		      return(0);
		    }
		    switch (id) {
		      case 1:
			  CREATE(d,struct room_direction_data,1);
			  d->key= -1;
			  d->general_description=NULL;
			  d->keyword=NULL;
			  d->exit_info=0;
			  _Read(dir);
			  rp->dir_option[dir]=d;
			  Read(d->to_room);
		      case 2: _ReadSD(d->general_description);
			      FIX(d->general_description);
			      break;
		      case 3: ReadSD(d->keyword);
		      case 4: Read(d->exit_info);
		      case 5: Read(d->key);
		    }
		  }
		} break;
	case 9: Read(rp->room_flags);
	default:
		vlog(LOG_URGENT,"Screwup in read_one_room (id=%d)",id);
		return(0);
    }
  }
  return(0);
}

void boot_shp()
{
  FILE *fp;
  short int id,t1;
  int temp;
  shop_data *s;
  int op,cl;

  nlog("Loading shops: %s",SHP_FILE);
  fp=fopen(SHP_FILE,"rb");
  if (!fp) {
    perror(SHP_FILE);
    exit(1);
  }
  shop_index=NULL;
  s=NULL;
  while (_Read(id)>0) {
    switch (id) {
	case 255: {
	  shop_data *new;
	  CREATE(new,shop_data,1);
	  bzero(new,sizeof(shop_data));
	  if (shop_index)
	    s->next=new;
	  else shop_index=new;
	  s=new;
	  _Read(s->index);
	  if (DEBUG) nlog("Booting shop %d",s->index);
	} break;
	case 0: Read(s->keeper);
	case 1: Read(s->in_room);
	case 2: ReadAD(s->type,s->n_type,u16);
	case 3: ReadAD(s->producing,s->n_producing,u32);
	case 6: Read(s->profit_sell);
	case 7: Read(s->profit_buy);
	case 8: _Read(op); _Read(cl);
		s->n_hours++;
		s->open=REALLOC(s->open,s->n_hours*sizeof(int),int);
		s->close=REALLOC(s->close,s->n_hours*sizeof(int),int);
		s->open[s->n_hours-1]=op;
		s->close[s->n_hours-1]=cl;
		break;
	case 10: Read(s->temper1);
	case 11: Read(s->temper2);
	case 12: ReadAD(s->vnum,s->n_vnum,u32);
	case 9: {
	      while (1) {
		_Read(id);
		if (id==254) break;
		switch (id) {
		  case 1: ReadSD(s->no_such_item1);
		  case 2: ReadSD(s->no_such_item2);
		  case 3: ReadSD(s->do_not_buy);
		  case 4: ReadSD(s->missing_cash1);
		  case 5: ReadSD(s->missing_cash2);
		  case 6: ReadSD(s->message_buy);
		  case 7: ReadSD(s->message_sell);
		  default:
		    nlog("Unknown id in shop/messages: %d",id);
		    exit(1);
		}
	      }
	} break;
	default:
		nlog("Unknown id in shop file: %d",id);
		exit(1);
    }
  }
  fclose(fp);
}

void finish_obj(struct obj_data *o)
{
  extern material_t materials[];

  if(!o->material){
    switch(o->obj_flags.type_flag){
      case ITEM_LIGHT: o->material = MAT_WOOD; break;
      case ITEM_SCROLL: o->material = MAT_PAPER; break;
      case ITEM_WAND: o->material = MAT_WOOD; break;
      case ITEM_STAFF: o->material = MAT_WOOD; break;
      case ITEM_WEAPON: o->material = MAT_STEEL; break;
      case ITEM_FIREWEAPON: o->material = MAT_WOOD; break;
      case ITEM_MISSILE: o->material = MAT_WOOD; break;
      case ITEM_TREASURE: o->material = MAT_GEMSTONE; break;
      case ITEM_ARMOR: o->material = MAT_LEATHER; break;
      case ITEM_POTION: o->material = MAT_GLASS; break;
      case ITEM_WORN: o->material = MAT_CLOTH; break;
      case ITEM_CONTAINER: o->material = MAT_CLOTH; break;
      case ITEM_NOTE: o->material = MAT_PAPER; break;
      case ITEM_DRINKCON: o->material = MAT_WOOD; break;
      case ITEM_KEY: o->material = MAT_IRON; break;
      case ITEM_FOOD: o->material = MAT_ORGANIC; break;
      case ITEM_MONEY: o->material = MAT_GOLD; break;
      case ITEM_BOAT: o->material = MAT_WOOD; break;
      case ITEM_BOARD: o->material = MAT_WOOD; break;
      case ITEM_BOOK: o->material = MAT_PAPER; break;
      case ITEM_SPELL_POUCH: o->material = MAT_LEATHER; break;
    }
  }
  if(o->obj_flags.mass == -1){
    o->obj_flags.mass = 454 * o->obj_flags.weight;
  }
  if(o->obj_flags.volume == -1){
    /* fetch volume info from density and mass */
    o->obj_flags.volume = (100 * o->obj_flags.mass) / materials[o->material].density;
  }
  if(IS_WEAPON(o) && !o->obj_flags.value[0]){
    if(real_objp(o->virtual)){
      o->obj_flags.value[0] = real_objp(o->virtual)->obj->obj_flags.value[0];
    }
  }
  if(!o->obj_flags.length && IS_WEAPON(o)){
    /* fetch the length from the default list according to type */
    o->obj_flags.length = weapons[o->obj_flags.value[0]].length;
  }
  if(IS_WEAPON(o) && !o->obj_flags.value[4]){
    o->obj_flags.value[4] = weapons[o->obj_flags.value[0]].speed;
  }
}

void init_obj(struct obj_data *o)
{
  switch(o->obj_flags.type_flag){
    case ITEM_DRINKCON:
      weight_change_object(o, 50 * o->obj_flags.value[1]);
      break;
    case ITEM_SPELL_POUCH:
    case ITEM_CONTAINER:
      /* roughly calculate container space */
      o->obj_flags.value[0] = (o->obj_flags.value[0] -
        (o->obj_flags.mass / 454)) * 100;
      break;
  }
}

void boot_obj()
{
  FILE *fp;
  short int id,t1;
  index_mem *h=0;
  struct obj_data *o = NULL;
  int cnt = 0;

  init_hash_table(&obj_db, sizeof(index_mem), 2000);

  nlog ("Loading objects: %s",  OBJ_FILE);
  fp=fopen(OBJ_FILE,"rb");
  if (!fp) {
    perror("Could not open object file");
    exit(1);
  }
  
  while (_Read(id)>0) {
    switch(id) {
	case 255:
                /* stuff to be performed after reading an object - SLB */
                if(o){
                  finish_obj(o);
                  init_obj(o);
                }
		if (h) {
		}
	        h=NULL;
		CREATE(h,index_mem,1);
		h->mob=NULL;
		h->obj=NULL;
		h->count=0;
		h->boot_count=0;
		h->max_at_boot = 32000;
		h->func=NULL;
		CREATE(h->obj,struct obj_data,1);
		o=h->obj;
		clear_object(o);
                /* set both to -1 so if not read they'll default */
                o->obj_flags.mass = o->obj_flags.volume = -1;
		_Read(o->virtual);
#ifdef MALLOC_DEBUG
		fprintf(stderr,"%d",o->virtual);
#endif
		hash_enter(&obj_db,o->virtual,h);
		break;
#define OBJ o->obj_flags
	case 0: ReadSD(o->short_description);
	case 1: cnt++;
		ReadSD(o->name);
	case 2: 
		/*nlog ("Object(%d): %s", cnt, o->name);*/
		ReadSD(o->description);
	case 3: ReadSD(o->action_description);
	case 7: Read(OBJ.weight);
	case 43: Read(OBJ.cost);
	case 44: Read(OBJ.cost_per_day);
	case 45: _Read(id);
		OBJ.type_flag=id;
		switch (OBJ.type_flag) {
		  case ITEM_TRASH:
		  case ITEM_BOAT:
		  case ITEM_KEY:
		  case ITEM_WORN:
		  case ITEM_BOARD:
		  case ITEM_NOTE:
		  case ITEM_PEN:
		  case ITEM_OTHER:
		  case ITEM_BOOK:
		  case ITEM_FIREWEAPON:
		  case ITEM_MISSILE:
		        Read(id);
		  default: 
			while (1) {
			  _Read(id);
			  if (id==254) break;
			  _Read(OBJ.value[id-1]);
			}
		        _Read(id);
		}
		break;
	case 46: {
		   struct extra_descr_data *ex;
		   ex=NULL;
		   while (1) {
		     _Read(id);
		     if (id==254) break;
		     if (!ex) {
			CREATE(ex,struct extra_descr_data,1);
			ex->next=o->ex_description;
			o->ex_description=ex;
			ex->description=NULL;
			ex->keyword=NULL;
		     }
		     switch (id) {
			case 1: _ReadSD(ex->keyword);
				if (ex->description) ex=NULL;
				break;
			case 2: _ReadSD(ex->description);
				if (ex->keyword) ex=NULL;
				break;
		     }
		   }
		 } break;
	case 47: Read(OBJ.extra_flags);
	case 48: Read(OBJ.wear_flags);
	case 49: {
		   int count=0;
		   while (1) {
			_Read(id);
			if (id==254) break;
			o->affected[count].location=id;
			_Read(o->affected[count].modifier);
			count++;
		   }
		 } break;
	case 50: Read(o->min_level);
	case 51: Read(h->max_at_boot);
        case 52: Read(o->material);
        case 53: Read(OBJ.mass);
        case 54: Read(OBJ.volume);
	default:
		nlog("Unknown id in mob file: %d",id);
		exit(1);
    }
  }
  if(o){
    finish_obj(o);
    init_obj(o);
  }
  nlog ("Object file has %d objects.", cnt);
  fclose(fp);
}


void boot_mob()
{
  FILE *fp;
  short int id,t1;
  int class_set = 0;
  index_mem *h=0;
  struct char_data *m=0;
  struct skill_entry *sk;
  struct spell_entry *sp;

  init_hash_table(&mob_db, sizeof(index_mem), 1500);

  fp = fopen(MOB_FILE, "rb");
  if (!fp) {
    perror("boot_mob");
    exit(1);
  }
  
  while (_Read(id)>0) {
    if (DEBUG>1) fprintf(stderr,"ID:%d\n",id);
    switch(id) {
	case 255:
		if (h) {
		  strcat(m->player.long_descr,"\n");
		  if (m->player.description)
			strcat(m->player.description,"\n");
		  else
			m->player.description=
			mystrdup("You see nothing special.\n");
		  SET_BIT(m->specials.act,ACT_ISNPC);
		  m->abilities=m->tmpabilities;
		  m->points.max_move=m->points.move;
		  m->points.max_mana=m->points.mana;
                  if(!class_set){
                    if(m->specials.magic_use){
                      GET_CLASS(m) = SCHOLAR;
                    }
                    else{
                      GET_CLASS(m) = WARRIOR;
                    }
                  }

                  if(IsHumanoid(m)){
                    for(sk = CLASS(m).skills; sk; sk = sk->next){
                      if((GET_LEVEL(m) >= sk->max_learn) && (find_skill(m, sk->num) < 0)){
                        set_skill(m, sk->num, sk->max_learn);
                      }
                    }
                    for(sp = CLASS(m).spells; sp; sp = sp->next){
                      if((GET_LEVEL(m) >= sp->max_learn) && (find_spell(m, sp->num) < 0)){
                        set_spell(m, sp->num, sp->max_learn);
                      }
                    }
                  }
		}
	        h=NULL;
		CREATE(h,index_mem,1);
		memset(h,0,sizeof(index_mem));
		CREATE(h->mob,struct char_data,1);
		m=h->mob;
		clear_char(m);
                class_set = 0;
		m->tmpabilities.str   = 11;
		m->tmpabilities.intel = 11;
		m->tmpabilities.wis   = 11;
		m->tmpabilities.dex   = 11;
		m->tmpabilities.con   = 11;
		m->player.weight = 175;
		m->player.height = 175;
		m->points.move = 150;
		m->points.mana = 100;
		GET_COND(m,THIRST)= -1;
		GET_COND(m,FULL)= -1;
		GET_COND(m,DRUNK)= -1;
		GET_BODY_AC(m)=100;
		GET_HEAD_AC(m)=100;
		GET_FEET_AC(m)=100;
		GET_ARMS_AC(m)=100;
		GET_LEGS_AC(m)=100;
		_Read(m->virtual);
		if (DEBUG>1)
		  fprintf(stderr,"V:%d",m->virtual);
		hash_enter(&mob_db,m->virtual,h);
		break;
	case 0: ReadSD(m->player.short_descr);
	case 1: ReadSD(m->player.name);
	case 2: ReadSD(m->player.long_descr);
	case 3: ReadSD(m->player.description);
	case 4: _Read(GET_RACE(m));
		if (GET_HEIGHT(m)==175)
			GET_HEIGHT(m)=generate_height(GET_RACE(m));
		if (GET_WEIGHT(m)==175)
			GET_WEIGHT(m)=generate_weight(GET_RACE(m));
		break;
	case 5: Read(GET_MANA(m));
	case 6: Read(GET_HEIGHT(m));
	case 7: Read(GET_WEIGHT(m));
	case 8: Read(m->player.level);
	case 9: Read(GET_ALIGNMENT(m));
	case 10:Read(GET_GOLD(m));
	case 11:Read(GET_HITROLL(m));
	case 12:Read(GET_DAMROLL(m));
	case 13:Read(m->specials.mult_att);
	case 15: break;
	case 44:
	case 14:{
		  int count,i;
		  u16 skill;

		  _Read(count);
		  count/=sizeof(skill);
		  assert(count<100);
		  for (i=0; i<count; i++) {
		    _Read(skill);
		    if (id==14) set_skill(m,skill,90);
		    else set_spell(m,skill,90);
		  }
		}
		break;
	case 16:Read(GET_STR(m));
	case 17:Read(GET_DEX(m));
	case 18:Read(GET_CON(m));
	case 19:Read(GET_INT(m));
	case 20:Read(GET_WIS(m));
	case 21:Read(GET_CHR(m));
	case 22:Read(GET_LCK(m));
	case 23:_Read(GET_HEAD_AC(m));
		 Read(GET_HEAD_STOPPING(m));
	case 24:_Read(GET_BODY_AC(m));
		 Read(GET_BODY_STOPPING(m));
	case 25:_Read(GET_ARMS_AC(m));
		 Read(GET_ARMS_STOPPING(m));
	case 27:_Read(GET_LEGS_AC(m));
		 Read(GET_LEGS_STOPPING(m));
	case 28:_Read(GET_FEET_AC(m));
		 Read(GET_FEET_STOPPING(m));
	case 29:Read(m->specials.M_immune);
	case 30:Read(m->specials.immune);
	case 31:Read(m->specials.susc);
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:Read(m->specials.apply_saving_throw[id-32]);
	case 37:
		_Read(m->specials.nohitdice);
                _Read(m->specials.sizehitdice);
                _Read(m->specials.extrahp);
		break;
	case 38: Read(GET_MOVE(m));
	case 39: Read(GET_EXP(m));
	case 40:
		_Read(m->specials.damnodice);
                _Read(m->specials.damsizedice);
                _Read(GET_DAMROLL(m));
		break;
	case 41: Read(m->specials.affected_by);
	case 42: Read(m->specials.act);
	case 43: Read(m->specials.magic_use);
	case 45: Read(GET_POWER(m));
        case 46: class_set = 1;
                 Read(GET_CLASS(m));
	default:
		nlog("Unknown id in mob file: %d",id);
		exit(1);
    }
  }
  strcat(m->player.long_descr,"\n");
  if (m->player.description)
	strcat(m->player.description,"\n");
  else
	m->player.description= mystrdup("You see nothing special.\n");
  SET_BIT(m->specials.act,ACT_ISNPC);
  m->abilities=m->tmpabilities;
  m->points.max_move=m->points.move;
  m->points.max_mana=m->points.mana;

  if(!class_set){
    if(m->specials.magic_use){
      GET_CLASS(m) = SCHOLAR;
    }
    else{
      GET_CLASS(m) = WARRIOR;
    }
  }

  if(IsHumanoid(m)){
    for(sk = CLASS(m).skills; sk; sk = sk->next){
      if((GET_LEVEL(m) >= sk->max_learn) && (find_skill(m, sk->num) < 0)){
        set_skill(m, sk->num, sk->max_learn);
      }
    }
    for(sp = CLASS(m).spells; sp; sp = sp->next){
      if((GET_LEVEL(m) >= sp->max_learn) && (find_spell(m, sp->num) < 0)){
        set_spell(m, sp->num, sp->max_learn);
      }
    }
  }
  fclose(fp);
}

struct obj_data *get_obj(slong vnum)
{
  struct obj_data *o,*new;
  struct extra_descr_data *last,*ex,*newex;
  index_mem *h;

  if (DEBUG>1) vlog(LOG_DEBUG,"get_obj:%d",vnum);
  h=real_objp(vnum);
  if (!h) {
    vlog(LOG_URGENT,"Object %d does not exist.",vnum);
    return(NULL);
  }
  o=h->obj;
  new=NULL;
  CREATE(new, struct obj_data, 1);
  clear_object(new);
  *new=*o;
  assert(!new->equipped_by);
  assert(!new->in_obj);
  new->name=mystrdup(o->name);
  new->description=mystrdup(o->description);
  if (o->action_description)
    new->action_description=mystrdup(o->action_description);
  new->short_description=mystrdup(o->short_description);
  last = NULL;
  for (ex=o->ex_description; ex; ex=ex->next) {
    newex=NULL;
    CREATE(newex,struct extra_descr_data,1);
    if (last) last->next=newex;
    else new->ex_description=newex;
    newex->keyword=mystrdup(ex->keyword);
    newex->description=mystrdup(ex->description);
    last=newex;
  }
  h->count++;
  if (!db_boot) h->boot_count++;
  new->next = object_list;
  object_list = new;
  if (ITEM_TYPE(new,ITEM_BOARD))
    InitABoard(new);
  return(new);
}

struct char_data *get_mob(slong vnum)
{
  struct char_data *m,*new;
  index_mem *h;

  h=real_mobp(vnum);
  if (!h) {
    vlog(LOG_URGENT,"Mobile (V) %d does not exist in database.", vnum);
    return(0);
  }
  m=h->mob;
  
  new=NULL;
  CREATE(new, struct char_data, 1);
  clear_char(new);
  *new = *m;
 /* 
  new->player.name = mystrdup(m->player.name);
  new->player.short_descr = mystrdup(m->player.short_descr);
  new->player.long_descr = mystrdup(m->player.long_descr);
  new->player.description = mystrdup(m->player.description);
  new->player.title = 0;
*/
  new->points.max_hit=dice(new->specials.nohitdice,new->specials.sizehitdice)+
			new->specials.extrahp;
  GET_HIT(new)=GET_MAX_HIT(new);
  new->player.time.birth = time(0);
  new->player.time.logon  = time(0);
  h->count++;
  new->next=character_list;
  new->in_room=NOWHERE;
  character_list=new;
  mobs_in_game++;
  if(IS_SET(new->specials.act, ACT_SPEC)) mobs_with_special++;
  if (h->func==shop_keeper) shop_give_inventory(new);
  /* the reason this is random is because mobiles don't change speed much */
  /* yet, and so needed this field                                        */
  add_event(number(1, PULSE_MOBILE), EVT_MOBILE, new, NULL, NULL, -1, 0, NULL, event_mobile);
  return(new);
}

/* read an object from OBJ_FILE */
#ifdef OLD_OBJ
struct obj_data *read_object(slong nr, int type)
{
  struct obj_data *obj;
  slong tmp, i;
  char chk[50], buf[100];
  struct extra_descr_data *new_descr;
  
  i = nr;
  if (type == VIRTUAL) {
    nr = real_object(nr);
  }
  if (nr<0 || nr>top_of_objt) {
    sprintf(buf, "Object (V) %d does not exist in database.", i);
    return(0);
  }
  
  fseek(obj_f, obj_index[nr].pos, 0);
  
  obj=NULL;
  CREATE(obj, struct obj_data, 1);
  
  clear_object(obj);
  
  /* *** string data *** */
  
  obj->name = fread_string(obj_f);
  obj->short_description = fread_string(obj_f);
  obj->description = fread_string(obj_f);
  obj->action_description = fread_string(obj_f);
  
  /* *** numeric data *** */
  
  fscanf(obj_f, " %ld ", &tmp); obj->obj_flags.type_flag = tmp;
  fscanf(obj_f, " %ld ", &tmp); obj->obj_flags.extra_flags = tmp;
  fscanf(obj_f, " %ld ", &tmp); obj->obj_flags.wear_flags = tmp;
  fscanf(obj_f, " %ld ", &tmp); obj->obj_flags.value[0] = tmp;
  fscanf(obj_f, " %ld ", &tmp); obj->obj_flags.value[1] = tmp;
  fscanf(obj_f, " %ld ", &tmp); obj->obj_flags.value[2] = tmp;
  fscanf(obj_f, " %ld ", &tmp); obj->obj_flags.value[3] = tmp;
  fscanf(obj_f, " %ld ", &tmp); obj->obj_flags.weight = tmp;
  fscanf(obj_f, " %ld \n", &tmp); obj->obj_flags.cost = tmp;
  fscanf(obj_f, " %ld \n", &tmp); obj->obj_flags.cost_per_day = tmp;
  
  /* *** extra descriptions *** */
  
  obj->ex_description = 0;
  
  while (fscanf(obj_f, " %s \n", chk), *chk == 'E')	{
    CREATE(new_descr, struct extra_descr_data, 1);
    
    new_descr->keyword = fread_string(obj_f);
    new_descr->description = fread_string(obj_f);
    
    new_descr->next = obj->ex_description;
    obj->ex_description = new_descr;
    new_descr=NULL;
  }
  
  for( i = 0 ; (i < MAX_OBJ_AFFECT) && (*chk == 'A') ; i++)	{
    fscanf(obj_f, " %ld ", &tmp);
    obj->affected[i].location = tmp;
    fscanf(obj_f, " %ld \n", &tmp);
    obj->affected[i].modifier = tmp;
    fscanf(obj_f, " %s \n", chk);
  }
  
  for (;(i < MAX_OBJ_AFFECT);i++)	{
    obj->affected[i].location = APPLY_NONE;
    obj->affected[i].modifier = 0;
  }
  
  obj->in_room = NOWHERE;
  obj->next_content = 0;
  obj->carried_by = 0;
  obj->equipped_by = 0;
  obj->in_obj = 0;
  obj->contains = 0;
  obj->item_number = nr;	
  obj->virtual = obj_index[nr].virtual;
  
  obj->next = object_list;
  object_list = obj;
  
  obj_index[nr].number++;

  if (ITEM_TYPE(obj,ITEM_BOARD) {
    InitABoard(obj);
  }
  
  return (obj);  
}
#endif




#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void)
{
  int i;
  struct reset_q_element *update_u, *temp, *tmp2;
  extern struct reset_q_type reset_q;
  
  
  /* enqueue zones */
  
  for (i = 0; i <= top_of_zone_table; i++)	{
    if (zone_table[i].age < zone_table[i].lifespan &&
	zone_table[i].reset_mode)
      (zone_table[i].age)++;
    else
      if (zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode)	{
	/* enqueue zone */
	
	CREATE(update_u, struct reset_q_element, 1);
	
	update_u->zone_to_reset = i;
	update_u->next = 0;
	
	if (!reset_q.head)
	  reset_q.head = reset_q.tail = update_u;
	else  {
	  reset_q.tail->next = update_u;
	  reset_q.tail = update_u;
	}
	
	zone_table[i].age = ZO_DEAD;
      }
  }
  
  /* dequeue zones (if possible) and reset */
  
  for (update_u = reset_q.head; update_u; update_u = tmp2) {
    if (update_u->zone_to_reset > top_of_zone_table) {
      
      /*  this may or may not work */
      /*  may result in some lost memory, but the loss is not signifigant
	  over the short run
	  */
      update_u->zone_to_reset = 0;
      update_u->next = 0;
    }
    tmp2 = update_u->next;
    
    if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
	is_empty(update_u->zone_to_reset)) {
      reset_zone(update_u->zone_to_reset);
      /* dequeue */
      
      if (update_u == reset_q.head)
	reset_q.head = reset_q.head->next;
      else {
	for (temp = reset_q.head; temp->next != update_u; temp = temp->next)
	  ;
	
	if (!update_u->next) 
	  reset_q.tail = temp;
	
	temp->next = update_u->next;
	
      }
      
      FREE(update_u);
    }
  }
}




#define ZCMD zone_table[zone].cmd[cmd_no]

/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
  int cmd_no, last_cmd = 1;
  struct index_mem *ind;
  struct char_data *mob=0;
  struct obj_data *obj, *obj_to;
  struct room_data	*rp;
  
  if (DEBUG) vlog(LOG_DEBUG,"Reset zone %d",zone);
  for (cmd_no = 0;;cmd_no++) {
      if (ZCMD.command == 'S')
	break;
      if (ZCMD.line == -1) { last_cmd=0; continue; }
      
      if (last_cmd || !ZCMD.if_flag)
	switch(ZCMD.command) {
	  case 'M': /* read a mobile */
	    if (real_mobp(ZCMD.arg1)->count < ZCMD.arg2) {
	      mob = get_mob(ZCMD.arg1);
	      char_to_room(mob, ZCMD.arg3);
	      last_cmd = 1;
	    } else last_cmd = 0;
	    break;
	    
	  case 'O': /* read an object */
	    ind=real_objp(ZCMD.arg1);
	    if (!ind) {
	      last_cmd=0;
	      break;
	    }
	    if (ind->count < ZCMD.arg2 && ind->boot_count < ind->max_at_boot ) {
	      if (real_roomp(ZCMD.arg3)) {
		  rp=real_roomp(ZCMD.arg3);
		  if (!get_obj_in_list_num(ZCMD.arg1,rp->contents)) {
		    obj=get_obj(ZCMD.arg1);
		    obj_to_room(obj, ZCMD.arg3);
		    last_cmd = 1;
		  } else
		    last_cmd = 0;
	      } else {
		vlog(LOG_URGENT,"Error finding room #%d", ZCMD.arg3);
		last_cmd = 0;
	      }
	    } else last_cmd = 0;
	  break;
	    
	  case 'P': /* object to object */
	    ind=real_objp(ZCMD.arg1);
	    if (ind->count < ZCMD.arg2 && ind->boot_count < ind->max_at_boot ) {
	      obj = get_obj(ZCMD.arg1);
	      obj_to = get_obj_num(ZCMD.arg3);
	      if (obj_to) {
		obj_to_obj(obj, obj_to);
		last_cmd = 1;
	      } else {
		last_cmd = 0;
	      }
	    } else last_cmd = 0;
	    break;
	    
	  case 'G': /* obj_to_char */
	    ind=real_objp(ZCMD.arg1);
	    if (ind->count < ZCMD.arg2 && ind->boot_count < ind->max_at_boot ) {
	      obj=get_obj(ZCMD.arg1);
	      obj_to_char(obj, mob);
	      last_cmd = 1;
	    } else last_cmd = 0;
	    break;
	    
	  case 'H': /* hatred to char */
	    
	    AddHatred(mob, ZCMD.arg1, ZCMD.arg2);
	    last_cmd = 1;
	    break;
	    
	  case 'F': /* fear to char */
	    
	    AddFears(mob, ZCMD.arg1, ZCMD.arg2);
	    last_cmd = 1;
	    break;
	    
	  case 'E': /* object to equipment list */
	    ind=real_objp(ZCMD.arg1);
	    if (ind->count < ZCMD.arg2 && ind->boot_count < ind->max_at_boot ) {
	      obj=get_obj(ZCMD.arg1);
	      equip_char(mob, obj, ZCMD.arg3);
	      last_cmd = 1;
	    } else last_cmd = 0;
	    break;
	    
	  case 'D': /* set state of door */
	    rp = real_roomp(ZCMD.arg1);
	    if (rp && rp->dir_option[ZCMD.arg2]) {
	      switch (ZCMD.arg3) {
	      case 0:
		CLEAR_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
		CLEAR_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
		break;
	      case 1:
		SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
		CLEAR_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
		break;
	      case 2:
		SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
		SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
		break;
	      }
	      last_cmd = 1;
	    } else {
	      /* that exit doesn't exist anymore */
	    }
	    break;
	    
	  default:
	    nlog("Undefd cmd in reset table; zone %d cmd %d.", zone, cmd_no);
	    break;
	  }
      else
	last_cmd = 0;
      
    }
  
  zone_table[zone].age = 0;
}

#undef ZCMD

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
	struct connection_data *i;

  for (i = connection_list; i; i = i->next)
    if (!i->connected)
      if (real_roomp(i->character->in_room)->zone == zone_nr)
	return(0);
  return(1);
}





void delete_entry(struct char_data *ch, char *arg, int cmd)
{
	int ind;
	char name[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch)) return;
	arg=one_argument(arg,name);
	if (!*name) {
		send_to_char("Who do you want to delete?\n",ch);
		return;
	}
	ind = open(name_to_path(name),O_RDONLY);
	if (ind<0) {
		send_to_char("Not found.\n",ch);
		return;
	}
	close(ind);
	sprintf(buf,"/bin/mv %s PURGED/",name_to_path(name));
	system(buf);
	/*unlink(name_to_path(name)); */
	num_players--;
	if (cmd != -1)
	  vlog(LOG_URGENT,"%s deleted %s",GET_NAME(ch),name);
}



/* create a new entry in the in-memory index table for the player file */
void create_entry(char *name)
{
	FILE *f;

	num_players++;
	f=fopen(PLAYER_FILE,"a");
	fprintf(f,"%s\n",name);
	fclose(f);
}


/* for possible later use with qsort */
int compare(struct player_index_element *arg1, struct player_index_element 
	*arg2)
{
	return (str_cmp(arg1->name, arg2->name));
}




/************************************************************************
*  procs of a (more or less) general utility nature			*
********************************************************************** */


/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE *fl)
{
  char buf[MAX_STRING_LENGTH], tmp[500];
  char *rslt;
  register char *point;
  int flag;
  
  bzero(buf, sizeof(buf));
  
  do	{
    if (!fgets(tmp, MAX_STRING_LENGTH, fl))		{
      perror("fread_string");
      log("File read error.");
      return("Empty");
    }
    
    if (strlen(tmp) + strlen(buf) + 1 > MAX_STRING_LENGTH) {
      log("fread_string: string too large (db.c)");
      exit(0);
    } else
      strcat(buf, tmp);
    
    for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
	 point--);		
/* I probably messed this up, but it isn't really used, so I will fix it later.
ACC 8/11/93 */
    if ((flag = (*point == '~')))
      *(buf + strlen(buf) -2) = '\0';
    else {
	*(buf + strlen(buf) + 1) = '\0';
	*(buf + strlen(buf)) = '\n';
      }
  } while (!flag);
  
  /* do the allocate boogie  */
  
  if (strlen(buf) > 0)    {
      CREATE(rslt, char, strlen(buf) + 1);
      strcpy(rslt, buf);
  }  else
    rslt = 0;
  return(rslt);
}





/* release memory allocated for a char struct */
void free_char(struct char_data *ch)
{
	struct char_data *p;
	struct obj_data *o,*next;

	for (p=character_list; p && p!=ch; p=p->next);
	if (p) {
	  nlog("Free before extract");
	  extract_char(ch);
	}
	if (DEBUG>1 && !db_boot) nlog("Free char:%s",GET_NAME(ch));
        stopAllFromConsenting(ch); /* moved from extract_char - SLB */
	strip_char(ch);
	for (o=ch->warehouse; o; o=next) {
	  next=o->next_content;
	  obj_from_warehouse(o);
	  extract_obj(o);
	}
	if (!IS_NPC(ch)) {
	  FREE(ch->player.name);
	  FREE(ch->player.title);
	  FREE(ch->player.short_descr);
	  FREE(ch->player.long_descr);
	  FREE(ch->player.description);
	  FREE(ch->player.sounds);
	  FREE(ch->player.distant_snds);
	  FREE(ch->skills);
	}
	FREE(ch->player.pw);
	FREE(ch->player.talks);
	FREE(ch->kills);
	FREE(ch->prompt);

	while (ch->affected)
	  affect_remove(ch,ch->affected);

	FREE(ch);
}



/* release memory allocated for an obj struct */
void free_obj(struct obj_data *obj)
{
	struct extra_descr_data *this, *next_one;

	if (DEBUG>1 && !db_boot) vlog(LOG_DEBUG,"Free Obj: %s",obj->name);
	FREE(obj->name);
	FREE(obj->description);
	FREE(obj->short_description);
	FREE(obj->action_description);
	FREE(obj->held_for);

	for( this = obj->ex_description ;
		(this != 0);this = next_one )
	{
		next_one = this->next;
		FREE(this->keyword);
		FREE(this->description);
		FREE(this);
	}

	FREE(obj);
}





int file_to_q(char *name, struct connection_data *d, int more)
{
    char buf[MAX_STRING_LENGTH];

    file_to_string(name,buf);

    if(more)
      page_string(d, buf, 0);
    else 
      SEND_TO_Q(buf,d);

    return(0);
}

/* read contents of a text file, and place in buf */
int file_to_string(char *name, char *buf)
{
	FILE *fl;
	char tmp[256];

	*buf = '\0';

	if (!(fl = fopen(name, "r")))
	{
		perror(name);
		*buf = '\0';
		return(-1);
	}

	do {
		fgets(tmp, 255, fl);

		if (!feof(fl)) {
		  if (strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH) {
		    vlog(LOG_URGENT,"%s: string too big.",name);
		    *buf = '\0';
		    fclose(fl);
		    return(-1);
		  }

		  strcat(buf, tmp);
		}
	}
	while (!feof(fl));

	fclose(fl);

	return(0);
}




/* clear some of the the working variables of a char */
void reset_char(struct char_data *ch)
{
  struct obj_data *o[MAX_WEAR];
  struct affected_type *af;
  int i;
  int mod;
  
  if (GET_BANK(ch)>200000000) {
    vlog(LOG_URGENT,"Wiping %s's BANK = %d",GET_NAME(ch),GET_BANK(ch));
    GET_BANK(ch)=0;
  }
  if (GET_GOLD(ch)>200000000) {
    vlog(LOG_URGENT,"Wiping %s's gold = %d",GET_NAME(ch),GET_GOLD(ch));
    GET_GOLD(ch)=0;
  }
  for (i=0; i<MAX_WEAR; i++)
    if (ch->equipment[i])
	o[i]=unequip_char(ch,i);
    else o[i]=NULL;
  for (i=0; i<6; i++) {
    ch->points.armor[i]=100;
    ch->points.stopping[i]=0;
  }

  /* take into account any spells affecting armor */
  for (af=ch->affected; af; af=af->next) {
    mod = af->modifier;
    switch (af->location) {
      case APPLY_ALL_AC:
	GET_BODY_AC(ch) += mod;
	GET_HEAD_AC(ch) += mod;
	GET_ARMS_AC(ch) += mod;
	GET_LEGS_AC(ch) += mod;
	GET_FEET_AC(ch) += mod;
	break;

      case APPLY_BODY_AC: GET_BODY_AC(ch) +=mod; break;
      case APPLY_HEAD_AC: GET_HEAD_AC(ch) += mod; break;
      case APPLY_ARMS_AC: GET_ARMS_AC(ch) += mod; break;
      case APPLY_LEGS_AC: GET_LEGS_AC(ch) += mod; break;
      case APPLY_FEET_AC: GET_FEET_AC(ch) += mod; break;

      case APPLY_ALL_STOPPING:
	GET_BODY_STOPPING(ch) += mod;
	GET_HEAD_STOPPING(ch) += mod;
	GET_ARMS_STOPPING(ch) += mod;
	GET_LEGS_STOPPING(ch) += mod;
	GET_FEET_STOPPING(ch) += mod;
	break;

      case APPLY_BODY_STOPPING: GET_BODY_STOPPING(ch) +=mod; break;
      case APPLY_HEAD_STOPPING: GET_HEAD_STOPPING(ch) += mod; break;
      case APPLY_ARMS_STOPPING: GET_ARMS_STOPPING(ch) += mod; break;
      case APPLY_LEGS_STOPPING: GET_LEGS_STOPPING(ch) += mod; break;
      case APPLY_FEET_STOPPING: GET_FEET_STOPPING(ch) += mod; break;

/*	default:*/
      }
  }

  for (i=0; i<MAX_WEAR; i++)
    if (o[i]) equip_char(ch,o[i],i);

  if (GET_HIT(ch) <= 0) {
    GET_HIT(ch) = 1;
    ch->specials.position = POSITION_STANDING;
    ch->specials.default_pos = POSITION_STANDING;
  }
  ch->specials.position=POSITION_STANDING;
  clear_bit(ch->specials.affected_by,AFF_GROUP);

  ch->aggressive = -1;
  if (GET_MOVE(ch) <= 0)
    GET_MOVE(ch) = 1;
  if (GET_MANA(ch) < 0)
    GET_MANA(ch) = 0;
  if (GET_POWER(ch) < 0)
    GET_POWER(ch) = 0;

  if (GET_BANK(ch) > 2000000)
    vlog(LOG_MISC,"%s has %d coins in bank.", GET_NAME(ch), GET_BANK(ch));
  if (GET_GOLD(ch) > 2000000)
    vlog(LOG_MISC,"%s has %d coins.", GET_NAME(ch), GET_GOLD(ch));
}



/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
void clear_char(struct char_data *ch)
{
        int i;
	memset(ch, '\0', sizeof(struct char_data));

	ch->specials.beacon_location=LOAD_ROOM_MORTAL;
	ch->in_room = NOWHERE;
	ch->hates.clist = 0;
	ch->fears.clist = 0;
	ch->specials.was_in_room = NOWHERE;
	ch->specials.position = POSITION_STANDING;
	ch->specials.default_pos = POSITION_STANDING;
        for(i=0; i < LOCATION_MAX; i++) {
          ch->points.armor[i] = 100;
          ch->points.stopping[i] = 0;
        }
}


void clear_object(struct obj_data *obj)
{
	memset(obj, '\0', sizeof(struct obj_data));

	obj->in_room	  = NOWHERE;
}




/* initialize a new character only if class is set */
void init_char(struct char_data *ch)
{
	int i;

	/* *** if this is our first player --- he be God *** */

	if (num_players==0)	{
		printf("%s is the implementor\n",GET_NAME(ch));
		GET_EXP(ch) = 24000000;
		SET_LEVEL(ch,IMPLEMENTOR);
	} else {
	  ch->player.level=0;
	}

	set_title(ch);

	ch->player.short_descr = 0;
	ch->player.long_descr = 0;
	ch->player.description = 0;

	ch->player.hometown = number(1,4);

	ch->player.time.birth = time(0);
	ch->player.time.played = 0;
	ch->player.time.logon = time(0);

	ch->player.ntalks=0;
	ch->player.talks=NULL;

	ch->points.mana = GET_MAX_MANA(ch);
	ch->points.power = GET_MAX_POWER(ch);
	ch->points.hit = GET_MAX_HIT(ch);
	ch->points.move = GET_MAX_MOVE(ch);

        for(i=0; i < LOCATION_MAX; i++) {
           ch->points.armor[i] = 100;
           ch->points.stopping[i] = 0;
        }

	ch->nskills=0;
	ch->skills=NULL;
	ch->nspells=0;
	ch->spells=NULL;
	init_bits(ch->specials.affected_by);
	ch->specials.spells_to_learn = 0;

	for (i = 0; i < 5; i++)
		ch->specials.apply_saving_throw[i] = 0;

	for (i = 0; i < 3; i++)
		GET_COND(ch, i) = (GetMaxLevel(ch) > GOD ? -1 : 24);

	GET_HEIGHT(ch)=generate_height(GET_RACE(ch));
	GET_WEIGHT(ch)=generate_weight(GET_RACE(ch));
}

struct room_data *real_roomp(slong virtual)
{
  static long last=0;
  static struct room_data *rp=0;

  if (last==virtual && rp) return(rp);
  last=virtual;
  return(rp=hash_find(&room_db, virtual));
}


/* returns the real number of the monster with given virtual number */
index_mem *real_mobp(slong virtual)
{
  return((index_mem *)hash_find(&mob_db,virtual));
}

index_mem *real_objp(slong virtual)
{
  return((index_mem *)hash_find(&obj_db,virtual));
}


void wipe_obj(struct obj_data *obj)
{
  if(obj){
    wipe_obj(obj->contains);
    wipe_obj(obj->next_content);
    if (obj->in_obj)
      obj_from_obj(obj);
    extract_obj(obj);
  }
}

void special_boot_check(struct char_data *ch)
{
#if 0
  struct obj_data *o;
  int count=0;

  for (o=object_list; o; o=o->next)
	if (o->virtual==252) count++;
  if (count)
	fprintf(stderr,"%4d %s\n",count,GET_NAME(ch));
#endif
}
