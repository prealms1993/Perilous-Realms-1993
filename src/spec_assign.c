/* ************************************************************************
*  file: spec_assign.c , Special module.                  Part of DIKUMUD *
*  Usage: Procedures assigning function pointers.                         *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>

#include <fcns.h>
#include <externs.h>
#include <db.h>
#include <rooms.h>
#include <utils.h>

extern struct hash_header room_db;

struct special_proc_entry {
  int	vnum;
  int	(*proc)( struct char_data *, int,char *);
};

/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{

  static struct special_proc_entry specials[] = {

    { 1, puff },
    { 2, Ringwraith },
    { 3, tormentor },
    { 4, Inquisitor},
    { 25, magic_user },

    {199, AGGRESSIVE},
/*
**  D&D standard
*/

    { 210, snake },	   /* spider */
    { 223, ghoul },	   /* ghoul */
    { 236, ghoul },	    /* ghast */
    { 225, regenerator },	/* wererat */
    { 227, snake },	/* spider */
    { 230, BreathWeapon }, /* baby black */
    { 232, blink },       /* blink dog */
    { 233, BreathWeapon }, /* baby blue */
    { 237, regenerator }, /* werewolf  */
    { 239, shadow },      /* shadow    */
    { 240, snake },       /* toad      */
    { 243, BreathWeapon }, /* teenage white */
    { 246, regenerator }, /* wereboar */
    { 261, regenerator },
    { 271, regenerator },
    { 248, snake },       /* snake       */
    { 249, snake },       /* snake       */
    { 250, snake },       /* snake       */
    { 257, magic_user },  /* magic_user  */
    { 750, cityguard }, /*shadow officer */
	
/*
**   shire
*/
    { 1000, magic_user },
    { 1031, receptionist },

/*
**  chessboard
*/
    { 1499, sisyphus }, 
    { 1471, paramedics }, 
    { 1470, jabberwocky },
    { 1472, flame }, 
    { 1437, banana }, 
    { 1428, jugglernaut },
    { 1495, delivery_elf },  
    { 1493, delivery_beast },

/*
**  abyss
*/
    { 25000, magic_user }, /* Demi-lich  */
    { 25001, Keftab }, 
    { 25002, vampire },	/* Crimson */
    { 25003, StormGiant }, /* MistDaemon */
    { 25006, StormGiant }, /* Storm giant */
    { 25014, StormGiant }, /* DeathKnight */    
    { 25009, BreathWeapon }, /* hydra */
/*    { 25017, AbyssGateKeeper },*/ /* Abyss Gate Keeper */
    { 25017, abyss_guard }, /*abyss gatekeeper guards lvl 50 - 100 */


/*
**  Paladin's guild
*/
    { 25100, PaladinGuildGuard},
    { 25101, PaladinGuildGuard},

/*
**  Abyss Fire Giants
*/
    { 25500, Fighter },
    { 25501, Fighter },
    { 25502, Fighter },
    { 25505, Fighter },
    { 25504, BreathWeapon},
    { 25503, cleric  },

/*
**  Temple Labrynth
*/

    { 10900, temple_labrynth_liar },
    { 10901, temple_labrynth_liar },
    { 10902, temple_labrynth_sentry},

/*
**  Main City
*/

    { 3000, magic_user }, 
    { 3060, cityguard }, 
    { 3067, cityguard }, 
    { 3061, janitor },
    { 3062, fido }, 
    { 3066, fido },
    { 3005, receptionist },
    { 3020, GuildMaster }, 
    { 3021, GuildMaster }, 
    { 3022, GuildMaster }, 
    { 3023, GuildMaster }, 
    { 3028, GuildMaster }, 
    { 3029, GuildMaster }, 
    { 3030, GuildMaster }, 
    { 3031, GuildMaster }, 
    { 3032, GuildMaster }, 
    { 3033, GuildMaster }, 
    { 3034, GuildMaster }, 
    { 3035, GuildMaster }, 
    { 3036, GuildMaster }, 
    { 3037, GuildMaster }, 

    { 3024, guild_guard }, 
    { 3025, guild_guard }, 
    { 3026, guild_guard },
    { 3027, guild_guard },
    { 3070, RepairGuy }, 
    { 3071, RepairGuy },
    { 1, WeapRepairGuy },
    { 3069, cityguard },	/* post guard */

#if 0
    { 3068, ninja_master },

/*
**  Lower city
*/
    { 3143, mayor },
/*
**   Hammor's Stuff
*/
    { 3900, eric_johnson },
    { 3910, andy_wilcox },
    { 3950, zombie_master },
#endif
    { 3952, BreathWeapon },

/* 
**  MORIA 
*/
    { 4000, snake }, 
    { 4001, snake }, 
    { 4053, snake },

    { 4103, thief }, 
    { 4100, magic_user }, 
    { 4101, regenerator },
    { 4102, snake },

/*
**  Pyramid
*/

    { 5308, RustMonster },
    { 5303, vampire },

