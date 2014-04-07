/* ************************************************************************,
*  file: constants.c                                      Part of DIKUMUD *
*  Usage: For constants used by the game.                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <structs.h>
#include <limits.h>
#include <trap.h>
#include <spell_list.h>
#include <race.h>

char *spell_wear_off_msg[] = {
  "RESERVED DB.C",
  "You feel less protected.",
  "!Teleport!",
  "You feel less righteous.",
  "You feel a cloak of blindness disolve.",
  "!Burning Hands!",
  "!Call Lightning",
  "You feel more self-confident.",
  "!Chill Touch!",
  "!Clone!",
  "!Color Spray!",
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "!Cure Critic!",
  "!Cure Light!",
  "You feel better.",
  "You sense the red in your vision disappear.",
  "The detect invisible wears off.",
  "The detect magic wears off.",
  "The detect poison wears off.",
  "!Dispel Evil!",
  "!Earthquake!",
  "!Enchant Weapon!",
  "!Energy Drain!",
  "!Fireball!",
  "!Harm!",
  "!Heal",
  "You feel exposed.",
  "!Lightning Bolt!",
  "!Locate object!",
  "!Magic Missile!",
  "You feel less sick.",
  "You feel less protected.",
  "!Remove Curse!",
  "The white aura around your body fades.",
  "!Shocking Grasp!",
  "You feel less tired.",
  "You don't feel as strong.",
  "!Summon!",
  "!Ventriloquate!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You feel less aware of your suroundings.",
  "",  /* NO MESSAGE FOR SNEAK*/
  "!Hide!",
  "!Steal!",
  "!Backstab!",
  "!Pick Lock!",
  "!Kick!",
  "!Bash!",
  "!Rescue!",
  "!Identify!",
  "You feel disoriented as you lose your infravision.",
  "!cause light!",
  "!cause crit!",
  "!flamestrike!",
  "You feel somewhat stronger now...",
  "!dispel good!",
  "!knock!",
  "!know alignment!",
  "!animate dead!",
  "You feel freedom of movement.",
  "!remove paralysis!",
  "!fear!",
  "!acid blast!",     /* */
  "Your shield of force dissapates.",
  "You feel a tightness at your throat. ",
  "You feel heavier now, your flying ability is leaving you.",
  "spell1, please report.",
  "spell2, please report.",
  "spell3, please report.",
  "spell4, please report.",
  "spell5, please report.",
  "spell6, please report.",
  "spell7, please report.",
  "spell8, please report.",
  "spell9, please report.",
  "spell10, please report.",
  "spell11, please report.",
  "The red glow around your body fades",
  "spell13, please report.",
  "spell14, please report.",
  "spell15, please report.",
  "spell16, please report.",
  "spell17, please report.",
  "spell18, please report.",
  "spell19, please report.",
  "spell20, please report.",
  "spell21, please report.",
  "spell22, please report.",
  "Your skin returns to normal.",
  "spell22, please report.",
  "Your clarity of vision dissapears",
  "spell22, please report",
  "The pink glow around your body fades.",
  "Your body returns from stone to normal",
  "!conjure elemental",
  "The silver glow fades from your eyes.",
  "!minor creation",
  "!faerie fire",
  "The purple smoke surrounding you disappears",
  "!cacaodemon",
  "You polymorph back into your true form.",
  "!mana",
  "!astral walk",
  "!resurrection", /* pword blind! */
  "!summon object",
  "!find familiar",
  "!chain lightning",
  "!scare",
  "!sending",
  "!*108",
  "You feel the poison course through your veins at full speed again",
  "!aid",
  "!golem",
  "!command",
  "Your body stops shimmering",
  "!gust of wind",
  "!warp weapon",
  "!pword kill",
  "!mystic heal",
  "You feel less protected from fire",
  "!portal",
  "!shriek",
  "!drain vigor light",
  "!drain vigor serious",
  "!drain vigor critic",
  "!wither",
  "!summon demon",
  "!fury",
  "!summon elemental ii",
  "!mage fire",
  "!firestorm",
  "!full harm",
  "!holy word",
  "!unholy word",
  "!translook",
  "!telelook",
  "!teleimage",
  "!transmove",
  "!telemove",
  "!vitilize mana",
  "!rejuvenate",
  "!age",
  "!ageing",
  "The world returns to its normal speed",
  "!group invisibility",
  "!heroes feast",
  "!full heal",
  "!scry",
  "!nosleep",
  "!nosummon",
  "You feel a bit more clumsey ",
  "!nocharm",
  "!*151",
  "!recharger",
  "!chilly",
  "!sunray",
  "Your skin loses its metal luster",
  "!vampiric touch",
  "!vigorize light",
  "!vigorize serious",
  "!vigorize critic",
  "!vitality",
  "Your eyes lose their enhanced vision",
  "You feel a bit less courageous",
  "!ice lance",
  "!freeze",
  "!fire bolt",
  "!repulsor",
  "!mind thrust",
  "!disruptor",
  "!dispel invisibility",
  "!nofear",
  "!*171",
  "!*172",
  "!*173",
  "You feel less protected from cold",
  "!*175",
  "!*176",
  "!*177",
  "!*178",
  "!*179",
  "!hunt",
  "!find trap",
  "!set trap",
  "!disarm",
  "!read magic",
  "You feel much more calm",
  "!subterfuge",
  "!hitall",
  "!assasinate",
  "!appraise",
  "!extra attack one",
  "!extra attack two",
  "!extra attack three",
  "!extra attack four",
  "!extra attack five",
  "!extra attack six",
  "!extra attack seven",
  "!extra attack eight",
  "!extra attack nine",
  "!green slime",
  "!geyser",
  "!fire breath",
  "!gas breath",
  "!frost breath",
  "!acid breath",
  "!lightning breath",
  "The yellow glow fades from your body",
  "!chain lightning",  /* 207 */
};


int rev_dir[] =
{
	2,
	3,
	0,
	1,
	5,
	4
};

int TrapDir[] =
{
	TRAP_EFF_NORTH,
	TRAP_EFF_EAST,
	TRAP_EFF_SOUTH,
	TRAP_EFF_WEST,
	TRAP_EFF_UP,
	TRAP_EFF_DOWN
};

int movement_loss[]=
{
	1,  /* Inside     */
	2,  /* City       */
	2,  /* Field      */
	3,  /* Forest     */
	4,  /* Hills      */
	6,  /* Mountains  */
        8,  /* Swimming   */
        10, /* Unswimable */
        2,  /* Flying     */
        20,  /* Submarine  */
	6, /* fire */
	6 /* cold */
};

char *dirs[] =
{
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"\n"
};

