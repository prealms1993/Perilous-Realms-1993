/*  create sounds on objects */

#include <stdio.h>
#include <string.h>

#include <fcns.h>
#include <externs.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <spells.h>
#include <trap.h>


/* extern variables */

extern struct char_data *character_list;

int RecGetObjRoom(struct obj_data *obj) 
{
  if (obj->in_room != NOWHERE) {
    return(obj->in_room);
  }
  if (obj->carried_by) {
    return(obj->carried_by->in_room);
  }
  if (obj->equipped_by) {
    return(obj->equipped_by->in_room);
  }
  if (obj->in_obj) {
    return(RecGetObjRoom(obj->in_obj));
  }
  return(0);
}

void MakeNoise(int room, char *local_snd, char *distant_snd)
{
  int door;
  struct char_data *ch;
  struct room_data *rp, *orp;
  
  rp = real_roomp(room);
  
  if (rp) {
    for (ch = rp->people; ch; ch = ch->next_in_room) {
      if (!IS_NPC(ch)) {
         send_to_char(local_snd, ch);
      }
    }
    for (door = 0; door <= 5; door++) {
      if (rp->dir_option[door] &&
	  (orp = real_roomp(rp->dir_option[door]->to_room)) ) {
	for (ch = orp->people; ch; ch = ch->next_in_room) {
	  if (!IS_NPC(ch) || (!IS_SET(ch->specials.act, PLR_DEAF))) {
   	     send_to_char(distant_snd, ch);
	  }
	}
      }
    }
  }
}

static int ObjNoise(struct obj_data *obj,struct char_data *ch, int pulse)
{
  int room;

  if (!obj->action_description) return(0);
  if (!ITEM_TYPE(obj,ITEM_AUDIO)) return(0);
  if (((obj->obj_flags.value[0]) && (pulse % obj->obj_flags.value[0])==0) || (!number(0,5))) {
    if (obj->carried_by)
      room = obj->carried_by->in_room;
    else if (obj->equipped_by)
      room = obj->equipped_by->in_room;
    else if (obj->in_room != NOWHERE)
      room = obj->in_room;
    else
      room = RecGetObjRoom(obj);
    MakeNoise(room, obj->action_description, obj->action_description);
    return(1);
  }
  return(0);
}

void MakeSound(int pulse)
{
  struct char_data *ch;

/*  objects */
  
  ForAllObjects(NULL,pulse,ObjNoise);

/* mobiles */

  for (ch = character_list; ch; ch = ch->next) {
    if (IS_NPC(ch) && (ch->player.sounds) && (number(0,5)==0)) {
      if (ch->specials.default_pos > POSITION_SLEEPING) {
	if (GET_POS(ch) > POSITION_SLEEPING) {
	  /*  Make the sound; */
	  MakeNoise(ch->in_room, ch->player.sounds, ch->player.distant_snds);
	} else if (GET_POS(ch) == POSITION_SLEEPING) {
	  char buf[MAX_STRING_LENGTH];
	  /* snore */	 
	  sprintf(buf, "%s snores loudly.\n", ch->player.short_descr);
	  MakeNoise(ch->in_room, buf, "You hear a loud snore nearby.\n");
	}
      } else if (GET_POS(ch) == ch->specials.default_pos) {
	/* Make the sound */       
	MakeNoise(ch->in_room, ch->player.sounds, ch->player.distant_snds);
      }
    }
  }
}