/*
**  Arctica
*/
    { 6801, BreathWeapon },
    { 6802, BreathWeapon },
    { 6815, magic_user },
    { 6821, snake },
    { 6824, BreathWeapon },
    { 6825, thief },

/* 
** SEWERS 
*/
    { 7006, snake },
    { 7008, snake },
    { 7042, magic_user },	/* naga       */
    { 7040, BreathWeapon },     /* Red    */
    { 7041, magic_user },	/* sea hag    */
    { 7200, magic_user },	/* mindflayer */ 
    { 7201, magic_user },	/* senior     */
    { 7202, magic_user },	/* junior     */

/* 
** FOREST 
*/

    { 6111, magic_user },	/* tree */
    { 6113, snake },
    { 6114, snake },
    { 6112, BreathWeapon }, /* green */
    { 6910, magic_user },

/*
**  Great Eastern Desert
*/
    { 5000, thief },	/* rag. dervish */
    { 5002, snake },	/* coral snake */
    { 5003, snake },	/* scorpion    */
    { 5004, snake },	/* purple worm  */
    { 5014, cleric },	/* myconoid */
    { 5005, BreathWeapon }, /* brass */

    { 5010, magic_user },	/* dracolich */
    { 5104, cleric },
    { 5103, magic_user },	/* drow mage */
    { 5107, cleric },	/* drow mat. mot */
    { 5108, magic_user },	/* drow mat. mot */
    { 5109, cleric },	/* yochlol */

/*
**   Thalos
*/
    { 5200, magic_user },	/* beholder    */

/*
**  Zoo
*/
    { 9021, snake },	/* Gila Monster */

/*
**  Castle Python
*/

    { 11016, receptionist },
    { 11017, NudgeNudge },

/*
**  miscellaneous
*/
    { 9061, vampire},	/* vampiress  */

/*
**  White Plume Mountain
*/

    { 17004, magic_user }, /* gnyosphinx   */
    { 17017, magic_user }, /* ogre magi   */
    { 17014, ghoul },	/* ghoul  */
    { 17009, geyser },	/* geyser  */
    { 17011, vampire },	/* vampire Amelia  */
    { 17002, wraith },	/* wight*/
    { 17005, shadow },	/* shadow */
    { 17010, green_slime }, /* green slime */

/*
**  Arachnos
*/
    { 20001, snake },	/* Young (large) spider */
    { 20003, snake },	/* wolf (giant) spider  */
    { 20005, snake },	/* queen wasp      */
    { 20006, snake },	/* drone spider    */
    { 20010, snake },	/* bird spider     */
    { 20009, magic_user }, /* quasit         */
    { 20014, magic_user }, /* Arachnos        */
    { 20015, magic_user }, /* Ki Rin          */

    { 20002, BreathWeapon }, /* Yevaud */
    { 20017, BreathWeapon }, /* Elder  */
    { 20016, BreathWeapon }, /* Baby   */

/*
**   The Darklands
*/

    { 24050, cleric },
    { 25013, Tytan }, /* Fix Later */

/*
**   Abbarach
*/
    { 27001, magic_user },
    { 27002, magic_user },
    { 27003, magic_user },
    { 27004, magic_user },
    { 27005, magic_user },
    { 27006, Tytan },
    { 27007, replicant },
    { 27016, AbbarachDragon },
    { 27014, magic_user },
    { 27017, magic_user },
    { 27018, magic_user },
    { 27019, magic_user },
#if 0
    { 3055, tax_collector },
#endif
    { 3054, insurance_salesman },

/*
** Tombs of Toran
*/
    { 33000, tarin },
    { 33005, tarin_good },
    { 33006, tarin_good },
    { 33007, tarin_good },
    { 33016, god },
    { -1, NULL },
  };

  int	i;
  index_mem *h;
  char buf[MAX_STRING_LENGTH];

  for (i=0; specials[i].vnum>=0; i++) {
    h= real_mobp(specials[i].vnum);
    if (!h) {
      sprintf(buf, "mobile_assign: Mobile %d not found in database.",
	      specials[i].vnum);
      log(buf);
    } else {
      if (h->func) nlog("mob_assign: Mob %d already assigned.",specials[i].vnum);
      h->func = specials[i].proc;
      SET_BIT(h->mob->specials.act,ACT_SPEC);
    }
  }

	boot_shp();
	assign_the_shopkeepers();
}



/* assign special procedures to objects */
void assign_objects(void)
{
  if (real_objp(31))
    real_objp(31)->func    = portal;
  InitBoards();
}



/* assign special procedures to rooms */
void assign_rooms(void)
{
  static struct special_proc_entry specials[] = {
    { Bounce_Room,bounce },
    { Dump, dump },
    { Dump1, dump },
    { Alter, pray_for_items },
    { Hospital, hospital },
    { Bank, bank },

    { -1, NULL},
  };
  int i;
  struct room_data *rp;
  
  for (i=0; specials[i].vnum>=0; i++) {
    rp = real_roomp(specials[i].vnum);
    if (rp==NULL) {
      log("assign_rooms: unknown room");
    } else
      rp->funct = specials[i].proc;
  }
}
