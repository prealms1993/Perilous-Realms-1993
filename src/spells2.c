/* ************************************************************************
*  file: spells2.c , Implementation of magic.             Part of DIKUMUD *
*  Usage : All the non-offensive magic handling routines.                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>

#include <fcns.h>
#include <utils.h>
#include <comm.h>
#include <db.h>
#include <interpreter.h>
#include <spells.h>
#include <handler.h>
#include <poly.h>
#include <externs.h>

struct PolyType PolyList[31] = {
  {"goblin",      4, 201},
  {"parrot",      4, 9001},
  {"frog",        4, 215},
  {"gnoll",       6, 211 },
  {"parrot",      6, 9010 },
  {"lizard",      6, 224},
  {"ogre",        8, 4113},
  {"parrot",      8, 9011},
  {"wolf",        8, 3094},
  {"spider",      9, 227},
  {"beast",       9, 242},
  {"minotaur",    9, 247},
  {"snake",       10, 249},
  {"bull",        10, 1008},
  {"warg",        10, 6100},
  {"sapling",     12, 1421},
  {"ogre-maji",   12, 257},
  {"black",       12, 230},
  {"jackalwere",  12, 254},
  {"troll",       14, 4101},
  {"crocodile",   14, 259},
  {"mindflayer",  14, 7202},
  {"giant",       16, 261},
  {"shark",       16, 199},
  {"blue",        16, 233},
  {"enfan",       18, 21001},
  {"lamia",       18, 5201},
  {"drider",      18, 5011},
  {"wyvern",      20, 3415},
  {"mindflayer",  20, 7201},
  {"spider",      20, 20010},
};

#define LAST_POLY_MOB 30

void cast_resurrection(int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch(type){
  case SPELL_TYPE_SPELL:
    if (!tar_obj) return;
    spell_resurrection(level, ch, 0, tar_obj,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    if (!tar_obj) return;
    spell_resurrection(level, ch, 0, tar_obj,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious problem in 'resurrection'");
    break;
  }

}

void cast_mana (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch(type){
  case SPELL_TYPE_POTION:
    spell_mana(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if (!tar_ch) tar_ch = ch;
    spell_mana(level, ch, tar_ch, 0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_mana(level, ch, tar_ch, 0,eff_level);
  default:
    vlog(LOG_DEBUG,"Serious problem in 'mana'");
    break;
  }

}

void cast_haven (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{


   if(affected_by_spell(tar_ch, SPELL_HAVEN)) {
     sendf(ch,"The song is indeed beautiful, but too fresh in your mind.\n");
     return;
   };

   act("$n hums a little song and begins glowing with a yellow light.\n",
   0,ch,0,0,TO_ROOM);

   spell_haven(level,ch,tar_ch,0,eff_level);
}

void cast_armor (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( affected_by_spell(tar_ch, SPELL_ARMOR) ){
      send_to_char("Nothing seems to happen.\n", ch);
      return;
    }
    if (ch != tar_ch)
      act("$N is protected.", FALSE, ch, 0, tar_ch, TO_CHAR);

    spell_armor(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    if ( affected_by_spell(ch, SPELL_ARMOR) )
      return;
    spell_armor(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    if ( affected_by_spell(tar_ch, SPELL_ARMOR) )
      return;
    spell_armor(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if ( affected_by_spell(tar_ch, SPELL_ARMOR) )
      return;
    spell_armor(level,ch,ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in armor!");
    break;
  }
}

void cast_stone_skin (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( affected_by_spell(ch, SPELL_STONE_SKIN) ){
      send_to_char("Nothing seems to happen.\n", ch);
      return;
    }
    spell_stone_skin(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    if ( affected_by_spell(ch, SPELL_STONE_SKIN) )
      return;
    spell_stone_skin(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if ( affected_by_spell(ch, SPELL_STONE_SKIN) )
      return;
    spell_stone_skin(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if ( affected_by_spell(ch, SPELL_STONE_SKIN) )
      return;
    spell_stone_skin(level,ch,ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in stone_skin!");
    break;
  }
}

void cast_astral_walk (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_POTION:
  case SPELL_TYPE_SPELL:

    if(!tar_ch) send_to_char("Yes, but who do you wish to walk to?\n",ch);
    else spell_astral_walk(level, ch, tar_ch, 0,eff_level);
    break;

    default :
      vlog(LOG_DEBUG,"Serious screw-up in astral walk!");
    break;
  }
}


void cast_teleport (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_POTION:
  case SPELL_TYPE_SPELL:
    if (!tar_ch)
      tar_ch = ch;
    spell_teleport(level, ch, tar_ch, 0,eff_level);
    break;

  case SPELL_TYPE_WAND:
    if(!tar_ch) tar_ch = ch;
    spell_teleport(level, ch, tar_ch, 0,eff_level);
    break;

  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_teleport(level, ch, tar_ch, 0,eff_level);
    break;

    default :
      vlog(LOG_DEBUG,"Serious screw-up in teleport!");
    break;
  }
}


void cast_bless (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (tar_obj) {        /* It's an object */
      if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) ) {
	send_to_char("Nothing seems to happen.\n", ch);
	return;
      }
      spell_bless(level,ch,0,tar_obj,eff_level);

    } else {              /* Then it is a PC | NPC */

      if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
	  (GET_POS(tar_ch) == POSITION_FIGHTING)) {
	send_to_char("Nothing seems to happen.\n", ch);
	return;
      }
      spell_bless(level,ch,tar_ch,0,eff_level);
    }
    break;
  case SPELL_TYPE_POTION:
    if ( affected_by_spell(ch, SPELL_BLESS) ||
	(GET_POS(ch) == POSITION_FIGHTING))
      return;
    spell_bless(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) {        /* It's an object */
      if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) )
	return;
      spell_bless(level,ch,0,tar_obj,eff_level);

    } else {              /* Then it is a PC | NPC */

      if (!tar_ch) tar_ch = ch;

      if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
	  (GET_POS(tar_ch) == POSITION_FIGHTING))
	return;
      spell_bless(level,ch,tar_ch,0,eff_level);
    }
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) {        /* It's an object */
      if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) )
	return;
      spell_bless(level,ch,0,tar_obj,eff_level);

    } else {              /* Then it is a PC | NPC */

      if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
	  (GET_POS(tar_ch) == POSITION_FIGHTING))
	return;
      spell_bless(level,ch,tar_ch,0,eff_level);
    }
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in bless!");
    break;
  }
}