int opposite_dir[] = { 2, 3, 0, 1, 5, 4 };


char *ItemDamType[] =
{
	"burned",
	"frozen",
	"electrified",
	"crushed",
	"corroded"
};

char *weekdays[7] = {
	"the Day of the Moon",
	"the Day of the Bull",
	"the Day of the Deception",
	"the Day of Thunder",
	"the Day of Freedom",
	"the day of the Great Gods",
	"the Day of the Sun" };

char *month_name[17] = {
	"Month of Winter",           /* 0 */
	"Month of the Winter Wolf",
	"Month of the Frost Giant",
	"Month of the Old Forces",
	"Month of the Grand Struggle",
	"Month of the Spring",
	"Month of Nature",
	"Month of Futility",
	"Month of the Dragon",
	"Month of the Sun",
	"Month of the Heat",
	"Month of the Battle",
	"Month of the Dark Shades",
	"Month of the Shadows",
	"Month of the Long Shadows",
	"Month of the Ancient Darkness",
	"Month of the Great Evil"
};

int sharp[] = {
   0,
   0,
   0,
   1,    /* Slashing */
   0,
   0,
   0,
   0,    /* Bludgeon */
   0,
   0,
   0,
   0 };  /* Pierce   */

char *where[] = {
	"<used as light>      ",
	"<worn on finger>     ",
	"<worn on finger>     ",
	"<worn around neck>   ",
	"<worn around neck>   ",
	"<worn on body>       ",
	"<worn on head>       ",
	"<worn on legs>       ",
	"<worn on feet>       ",
	"<worn on hands>      ",
	"<worn on arms>       ",
	"<worn as shield>     ",
	"<worn about body>    ",
	"<worn about waist>   ",
	"<worn around wrist>  ",
	"<worn around wrist>  ",
	"<wielded>            ",
	"<held>               ",
	"<spell pouch>        "
};

char *drinks[]=
{
	"water",
	"beer",
	"wine",
	"ale",
	"dark ale",
	"whisky",
	"lemonade",
	"firebreather",
	"local speciality",
	"slime mold juice",
	"milk",
	"tea",
	"coffee",
	"blood",
	"salt water",
	"coke",
	"\n"
};

char *drinknames[]=
{
	"water",
	"beer",
	"wine",
	"ale",
	"ale",
	"whisky",
	"lemonade",
	"firebreather",
	"local",
	"juice",
	"milk",
	"tea",
	"coffee",
	"blood",
	"salt",
	"coke",
	"\n"
};

int RacialMax[][4] = {

  {(LOW_IMMORTAL-1),(LOW_IMMORTAL-1),(LOW_IMMORTAL-1),(LOW_IMMORTAL-1)},
  {(LOW_IMMORTAL-1),(LOW_IMMORTAL-1),(LOW_IMMORTAL-1),(LOW_IMMORTAL-1)},
  {(LOW_IMMORTAL-1),(LOW_IMMORTAL-1),(LOW_IMMORTAL-1),(LOW_IMMORTAL-1)},
  {(LOW_IMMORTAL-1),(LOW_IMMORTAL-1),(LOW_IMMORTAL-1),(LOW_IMMORTAL-1)},
  {(LOW_IMMORTAL-1),(LOW_IMMORTAL-1),(LOW_IMMORTAL-1),(LOW_IMMORTAL-1)},
  {(LOW_IMMORTAL-1),(LOW_IMMORTAL-1),(LOW_IMMORTAL-1),(LOW_IMMORTAL-1)}

};

/*  fire cold elec blow acid */

int ItemSaveThrows[22][5] = {
  {15, 2, 10, 10, 10},
  {19, 2, 16, 2, 7},
  {11, 2, 2, 13, 9},
  {7,  2, 2, 10, 8},
  {6,  2, 2, 7, 13},
  {10, 10, 10, 10, 10},  /* not defined */
  {10, 10, 10, 10, 10},  /* not defined */
  {6,  2, 2, 7, 13},  /* treasure */
  {6,  2, 2, 7, 13},  /* armor */
  {7,  6, 2, 20, 5},  /* potion */
  {10, 10, 10, 10, 10},  /* not defined */
  {10, 10, 10, 10, 10},  /* not defined */
  {10, 10, 10, 10, 10},  /* not defined */
  {10, 10, 10, 10, 10},  /* not defined */
  {19, 2, 2, 16, 7},
  {7,  6, 2, 20, 5},  /* drinkcon */
  {6,  2, 2, 7, 13},
  {6,  3, 2, 3, 10},
  {6,  2, 2, 7, 13},  /* treasure */
  {11, 2, 2, 13, 9},
  {7,  2, 2, 10, 8}
};


int drink_aff[][3] = {
	{ 0,24,24 },  /* Water    */ /*{ 0, 1, 10 }*/
	{ 3,2,5 },   /* beer     */
	{ 5,2,5 },   /* wine     */
	{ 2,2,5 },   /* ale      */
	{ 1,2,5 },   /* ale      */
	{ 6,1,4 },   /* Whiskey  */
	{ 0,1,8 },   /* lemonade */
	{ 10,0,0 },  /* firebr   */
	{ 3,3,3 },   /* local    */
	{ 0,4,-8 },  /* juice    */
	{ 0,3,6 },
	{ 0,1,6 },
	{ 0,1,6 },
	{ 0,2,-1 },
	{ 0,1,-2 },
	{ 0,1,5 },
	{ 0, 0, 0}
};

char *color_liquid[]=
{
	"clear",
	"brown",
	"clear",
	"brown",
	"dark",
	"golden",
	"red",
	"green",
	"clear",
	"light green",
	"white",
	"brown",
	"black",
	"red",
	"clear",
	"black",
	"\n"
};

char *fullness[] = {
	"less than half ",
	"about half ",
	"more than half ",
	""
};


#define NONE 0
/* Hey Scarrow -- don't use NULL for something that isn't a pointer -- Alc */

