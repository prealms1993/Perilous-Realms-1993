/* ************************************************************************
*  file: structs.h , Structures        .                  Part of DIKUMUD *
*  Usage: Declarations of central data structures                         *
************************************************************************* */
#ifndef _STRUCTSH_
#define _STRUCTSH_


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>

#include <compat.h>
typedef signed char sbyte;
typedef unsigned char ubyte;
typedef signed int sh_int;
typedef unsigned int ush_int;
typedef unsigned short int u16;
typedef signed short int s16;
typedef unsigned long int u32;
#if 1
typedef unsigned long int ulong;
#endif
typedef signed long int s32;
typedef unsigned char u8;
typedef signed char s8;
typedef char bool;
typedef signed char byte;
typedef signed long slong;

typedef char           int8;
typedef unsigned char  uint8;
typedef short          int16;
typedef unsigned short uint16;
typedef int            int32;
typedef unsigned int   uint32;
typedef char           *string;

#define BADDOMS 16
#define MAX_CLASS 32
#define MAX_SNOOP 5
#define MAX_KILL_KEEP 100
#define ASSASINATE_LEVEL_GAIN 5
#define MAX_ASSASINATE_SKILL 15

/* EVENT types */
#define EV_ROOM_FLAG    1
#define EV_CHAR_DATA    2
#define EV_FUNC         3
#define EV_OBJECT	4

/* Scarrow's event stuff */
#define EVT_REMOVED -1
#define EVT_SPELL    1
#define EVT_SKILL    2
#define EVT_STRIKE   3
#define EVT_MOBILE   4
#define EVT_DEATH    5
#define EVT_RIVER    6
#define EVT_TELEPORT 7

typedef struct event_t {
  int              type;
  struct char_data *ch;
  struct char_data *victim;
  struct obj_data  *obj;
  int              room;
  int              virtual;
  char             *args;
  void             (*function)(struct event_t *);
  struct event_t   *next;
} event_t;

/* new stuff for poof in and out */
#define BIT_POOF_IN  1
#define BIT_POOF_OUT 2

#define PULSE_COMMAND   0
#define PULSE_TICK      1

#define DROP_TICKS     30

#define LOG_CONNECTION	(1<<0)
#define LOG_MORTSPY	(1<<1)
#define LOG_IMMSPY	(1<<2)
#define LOG_URGENT	(1<<3)
#define LOG_DEBUG	(1<<4)
#define LOG_REJECT	(1<<5)
#define LOG_MISC	(1<<6)
#define LOG_DEATH	(1<<7)



#define ASSASSIN  0
#define AVENGER   1
#define BARBARIAN 2
#define BARD 3
#define CAVALIER 4
#define CLERIC 5
#define DRUID 6
#define ILLUSIONIST 7
#define MAGE 8
#define NINJA 9
#define PALADIN 10
#define RANGER 11
#define THIEF 12
#define WARRIOR 13
#define HIGHWAYMAN 14
#define ROGUE 15
#define POWERMONGER 16
#define SCOUT 17
#define TEMPLAR 18
#define EXPOSITOR 19
#define SCHOLAR 20
#define NECROMANCER 21
#define WARLOCK 22
#define WARLORD 23
#define FORESTER 24
#define HERBALIST 25
#define CELTIC 26
#define WOODSMAN 27
#define OVERLORD 28
#define SOHEI 29
#define BRIGAND 30
#define DRAGOON 31
#define MERCENARY 32
#define WARDER 33
#define BUILDER 34
#define CLASS_IMMORTAL 35

/* end of classes */

#define FIRE_DAMAGE 1
#define COLD_DAMAGE 2
#define ELEC_DAMAGE 3
#define BLOW_DAMAGE 4
#define ACID_DAMAGE 5

#define HATE_SEX   1
#define HATE_RACE  2
#define HATE_CHAR  4
#define HATE_CLASS 8
#define HATE_EVIL  16
#define HATE_GOOD  32
#define HATE_VNUM  64

#define FEAR_SEX   1
#define FEAR_RACE  2
#define FEAR_CHAR  4
#define FEAR_CLASS 8
#define FEAR_EVIL  16
#define FEAR_GOOD  32
#define FEAR_VNUM  64

#define OP_SEX   1
#define OP_RACE  2
#define OP_CHAR  3
#define OP_CLASS 4
#define OP_EVIL  5
#define OP_GOOD  6
#define OP_VNUM  7

#define ABS_MAX_LVL  2011
#define MAXLVLMORTAL 2000
#define MAX_MORT     2000
#define LOW_IMMORTAL 2001
#define IMMORTAL     2001
#define REAL_IMMORT  2002
#define CREATOR      2003
#define SAINT        2004
#define DEMIGOD      2005
#define LESSER_GOD   2006
#define GOD          2007
#define GREATER_GOD  2008
#define SILLYLORD    2009
#define IMPLEMENTOR  2010

#define IMM_FIRE        1
#define IMM_COLD        2
#define IMM_ELEC        4
#define IMM_ENERGY      8
#define IMM_BLUNT      16
#define IMM_PIERCE     32
#define IMM_SLASH      64
#define IMM_ACID      128
#define IMM_POISON    256
#define IMM_DRAIN     512
#define IMM_SLEEP    1024
#define IMM_CHARM    2048
#define IMM_HOLD     4096
#define IMM_NONMAG   8192
#define IMM_PLUS1   16384
#define IMM_PLUS2   32768
#define IMM_PLUS3   65536
#define IMM_PLUS4  131072
#define IMM_BACKSTAB 262144

#define MAX_ROOMS   5000


typedef struct kill_info {
  ulong vnum;
  u16 nkills;
} kill_info;

struct nodes
{
  slong visited;
  slong ancestor;
};

struct room_q
{
  slong room_nr;
  struct room_q *next_q;
};

struct string_block {
  int	size;
  char	*data;
};


/* memory stuff */

struct char_list {
  struct char_data *op_ch;
  char name[50];
  struct char_list *next;
};

