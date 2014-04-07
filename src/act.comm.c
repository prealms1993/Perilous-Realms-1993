/* ************************************************************************
*  file: act.comm.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Communication.                                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include <structs.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <spells.h>
#include <fcns.h>
#include <externs.h>

void do_say(struct char_data *ch, char *arg, int cmd)
{
	char buf[MAX_INPUT_LENGTH+40];

	if (IS_SET(ch->specials.act, PLR_STUPID)) {
	   sendf(ch,"Stop being stupid or go away.\n");
	   return;
         }

	for (; *arg && isspace(*arg); arg++);

	if (!*arg)
		send_to_char("Yes, but WHAT do you want to say?\n", ch);
	else	{
		sprintf(buf,"$c4$n says '%s'$c0", arg);
		act(buf,FALSE,ch,0,0,TO_ROOM);
		if (IS_NPC(ch)||(IS_SET(ch->specials.act, PLR_ECHO))) {
                   sendf(ch,"You say '%s'\n", arg);
		} else send_to_char("Ok.\n",ch);
	}
}




void do_shout(struct char_data *ch, char *argument, int cmd)
{
	char buf1[MAX_INPUT_LENGTH+40];
        struct connection_data *i;

         if (IS_SET(ch->specials.act, PLR_STUPID)) {  
           sendf(ch,"Stop being stupid or go away.\n");
           return;
         } 

	if (IS_SET(ch->specials.act, PLR_NOSHOUT))	{
		send_to_char("You can't shout!!\n", ch);
		return;
	}

	for (; *argument == ' '; argument++);

	if (ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
	  send_to_char("I don't think so :-)", ch->master);
	  return;
	}

	if (!(*argument))
	  send_to_char("Shout? Yes! Fine! Shout we must, but WHAT??\n", ch);
	else	{
	    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
                sendf(ch,"You shout '%s'\n", argument);
	    } else send_to_char("Ok.\n",ch);
		sprintf(buf1, "$c7$n shouts '%s'$c0", argument);

       	        for (i = connection_list; i; i = i->next)
      	        if (i->character != ch && !i->connected &&
			!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
		        !IS_SET(i->character->specials.act, PLR_DEAF))
				act(buf1, 0, ch, 0, i->character, TO_VICT);
	}
}


void do_chat(struct char_data *ch, char *argument, int cmd)
{
  struct connection_data *i;
  struct char_data *victim;
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    return;
  sprintf(buf,"%s> %s\n",ch->player.name,argument);
  for (i = connection_list; i; i = i->next)
    if (!i->connected) {
      if(i->original) continue;
      victim = i->character;
      if((GetMaxLevel(victim) >= SILLYLORD) && (GetMaxLevel(victim) <= (ABS_MAX_LVL)))
        send_to_char(buf,victim);
    }
}


void do_commune(struct char_data *ch, char *argument, int cmd)
{
  struct connection_data *i;
  struct char_data *victim;
  char buf[MAX_STRING_LENGTH];
  extern char rmud_buf[];
  extern int rcon;

  if (cmd!=-1 && IS_NPC(ch))
    return;
  if (cmd!=-1) {
    sprintf(buf,"%s: %s\n",ch->player.name,argument);
    if (rcon != -1) {
      strcat(rmud_buf,"!:");
      strcat(rmud_buf,buf);
    }
  } else
    strcpy(buf,argument);
  for (i = connection_list; i; i = i->next)
    if (!i->connected) {
      if(i->original) continue;
      victim = i->character;
      if((GetMaxLevel(victim) >= LOW_IMMORTAL) && (GetMaxLevel(victim) <= (ABS_MAX_LVL)))
        send_to_char(buf,victim);
    }
}


void do_tell(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[MAX_INPUT_LENGTH], message[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  
 if (IS_SET(ch->specials.act, PLR_STUPID)) {  
    sendf(ch,"Stop being stupid or go away.\n");
    return;                                   
  }

  half_chop(argument,name,message);
  
  if(!*name || !*message) {
    send_to_char("Who do you wish to tell what??\n", ch);
    return;
  } else if (!(vict = get_char_vis(ch, name))) {
    send_to_char("No-one by that name here..\n", ch);
    return;
  } else if (ch == vict) {
    send_to_char("You try to tell yourself something.\n", ch);
    return;
  } else if (GET_POS(vict) == POSITION_SLEEPING)	{
    act("$E is asleep, shhh.",FALSE,ch,0,vict,TO_CHAR);
    return;
  } else if (IS_NPC(vict) && !(vict->desc)) {
    send_to_char("No-one by that name here..\n", ch);
    return;
  } else if (!vict->desc) {
    send_to_char("They can't hear you, link dead.\n", ch);
    return;
  }
  if (IS_SET(vict->specials.act,PLR_VEG) && GetMaxLevel(ch)<IMPLEMENTOR) {
    send_to_char("They can't hear you right now... Editing a string\n",ch);
    return;
  }
  if (IS_SET(vict->specials.act,PLR_NOTELL) && GetMaxLevel(ch)<IMPLEMENTOR) {
    send_to_char("They aren't listening right now.\n",ch);
    return;
  }
  if (IS_SET(ch->specials.act,PLR_NOTELL) && GetMaxLevel(ch)<IMPLEMENTOR) {
    send_to_char("You can't tell anyone anything with your tell turned off!\nThat would be rude!\n",ch);
    return;
  }
  if(IS_AWAY(vict)){
    sendf(ch, "%s is away: %s\n", GET_NAME(vict), vict->specials.away);
  }
  sprintf(buf,"1%s tells you '%s'0\n",
	  (CAN_SEE(ch, vict) ? (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)) : "Someone"), message);
  if (!IS_SET(vict->specials.act,PLR_VEG))
    send_to_char(buf, vict);
  else
    write_to_q(buf,&vict->desc->output,0);

  if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) { 
     sprintf(buf,"You tell %s '%s'\n",
	  (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict)), message);
     send_to_char(buf, ch);
  } else send_to_char("Ok.\n",ch);
  if (IS_SET(vict->specials.act,PLR_VEG))
    send_to_char("They are VEGGED... sent anyways\n",ch);
}

void do_whisper(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH],
  buf[MAX_INPUT_LENGTH];
  
 if (IS_SET(ch->specials.act, PLR_STUPID)) {  
    sendf(ch,"Stop being stupid or go away.\n");
    return;                                   
  }

  half_chop(argument,name,message);
  
  if(!*name || !*message)
    send_to_char("Who do you want to whisper to.. and what??\n", ch);
  else if (!(vict = get_char_room_vis(ch, name)))
    send_to_char("No-one by that name here..\n", ch);
  else if (vict == ch) {
    act("$n whispers quietly to $mself.",FALSE,ch,0,0,TO_ROOM);
    send_to_char
      ("You can't seem to get your mouth close enough to your ear...\n",ch);
  }  else    {
      sprintf(buf,"$n whispers to you, '%s'",message);
      act(buf, FALSE, ch, 0, vict, TO_VICT);
      if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
        sprintf(buf,"You whisper to %s, '%s'\n",
	      (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), message);
        send_to_char(buf, ch);
      } else send_to_char("Ok.\n",ch);
      act("$n whispers something to $N.", FALSE, ch, 0, vict, TO_NOTVICT);
    }
}


void do_ask(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH],
  buf[MAX_INPUT_LENGTH];
  
 if (IS_SET(ch->specials.act, PLR_STUPID)) {  
    sendf(ch,"Stop being stupid or go away.\n");
    return;                                   
  }

  half_chop(argument,name,message);
  
  if(!*name || !*message)
    send_to_char("Who do you want to ask something.. and what??\n", ch);
  else if (!(vict = get_char_room_vis(ch, name)))
    send_to_char("No-one by that name here..\n", ch);
  else if (vict == ch)	{
    act("$n quietly asks $mself a question.",FALSE,ch,0,0,TO_ROOM);
    send_to_char("You think about it for a while...\n", ch);
  }  else	{
    sprintf(buf,"$n asks you '%s'",message);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    
    if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
      sprintf(buf,"You ask %s, '%s'\n",
	    (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), message);
      send_to_char(buf, ch);
    } else send_to_char("Ok.\n",ch);
    act("$n asks $N a question.",FALSE,ch,0,vict,TO_NOTVICT);
  }
}



#define MAX_NOTE_LENGTH 1000      /* arbitrary */