material_t materials[] = {
  /* dens., iron %, flags */
  {  150,   0, NONE },                          /* none */
  {  200,   0, MATF_METAL | MATF_PRECIOUS },    /* adamantium */
  {  270,   0, MATF_METAL },                    /* aluminum */
  {  180,   0, MATF_ORGANIC },                  /* bone */
  {  400,   0, MATF_METAL },                    /* brass */
  {  450,   0, MATF_METAL },                    /* bronze */
  {  351,   0, MATF_MINERAL },                  /* carbon */
  {  240,   0, NONE },                          /* ceramic */
  {   90,   0, NONE },                          /* cloth */
  {  892,   0, MATF_METAL },                    /* copper */
  {  265,   0, MATF_MINERAL },                  /* crystal */
  {   90,   0, MATF_ORGANIC },                  /* fur */
  {  350,   0, MATF_MINERAL | MATF_PRECIOUS },  /* gemstone */
  {  250,   0, NONE },                          /* glass */
  { 1932,   0, MATF_METAL | MATF_PRECIOUS },    /* gold */
  {   92,   0, NONE },                          /* ice */
  {  790, 100, MATF_METAL },                    /* iron */
  { 1134,   0, MATF_METAL },                    /* lead */
  {   86,   0, MATF_ORGANIC },                  /* leather */
  {  270,   0, MATF_METAL | MATF_PRECIOUS },    /* mithril */
  {   94,   0, MATF_ORGANIC },                  /* organic */
  {   80,   0, MATF_ORGANIC },                  /* paper */
  {  130,   0, NONE },                          /* plastic */
  { 2150,   0, MATF_METAL | MATF_PRECIOUS },    /* platinum */
  {  160,   0, MATF_ORGANIC },                  /* scale */
  {   50,   0, MATF_ORGANIC },                  /* silk */
  { 1050,   0, MATF_METAL | MATF_PRECIOUS },    /* silver */
  {  780,  70, MATF_METAL },                    /* steel */
  {  270,   0, MATF_STONE },                    /* stone */
  {  400,   0, MATF_METAL },                    /* tin */
  {  270,   0, MATF_METAL },                    /* titanium */
  {   90,   0, MATF_ORGANIC },                  /* wax */
  {   75,   0, MATF_ORGANIC },                  /* wood */
  {  714,   0, MATF_METAL },                    /* zinc */
  {  200,  30, MATF_METAL | MATF_ORGANIC }      /* banded */
};

char *material_types[] = {
  "none",
  "adamantium",
  "aluminum",
  "bone",
  "brass",
  "bronze",
  "carbon",
  "ceramic",
  "cloth",
  "copper",
  "crystal",
  "fur",
  "gemstone",
  "glass",
  "gold",
  "ice",
  "iron",
  "lead",
  "leather",
  "mithril",
  "organic",
  "paper",
  "plastic",
  "platinum",
  "scale",
  "silk",
  "silver",
  "steel",
  "stone",
  "tin",
  "titanium",
  "wax",
  "wood",
  "zinc",
  "banded",
  "\n"
};


struct weapon_t weapons[] = {
  /* length, flags, speed */
  {    0, NONE,      10 },  /* none */
  {   21, NONE,      14 },  /* dagger */
  {   21, NONE,      14 },  /* knife */
  {   61, NONE,      12 },  /* shortsword */
  {  114, NONE,      10 },  /* longsword */
  {  114, NONE,       9 },  /* broadsword */
  {  167, WEAPF_TWO,  7 },  /* bastard sword */
  {  182, WEAPF_TWO,  6 },  /* claymore */
  {  127, NONE,      11 },  /* scimitar */
  {  127, NONE,      13 },  /* rapier */
  {  114, NONE,      10 },  /* sabre */
  {   91, NONE,       9 },  /* club */
  {  152, WEAPF_TWO,  8 },  /* staff */
  {   76, NONE,       9 },  /* mace */
  {  366, NONE,       6 },  /* whip */
  {   76, NONE,      12 },  /* handaxe */
  {  123, WEAPF_TWO,  8 },  /* battleaxe */
  {  305, WEAPF_TWO,  4 },  /* halberd */
  {  366, WEAPF_TWO,  5 },  /* pike */
  {  244, WEAPF_TWO,  4 },  /* glaive */
  {  168, WEAPF_TWO,  8 },  /* longbow */
  {  107, WEAPF_TWO, 10 },  /* shortbow */
  {  122, WEAPF_TWO,  2 },  /* crossbow */
  {  183, WEAPF_TWO,  9 },  /* spear */
  {  183, WEAPF_TWO, 10 },  /* javelin */
  {  183, WEAPF_TWO,  8 },  /* trident */
  {   91, NONE,       8 },  /* flail */
  {   11, NONE,      15 },  /* dart */
  {   11, NONE,      16 },  /* shuriken */
  {   30, WEAPF_TWO, 14 },  /* claw */
  {  244, WEAPF_TWO,  6 },  /* scythe */
  {  106, WEAPF_TWO,  7 },  /* pick */
  {   91, WEAPF_TWO, 10 },  /* axe */
  {   91, WEAPF_TWO,  9 },  /* warhammer */
  {   61, NONE,      11 }   /* hammer */
};

char *weapon_types[] = {
  "none",
  "dagger",
  "knife",
  "shortsword",
  "longsword",
  "broadsword",
  "bastard",
  "claymore",
  "scimitar",
  "rapier",
  "sabre",
  "club",
  "staff",
  "mace",
  "whip",
  "handaxe",
  "battleaxe",
  "halberd",
  "pike",
  "glaive",
  "longbow",
  "shortbow",
  "crossbow",
  "spear",
  "javelin",
  "trident",
  "flail",
  "dart",
  "shuriken",
  "claw",
  "scythe",
  "pickaxe",
  "axe",
  "warhammer",
  "hammer",
  "\n"
};

char *item_types[] = {
	"undefined",
	"light",
	"scroll",
	"wand",
	"staff",
	"weapon",
	"fire weapon",
	"missile",
	"treasure",
	"armor",
	"potion",
	"worn",
	"other",
	"trash",
	"trap",
	"container",
	"note",
	"liquid container",
	"key",
	"food",
	"money",
	"pen",
	"boat",
	"audio",
	"board",
	"book",
	"pouch",
	"component",
	"\n"
};

char *wear_bits[] = {
	"take",
	"finger",
	"neck",
	"body",
	"head",
	"legs",
	"feet",
	"hands",
	"arms",
	"shield",
	"about",
	"waist",
	"wrist",
	"wield",
	"hold",
	"throw",
	"light",
	"damaged",
	"construction",
	"pouch",
	"\n"
};

char *extra_bits[] = {
	"glow",
	"hum",
	"flexible",
	"mundane",
	"unused",
	"invisible",
	"magic",
	"nodrop",
	"bless",
	"anti-good",
	"anti-evil",
	"anti-neutral",
        "anti-cleric",
        "anti-mage",
        "anti-thief",
        "anti-warrior",
	"anti-ranger", 
	"anti-paladin", 
	"anti-assassin", 
	"anti-ninja", 
	"anti-bard", 
	"anti-cavalier", 
	"anti-avenger", 
	"anti-barbarian", 
	"anti-druid", 
	"anti-illusionist",
	"\n"
};

