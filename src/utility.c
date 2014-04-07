/* ************************************************************************
*  file: utility.c, Utility module.                       Part of DIKUMUD *
*  Usage: Utility procedures                                              *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#ifdef USE_MALLOP
#include <malloc.h>
#endif
#include <string.h>
#include <assert.h>
#if 0
#include <unistd.h>
#endif

#include <time.h>
#ifdef _SEQUENT_
#include <sys/types.h>
#include <sys/procstats.h>
#endif
#include <fcns.h>
#include <utils.h>
#include <spells.h>
#include <race.h>
#include <db.h>
#include <comm.h>
#include <hash.h>
#include <ticks.h>
#include <externs.h>

extern struct obj_data  *object_list;

extern struct char_data *character_list;
extern struct hash_header room_db;	                  /* In db.c */
extern char *dirs[]; 

#ifndef _SEQUENT_
void cftime(char *buf,char *fmt,time_t *time)
{
  strftime(buf,1024,fmt,localtime(time));
}
#endif


/* external functions */
void stop_fighting(struct char_data *ch);
void fake_setup_dir(FILE *fl, int room, int dir);
void setup_dir(FILE *fl, int room, int dir);
char *fread_string(FILE *fl);


int MIN(int a, int b)
{
	return a < b ? a:b;
}


int MAX(int a, int b)
{
	return a > b ? a:b;
}

void ForAllObjects(struct char_data *ch, int val,int (*func)(struct obj_data *,struct char_data *,int))
{
  int result;
  struct obj_data *o,*next;

  for (o=object_list; o; o=next) {
    next=o->next;
    result=func(o,ch,val);
    if (result==MSG_DONE) return;
  }
}


int GetItemClassRestrictions(struct obj_data *obj)
{
  int total=0;
/* ANTIS DISABLED UNTIL FIXED!
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MAGE)) {
    total += MAGE;
  }
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_THIEF)) {
    total += THIEF;
  }
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_FIGHTER)) {
    total += WARRIOR;
  }
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_CLERIC)) {
    total += CLERIC;
  }
 if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_RANGER)) {
   total += RANGER;
 }
 if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_PALADIN)) {
   total += PALADIN;
 }
 if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_ASSASSIN)) {
   total += ASSASSIN;
 }
 if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_NINJA)) {
   total += NINJA;
 }
 if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_BARD)) {
   total += BARD;
 }
 if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_CAVALIER)) {
   total += CAVALIER;
 }
 if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_AVENGER)) {
   total += AVENGER;
 }
 if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_BARBARIAN)) {
   total += BARBARIAN;
 }
 if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_DRUID)) {
   total += DRUID;
 }
 if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_ILLUSIONIST)) {
   total += ILLUSIONIST;
 }
*/
  return(total);

}

int CAN_SEE(struct char_data *s, struct char_data *o)
{

  if (!o)
    return(FALSE);

  if (GET_LEVEL(s)==IMPLEMENTOR && !IS_NPC(s)) return(TRUE);

  if ((s->in_room == -1) || (o->in_room == -1)) {
    return(FALSE);
  }

  if (IS_IMMORTAL(s) && !IS_IMMORTAL(o)) 
    return(TRUE);

  if (IS_IMMORTAL(s) && IS_IMMORTAL(o) && GET_LEVEL(s)>=GET_LEVEL(o))
    return(TRUE);

  if (o->specials.invis_level > GET_LEVEL(s))
    return FALSE;

  if (IS_AFFECTED(s, AFF_TRUE_SIGHT))
    return(TRUE);

  if (IS_AFFECTED(s, AFF_BLIND))
    return(FALSE);

  if (IS_AFFECTED(o, AFF_HIDE))
    return(FALSE);

  if (IS_AFFECTED(o, AFF_INVISIBLE)) {
    if (IS_IMMORTAL(o))
      return(FALSE);
    if (!IS_AFFECTED(s, AFF_DETECT_INVISIBLE))
      return(FALSE);
  }

  if ((IS_DARK(s->in_room) || IS_DARK(o->in_room)) &&
        (!IS_AFFECTED(s, AFF_INFRAVISION)))
        return(FALSE);

  return(TRUE);
}


int exit_ok(struct room_direction_data	*exit, struct room_data **rpp)
{
  struct room_data	*rp;
  if (rpp==NULL)
    rpp = &rp;
  if (!exit) {
    *rpp = NULL;
    return FALSE;
  }
  *rpp = real_roomp(exit->to_room);
  return (*rpp!=NULL);
}

int MobVnum( struct char_data *c)
{
  return(c->virtual);
}