typedef struct {
       struct char_list  *clist;
       int    sex;   /*number 1=male,2=female,3=both,4=neut,5=m&n,6=f&n,7=all*/
       int    race;  /*number */
       u8     class;
       slong  vnum;  /* # */
       int    evil;  /* align < evil = attack */
       int    good;  /* align > good = attack */
}  Opinion;




/*
   old stuff.
*/

#define MAX_STRING_LENGTH   16384
#define MAX_INPUT_LENGTH     241
#define MAX_MESSAGES         100
#define MAX_ITEMS            153

#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3

/* The following defs are for obj_data  */

/* For 'type_flag' */

#define ITEM_LIGHT      1
#define ITEM_SCROLL     2
#define ITEM_WAND       3
#define ITEM_STAFF      4
#define ITEM_WEAPON     5
#define ITEM_FIREWEAPON 6
#define ITEM_MISSILE    7
#define ITEM_TREASURE   8
#define ITEM_ARMOR      9
#define ITEM_POTION    10 
#define ITEM_WORN      11
#define ITEM_OTHER     12
#define ITEM_TRASH     13
#define ITEM_TRAP      14
#define ITEM_CONTAINER 15
#define ITEM_NOTE      16
#define ITEM_DRINKCON  17
#define ITEM_KEY       18
#define ITEM_FOOD      19
#define ITEM_MONEY     20
#define ITEM_PEN       21
#define ITEM_BOAT      22
#define ITEM_AUDIO     23
#define ITEM_BOARD     24
#define ITEM_BOOK   25
#define ITEM_SPELL_POUCH 26
#define ITEM_COMPONENT 27

/* Bitvector For 'wear_flags' */

#define ITEM_TAKE              1 
#define ITEM_WEAR_FINGER       2
#define ITEM_WEAR_NECK         4
#define ITEM_WEAR_BODY         8
#define ITEM_WEAR_HEAD        16
#define ITEM_WEAR_LEGS        32
#define ITEM_WEAR_FEET        64
#define ITEM_WEAR_HANDS      128 
#define ITEM_WEAR_ARMS       256
#define ITEM_WEAR_SHIELD     512
#define ITEM_WEAR_ABOUT     1024 
#define ITEM_WEAR_WAISTE    2048
#define ITEM_WEAR_WRIST     4096
#define ITEM_WIELD          8192
#define ITEM_HOLD          16384
#define ITEM_THROW         32768
#define ITEM_LIGHT_NOT_USED	(1<<16)
#define ITEM_DAMAGED	(1<<17)		/* can not wear or wield */
#define ITEM_CONSTRUCTION (1<<18)
#define ITEM_POUCH		(1<<19)

/* Bitvector for 'extra_flags' */

#define ITEM_GLOW            1
#define ITEM_HUM             2
#define ITEM_FLEXIBLE        4  /* volume grows with contents */
#define ITEM_MUNDANE         8  /* unenchantable, but not magic */
#define ITEM_ORGANIC        16  /* undefined?    */
#define ITEM_INVISIBLE      32
#define ITEM_MAGIC          64
#define ITEM_NODROP        128
#define ITEM_BLESS         256
#define ITEM_ANTI_GOOD     512 /* not usable by good people    */
#define ITEM_ANTI_EVIL    1024 /* not usable by evil people    */
#define ITEM_ANTI_NEUTRAL 2048 /* not usable by neutral people */
#define ITEM_ANTI_CLERIC  4096
#define ITEM_ANTI_MAGE    8192
#define ITEM_ANTI_THIEF   16384
#define ITEM_ANTI_FIGHTER 32768
#define ITEM_ANTI_RANGER         (1<<16)
#define ITEM_ANTI_PALADIN        (1<<17)
#define ITEM_ANTI_ASSASSIN       (1<<18)
#define ITEM_ANTI_NINJA          (1<<19)
#define ITEM_ANTI_BARD           (1<<20)
#define ITEM_ANTI_CAVALIER       (1<<21)
#define ITEM_ANTI_AVENGER        (1<<22)
#define ITEM_ANTI_BARBARIAN      (1<<23)
#define ITEM_ANTI_DRUID          (1<<24)
#define ITEM_ANTI_ILLUSIONIST    (1<<25)
#define ITEM_ANTI_BIG		 (1<<26)
#define ITEM_ANTI_MEDIUM	 (1<<27)
#define ITEM_ANTI_SMALL		 (1<<28)

#define HEIGHT_BIG    200
#define HEIGHT_SMALL  150

/* Some different kind of liquids */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_COKE       15

/* for containers  - value[1] */

#define CONT_CLOSEABLE      1
#define CONT_PICKPROOF      2
#define CONT_CLOSED         4
#define CONT_LOCKED         8

/* material flags */
#define MATF_STONE    1
#define MATF_METAL    2
#define MATF_ORGANIC  4
#define MATF_MINERAL  8
#define MATF_PRECIOUS 16

/* materials */
#define MAT_NONE       0
#define MAT_ADAMANTIUM 1
#define MAT_ALUMINUM   2
#define MAT_BONE       3
#define MAT_BRASS      4
#define MAT_BRONZE     5
#define MAT_CARBON     6
#define MAT_CERAMIC    7
#define MAT_CLOTH      8
#define MAT_COPPER     9
#define MAT_CRYSTAL    10
#define MAT_FUR        11
#define MAT_GEMSTONE   12
#define MAT_GLASS      13
#define MAT_GOLD       14
#define MAT_ICE        15
#define MAT_IRON       16
#define MAT_LEAD       17
#define MAT_LEATHER    18
#define MAT_MITHRIL    19
#define MAT_ORGANIC    20
#define MAT_PAPER      21
#define MAT_PLASTIC    22
#define MAT_PLATINUM   23
#define MAT_SCALE      24
#define MAT_SILK       25
#define MAT_SILVER     26
#define MAT_STEEL      27
#define MAT_STONE      28
#define MAT_TIN        29
#define MAT_TITANIUM   30
#define MAT_WAX        31
#define MAT_WOOD       32
#define MAT_ZINC       33

