/* ************************************************************************
*  file: magic3.c ,  Implementation of more spells.           Part of     *
*  Usage : The actual effect of magic spells              Perilous Realms *
**************************************************************************/
 
#include <stdio.h>
#include <assert.h>

#include <fcns.h>
#include <rooms.h>
#include <utils.h>
#include <comm.h>
#include <spells.h>
#include <handler.h>
#include <limits.h>
#include <db.h>
#include <race.h>
#include <ticks.h>
 
#define ELVL20 MIN(eff_level,20)
 
/* Extern structures */
extern struct room_data *world;
extern struct obj_data  *object_list;
extern struct char_data *character_list;
extern struct hash_header room_db;
 
 
 
void spell_protect_elem_fire(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;
 
  if (affected_by_spell(victim, SPELL_PROTECT_ELEM_FIRE)) {
    send_to_char("Already in effect\n", ch);
    return;
  }
 
  act("$n is protected from elemental fire", FALSE, victim, 0, 0, TO_ROOM);
  send_to_char("You are now protected from elemental fire\n", victim);
 
  af.type      = SPELL_PROTECT_ELEM_FIRE;
  af.duration  = 15;
  af.modifier  = 0;
  af.location  = 0;
  init_bits(af.bitvector);
  affect_to_char(victim, &af);
 
}
 
void spell_protect_elem_cold(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  struct affected_type af;
 
  if (affected_by_spell(victim, SPELL_PROTECT_ELEM_COLD)) {
    send_to_char("Already in effect\n", ch);
    return;
  }
 
  act("$n is protected from elemental cold", FALSE, victim, 0, 0, TO_ROOM);
  send_to_char("You are now protected from elemental cold\n", victim);
 
  af.type      = SPELL_PROTECT_ELEM_COLD;
  af.duration  = 15;
  af.modifier  = 0;
  af.location  = 0;
  init_bits(af.bitvector);
  affect_to_char(victim, &af);
 
}

void spell_beacon(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  if (eff_level==1) {
    if (ROOM_FLAG(ch->specials.beacon_location,NO_GOTO) ||
	ROOM_FLAG(ch->specials.beacon_location,LORD_ONLY) ||
	ROOM_FLAG(ch->specials.beacon_location,PRIVATE) ||
	ROOM_FLAG(ch->specials.beacon_location,ARENA)) {
      sendf(ch,"Something about this place prevents you from placing a beacon here.\n");
      return;
    }
    ch->specials.beacon_location=ch->in_room;
    sendf(ch,"You place a magical beacon here.\n");
    act("$n places a magical beacon.", FALSE, ch, 0, 0, TO_ROOM);
    return;
  }

  act("$n disappears in a frenzy of lights.",FALSE,ch, 0,0,TO_ROOM);
  char_from_room(ch);
  char_to_room(ch,ch->specials.beacon_location);
  act("$n appears in a frenzy of lights.",FALSE,ch, 0,0,TO_ROOM);
  sendf(ch,"You teleport to your beacon.\n");
}

void delayed_teleport(struct char_data *ch, int data)
{
  sendf(ch,"You feel yourself being pushed out of this locality of space.\n");
  spell_teleport(GET_LEVEL(ch),ch,ch,0,0);
}

void spell_delayed_teleport(int level, struct char_data *ch, struct char_data *victim, struct obj_data *obj, int eff_level)
{
  int x;
  x=number(1,10);
  schedule(x,delayed_teleport,EV_CHAR_DATA,victim,0);
}