int ObjVnum( struct obj_data *o)
{
  return(o->virtual);
}


void Zwrite (FILE *fp, char cmd, int tf, int arg1, int arg2, int arg3, char *desc)
{
   char buf[100];

   if (*desc) {
     sprintf(buf, "%c %d %d %d %d   ; %s\n", cmd, tf, arg1, arg2, arg3, desc);
     fputs(buf, fp);
   } else {
     sprintf(buf, "%c %d %d %d %d\n", cmd, tf, arg1, arg2, arg3); 
     fputs(buf, fp);
   }
}

void RecZwriteObj(FILE *fp, struct obj_data *o)
{
   struct obj_data *t;

   if (GET_ITEM_TYPE(o) == ITEM_CONTAINER) {
     for (t = o->contains; t; t=t->next_content) {
       Zwrite(fp, 'P', 1, t->virtual, real_objp(t->virtual)->count, o->virtual, 
	      t->short_description);
       RecZwriteObj(fp, t);
     }
     return;
   }
}

FILE *MakeZoneFile( struct char_data *c)
{
  char buf[256];
  char buf1[256];
  FILE *fp;

  sprintf(buf, "Zone/%s.zon", GET_NAME(c));
  sprintf(buf1,"mv %s %s.bak",buf,buf);
  system(buf1);

  if ((fp = fopen(buf, "w+")) != NULL)
    return(fp);
  else
    return(0);

}


int IsImmune(struct char_data *ch, int bit)
{
  return(IS_SET(bit, ch->specials.M_immune));
}

int IsResist(struct char_data *ch, int bit)
{
  return(IS_SET(bit, ch->specials.immune));
}

int IsSusc(struct char_data *ch, int bit)
{
  return(IS_SET(bit, ch->specials.susc));
}

/* creates a random number in interval [from;to] */
int number(int from, int to)
{
   if (to - from + 1 )
	return((random() % (to - from + 1)) + from);
   else
       return(from);
}



/* simulates dice roll */
int dice(int number, int size) 
{
  int r;
  int sum = 0;

  if (size <= 0) return(0);
  if (size*number>65535) return(0);

  for (r = 1; r <= number; r++) sum += ((random() % size)+1);
  return(sum);
}



/* Create a duplicate of a string */
char *mystrdup(char *source) {
	char *new;

	if (source) {
	  CREATE(new, char, strlen(source)+1);
	  return(strcpy(new, source));
        }
	CREATE(new,char,1);
	*new='\0';
	return(new);
}

int scan_number(char *text, int *rval)
{
  int	length;
  if (1!=sscanf(text, " %i %n", rval, &length))
    return 0;
  if (text[length] != 0)
    return 0;
  return 1;
}


/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
	int chk, i;


        if ((!arg2) || (!arg1))
	  return(1);

	for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
		if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
			if (chk < 0)
				return (-1);
			else 
				return (1);
	return(0);
}



/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(char *arg1, char *arg2, int n)
{
	int chk, i;

	for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n>0); i++, n--)
		if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
			if (chk < 0)
				return (-1);
			else 
				return (1);

	return(0);
}


/* writes a string to the log */
void log2(char *str)
{
  logl(SILLYLORD,"{}",str);
}

/* writes a string to the log */
void log(char *str)
{
  logl(CREATOR,"//",str);
}

void slog(char *str)
{
	long ct;
	char *tmstr;

	ct = time(0);
	tmstr = asctime(localtime(&ct));
	*(tmstr + strlen(tmstr) - 1) = '\0';
	fprintf(stderr, "%s :: %s\n", tmstr, str);

}
	


void sprintbit(unsigned long vektor, char *names[], char *result)
{
  long nr;
  
  *result = '\0';
  
  for(nr=0; vektor; vektor>>=1)
    {
      if (IS_SET((long)1, vektor))
	if (*names[nr] != '\n') {
	  strcat(result,names[nr]);
	  strcat(result," ");
	} else {
	  strcat(result,"UNDEFINED");
	  strcat(result," ");
	}
      if (*names[nr] != '\n')
	nr++;
    }
  
  if (!*result)
    strcat(result, "NOBITS");
}