#define DMG_BLUNT    0
#define DMG_PIERCE   1
#define DMG_SLASH    2
#define DMG_FIRE     3
#define DMG_HEAT     4
#define DMG_COLD     5
#define DMG_SHOCK    6
#define DMG_ACID     7
#define DMG_WATER    8
#define DMG_MAGIC    9
#define NUM_DMG      10

/* susceptibilities/resistances */
typedef struct susres_t {
  int8   value;
  string message;
  int32  become;
} susres_t;

/* material information ... susres to come later */
typedef struct material_t {
  int             density;
  int             iron;
  int             flags;
  struct susres_t susres[NUM_DMG];
} material_t;

#define WEAPF_TWO 1

/* weapon information */
typedef struct weapon_t {
  int length;
  int flags;
  int speed;
} weapon_t;

struct extra_descr_data
{
	char *keyword;                 /* Keyword in look/examine          */
	char *description;             /* What to see                      */
	struct extra_descr_data *next; /* Next in list                     */
};

#define MAX_OBJ_AFFECT 5
#define OBJ_NOTIMER    -7000000
#define MAX_VALUE 5

struct obj_flag_data
{
	int value[MAX_VALUE];       /* Values of the item (see list)    */
	byte type_flag;     /* Type of item                     */
	ulong wear_flags;     /* Where you can wear it            */
	ulong extra_flags;    /* If it hums,glows etc             */
	int weight;         /* Weigt what else                  */
        int mass;           /* mass in grams */
        int volume;         /* volume in cm^3 */
        int length;         /* length in cm */
	int cost;           /* Value when sold (gp.)            */
	int cost_per_day;   /* Cost to keep pr. real day        */
	int timer;          /* Timer for object                 */
	int timer2; /* for object desrtuction */
	ulong bitvector[8];     /* To set chars bits                */
	ulong xtra_bits;
};

/* do not change */
struct obj_affected_type {
	byte location;      /* Which ability to change (APPLY_XXX) */
	s32 modifier;       /* How much it changes by              */
};

/* ======================== Structure for object ========================= */
struct obj_data
{
	slong virtual;			/* virtual num */
	slong  in_room;                /* In what room -1 when conta/carr  */ 
	struct obj_flag_data obj_flags;/* Object information               */
	struct obj_affected_type
	    affected[MAX_OBJ_AFFECT];  /* Which abilities in PC to change  */

	struct char_data *killer;      /* for use with corpses */
	char *name;                    /* Title of object :get etc.        */
	char *description ;            /* When in room                     */
	char *short_description;       /* when worn/carry/in cont.         */
 	char *action_description;      /* What to write when used          */
	char *held_for;	/* for insurance guy -- who owns this */
 	struct extra_descr_data *ex_description; /* extra descriptions     */
	struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
	int min_level;                 /* for min usable level */
        int material;
	u32 time_stamp;		       /* for warehouse */
	struct char_data *in_warehouse; /* ditto */
	sh_int    eq_pos;                 /* what is the equip. pos?          */
	struct char_data *equipped_by; /* equipped by :NULL in room/conta  */

	struct obj_data *in_obj;       /* In what object NULL when none    */
	struct obj_data *contains;     /* Contains objects                 */

	struct obj_data *next_content; /* For 'contains' lists             */
	struct obj_data *next;         /* For the object list              */
};
/* ======================================================================= */

/* The following defs are for room_data  */

#define NOWHERE    -1    /* nil reference for room-database    */

/* Constants for room affects */

#define RAFF_TRAP      1

/* Constants for room affects -> traps */

#define TRAP_PRESSURE   1

/* Constants for trap effects */

#define TEFF_DAMAGE     1
#define TEFF_SPELL      2

/* Bitvector For 'room_flags' */

#define DARK           1
#define DEATH          2
#define NO_MOB         4
#define INDOORS        8
#define PEACEFUL      16  /* No fighting */
#define NOSTEAL       32  /* No Thieving */
#define NO_SUM        64  /* no summoning */
#define NO_MAGIC     128
#define TUNNEL       256 /* ? */
#define PRIVATE      512
#define NO_GOTO      (1<<10)
#define GOOD_ONLY	(1<<11)
#define NEUTRAL_ONLY	(1<<12)
#define EVIL_ONLY	(1<<13)
#define LORD_ONLY       (1<<14)
#define RENTABLE	(1<<15)
#define ARENA		(1<<16)
#define FAST_MANA	(1<<17) /* point regeneration flags */
#define FAST_HIT	(1<<18)
#define FAST_MOVE	(1<<19)
#define MAPABLE		(1<<20)
#define NOSHOWEXIT	(1<<21)	/* don't display exit list */
#define FALL            (1<<22) /* Deths great fall flag */
#define GOOD_ALTAR      (1<<23)
#define NEUTRAL_ALTAR   (1<<24)
#define EVIL_ALTAR      (1<<25)

/* For 'dir_option' */

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5

#define EX_ISDOOR      	1
#define EX_CLOSED      	2
#define EX_LOCKED      	4
#define EX_SECRET	8
#define EX_PICKPROOF    16

#define DOOR_1 (EX_ISDOOR|EX_CLOSED)
#define DOOR_2 (EX_PICKPROOF|EX_ISDOOR|EX_CLOSED|EX_LOCKED)
#define DOOR_3 (EX_SECRET|EX_ISDOOR|EX_CLOSED)
#define DOOR_4 (EX_SECRET|EX_ISDOOR|EX_PICKPROOF|EX_CLOSED|EX_LOCKED)
#define DOOR_5 (EX_ISDOOR|EX_CLOSED|EX_LOCKED)
#define DOOR_6 (EX_ISDOOR)
#define DOOR_7 (EX_SECRET|EX_ISDOOR)

/* For 'Sector types' */

