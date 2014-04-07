/*****************************************************************************
**									    **
**			OEdit.h--By Adam Caldwell			    **
**									    **
*****************************************************************************/

extern char *spells[];
extern char *drinknames[];

#define NAME_HELP "\n\r\
This should be a list of names that this object can be called by.\n\r\
For example, for a thermonuclear bomb.  A good respones would be:\n\r\
nuke bomb thermonuclear\n\r\n\r\
"

#define DESCRIPTION_HELP "\n\r\
This should be what the player would see if he walked into a room and it\n\r\
was just sitting there.  For example, for a thermonuclear bomb:\n\r\
A thermonuclear bomb of awesome potential is here.\n\r\n\r\
"

#define SHORT_NAME_HELP "\n\r\
This should be what others see when a player uses this item.  It is also\n\r\
used for the equipment list.  For example, for a thermonuclar bomb:\n\r\
a bomb\n\r\n\r\
"

#define WEIGHT_HELP "\
This should detail the weight guidelines.\n\r\
"

#define COST_HELP "\
This should detail the pricing guidelines.\n\r\
"

static char *obj_type[]= {
        "light",
        "scroll",
        "wand",
        "staff",
        "weapon",
        "fireweapon",
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
        "drinkcon",
        "key",
        "food",
        "money",
        "pen",
        "boat",
        "\n" };

static char *wear_bits[] = {
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
	"waiste",
	"wrist",
	"wield",
	"hold",
	"throw",
	"construction",
	"\n" };

static char *extra_bits[] = {
	"glow",
	"hum",
	"metal",
	"mineral",
	"organic",
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
	"anti-fighter",
	"\n" };

static char *bitvector_bits[] = {
	"blind",
	"invisible",
	"detect-evil",
	"detect-invisible",
	"detect-magic",
	"sense-life",
	"hold",
	"sanctuary",
	"group",
	"undef",
	"curse",
	"flying",
	"poison",
	"protect-evil",
	"paralysis",
	"infravision",
	"waterbreath",
	"sleep",
	"drug-free",
	"sneak",
	"hide",
	"fear",
	"charm",
	"follow",
	"undef-1",
	"true-sight",
	"scrying",
	"fireshield",
	"undef-5",
	"undef-6",
	"undef-7",
	"\n" };

typedef struct {
  char *field;
  int min,max;
} list_min_max;

static list_min_max apply_list[] = {
	{ "str",	-10,	10 },
	{ "dex",	-10,	10 },
	{ "int",	-10,	10 },
	{ "wis",	-10,	10 },
	{ "con",	-10,	10 },
	{ "sex",	-10,	10 },
	{ "class",	-10,	10 },
	{ "level",	-10,	10 },
	{ "age",	-10,	10 },
	{ "char-weight",-10,	10 },
	{ "char-height",-10,	10 },
	{ "mana",	-10,	10 },
	{ "hit",	-10,	10 },
	{ "move",	-10,	10 },
	{ "gold",	-10,	10 },
	{ "exp",	-10,	10 },
	{ "all-ac",	-10,	10 },
	{ "hitroll",	-10,	10 },
	{ "damroll",	-10,	10 },
	{ "saving-para",-3,	3 },
	{ "saving-rod",	-3,	3 },
	{ "saving-petri",-3,	3 },
	{ "saving-breath",-3,	3 },
	{ "saving-spell",-3,	3 },
	{ "save-all",	-2,	2 },
	{ "immune",	-10,	10 },
	{ "susc",	-10,	10 },
	{ "m-immune",	-10,	10 },
	{ "spell",	  1,	97 },
	{ "weapon-spell", 1,	97 },
	{ "eat-spell",	 1,	97 },
	{ "backstab",	-10,	10 },
	{ "kick",	-10,	10 },
	{ "sneak",	-10,	10 },
	{ "hide",	-10,	10 },
	{ "bash",	-10,	10 },
	{ "pick",	-10,	10 },
	{ "steal",	-10,	10 },
	{ "track",	-10,	10 },
	{ "hitndam",	-10,	10 },
	{ "feet-ac",	-10,	10 },
	{ "legs-ac",	-10,	10 },
	{ "arms-ac",	-10,	10 },
	{ "body-ac",	-10,	10 },
	{ "head-ac",    -10,    10 },
	{ "all-stopping",-10,	10 },
	{ "feet-stopping",-10,	10 },
	{ "legs-stopping",-10,	10 },
	{ "arms-stopping",-10,	10 },
	{ "body-stopping",-10,	10 },
	{ "head-stopping",-10,  10 },
	{ "\n",0,0 }};

static char *hit_type[]= {
	"hit",   /* -1 */
	"smite", /* 0 */
	"stab",
	"whip",
	"slash",
	"smash",
	"cleave",
	"crush",
	"bludgeon",
	"claw",
	"bite",
	"sting"	,
	"pierce",
	"\n"};


static char *immune_bits[] = {
	"fire",
	"cold",
	"elec",
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
	"nonmag",
	"plus1",
	"plus2",
	"plus3",
	"plus4",
	"\n"};