void sprinttype(int type, char *names[], char *result)
{
	int nr;

	for(nr=0;(*names[nr]!='\n');nr++);
	if(type < nr)
		strcpy(result,names[type]);
	else
		strcpy(result,"UNDEFINED");
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
	long secs;
	struct time_info_data now;

	secs = (long) (t2 - t1);

  now.hours = (secs/SECS_PER_REAL_HOUR) % 24;  /* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR*now.hours;

  now.day = (secs/SECS_PER_REAL_DAY);          /* 0..34 days  */
  secs -= SECS_PER_REAL_DAY*now.day;

	now.month = -1;
  now.year  = -1;

	return now;
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
	long secs;
	struct time_info_data now;

	secs = (long) (t2 - t1);

  now.hours = (secs/SECS_PER_MUD_HOUR) % 24;  /* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR*now.hours;

  now.day = (secs/SECS_PER_MUD_DAY) % 35;     /* 0..34 days  */
  secs -= SECS_PER_MUD_DAY*now.day;

	now.month = (secs/SECS_PER_MUD_MONTH) % 17; /* 0..16 months */
  secs -= SECS_PER_MUD_MONTH*now.month;

  now.year = (secs/SECS_PER_MUD_YEAR);        /* 0..XX? years */

	return now;
}



struct time_info_data age(struct char_data *ch)
{
	struct time_info_data player_age;

	player_age = mud_time_passed(time(0),ch->player.time.birth);

  player_age.year += 17;   /* All players start at 17 */

	return(player_age);
}


char in_group ( struct char_data *ch1, struct char_data *ch2)
{



/* 
   three possibilities ->
   1.  char is char2's master
   2.  char2 is char's master
   3.  char and char2 follow same.
  

    otherwise not true.
 
*/
   if (ch1 == ch2)
      return(TRUE);

   if ((!ch1) || (!ch2))
      return(0);

   if ((!ch1->master) && (!ch2->master))
      return(0);

   if (ch2->master)
      if (!strcmp(GET_NAME(ch1),GET_NAME(ch2->master))) {
         return(1);
      }

   if (ch1->master)
      if (!strcmp(GET_NAME(ch1->master),GET_NAME(ch2))) {
         return(1);
      }

   if ((ch2->master) && (ch1->master)) 
      if (!strcmp(GET_NAME(ch1->master),GET_NAME(ch2->master))) {
         return(1);
      }
  

   return(0);


}


/*
  more new procedures 
*/


/*
   these two procedures give the player the ability to buy 2*bread
   or put all.bread in bag, or put 2*bread in bag...
*/

char getall(char *name, char *newname)
{
   char arg[40],tmpname[80], otname[80];
   char prd;
   int i;

   for (i=0; i<40; i++) { arg[i]='\0'; tmpname[i]='\0'; otname[i]='\0'; }

   sscanf(name,"%s ",otname);   /* reads up to first space */

   if (strlen(otname)<5)
      return(FALSE);

   sscanf(otname,"%3s%c%s",arg,&prd,tmpname);

   if (prd != '.')
     return(FALSE);
   if (tmpname == NULL) 
      return(FALSE);
   if (strcmp(arg,"all"))
      return(FALSE);

   while (*name != '.')
       name++;

   name++;

   for (; (*newname = *name); name++,newname++);

   return(TRUE);
}


int getabunch(char *name, char  *newname)
{
   int num=0;
   char tmpname[80];

   *tmpname='\0';
   sscanf(name,"%d*%s",&num,tmpname);
   if (tmpname[0] == '\0')
      return(FALSE);
   if (num < 1)
      return(FALSE);
   if (num>9)
      num = 9;

   while (*name != '*')
       name++;

   name++;

   for (; (*newname = *name); name++,newname++);

   return(num);

}