char *room_bits[] = {
	"dark",
	"death",
	"no_mob",
	"indoors",
	"peaceful",
	"nosteal",
	"no_sum",
	"no_magic",
	"tunnel",
	"private",
	"no_goto",
	"good_only",
	"neutral_only",
	"evil_only",
	"lord_only",
	"rent_room",
	"arena",
	"fast_mana",
	"fast_hit_points",
	"fast_moves",
	"mapable",
      	"noshowexit",
	"fall",
        "good_altar",
	"neutral_altar",
	"evil_altar",
	"\n"
};

char *container_bits[] = {
	"closeable",
	"pickproof",
	"closed",
	"locked",
	"\n"
};

char *exit_bits[] = {
	"door",
	"closed",
	"locked",
	"secret",
	"pickproof",
	"\n"
};

char *sector_types[] = {
	"Inside",
	"City",
	"Field",
	"Forest",
	"Hills",
	"Mountains",
	"Water Swim",
	"Water NoSwim",
        "Air",
        "Underwater",
	"Fire",
	"Cold",
	"Desert",
	"Arctic",
	"Road",
	"\n"
};

char *equipment_types[] = {
	"Special",
	"Worn on right finger",
	"Worn on left finger",
	"First worn around Neck",
	"Second worn around Neck",
	"Worn on body",
	"Worn on head",
	"Worn on legs",
	"Worn on feet",
	"Worn on hands",
	"Worn on arms",
	"Worn as shield",
	"Worn about body",
	"Worn around waist",
	"Worn around right wrist",
	"Worn around left wrist",
	"Wielded",
	"Held",
	"Worn as a spell pouch",
	"\n"
};

char *affected_bits[] =
{	"undefined*",
	"blind",
	"invisible",
	"detect-evil",
	"detect-invisible",
	"detect-magic",
	"sense-life",
	"hold",
	"sanctuary",
	"group",
	"curse",
	"flying",
	"poison",
	"protect-evil",
	"paralysis",
	"infravision",
	"water-breath",
	"sleep",
	"sneak",
	"hide",
	"fear",
	"charm",
	"follow",
	"true-sight",
	"scrying",
	"fireshield",
	"invulnerability",
	"berserk",
	"berserk-recover",
	"nosleep",
	"nosummon",
	"nocharm",
	"haste",
	"recharger",
	"vitality",
	"farsee",
	"nofear",
	"\n"
};

char *immunity_names[] =
{	"fire",
	"cold",
	"electricity",
	"energy",
	"blunt",
	"pierce",
	"slash",
	"acid",
	"poison",
	"drain",
	"sleep",
	"charm",
	"hold",
	"non-magical",
	"plus 1",
	"plus 2",
	"plus 3",
	"plus 4",
	"backstab",
	"20",
	"21",
	"22",
	"23",
	"24",
	"25",
	"26",
	"27",
	"28",
	"29",
	"30",
	"31",
	"32",
	"\n"
};

char *apply_types[] = {
	"NONE",
	"STR",
	"DEX",
	"INT",
	"WIS",
	"CON",
	"SEX",
	"CLASS",
	"LEVEL",
	"AGE",
	"CHAR_WEIGHT",
	"CHAR_HEIGHT",
	"MANA",
	"HIT",
	"MOVE",
	"GOLD",
	"EXP",
	"ALL_AC",
	"HITROLL",
	"DAMROLL",
	"SAVING_PARA",
	"SAVING_ROD",
	"SAVING_PETRI",
	"SAVING_BREATH",
	"SAVING_SPELL",
	"SAVING_ALL",
	"RESISTANCE",
	"SUSCEPTIBILITY",
	"IMMUNITY",
	"SPELL AFFECT",
	"WEAPON SPELL",
	"EAT SPELL",
	"BACKSTAB",
	"KICK",
	"SNEAK",
	"HIDE",
	"BASH",
	"PICK",
	"STEAL",
	"TRACK",
        "HIT-N-DAM",
	"FEET-AC",
	"LEGS-AC",
	"ARMS-AC",
	"BODY-AC",
	"HEAD-AC",
	"ALL-STOPPING",
	"FEET-STOPPING",
	"LEGS-STOPPING",
	"ARMS-STOPPING",
	"BODY-STOPPING",
	"HEAD-STOPPING",
	"BATTER",
	"POWER",
	"\n"
};

char *npc_types[] = {
	"Normal",
	"Undead",
	"\n"
};

char *action_bits[] = {
	"special",
	"sentinel",
	"scavenger",
	"npc",
	"nice-thief",
	"aggressive",
	"stay-zone",
	"wimpy",
        "annoying",
	"hateful",
	"afraid",
	"immortal",
	"hunting",
        "deadly",
	"polymorphed",
	"polymorphed",
	"guarding",
	"citizen",
	"healer",
	"nosummon",
	"\n"
};


char *player_bits[] = {
	"BRIEF",
	"NOSHOUT",
	"COMPACT",
	"DONTSET",
        "WIMPY",
	"NOHASSLE",
	"STEALTH",
	"HUNTING",
	"DEAF",
	"ECHO",
	"",
	"NOVIEWLOG",
	"BANISHED",
	"CRIMINAL",
	"NOTELL",
	"AUCTION",
	"NOSPORTS",
	"GOSSIP",
	"SILENCED",
	"VEG",
	"DEBUG",
	"NOEXITS",
	"COUNCIL",
	"MAYOR",
	"CORRUPT",
	"STUPID",
	"REGEN",
	"PRIORITY",
	"\n"
};


char *position_types[] = {
	"Dead",
	"Mortally wounded",
	"Incapacitated",
	"Stunned",
	"Sleeping",
	"Resting",
	"Praying",
	"Sitting",
	"Fighting",
	"Standing",
	"\n"
};

char *connected_types[]	=	{
	"Playing",
	"Get name",
	"Confirm name",
	"Read Password",
	"Get new password",
	"Confirm new password",
	"Get sex",
	"Read messages of today",
	"Menu",
	"Get extra description",
	"Get class",
	"Link Dead",
	"New password",
	"New password Confirm",
	"Kicking out",
	"Get Race",
	"Unknown?",
	"Confirm Prompt",
	"Get confirmation",
	"Get Order of Stats",
	"Get Align",
	"Rolling Stats",
	"Final Confirmation",
	"Creating Object",
	"Creating Mobile",
	"Prompt for deletion",
	"Prompt for regen mode",
	"Password expired",
	"Password expired confirm",
	"Prompt for Full name",
	"Prompt for Email",
	"Prompt for Terminal",
	"\n"
};

