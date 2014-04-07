/*
** File: act.new.c , misc commands.
*/

/* Headers */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <fcns.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <spells.h>
#include <limits.h>
#include <race.h>
#include <ticks.h>
#include <externs.h>

/* Actions */

#define MAX_GRID_X 39
#define MAX_GRID_Y 11

typedef struct _point {
  int x,y;
  struct _point *next;
} point_struct;

static int rooms[MAX_GRID_X][MAX_GRID_Y];
static point_struct *point_q,*point_end;

void queue_point(int x,int y)
{
  point_struct *p;
  CREATE(p,point_struct,1);
  p->x=x; p->y=y; p->next=NULL;
  if (!point_q)
    point_q=p;
  else
    point_end->next=p;
  point_end=p;
}

void grid_expand(int x, int y,int max_x,int max_y)
{
  struct room_data *rp;
  struct room_direction_data *exitp,*backp;
  int to_room,dir;

  rp=real_roomp(rooms[x][y]);
  if (!rp) return;
  for (dir=0; dir<4; dir++) {
    exitp=rp->dir_option[dir];
    if (exitp && real_roomp(exitp->to_room)) {
      to_room=exitp->to_room;
      backp=real_roomp(to_room)->dir_option[rev_dir[dir]];
      if (!backp || backp->to_room!=rooms[x][y]) continue;
      switch(dir) {
	case 0: if (y>0 && rooms[x][y-1]==NOWHERE) {
		  rooms[x][y-1]=to_room;
		  queue_point(x,y-1);
		} break;
	case 1: if (x+1<max_x && rooms[x+1][y]==NOWHERE) {
		  rooms[x+1][y]=to_room;
		  queue_point(x+1,y);
		} break;
	case 2: if (y+1<max_y && rooms[x][y+1]==NOWHERE) {
		  rooms[x][y+1]=to_room;
		  queue_point(x,y+1);
		} break;
	case 3: if (x>0 && rooms[x-1][y]==NOWHERE) {
		  rooms[x-1][y]=to_room;
		  queue_point(x-1,y);
		} break;
      }
    }
  }
}

void do_map(struct char_data *ch, char *arg, int cmd)
{
  int i,j,k,no_rooms;
  char names[MAX_GRID_X*MAX_GRID_Y][21];
  char build[132],build1[132];
  int letters[MAX_GRID_X*MAX_GRID_Y];
  char letter;
  int type;
  int width, height;
  struct room_data *rp;
  struct room_direction_data *exitp;
  int home=0,shrt;
  char buf[MAX_INPUT_LENGTH];

  point_end=point_q=NULL; 
  shrt=0;

  if (!ROOM_FLAG(ch->in_room,MAPABLE) && !IS_IMMORTAL(ch)) {
    sendf(ch,"Not here.\n");
    return;
  }
  if (!ROOM_FLAG(ch->in_room,MAPABLE))
    CHK_PLAYER_ZONE(ch,ch->in_room);
  sendf(ch,"\n");
  arg=one_argument(arg,buf);
  if (is_abbrev(buf,"short")) shrt=1;
  for(i=0; i<MAX_GRID_X; i++)
    for (j=0; j<MAX_GRID_Y; j++)
      rooms[i][j]=NOWHERE;
  if (!IS_IMMORTAL(ch) || is_abbrev(buf,"mortal")) {
    width=5; height=5; type=1;
  } else if (shrt) {
    width=MAX_GRID_X; height=MAX_GRID_Y; type=2;
  } else {
    width=13; height=11; type=3;
  }
  rooms[(width-1)/2][(height-1)/2]=ch->in_room;
  queue_point((width-1)/2,(height-1)/2);
  do {
    int x,y;
    point_struct *p;
    p=point_q;
    x=p->x; y=p->y;
    point_q=p->next;
    if (p==point_end) point_end=p->next;
    FREE(p);
    grid_expand(x,y,width,height);
  } while(point_q);

  no_rooms=0;
  for (j=0; j<height; j++) {
    *build='\0';
    *build1='\0';
    for (i=0; i<width; i++) {
      if (rooms[i][j]!=NOWHERE) {
        rp=real_roomp(rooms[i][j]);
	names[no_rooms][20]='\0';
	names[no_rooms][19]='\0';
        strncpy(names[no_rooms],rp->name,19);
        letters[no_rooms]=rooms[i][j];
        for (k=0; k<no_rooms; k++)
	  if (letters[k]==letters[no_rooms]) break;
        if (k!=no_rooms)
	  letter=k;
	else
          letter=no_rooms++;
	k=strlen(build1);
	switch (type) {
	  case 1:
		if (i==(width-1)/2 && j==(height-1)/2) {
		  home=letter;
		  sprintf(build1+k,"*");
		} else sprintf(build1+k,"%c",letter+'A');
		break;
	  case 2: if (i==(width-1)/2 && j==(height-1)/2)
		    sprintf(build1+k,"*");
		  else
		    sprintf(build1+k,"+");
		break;
	  case 3: sprintf(buf,"%5d",rooms[i][j]);
                  for(k=0; buf[k]==' '; k++) buf[k]='0';
		  strcat(build1+strlen(build1),buf+strlen(buf)-5);
		break;
	};
        if (i+1<width) {
	  exitp=rp->dir_option[1];
	  sprintf(build1+strlen(build1),"%c",
		(exitp && exitp->to_room==rooms[i+1][j])?'-':' ');
        }
        if (j+1<height) {
          exitp=rp->dir_option[2];
	  if (type==3)
	    strcat(build,(exitp && exitp->to_room==rooms[i][j+1])?
				"  |   ":"      ");
	  else
	    strcat(build,(exitp && exitp->to_room==rooms[i][j+1])?"| ":"  ");
        }
      } else {
	if (type==3) {
	  strcat(build1,"      ");
	  strcat(build,"      ");
	} else {
	  strcat(build1,"  ");
	  strcat(build,"  ");
	}
      }
    }
    sendf(ch,"%s\n%s\n",build1,build);
  }
  if (type==1 && !shrt)
    for (i=0; i<no_rooms; i++)
      sendf(ch,"%c=%-20s%s",i+1==home?'*':i+'A',names[i],(i+1)%3?"  ":"\n");
}

void do_priority(struct char_data *ch, char *arg, int cmd)
{
  if(IS_NPC(ch)) return;

  if(IS_SET(ch->specials.act, PLR_PRIORITY)){
    ch->specials.act &= ~PLR_PRIORITY;
    sendf(ch, "Commands will not be ordered by priority.\n");
  }
  else{
    ch->specials.act |= PLR_PRIORITY;
    sendf(ch, "Commands will be ordered by priority.\n");
  }
}

