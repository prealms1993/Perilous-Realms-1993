/*
**  magicutils -- stuff that makes the magic files easier to read.
*/

#include <stdio.h>
#include <assert.h>

#include <fcns.h>
#include <utils.h>
#include <comm.h>
#include <spells.h>
#include <handler.h>
#include <limits.h>
#include <externs.h>


void SwitchStuff( struct char_data *giver, struct char_data *taker)
{
  struct obj_data *obj, *next;
  float ratio;
  int j;
  
  /*
   *  take all the stuff from the giver, put in on the
   *  taker
   */
  
  for (j = 0; j< MAX_WEAR; j++) {
    if (giver->equipment[j]) {
      obj = unequip_char(giver, j);
      equip_char(taker, obj, j);
    }
  }
  
  for (obj = giver->carrying; obj; obj = next) {
    next = obj->next_content;
    obj_from_char(obj);
    obj_to_char(obj, taker);
  }
  
  /*
   *    gold...
   */

  GET_GOLD(taker) = GET_GOLD(giver);
  
  /*
   *   hit point ratio
   */

   ratio = (float) GET_HIT(giver) / GET_MAX_HIT(giver);
   GET_HIT(taker) = ratio * GET_MAX_HIT(taker);

  /*
   * experience
   */

  if (!IS_IMMORTAL(taker)) {
     if (!IS_IMMORTAL(giver))
        GET_EXP(taker) = GET_EXP(giver);

     GET_EXP(taker) = MIN(GET_EXP(taker), 15000000);

  }

  /*
   *  humanoid monsters can cast spells
   */

  GET_MANA(taker) = GET_MANA(giver);
  GET_POWER(taker) = GET_POWER(giver);

  log("Fix this... Switch stuff");
  return;
  if (IS_NPC(taker)) {
    taker->player.class = giver->player.class;
    for (j = 0; j< MAX_SKILLS; j++)
      taker->skills[j] = giver->skills[j];
    taker->player.level = giver->player.level;
  }


}


void FailCharm(struct char_data *victim, struct char_data *ch)
{
  if (IS_NPC(victim)) {
    if (!victim->specials.fighting) {
      set_fighting(victim,ch);
    } 
  } else {
    send_to_char("You feel charmed, but the feeling fades.\n",victim);
  }
}

void FailSleep(struct char_data *victim, struct char_data *ch)
{
  
  send_to_char("You feel sleepy for a moment,but then you recover\n",victim);
  if (IS_NPC(victim))
    if ((!victim->specials.fighting) && (GET_POS(victim) > POSITION_SLEEPING))
      set_fighting(victim,ch);
}


void FailPara(struct char_data *victim, struct char_data *ch)
{
  send_to_char("You feel frozen for a moment,but then you recover\n",victim);
  if (IS_NPC(victim))
    if ((!victim->specials.fighting) && (GET_POS(victim) > POSITION_SLEEPING))
      set_fighting(victim,ch);
}

void FailCalm(struct char_data *victim, struct char_data *ch)
{
  send_to_char("You feel happy and easygoing, but the effect soon fades.\n",victim);
  if (IS_NPC(victim))
    if (!victim->specials.fighting)
      set_fighting(victim,ch);
}