#define SECT_INSIDE          0
#define SECT_CITY            1
#define SECT_FIELD           2
#define SECT_FOREST          3
#define SECT_HILLS           4
#define SECT_MOUNTAIN        5
#define SECT_WATER_SWIM      6
#define SECT_WATER_NOSWIM    7
#define SECT_AIR             8
#define SECT_UNDERWATER      9
#define SECT_FIRE		10
#define SECT_COLD		11
#define SECT_DESERT          12
#define SECT_ARCTIC          13
#define SECT_ROAD            14
#define MAX_SECTORS  	     14	/* keep this the same as last */

struct room_affect_data
{
	void *aff; /* pointer to affect */
	int type;  /* indicator so we know what affect is */
	struct room_affect_data *next;  /* Next in the queue */
};

struct trap_pressure_data /* A room affect */
{
	int weight;   /* how much weight to trip the trap */
	bool triggered; /* Is the trap still around */
	void *effect;   /* pointer to various effects */
	int type;     /* So we'll know what kind of trap */
};

struct trap_type_spell    /* A type of trap effect */
{
	int level;
	int spell;
	int type;
};

struct trap_type_damage   /* A type of trap effect */
{
	char *msg_to_char;
	char *msg_to_room;
	int num,dice,bonus;
};

struct room_direction_data
{
	char *general_description;       /* When look DIR.                  */ 
	char *keyword;                   /* for open/close                  */	
	u32 exit_info;                /* Exit info                       */
	slong key;	                 /* Key's number (-1 for no key)    */
	slong to_room;                  /* Where direction leeds (NOWHERE) */
};

/* ========================= Structure for room ========================== */
struct room_data
{
	slong number;               /* Rooms number                       */
	sh_int zone;                 /* Room zone (for resetting)          */
	u8 sector_type;             /* sector type (move/hide)            */

        u8 river_dir;               /* dir of flow on river               */
        int river_speed;             /* speed of flow on river             */

	int  tele_time;              /* time to a teleport                 */
	int  tele_targ;              /* target room of a teleport          */
	char tele_look;              /* do a do_look or not when 
					teleported                         */


	char *name;                  /* Rooms name 'You are ...'           */
	char *description;           /* Shown when entered                 */
	struct extra_descr_data *ex_description; /* for examine/look       */
	struct room_direction_data *dir_option[6]; /* Directions           */
	ulong room_flags;           /* DEATH,DARK ... etc                 */ 
	int light;                  /* Number of lightsources in room     */
	int dark;                  /* for shifts in daylight */
	int (*funct)(struct char_data *,int,char *); /* special procedure  */
         
	struct obj_data *contents;   /* List of items in room              */
	struct char_data *people;    /* List of NPC / PC in room           */
        struct room_affect_data *affects; /* List of room affects          */
	int moblim;
};
/* ======================================================================== */
/* For wear locations in ch->points.armor */

#define LOCATION_UNKNOWN 0
#define LOCATION_FEET    1
#define LOCATION_LEGS    2
#define LOCATION_ARMS    3
#define LOCATION_BODY    4
#define LOCATION_HEAD    5
#define LOCATION_MAX     6


/* The following defs and structures are related to char_data   */

/* For 'equipment' */

#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAISTE    13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WIELD          16
#define HOLD           17
#define POUCH		18
#define MAX_WEAR	19	/* increment as needed, to be used in loops */


/* For 'char_payer_data' */


/*
**  #2 has been used!!!!  Don't try using the last of the 3, because it is
**  the keeper of active/inactive status for dead characters for ressurection!
*/
#define MAX_TOUNGE  10    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */


#define MAX_SKILLS  207   /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_AFFECT  50    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

/* Predifined  conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2

/* Bitvector for 'affected_by' */
#define AFF_BLIND             1
#define AFF_INVISIBLE         2
#define AFF_DETECT_EVIL       3
#define AFF_DETECT_INVISIBLE  4
#define AFF_DETECT_MAGIC      5
#define AFF_SENSE_LIFE        6
#define AFF_HOLD              7
#define AFF_SANCTUARY         8
#define AFF_GROUP             9
#define AFF_CURSE             10
#define AFF_FLYING            11
#define AFF_POISON            12
#define AFF_PROTECT_EVIL      13
#define AFF_PARALYSIS         14
#define AFF_INFRAVISION       15
#define AFF_WATERBREATH       16
#define AFF_SLEEP             17
#define AFF_SNEAK             18
#define AFF_HIDE              19
#define AFF_FEAR              20
#define AFF_CHARM             21
#define AFF_FOLLOW            22
#define AFF_TRUE_SIGHT        23
#define AFF_SCRYING           24
#define AFF_FIRESHIELD        25
#define AFF_INVULNERABLE	26
#define AFF_BERSERK		27
#define AFF_BERRECV		28
#define AFF_NOSLEEP		29
#define AFF_NOSUMMON		30
#define AFF_NOCHARM		31
#define AFF_HASTE		32
#define AFF_RECHARGER		33
#define AFF_VITALITY		34
#define AFF_FARSEE		35
#define AFF_NOFEAR		36


/* modifiers to char's abilities */