/* [ch] strength apply (all) */
struct str_app_type str_app[31] = {
	{ -5,-4,   0,  0 },  /* 0  */
	{ -5,-4,   3,  1 },  /* 1  */
	{ -3,-2,   3,  2 },
	{ -3,-1,  10,  3 },  /* 3  */
	{ -2,-1,  25,  4 },
	{ -2,-1,  55,  5 },  /* 5  */
	{ -1, 0,  80,  6 },
	{ -1, 0,  90,  7 },
	{  0, 0, 100,  8 },
	{  0, 0, 100,  9 },
	{  0, 0, 115, 10 }, /* 10  */
	{  0, 0, 115, 11 },
	{  0, 0, 140, 12 },
	{  0, 0, 140, 13 },
	{  0, 0, 170, 14 },
	{  0, 0, 170, 15 }, /* 15  */
	{  0, 1, 195, 16 },
	{  1, 1, 220, 18 },
	{  1, 2, 255, 20 }, /* 18  */
	{  3, 7, 640, 40 },
	{  3, 8, 700, 40 }, /* 20  */
	{  4, 9, 810, 40 },
	{  4,10, 970, 40 },
	{  5,11,1130, 40 },
	{  6,12,1440, 40 },
	{  7,14,1750, 40 }, /* 25            */
	{  1, 3, 280, 22 }, /* 18/01-50      */
	{  2, 3, 305, 24 }, /* 18/51-75      */
	{  2, 4, 330, 26 }, /* 18/76-90      */
	{  2, 5, 380, 28 }, /* 18/91-99      */
	{  3, 6, 480, 30 }  /* 18/100   (30) */
};

/* [dex] skillapply (thieves only) */
struct dex_skill_type dex_app_skill[26] = {
	{-99,-99,-90,-99,-60},   /* 0 */
	{-90,-90,-60,-90,-50},   /* 1 */
	{-80,-80,-40,-80,-45},
	{-70,-70,-30,-70,-40},
	{-60,-60,-30,-60,-35},
	{-50,-50,-20,-50,-30},   /* 5 */
	{-40,-40,-20,-40,-25},
	{-30,-30,-15,-30,-20},
	{-20,-20,-15,-20,-15},
	{-15,-10,-10,-20,-10},
	{-10, -5,-10,-15, -5},   /* 10 */
	{ -5,  0, -5,-10,  0},
	{  0,  0,  0, -5,  0},
	{  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0},   /* 15 */
	{  0,  5,  0,  0,  0},
	{  5, 10,  0,  5,  5},
	{ 10, 15,  5, 10, 10},
	{ 15, 20, 10, 15, 15},
	{ 15, 20, 10, 15, 15},   /* 20 */
	{ 20, 25, 10, 15, 20},
	{ 20, 25, 15, 20, 20},
	{ 25, 25, 15, 20, 20},
	{ 25, 30, 15, 25, 25},
	{ 25, 30, 15, 25, 25}    /* 25 */
};

/* [level] backstab multiplyer (thieves only) */
byte backstab_mult[51] ={
	1,   /* 0 */
	2,   /* 1 */
	2,
	2,
	2,
	3,   /* 5 */
	3,
	3,
	3,
	4,
	4,   /* 10 */
	4,
	4,
	4,
	5,
	5,   /* 15 */
	5,
	5,
	5,
	5,
	5,   /* 20 */
	5,
	5,
	5,
	5,    /* 25 */
	5,
	5,
	5,
	5,
	5,   /* 30 */
	5,
	5,
	5,
	5,
	5,   /* 35 */
	5,
	5,
	5,
	5,
	5,  /* 40 */
	5,
	5,
	5,
	5,
	5,  /* 45 */
	5,
	5,
	5,
	5,
	5,
	5  /* 50? */
};

/* [dex] apply (all) */
struct dex_app_type dex_app[26] = {
	{-7,-7, 60},   /* 0 */
	{-6,-6, 50},   /* 1 */
	{-4,-4, 50},
	{-3,-3, 40},
	{-2,-2, 30},
	{-1,-1, 20},   /* 5 */
	{ 0, 0, 10},
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0, 0},   /* 10 */
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0,-10},   /* 15 */
	{ 1, 1,-20},
	{ 2, 2,-30},
	{ 2, 2,-40},
	{ 3, 3,-40},
	{ 3, 3,-40},   /* 20 */
	{ 4, 4,-50},
	{ 4, 4,-50},
	{ 4, 4,-50},
	{ 5, 5,-60},
	{ 5, 5,-60}    /* 25 */
};

/* [con] apply (all) */
struct con_app_type con_app[26] = {
	{-4,20},   /* 0 */
	{-3,25},   /* 1 */
	{-2,30},
	{-2,35},
	{-1,40},
	{-1,45},   /* 5 */
	{-1,50},
	{ 0,55},
	{ 0,60},
	{ 0,65},
	{ 0,70},   /* 10 */
	{ 0,75},
	{ 0,80},
	{ 0,85},
	{ 0,88},
	{ 1,90},   /* 15 */
	{ 2,95},
	{ 3,97},
	{ 3,99},
	{ 4,99},
	{ 5,99},   /* 20 */
	{ 6,99},
	{ 6,99},
	{ 7,99},
	{ 8,99},
	{ 9,100}   /* 25 */
};

/* [int] apply (all) */
struct int_app_type int_app[26] = {
  { 3 },
  { 5 },    /* 1 */
  { 7 },
  { 8 },
  { 9 },
  { 10 },   /* 5 */
  { 11 },
  { 12 },
  { 13 },
  { 15 },
  { 17 },   /* 10 */
  { 19 },
  { 22 },
  { 25 },
  { 30 },
  { 35 },   /* 15 */
  { 40 },
  { 45 },
  { 50 },
  { 53 },
  { 55 },   /* 20 */
  { 56 },
  { 60 },
  { 70 },
  { 80 },
  { 99 }    /* 25 */
};

/* [wis] apply (all) */
struct wis_app_type wis_app[26] = {
 {0},   /* 0 */
 {0},   /* 1 */
 {0},
 {0},
 {0},
 {0},   /* 5 */
 {1},
 {1},
 {1},
 {1},
 {1},   /* 10 */
 {1},
 {2},
 {2},
 {3},
 {3},   /* 15 */
 {3},
 {4},
 {4},   /* 18 */
 {5},
 {5},   /* 20 */
 {6},
 {6},
 {6},
 {6},
 {6}   /* 25 */
};

const char *trap_eff_flags[] = {
	"move",
	"object",
	"room",
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"\n"
};