void do_write(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *paper = 0, *pen = 0;
  char papername[MAX_INPUT_LENGTH], penname[MAX_INPUT_LENGTH];
  
 if (IS_SET(ch->specials.act, PLR_STUPID)) {  
    sendf(ch,"Stop being stupid or go away.\n");
    return;                                   
  }

  argument_interpreter(argument, papername, penname);
  
  if (!ch->desc)
    return;
  
  if (!*papername)  /* nothing was delivered */    {   
      send_to_char("write (on) papername (with) penname.\n", ch);
      return;
    }

  if (!*penname) {
      send_to_char("write (on) papername (with) penname.\n", ch);
      return;
  }
  if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))	{
	  sendf(ch, "You have no %s.\n", papername);
	  return;
   }
   if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying)))	{
	  sendf(ch, "You have no %s.\n", penname);
	  return;
    }

  /* ok.. now let's see what kind of stuff we've found */
  if (pen->obj_flags.type_flag != ITEM_PEN) {
      act("$p is no good for writing with.",FALSE,ch,pen,0,TO_CHAR);
  } else if (paper->obj_flags.type_flag != ITEM_NOTE)    {
      act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
  } else if (paper->action_description) {
    send_to_char("There's something written on it already.\n", ch);
    return;
  } else {
      /* we can write - hooray! */
      send_to_char("Ok.. go ahead and write.. end the note with a @.\n", ch);
      act("$n begins to jot down a note.", TRUE, ch, 0,0,TO_ROOM);
      ch->desc->str = &paper->action_description;
      ch->desc->max_str = MAX_NOTE_LENGTH;
    }
}