void do_away(struct char_data *ch, char *arg, int cmd)
{
  if(IS_NPC(ch)) return;

  for(; isspace(*arg); arg++);

  if(!*arg && !IS_AWAY(ch)){
    sendf(ch, "Please specify an away message.\n");
    return;
  }
  if(*arg && IS_AWAY(ch)){
    FREE(ch->specials.away);
    ch->specials.away = mystrdup(arg);
    sendf(ch, "Away message changed.\n");
    return;
  }
  if(*arg && !IS_AWAY(ch)){
    ch->specials.away = mystrdup(arg);
    sendf(ch, "You are now marked as away.\n");
    return;
  }
  else{
    FREE(ch->specials.away);
    sendf(ch, "You are no longer marked as away.\n");
    return;
  }
}

void do_enable(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int i, cmd_no;

  if(IS_NPC(ch)) return;

  one_argument(arg,buf);

  if(!*buf){
    sendf(ch, "Enabled commands:\n");
    for(i = 0; *cmd_info[i].cmd != '\n'; i++){
      if(!IS_SET(cmd_info[i].flags, CMDF_DISABLED)){
        sendf(ch, "   %s\n", cmd_info[i].cmd);
      }
    }
    return;
  }

  cmd_no = find_command(buf);
  if(cmd_no < 0){
    sendf(ch, "Unknown command.\n");
    return;
  }

  if(!IS_SET(cmd_info[cmd_no].flags, CMDF_DISABLED)){
    sendf(ch, "That command is already enabled.\n");
    return;
  }

  cmd_info[cmd_no].flags &= ~CMDF_DISABLED;
  sendf(ch, "Command enabled.\n");
  vlog(LOG_MISC, "Command \"%s\" enabled.\n", cmd_info[cmd_no].cmd);
}

void do_disable(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int i, cmd_no;

  if(IS_NPC(ch)) return;

  one_argument(arg,buf);

  if(!*buf){
    sendf(ch, "Disabled commands:\n");
    for(i = 0; *cmd_info[i].cmd != '\n'; i++){
      if(IS_SET(cmd_info[i].flags, CMDF_DISABLED)){
        sendf(ch, "   %s\n", cmd_info[i].cmd);
      }
    }
    return;
  }

  cmd_no = find_command(buf);
  if(cmd_no < 0){
    sendf(ch, "Unknown command.\n");
    return;
  }

  if(IS_SET(cmd_info[cmd_no].flags, CMDF_DISABLED)){
    sendf(ch, "That command is already disabled.\n");
    return;
  }
  else if(IS_SET(cmd_info[cmd_no].flags, CMDF_NODISABLE)){
    sendf(ch, "That command can not be disabled.\n");
    return;
  }

  cmd_info[cmd_no].flags |= CMDF_DISABLED;
  sendf(ch, "Command disabled.\n");
  vlog(LOG_MISC, "Command \"%s\" disabled.\n", cmd_info[cmd_no].cmd);
}

int chaos_lo=0,chaos_hi=0;
extern int arena_pot;
extern int arena_cost_per_level;
extern int arena_ticks_till_start;

void do_arena(struct char_data *ch, char *arg, int cmd)
{
  struct affected_type *a;
  if (arena_ticks_till_start<=0) {
	sendf(ch,"The killing fields are currently closed to new people.\n");
	return;
  }
  if (IS_NPC(ch)) return;
  if (GetMaxLevel(ch)<chaos_lo || GetMaxLevel(ch)>chaos_hi) {
    sendf(ch,"The current match is for %d to %d levels only.\n",
		chaos_lo,chaos_hi);
    return;
  }
  if (ROOM_FLAG(ch->in_room,ARENA)) {
    sendf(ch,"You are already in the arena.\n");
    return;
  }
  sendf(ch,"It will cost you %d gold to join.\n",
	arena_cost_per_level*GET_LEVEL(ch));
  if (GET_GOLD(ch)<arena_cost_per_level*GET_LEVEL(ch)) {
    sendf(ch,"You don't have enought money.\n");
    return;
  }
  GET_GOLD(ch) -= (arena_cost_per_level*GET_LEVEL(ch));
  arena_pot += (arena_cost_per_level*GET_LEVEL(ch));
  act("$n has been whisked off to the arena.\n",TRUE,ch,0,0,TO_ROOM);
  do_restore(ch,"",-1);
  for(a=ch->affected; a; a=a->next)
    affect_remove(ch,a);
  char_from_room(ch);
  char_to_room(ch,900+number(0,63));
  do_look(ch,"",15);
  sports_cast("%s has joined the blood bath.",GET_NAME(ch));
}

void do_puke(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];

  one_argument(argument,buf);
  if (!str_cmp(buf,"all")) {
     act("$n pukes all over the place.  Gross.\n", TRUE, ch, 0, 0, TO_ROOM);
     sendf(ch,"You empty your stomach.\n");
     if (IS_IMMORTAL(ch)) {
       GET_COND(ch,FULL)= -1;
       GET_COND(ch,THIRST)= -1;
       GET_COND(ch,DRUNK)= -1;
     } else {
       GET_COND(ch,FULL)=0;
       GET_COND(ch,THIRST)=0;
     }
  } else do_action(ch,argument,cmd);
}

void do_sing(struct char_data *ch, char *argument, int cmd)
{
   if(!IsClass(ch,BARD)){
      do_action(ch,argument,129); /* Sing */
      return;
   }

   if(*argument)
      do_cast(ch,argument,129);
   else {
      sendf(ch,"You raise your melodious voice to the heavens.\n");
      act("$n sings a battle hymn of old.\n",1,ch,0,0,TO_ROOM);
   }
}

void do_slice(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *slice, *corpse;
  char vict[64],buf[128];

  while(isspace(*argument))
     argument++;
  
  if(!*argument || strncmp(argument,"corpse",6)) {
    sendf(ch,"Syntax: slice [corpse | 2.corpse | ...]\n");
    return;
  }

  corpse = (struct obj_data *)get_obj_vis_accessible(ch, argument);

  if(!corpse) {
    sendf(ch,"You can't carve the corpse if it isn't here!\n");
    return;
  }

  strcpy(vict,corpse->name+7); /* Remove the "corpse " */

  if(corpse->obj_flags.value[2] < 6) {  /*  value 2 set in make_corpse */
     sendf(ch,"There's not much left of that corpse...certainly nothing edible.\n");
     sprintf(buf,"$n picks through the remains of %s.\n",vict);
     act(buf, TRUE, ch, 0, 0, TO_ROOM);
     return;
  }

  CREATE(slice, struct obj_data, 1);
  clear_object(slice);
  
  slice->in_room = NOWHERE;
  
  sprintf(buf, "chunk slice %s",vict);
  slice->name = mystrdup(buf);
    
  sprintf(buf, "A chunk of %s is lying here.", vict);
  slice->description = mystrdup(buf);
    
  sprintf(buf, "A slice of %s",vict);
  slice->short_description = mystrdup(buf);
    
  slice->contains = 0;
  
  slice->obj_flags.type_flag = ITEM_FOOD;
  slice->obj_flags.wear_flags = ITEM_TAKE;
  slice->obj_flags.value[0] = 10; /* Fills you for 10 hours */
  slice->obj_flags.value[1] = 0; /* Not used */
  slice->obj_flags.value[2] = 0; /* Not used */
  slice->obj_flags.value[3] = 0; /* Not poisoned */
  slice->obj_flags.weight = 5;
  corpse->obj_flags.value[2] -= 6; /*  value 2 set in make_corpse */
  slice->obj_flags.cost_per_day = 100;
  
  slice->carried_by = ch;
  slice->equipped_by = 0;
  
  obj_to_char(slice, ch);

  sendf(ch,"You carve a big, healthy chunk out of %s.\n",vict);
  sprintf(buf,"$n carves a slice out of the corpse of %s.\n",vict);
  act(buf, TRUE, ch, 0, 0, TO_ROOM);
}