int DetermineExp( struct char_data *mob, int exp_flags)
{

int base;
int phit;
int sab;

/* 
reads in the monster, and adds the flags together 
for simplicity, 1 exceptional ability is 2 special abilities 
*/

  if (GET_LEVEL(mob) < 0)
    return(1);

  switch(GET_LEVEL(mob)) {
    case 0:   base = 5;     phit = 1;   sab = 10; break;
    case 1:   base = 10;    phit = 1;   sab = 15; break;
    case 2:   base = 20;    phit = 2;   sab = 20; break;
    case 3:   base = 35;    phit = 3;   sab = 25; break;
    case 4:   base = 60;    phit = 4;   sab = 30; break;
    case 5:   base = 90;    phit = 5;   sab = 40; break;
    case 6:   base = 150;   phit = 6;   sab = 75; break;
    case 7:   base = 225;   phit = 8;   sab = 125; break;
    case 8:   base = 600;   phit = 12;  sab = 175; break;
    case 9:   base = 900;   phit = 14;  sab = 300; break;
    case 10:  base = 1100;  phit = 15;  sab = 450; break;
    case 11:  base = 1300;  phit = 16;  sab = 700; break;
    case 12:  base = 1550;  phit = 17;  sab = 700; break;
    case 13:  base = 1800;  phit = 18;  sab = 950; break;
    case 14:  base = 2100;  phit = 19;  sab = 950; break;
    case 15:  base = 2400;  phit = 20;  sab = 1250; break;
    case 16:  base = 2700;  phit = 23;  sab = 1250; break;
    case 17:  base = 3000;  phit = 25;  sab = 1550; break;
    case 18:  base = 3500;  phit = 28;  sab = 1550; break;
    case 19:  base = 4000;  phit = 30;  sab = 2100; break;
    case 20:  base = 4500;  phit = 33;  sab = 2100; break;
    case 21:  base = 5000;  phit = 35;  sab = 2600; break;
    case 22:  base = 6000;  phit = 40;  sab = 3000; break;
    case 23:  base = 7000;  phit = 45;  sab = 3500; break;
    case 24:  base = 8000;  phit = 50;  sab = 4000; break;
    case 25:  base = 9000;  phit = 55;  sab = 4500; break;
    case 26:  base = 10000; phit = 60;  sab = 5000; break;
    case 27:  base = 12000; phit = 70;  sab = 6000; break;
    case 28:  base = 14000; phit = 80;  sab = 7000; break;
    case 29:  base = 16000; phit = 90;  sab = 8000; break;
    case 30:  base = 20000; phit = 100; sab = 10000; break;
    default : base = 25000; phit = 150; sab = 20000; break;
  }
  return(base + (phit * GET_HIT(mob)) + (sab * exp_flags));
}

void event_river(struct event_t *event)
{
  struct room_data *rp;
  char buf[MAX_STRING_LENGTH];
  int rd;

  if(!(rp = real_roomp(event->room)) || (event->ch->in_room != event->room)){
    return;
  }
  rd = rp->river_dir % 6;
  if(!IS_NPC(event->ch) && !IS_AFFECTED(event->ch, AFF_FLYING)){
    if(rp->dir_option[rd] && rp->dir_option[rd]->to_room &&
        (EXIT(event->ch, rd)->to_room != NOWHERE) && rp->river_speed){
      if(event->ch->specials.fighting){
        stop_fighting(event->ch);
      }
      sprintf(buf, "You drift %s.\n", dirs[rd]);
      send_to_char(buf, event->ch);
      sprintf(buf, "$N drifts %s.", dirs[rd]);
      act(buf, 1, event->ch, NULL, NULL, TO_ROOM);
      char_from_room(event->ch);
      char_to_room(event->ch, rp->dir_option[rd]->to_room);
      do_look(event->ch, "\0",15);
      if(IS_SET(RM_FLAGS(event->ch->in_room), DEATH) && 
          GET_LEVEL(event->ch) < LOW_IMMORTAL){
        death_trap(event->ch);
      }
    }
  }
}