const int locations[12][10] = {  /* For combat */
 {1,1,1,1,1,2,2,2,2,2,},
 {1,1,1,2,2,2,2,2,2,2,},
 {1,2,2,2,2,3,3,4,4,4,},
 {2,2,2,3,3,3,4,4,4,4,},
 {2,2,3,3,4,4,4,4,4,4,},
 {2,3,3,4,4,4,4,4,4,5,},
 {2,3,4,4,4,4,4,4,4,5,},
 {3,3,4,4,4,4,4,4,5,5,},
 {3,4,4,4,4,4,4,5,5,5,},
 {4,4,4,4,4,5,5,5,5,5,},
 {4,4,4,5,5,5,5,5,5,5,},
 {4,4,5,5,5,5,5,5,5,5,},
};

/* Hit locations for races */
const char *location_text[] = {
  "worn nowhere",
  "worn on head",
  "worn around neck",
  "worn on body",
  "worn on arms",
  "worn on wrist",
  "worn on hands",
  "held in primary",
  "held in secondary",
  "worn on finger",
  "worn around waist",
  "worn on legs",
  "worn on feet",
  "worn about body",
  "worn on tail",
  "worn on wings",
  "held in claw",
  "held in tentacle",
  "worn as shield"
};

const char *location_list[] = {
  "none",
  "head",
  "neck",
  "body",
  "arms",
  "wrist",
  "hands",
  "primary",
  "secondary",
  "finger",
  "waist",
  "legs",
  "feet",
  "about",
  "tail",
  "wings",
  "claw",
  "tentacle",
  "shield",
  "\n"
};

  const char *humanoid_limbs[] = {
      "nowhere",
      "feet",
      "legs",
      "arms",
      "body",
      "head",
    };
  const char *dragon_limbs[] = {
      "nowhere",
      "tail",
      "legs",
      "wings",
      "body",
      "head",
    };
  const char *avian_limbs[] = {
      "nowhere",
      "claws",
      "legs",
      "wings",
      "body",
      "head",
    };
  const char *insectoid_limbs[] = {
      "nowhere",
      "pincers",
      "legs",
      "legs",
      "body",
      "head",
    };


char *damage_types[]= {
"smite",
"stab",
"whip",
"slash",
"smash",
"cleave",
"crush",
"bludgeon",
"claw",
"bite",
"sting",
"pierce",
"\n"
};

const char *class_list[] = {
  "assassin",
  "avenger",
  "barbarian",
  "bard",
  "cavalier",
  "cleric",
  "druid",
  "illusionist",
  "mage",
  "ninja",
  "paladin",
  "ranger",
  "thief",
  "warrior",
  "highwayman",
  "rogue",
  "powermonger",
  "scout",
  "templar",
  "expositor",
  "scholar",
  "necromancer",
  "warlock",
  "warlord",
  "forester",
  "herbalist",
  "celtic",
  "woodsman",
  "overlord",
  "sohei",
  "brigand",
  "dragoon",
  "mercenary",
  "warder",
  "builder",
  "immortal",
  "animal",
  "\n"
};

const char *race_list[] = {
"None",
"Human",
"Elven",
"Dwarf",
"Hobbit",
"Gnome",
"Reptile",
"Special",
"Lycanthrope",
"Dragon",
"Undead",
"Orc",
"Insect",
"Arachnid",
"Dinosaur",
"Fish",
"Bird",
"Giant",
"Predator",
"Parasite",
"Slime",
"Demon",
"Snake",
"Herbiv",
"Tree",
"Veggie",
"Element",
"Planar",
"Devil",
"Ghost",
"Goblin",
"Troll",
"Vegman",
"MindFlayer",
"Primate",
"Enfan",
"Rat",
"Turtle",
"38",
"39",
"Patryn",
"Labrat",
"Sartan",
"Tytan",
"44",
"Mutant",
"Halfbreed",
"Half-Elf",
"Halfling",
"Half-Orc",
"Ogre",
"Pixie",
"Lizardman",
"Snail",
"\n"
};


#if 0
#define ANY (NEUTRAL_ONLY | EVIL_ONLY | GOOD_ONLY)
#define NG  (NEUTRAL_ONLY | GOOD_ONLY)
#define NE  (NEUTRAL_ONLY | EVIL_ONLY)