char *Nice_Name(struct char_data *seer, struct char_data *ch)
{
  static char buffer[100];

	strcpy (buffer, PERS (ch, seer));
	/*
  if (!IS_NPC(ch)) {
    strcpy(buffer,GET_NAME(ch));
  } else {
    strcpy(buffer, ch->player.short_descr);
    CAP(buffer);
  }
							  
#define PERS(ch, vict)   (CAN_SEE(vict, ch) ?
  (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr) : "someone")

  */
  CAP (buffer);
  return(buffer);
}

int in_group_and_room(struct char_data *ch1,struct char_data *ch2)
{
  return(in_group(ch1,ch2) && ch1->in_room==ch2->in_room &&
   IS_AFFECTED(ch1,AFF_GROUP) && IS_AFFECTED(ch2,AFF_GROUP));
}

int num_in_group_and_room(struct char_data *ch)
{
  struct char_data *leader;
  struct follow_type *f;
  int count;

  if (!ch->master) leader=ch;
  else leader=ch->master;

  count=1;
  if (leader->followers)
  for (f=leader->followers; f; f=f->next)
    if (in_group_and_room(leader,f->follower)) count++;
  return(count);
}

void do_split(struct char_data *ch, char *arg, int cmd)
{
  slong amount;
  slong ngroup;
  slong share,total;
  struct follow_type *k;
  struct char_data *p;

  for (;*arg && isspace(*arg);arg++);
  if (!*arg) {
    send_to_char("Usage: split <# of coins>\nExample: Split 1000\n",ch);
    return;
  }
  total=amount=atoi(arg);
  if (amount>GET_GOLD(ch)) {
    sendf(ch,"But you only have %d!\n",GET_GOLD(ch));
    return;
  } else if (amount <0) {
    send_to_char("Get real.\n",ch);
    return;
  }
  ngroup=num_in_group_and_room(ch);
  if (ngroup<2) {
    send_to_char("No one in this room is in your group!\n",ch);
    return;
  }
  share = amount/ngroup;
  if (share<=0) {
    send_to_char("Try dividing up a little bit more!\n",ch);
    return;
  }
  GET_GOLD(ch)-=amount;
  act("$n divides up some gold.\n",TRUE,ch,0,0,TO_ROOM);
  p=(ch->master?ch->master:ch);
  sendf(ch,"You give %d coins to:\n",share);
  for (k=p->followers; k; k=k->next)
    if (in_group_and_room(k->follower,ch) && k->follower!=ch) {
      sendf(k->follower,"%s divides up %d coins.  Your share is %d.\n",
		GET_NAME(ch),total,share);
      sendf(ch,"    %s\n",GET_NAME(k->follower));
      GET_GOLD(k->follower) += share;
      amount -= share;
    }
  sendf(ch,"and keep %d for yourself.\n",amount);
  GET_GOLD(ch) += amount;
}

/* Scan for mobs and people */
void do_scan(struct char_data *ch, char *argument, int cmd)
{
   struct room_data *home_room;
   struct room_data *next_room, *room;
   struct char_data *people;
   int scan_dis, scan_dir, scan_max = 3;
   int percent;
   int bonus=0;
   int count;
   int range;

   char *direction[6] = {
      "to the north",
      "to the east",
      "to the south",
      "to the west",
      "above your head",
      "below this room",
   };
   char *distance[] = {
      "A tiny bit ",
      "A small distance ",
      "A small distance ",
      "A good ways ",
   };

   home_room = real_roomp(ch->in_room);
   count=0;

   if (ROOM_FLAG(ch->in_room,ARENA)) {
	sendf(ch,"Not here.\n");
	return;
   }
   send_to_char("Scanning for life...\n",ch);
   act("$n furiously looks around.",FALSE,ch,0,0,TO_ROOM);

   for(people = home_room->people; people; people=people->next_in_room)
     if (CAN_SEE(ch,people) && ch!=people) {
	  count++;
	  sendf(ch," %20s - Right here.\n",Nice_Name(ch, people));
     }
   scan_max = 2;
   range=1;
   if (IsClass(ch,DRUID) || IsClass(ch,RANGER)) {
	range++;
	bonus += 10;
    }
   if (GET_RACE(ch)==RACE_ELVEN)
	bonus += 5;
	if (get_skill (ch, SKILL_SCAN))
		bonus += get_skill (ch, SKILL_SCAN) / 10 + 1;

   scan_max = range;

   for(scan_dir = 0; scan_dir < 6; scan_dir++) {
      for( room=home_room, scan_dis = 0; scan_dis < scan_max; room=next_room) {
	if (!room->dir_option[scan_dir]) break;
	if (room->dir_option[scan_dir]->to_room <= 0) break;
	if (IS_SET(room->dir_option[scan_dir]->exit_info, EX_CLOSED)) break;
	if (!(next_room=real_roomp(room->dir_option[scan_dir]->to_room))) break;
	percent = (60 + bonus) / (scan_dis + 1);
	if (next_room != home_room && next_room->people)
	  for(people=next_room->people; people; people=people->next_in_room)
	    if (CAN_SEE(ch,people) && number(1,100)<percent) {
	      count++;
	      sendf(ch," %20s - %s%s.\n",Nice_Name(ch, people),
		 distance[scan_dis],direction[scan_dir]);
	    }
	scan_dis++;
      } 
   }
  if (!count) send_to_char("You don't see anyone around.\n",ch);
}




/*
 * Throw an item in combat 
 *
 */

void do_throw(struct char_data *ch, char *argument, int cmd)
{
  char arg[80];
  char arg2[80];
  int num;
  struct char_data *victim;
  
  if (check_peaceful(ch,"You feel too peaceful to contemplate violence.\n"))
    return;

  argument_interpreter(argument, arg, arg2);
  
  if (*arg) {
    victim = get_char_room_vis(ch, arg);
    if (victim) {
      if (victim == ch) {
	send_to_char("You hit yourself..OUCH!.\n", ch);
	act("$n hits $mself, and says OUCH!", FALSE, ch, 0, victim, TO_ROOM);
      } else {
	if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
	  act("$N is just such a good friend, you simply can't hit $M.",
	      FALSE, ch,0,victim,TO_CHAR);
	  return;
	}
	if ((GET_POS(ch)==POSITION_STANDING) &&
	    (victim != ch->specials.fighting)) {
           if(!arg2) {
               ch->points.aimloc = LOCATION_BODY;
	      hit(ch, victim, TYPE_UNDEFINED);
	      WAIT_STATE(ch, PULSE_VIOLENCE+2);
           } else {
              switch(*arg2) {
                case 'f': /* Feet */
                case 'c': /* Claws */
                case 't': /* Tail */
                    num = 1;
                break;
                case 'l': /* Legs */
                    num = 2;
                break;
                case 'a': /* Arms */
                case 'w': /* Wings */
                    num = 3;
                break;
                case 'h': /* Head */
                    num = 5;
                break;
                default:
                    num = 4;
                break;
              }
              ch->points.aimloc = num;
	      hit(ch, victim, TYPE_UNDEFINED);
	      WAIT_STATE(ch, PULSE_VIOLENCE+2);
           }
	} else {
	  send_to_char("You do the best you can!\n",ch);
	}
      }
    } else {
      send_to_char("They aren't here.\n", ch);
    }
  } else {
    send_to_char("Hit who?\n", ch);
  }
}