#define APPLY_NONE              0
#define APPLY_STR               1
#define APPLY_DEX               2
#define APPLY_INT               3
#define APPLY_WIS               4
#define APPLY_CON               5
#define APPLY_SEX               6
#define APPLY_CLASS             7
#define APPLY_LEVEL             8
#define APPLY_AGE               9
#define APPLY_CHAR_WEIGHT      10
#define APPLY_CHAR_HEIGHT      11
#define APPLY_MANA             12
#define APPLY_HIT              13
#define APPLY_MOVE             14
#define APPLY_GOLD             15
#define APPLY_EXP              16
#define APPLY_ALL_AC           17
#define APPLY_ARMOR            17
#define APPLY_HITROLL          18
#define APPLY_DAMROLL          19
#define APPLY_SAVING_PARA      20
#define APPLY_SAVING_ROD       21
#define APPLY_SAVING_PETRI     22
#define APPLY_SAVING_BREATH    23
#define APPLY_SAVING_SPELL     24
#define APPLY_SAVE_ALL         25
#define APPLY_IMMUNE           26
#define APPLY_SUSC             27
#define APPLY_M_IMMUNE         28
#define APPLY_SPELL            29
#define APPLY_WEAPON_SPELL     30
#define APPLY_EAT_SPELL        31
#define APPLY_BACKSTAB         32
#define APPLY_KICK             33
#define APPLY_SNEAK            34
#define APPLY_HIDE             35
#define APPLY_BASH             36
#define APPLY_PICK             37
#define APPLY_STEAL            38
#define APPLY_TRACK            39
#define APPLY_HITNDAM          40
#define APPLY_FEET_AC          41
#define APPLY_LEGS_AC          42
#define APPLY_ARMS_AC          43
#define APPLY_BODY_AC          44
#define APPLY_HEAD_AC          45
#define APPLY_ALL_STOPPING    46
#define APPLY_FEET_STOPPING   47
#define APPLY_LEGS_STOPPING   48
#define APPLY_ARMS_STOPPING   49
#define APPLY_BODY_STOPPING   50
#define APPLY_HEAD_STOPPING   51
#define APPLY_BATTER          52
#define APPLY_POWER           53

/* sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* positions */
#define POSITION_DEAD       0
#define POSITION_MORTALLYW  1
#define POSITION_INCAP      2
#define POSITION_STUNNED    3
#define POSITION_SLEEPING   4
#define POSITION_RESTING    5
#define POSITION_PRAYING    6
#define POSITION_SITTING    7
#define POSITION_FIGHTING   8
#define POSITION_STANDING   9

/* for mobile actions: specials.act */
#define ACT_SPEC       (1<<0)  /* special routine to be called if exist   */
#define ACT_SENTINEL   (1<<1)  /* this mobile not to be moved             */
#define ACT_SCAVENGER  (1<<2)  /* pick up stuff lying around              */
#define ACT_ISNPC      (1<<3)  /* This bit is set for use with IS_NPC()   */
#define ACT_NICE_THIEF (1<<4)  /* Set if a thief should NOT be killed     */
#define ACT_AGGRESSIVE (1<<5)  /* Set if automatic attack on NPC's        */
#define ACT_STAY_ZONE  (1<<6)  /* MOB Must stay inside its own zone       */
#define ACT_WIMPY      (1<<7)  /* MOB Will flee when injured, and if      */
                               /* aggressive only attack sleeping players */
#define ACT_ANNOYING   (1<<8)  /* MOB is so utterly irritating that other */
                               /* monsters will attack it...              */
#define ACT_HATEFUL    (1<<9)  /* MOB will attack a PC or NPC matching a  */
                               /* specified name                          */
#define ACT_AFRAID    (1<<10)  /* MOB is afraid of a certain PC or NPC,   */
                               /* and will always run away ....           */
#define ACT_IMMORTAL  (1<<11)  /* MOB is a natural event, can't be kiled  */
#define ACT_HUNTING   (1<<12)  /* MOB is hunting someone                  */
#define ACT_DEADLY    (1<<13)  /* MOB has deadly poison                   */
#define ACT_POLYSELF  (1<<14)  /* MOB is a polymorphed person             */
#define ACT_POLYOTHER (1<<15)  /* MOB is a polymorphed person             */
#define ACT_GUARDIAN  (1<<16)  /* MOB will guard master                   */
#define ACT_CITIZEN   (1<<17)  /* MOB will protect other NPC's            */
#define ACT_HEALER    (1<<18)  /* MOB will heal itself over time          */
#define ACT_NOSUMMON  (1<<19)  /* can't be summoned */

/* For players : specials.act */
#define PLR_BRIEF     (1<<0)
#define PLR_NOSHOUT   (1<<1)/* the player is not allowed to shout */
#define PLR_COMPACT   (1<<2)
#define PLR_DONTSET   (1<<3) /* Dont EVER set */
#define PLR_WIMPY     (1<<4) /* character will flee when seriously injured */
#define PLR_NOHASSLE  (1<<5) /* char won't be attacked by aggressives.      */
#define PLR_STEALTH   (1<<6) /* char won't be announced in a variety of situations */
#define PLR_HUNTING   (1<<7) /* the player is hunting someone, do a track each look */
#define PLR_DEAF      (1<<8) /* The player does not hear shouts */
#define PLR_ECHO      (1<<9) /* Messages (tells, shout,etc) echo back */
#define PLR_LOG	      (1<<10)
#define PLR_NOVIEWLOG (1<<11)
#define PLR_BANISHED  (1<<12)
#define PLR_CRIMINAL  (1<<13)
#define PLR_NOTELL    (1<<14)
#define PLR_AUCTION   (1<<15) /* If on, player hears auctions          */ 
#define PLR_NOSPORTS  (1<<16) /* If on, player won't hear sports */
#define PLR_GOSSIP    (1<<17) /* If on, player hears gossip            */ 
#define PLR_SILENCE   (1<<18)
#define PLR_VEG	      (1<<19)
#define PLR_DEBUG     (1<<20)
#define PLR_NOEXITS   (1<<21)
#define PLR_COUNCIL   (1<<22)
#define PLR_MAYOR     (1<<23)
#define PLR_CORRUPT   (1<<24)
#define PLR_STUPID    (1<<25) /* for real dorks cant talk title emote gossip etc */
#define PLR_REGEN	(1<<26) /* player choose regen mode offline */
#define PLR_PRIORITY    (1<<27) /* player wants to use priority system */
#define PLR_NOFINGER    (1<<28) /* player doesn't want full/email seen */

typedef struct form_t {
  int32           virtual;
  string          name;
  int8            nlocs;
  struct locdef_t *locdefs;
} form_t;


/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data
{
	byte hours, day, month;
	sh_int year;
};