/* Stat order: STR INT WIS DEX CON CHR LCK */
const min_max r[] = {
  { CLERIC, ANY,
	{  0, 0, 9, 0, 0, 0, 0 }, /* min abils */
	{ 18,18,18,18,18,18,18 }, /* max abils */
	{ 2, 7, 8, 9, 14}, /* up to 5 vnums to load and give to newbie */
	{ RACE_HUMAN, RACE_DWARF, RACE_ELVEN, RACE_H_ELF, RACE_GNOME, RACE_HALFLING, RACE_H_ORC, RACE_PIXIE }}, /* Up to 20 races */
  { MAGE, ANY,
	{  0, 9, 0, 6, 0, 0, 0 }, /* min abils */
	{ 18,18,18,18,18,18,18 }, /* max abils */
	{ 2, 7, 8, 9, 14}, /* up to 5 vnums to load and give to newbie */
	{ RACE_HUMAN, RACE_DWARF, RACE_ELVEN, RACE_H_ELF, RACE_PIXIE }},
  { WARRIOR, ANY,
	{  9, 0, 0, 0, 7, 0, 0 }, /* min abils */
	{ 18,18,18,18,18,18,18 }, /* max abils */
	{ 1, 5, 8, 10, 0}, /* up to 5 vnums to load and give to newbie */
	{ RACE_HUMAN, RACE_DWARF, RACE_ELVEN, RACE_H_ELF, RACE_GNOME, RACE_HALFLING, RACE_H_ORC, RACE_OGRE, RACE_GIANT, RACE_LIZARDMAN, RACE_TROLL }},
  { THIEF, ANY,
	{  0, 0, 0, 9, 0, 0, 0 }, /* min abils */
	{ 18,18,18,18,18,18,18 }, /* max abils */
	{ 3, 9, 2, 0, 0}, /* up to 5 vnums to load and give to newbie */
	{ RACE_HUMAN, RACE_DWARF, RACE_ELVEN, RACE_H_ELF, RACE_GNOME, RACE_HALFLING, RACE_H_ORC, RACE_LIZARDMAN, RACE_TROLL }},
  { WARRIOR | CLERIC, ANY,		/* Multi-Class */
	{ 15, 0,15, 0,14, 0,10 }, /* min abils */
	{ 18,18,18,18,18,18,18 }, /* max abils */
	{ 2, 8, 4, 9, 0}, /* up to 5 vnums to load and give to newbie */
	{ RACE_DWARF, RACE_TROLL }},
  { MAGE | WARRIOR, ANY,
	{ 14,17,10, 0,13, 0,10 }, /* min abils */
	{ 18,18,18,18,18,18,18 }, /* max abils */
	{ 2, 4, 9, 0, 0}, /* up to 5 vnums to load and give to newbie */
	{ RACE_ELVEN, RACE_H_ELF, RACE_OGRE }},
  { MAGE | THIEF, ANY,
	{  0,16,10,17, 0, 0,15 }, /* min abils */
	{ 18,18,18,18,18,18,18 }, /* max abils */
	{ 2, 3, 0, 0, 0}, /* up to 5 vnums to load and give to newbie */
	{ RACE_ELVEN, RACE_H_ELF }},
  { MAGE | CLERIC, ANY,
	{ 10,16,16, 0,13, 0, 0 }, /* min abils */
	{ 18,18,18,18,18,18,18 }, /* max abils */
	{ 2, 7, 9, 0, 0}, /* up to 5 vnums to load and give to newbie */
	{ RACE_H_ELF }},
  { BARBARIAN, ANY,
	{ 15, 0, 0, 0,15, 0, 0 }, /* min abils */
	{ 18,16,18,18,18,18,18 }, /* max abils */
	{ 6, 1, 10, 0, 0}, /* up to 5 vnums to load and give to newbie */
	{ RACE_HUMAN, RACE_DWARF, RACE_GIANT, RACE_LIZARDMAN, RACE_TROLL }},
  { ASSASSIN, EVIL_ONLY,
	{ 12,11, 0,14, 0, 0, 7 }, /* min abils */
	{ 18,18,18,18,18,18,18 }, /* max abils */
	{ 2, 3, 9, 0, 0}, /* up to 5 vnums to load and give to newbie */
	{ RACE_HUMAN, RACE_DWARF, RACE_ELVEN, RACE_H_ELF, RACE_H_ORC, RACE_LIZARDMAN }},
  { PALADIN, GOOD_ONLY,
	{ 12, 9,13,13, 9,17, 5 }, /* min abils */
	{ 18,18,18,18,18,18,18 }, /* max abils */
	{ 1, 5, 8, 11, 0}, /* up to 5 vnums to load and give to newbie */
	{ RACE_HUMAN, RACE_H_ELF }},
  { AVENGER, EVIL_ONLY,
        { 12, 9,13,13, 9,14, 5 }, /* min abils */
	{ 18,18,18,18,18,18,18 }, /* max abils */
	{ 1, 5, 8, 11, 0}, /* up to 5 vnums to load and give to newbie */
	{ RACE_HUMAN, RACE_H_ELF }},
  { DRUID, ANY,
	{  0,12,12, 0, 0,10, 0 }, /* min abils */
	{ 18,18,18,18,18,18,18 }, /* max abils */
	{ 2, 4, 9, 0, 0}, /* up to 5 vnums to load and give to newbie */
        { RACE_HUMAN, RACE_H_ELF, RACE_HALFLING, RACE_PIXIE }},
  { ILLUSIONIST, ANY,
        {  0,14,10, 0, 0,10, 0 },
        { 18,18,18,18,18,18,18 }, /* max abils */
	{ 2, 4, 9, 0, 0}, /* up to 5 vnums to load and give to newbie */
        { RACE_HUMAN, RACE_H_ELF, RACE_GNOME, RACE_PIXIE }},
  { CAVALIER, NG,
        { 12, 9,10,13, 9,15, 0 },
        { 18,18,18,18,18,18,18 },
	{ 1, 5, 9, 8, 0}, /* up to 5 vnums to load and give to newbie */
        { RACE_HUMAN, RACE_H_ELF, RACE_ELVEN }},
  { NINJA, ANY,
       {  0, 0, 0,15,10, 0, 7 },
       { 18,18,18,18,18,18,18 },
       { 2, 4, 9, 0, 0}, /* up to 5 vnums to load and give to newbie */
       { RACE_HUMAN, RACE_ELVEN, RACE_H_ELF }},
  { BARD, NG,
       {  0, 0,13, 0, 0,15, 7 },
       { 18,18,18,18,18,18,18 },
       { 2, 4, 9, 0, 0}, /* up to 5 vnums to load and give to newbie */
       { RACE_HUMAN, RACE_DWARF }},
  { RANGER, ANY,
       { 13, 8, 0, 0,10,13, 5 },
       { 18,18,18,18,18,18,18 },
       { 2, 4, 9, 0, 0}, /* up to 5 vnums to load and give to newbie */
       { RACE_HUMAN, RACE_H_ELF }},
  { RANGER | DRUID, ANY,
       { 13,14,14, 0,10,13, 5 },
       { 18,18,18,18,18,18,18 },
       { 2, 4, 9, 0, 0}, /* up to 5 vnums to load and give to newbie */
       { RACE_H_ELF }},
  { WARRIOR | THIEF, ANY,
       { 15, 0, 0,14,12, 0, 9 },
       { 18,18,18,18,18,18,18 },
       { 2, 4, 9, 0, 0}, /* up to 5 vnums to load and give to newbie */
       { RACE_DWARF, RACE_ELVEN, RACE_H_ELF, RACE_GNOME, RACE_HALFLING, RACE_H_ORC }},
  { WARRIOR | THIEF | MAGE, ANY,
       { 16,16,10,16,13,10,12 },
       { 18,18,18,18,18,18,18 },
       { 2, 4, 9, 0, 0}, /* up to 5 vnums to load and give to newbie */
       { RACE_ELVEN, RACE_H_ELF }},
  { WARRIOR | DRUID, ANY,
       { 14,14,14, 9, 7,10, 0 },
       { 18,18,18,18,18,18,18 },
       { 2, 4, 9, 0, 0}, /* up to 5 vnums to load and give to newbie */
       { RACE_H_ELF }},
  { DRUID | MAGE, ANY,
       {  0,14,14, 6, 0,14, 0 },
       { 18,18,18,18,18,18,18 },
       { 2, 4, 9, 0, 0}, /* up to 5 vnums to load and give to newbie */
       { RACE_H_ELF }},
  { THIEF | ILLUSIONIST, ANY,
       {  0, 19, 9, 9, 0,10,10 },
       { 18,18,18,18,18,18,18 },
       { 2, 4, 9, 0, 0}, /* up to 5 vnums to load and give to newbie */
       { RACE_GNOME }},
  { 0, 0,{ 26,26,26,26,26,26,26 }, {27,27,27,27,27,27,27}, {0 }}
};
#endif
 						     /* min avg max */