/* 
 * Turn the pages in books
 *
 */
void do_turn(struct char_data *ch, char *argument, int cmd)
{

    char buf[128];
    char *book, *arg;
    int page;         /* the page looking for */
    struct char_data *dummy;
    struct obj_data *obj_object;  /* the potential book          */
    struct extra_descr_data *extra;


    if(!(book=MALLOC(128,char)))
      log("Error: out of memory (act.informative.c do_turn)");

    if(!argument) {
      send_to_char("Syntax: turn to page <number> in <book>\n",ch);
      return;
    }
         
    last_argument(argument,book);
    if(*book && generic_find(book, FIND_OBJ_ROOM | FIND_OBJ_EQUIP | 
        FIND_OBJ_INV | FIND_CHAR_ROOM, ch, &dummy, &obj_object)) {
  
         if(GET_ITEM_TYPE(obj_object) != ITEM_OTHER) {
            sprintf(buf,"You cannot turn to a page in the %s.\n",book);
            send_to_char(buf,ch);
         }

         arg = strpbrk(argument,"0123456789"); /* pull out the page number */
         if(!arg) {
           send_to_char("Syntax: turn to page <number> in <book>\n",ch);
           return;
         }

         if(*(arg-1) == '-')
                 --arg;

         page = atoi(arg);

         for(extra = obj_object->ex_description; extra;
             extra = extra->next) {
             if(atoi(extra->keyword)==page)
               break;
         }
  
         if(!extra) {
            sprintf(buf,"The book doesn't have a page %d.\n",page);
         } else {
            obj_object->obj_flags.value[0] = page;
            sprintf(buf,"You turn to page %d in %s.\n",page,
                    obj_object->short_description);
            act("$n flips through $p",FALSE,ch,obj_object,0,TO_ROOM);
         }
         send_to_char(buf,ch);
         return;
      } else {
         if(*book) {
             sprintf(buf,"You do not see a '%s'.\n",book);
             send_to_char(buf,ch);
         } else {
             send_to_char("Syntax: turn to page <number> in <book>\n",ch);
         }
      }
}

/*
** Set PC's "aggressivity" so that PC will attack aggressive monsters
** instanteously when s/he enters a room.
*/
void do_aggr(struct char_data *ch, char *arg, int cmd)
{
  char	buff[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    return;

  one_argument(arg, buff);

  if (*buff == '\0') {		/* Check Aggressivity */

    if (ch->aggressive == -1) {
      send_to_char("You are not aggressive to monsters\n", ch);
      return;
    }

    sprintf(buff,
	    "You will be aggressive unless hp < %d\n", ch->aggressive);

    send_to_char(buff, ch);
    return;
  }
  else if (!strcmp(buff, "off")) { 	/* Turn off aggressivity */

    ch->aggressive = -1;
    send_to_char("You are no longer aggressive to monsters.\n", ch);
    return;
  }
  else {			/* Turn on aggressivity */

    int	hp = atoi(buff);

    ch->aggressive = (short) hp;
    send_to_char("OK.\n", ch);
    return;
  }
}


/*
** Same as "say", except that it only sends what is said
** to people in your group, and also works if group members
** are in different rooms (sort of like conference).
*/
void do_gsay(struct char_data *ch, char *arg, int cmd)
{
  struct follow_type	*follow;
  struct char_data	*master;
  char			mesg[MAX_STRING_LENGTH];
  int			i;

  if (IS_SET(ch->specials.act, PLR_STUPID)) {  
     sendf(ch,"Stop being stupid or go away.\n");
     return;                                   
   }

  for (i = 0; arg[i] == ' '; i++);
  
  if (arg[i] == '\0') {
    send_to_char("Yes, but WHAT do you want to gsay?\n", ch);
    return;
  }

  if (!IS_AFFECTED(ch, AFF_GROUP)) {
    send_to_char("But you are a member of no group?!\n", ch);
    return;
  }

  master = (ch->master) ? ch->master : ch;
  sprintf(mesg, "$n group-says '%s'", arg + i);
  send_to_char("Ok.\n", ch);

  if (ch != master) {
    act(mesg, FALSE, ch, 0, master, TO_VICT);
  }

  for (follow = master->followers; follow; follow = follow->next) {
    if (IS_AFFECTED(follow->follower, AFF_GROUP)) {
      act(mesg, FALSE, ch, 0, follow->follower, TO_VICT);
    }
  }
}


/*
** Give consent to another player.
*/
void do_consent(struct char_data *ch, char *argument, int cmd)
{
  char			arg[MAX_STRING_LENGTH];
  struct char_data	*target;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("You no longer feel generous and revoke your consent\n", 
		 ch);
    ch->consent = NULL;
    return;
  }

  if ((target = get_char_vis(ch, arg)) == NULL) {
    send_to_char("No one by that name here ....\n", ch);
    return;
  }

  if (IS_NPC(target)) {
    send_to_char("No one by that name here ....\n", ch);
    return;
  }

  if (ch->consent) {
    sprintf(arg, 
	    "You no longer give consent to %s.\n", GET_NAME(ch->consent));
    send_to_char(arg, ch);
  }

  sprintf(arg, "You now give consent to %s\n", GET_NAME(target));
  send_to_char(arg, ch);
  ch->consent = target;

  act("$n has just given you $s consent.\n", 
      FALSE, ch, 0, target, TO_VICT);
}


void do_report(struct char_data *ch, char *argument, int cmd)
{
  int  i;
  char info[160];
  char buf[MAX_STRING_LENGTH];

  if (IS_SET(ch->specials.act, PLR_STUPID)) {  
     sendf(ch,"Stop being stupid or go away.\n");
     return;                                   
   }

  sprintf(info, "%d/%dH %d/%dM %d/%dP %d/%dV", GET_HIT(ch), GET_MAX_HIT(ch),
	  GET_MANA(ch), GET_MAX_MANA(ch), GET_POWER(ch), GET_MAX_POWER(ch),
	  GET_MOVE(ch), GET_MAX_MOVE(ch));

  for (i = 0; *(argument + i) == ' '; i++)
    ;

  if (!*(argument + i)) {
    sprintf(buf, "$n reports '%s'", info);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("Ok.\n", ch);
  } else {
    sprintf(buf, "$n reports '%s. %s'", info, argument + i);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("Ok.\n", ch);
  }
}


