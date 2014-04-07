/* ************************************************************************
*  file: db.h , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars booting world.                             *
************************************************************************* */
#ifndef _DBH_
#define _DBH_


/* data files used by the game system */

#define DFLT_DIR          "lib"           /* default data directory     */

#define WELCOME_FILE      "welcome"
#define WORLD_FILE        "tinyworld.wld" /* room definitions           */
#define MOB_FILE          "mob.out" /* monster prototypes         */
#define OBJ_FILE          "obj.out" /* object prototypes          */
#define NEW_WORLD_FILE		  "world.out"	//Replaces old boot method
#define SHP_FILE	  "shop.out" /* shop file */
#define ZONE_FILE         "tinyworld.zon" /* zone defs & command tables */
#define CREDITS_FILE      "credits"       /* for the 'credits' command  */
#define NEWS_FILE         "news"          /* for the 'news' command     */
#define OLD_NEWS_FILE      "news.old"      /* for the 'oldnews' command  */
#define DONATIONS_FILE    "donations"
#define MOTD_FILE         "motd"          /* messages of today          */
#define PLAYER_FILE       "players.new"   /* the player database        */
#define TIME_FILE         "time"          /* game calendar information  */
#define IDEA_FILE         "ideas"         /* for the 'idea'-command     */
#define TYPO_FILE         "typos"         /*         'typo'             */
#define BUG_FILE          "bugs"          /*         'bug'              */
#define MESS_FILE         "messages"      /* damage message             */
#define SOCMESS_FILE      "actions"       /* messgs for social acts     */
#define HELP_KWRD_FILE    "help_table"    /* for HELP <keywrd>          */
#define HELP_PAGE_FILE    "help"          /* for HELP <CR>              */
#define INFO_FILE         "info"          /* for INFO                   */
#define WIZLIST_FILE      "wizlist"       /* for WIZLIST                */
#define POSEMESS_FILE     "poses"         /* for 'pose'-command         */
#define BAD_NAMES_FILE    "trashcan"
#define ALLOW_SITES_ALWAYS	"always_sites"	/* for allow_desc */
#define CLASS_INFO_FILE	"class_help_info" /* for during roll stats */
#define TOOMANY_FILE "reject_msg"

#define REAL 0
#define VIRTUAL 1

/* structure for the reset commands */
struct reset_com
{
	int line;		/* line number */
	char command;   /* current command                      */ 
	bool if_flag;   /* if TRUE: exe only if preceding exe'd */
	slong arg1;       /*                                      */
	slong arg2;       /* Arguments to the command             */
	slong arg3;       /*                                      */

	/* 
	*  Commands:              *
	*  'M': Read a mobile     *
	*  'O': Read an object    *
	*  'G': Give obj to mob   *
	*  'P': Put obj in obj    *
	*  'G': Obj to char       *
	*  'E': Obj to char equip *
	*  'D': Set state of door *
	*/
};



/* zone definition structure. for the 'zone-table'   */
struct zone_data
{
	char *name;             /* name of this zone                  */
	int lifespan;           /* how long between resets (minutes)  */
	int age;                /* current age of this zone (minutes) */
	slong top;                /* upper limit for rooms in this zone */

	int reset_mode;         /* conditions for reset (see below)   */
	struct reset_com *cmd;  /* command table for reset	           */

	/*
	*  Reset mode:                              *
	*  0: Don't reset, and don't update age.    *
	*  1: Reset if no PC's are located in zone. *
	*  2: Just reset.                           *
	*/
};




/* element in monster and object index-tables   */
struct index_data
{
	slong virtual;    /* virtual number of this mob/obj           */
	long pos;       /* file position of this field              */
	int number;     /* number of existing units of this mob/obj	*/
	int abs_max_exist; /* max in universe */
	int (*func)();  /* special procedure for this mob/obj       */
	char *name;
	char *full_name;
};




/* for queueing zones for update   */
struct reset_q_element
{
	int zone_to_reset;            /* ref to zone_data */
	struct reset_q_element *next;	
};



/* structure for the update queue     */
struct reset_q_type
{
	struct reset_q_element *head;
	struct reset_q_element *tail;
} reset_q;



struct player_index_element
{
	char *name;
	int nr;
};


struct help_index_element
{
	char *keyword;
	long pos;
};
#endif
