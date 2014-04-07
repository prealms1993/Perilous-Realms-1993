/* ************************************************************************
*  file: spells1.c , handling of magic.                   Part of DIKUMUD *
*  Usage : Procedures handling all offensive magic.                       *
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
#include <externs.h>

void cast_nova(int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch(type){
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
      spell_nova(level, ch, 0, 0, eff_level);
      break;
    default:
      vlog(LOG_DEBUG, "Serious screw-up in nova.");
  }
}

void cast_burning_hands (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
	switch (type) {
		case SPELL_TYPE_SPELL:
	        case SPELL_TYPE_STAFF:
                case SPELL_TYPE_SCROLL:
			spell_burning_hands(level, ch, 0, 0,eff_level);
			break;
                default :
                     vlog(LOG_DEBUG,"Serious screw-up in burning hands!");
                break;
       }
}


void cast_chain_lightn(int level, struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  if (!victim) victim = ch;

  switch(type) {         
  case SPELL_TYPE_SPELL: 
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:  
  case SPELL_TYPE_STAFF:
      spell_chain_lightn(level, ch, victim, 0, eff_level);
      break;
  default:
    vlog(LOG_DEBUG,"serious screw-up in chain lightn.");
    break;
  }
}

void cast_call_lightning (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  extern struct weather_data weather_info;

	switch (type) {
		case SPELL_TYPE_SPELL:
			if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
				spell_call_lightning(level, ch, victim, 0,eff_level);
			} else {
				send_to_char("You fail to call upon the lightning from the sky!\n", ch);
			}
			break;
      case SPELL_TYPE_POTION:
			if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
				spell_call_lightning(level, ch, ch, 0,eff_level);
			}
			break;
      case SPELL_TYPE_SCROLL:
			if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
				if(victim)
					spell_call_lightning(level, ch, victim, 0,eff_level);
				else if(!tar_obj) spell_call_lightning(level, ch, ch, 0,eff_level);
			}
			break;
      case SPELL_TYPE_STAFF:
			if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
				for (victim = real_roomp(ch->in_room)->people ;
                 victim ; victim = victim->next_in_room )
				  if (!in_group(victim,ch))
				   spell_call_lightning(level, ch, victim, 0,eff_level);
			}
			break;
      default :
         vlog(LOG_DEBUG,"Serious screw-up in call lightning!");
         break;
	}
}


void cast_chill_touch(int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_chill_touch(level, ch, victim, 0,eff_level);
			break;
      default :
         vlog(LOG_DEBUG,"Serious screw-up in chill touch!");
         break;
	}
}


void cast_shocking_grasp(int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_shocking_grasp(level, ch, victim, 0,eff_level);
			break;
      default :
         vlog(LOG_DEBUG,"Serious screw-up in shocking grasp!");
         break;
	}
}


void cast_colour_spray (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_colour_spray(level, ch, victim, 0,eff_level);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
            spell_colour_spray(level, ch, victim, 0,eff_level);
         else if (!tar_obj)
				spell_colour_spray(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
            spell_colour_spray(level, ch, victim, 0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in colour spray!");
         break;
	}
}


void cast_earthquake (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
			spell_earthquake(level, ch, 0, 0,eff_level);
	      break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in earthquake!");
         break;
	}
}


void cast_energy_drain (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_energy_drain(level, ch, victim, 0,eff_level);
			break;
    case SPELL_TYPE_POTION:
         spell_energy_drain(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
       		spell_energy_drain(level, ch, victim, 0,eff_level);
         else if(!tar_obj)
            spell_energy_drain(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
       	       	spell_energy_drain(level, ch, victim, 0,eff_level);
         break;
    case SPELL_TYPE_STAFF:
         for (victim = real_roomp(ch->in_room)->people ;
              victim ; victim = victim->next_in_room )
            if (!in_group(ch,victim))
               if (victim != ch)
                  spell_energy_drain(level, ch, victim, 0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in energy drain!");
         break;
	}
}


void cast_fireball (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{


  switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
			spell_fireball(level, ch, 0, 0,eff_level);
	      break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in fireball");
         break;
	}



/*
  switch (type) {
    case SPELL_TYPE_SPELL:
		  spell_fireball(level, ch, victim, 0,eff_level);
	    break;
    case SPELL_TYPE_SCROLL:
         if(victim)
				spell_fireball(level, ch, victim, 0,eff_level);
         else if(!tar_obj)
            spell_fireball(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
				spell_fireball(level, ch, victim, 0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in fireball!");
         break;

	}
*/

}