void do_gamble(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
  struct obj_data *temp;
  int wheel1, wheel2, wheel3, wheel4, money_in;

  one_argument(argument, arg);

  money_in = atoi(arg);
/* */
  if(!(temp=get_obj_in_list_vis(ch, "casino", real_roomp(ch->in_room)->contents))) {
    act("You can't gamble here!", FALSE, ch, 0, 0, TO_CHAR);
    act("$n doesn't realize that he can't gamble here!",FALSE,ch,0,0,TO_ROOM);
    return;
  }
/* */
  if (GET_GOLD(ch) < atoi(arg)) {
    send_to_char("You do not have enough money, you rascal!\n", ch);
    return;
  }

  wheel1 = dice(1,10);
  wheel2 = dice(1,10);
  wheel3 = dice(1,10);
  wheel4 = dice(1,10);

  if (wheel1 == wheel2 && wheel3 == wheel4 && wheel2 == wheel3) {
    if (wheel1 == 5) {
      sprintf(buf, "Congrats! You have won the grand Jackpot -- %d coins.\n",
	      1000 * money_in);
      send_to_char(buf, ch);
      act("$n has just won the grand jackpot!", TRUE, ch, 0, 0, TO_ROOM);
      GET_GOLD(ch) += 1000 * money_in;
    } else {
      sprintf(buf, "Congrats! You have won the mini-Jackpot -- %d coins.\n",
	      100 * money_in);
      send_to_char(buf, ch);
      GET_GOLD(ch) += 100 * money_in;
      return;
    }
  } else if (wheel1 == 2 || wheel2 == 4 || wheel3 == 6) {
    sprintf(buf, "Congrats! You have won the micro-Jackpot -- %d coins.\n",
	    2 * money_in);
    send_to_char(buf, ch);
    GET_GOLD(ch) += 2 * money_in;
  } else {
    send_to_char("Sorry, Better luck next time, dude.\n", ch);
    GET_GOLD(ch) -= money_in;
    return;
  }
}


