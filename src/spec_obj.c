/* ************************************************************************
*  file: spec_procs.c , Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <fcns.h>
#include <externs.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <spells.h>
#include <limits.h>
#include <hash.h>
#include <rooms.h>

/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct time_info_data time_info;
extern struct weather_data weather_info;
extern int top_of_world;
extern struct int_app_type int_app[26];

/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */



#define CHAL_ACT \
"You are torn out of reality!\n\
You roll and tumble through endless voids for what seems like eternity...\n\
\n\
After a time, a new reality comes into focus... you are elsewhere.\n"


int chalice(struct char_data *ch, int cmd, char *arg)
{
  /* 222 is the normal chalice, 223 is chalice-on-altar */

  struct obj_data *chalice;
  char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  static int chl = 222, achl = 223;

  
  switch(cmd)
    {
    case 10:    /* get */
      if (!(chalice = get_obj_in_list_num(chl,
                                          real_roomp(ch->in_room)->contents))
          && CAN_SEE_OBJ(ch, chalice))
        if (!(chalice = get_obj_in_list_num(achl,
                                            real_roomp(ch->in_room)->contents)) && CAN_SEE_OBJ(ch, chalice))
          return(0);

      /* we found a chalice.. now try to get us */
      do_get(ch, arg, cmd);
      /* if got the altar one, switch her */
      if (chalice == get_obj_in_list_num(achl, ch->carrying))
        {
          extract_obj(chalice);
          chalice = get_obj(chl);
          obj_to_char(chalice, ch);
        }
      return(1);
      break;
    case 67: /* put */
      if (!(chalice = get_obj_in_list_num(chl, ch->carrying)))
        return(0);

      argument_interpreter(arg, buf1, buf2);
      if (!str_cmp(buf1, "chalice") && !str_cmp(buf2, "altar"))
        {
          extract_obj(chalice);
          chalice = get_obj(achl);
          obj_to_room(chalice, ch->in_room);
          send_to_char("Ok.\n", ch);
        }
      return(1);
      break;
    case 176: /* pray */
      if (!(chalice = get_obj_in_list_num(achl,
                                          real_roomp(ch->in_room)->contents)))
        return(0);

      do_action(ch, arg, cmd);  /* pray */
      send_to_char(CHAL_ACT, ch);
      extract_obj(chalice);
      act("$n is torn out of existence!", TRUE, ch, 0, 0, TO_ROOM);
      char_from_room(ch);
      char_to_room(ch, 2500);   /* before the fiery gates */
      do_look(ch, "", 15);
      return(1);
      break;
    default:
      return(0);
      break;
    }
}

int kings_hall(struct char_data *ch, int cmd, char *arg)
{
  if (cmd != 176)
    return(0);

  do_action(ch, arg, 176);

  send_to_char("You feel as if some mighty force has been offended.\n", ch);
  send_to_char(CHAL_ACT, ch);
  act("$n is struck by an intense beam of light and vanishes.",
      TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, 1420);  /* behind the altar */
  do_look(ch, "", 15);
  return(1);
}

#define ENTER 7
 
int portal(struct char_data *ch, int cmd, char *arg)
{
  struct obj_data *port;
  char obj_name[MAX_INPUT_LENGTH];
 
  if (cmd != ENTER) return(FALSE);
 
  arg=one_argument(arg,obj_name);
  if (!(port = get_obj_in_list_vis(ch, obj_name, real_roomp(ch->in_room)->contents)))	{
    return(FALSE);
  }
 
  if (real_objp(port->virtual)->func!=portal)
    return(FALSE);
 
  if (!real_roomp(port->obj_flags.value[1])) {
    send_to_char("The portal leads nowhere\n", ch);
    return(TRUE);
  }
 
  act("$n enters $p, and vanishes!", FALSE, ch, port, 0, TO_ROOM);
  act("You enter $p, and you are transported elsewhere", FALSE, ch, port, 0, TO_CHAR);
  char_from_room(ch);
  char_to_room(ch, port->obj_flags.value[1]);
  do_look(ch, "", 0);
  act("$n appears from thin air!", FALSE, ch, 0, 0, TO_ROOM);
  return(TRUE);
}
 
int scraps(struct char_data *ch, int cmd, char *arg, struct obj_data *obj, int type)
{
 
  if (type == PULSE_COMMAND) {
    return(FALSE);
  } else {
    if (obj->obj_flags.value[0])
      obj->obj_flags.value[0]--;
 
    if (obj->obj_flags.value[0] == 0 && obj->in_room) {
      if ((obj->in_room != NOWHERE) &&(real_roomp(obj->in_room)->people)) {
	act("$p disintegrates into atomic particles!",
	    FALSE, real_roomp(obj->in_room)->people, obj, 0, TO_ROOM);
	act("$p disintegrates into atomic particles!",
	    FALSE, real_roomp(obj->in_room)->people, obj, 0, TO_CHAR);
      }
      extract_obj(obj);
    }
  }
  return(FALSE);
}
 
