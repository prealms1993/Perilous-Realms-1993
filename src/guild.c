/* ************************************************************************
*  file: guild.c							  *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

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
#include <hash.h>
#include <externs.h>


char *how_good(int percent)
{
  static char buf[256];
  
  if (percent == 0)
    strcpy(buf, " (not learned)");
  else if (percent <= 10)
    strcpy(buf, " (awful)");
  else if (percent <= 20)
    strcpy(buf, " (bad)");
  else if (percent <= 40)
    strcpy(buf, " (poor)");
  else if (percent <= 55)
    strcpy(buf, " (average)");
  else if (percent <= 70)
    strcpy(buf, " (fair)");
  else if (percent <= 80)
    strcpy(buf, " (good)");
  else if (percent <= 85)
    strcpy(buf, " (very good)");
  else
    strcpy(buf, " (Superb)");
  
  return (buf);
}

int GainLevel(struct char_data *ch)
{

  if (CAN_ADVANCE(ch)) {
     send_to_char("You raise a level\n", ch);
     advance_level(ch);
  } else {
     send_to_char("You haven't got enough experience!\n",ch);
  }
  return(FALSE);
}

struct char_data *FindMobInRoomWithFunction(int room, int (*func)())
{
  struct char_data *temp_char, *targ;

  targ = 0;

  if (room > NOWHERE) {
    for (temp_char = real_roomp(room)->people; (!targ) && (temp_char); 
       temp_char = temp_char->next_in_room)
       if (IS_MOB(temp_char))
         if (real_mobp(temp_char->virtual)->func == func)
	   targ = temp_char;

  } else {
    return(0);
  }

  return(targ);

}

int GuildMaster(struct char_data *ch, int cmd, char *arg)
{
  int min_pre;
  int new_value;
  int number, i;
  int max;
  int count;
  skill_entry *sk;
  spell_entry *sp;
  struct char_data *gm;
  struct string_block sb;
  
  if ((cmd != 164) && (cmd != 170) && (cmd != 243)) return(FALSE);

  gm=FindMobInRoomWithFunction(ch->in_room,GuildMaster);
  if (!gm && cmd==243) return(FALSE);

  if (cmd==243 && IS_IMMORTAL(ch)) {
	send_to_char("Shame on you!\n",ch);
	return(TRUE);
  }

  if (cmd == 243) {  /* gain */
	if (GET_LEVEL(ch) < GetMaxLevel(gm)-10)
          GainLevel(ch);
	else
	  send_to_char("I cannot train you.. You must find another.\n",ch);
        return(TRUE);
  }

  if (!*arg) {
    init_string_block(&sb);
    sb_printf(&sb,"You have got %d practice sessions left.\n", 
	      ch->specials.spells_to_learn);

    if (CLASS(ch).skills) {
      sb_printf(&sb,"You can practice any of these skills:\n");
      sb_printf(&sb,"Level Name             Difficulty cost GLD MAX How Well\n");

      for (sk=CLASS(ch).skills; sk; sk=sk->next) {
	if ((sk->min_level > GetMaxLevel(ch)) || (sk->num >= PROF_BASE)) 
          continue;
	sb_printf(&sb,"[%3d] %-20s %3d %5d   %3d %3d %d\n",
		sk->min_level,
		skills[sk->num],
		sk->difficulty,
		sk->cost,
		sk->max_at_guild?sk->max_at_guild:sk->max_learn,
		sk->max_learn,
		get_skill(ch,sk->num));
      }
      sb_printf(&sb,"You can practice any of these proficiencies:\n");
      sb_printf(&sb,"Level Two  Name             Difficulty cost GLD MAX How Well\n");

      for (sk=CLASS(ch).skills; sk; sk=sk->next) {
	if ((sk->min_level > GetMaxLevel(ch)) || (sk->num < PROF_BASE)) 
          continue;
	sb_printf(&sb,"[%3d] %s %-20s %3d %5d   %3d %3d %d\n",
		sk->min_level,
		(weapons[sk->num - PROF_BASE].flags & WEAPF_TWO) ? "<2h>" :
	        "    ",
		weapon_types[sk->num - PROF_BASE],
		sk->difficulty,
		sk->cost,
		sk->max_at_guild?sk->max_at_guild:sk->max_learn,
		sk->max_learn,
		get_skill(ch,sk->num));
      }
    }
    if (CLASS(ch).spells) {
      sb_printf(&sb,"You can practice any of these spells:\n");
      sb_printf(&sb,"Level Name             Difficulty cost GLD MAX How Well\n");
      for (sp=CLASS(ch).spells; sp; sp=sp->next) {
	if (sp->min_level > GetMaxLevel(ch)) continue;
	sb_printf(&sb,"[%3d] %-20s %3d %5d   %3d %3d %d\n",
		sp->min_level,
		spells[sp->num],
		sp->difficulty,
		sp->cost,
		sp->max_at_guild?sp->max_at_guild:sp->max_learn,
		sp->max_learn,
		get_spell(ch,sp->num));
      }
    }
    page_string_block(&sb,ch);
    destroy_string_block(&sb);
    return(TRUE);
  }

  for (;isspace(*arg);arg++);
  count=atoi(arg);
  if (count>0)
    for (;isspace(*arg) || isdigit(*arg); arg++);
  else count=1;

  if (ch->specials.spells_to_learn < count) {
    sendf(ch,"You currently only have %d practice sessions.\n",
		ch->specials.spells_to_learn);
    return(TRUE);
  }

  for (sp=CLASS(ch).spells; sp; sp=sp->next)
    if (is_abbrev(arg,spells[sp->num])) break;
    
  if (!sp) {
    for (sk=CLASS(ch).skills; sk; sk=sk->next)
      if (sk->num < PROF_BASE) {
        if (is_abbrev(arg,skills[sk->num])) break;
      }
      else {
	if (is_abbrev(arg,weapon_types[sk->num - PROF_BASE])) break;
      }
    if (!sk) {
	sendf(ch,"'%s' isn't on any of my lists!\n",arg);
	return(TRUE);
    }
    number = sk->num;
    if (sk->min_level>GetMaxLevel(ch)) {
	sendf(ch,"You can't learn '%s' at this time.\n",
          ((number < PROF_BASE) ? skills[number] :
	  weapon_types[number - PROF_BASE]));
	return(TRUE);
    }
    if (sk->min_level>GetMaxLevel(gm)) {
	sendf(ch,"I never learned '%s'.  You'll have to find someone else.\n",
	  ((number < PROF_BASE) ? skills[number] :
	  weapon_types[number - PROF_BASE]));
	return(TRUE);
    }
    if (count<sk->difficulty) {
      sendf(ch,"The minimum you can do is %d practice sessions.\nType help practice to find out how.\n",sk->difficulty);
      return(TRUE);
    }
    max = (sk->max_at_guild?sk->max_at_guild:sk->max_learn);
    if (get_skill(ch,number) >= max) {
	sendf(ch,"You have already learned as much as I can teach you.\n");
	return(TRUE);
    }
    new_value = get_skill(ch,number)+count/sk->difficulty;
    min_pre=100;
    for (i=0; i<sk->n_pre_reqs; i++)
      min_pre = MIN(min_pre,get_skill(ch,sk->pre_reqs[i]));
    if (new_value>min_pre && new_value<=max) {
	sendf(ch,"You must first strive harder at the basics.\n");
	return(TRUE);
    }
    if (new_value > max) {
	sendf(ch,"I can only train you for %d more sessions.\n",
		  (max - get_skill(ch,number))*sk->difficulty);
	return(TRUE);
    }
    if (GET_GOLD(ch)<count*sk->cost) {
	sendf(ch,"You don't have enough money.\n");
	return(TRUE);
    }
    sendf(ch,"You pay %d coins for %d training sessions of %s.\n",
	  count*sk->cost, count, ((number < PROF_BASE) ? skills[number] :
	  weapon_types[number - PROF_BASE]));
    GET_GOLD(ch)-=count*sk->cost;
    ch->specials.spells_to_learn -= count;
    set_skill(ch,number,count/sk->difficulty);
  } else {
    number = sp->num;
    if (sp->min_level>GetMaxLevel(ch)) {
	sendf(ch,"You can't learn '%s' at this time.\n",
	  ((number < PROF_BASE) ? skills[number] :
	  weapon_types[number - PROF_BASE]));
	return(TRUE);
    }
    if (sp->min_level>GetMaxLevel(gm)) {
	sendf(ch,"I never learned '%s'.  You'll have to find someone else.\n",
	  ((number < PROF_BASE) ? skills[number] :
	  weapon_types[number - PROF_BASE]));
	return(TRUE);
    }
    if (count<sp->difficulty) {
      sendf(ch,"The minimum you can do is %d practice sessions.\nType help practice to find out how.\n",sp->difficulty);
      return(TRUE);
    }
    max=(sp->max_at_guild?sp->max_at_guild:sp->max_learn);
    if (get_spell(ch,number) >= max) {
	sendf(ch,"You have already learned as much as I can teach you.\n");
	return(TRUE);
    }
    new_value = get_spell(ch,number) + count/sp->difficulty;
    min_pre = 100;
    for (i=0; i<sp->n_pre_reqs; i++)
      min_pre = MIN(min_pre,get_spell(ch,sp->pre_reqs[i]));
    if (new_value>min_pre && new_value<=max) {
	sendf(ch,"You must first strive harder at the basics.\n");
	return(TRUE);
    }
    if (new_value > max) {
	sendf(ch,"I can only train you for %d more sessions.\n",
		(max - get_spell(ch,number))*sp->difficulty);
	return(TRUE);
    }
    if (GET_GOLD(ch)<count*sp->cost) {
	sendf(ch,"You don't have enough money.\n");
	return(TRUE);
    }
    sendf(ch,"You pay %d coins for %d training sessions of %s.\n",
		count*sp->cost,count,spells[sp->num]);
    GET_GOLD(ch) -= count*sp->cost;
    ch->specials.spells_to_learn -= count;
    set_spell(ch,number,count/sp->difficulty);
  }
  return(TRUE);
}