#if 0
 void do_whet(struct char_data *ch, char *argument, int cmd)
{
  int i;
  char arg[MAX_STRING_LENGTH];
  struct obj_data *weapon, *t;

  one_argument(argument, arg);

  if(!(t = get_obj_in_list_vis(ch, "whetstone", world[ch->in_room].contents)))
    {
      act("You can't whet it here!", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }

  if (ch->equipment[WIELD] != NULL) {
    if (CAN_SEE_OBJ(ch, ch->equipment[WIELD])) {
      if (ch->equipment[WIELD]->obj_flags.wear_n_tear <
	  (ch->equipment[WIELD]->obj_flags.cost_per_day * 500)) {
	i = number(GET_STR(ch), 2 * GET_STR(ch));
	if (GET_MOVE(ch) < i) {
	  send_to_char("You are too tired to whet your weapon!\n", ch);
	  return;
	} else {
	  send_to_char("You strain, but succeed in whetting a bit.\n", ch);
	  GET_MOVE(ch) -= i;
	  ch->equipment[WIELD]->obj_flags.wear_n_tear += (2 * i);
	}
      } else {
	send_to_char("Your weapon is already fully whetted!\n", ch);
	GET_MOVE(ch) -= i;
      }
    } else {
      send_to_char("You cannot whet what you cannot see!\n", ch);
    }
  } else {
    send_to_char("You need to be wielding your weapon to whet it.\n", ch);
  }
}
#endif
  

void do_polish(struct char_data *ch, char *argument, int cmd)
{
  return;
}

/*
** Thief skill "dodge"
*/
void do_dodge(struct char_data *ch, char *argument, int cmd)
{

  GET_MOVE(ch) -= 1;

  if (GET_POS(ch) != POSITION_FIGHTING) {
    send_to_char("But you are not fighting anyone?!\n", ch);
    return;
  }

  /* Get here iff. can dodge */

#if 0
  GET_DODGE(ch) = 1;
#endif
  WAIT_STATE(ch, PULSE_VIOLENCE * 2 + 1);
}


/*
** Warrior skill "hitall"
*/
void do_hitall(struct char_data *ch, char *arg, int cmd)
{
  byte		    percent;
  int		    count, hit_all;
  struct char_data  *mob, *next_mob;
  char		    buff[MAX_STRING_LENGTH];

  if (check_peaceful(ch,"You feel ashamed to try to disrrupt the tranquility"
		 " of this place.\n"))
    return;

  /* Find out whether to hit "all" or just aggressive */
  /* monsters */

  if (get_skill(ch,SKILL_HITALL)<=0) {
    act("$n makes a fool of himself, thinking he is proficient in hitall.\n",
	FALSE,ch,0,0,TO_ROOM);
    send_to_char("Maybe when you learn it!\n",ch);
  }
  one_argument(arg, buff);
  hit_all = !strcmp(buff, "all");

  /* Hit all aggressive monsters in room */

  count = 0;

  for (mob = real_roomp(ch->in_room)->people;
       mob;
       mob = next_mob) {

    next_mob = mob->next_in_room;

    if (!CAN_SEE(ch, mob))
      continue;

    GET_MOVE(ch) -= 1;

    percent = number(1, 131);

    if (IS_NPC(mob)) {

      if (!hit_all && !IS_SET(mob->specials.act,ACT_AGGRESSIVE))
	continue;

      if (get_skill(ch,SKILL_HITALL)>= percent)
	hit(ch, mob, TYPE_UNDEFINED);

      count++;

      if (count == 1)
	continue;
    }
  }


  WAIT_STATE(ch, PULSE_VIOLENCE * count);
}

/** Warrior's skill berserk */
void do_berserk(struct char_data *ch, char *arg, int cmd)
{
  byte			percent;
  int			type;	/* 0 for AC, 1 for HITROLL, 2 for DAMROLL */

  if (IS_NPC(ch)) {
    return;
  }

  if (IS_AFFECTED(ch,AFF_BERSERK)) {
    send_to_char("But you are already BERSERK?!\n", ch);
    return;
  }

  if (IS_AFFECTED(ch,AFF_BERRECV)) {
    send_to_char("Cannot be berserk until you have recovered!\n", ch);
    return;
  }

  type = number(0, 2);		/* No option allowed */

  /* Check against how learn */

  percent = number(1, 131);

  if (get_skill(ch,SKILL_BERSERK)+ GET_WIS(ch) < percent) {

    send_to_char("You fail to become berserk ...\n", ch);
    send_to_char("What's more, your futile attempt makes you tired.\n", ch);

    berserk_setRecover(ch, 10);

    return;
  }

  /* Otherwise, successful */

  send_to_char("You feel a sudden surge of energy!!!!!\n", ch);
  send_to_char("You are now feeling BERSERK!!!! KILL!!!!\n", ch);

  berserk_setBerserk(ch, type, 35 - (GetMaxLevel(ch) >> 1));

  berserk_setRecover(ch, 40 - (GetMaxLevel(ch) >> 1));
}


/*Command to prevent 1 pc from killing another pc */
void do_nokill(struct char_data *ch, char *arg, int cmd)
{
  char	buff[MAX_STRING_LENGTH];

  one_argument(arg, buff);

  if (IS_NPC(ch)) 
    return;

  if (*buff == '\0') {
    send_to_char("Syntax:  nokill true or false\n", ch);
    return;
  }

  if (!str_cmp(buff, "false")) {
    send_to_char("You are now free to attack other players!\n", ch);
    /*GET_NOKILL(ch) = 0;*/
  }
  else if (!str_cmp(buff, "true")) {
    send_to_char("You no longer will accidentally attack another player\n",
		 ch);
    /*GET_NOKILL(ch) = 1;*/
  }
  else {
    send_to_char("Syntax:  nokill true or false\n", ch);
    return;
  }
}

void do_aid(struct char_data *ch, char *arg, int cmd)
{
  int heal;
  char name[MAX_STRING_LENGTH];
  struct affected_type af;
  struct char_data *vict;

  one_argument(arg, name);
  if(!*name){
    vict = ch;
  }
  else if(!(vict = get_char_room_vis(ch, name))){
    sendf(ch, "You don't see that person here.\n");
    return;
  }

  if(affected_by_spell(vict, SKILL_AID) ||
      (GET_HIT(vict) > 0)){
    if(ch == vict){
      act("Your wounds aren't bleeding.", FALSE, ch, NULL, NULL, TO_CHAR);
    }
    else{
      act("$N's wounds aren't bleeding.", FALSE, ch, NULL, vict, TO_CHAR);
    }
    return;
  }

  heal = number(1, 8);
  GET_HIT(vict) = MIN(GET_MAX_HIT(vict), GET_HIT(vict) + heal);
  update_pos(vict);

  af.type = SKILL_AID;
  af.duration = 10;
  af.location = APPLY_NONE;
  init_bits(af.bitvector);
  affect_to_char(vict, &af);

  if(vict == ch){
    if(!heal){
      sendf(ch, "You try to bandage up your wounds, but to no avail.\n");
    }
    else{
      sendf(ch, "You bandage up your wounds.\n");
    }
  }
  else{
    if(!heal){
      act("You try to bandage $N's wounds, but to no avail.\n", FALSE, ch,
	NULL, vict, TO_CHAR);
      act("$n tries to bandage your wounds, but to no avail.\n", FALSE, ch,
	NULL, vict, TO_VICT);
      act("$n tries to bandage $N's wounds, but to no avail.\n", FALSE, ch,
	NULL, vict, TO_NOTVICT);
    }
    else{
      act("You bandage $N's wounds for $M.", FALSE, ch, NULL, vict, TO_CHAR);
      act("$n bandages your wounds for you.", FALSE, ch, NULL, vict, TO_VICT);
      act("$n bandages $N's wounds for $M.", FALSE, ch, NULL, vict,
        TO_NOTVICT);
    }
  }
}

/*
** Thief's skill subterfuge.  Basically, confuse NPC's with
** memories as to erase their entire memory.
*/
#if 0
void do_subterfuge(struct char_data *ch, char *arg, int cmd)
{
  struct char_data	*npc;	/* Which NPC */
  char			name[MAX_STRING_LENGTH];
  byte			percent;


  if (GET_POS(ch) == POSITION_FIGHTING) {
    send_to_char("No way!! You simply cannot concentrate!\n", ch);
    return;
  }

  if (GET_MOVE(ch) < 20) {
    send_to_char("You don't have enough mp to make the move\n", ch);
    return;
  }

  /* Get here iff. no restriction on "subterfuge" */

  one_argument(arg, name);
  if (!(npc = get_char_room_vis(ch, name))) {
    send_to_char("But who is your victim\n", ch);
    return;
  }


  if (!IS_NPC(npc) || npc->memory == NULL || 
      !IS_SET(npc->specials.act, ACT_MEMORY)) {

    send_to_char("But s/he simply does not have the intellect!\n", ch);
    return;
  }

  /*
  ** OK, now mobile has memory, we determine if it succeeds
  ** by how learn the person is at skill, and the level
  ** difference.
  */

  percent = number(1, 101);

  if (get_skill(ch,SKILL_SUBTERFUGE)+ GET_INT(ch) < percent) {
    send_to_char("You simplely failed to confuse your target\n", ch);
  }
  else if ((GetMaxLevel(npc) - GetMaxLevel(ch)) * 5 > number(1, 101)) {
    send_to_char("His/her mind is too clear to be confused\n", ch);
  }
  else {
    send_to_char("All right.. you have fooled the stupid NPC\n", ch);
    /*mem_destroy((Memory *) npc->memory);
    npc->memory = (void *) mem_create(16);*/
  }

  /* Now adjust movement */

  GET_MOVE(ch) -= 20;
  GET_MOVE(ch) = MAX(0, GET_MOVE(ch));
}
#endif


/* Private Interface */

void berserk_setRecover(struct char_data *ch, int how_long)
{
  struct affected_type	af;

  af.type 	= SKILL_BERSERK;
  af.duration	= how_long;
  af.modifier	= 0;
  af.location	= APPLY_NONE;
  init_set(af.bitvector,AFF_BERRECV);

  affect_to_char(ch, &af);
}


void berserk_setBerserk(struct char_data *ch, int type, int how_long)
{
#define BERSERK_AC_AFFECT	(-50)
  /* Modify AC by this much */

  struct affected_type	af;

  af.type	= SKILL_BERSERK;
  af.duration	= how_long;
  init_set(af.bitvector,AFF_BERSERK);

  switch (type) {
  case 0:			/* AC */
    af.modifier = BERSERK_AC_AFFECT;
    af.location	= APPLY_ALL_AC;
    break;

  case 1:			/* Hit roll */
    af.modifier	= MIN(127, (int) (GET_HITROLL(ch)));
    af.location	= APPLY_HITROLL;
    break;

  case 2:
    af.modifier	= MIN(127, (int) (GET_DAMROLL(ch)));
    af.location	= APPLY_DAMROLL;
    break;
  }

  affect_to_char(ch, &af);

#undef BERSERK_AC_AFFECT
}


#define TRUE 1
#define FALSE 0


#define Multiplier 1023

static int Hash(char *s)
{
    register char *cp;
    long hashcode;

    hashcode = 0;
    for (cp = s; *cp; cp++) hashcode = hashcode * Multiplier + *cp;
    return ((int) (hashcode & 0x7FFF));
}

 /* rescue a player who bought insurance */
void do_save_player(struct char_data *ch)
 {
 struct obj_data *tmp_object;

 if (IN_ARENA(ch)) return;
 tmp_object = get_obj_in_list_vis(ch, "insurance_token", ch->carrying);
 if (tmp_object) { 
  obj_from_char(tmp_object);
  extract_obj(tmp_object); 
  act("An Insurance agent rushes into the room and screams 'I must save $n!'", FALSE, ch, 0, 0, TO_ROOM);
  act("The Insurance agent points at $n", FALSE, ch, 0, 0, TO_ROOM);
  sendf(ch,"An Insurance agent rushes into the room and screams 'I must save you!'\n");
  vlog(LOG_MISC|LOG_DEATH,"%s saved by insurance in room %s [%d] (cost=%d)", GET_NAME(ch),(real_roomp(ch->in_room))->name,ch->in_room,deductable(ch,0));
  spell_word_of_recall(IMPLEMENTOR, ch, ch, 0,IMPLEMENTOR);
  sendf(ch,"Your deductable comes to %d.\n",deductable(ch,0));
  fine(ch,deductable(ch,0),InsuranceGuy());
  }
 }

void do_appraise(struct char_data *ch, char *argument, int cmd)
{
   
   struct obj_data *object;
   int percent, i;
   char name[128],hashstring[128];

   one_argument(argument, name);

   if(!name) {
   send_to_char("What do you want to appraise?\n",ch);
   return;
   }

   if(!(object = get_obj_in_list_vis(ch,name,ch->carrying))) {
   send_to_char("You can't appraise an item that you don't have.\n", ch);
   return;
   }

   /* Parse input and build string for the hash function */
   strcpy(hashstring,GET_NAME(ch));
   strcpy(name,object->name);
   for(i=0; *(name+i)!='\0'; i++)
   if(*(name+i)==' ') {
   *(name+i) = '\0';
   break;
   }
   strcat(hashstring,name);

   percent = Hash(hashstring) % 45;

   percent += get_skill(ch,SKILL_APPRAISE);

   if (IS_SET(ch->specials.act,PLR_DEBUG))
      sendf(ch,"Skill = %d    Bonus = %d\n",
	get_skill(ch,SKILL_APPRAISE),
	Hash(hashstring) % 45);

   if(GetMaxLevel(ch) >= IMMORTAL) 
       percent = 200;
	 
   act("$n looks quizically at the $o.",TRUE,ch,object,0,TO_ROOM);
   identify_new(ch, object, percent);
}

void identify_new(struct char_data *ch, struct obj_data *object, int percent)
{
    char buf[1024],buf2[80],name[80],hashstring[20];
    int i,j,virtual,damage;
    struct affected_type *aff;
    struct extra_descr_data *desc;
    struct follow_type *fol;
    bool found=FALSE;
    char *weap_hit_msgs[] = {
       "You feel a powerful disenchantment emanating from this weapon.",
       "This weapon's made of shoddy materials and has bad balance.",
       "The craftwork on this weapon is a bit below average.",
       "It seems to be of about average quality.",
       "This weapon sits well in your hand and is fairly well balanced.",
       "It's very well balanced and made of excellent material.",
       "A strong guiding enchantment has been placed on this weapon.",
    };
    char *weap_dmg_msgs[] = {
       "This is more of a child's toy than a weapon.",
       "You predict that this weapon would do moderate damage.",
       "This weapon could do quite a bit of damage.",
       "This weapon is capable of doing a large amount of damage.",
       "A weapon of this calibre could reape huge amounts of damage.",
       "Rarely have you seen such potential for mass destruction.",
       "This weapon is responsible for more deaths than several plagues.",
    };
    char *armor_msgs[] = {
       "Walking into combat with this on would be suicide.",
       "This armor might actually increase your odds of getting hit.",
       "You don't think this armor offers much protection at all.",
       "This armor would offer you some mild protection.",
       "This armor would protect you fairly well in combat.",
       "This armor offers very good protection.",
       "You highly suspect that this armor is magically enchanted.",
       "You'd be suprised if anything at all managed to penetrate this armor.",
    };
    char *item_types[] = {
      "Undefined",
      "Light",
      "Scroll",
      "Wand",
      "Staff",
      "Weapon",
      "Missile Weapon",
      "Missile",
      "Treasure",
      "Armor",
      "Potion",
      "Worn",
      "Other",
      "Trash",
      "Trap",
      "Container",
      "Note",
      "Liquid Container",
      "Key",
      "Food",
      "Money",
      "Pen",
      "Boat",
      "Audio",
      "Board",
      "\n",
    };
    char *wear_bits[] = {
       "",
       "Finger",
       "Neck",
       "Body",
       "Head",
       "Legs",
       "Feet",
       "Hands",
       "Arms",
       "Shield",
       "About_Body",
       "Waist",
       "Wrist",
       "Wielded",
       "Held",
       "Thrown",
       "Light",
       "\n",
    };
    char *extra_bits[] = {
       "Glow",
       "Hum",
       "Metal",
       "Mineral",
       "Organic",
       "Invisible",
       "Magic",
       "",
       "Bless",
       "Anti-Good",
       "Anti-Evil",
       "Anti-Neutral",
       "Anti-Cleric",
       "Anti-Mage",
       "Anti-Thief",
       "Anti-Warrior",
       "\n"
    };
    extern char *spells[];
    extern char *drinks[];
    extern char *apply_types[];

    sprintf(buf, "\nObject name: %s    Item type :",object->short_description);
            sprinttype(GET_ITEM_TYPE(object),item_types,buf2);
            strcat(buf,buf2); strcat(buf,"\n");
            send_to_char(buf, ch);
    send_to_char("Can be used :", ch);
            sprintbit(object->obj_flags.wear_flags,wear_bits,buf);
            strcat(buf,"\n");
            send_to_char(buf, ch);

    if(percent>=50) {
        if (object->obj_flags.extra_flags) {
          send_to_char("Magical Properties : ", ch);
               sprintbit(object->obj_flags.extra_flags,extra_bits,buf);
               strcat(buf,"\n");
               send_to_char(buf,ch);
        } else {
          send_to_char("Magical Properties:  None\n",ch);
        }
    } else {
          send_to_char("Magical Properties:  You're unsure...\n",ch);
    }

    sprintf(buf,"Weight: %d lbs., Volume: %d pints, ",
      GET_OBJ_MASS(object) / 454,
      GET_OBJ_VOLUME(object) / 237);
    send_to_char(buf,ch);

    if (percent>=85) {
       sprintf(buf,"Value: %d\n\n",object->obj_flags.cost);
    } else {
       strcpy(buf,"Value: You're unsure\n\n");
    }
    send_to_char(buf, ch);

        /* Item specific information */
        switch (object->obj_flags.type_flag) {
           case ITEM_LIGHT : 
               sprintf(buf, "Hours Left : %d",
                   object->obj_flags.value[2]);
               break;
           case ITEM_SCROLL : 
           case ITEM_POTION :
               sprintf(buf, "Spells : ");
               if(percent>=70) {
                 for(j = 1; j < 4; j++)
                   if(object->obj_flags.value[j] > 0) {
                     sprintf(buf2," '%s'",spells[object->obj_flags.value[j]]);
                     strcat(buf,buf2);
                   }
               } else {
                 strcat(buf,"You're unsure...\n");
               }
               break;
           case ITEM_WAND : 
           case ITEM_STAFF:
               if(percent>=80)
                 sprintf(buf, "Spell : '%s'   Level: %d   Charges Left : %d",
                     spells[object->obj_flags.value[3]],
                     object->obj_flags.value[0],
                     object->obj_flags.value[2]);
               else
                 sprintf(buf, "Spell : ???  Level: ???  Charges Left : ???");
               break;
           case ITEM_WEAPON:
           case ITEM_FIREWEAPON:
           case ITEM_MISSILE:
	       sprintf(buf, "Weapon Type: %s%s\n",
		 weapon_types[object->obj_flags.value[0]],
		 (weapons[object->obj_flags.value[0]].flags & WEAPF_TWO) ?
		 " <2-handed>" : "");
               if(percent>=75) {
/*                  if(object->obj_flags.value[0] <= -5) 
                     strcat(buf,weap_hit_msgs[0]);
                  else if(object->obj_flags.value[0] <= -3)
                     strcat(buf,weap_hit_msgs[1]);
                  else if(object->obj_flags.value[0] <= -1) 
                     strcat(buf,weap_hit_msgs[2]);
                  else if(object->obj_flags.value[0] == 0) 
                     strcat(buf,weap_hit_msgs[3]);
                  else if(object->obj_flags.value[0] <= 2) 
                     strcat(buf,weap_hit_msgs[4]);
                  else if(object->obj_flags.value[0] <= 4) 
                     strcat(buf,weap_hit_msgs[5]);
                  else 
                     strcat(buf,weap_hit_msgs[6]);
                  strcat(buf,"\n");
*/
                  damage = object->obj_flags.value[1]*object->obj_flags.value[2]/2;   
                  if (damage < 2)
                    strcat(buf,weap_dmg_msgs[0]);
                  else if (damage < 4)
                    strcat(buf,weap_dmg_msgs[1]);
                  else if (damage < 8)
                    strcat(buf,weap_dmg_msgs[2]);
                  else if (damage < 12)
                    strcat(buf,weap_dmg_msgs[3]);
                  else if (damage < 16)
                    strcat(buf,weap_dmg_msgs[4]);
                  else if (damage < 20)
                    strcat(buf,weap_dmg_msgs[5]);
                  else
                    strcat(buf,weap_dmg_msgs[6]);
               } else {
                  strcat(buf,"You're unsure about the quality of this weapon.\n");
               }
               break;
           case ITEM_ARMOR :
               if(percent>=85) {
                    if(object->obj_flags.value[0] <= -3) 
                       strcpy(buf,armor_msgs[0]);
                    else if(object->obj_flags.value[0] <= -1) 
                       strcpy(buf,armor_msgs[1]);
                    else if(object->obj_flags.value[0] == 0) 
                       strcpy(buf,armor_msgs[2]);
                    else if(object->obj_flags.value[0] <= 2) 
                       strcpy(buf,armor_msgs[3]);
                    else if(object->obj_flags.value[0] <= 4) 
                       strcpy(buf,armor_msgs[4]);
                    else if(object->obj_flags.value[0] <= 6) 
                       strcpy(buf,armor_msgs[5]);
                    else if(object->obj_flags.value[0] <= 8) 
                       strcpy(buf,armor_msgs[6]);
                    else
                       strcpy(buf,armor_msgs[7]);
                    strcat(buf,"\n");
                } else {
                    strcpy(buf,"You're unsure about this armour's quality.\n");
                }
               break;
           case ITEM_TRAP :
               sprintf(buf, "Spell : '%s'    -Hitpoints : %d",
                   spells[object->obj_flags.value[0]-1],
                   object->obj_flags.value[1]);
               break;
           case ITEM_CONTAINER :
           case ITEM_SPELL_POUCH:
               sprintf(buf, "Max-contains : %d    Corpse : %s",
                   object->obj_flags.value[0],
                   object->obj_flags.value[3]?"Yes":"No");
               break;
           case ITEM_DRINKCON :
               sprinttype(object->obj_flags.value[2],drinks,buf2);
               sprintf(buf, "Max-contains : %d   Contains : %d   Poisoned : %s   Liquid : %s",
                   object->obj_flags.value[0],
                   object->obj_flags.value[1],
                   object->obj_flags.value[3]?"Yes":"No",
                   buf2);
               break;
           case ITEM_NOTE :
               sprintf(buf, "Tounge : %d",
                   object->obj_flags.value[0]);
               break;
           case ITEM_FOOD :
               sprintf(buf, "Makes full : %d   Poisoned : %s",
                   object->obj_flags.value[0],
                   object->obj_flags.value[3]?"Yes":"No");
               break;
           default :
               strcpy(buf,"Unknown object...oops");
               break;
       }
       strcat(buf,"\n\n");
       send_to_char(buf, ch);

       if (percent >= 125) {
	 sendf(ch,"Mystical Features...\n");
         for (i=0;i<MAX_OBJ_AFFECT;i++) {
             sprinttype(object->affected[i].location,apply_types,buf2);
             sprintf(buf,"    Affects : %s By %d\n", buf2,object->affected[i].modifier);
             if(strcmp(buf2,"NONE")) {
               send_to_char(buf, ch);
               found = TRUE;
             }
         }            
       if(found)
           send_to_char("\n",ch);
       } 
       if (percent >= 60) {
         strcpy(buf,(real_objp(object->virtual)->func) ? "There is something special about this object...\n" : "You sense nothing special about this object.\n");
       } else {
           strcpy(buf, "You sense nothing about this object\n");
       }
       send_to_char(buf, ch);

       send_to_char("\n",ch);
       return;
}


/* Skill: Appraise
   Usage: appraise [item in inventory]

   Bugs: Um...I hope I got 'em all....

   Required Installation:
     Add the command and all that garbage...(SKILL_APPRAISE, etc)
     In do_appraise, add a check for the proper class (bard, magic-user, etc.)

   Recommened Installation:
     There are a _lot_ of extern char*[]'s used in appraise.  Just to
     make things neater, I changed a lot of the all caps tables in
     constants.c to normal looking words (FINGER to Finger, etc.).  Not
     necessary, but it cleans things up a bit.

   Appraise gives the PC limited information about an item including the
   name, weight, value, basic effects, and usage locations (hand, wrist,
   etc).  In addition, appraise gives "fuzzy" information about weapons
   and armor (e.g. "This sword could do a lot of damage").  It's not as
   good as an identify, but it is, after all, free =).  Although you can
   give it to whoever you want, I'd recommend either a bard (!) or magic
   user (?) and only advance the skill as levels advance.

   The innerworkings go something like this:
   The PC, as always, has a skill rating from 0% to 95%.  For each item,
   the PC gets a 'bonus' to his percent from 0% to 45%.  This number is
   unique for each PC/item pair (This is done by cating the PC's name
   with the object's name and feeding it to a generic hash function.
   For example, we might send in "Kemperring" or "Kemperscroll".  This
   is done to add a certain random element while preventing PC's from
   appraising items over and over again to see if they can learn more 
   information).  Although some things (like weight) are always known,
   others (like value and weapon/armor information) are only known after
   a certain percent is reached.  

   Hope you like it...
     
            Kemper
*/