/* These data contain information about a players time data */
struct time_data
{
  time_t birth;    /* This represents the characters age                */
  time_t logon;    /* Time of the last logon (used to calculate played) */
  time_t motd;     /* Date of last motd read                            */
  time_t password; /* Time of last password change (for expiration)     */
  int played;      /* This is the total accumulated time played in secs */
};

struct char_player_data
{
	char *name;    	    /* PC / NPC s name (kill ...  )         */
	char *pw;
	char *short_descr;  /* for 'actions'                        */
	char *long_descr;   /* for 'look'.. Only here for testing   */
	char *description;  /* Extra descriptions                   */
	char *title;        /* PC / NPC s title                     */
	char *email;        /* e-mail address of player             */
        char *full_name;    /* full name of player                  */
        char *plan;         /* finger information                   */
        char *last_site;    /* last logon site */
	char *sounds;       /* Sound that the monster makes (in room) */
	char *distant_snds; /* Sound that the monster makes (other) */
	byte sex;           /* PC / NPC s sex                       */
	u8 class;
	int race;
        int32 form;         /* PC / NPC s body type */
        form_t *formdef;    /* pointer to body type definition */
	int level;  /* PC / NPC s level                     */
	int max_level; /* max level if it has gone down */
	int hometown;       /* PC s Hometown (zone)                 */
	int ntalks;
	bool *talks; /* PC s Tounges 0 for NPC           */
 	struct time_data time; /* PC s AGE in days                 */
	s16 weight;       /* PC / NPC s weight                    */
	s16 height;       /* PC / NPC s height                    */
	byte guild;
};

struct char_ability_data
{
	sbyte str; 
	sbyte intel;
	sbyte wis; 
	sbyte dex; 
	sbyte con; 
	sbyte chr;
	sbyte lck;
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_point_data
{
        sh_int mana;         
        sh_int max_mana;
	sh_int power;
	sh_int max_power;

	sh_int hit;   
	sh_int max_hit;      /* Max hit for NPC                  */
	sh_int move;  
	sh_int max_move;     /* Max move for NPC                        */

	sh_int armor[6];     /* Internal -100..100, external -10..10 AC */
        sh_int stopping[6];  /* Stopping power of armor by location */
        sh_int aimloc;       /* Where the PC is aiming in combat */
        sh_int hitloc;       /* Where the PC actually hits in combat */
	ulong gold;            /* Money carried                           */
        ulong bankgold;        /* gold in the bank.                       */
	slong exp;             /* The experience of the player            */

	int hitroll;       /* Any bonus or penalty to the hit roll    */
	int damroll;       /* Any bonus or penalty to the damage roll */
};


struct char_special_data
{
	struct char_data *fighting; /* Opponent                             */
	struct char_data *hunting;  /* Hunting person..                     */
	u32 affected_by[8];
	byte position;           /* Standing or ...                         */
	byte default_pos;        /* Default position for NPC                */
	ulong act;      /* flags for NPC behavior                  */
	int spells_to_learn;    /* How many can you learn yet this level   */
	int carry_weight;        /* Carried weight                          */
        int carry_mass;          /* carried mass in grams */
        int carry_volume;        /* carried volume in cm^3 */
	byte carry_items;        /* Number of items carried                 */
	int timer;               /* Timer for update                        */
	slong was_in_room;      /* storage of location for linkdead people */
	sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)             */
	sbyte conditions[3];      /* Drunk full etc.                        */
	int damnodice;           /* The number of damage dice's            */
	int damsizedice;         /* The size of the damage dice's          */
	byte last_direction;      /* The last direction the monster went    */
	int attack_type;          /* The Attack Type Bitvector for NPC's    */
	int alignment;            /* +-1000 for alignments                  */
	int true_align;
	int whimpy_level;	/* Will flee if goes below this many hp */
	int invis_level;		      /* visibility of gods */
	ulong immune;        /* Immunities                  */
	ulong M_immune;      /* Meta Immunities             */
        ulong susc;          /* susceptibilities            */
	sh_int mult_att;     /* the number of attacks       */
	int nohitdice,sizehitdice,extrahp;
	u8 magic_use;       /* % of time will use spells if has them (mob) */
	u32 log_bits;
	u32 beacon_location; /* does not get saved */
        int wait;            /* delay on a player or mobile */
	char *away;          /* away message for players */
};


struct old_char_skill_data {
  int skill_number;
  byte learned;
  bool recognize;
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_skill_data
{
  u16 skill_number;
  s16 learned;
};

struct char_spell_data
{
  u16 spell_number;
  s16 learned;
};



/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct affected_type {
	int type;           /* The type of spell that caused this      */
	sh_int duration;      /* For how long its effects will last      */
	u32 modifier;       /* This is added to apropriate ability     */
	byte location;        /* Tells which ability to change(APPLY_XXX)*/
	u32 bitvector[8];

	struct affected_type *next;
};

struct follow_type
{
	struct char_data *follower;
	struct follow_type *next;
};

typedef struct build_data {
	int offset;
	int state;
	struct obj_data *obj_build;
	struct char_data *mob_build;
	int m_virtual;
	int hpd;
	int hps;
} build_data;

typedef struct mail_item {
  char *mail;
  struct mail_item *next;
} mail_item;

typedef struct locdef_t {
  int8            type;
  int8            hit_factor;
  int8            hp_factor;
  struct locdef_t *next;
} locdef_t;

#define FORM_CUSTOM   -1
#define FORM_HUMANOID  0
#define FORM_QUADRUPED 1
#define FORM_SERPENT   2
#define FORM_DRAGON    3
#define FORM_BLOB      4

#define LOC_NONE      0
#define LOC_HEAD      1
#define LOC_NECK      2
#define LOC_BODY      3
#define LOC_ARMS      4
#define LOC_WRIST     5
#define LOC_HANDS     6
#define LOC_PRIMARY   7
#define LOC_SECONDARY 8
#define LOC_FINGER    9
#define LOC_WAIST     10
#define LOC_LEGS      11
#define LOC_FEET      12
#define LOC_ABOUT     13
#define LOC_TAIL      14
#define LOC_WINGS     15
#define LOC_CLAW      16
#define LOC_TENTACLE  17
#define LOC_SHIELD    18

/* body hit location */
typedef struct location_t {
  int8              type;                 /* identity of location */
  int8              hit_factor;           /* percent of damage to inherit */
  int16             points;               /* current damage value */
  int16             max_points;           /* maximum damage value */
  struct obj_data   *worn;                /* item in use at this location */
  struct susres_t   susres[NUM_DMG];      /* susceptibilities/resistances */
} location_t;

/* ================== Structure for player/non-player ===================== */
struct char_data
{
	slong in_room;                       /* Location                    */