racial_adjustment race_adj[] = {                /* height */    /* weight */
 { RACE_HUMAN,    {  0,  0,  0,  0,  0,  0,  0 },{150,170,200},{145,165,200}},
 { RACE_DWARF,    {  0, -1,  0,  0, +1, -1,  0 },{105,120,130},{135,155,175}},
 { RACE_ELVEN,    {  0,  0,  0, +1, -1,  0,  0 },{140,155,170},{ 95,110,125}},
 { RACE_H_ELF,    {  0,  0,  0,  0,  0,  0,  0 },{145,160,175},{110,125,145}},
 { RACE_GNOME,    {  0, +1, -1,  0,  0, -1,  0 },{ 45, 75, 90},{ 55, 75, 95}},
 { RACE_HALFLING, { -1,  0,  0, +1,  0,  0,  0 },{ 50, 80, 95},{ 55, 75, 95}},
 { RACE_H_ORC,    { +1, -1,  0,  0,  0, -2,  0 },{170,181,220},{165,195,235}},
 { RACE_OGRE,     { +1, -1,  0,  0, +1, -1,  0 },{210,225,240},{200,240,300}},
 { RACE_GIANT,    { +2, -2, -2, -1, +2,  0, +1 },{270,300,330},{400,600,800}},
 { RACE_PIXIE,    { -2, +2, +2, +1, -2, -1, +1 },{ 10, 20, 30},{ 15, 25, 35}},
 { RACE_LIZARDMAN,{  0, +1,  0, +2, -1, -2,  0 },{160,180,200},{165,185,200}},
 { RACE_TROLL,    { +1, -1,  0, -1, +1, -1,  0 },{210,225,240},{200,220,240}},
 { RACE_HOBBIT,   {  0,  0,  0,  0,  0,  0,  0 },{ 90,120,150},{ 80,100,120}},
 { RACE_REPTILE,  {  0,  0,  0,  0,  0,  0,  0 },{210,225,240},{200,220,240}},
 { RACE_SPECIAL,  {  0,  0,  0,  0,  0,  0,  0 },{150,170,200},{145,165,200}},
 { RACE_LYCANTH,  {  0,  0,  0,  0,  0,  0,  0 },{150,170,200},{145,165,200}},
 { RACE_DRAGON,   {  0,  0,  0,  0,  0,  0,  0 },{360,550,900},{1000,1500,2000}},
 { RACE_UNDEAD,   {  0,  0,  0,  0,  0,  0,  0 },{210,225,240},{200,220,240}},
 { RACE_ORC,      {  0,  0,  0,  0,  0,  0,  0 },{170,181,220},{165,195,235}},
 { RACE_INSECT,   {  0,  0,  0,  0,  0,  0,  0 },{  5, 10, 20},{  1,  1,  2}},
 { RACE_ARACHNID, {  0,  0,  0,  0,  0,  0,  0 },{ 40, 55, 70},{  5,  7, 14}},
 { RACE_DINOSAUR, {  0,  0,  0,  0,  0,  0,  0 },{360,550,900},{1000,1500,2000}},
 { RACE_FISH,     {  0,  0,  0,  0,  0,  0,  0 },{  5, 10, 20},{  1,  3,  5}},
 { RACE_BIRD,     {  0,  0,  0,  0,  0,  0,  0 },{  5, 10, 20},{  1,  3,  5}},
 { RACE_PREDATOR, {  0,  0,  0,  0,  0,  0,  0 },{170,191,230},{165,195,235}},
 { RACE_PARASITE, {  0,  0,  0,  0,  0,  0,  0 },{  5, 10, 20},{  1,  3,  5}},
 { RACE_SLIME,    {  0,  0,  0,  0,  0,  0,  0 },{  5, 10, 20},{  3,  5,  8}},
 { RACE_DEMON,    {  0,  0,  0,  0,  0,  0,  0 },{300,360,450},{200,350,500}},
 { RACE_SNAKE,    {  0,  0,  0,  0,  0,  0,  0 },{  5, 10, 20},{  3,  5,  8}},
 { RACE_HERBIV,   {  0,  0,  0,  0,  0,  0,  0 },{170,191,230},{165,195,235}},
 { RACE_TREE,     {  0,  0,  0,  0,  0,  0,  0 },{360,550,900},{1000,1200,1500}},
 { RACE_VEGGIE,   {  0,  0,  0,  0,  0,  0,  0 },{170,191,230},{165,195,235}},
 { RACE_ELEMENT,  {  0,  0,  0,  0,  0,  0,  0 },{300,360,400},{200,300,450}},
 { RACE_PLANAR,   {  0,  0,  0,  0,  0,  0,  0 },{300,360,400},{200,300,450}},
 { RACE_DEVIL,    {  0,  0,  0,  0,  0,  0,  0 },{300,360,450},{200,350,500}},
 { RACE_GHOST,    {  0,  0,  0,  0,  0,  0,  0 },{ 90,120,150},{  1,  1,  1}},
 { RACE_GOBLIN,   {  0,  0,  0,  0,  0,  0,  0 },{ 90,120,150},{ 80,100,120}},
 { RACE_VEGMAN,   {  0,  0,  0,  0,  0,  0,  0 },{170,191,230},{165,195,235}},
 { RACE_MFLAYER,  {  0,  0,  0,  0,  0,  0,  0 },{170,191,230},{165,195,235}},
 { RACE_PRIMATE,  {  0,  0,  0,  0,  0,  0,  0 },{150,170,200},{145,165,200}},
 { RACE_ENFAN,    {  0,  0,  0,  0,  0,  0,  0 },{ 90,120,150},{ 80,100,120}},
 { RACE_RAT,      {  0,  0,  0,  0,  0,  0,  0 },{  5, 10, 20},{  1,  2,  3}},
 { RACE_TURTLE,   {  0,  0,  0,  0,  0,  0,  0 },{ 10, 20, 30},{  2,  4,  8}},
 { RACE_PATRYN,   {  0,  0,  0,  0,  0,  0,  0 },{170,181,220},{165,195,235}},
 { RACE_LABRAT,   {  0,  0,  0,  0,  0,  0,  0 },{  5, 10, 20},{  1,  3,  5}},
 { RACE_SARTAN,   {  0,  0,  0,  0,  0,  0,  0 },{170,191,230},{165,195,235}},
 { RACE_TYTAN,    {  0,  0,  0,  0,  0,  0,  0 },{280,320,360},{400,650,900}},
 { RACE_MUTANT,   {  0,  0,  0,  0,  0,  0,  0 },{150,170,200},{145,165,200}},
 { RACE_HALFBREED,{  0,  0,  0,  0,  0,  0,  0 },{150,170,200},{145,165,200}},
 { RACE_SNAIL,    {  0,  0,  0,  0,  0,  0,  0 },{  5, 10, 20},{  1,  1,  2}},
 { 0, {0, 0, 0, 0, 0, 0, 0},{0,0,0},{0,0,0}}
};