void RoomSave(struct char_data *ch, long start, long end)
{
   char temp[2048], dots[500];
   int rstart, rend, i, j, k, x;
   struct extra_descr_data *exptr;
   FILE *fp;
   struct room_data	*rp;
   struct room_direction_data	*rdd;

   sprintf(temp,"Area/%s",ch->player.name);
   if ((fp = fopen(temp,"w")) == NULL) {
     send_to_char("Can't write to disk now..try later \n",ch);
     return;
   }


   rstart = start;
   rend = end;

   if (((rstart <= -1) || (rend <= -1))) {
    send_to_char("I don't know those room #s.  make sure they are all\n",ch);
    send_to_char("contiguous.\n",ch);
    fclose(fp);
    return;
   }
   fprintf(fp,"%lu %lu\n",rstart,rend);

   send_to_char("Saving\n",ch);
   strcpy(dots, "\0");
   
   for (i=rstart;i<=rend;i++) {

     rp = real_roomp(i);
     if (rp==NULL)
       continue;


     if (rend-rstart<499)
       strcat(dots, ".");

/*
   strip ^Ms from description
*/
     x = 0;

     if (!rp->description) {
       CREATE(rp->description, char, 128);
       strcpy(rp->description, "Empty");
     }

     for (k = 0; k <= strlen(rp->description); k++) {
       if (rp->description[k] != 13)
	 temp[x++] = rp->description[k];
     }
     temp[x] = '\0';

     fprintf(fp,"#%d\n%s~\n%s~\n",rp->number,rp->name, temp);

     if (!rp->tele_targ) {
        fprintf(fp,"%d %d %d",rp->zone, rp->room_flags, rp->sector_type);
      } else {
	fprintf(fp, "%d %d -1 %d %d %d %d", rp->zone, rp->room_flags,
		rp->tele_time, rp->tele_targ, 
		rp->tele_look, rp->sector_type);
      }
     if (rp->sector_type == SECT_WATER_NOSWIM) {
        fprintf(fp," %d %d",rp->river_speed,rp->river_dir);
     } 

     fprintf(fp,"\n");     

     for (j=0;j<6;j++) {
       rdd = rp->dir_option[j];
       if (rdd) {
          fprintf(fp,"D%d\n",j);

	  if (rdd->general_description) {
	   if (strlen(rdd->general_description) > 0)
              x = 0;

              for (k = 0; k <= strlen(rdd->general_description); k++) {
                 if (rdd->general_description[k] != 13)
	            temp[x++] = rdd->general_description[k];
              }
	      temp[x] = '\0';

            fprintf(fp,"%s~\n", temp);
	  } else {
	    fprintf(fp,"~\n");
	  }

	  if (rdd->keyword) {
	   if (strlen(rdd->keyword)>0)
	     fprintf(fp, "%s~\n",rdd->keyword);
	  } else { 
	    fprintf(fp, "~\n");
	  }

          if ((rdd->exit_info & DOOR_1)==DOOR_1) fprintf(fp,"1");
          else if ((rdd->exit_info & DOOR_2)==DOOR_2) fprintf(fp,"2");
          else if ((rdd->exit_info & DOOR_3)==DOOR_3) fprintf(fp,"3");
          else if ((rdd->exit_info & DOOR_4)==DOOR_4) fprintf(fp,"4");
          else if ((rdd->exit_info & DOOR_5)==DOOR_5) fprintf(fp,"5");
          else if ((rdd->exit_info & DOOR_6)==DOOR_6) fprintf(fp,"6");
          else if ((rdd->exit_info & DOOR_7)==DOOR_7) fprintf(fp,"7");
          else
	    fprintf(fp, "0");

	  fprintf(fp," %d ", 
		  rdd->key);

	  fprintf(fp,"%d\n", rdd->to_room);
       }
     }

/*
  extra descriptions..
*/

   for (exptr = rp->ex_description; exptr; exptr = exptr->next) {
     x = 0;

    if (exptr->description) {
      for (k = 0; k <= strlen(exptr->description); k++) {
       if (exptr->description[k] != 13)
	 temp[x++] = exptr->description[k];
      }
      temp[x] = '\0';

     fprintf(fp,"E\n%s~\n%s~\n", exptr->keyword, temp);
    }
   }

   fprintf(fp,"S\n");

   }

   fclose(fp);
   send_to_char(dots, ch);
   send_to_char("\nDone\n",ch);
}


void RoomLoad(struct char_data *ch,int start, int end,char *name)
{
  FILE *fp;
  int vnum, found = FALSE;
  char buf[80];
  struct room_data dummy, *rp;
  ulong hi,lo;
  
  sprintf(buf,"Area/%s",name);
  if ((fp = fopen(buf,"r")) == NULL) {
	nlog("%s",buf);
    send_to_char("You don't appear to have an area...\n",ch);
    return;
  }
  
  send_to_char("Searching and loading rooms\n",ch);
  
  fscanf(fp,"%lu %lu\n",&lo,&hi);
  if (strcmp(GET_NAME(ch),name)!=0 || start==-1) {
	start=lo;
	end=hi;
  }
  llog(SILLYLORD,"Loading rooms (%s) %lu-%lu",name,start,end);
  if (start<lo || end>hi) {
	sprintf(buf,"Unable to fulfill request: Room range is %lu-%lu",lo,hi);
	send_to_char(buf,ch);
	send_to_char("\n",ch);
	log(buf);
	return;
  }
  do {
    unsigned long p;
    char chk[129];

    p=ftell(fp);
    if (!fgets(chk,128,fp)) vnum=start;
    if (*chk=='#') vnum=atoi(chk+1);
    if (vnum>=start) fseek(fp,p,SEEK_SET);
  } while (vnum<start);

  found=0;
  while (!found && !feof(fp)) {
    if (fscanf(fp, "#%d\n",&vnum)!=EOF) {
    if ((vnum >= start) && (vnum <= end)) {
      if (vnum == end)
	found = TRUE;
      
      if (NULL==(rp=hash_find(&room_db, vnum))) {
	rp = (void*)malloc(sizeof(*rp));
	bzero(rp, sizeof(*rp));
	hash_enter(&room_db, vnum, rp);
	send_to_char("+",ch);
      } else {
	if (rp->people) {
	  act("$n reaches down and scrambles reality.", FALSE, ch, NULL,
	      rp->people, TO_ROOM);
	}
	cleanout_room(rp);
	send_to_char("-",ch);
      }
      
      rp->number = vnum;
      load_one_room(fp, rp);
      
    } else {
      send_to_char(".",ch);
      /*  read w/out loading */
      load_one_room(fp, &dummy);
      cleanout_room(&dummy);
    }
    }
  }
  fclose(fp);
  
  if (!found) {
    send_to_char("\nThe room number(s) that you specified could not all be found.\n",ch);
  } else {
    send_to_char("\nDone.\n",ch);
  }
  
}    