void do_mail(struct char_data *ch, char *arg, int cmd)
{
  struct obj_data *paper = 0;
  extern struct char_data *character_list;
  struct char_data *v;
  char to[MAX_INPUT_LENGTH],papername[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char *date;
  time_t ct;

 if (IS_SET(ch->specials.act, PLR_STUPID)) {  
    sendf(ch,"Stop being stupid or go away.\n");
    return;                                   
  }

  arg=one_argument(arg,papername);
  arg=one_argument(arg,to);
  if (!*papername || !to) {
    sendf(ch,"Syntax: mail note character\n");
    return;
  }
  if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
    sendf(ch,"You have no %s.\n", papername);
    return;
  }
  if (paper->obj_flags.type_flag != ITEM_NOTE) {
    act("$p isn't a note.", FALSE, ch, paper, 0, TO_CHAR);
    return;
  }
  if (!paper->action_description) {
    sendf(ch,"Try writing something on the note first!\n");
    return;
  }
  for (v = character_list; v; v=v->next)
    if (isname(to, GET_NAME(v)) && !IS_NPC(v))
  break;

  ct=time(0);
  date = asctime(localtime(&ct));
  *(date + strlen(date) - 9) = '\0';

  sprintf(buf,"From: %s\nTo:   %s\nDate: %s\n\n%s",
	GET_NAME(ch),
	CAP(to),
	date,
	paper->action_description);
  if (v) {
    sendf(ch,"Mail sent.\n");
    obj_from_char(paper);
    FREE(paper->action_description);
    paper->action_description=mystrdup(buf);
    obj_to_char(paper,v);
    act("A mail daemon runs up to $n and gives $m a note.",TRUE,v,0,0,TO_ROOM);
    sendf(v,"\nA mail daemon runs up to you and gives you a note.\n\n");
    return;
  }
  if (SendMail(to,buf)) {
    sendf(ch,"Mail sent.\n");
    obj_from_char(paper);
    extract_obj(paper);
    return;
  } else {
    sendf(ch,"%s is unknown to me.\n",to);
    return;
  }
}