void cast_infravision (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( IS_AFFECTED(tar_ch, AFF_INFRAVISION) ){
      send_to_char("Nothing seems to happen.\n", ch);
      return;
    }
    spell_infravision(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    if ( IS_AFFECTED(ch, AFF_INFRAVISION) )
      return;
    spell_infravision(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    if ( IS_AFFECTED(tar_ch, AFF_INFRAVISION) )
      return;
    spell_infravision(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if ( IS_AFFECTED(tar_ch, AFF_INFRAVISION) )
      return;
    spell_infravision(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	if (!(IS_AFFECTED(tar_ch, AFF_INFRAVISION)))
	  spell_infravision(level,ch,tar_ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in infravision!");
    break;
  }

}

void cast_true_seeing (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT) ){
      send_to_char("Nothing seems to happen.\n", ch);
      return;
    }
    spell_true_seeing(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    if ( IS_AFFECTED(ch, AFF_TRUE_SIGHT) )
      return;
    spell_true_seeing(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    if ( IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT) )
      return;
    spell_true_seeing(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if ( IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT) )
      return;
    spell_true_seeing(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	if (!(IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)))
	  spell_true_seeing(level,ch,tar_ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in true_seeing!");
    break;
  }

}

void cast_blindness (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if ( IS_AFFECTED(tar_ch, AFF_BLIND) ){
      send_to_char("Nothing seems to happen.\n", ch);
      return;
    }
    spell_blindness(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    if ( IS_AFFECTED(ch, AFF_BLIND) )
      return;
    spell_blindness(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    if ( IS_AFFECTED(tar_ch, AFF_BLIND) )
      return;
    spell_blindness(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    if ( IS_AFFECTED(tar_ch, AFF_BLIND) )
      return;
    spell_blindness(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (!in_group(ch, tar_ch))
	if (!(IS_AFFECTED(tar_ch, AFF_BLIND)))
	  spell_blindness(level,ch,tar_ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in blindness!");
    break;
  }
}

void cast_light (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_light(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    spell_calm(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    spell_calm(level,ch,ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in light!");
    break;
  }
}

void cast_cont_light (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cont_light(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    spell_cont_light(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    spell_cont_light(level,ch,ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in continual light!");
    break;
  }
}

#if 0
 void cast_calm (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_calm(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_calm(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_calm(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      spell_calm(level,ch,tar_ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in continual light!");
    break;
  }
}
#endif

void cast_clone (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
	send_to_char("Not *YET* implemented.", ch);
	return;

  /* clone both char and obj !!*/

/*
  switch (type) {
    case SPELL_TYPE_SPELL:
			if (tar_ch) {
				sprintf(buf, "You create a duplicate of %s.\n", GET_NAME(tar_ch));
				send_to_char(buf, ch);
				sprintf(buf, "%%s creates a duplicate of %s,\n", GET_NAME(tar_ch));
				perform(buf, ch, FALSE);

				spell_clone(level,ch,tar_ch,0,eff_level);
			} else {
				sprintf(buf, "You create a duplicate of %s %s.\n",SANA(tar_obj),tar_obj->short_description);
				send_to_char(buf, ch);
				sprintf(buf, "%%s creates a duplicate of %s %s,\n",SANA(tar_obj),tar_obj->short_description);
				perform(buf, ch, FALSE);

				spell_clone(level,ch,0,tar_obj,eff_level);
			};
			break;


    default :
         vlog(LOG_DEBUG,"Serious screw-up in clone!");
         break;
	}
*/
		 /* MISSING REST OF SWITCH -- POTION, SCROLL, WAND */
}


void cast_control_weather (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
	char buffer[MAX_STRING_LENGTH];
	extern struct weather_data weather_info;

  switch (type) {
    case SPELL_TYPE_SPELL:

			one_argument(arg,buffer);

			if (str_cmp("better",buffer) && str_cmp("worse",buffer))
			{
				send_to_char("Do you want it to get better or worse?\n",ch);
				return;
			}
                        if (!OUTSIDE(ch)) {
			  send_to_char("You need to be outside.\n",ch);
                        }

			if(!str_cmp("better",buffer)) {
                          if (weather_info.sky == SKY_CLOUDLESS)
                             return;
                          if (weather_info.sky == SKY_CLOUDY) {
			    send_to_outdoor("The clouds disappear.\n");
			    weather_info.sky=SKY_CLOUDLESS;
     			  }
			  if (weather_info.sky == SKY_RAINING) {
                             if ((time_info.month>3)&&(time_info.month < 14))
	   		       send_to_outdoor("The rain has stopped.\n");
                             else
                               send_to_outdoor("The snow has stopped. \n");
			     weather_info.sky=SKY_CLOUDY;
			  }
			  if (weather_info.sky == SKY_LIGHTNING) {
                             if ((time_info.month>3)&&(time_info.month<14))
		                send_to_outdoor("The lightning has gone, but it is still raining.\n");
			     else
			       send_to_outdoor("The blizzard is over, but it is still snowing.\n");
		 	     weather_info.sky=SKY_RAINING;
			  }
			  return;
			} else {
                          if (weather_info.sky == SKY_CLOUDLESS) {
			send_to_outdoor("The sky is getting cloudy.\n");
			weather_info.sky=SKY_CLOUDY;
                             return;
			   }
                          if (weather_info.sky == SKY_CLOUDY) {
                        if ((time_info.month > 3) && (time_info.month < 14))
			   send_to_outdoor("It starts to rain.\n");
                        else
                           send_to_outdoor("It starts to snow. \n");
			weather_info.sky=SKY_RAINING;
     			  }
			  if (weather_info.sky == SKY_RAINING) {
                             if ((time_info.month>3)&&(time_info.month < 14))
	      	     send_to_outdoor("You are caught in lightning storm.\n");
                          else
                        send_to_outdoor("You are caught in a blizzard. \n");
			   weather_info.sky=SKY_LIGHTNING;
			  }
			  if (weather_info.sky == SKY_LIGHTNING) {
                             return;
			  }

			  return;
                        }
                  break;

      default :
         vlog(LOG_DEBUG,"Serious screw-up in control weather!");
         break;
	}
}



void cast_create_food (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
    case SPELL_TYPE_SPELL:
			act("$n magically creates a mushroom.",FALSE, ch, 0, 0, TO_ROOM);
         spell_create_food(level,ch,0,0,eff_level);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if(tar_ch) return;
         spell_create_food(level,ch,0,0,eff_level);
			break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in create food!");
         break;
	}
}

void cast_create_water (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			if (tar_obj->obj_flags.type_flag != ITEM_DRINKCON) {
				send_to_char("It is unable to hold water.\n", ch);
				return;
			}
			spell_create_water(level,ch,0,tar_obj,eff_level);
			break;
      default :
         vlog(LOG_DEBUG,"Serious screw-up in create water!");
         break;
	}
}




void cast_water_breath (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
    case SPELL_TYPE_SPELL:
       	spell_water_breath(level,ch,tar_ch,0,eff_level);
       	break;
      case SPELL_TYPE_POTION:
       	spell_water_breath(level,ch,tar_ch,0,eff_level);
       	break;
      case SPELL_TYPE_WAND:
       	spell_water_breath(level,ch,tar_ch,0,eff_level);
       	break;

      default :
         vlog(LOG_DEBUG,"Serious screw-up in water breath");
         break;
	}
}

void cast_protect_elem_fire (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
 char buf[MAX_STRING_LENGTH];

  switch (type) {
    case SPELL_TYPE_SPELL:
        spell_protect_elem_fire(level,ch,tar_ch,0,eff_level);
        break;
      case SPELL_TYPE_POTION:
        spell_protect_elem_fire(level,ch,tar_ch,0,eff_level);
        break;
      case SPELL_TYPE_WAND:
        spell_protect_elem_fire(level,ch,tar_ch,0,eff_level);
        break;
      default :
        sprintf(buf,"%s just caused an error in protect_elem_fire in room [%d] SPELL_TYPE was: %d", GET_NAME(ch), ch->in_room, type);
        vlog(LOG_DEBUG,buf);
         break;
        }
}
 
void cast_protect_elem_cold (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
 char buf[MAX_STRING_LENGTH];
  switch (type) {
    case SPELL_TYPE_SPELL:
        spell_protect_elem_cold(level,ch,tar_ch,0,eff_level);
        break;
      case SPELL_TYPE_POTION:
        spell_protect_elem_cold(level,ch,tar_ch,0,eff_level);
        break;
      case SPELL_TYPE_WAND:
        spell_protect_elem_cold(level,ch,tar_ch,0,eff_level);
        break;
      default :
        sprintf(buf,"%s just caused an error in protect_elem_cold in room [%d] SPELL_TYPE was: %d", GET_NAME(ch), ch->in_room, type);
        vlog(LOG_DEBUG,buf);
         break;
        }
}

void cast_flying (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{


  switch (type) {
    case SPELL_TYPE_SPELL:
       	spell_fly(level,ch,tar_ch,0,eff_level);
       	break;
      case SPELL_TYPE_POTION:
       	spell_fly(level,ch,tar_ch,0,eff_level);
       	break;
      case SPELL_TYPE_WAND:
       	spell_fly(level,ch,tar_ch,0,eff_level);
       	break;

      default :
         vlog(LOG_DEBUG,"Serious screw-up in fly");
         break;
	}
}



void cast_cure_blind (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_cure_blind(level,ch,tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_POTION:
			spell_cure_blind(level,ch,ch,0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
               spell_cure_blind(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in cure blind!");
         break;
	}
}



#if 0
 void cast_cure_critic (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_cure_critic(level,ch,tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_POTION:
			spell_cure_critic(level,ch,ch,0,eff_level);
			break;
    case SPELL_TYPE_WAND:
			if (!tar_ch) tar_ch = ch;
			spell_cure_critic(level,ch,tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
               spell_cure_critic(level,ch,tar_ch,0,eff_level);
         break;
      default :
         vlog(LOG_DEBUG,"Serious screw-up in cure critic!");
         break;

	}
}
#endif



void cast_cure_light (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_light(level,ch,tar_ch,0,eff_level);
      break;
    case SPELL_TYPE_POTION:
			spell_cure_light(level,ch,ch,0,eff_level);
			break;
    case SPELL_TYPE_WAND:
			if (!tar_ch) tar_ch = ch;
			spell_cure_light(level,ch,tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
               spell_cure_light(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in cure light!");
         break;
  }
}

void cast_cure_serious (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_serious(level,ch,tar_ch,0,eff_level);
      break;
    case SPELL_TYPE_POTION:
			spell_cure_serious(level,ch,ch,0,eff_level);
			break;
    case SPELL_TYPE_WAND:
			if (!tar_ch) tar_ch = ch;
			spell_cure_serious(level,ch,tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
               spell_cure_serious(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in cure serious!");
         break;
  }
}

void cast_refresh (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_refresh(level,ch,tar_ch,0,eff_level);
      break;
    case SPELL_TYPE_POTION:
			spell_refresh(level,ch,ch,0,eff_level);
			break;
    case SPELL_TYPE_WAND:
			if (!tar_ch) tar_ch = ch;
			spell_refresh(level,ch,tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
               spell_refresh(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in refresh!");
         break;
  }
}

void cast_second_wind (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_second_wind(level,ch,tar_ch,0,eff_level);
      break;
    case SPELL_TYPE_POTION:
			spell_second_wind(level,ch,ch,0,eff_level);
			break;
    case SPELL_TYPE_WAND:

    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
               spell_second_wind(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in second_wind!");
         break;
  }
}

void cast_shield (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_shield(level,ch,tar_ch,0,eff_level);
      break;
    case SPELL_TYPE_POTION:
			spell_shield(level,ch,ch,0,eff_level);
			break;
    case SPELL_TYPE_WAND:
			if (!tar_ch) tar_ch = ch;
			spell_shield(level,ch,tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
               spell_shield(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in shield!");
         break;
  }

}


void cast_curse (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (tar_obj)   /* It is an object */
      spell_curse(level,ch,0,tar_obj,eff_level);
    else {              /* Then it is a PC | NPC */
      spell_curse(level,ch,tar_ch,0,eff_level);
    }
    break;
  case SPELL_TYPE_POTION:
    spell_curse(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)   /* It is an object */
      spell_curse(level,ch,0,tar_obj,eff_level);
    else {              /* Then it is a PC | NPC */
      if (!tar_ch) tar_ch = ch;
      spell_curse(level,ch,tar_ch,0,eff_level);
    }
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_curse(level,ch,tar_ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in curse!");
    break;
  }
}


void cast_detect_evil (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			if ( affected_by_spell(tar_ch, SPELL_DETECT_EVIL) ){
				send_to_char("Nothing seems to happen.\n", tar_ch);
				return;
			}
			spell_detect_evil(level,ch,tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_POTION:
			if ( affected_by_spell(ch, SPELL_DETECT_EVIL) )
				return;
			spell_detect_evil(level,ch,ch,0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
               if(!(IS_AFFECTED(tar_ch, AFF_DETECT_EVIL)))
                  spell_detect_evil(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in detect evil!");
         break;
	}
}



void cast_detect_invisibility (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
                if (IS_AFFECTED(tar_ch, AFF_DETECT_INVISIBLE)) {
       			  send_to_char("Nothing seems to happen.\n", tar_ch);
			  return;
			}
			spell_detect_invisibility(level,ch,tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_POTION:
			if ( IS_AFFECTED(ch, AFF_DETECT_INVISIBLE) )
				return;
			spell_detect_invisibility(level,ch,ch,0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (in_group(ch,tar_ch))
               if (!(IS_AFFECTED(tar_ch, AFF_DETECT_INVISIBLE)))
                  spell_detect_invisibility(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in detect invisibility!");
         break;
	}
}



void cast_detect_magic (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			if ( affected_by_spell(tar_ch, SPELL_DETECT_MAGIC) ){
				send_to_char("Nothing seems to happen.\n", tar_ch);
				return;
			}
			spell_detect_magic(level,ch,tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_POTION:
			if ( affected_by_spell(ch, SPELL_DETECT_MAGIC) )
				return;
			spell_detect_magic(level,ch,ch,0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
               if (!(IS_AFFECTED(tar_ch, SPELL_DETECT_MAGIC)))
                  spell_detect_magic(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in detect magic!");
         break;
	}
}



void cast_detect_poison (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_detect_poison(level, ch, tar_ch,tar_obj,eff_level);
			break;
    case SPELL_TYPE_POTION:
			spell_detect_poison(level, ch, ch,0,eff_level);
			break;
    case SPELL_TYPE_SCROLL:
         if (tar_obj) {
				spell_detect_poison(level, ch, 0, tar_obj,eff_level);
            return;
         }
         if (!tar_ch) tar_ch = ch;
			spell_detect_poison(level, ch, tar_ch, 0,eff_level);
			break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in detect poison!");
         break;
	}
}



void cast_dispel_evil (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_dispel_evil(level, ch, tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_POTION:
			spell_dispel_evil(level,ch,ch,0,eff_level);
			break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) return;
      if (!tar_ch) tar_ch = ch;
			spell_dispel_evil(level, ch, tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_WAND:
      if (tar_obj) return;
			spell_dispel_evil(level, ch, tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (!in_group(tar_ch,ch))
              spell_dispel_evil(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in dispel evil!");
         break;
	}
}

#if 0
 void cast_dispel_good (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_dispel_good(level, ch, tar_ch, 0,eff_level);
			break;
    case SPELL_TYPE_POTION:
			spell_dispel_good(level,ch,ch,0,eff_level);
			break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) return;
      if (!tar_ch) tar_ch = ch;
			spell_dispel_good(level, ch, tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_WAND:
      if (tar_obj) return;
			spell_dispel_good(level, ch, tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (!in_group(tar_ch,ch))
              spell_dispel_good(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in dispel good!");
         break;
	}
}
#endif

void cast_faerie_fire (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_faerie_fire(level, ch, tar_ch, 0,eff_level);
			break;
    case SPELL_TYPE_POTION:
			spell_faerie_fire(level,ch,ch,0,eff_level);
			break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) return;
      if (!tar_ch) tar_ch = ch;
			spell_faerie_fire(level, ch, tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_WAND:
      if (tar_obj) return;
			spell_faerie_fire(level, ch, tar_ch,0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (!in_group(tar_ch,ch))
              spell_faerie_fire(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in dispel good!");
         break;
	}
}



void cast_enchant_weapon (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_enchant_weapon(level, ch, 0,tar_obj,eff_level);
			break;

    case SPELL_TYPE_SCROLL:
			if(!tar_obj) return;
			spell_enchant_weapon(level, ch, 0,tar_obj,eff_level);
			break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in enchant weapon!");
      break;
	}
}


void cast_enchant_armor (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
/*			spell_enchant_armor(level, ch, 0,tar_obj,eff_level);
			break;
*/
    case SPELL_TYPE_SCROLL:
/*			if(!tar_obj) return;
			spell_enchant_armor(level, ch, 0,tar_obj,eff_level);
			break;
*/
    default :
      vlog(LOG_DEBUG,"Serious screw-up in enchant armor!");
      break;
	}
}


void cast_heal (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			act("$n heals $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
			act("You heal $N.", FALSE, ch, 0, tar_ch, TO_CHAR);
			spell_heal(level, ch, tar_ch, 0,eff_level);
			break;
    case SPELL_TYPE_POTION:
         spell_heal(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
              spell_heal(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in heal!");
         break;
	}
}


void cast_invisibility (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			if (tar_obj) {
				if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE) )
					send_to_char("Nothing new seems to happen.\n", ch);
				else
					spell_invisibility(level, ch, 0, tar_obj,eff_level);
			} else { /* tar_ch */
				if ( IS_AFFECTED(tar_ch, AFF_INVISIBLE) )
					send_to_char("Nothing new seems to happen.\n", ch);
				else
					spell_invisibility(level, ch, tar_ch, 0,eff_level);
			}
			break;
    case SPELL_TYPE_POTION:
         if (!IS_AFFECTED(ch, AFF_INVISIBLE) )
            spell_invisibility(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_SCROLL:
			if (tar_obj) {
				if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) )
					spell_invisibility(level, ch, 0, tar_obj,eff_level);
			} else { /* tar_ch */
            if (!tar_ch) tar_ch = ch;

				if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
					spell_invisibility(level, ch, tar_ch, 0,eff_level);
			}
			break;
    case SPELL_TYPE_WAND:
			if (tar_obj) {
				if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) )
					spell_invisibility(level, ch, 0, tar_obj,eff_level);
			} else { /* tar_ch */
				if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
					spell_invisibility(level, ch, tar_ch, 0,eff_level);
			}
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
               if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
                  spell_invisibility(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in invisibility!");
         break;
	}
}




void cast_locate_object (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_locate_object(level, ch, 0, tar_obj,eff_level);
			break;
      default :
         vlog(LOG_DEBUG,"Serious screw-up in locate object!");
         break;
	}
}


void cast_poison (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_poison(level, ch, tar_ch, tar_obj,eff_level);
			break;
    case SPELL_TYPE_POTION:
			spell_poison(level, ch, ch, 0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
                  spell_poison(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in poison!");
         break;
	}
}


#if 0
 void cast_protection_from_evil (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_protection_from_evil(level, ch, tar_ch, 0,eff_level);
			break;
    case SPELL_TYPE_POTION:
         spell_protection_from_evil(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if(!tar_ch) tar_ch = ch;
			spell_protection_from_evil(level, ch, tar_ch, 0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
                  spell_protection_from_evil(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in protection from evil!");
         break;
	}
}
#endif

void cast_remove_curse (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_remove_curse(level, ch, tar_ch, tar_obj,eff_level);
			break;
    case SPELL_TYPE_POTION:
         spell_remove_curse(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) {
				spell_remove_curse(level, ch, 0, tar_obj,eff_level);
 				return;
			}
         if(!tar_ch) tar_ch = ch;
			spell_remove_curse(level, ch, tar_ch, 0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
                  spell_remove_curse(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in remove curse!");
         break;
	}
}



void cast_remove_poison (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_remove_poison(level, ch, tar_ch, tar_obj,eff_level);
			break;
    case SPELL_TYPE_POTION:
         spell_remove_poison(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
                  spell_remove_poison(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in remove poison!");
         break;
	}
}


void cast_remove_paralysis (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
       	spell_remove_paralysis(level, ch, tar_ch, tar_obj,eff_level);
       	break;
    case SPELL_TYPE_POTION:
         spell_remove_paralysis(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_WAND:
	if (!tar_ch) tar_ch = ch;
         spell_remove_paralysis(level, ch, tar_ch, 0,eff_level);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
                  spell_remove_paralysis(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in remove paralysis!");
         break;
	}
}




void cast_sanctuary (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_sanctuary(level, ch, tar_ch, 0,eff_level);
			break;

    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
         spell_sanctuary(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj)
 				return;
         if(!tar_ch) tar_ch = ch;
			spell_sanctuary(level, ch, tar_ch, 0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
                  spell_sanctuary(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in sanctuary!");
         break;
	}
}

void cast_fireshield (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_fireshield(level, ch, tar_ch, 0,eff_level);
			break;

    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
         spell_sanctuary(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj)
 				return;
         if(!tar_ch) tar_ch = ch;
			spell_fireshield(level, ch, tar_ch, 0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
                  spell_fireshield(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in fireshield!");
         break;
	}
}


void cast_sleep (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_sleep(level, ch, tar_ch, 0,eff_level);
			break;
    case SPELL_TYPE_POTION:
			spell_sleep(level, ch, ch, 0,eff_level);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_sleep(level, ch, tar_ch, 0,eff_level);
         break;
    case SPELL_TYPE_WAND:
         if(tar_obj) return;
         spell_sleep(level, ch, tar_ch, 0,eff_level);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
                  spell_sleep(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in sleep!");
         break;
	}
}


void cast_strength (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_strength(level, ch, tar_ch, 0,eff_level);
			break;
    case SPELL_TYPE_POTION:
			spell_strength(level, ch, ch, 0,eff_level);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_strength(level, ch, tar_ch, 0,eff_level);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
                  spell_strength(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in strength!");
         break;
	}
}


void cast_ventriloquate (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
	struct char_data *tmp_ch;
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];

	if (type != SPELL_TYPE_SPELL) {
		vlog(LOG_DEBUG,"Attempt to ventriloquate by non-cast-spell.");
		return;
	}
	for(; *arg && (*arg == ' '); arg++);
	if (tar_obj) {
		sprintf(buf1, "The %s says '%s'\n", fname(tar_obj->name), arg);
		sprintf(buf2, "Someone makes it sound like the %s says '%s'.\n",
		  fname(tar_obj->name), arg);
	}	else {
		sprintf(buf1, "%s says '%s'\n", GET_NAME(tar_ch), arg);
		sprintf(buf2, "Someone makes it sound like %s says '%s'\n",
		  GET_NAME(tar_ch), arg);
	}

	sprintf(buf3, "Someone says, '%s'\n", arg);

	for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch;
	  tmp_ch = tmp_ch->next_in_room) {

		if ((tmp_ch != ch) && (tmp_ch != tar_ch)) {
			if ( saves_spell(tmp_ch, SAVING_SPELL) )
				send_to_char(buf2, tmp_ch);
			else
				send_to_char(buf1, tmp_ch);
		} else {
			if (tmp_ch == tar_ch)
				send_to_char(buf3, tar_ch);
		}
	}
}

void cast_word_of_recall (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_word_of_recall(level, ch, ch, 0,eff_level);
			break;
    case SPELL_TYPE_POTION:
			spell_word_of_recall(level, ch, ch, 0,eff_level);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_word_of_recall(level, ch, tar_ch, 0,eff_level);
         break;
    case SPELL_TYPE_WAND:
         if(tar_obj) return;
         spell_word_of_recall(level, ch, tar_ch, 0,eff_level);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
                  spell_word_of_recall(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in word of recall!");
         break;
	}
}



void cast_summon (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {

    case SPELL_TYPE_SPELL:
			spell_summon(level, ch, tar_ch, 0,eff_level);
			break;
      default :
         vlog(LOG_DEBUG,"Serious screw-up in summon!");
         break;
	}
}



#if 0
void cast_charm_person (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
		case SPELL_TYPE_SPELL:
			spell_charm_person(level, ch, tar_ch, 0,eff_level);
			break;
      case SPELL_TYPE_SCROLL:
         if(!tar_ch) return;
         spell_charm_person(level, ch, tar_ch, 0,eff_level);
         break;
      case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (!in_group(tar_ch,ch))
                  spell_charm_person(level,ch,tar_ch,0,eff_level);
         break;
      default :
         vlog(LOG_DEBUG,"Serious screw-up in charm person!");
         break;
	}
}
#endif

void cast_charm_monster (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
      case SPELL_TYPE_SPELL:
         spell_charm_monster(level, ch, tar_ch, 0,eff_level);
       	 break;
      case SPELL_TYPE_SCROLL:
         if(!tar_ch) return;
         spell_charm_monster(level, ch, tar_ch, 0,eff_level);
         break;
      case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (!in_group(tar_ch,ch))
                  spell_charm_monster(level,ch,tar_ch,0,eff_level);
         break;
      default :
         vlog(LOG_DEBUG,"Serious screw-up in charm monster!");
         break;
	}
}



void cast_sense_life (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
		case SPELL_TYPE_SPELL:
			spell_sense_life(level, ch, ch, 0,eff_level);
			break;
      case SPELL_TYPE_POTION:
         spell_sense_life(level, ch, ch, 0,eff_level);
         break;
      case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
                  spell_sense_life(level,ch,tar_ch,0,eff_level);
         break;
      default :
         vlog(LOG_DEBUG,"Serious screw-up in sense life!");
         break;
	}
}


void cast_identify (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
		case SPELL_TYPE_SCROLL:
			spell_identify(level, ch, tar_ch, tar_obj,eff_level);
			break;
		default :
			vlog(LOG_DEBUG,"Serious screw-up in identify!");
			break;
	}
}

#define MAX_BREATHS 3
struct pbreath {
  int	vnum, spell[MAX_BREATHS];
} breath_potions[] = {
  { 3970, {201, 0} },
  { 3971, {202, 0} },
  { 3972, {203, 0} },
  { 3973, {204, 0} },
  { 3974, {205, 0} },
  { 0 },
};


void cast_dragon_breath(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *potion, int eff_level)
{
  struct pbreath	*scan;
  int	i;
  struct affected_type af;

  for (scan=breath_potions;
       scan->vnum && scan->vnum != potion->virtual;
       scan++)
    ;
  if (scan->vnum==0) {
    char	buf[MAX_STRING_LENGTH];
    send_to_char("Hey, this potion isn't in my list!\n", ch);
    sprintf(buf,"unlisted breath potion %s %d", potion->short_description,
	    potion->virtual);
    vlog(LOG_DEBUG,buf);
    return;
  }

  for (i=0; i<MAX_BREATHS && scan->spell[i]; i++) {
    if (!affected_by_spell(ch, scan->spell[i])) {
      af.type = scan->spell[i];
      af.duration = 1+dice(1,2);
      if (GET_CON(ch) < 4) {
	send_to_char("You are too weak to stomach the potion and spew it all over the floor.\n", ch);
	act("$n gags and pukes glowing goop all over the floor.",
	    FALSE, ch, 0,ch, TO_NOTVICT);
	break;
      }
      if (level > MIN(GET_CON(ch)-1, GetMaxLevel(ch)) ) {
	send_to_char("!GACK! You are too weak to handle the full power of the potion.\n", ch);
	act("$n gags and flops around on the floor a bit.",
	    FALSE, ch, 0,ch, TO_NOTVICT);
	level = MIN(GET_CON(ch)-1, GetMaxLevel(ch));
      }
      af.modifier = -level;
      af.location = APPLY_CON;
      init_bits(af.bitvector);
      affect_to_char(ch, &af);
      send_to_char("You feel powerful forces build within your stomach...\n", ch);
    }
  }
}

void cast_fire_breath (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_fire_breath(level, ch, tar_ch, 0,eff_level);
			break;   /* It's a spell.. But people can'c cast it! */
      default :
         vlog(LOG_DEBUG,"Serious screw-up in firebreath!");
         break;
	}
}

void cast_frost_breath (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_frost_breath(level, ch, tar_ch, 0,eff_level);
			break;   /* It's a spell.. But people can'c cast it! */
      default :
         vlog(LOG_DEBUG,"Serious screw-up in frostbreath!");
         break;
	}
}

void cast_acid_breath (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_acid_breath(level, ch, tar_ch, 0,eff_level);
			break;   /* It's a spell.. But people can'c cast it! */
      default :
         vlog(LOG_DEBUG,"Serious screw-up in acidbreath!");
         break;
	}
}

void cast_gas_breath (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
				spell_gas_breath(level,ch,tar_ch,0,eff_level);
         break;
			/* THIS ONE HURTS!! */
      default :
         vlog(LOG_DEBUG,"Serious screw-up in gasbreath!");
         break;
	}
}

void cast_lightning_breath (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_lightning_breath(level, ch, tar_ch, 0,eff_level);
			break;   /* It's a spell.. But people can'c cast it! */
      default :
         vlog(LOG_DEBUG,"Serious screw-up in lightningbreath!");
         break;
	}
}



void cast_knock (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
	int door, other_room;
	char dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
        char otype[MAX_INPUT_LENGTH];
	struct room_direction_data *back;
	struct obj_data *obj;
        struct char_data *victim;

   switch(type) {
   case SPELL_TYPE_SPELL:
   case SPELL_TYPE_SCROLL:
   case SPELL_TYPE_WAND: {

   argument_interpreter(arg, otype, dir);

   if (!otype) {
      send_to_char("Knock on what?\n",ch);
      return;
   }

   if (generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {

       	if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
                sprintf(buf," %s is not a container.\n ",obj->name);
        } else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
                sprintf(buf, " Silly! %s isn't even closed!\n ", obj->name);
        } else if (obj->obj_flags.value[2] < 0) {
                sprintf(buf,"%s doesn't have a lock...\n",obj->name);
       	} else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
                sprintf(buf,"Hehe.. %s wasn't even locked.\n",obj->name);
	} else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF)) {
                sprintf(buf,"%s resists your magic.\n",obj->name);
	} else {
	     REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	     sprintf(buf,"<Click>\n");
	     act("$n magically opens $p", FALSE, ch, obj, 0, TO_ROOM);
        }
        send_to_char(buf,ch);
        return;
   } else if ((door = find_door(ch, otype, dir)) >= 0) {

       	if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	       	send_to_char("That's absurd.\n", ch);
       	else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	    send_to_char("You realize that the door is already open.\n", ch);
       	else if (EXIT(ch, door)->key < 0)
	      send_to_char("You can't seem to spot any lock to pick.\n", ch);
       	else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	       	send_to_char("Oh.. it wasn't locked at all.\n", ch);
	else if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF))
	     send_to_char("You seem to be unable to knock this...\n", ch);
	else {
	     REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	     if (EXIT(ch, door)->keyword)
		 act("$n magically opens the lock of the $F.", 0, ch, 0,
		      EXIT(ch, door)->keyword, TO_ROOM);
	     else
		 act("$n magically opens the lock.", TRUE, ch, 0, 0, TO_ROOM);
       	     send_to_char("The lock quickly yields to your skills.\n", ch);
	     if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	       	if ((back = real_roomp(other_room)->dir_option[rev_dir[door]]))
	       	   if (back->to_room == ch->in_room)
		      	REMOVE_BIT(back->exit_info, EX_LOCKED);
	   }
         }
      }
      break;
   default:
      vlog(LOG_DEBUG,"serious error in Knock.");
      break;
   }
}


void cast_know_alignment (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_know_alignment(level, ch, tar_ch,tar_obj,eff_level);
			break;
    case SPELL_TYPE_POTION:
			spell_know_alignment(level, ch, ch,0,eff_level);
			break;
    case SPELL_TYPE_SCROLL:
         if (!tar_ch) tar_ch = ch;
			spell_know_alignment(level, ch, tar_ch, 0,eff_level);
			break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in know alignment!");
         break;
	}
}

void cast_weakness (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_weakness(level, ch, tar_ch, 0,eff_level);
			break;
    case SPELL_TYPE_POTION:
			spell_weakness(level, ch, ch, 0,eff_level);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_weakness(level, ch, tar_ch, 0,eff_level);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            //if (!in_group)	//2014 Bug Fix (I think)
		 if (!in_group(tar_ch, ch))
                  spell_weakness(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in weakness!");
         break;
	}
}

void cast_dispel_magic (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
       	spell_dispel_magic(level, ch, tar_ch, tar_obj,eff_level);
       	break;
    case SPELL_TYPE_POTION:
         spell_dispel_magic(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_SCROLL:
/*
         if(tar_obj) {
				spell_dispel_magic(level, ch, 0, tar_obj,eff_level);
 				return;
			}
*/
         if(!tar_ch) tar_ch = ch;
			spell_dispel_magic(level, ch, tar_ch, 0,eff_level);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
                  spell_dispel_magic(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in dispel magic");
         break;
	}
}


void cast_animate_dead (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

    struct obj_data *i;

    switch(type) {

    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
       if (tar_obj) {
      if (IS_CORPSE(tar_obj)) {
             spell_animate_dead(level, ch, 0, tar_obj,eff_level);
	  } else {
	    send_to_char("That's not a corpse!\n",ch);
	    return;
	  }
	} else {
	  send_to_char("That isn't a corpse!\n",ch);
	  return;
	}
       	break;
    case SPELL_TYPE_POTION:
       send_to_char("Your body revolts against the magic liquid.\n",ch);
       ch->points.hit = 0;
       break;
    case SPELL_TYPE_STAFF:
	for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
       	   if (GET_ITEM_TYPE(i)==ITEM_CONTAINER && i->obj_flags.value[3]) {
              spell_animate_dead(level,ch,0,i,eff_level);
	    }
	 }
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in animate_dead!");
         break;
	}
}


void cast_succor (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

   switch(type) {
   case SPELL_TYPE_SPELL:
   case SPELL_TYPE_WAND:
   case SPELL_TYPE_STAFF:
       spell_succor(level, ch, 0, 0,eff_level);
   }

}

void cast_paralyze (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_paralyze(level, ch, tar_ch, 0,eff_level);
			break;
    case SPELL_TYPE_POTION:
			spell_paralyze(level, ch, ch, 0,eff_level);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_paralyze(level, ch, tar_ch, 0,eff_level);
         break;
    case SPELL_TYPE_WAND:
         if(tar_obj) return;
         spell_paralyze(level, ch, tar_ch, 0,eff_level);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
                  spell_paralyze(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in paralyze");
         break;
	}
}

void cast_fear (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_fear(level, ch, tar_ch, 0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_fear(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_fear(level, ch, tar_ch, 0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_fear(level, ch, tar_ch, 0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (!in_group(tar_ch,ch) )
	spell_fear(level,ch,tar_ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in fear");
    break;
  }
}

void cast_turn (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
    case SPELL_TYPE_SPELL:
       	spell_turn(level, ch, tar_ch, 0,eff_level);
       	break;
    case SPELL_TYPE_SCROLL:
        if(tar_obj) return;
        if (!tar_ch) tar_ch = ch;
          spell_turn(level, ch, tar_ch, 0,eff_level);
        break;
    case SPELL_TYPE_WAND:
         if (tar_obj) return;
         if (!tar_ch) tar_ch = ch;
            spell_turn(level, ch, tar_ch, 0,eff_level);
            break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = real_roomp(ch->in_room)->people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (!in_group(tar_ch,ch) )
                  spell_turn(level,ch,tar_ch,0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in turn");
         break;
	}
}

void cast_faerie_fog (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
	switch (type) {
		case SPELL_TYPE_SPELL:
	        case SPELL_TYPE_STAFF:
                case SPELL_TYPE_SCROLL:
			spell_faerie_fog(level, ch, 0, 0,eff_level);
			break;
                default :
                     vlog(LOG_DEBUG,"Serious screw-up in faerie fog!");
                break;
       }
}


void cast_poly_self (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  char buffer[40];
  int mobn=0, X=LAST_POLY_MOB, found=FALSE;
  struct char_data *mob;

  one_argument(arg,buffer);

  if (IS_NPC(ch)) {
    send_to_char("You don't really want to do that.\n",ch);
    return;
  }

  switch(type) {
  case SPELL_TYPE_SPELL:   {

    while (!found) {
      if (PolyList[X].level > level) {
	X--;
      } else {
	if (!str_cmp(PolyList[X].name, buffer)) {
	  mobn = PolyList[X].number;
	  found = TRUE;
	} else {
      	   X--;
	}
	if (X < 0)
          break;
      }
    }

    if (!found) {
      send_to_char("Couldn't find any of those\n", ch);
      return;
    } else {
      mob = get_mob(mobn);
      if (mob) {
	spell_poly_self(level, ch, mob, 0,eff_level);
      } else {
	send_to_char("You couldn't summon an image of that creature\n", ch);
      }
      return;
    }

  } break;

  default: {
    vlog(LOG_DEBUG,"Problem in poly_self");
  } break;
  }

}


#define LONG_SWORD   3022
#define SHIELD       3042
#define RAFT         3060
#define BAG          3032
#define WATER_BARREL 6013
#define BREAD        3010

void cast_minor_creation (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  char buffer[40];
  int obj;
  struct obj_data *o;

  one_argument(arg,buffer);

  if (!str_cmp(buffer, "sword")) {
    obj = LONG_SWORD;
  } else if (!str_cmp(buffer, "shield")) {
    obj=SHIELD;
  } else if (!str_cmp(buffer, "raft")) {
    obj=RAFT;
  } else if (!str_cmp(buffer, "bag")) {
    obj=BAG;
  } else if (!str_cmp(buffer, "barrel")) {
    obj=WATER_BARREL;
  } else if (!str_cmp(buffer, "bread")) {
    obj=BREAD;
  } else {
    send_to_char("There is nothing of that available\n", ch);
    return;
  }

  o = get_obj(obj);
  if (!o) {
      send_to_char("There is nothing of that available\n", ch);
      return;
  }

  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
      spell_minor_create(level, ch, 0, o,eff_level);
      break;
  default:
      vlog(LOG_DEBUG,"serious screw-up in minor_create.");
      break;
  }

}

#define FIRE_ELEMENTAL  10
#define WATER_ELEMENTAL 11
#define AIR_ELEMENTAL   12
#define EARTH_ELEMENTAL 13

#define RED_STONE       5233
#define PALE_BLUE_STONE 5230
#define CLEAR_STONE     5243
#define GREY_STONE      5239

void cast_conjure_elemental (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  char buffer[40];
  int mob, obj;
  struct obj_data *sac;
  struct char_data *el;

  one_argument(arg,buffer);

  if (!str_cmp(buffer, "fire")) {
    mob = FIRE_ELEMENTAL;
    obj = RED_STONE;
  } else if (!str_cmp(buffer, "water")) {
    mob = WATER_ELEMENTAL;
    obj = PALE_BLUE_STONE;
  } else if (!str_cmp(buffer, "air")) {
    mob = AIR_ELEMENTAL;
    obj = CLEAR_STONE;
 } else if (!str_cmp(buffer, "earth")) {
    mob = EARTH_ELEMENTAL;
    obj = GREY_STONE;
  } else {
    send_to_char("There are no elementals of that type available\n", ch);
    return;
  }
  if (!ch->equipment[HOLD]) {
    send_to_char(" You must be holding the correct stone\n", ch);
    return;
  }

  sac = unequip_char(ch, HOLD);
  if (sac) {
    obj_to_char(sac, ch);
    if (ObjVnum(sac) != obj) {
     send_to_char("You must have the correct item to sacrifice.\n", ch);
     return;
    }
    el = get_mob(mob);
    if (!el) {
      send_to_char("There are no elementals of that type available\n", ch);
      return;
    }
  } else {
     send_to_char("You must be holding the correct item to sacrifice.\n", ch);
     return;
  }

  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
      spell_conjure_elemental(level, ch, el, sac,eff_level);
      break;
  default:
      vlog(LOG_DEBUG,"serious screw-up in conjure_elemental.");
      break;
  }

}

#define DEMON_TYPE_I     20
#define DEMON_TYPE_II    21
#define DEMON_TYPE_III   22
#define DEMON_TYPE_IV    23
#define DEMON_TYPE_V     24
#define DEMON_TYPE_VI    25

#define SWORD_ANCIENTS   25000
#define SHADOWSHIV       25014
#define FIRE_SWORD       25015
#define SILVER_TRIDENT   25016
#define JEWELLED_DAGGER  25019
#define SWORD_SHARPNESS  25017

void cast_cacaodemon (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{
  char buffer[40];
  int mob, obj;
  struct obj_data *sac;
  struct char_data *el;

  one_argument(arg,buffer);

  if (!str_cmp(buffer, "one")) {
    mob = DEMON_TYPE_I;
    obj = SWORD_SHARPNESS;
  } else if (!str_cmp(buffer, "two")) {
    mob = DEMON_TYPE_II;
    obj = JEWELLED_DAGGER;
  } else if (!str_cmp(buffer, "three")) {
    mob = DEMON_TYPE_III;
    obj = SILVER_TRIDENT;
 } else if (!str_cmp(buffer, "four")) {
    mob = DEMON_TYPE_IV;
    obj = FIRE_SWORD;
 } else if (!str_cmp(buffer, "five")) {
    mob = DEMON_TYPE_V;
    obj = SHADOWSHIV;
 } else if (!str_cmp(buffer, "six")) {
    mob = DEMON_TYPE_VI;
    obj = SWORD_ANCIENTS;
  } else {
    send_to_char("There are no demons of that type available\n", ch);
    return;
  }
  if (!ch->equipment[WIELD]) {
    send_to_char(" You must be wielding the correct item\n", ch);
    return;
  }

  sac = unequip_char(ch, WIELD);
  if (sac) {
    obj_to_char(sac, ch);
    if (ObjVnum(sac) != obj) {
     send_to_char("You must have the correct item to sacrifice.\n", ch);
     return;
    }
    el = get_mob(mob);
    if (!el) {
      send_to_char("There are no demons of that type available\n", ch);
      return;
    }
  } else {
     send_to_char("You must be holding the correct item to sacrifice.\n", ch);
     return;
  }

  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
      spell_cacaodemon(level, ch, el, sac,eff_level);
      break;
  default:
      vlog(LOG_DEBUG,"serious screw-up in conjure_elemental.");
      break;
  }

}

void cast_mon_sum1 (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
         spell_Create_Monster(5, ch, 0, 0,eff_level);
         break;
       default:
     vlog(LOG_DEBUG,"Serious screw-up in monster_summoning_1");
	 break;
       }
}

void cast_mon_sum2 (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
         spell_Create_Monster(7, ch, 0, 0,eff_level);
         break;
       default:
     vlog(LOG_DEBUG,"Serious screw-up in monster_summoning_1");
	 break;
       }
}

void cast_mon_sum3 (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
         spell_Create_Monster(9, ch, 0, 0,eff_level);
         break;
       default:
     vlog(LOG_DEBUG,"Serious screw-up in monster_summoning_1");
	 break;
       }
}

void cast_mon_sum4 (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
         spell_Create_Monster(11, ch, 0, 0,eff_level);
         break;
       default:
     vlog(LOG_DEBUG,"Serious screw-up in monster_summoning_1");
	 break;
       }
}

void cast_mon_sum5 (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
         spell_Create_Monster(13, ch, 0, 0,eff_level);
         break;
       default:
     vlog(LOG_DEBUG,"Serious screw-up in monster_summoning_1");
	 break;
       }
}

void cast_mon_sum6 (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
         spell_Create_Monster(15, ch, 0, 0,eff_level);
         break;
       default:
     vlog(LOG_DEBUG,"Serious screw-up in monster_summoning_1");
	 break;
       }
}

void cast_mon_sum7 (int level,struct char_data *ch,char *arg,int type,struct char_data *tar_ch,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
         spell_Create_Monster(17, ch, 0, 0,eff_level);
         break;
       default:
     vlog(LOG_DEBUG,"Serious screw-up in monster_summoning_1");
	 break;
       }
}

#if 0
 void cast_invulnerability(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_invulnerability(level, ch, tar_ch, 0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
  case SPELL_TYPE_POTION:
    spell_invulnerability(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj)
      return;
    if(!tar_ch) tar_ch = ch;
    spell_invulnerability(level, ch, tar_ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in invulnerability!");
  }
}
#endif
void cast_portal( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level )
{
  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_STAFF:
    if (!tar_ch) tar_ch = ch;
    spell_portal(level, ch, tar_ch, 0, eff_level);
    break;
  default:
      vlog(LOG_DEBUG,"serious screw-up in portal");
      break;
 
  }
}
void cast_pword_kill( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level )
{
 
  if (!tar_ch) tar_ch = ch;
  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
      spell_pword_kill(level, ch, tar_ch, 0, eff_level);
      break;
  default:
      vlog(LOG_DEBUG,"serious screw-up in pword_kill.");
      break;
  }
 
}
 
void cast_pword_blind( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level )
{
 
  if (!tar_ch) tar_ch = ch;
 
  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
      spell_pword_blind(level, ch, tar_ch, 0, eff_level);
      break;
  default:
      vlog(LOG_DEBUG,"serious screw-up in pword_blind.");
      break;
  }
}
 
 
void cast_scare( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level )
{
 
  if (!tar_ch) tar_ch = ch;
 
  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
      spell_scare(level, ch, tar_ch, 0, eff_level);
      break;
  default:
      vlog(LOG_DEBUG,"serious screw-up in scare.");
      break;
  }
}
 
void cast_familiar( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level )
{
  char buf[128];
 
  /* if (NoSummon(ch)) return; */
 
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_familiar(level, ch, &tar_ch, 0, eff_level);
 
    if (tar_ch) {
      sprintf(buf, "%s %s", GET_NAME(tar_ch), fname(arg));
      FREE(GET_NAME(tar_ch));
      GET_NAME(tar_ch) = (char *)malloc(strlen(buf)+1);
      strcpy(GET_NAME(tar_ch), buf);
    }
    break;
  default:
    vlog(LOG_DEBUG,"serious screw-up in familiar.");
    break;
  }
}
 
void cast_aid( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level )
{
  if (!tar_ch) tar_ch = ch;
 
  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
      spell_aid(level, ch, tar_ch, 0, eff_level);
      break;
  default:
      vlog(LOG_DEBUG,"serious screw-up in aid.");
      break;
  }
}
 
void cast_slow_poison( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level )
{
 
  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_POTION:
  case SPELL_TYPE_WAND:
    if (!tar_ch) tar_ch = ch;
      spell_slow_poison(level, ch, tar_ch, 0, eff_level);
      break;
  default:
      vlog(LOG_DEBUG,"serious screw-up in slow_poison.");
      break;
  }
}
 
void cast_gust_of_wind( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level )
{
 
  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
    spell_gust_of_wind(level, ch, 0, 0, eff_level);
    break;
  default:
      vlog(LOG_DEBUG,"serious screw-up in gust of wind.");
      break;
  }
}
 
void cast_warp_weapon( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level )
{
 
  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    if (!tar_ch) tar_ch = ch;
    spell_warp_weapon(level, ch, tar_ch, tar_obj, eff_level);
    break;
  default:
      vlog(LOG_DEBUG,"serious screw-up in warp_weapon.");
      break;
  }
}
 
 
void cast_beacon( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level )
{
  char buf[MAX_INPUT_LENGTH]; 
  switch(type) {
  case SPELL_TYPE_SPELL:
    one_argument(arg,buf);
    if (str_cmp(buf,"set")==0)
	eff_level=1;
    else eff_level=2;
    spell_beacon(level,ch,tar_ch,tar_obj,eff_level);
    break;
  default:
      vlog(LOG_DEBUG,"becon not as spell");
      break;
  }
}

void cast_delayed_teleport( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level )
{

  switch (type) {
  case SPELL_TYPE_POTION:
    spell_delayed_teleport(level,ch,ch,tar_obj,eff_level);
    break;
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    if (!tar_ch) tar_ch = ch;
    spell_delayed_teleport(level, ch, tar_ch, tar_obj, eff_level);
    break;
  default:
	vlog(LOG_DEBUG,"delayed teleport mess up.\n");
  }
}