void fake_setup_dir(FILE *fl, int room, int dir)
{
	int tmp;
	char buf[256], *temp;

	temp = (char *)buf;

	temp =	fread_string(fl); /* descr */
	temp =	fread_string(fl); /* key */

	fscanf(fl, " %d ", &tmp); 
	fscanf(fl, " %d ", &tmp);
	fscanf(fl, " %d ", &tmp);
}


int IsHumanoid( struct char_data *ch)
{
/* these are all very arbitrary */

  switch(GET_RACE(ch))
    {
    case RACE_HUMAN:
    case RACE_GNOME:
    case RACE_ELVEN:
    case RACE_DWARF:
    case RACE_HOBBIT:
    case RACE_ORC:
    case RACE_LYCANTH:
    case RACE_UNDEAD:
    case RACE_GIANT:
    case RACE_GOBLIN:
    case RACE_DEVIL:
    case RACE_TROLL:
    case RACE_VEGMAN:
    case RACE_MFLAYER:
    case RACE_ENFAN:
    case RACE_PATRYN:
    case RACE_SARTAN:
    case RACE_H_ELF:
    case RACE_H_ORC:
    case RACE_MUTANT:
    case RACE_HALFLING:
    case RACE_OGRE:
    case RACE_PIXIE:
    case RACE_LIZARDMAN:
      return(TRUE);
      break;

    default:
      return(FALSE);
      break;
    }

}

int IsAnimal( struct char_data *ch)
{

  switch(GET_RACE(ch))
    {
    case RACE_PREDATOR:
    case RACE_FISH:
    case RACE_BIRD:
    case RACE_HERBIV:
    case RACE_REPTILE:
    case RACE_LABRAT:
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
    }

}

int IsUndead( struct char_data *ch)
{

  switch(GET_RACE(ch)) {
  case RACE_UNDEAD: 
  case RACE_GHOST:
    return(TRUE);
    break;
  default:
    return(FALSE);
    break;
  }

}

int IsLycanthrope( struct char_data *ch)
{
  switch (GET_RACE(ch)) {
  case RACE_LYCANTH:
    return(TRUE);
    break;
  default:
    return(FALSE);
    break;
  }

}

int IsDiabolic( struct char_data *ch)
{

  switch(GET_RACE(ch))
    {
    case RACE_DEMON:
    case RACE_DEVIL:
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
    }

}

int IsReptile( struct char_data *ch)
{
  switch(GET_RACE(ch)) {
    case RACE_REPTILE:
    case RACE_DRAGON:
    case RACE_DINOSAUR:
    case RACE_SNAKE:
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
    }
}

int HasHands( struct char_data *ch)
{

  if (IsHumanoid(ch))
    return(TRUE);
  if (IsUndead(ch)) 
    return(TRUE);
  if (IsLycanthrope(ch)) 
    return(TRUE);
  if (IsDiabolic(ch))
    return(TRUE);
  if (GET_RACE(ch) == RACE_DRAGON)
   return(FALSE);
  return(FALSE);
}

int IsPerson( struct char_data *ch)
{

  switch(GET_RACE(ch))
    {
    case RACE_HUMAN:
    case RACE_ELVEN:
    case RACE_DWARF:
    case RACE_HOBBIT:
    case RACE_GNOME:
      return(TRUE);
      break;

    default:
      return(FALSE);
      break;

    }

}


int IsExtraPlanar( struct char_data *ch)
{
  switch(GET_RACE(ch)) {
  case RACE_DEMON:
  case RACE_DEVIL:
  case RACE_PLANAR:
  case RACE_ELEMENT:
    return(TRUE);
    break;
  default:
    return(FALSE);
    break;
  }
}