void do_beep(struct char_data *ch, char *argument, int cmd)
{
  if (ch->desc)
    send_to_char("\007", ch);
}
 
 
void do_cls(struct char_data *ch, char *argument, int cmd)
{
  if (ch->desc)
    send_to_char("\033[H\033[J", ch);
}

void do_reset(struct char_data *ch, char *argument, int cmd)
{
 if (ch->desc)
   send_to_char("\x1b[r\x1b[2J\x1b[0m", ch);
} 
 
void do_channel(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];

  if(IS_NPC(ch)) return;
 
  one_argument(argument, buf);
 
  if (!*buf) {
      send_to_char("Syntax: channel < auction | gossip | sports> ",ch);
    return;
  }
 
  if (!str_cmp("sports", buf)) {
    if(IS_SET(ch->specials.act, PLR_NOSPORTS)) {
     send_to_char("You have now tuned into the sports channel.\n", ch);
     CLEAR_BIT(ch->specials.act, PLR_NOSPORTS);
   } else {
     send_to_char("You have now tuned out of the sports channel.\n",ch);
     SET_BIT(ch->specials.act, PLR_NOSPORTS);
     }
  }

  if (!str_cmp("auction", buf)) {
    if(IS_SET(ch->specials.act, PLR_AUCTION)) {
	send_to_char("You have now tuned out of the auction channel.\n", ch);
	CLEAR_BIT(ch->specials.act, PLR_AUCTION);
      } else {
	send_to_char("You have now tuned into the auction channel.\n", ch);
	SET_BIT(ch->specials.act, PLR_AUCTION);
      }
  }
 
  if (!str_cmp("gossip", buf)) {
      if(IS_SET(ch->specials.act, PLR_GOSSIP)) {
	send_to_char("You have now tuned out of the gossip channel.\n", ch);
	CLEAR_BIT(ch->specials.act, PLR_GOSSIP);
      } else {
	send_to_char("You have now tuned into the gossip channel.\n", ch);
	SET_BIT(ch->specials.act, PLR_GOSSIP);
      }
   }
}
 
 
void do_auction(struct char_data *ch, char *argument, int cmd)
{
  static char buf1[MAX_STRING_LENGTH];
  struct connection_data *i;
 
 if (IS_SET(ch->specials.act, PLR_STUPID)) {  
    sendf(ch,"Stop being stupid or go away.\n");
    return;                                   
  }

 if(IS_NPC(ch) || IS_SET(ch->specials.act, PLR_SILENCE)) {
    send_to_char("You can't auction!\n", ch);
    return;
  }
 
  if (!IS_SET(ch->specials.act, PLR_AUCTION)) {
    send_to_char("You are not tuned into the auction channel??\n", ch);
    return;
    }
 
  while (*argument == ' ' && *argument != '\0')
    argument++;
 
  if(!*argument)
    send_to_char("Auction? Yes! Fine! Auction we must, but WHAT??\n", ch);
  else {
    send_to_char("Ok.\n", ch);
    sprintf(buf1, "$n auctions '%s'", argument);
 
    if (IS_SET(ch->specials.act, PLR_ECHO))
	act(buf1, 0, ch, 0, ch, TO_CHAR);
 
    for (i = connection_list; i ; i = i->next)
      if(i->character != ch && !i->connected &&
	 IS_SET(i->character->specials.act, PLR_AUCTION))
	act(buf1, 0, ch, 0, i->character, TO_VICT);
  }
}
 
 
void do_gossip(struct char_data *ch, char *argument, int cmd)
{
  static char buf1[MAX_STRING_LENGTH];
  struct connection_data *i;
 
 if (IS_SET(ch->specials.act, PLR_STUPID)) {  
    sendf(ch,"Stop being stupid or go away.\n");
    return;                                   
  }

  if (IS_NPC(ch) && ch->master) return;
  if (!IS_NPC(ch)) {
    if(IS_SET(ch->specials.act, PLR_SILENCE)) {
      send_to_char("You can't gossip!\n", ch);
      return;
    }
 
    if (!IS_SET(ch->specials.act, PLR_GOSSIP)) {
      send_to_char("You are not tuned into the gossip channel??\n", ch);
      return;
    }
  }
 
  while (*argument == ' ' && *argument != '\0')
    argument++;
 
  if(!*argument)
    send_to_char("Gossip? Yes! Fine! Gossip we must, but WHAT??\n", ch);
  else {
    send_to_char("Ok.\n", ch);
    sprintf(buf1, "$c8$n gossips '%s'$c0", argument);
 
    if (!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_ECHO))
	act(buf1, 0, ch, 0, ch, TO_CHAR);
 
    for (i = connection_list; i ; i = i->next)
      if(i->character != ch && !i->connected &&
	 IS_SET(i->character->specials.act, PLR_GOSSIP))
	act(buf1, 0, ch, 0, i->character, TO_VICT);
  }
}
 