	byte needs_saved;
	byte   attackers;

	sh_int fallspeed;                     /* rate of descent for player */
	sh_int hunt_dist;                    /* max dist the player can hunt */

        unsigned short hatefield;
        unsigned short fearfield;

	Opinion hates;
	Opinion fears;

	sh_int  persist;
	slong  old_room;

	ulong act_ptr;    /* numeric argument for the mobile actions */
	void *user_data;
	void (*funct)(struct char_data *);    /* called after @ hit in string */

	struct char_player_data player;       /* Normal data                 */
	struct char_ability_data abilities;   /* Abilities                   */
	struct char_ability_data tmpabilities;/* The abilities we will use   */
	struct char_point_data points;        /* Points                      */
	struct char_special_data specials;    /* Special plaing constants    */

	int nskills;
	struct char_skill_data *skills; /* Skills                 */
	int nspells;
	struct char_spell_data *spells;

	int naffected;

	struct affected_type *affected;       /* affected by what spells     */
	struct obj_data *equipment[MAX_WEAR]; /* Equipment array             */
        location_t **locations;               /* location data */
	struct obj_data *warehouse;	      /* what's stored in wharehouse */
	struct obj_data *carrying;            /* Head of list                */
	struct connection_data *desc;         /* NULL for mobiles            */
	struct char_data *orig;               /* Special for polymorph       */

	struct char_data *next_in_room;     /* For room->people - list       */
	struct char_data *next;             /* For either monster or ppl-lis */
	struct char_data *next_fighting;    /* For fighting list             */

	struct follow_type *followers;        /* List of chars followers     */
	struct char_data *master;             /* Who is char following?      */

	struct build_data build;
	slong build_lo,build_hi,virtual;
	slong aggressive;
	struct char_data *consent;
	struct mail_item *mail_list;
	struct kill_info *kills;
	char *prompt;
};


/* ======================================================================== */

/* How much light is in the land ? */

#define SUN_DARK	0
#define SUN_RISE	1
#define SUN_LIGHT	2
#define SUN_SET		3
#define MOON_SET        4
#define MOON_RISE       5

/* And how is the sky ? */

#define SKY_CLOUDLESS	0
#define SKY_CLOUDY	1
#define SKY_RAINING	2
#define SKY_LIGHTNING	3

struct weather_data
{
	int pressure;	/* How is the pressure ( Mb ) */
	int change;	/* How fast and what way does it change. */
	int sky;	/* How is the sky. */
	int sunlight;	/* And how much sun. */
};



/* ***********************************************************************
*  file element for object file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */

struct obj_cost { /* used in act.other.c:do_save as well as in reception2.c */
	int total_cost;
	int no_carried;
	bool ok;
};


/* ***********************************************************
*  The following structures are related to connection_data   *
*********************************************************** */

struct txt_block
{
  int              copies;
  int              priority;
  int              command;
  int              length;
  char             *text;
  struct txt_block *next;
};

struct txt_q
{
	struct txt_block *head;
	struct txt_block *tail;
};



/* modes of connectedness */

#define CON_PLYNG          0
#define CON_NME	           1
#define CON_NMECNF         2
#define CON_PWDNRM         3
#define CON_PWDGET         4
#define CON_PWDCNF         5
#define CON_QSEX           6
#define CON_RMOTD          7
#define CON_SLCT           8
#define CON_EXDSCR         9
#define CON_QCLASS         10
#define CON_LDEAD          11
#define CON_PWDNEW         12
#define CON_PWDNCNF        13
#define CON_WIZLOCK        14
#define CON_QRACE          15
#define CON_RACPAR         16
#define CON_CONFIRM_PROMPT 17
#define CON_CONFIRM_GET    18
#define CON_QORDER         19
#define CON_QALIGN         20
#define CON_QSTATS         21
#define CON_FINAL          22
#define CON_OBJECT         23
#define CON_MOB            24
#define CON_DELETE_ME      25
#define CON_REGEN          26
#define CON_PWDEXP         27
#define CON_PWDECNF        28
#define CON_FULLNAME       29
#define CON_EMAIL          30
#define CON_TERM	   31

#define CONNF_PRIV 1

struct connection_data
{
  int id;          /* unique integer id of this connection */
  int flags;
  int term;	   /* terminal type id */
  char host[16];   /* hostname  123.345.567.890            */
  char pwd[12];	   /* password                   */
  int connected;   /* mode of 'connectedness'    */
  int wait;	   /* wait for how many loops    */
  int last_tick;   /* time last prompt was sent */
  int need_prompt;

  struct txt_q output;          /* q of strings to send       */
  struct txt_q input;           /* q of unprocessed input     */

  struct char_data *character;  /* linked to char             */
  struct char_data *original;   /* original char              */

  struct connection_data *snooping;
  struct connection_data *snooped_by[MAX_SNOOP];

  char buf[MAX_STRING_LENGTH];	/* buffer for raw input       */
  char last_input[MAX_INPUT_LENGTH];/* the last input           */

  char *showstr_head;		/* for paging through texts   */
  char *showstr_point;		/*       -                    */
  char **str;			/* for the modify-str system  */
  int max_str;			/* -                          */