void SetHunting( struct char_data *ch, struct char_data *tch)
{
   int persist, dist;
   char buf[256];
 
   if (!IS_NPC(ch)) return;
   persist =  GET_LEVEL(ch);
   persist *= (int) GET_ALIGNMENT(ch) / 100;

   if (persist < 0)
     persist = -persist;

   dist = GET_ALIGNMENT(tch) - GET_ALIGNMENT(ch);
   dist = (dist > 0) ? dist : -dist;
   if (Hates(ch, tch))
       dist *=2;

   SET_BIT(ch->specials.act, ACT_HUNTING);
   ch->specials.hunting = tch;
   ch->hunt_dist = dist;
   ch->persist = persist;
   ch->old_room = ch->in_room;

    if (GET_LEVEL(tch) >= IMMORTAL) {
        sprintf(buf, ">>%s is hunting you from %s\n", 
       	   ch->player.short_descr, 
       	   (real_roomp(ch->in_room))->name);
        send_to_char(buf, tch);
    }
}


void CallForGuard( struct char_data *ch, struct char_data *vict, int lev)
{
  struct char_data *i;
  if (lev == 0) lev = 3;

  if (!IS_CITIZEN(ch)) return;
  for (i = character_list; i && lev>0; i = i->next) {
      if (IS_NPC(i) && (i != ch)) {
	 if (!i->specials.fighting) {
	    if (i->virtual == 3060) {
	       if (number(1,6) == 1) {
	         if (!IS_SET(i->specials.act, ACT_HUNTING)) {
                   if (vict) {
		      SetHunting(i, vict);
                      lev--;
		    }
	         }
	       }
	    } else if (i->virtual == 3069) {
	       if (number(1,6) == 1) {
	          if (!IS_SET(i->specials.act, ACT_HUNTING)) {
		    if (vict) {
		      SetHunting(i, vict);
	              lev-=2;
		    }
		  }
		}
	     }
	  }
       }
    }
}

void StandUp (struct char_data *ch)
{
   if ((GET_POS(ch)<POSITION_STANDING) && 
       (GET_POS(ch)>POSITION_STUNNED)) {
       if (ch->points.hit > (ch->points.max_hit / 2))
         act("$n quickly stands up.", 1, ch,0,0,TO_ROOM);
       else if (ch->points.hit > (ch->points.max_hit / 6))
         act("$n slowly stands up.", 1, ch,0,0,TO_ROOM);
       else 
         act("$n gets to $s feet very slowly.", 1, ch,0,0,TO_ROOM);
       GET_POS(ch)=POSITION_STANDING;
   }
}


void FighterMove( struct char_data *ch)
{
  int num;

  num = number(1,4);
  if (num <= 2) {
      if (!get_skill(ch,SKILL_BASH))
	set_skill(ch,SKILL_BASH, 10 + GET_LEVEL(ch)*4);
      do_bash(ch, GET_NAME(ch->specials.fighting), 0);
    } else if (num == 3) {
      if (ch->equipment[WIELD]) {
         if (!get_skill(ch,SKILL_DISARM))
	   set_skill(ch,SKILL_DISARM, 10 + GET_LEVEL(ch)*4);
         do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
       } else {
         if (!get_skill(ch,SKILL_DISARM))
	   set_skill(ch,SKILL_DISARM, 60 + GET_LEVEL(ch)*4);
         do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
       }
   } else {
      if (!get_skill(ch,SKILL_KICK))
	   set_skill(ch,SKILL_KICK, 10 + GET_LEVEL(ch)*4);
      do_kick(ch, GET_NAME(ch->specials.fighting), 0);
   }
}

void DevelopHatred( struct char_data *ch, struct char_data *v)
{
   int diff, patience, var;

   if (Hates(ch, v))
     return;

  if (ch == v)
    return;

  diff = GET_ALIGNMENT(ch) - GET_ALIGNMENT(v);
  diff = (diff > 0) ? diff : -diff;
  
  diff /= 5;
  
  if (GET_MAX_HIT (ch) == 0)	/* keeps crashing here due to corpse */
	return;
  patience = (100 * GET_HIT(ch)) / GET_MAX_HIT(ch);

  var = number(1,40) - 20;

  if (patience+var < diff)
     AddHated(ch, v);
}