void do_prompt(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int i;

  if (IS_NPC(ch)) return;

  if (!*arg) {
	if (!ch->prompt || !*ch->prompt) {
	  send_to_char("You currently have no prompt.\n",ch);
	  send_to_char("Type HELP PROMPT if you want to know how to make one.\n",ch);
	  return;
	} else {
	  send_to_char("Your current prompt is '",ch);
	  send_to_char(ch->prompt,ch);
	  send_to_char("'\n",ch);
	  return;
	}
  }

  while (*arg && isspace(*arg)) arg++;
  if (is_abbrev(arg,"list")) {
	FILE *f;

	if (!(f=fopen("prompts","r"))) {
	  vlog(LOG_URGENT,"Couldn't open prompts file.");
	  return;
	}
	for (i=0; fgets(buf,255,f);)
	  if (strlen(buf)>1) {
	    char *p;
	    i++;
	    p=strchr(buf,'"')+1;
	    buf[strlen(buf)-2]='\0';
	    sendf(ch,"%2d. %s\n",i,p);
	  }
	fclose(f);
	return;
  }
  if (isdigit(*arg)) {
	if (load_prompt(atoi(arg),buf))
	  arg=buf;
        else {
	  send_to_char("There aren't that many prompts avaiable.\n",ch);
	  return;
	}
  }

  if (*arg!='\'') {
	sendf(ch,"Prompt string must be enclosed in quotes.\n");
	sendf(ch,"Type 'HELP PROMPT' for further information.\n");
	sendf(ch,"If you want a pre-made prompt, type prompt list, then prompt ##\n");
	return;
  }
  if (ch->prompt) FREE(ch->prompt);
  ch->prompt=NULL;

  i=0;
  arg++;
  while (*arg && *arg!='\'') buf[i++]= *arg++;
  buf[i]=0;
  ch->prompt=MALLOC(i+1,char);
  strcpy(ch->prompt,buf);
  send_to_char("Ok.\n",ch);
}

int load_prompt(int number, char *buf)
{
  FILE *f;
  int i;

  if (number<1) return(0);
  if (!(f=fopen("prompts","r"))) {
    vlog(LOG_URGENT,"Couldn't open prompts file.");
    return(0);
  }
  i=0;
  while (i<number && fgets(buf,255,f)) i++;
  fclose(f);
  if (i!=number) return(0);
  return(1);
}

void do_emote(struct char_data *ch, char *argument, int cmd)
{
        int i;
        char buf[MAX_INPUT_LENGTH];

       if (IS_SET(ch->specials.act, PLR_STUPID)) {  
          sendf(ch,"Stop being stupid or go away.\n");
          return;                                   
        }

        if (IS_NPC(ch) && (cmd != 0))
                return;

        for (i = 0; *(argument + i) == ' '; i++);

        if (!*(argument + i))
                send_to_char("Yes.. But what?\n", ch);
        else    {
                sprintf(buf,"$n %s", argument + i);
                act(buf,FALSE,ch,0,0,TO_ROOM);
                send_to_char("Ok.\n", ch);
        }
}