void cast_harm (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
	switch (type) {
    case SPELL_TYPE_SPELL:
         spell_harm(level, ch, victim, 0,eff_level);
         break;
    case SPELL_TYPE_POTION:
         spell_harm(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_STAFF:
         for (victim = real_roomp(ch->in_room)->people ;
              victim ; victim = victim->next_in_room )
            if (!in_group(ch,victim))
               spell_harm(level, ch, victim, 0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in harm!");
         break;

  }
}


void cast_lightning_bolt (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
         spell_lightning_bolt(level, ch, victim, 0,eff_level);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
				spell_lightning_bolt(level, ch, victim, 0,eff_level);
         else if(!tar_obj)
            spell_lightning_bolt(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
				spell_lightning_bolt(level, ch, victim, 0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in lightning bolt!");
         break;

  }
}

void cast_acid_blast (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
         spell_acid_blast(level, ch, victim, 0,eff_level);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
       		spell_acid_blast(level, ch, victim, 0,eff_level);
         else
            spell_acid_blast(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
       		spell_acid_blast(level, ch, victim, 0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in acid blast!");
         break;

  }
}

void cast_cone_of_cold (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
         spell_cone_of_cold(level, ch, 0, 0,eff_level);
         break;

    default :
         vlog(LOG_DEBUG,"Serious screw-up in cone of cold!");
         break;

  }
}

void cast_firestorm (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
       	spell_firestorm(level, ch, 0, 0,eff_level);
        break;

    default :
         vlog(LOG_DEBUG,"Serious screw-up in firestorm");
         break;

  }
}

void cast_ice_storm (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
       	spell_ice_storm(level, ch, 0, 0,eff_level);
        break;

    default :
         vlog(LOG_DEBUG,"Serious screw-up in acid blast!");
         break;

  }
}

void cast_meteor_swarm (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{

  switch (type) {
    case SPELL_TYPE_SPELL:
         spell_meteor_swarm(level, ch, victim, 0,eff_level);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
       		spell_meteor_swarm(level, ch, victim, 0,eff_level);
	 else
            spell_meteor_swarm(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
       		spell_meteor_swarm(level, ch, victim, 0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in meteor swarm!");
         break;

  }
}

void cast_flamestrike (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
         spell_flamestrike(level, ch, victim, 0,eff_level);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
       	      	spell_flamestrike(level, ch, victim, 0,eff_level);
         else if(!tar_obj)
                spell_flamestrike(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
               	spell_flamestrike(level, ch, victim, 0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in flamestrike!");
         break;

  }
}

void cast_magic_missile (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_magic_missile(level, ch, victim, 0,eff_level);
      break;
    case SPELL_TYPE_SCROLL:
         if(victim)
       	    	spell_magic_missile(level, ch, victim, 0,eff_level);
         else if(!tar_obj)
            spell_magic_missile(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
       	       	spell_magic_missile(level, ch, victim, 0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in magic missile!");
         break;

  }
}


void cast_cause_light (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
	switch (type) {
    case SPELL_TYPE_SPELL:
         spell_cause_light(level, ch, victim, 0,eff_level);
         break;
    case SPELL_TYPE_POTION:
         spell_cause_light(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_STAFF:
         for (victim = real_roomp(ch->in_room)->people ;
              victim ; victim = victim->next_in_room )
            if (!in_group(ch,victim))
               spell_cause_light(level, ch, victim, 0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in cause light wounds!");
         break;

  }
}

void cast_cause_serious (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
	switch (type) {
    case SPELL_TYPE_SPELL:
         spell_cause_serious(level, ch, victim, 0,eff_level);
         break;
    case SPELL_TYPE_POTION:
         spell_cause_serious(level, ch, ch, 0,eff_level);
         break;
       case SPELL_TYPE_WAND:
	 if (tar_obj) return;
	 if (!victim) victim = ch;
	 spell_cause_serious(level, ch, victim, 0,eff_level);
    case SPELL_TYPE_STAFF:
         for (victim = real_roomp(ch->in_room)->people ;
              victim ; victim = victim->next_in_room )
            if (!in_group(ch,victim))
               spell_cause_serious(level, ch, victim, 0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in cause serious wounds!");
         break;

  }
}


void cast_cause_critic (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
	switch (type) {
    case SPELL_TYPE_SPELL:
         spell_cause_critical(level, ch, victim, 0,eff_level);
         break;
    case SPELL_TYPE_POTION:
         spell_cause_critical(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_STAFF:
         for (victim = real_roomp(ch->in_room)->people ;
              victim ; victim = victim->next_in_room )
            if (!in_group(ch,victim))
               spell_cause_critical(level, ch, victim, 0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in cause critical!");
         break;

  }
}


void cast_geyser (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
			spell_geyser(level, ch, 0, 0,eff_level);
	      break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in geyser!");
         break;
	}
}


void cast_green_slime (int level,struct char_data *ch,char *arg,int type,struct char_data *victim,struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_green_slime(level, ch, victim, 0,eff_level);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
            spell_green_slime(level, ch, victim, 0,eff_level);
         else if (!tar_obj)
	    spell_green_slime(level, ch, ch, 0,eff_level);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
            spell_green_slime(level, ch, victim, 0,eff_level);
         break;
    default :
         vlog(LOG_DEBUG,"Serious screw-up in green Slime!");
         break;
	}
}

/* NEW */


void cast_invulnerability(int level, struct char_data *ch, char *arg,int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
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


void cast_haste(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_haste(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
  case SPELL_TYPE_POTION:
    spell_haste(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if(tar_obj)
      return;
    if(!tar_ch) tar_ch = ch;
    spell_haste(level,ch,tar_ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in haste!");
    break;
  }
}


void cast_translook( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_translook(level, ch, tar_ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in translook!");
  }
}

void cast_telelook( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_telelook(level, ch, tar_ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in telelook!");
  }
}

void cast_teleimage( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_teleimage(level, ch, tar_ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in teleimage!");
  }
}

void cast_age(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_age(level, ch, tar_ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw up in age!");
    break;
  }
}


void cast_ageing(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_ageing(level, ch, tar_ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in ageing!");
  }
}


void cast_fury(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_fury(level, ch, tar_ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in calm!");
    break;
  }
}


void cast_calm( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_calm(level, ch, tar_ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in calm!");
    break;
  }
}


void cast_full_heal( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    act("$n does a full heal on $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
    act("You do a full heal on $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
    spell_full_heal(level, ch, tar_ch, 0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_full_heal(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_full_heal(level,ch,tar_ch,0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in full heal!");
    break;
  }
}

void cast_mystic_heal( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)

{                                                                               
  switch (type) {                                                               
  case SPELL_TYPE_SPELL:                                                        
    act("$n performs a mystic act on $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);   
    act("You perform a mystic act on $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);   
    spell_mystic_heal(level, ch, tar_ch, 0,eff_level);                          
    break;                                                                      
  case SPELL_TYPE_POTION:                                                       
    spell_mystic_heal(level, ch, tar_ch, 0,eff_level);                          
    break;
  default:
    vlog(LOG_DEBUG,"Someones trying to use mystic heal on an unsupported object!");
    break;
  }
}
void cast_scry( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_scry(level, ch, tar_ch, tar_obj,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_scry(level, ch, ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in scry!");
    break;
  }
}


void cast_protection_from_evil( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
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
  default:
    vlog(LOG_DEBUG,"Serious screw-up in protection from evil!");
    break;
  }
}


void cast_nosleep(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_nosleep(level, ch, tar_ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw up in nosleep!");
    break;
  }
}

void cast_heroism(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_heroism(level, ch, tar_ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in heroism!");
    break;
  }
}



void cast_dexterity(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_dexterity(level, ch, tar_ch, 0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_dexterity(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if(tar_ch != ch)
	spell_dexterity(level, ch, tar_ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in dexterity!");
    break;
  }
}


void cast_nocharm(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_nocharm(level, ch, tar_ch, 0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if(!tar_ch) return;
    spell_nocharm(level, ch, tar_ch, 0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_nocharm(level, ch, tar_ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in nocharm!");
    break;
  }
}

void cast_charm_person( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
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
      if (tar_ch != ch)
	spell_charm_person(level,ch,tar_ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in charm person!");
    break;
  }
}


void cast_telemove( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_POTION:
  case SPELL_TYPE_SPELL:
    if (!tar_ch)
      tar_ch = ch;
    spell_telemove(level, ch, tar_ch, 0,eff_level);
    break;

  case SPELL_TYPE_WAND:
    if(!tar_ch) return;
    spell_telemove(level, ch, tar_ch, 0,eff_level);
    break;

  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ;
         tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
        spell_telemove(level, ch, tar_ch, 0,eff_level);
    break;

    default :
      vlog(LOG_DEBUG,"Serious screw-up in telemove!");
    break;
  }
}


void cast_transmove( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_POTION:
  case SPELL_TYPE_SPELL:
    if (!tar_ch)
      tar_ch = ch;
    spell_transmove(level, ch, tar_ch, 0,eff_level);
    break;

  case SPELL_TYPE_WAND:
    if(!tar_ch) return;
    spell_transmove(level, ch, tar_ch, 0,eff_level);
    break;

    case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_transmove(level, ch, tar_ch, 0,eff_level);
    break;

    default :
      vlog(LOG_DEBUG,"Serious screw-up in transmove!");
    break;
  }
}

void cast_vigorize_critic(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_vigorize_critic(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_vigorize_critic(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_vigorize_critic(level,ch,tar_ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in vigorize critic!");
    break;

  }
}


void cast_vitalize_mana(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_vitalize_mana(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_vitalize_mana(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_vitalize_mana(level,ch,tar_ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in vitalize mana!");
    break;

  }
}


void cast_cure_critic( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cure_critic(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_cure_critic(level,ch,ch,0,eff_level);
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

void cast_vigorize_light( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_vigorize_light(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_vigorize_light(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_vigorize_light(level,ch,tar_ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in vigorize light!");
    break;

  }
}

void cast_vigorize_serious( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_vigorize_serious(level,ch,tar_ch,0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_vigorize_serious(level,ch,ch,0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people ;
	 tar_ch ; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_vigorize_serious(level,ch,tar_ch,0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in vigorize serious!");
    break;

  }
}

void cast_summon_demon( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_summon_demon(level, ch, 0, 0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
  case SPELL_TYPE_POTION:
    spell_summon_demon(level, ch, 0, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw up in summon elemental!");
    break;
  }
}

void cast_summon_elemental_ii( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_summon_elemental_ii(level, ch, 0, 0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
  case SPELL_TYPE_POTION:
    spell_summon_elemental_ii(level, ch, 0, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw up in summon elemental 2!");
    break;
  }
}

#if 0
void cast_nofear( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    if ( affected_by_spell(tar_ch, SPELL_NOFEAR)) {
      send_to_char("Nothing seems to happen.\n", tar_ch);
      return;
    }
	spell_nofear(level, ch, tar_ch,0,eff_level);
	break;
      default :
	vlog(LOG_DEBUG,"Serious screw-up in nofear");
	break;
      }
}
#endif


void cast_nosummon( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    if ( affected_by_spell(tar_ch, SPELL_NOSUMMON)) {
      send_to_char("Nothing seems to happen.\n", tar_ch);
      return;
    }
	spell_nosummon(level, ch, tar_ch,0,eff_level);
	break;
      default :
	vlog(LOG_DEBUG,"Serious screw-up in nosummon");
	break;
      }
}


void cast_dispel_invisible(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (tar_obj) {
      if (!IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE))
	send_to_char("Nothing new seems to happen.\n", ch);
      else
	spell_dispel_invisible(level, ch, 0, tar_obj,eff_level);
    } else {
      if (!IS_AFFECTED(tar_ch, AFF_INVISIBLE))
	send_to_char("Nothing new seems to happen.\n", ch);
      else
	spell_dispel_invisible(level, ch, tar_ch, 0,eff_level);
    }
    break;
  case SPELL_TYPE_POTION:
    if (IS_AFFECTED(ch, AFF_INVISIBLE))
      spell_dispel_invisible(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) {
      if (IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE))
	spell_dispel_invisible(level, ch, 0, tar_obj,eff_level);
    } else {
      if (!tar_ch) tar_ch = ch;
      if (IS_AFFECTED(tar_ch, AFF_INVISIBLE))
	spell_dispel_invisible(level, ch, tar_ch, 0,eff_level);
    }
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in dispel invisible!");
    break;
  }
}


void cast_dispel_good( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_dispel_good(level, ch, tar_ch,0,eff_level);
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
      if (tar_ch != ch)
	spell_dispel_good(level,ch,tar_ch,0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in dispel good!");
    break;
  }
}

void cast_chilly( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_chilly(level, ch, victim, 0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    spell_chilly(level, ch, victim, 0,eff_level);
    break;
  default:
      vlog(LOG_DEBUG,"Serious screw-up in chilly!");
    break;
  }
}

void cast_metalskin(int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_metalskin(level, ch, victim, 0,eff_level);
    break;
  default:
      vlog(LOG_DEBUG,"Serious screw-up in metal skin!");
    break;
  }
}

void cast_wither(int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_wither(level, ch, victim, 0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_wither(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    spell_wither(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (victim = real_roomp(ch->in_room)->people ;
         victim ; victim = victim->next_in_room )
      if(victim != ch)
        spell_wither(level, ch, victim, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw up in wither!");
  }
}


void cast_drain_vigor_light( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_drain_vigor_light(level, ch, victim, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw up in drain vigor light");
    break;
  }
}


void cast_drain_vigor_serious( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_drain_vigor_serious(level, ch, victim, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw up in drain vigor serious");
    break;
  }
}



void cast_drain_vigor_critic( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_drain_vigor_critic(level, ch, victim, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw up in drain vigor critic");
    break;
  }
}


void cast_farsee( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_farsee(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_farsee(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    spell_farsee(level, ch, ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw up in farsee");
    break;
  }
}

void cast_vitality( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_vitality(level, ch, victim, 0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_vitality(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for(victim = real_roomp(ch->in_room)->people;
	victim; victim = victim->next_in_room);
    if(victim != ch)
      spell_vitality(level, ch, victim, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw up in vitality");
    break;
  }
}

void cast_rejuvenate(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_rejuvenate(level, ch, tar_ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw up in rejuvenate!");
    break;
  }
}



void cast_recharger( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_recharger(level, ch, victim, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in sanctuary!");
  }
}

void cast_shriek(  int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
     spell_shriek(level, ch, victim, 0,eff_level);
     break;
  case SPELL_TYPE_SCROLL:
    if(victim)
      spell_shriek(level, ch, victim, 0,eff_level);
    else if(!tar_obj)
      spell_shriek(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if(victim)
      spell_shriek(level, ch, victim, 0,eff_level);
    break;
   default:
      vlog(LOG_DEBUG,"Serious screw-up in shriek!");
    break;

   }
 }


void cast_mage_fire( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_mage_fire(level, ch, victim, 0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if(victim)
      spell_mage_fire(level, ch, victim, 0,eff_level);
    else if(!tar_obj)
      spell_mage_fire(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    if(victim)
      spell_mage_fire(level, ch, victim, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw-up in mage-fire!");
    break;
  }
}


void cast_ice_lance( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    spell_ice_lance(level, ch, victim, 0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if(victim)
      spell_ice_lance(level, ch, victim, 0,eff_level);
    else if(!tar_obj)
      spell_ice_lance(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if(victim)
      spell_ice_lance(level, ch, victim, 0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in ice_lance!");
    break;

  }
}

void cast_freeze( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    spell_freeze(level, ch, victim, 0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if(victim)
      spell_freeze(level, ch, victim, 0,eff_level);
    else if(!tar_obj)
      spell_freeze(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if(victim)
      spell_freeze(level, ch, victim, 0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in freeze!");
    break;

  }
}


void cast_fire_bolt( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    spell_fire_bolt(level, ch, victim, 0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if(victim)
      spell_fire_bolt(level, ch, victim, 0,eff_level);
    else if(!tar_obj)
      spell_fire_bolt(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if(victim)
      spell_fire_bolt(level, ch, victim, 0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in fire_bolt!");
    break;

  }
}


void cast_repulsor( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    spell_repulsor(level, ch, victim, 0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if(victim)
      spell_repulsor(level, ch, victim, 0,eff_level);
    else if(!tar_obj)
      spell_repulsor(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if(victim)
      spell_repulsor(level, ch, victim, 0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in repulsor!");
    break;

  }
}


void cast_mind_thrust( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    spell_mind_thrust(level, ch, victim, 0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if(victim)
      spell_mind_thrust(level, ch, victim, 0,eff_level);
    else if(!tar_obj)
      spell_mind_thrust(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if(victim)
      spell_mind_thrust(level, ch, victim, 0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in mind_thrust!");
    break;

  }
}


void cast_disruptor( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    spell_disruptor(level, ch, victim, 0,eff_level);
    break;
  case SPELL_TYPE_SCROLL:
    if(victim)
      spell_disruptor(level, ch, victim, 0,eff_level);
    else if(!tar_obj)
      spell_disruptor(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_WAND:
    if(victim)
      spell_disruptor(level, ch, victim, 0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in disruptor!");
    break;

  }
}


void cast_full_harm( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_full_harm(level, ch, victim, 0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_full_harm(level, ch, ch, 0,eff_level);
    break;
  case SPELL_TYPE_STAFF:
    for (victim = real_roomp(ch->in_room)->people ;
         victim ; victim = victim->next_in_room )
      if(victim != ch)
        spell_full_harm(level, ch, victim, 0,eff_level);
    break;
    default :
      vlog(LOG_DEBUG,"Serious screw-up in full_harm!");
    break;
  }
}


void cast_holy_word ( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_holy_word(level, ch, victim, 0,eff_level);
      break;
    case SPELL_TYPE_POTION:
     spell_holy_word(level, ch, ch, 0,eff_level);
     break;
    default:
     vlog(LOG_DEBUG,"Serious screw-up in holy word!");
     break;
  }
}


void cast_unholy_word ( int level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_unholy_word(level, ch, victim, 0,eff_level);
      break;
    case SPELL_TYPE_POTION:
     spell_unholy_word(level, ch, ch, 0,eff_level);
     break;
    default:
     vlog(LOG_DEBUG,"Serious screw-up in unholy word!");
     break;
  }
}


void cast_sunray( int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_sunray(level, ch, tar_ch, 0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_sunray(level, ch, ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw up in sunray!");
    break;
  }
}

void cast_vampiric_touch(int level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj, int eff_level)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_vampiric_touch(level, ch, tar_ch, 0,eff_level);
    break;
  case SPELL_TYPE_POTION:
    spell_vampiric_touch(level, ch, ch, 0,eff_level);
    break;
  default:
    vlog(LOG_DEBUG,"Serious screw up in vampiric_touch!");
    break;
  }
}