  struct connection_data *next; /* link to next connection */
};

struct msg_type 
{
	char *attacker_msg;  /* message to attacker */
	char *victim_msg;    /* message to victim   */
	char *room_msg;      /* message to room     */
};

struct message_type
{
	struct msg_type die_msg;      /* messages when death            */
	struct msg_type miss_msg;     /* messages when miss             */
	struct msg_type hit_msg;      /* messages when hit              */
	struct msg_type sanctuary_msg;/* messages when hit on sanctuary */
	struct msg_type god_msg;      /* messages when hit on god       */
	struct message_type *next;/* to next messages of this kind.*/
};

struct message_list
{
	int a_type;               /* Attack type			 */
	int number_of_attacks;	  /* How many attack messages to chose from. */
	struct message_type *msg; /* List of messages.			 */
};

struct dex_skill_type
{
	sh_int p_pocket;
	sh_int p_locks;
	sh_int traps;
	sh_int sneak;
	sh_int hide;
};

struct dex_app_type
{
	sh_int reaction;
	sh_int miss_att;
	sh_int defensive;
};

struct str_app_type
{
	sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
	sh_int todam;    /* Damage Bonus/Penalty                */
	sh_int carry_w;  /* Maximum weight that can be carrried */
	sh_int wield_w;  /* Maximum weight that can be wielded  */
};

struct wis_app_type
{
	byte bonus;       /* how many bonus skills a player can */
	                  /* practice pr. level                 */
};

struct int_app_type
{
	byte learn;       /* how many % a player learns a spell/skill */
};

struct con_app_type
{
	sh_int hitp;
	sh_int shock;
};

/************************************************************/

typedef struct shop_data
{
  int index;
  slong n_producing;
  u32 *producing;       /* Which item to produce (virtual)      */
  double profit_buy;     /* Factor to multiply cost with.        */
  double profit_sell;    /* Factor to multiply cost with.        */
  int n_type;
  u16 *type;           /* Which item to trade.                 */
  slong n_vnum;
  u32 *vnum;            /* vnums which will buy                 */
  char *no_such_item1;  /* Message if keeper hasn't got an item */
  char *no_such_item2;  /* Message if player hasn't got an item */
  char *missing_cash1;  /* Message if keeper hasn't got cash    */
  char *missing_cash2;  /* Message if player hasn't got cash    */
  char *do_not_buy;     /* If keeper dosn't buy such things.    */
  char *message_buy;    /* Message when player buys item        */
  char *message_sell;   /* Message when player sells item       */
  int temper1;          /* How does keeper react if no money    */
  int temper2;          /* How does keeper react when attacked  */
  int keeper;           /* The mobil who owns the shop (virtual)*/
  int in_room;          /* Where is the shop?                   */
  int n_hours;
  int *open,*close;
  struct shop_data *next;
} shop_data;



typedef void (*funcp)();

struct breather {
  int	vnum;
  int	cost;
  funcp	*breaths;
};


typedef struct {
  int race;
  int adj[7];
  int height[3]; /* min avg max */
  int weight[3]; /* min avg max */
} racial_adjustment;

struct mock_char_data {
   int hp;
   int ac;
   int dam;
   int thaco;
};

#define MSG_DONE -1

typedef struct index_mem {
  struct char_data *mob;
  struct obj_data *obj;
  int boot_count;
  int max_at_boot;
  int count;
  int (*func)();  /* special procedure */
} index_mem;

typedef struct skill_entry {
  u16 num;		/* number of skill */
  u16 min_level;
  u8  max_at_guild;
  u8  max_learn;
  u8  difficulty;
  u16 cost;
  u8  n_pre_reqs;
  u16 *pre_reqs;
  struct skill_entry *next;
} skill_entry;

#define SOURCE_MANA  0
#define SOURCE_POWER 1

typedef struct spell_entry {
  u16 num;
  u16 min_level;
  u8  max_at_guild;
  u8  max_learn;
  int mana;
  u8  difficulty;
  u16 cost;
  u8  source;
  u8  n_pre_reqs;
  u16 *pre_reqs;
  u8  n_spell_components;
  u32 *components;
  struct spell_entry *next;
} spell_entry;


#define ALIGN_G (1<<0)
#define ALIGN_N (1<<1)
#define ALIGN_E (1<<2)

#define FLAG_CLERIC	(1<<0)
#define FLAG_MAGE	(1<<1)
#define FLAG_FIGHTER	(1<<2)
#define FLAG_THIEF	(1<<3)

typedef struct class_entry {
  u8 index;
  char *class_name; /* name of this class */
  char abbrev[3];   /* 2 char abbreviation */
  double multi;     /* Exp multiplier */
  u8 thaco_min;     /* min thac0 for a class */
  u8 thaco_level;   /* number of levels before thac0 loweres by 1 */
  uint8 speed_max;  /* maximum speed for a class */
  uint8 speed_level; /* number of levels before speed raises by 1 */
  u8 alignment;     /* ALIGN_X bit vectory */
  u8 hp[3];         /* hp[0]=num dice, [1]=size [2]=additional */
  s8 min[7];        /* min abils */
  s8 max[7];        /* max abils */
  u8 saves[6];      /* base saving throws (100 based) */
  u8 decrease[6];   /* How many levels before decrease of 1 */
  u8 minsave[6];    /* Minimum values for saves */
  u8 nraces;        /* how many races for this record */
  u16 *races;       /* actual races allowed */
  u8 nitems;        /* how many items to give to newbie */
  u32 *items;       /* vnums of items to give */
  u32 flags;        /* FLAG_XXXXX */
  u32 proficiency;  /* starting weapon proficiency */
  struct spell_entry *spells;
  struct skill_entry *skills;
} class_entry;

typedef struct tree_node {
  char *name;
  int value;
  struct tree_node *left, *right;
} tree_node;

#define MAX_CAPS 10
#define MAX_COLORS 10
#define TERM_OTHER 6


typedef struct {
  char *name;
  char *cap[MAX_CAPS];  /* term capabilities */
  char *color[MAX_COLORS];
} termcap_entry;

#define REMOTE_PORT 8436

#endif