void event_teleport(event_t *event)
{
  struct room_data *rp, *dest;

  if(IS_NPC(event->ch) || (event->ch->in_room != event->room)){
    return;
  }
  rp = real_roomp(event->ch->in_room);
  if(rp && (rp->tele_targ > 0) && (rp->tele_targ != rp->number)){
    dest = real_roomp(rp->tele_targ);
    if(!dest){
      log("invalid tele_targ");
      return;
    }
    if(IS_SET(dest->room_flags, LORD_ONLY)){
      vlog(LOG_URGENT, "telport to LORD_ONLY room");
      return;
    }
    if(IS_SET(dest->room_flags, NO_GOTO)){
      vlog(LOG_URGENT, "telport to NO_GOTO room");
      return;
    }
    char_from_room(event->ch);
    char_to_room(event->ch, dest->number);
    if(rp->tele_look)
      do_look(event->ch, "\0",15);
    if(IS_SET(dest->room_flags, DEATH) && GET_LEVEL(event->ch) < LOW_IMMORTAL){
      death_trap(event->ch);
    }
  }
}

int HasObject( struct char_data *ch, int ob_num)
{
int j, found;
struct obj_data *i;

/*
   equipment too
*/

found = 0;

        for (j=0; j<MAX_WEAR; j++)
     	   if (ch->equipment[j])
       	     found += RecCompObjNum(ch->equipment[j], ob_num);

      if (found > 0)
	return(TRUE);

  /* carrying  */
       	for (i = ch->carrying; i; i = i->next_content)
       	  found += RecCompObjNum(i, ob_num);

     if (found > 0)
       return(TRUE);
     else
       return(FALSE);
}


int room_of_object(struct obj_data *obj)
{
  if (obj->in_room != NOWHERE)
    return obj->in_room;
  else if (obj->carried_by)
    return obj->carried_by->in_room;
  else if (obj->equipped_by)
    return obj->equipped_by->in_room;
  else if (obj->in_obj)
    return room_of_object(obj->in_obj);
  else
    return NOWHERE;
}

struct char_data *char_holding(struct obj_data *obj)
{
  if (obj->in_room != NOWHERE)
    return NULL;
  else if (obj->carried_by)
    return obj->carried_by;
  else if (obj->equipped_by)
    return obj->equipped_by;
  else if (obj->in_obj)
    return char_holding(obj->in_obj);
  else
    return NULL;
}


int RecCompObjNum( struct obj_data *o, int obj_num)
{

int total=0;
struct obj_data *i;

  if (o->virtual == obj_num)
    total = 1;

  if (ITEM_TYPE(o,ITEM_CONTAINER)) {
    for (i = o->contains; i; i = i->next_content)
      total += RecCompObjNum( i, obj_num);
  }
  return(total);

}

void RestoreChar(struct char_data *ch)
{

  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_POWER(ch) = GET_MAX_POWER(ch);
  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch); 
  if (GET_LEVEL(ch) < LOW_IMMORTAL) {
    GET_COND(ch,THIRST) = 24;
    GET_COND(ch,FULL) = 24;
  } else {
    GET_COND(ch,THIRST) = -1;
    GET_COND(ch,FULL) = -1;
  }

}


void RemAllAffects( struct char_data *ch)
{
  spell_dispel_magic(IMPLEMENTOR,ch,ch,0,IMPLEMENTOR);

}

int which_class(struct char_data *ch,char *s,int list_on_error)
{
  int i;
  for (i=0; i<MAX_DEF_CLASS; i++)
    if (LOWER(s[0])==LOWER(classes[i].abbrev[0]) &&
        LOWER(s[1])==LOWER(classes[i].abbrev[1])) 
	return(i);
  if (!list_on_error) return(-1);
  sendf(ch,"Not a recognized class!  Legal Abbreviations are:\n");
  for (i=0; i<MAX_DEF_CLASS; i++) {
    sendf(ch,"%2s - %-20s",classes[i].abbrev,classes[i].class_name);
    if ((i+1)%3==0) sendf(ch,"\n");
  }
  sendf(ch,"\n");
  return(-1);
}


int can_have(struct char_data *ch, struct obj_data *o)
{
  if (!o) return(1);
  if (o->min_level>GET_LEVEL(ch)) return(0);
  if (o->in_obj && !can_have(ch,o->next_content)) return(0);
  return(can_have(ch,o->contains));
}

char *str_str(char *p,char *q)
{
  char b1[MAX_STRING_LENGTH],b2[MAX_STRING_LENGTH];
  char *r;

  if (!p || !q) return(strstr(p,q));
  for (r=b1;*p;p++,r++)
    *r = LOWER(*p);
  *r='\0';
  for (r=b2;*q;q++,r++)
    *r = LOWER(*q);
  *r='\0';
  return(strstr(b1,b2));
}

#ifdef _SEQUENT_
int gettimeofday(struct timeval *tvp, void *ignored) {
  return(get_process_stats(tvp, PS_SELF, NULL, NULL));
}
#endif /* _SEQUENT_ */

