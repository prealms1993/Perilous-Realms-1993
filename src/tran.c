#include <stdio.h>
#include <ctype.h>
/*#include <unistd.h>*/
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>


#include <structs.h>
#include <utils.h>
#include <write.h>

#define TRAN_OBJ 1
#define TRAN_MOB 2
#define TRAN_SHP 3
#define TRAN_ROM 4
#define TRAN_ZON 5

char *mystrdup(char *source);

int tran_type;
int OFFSET = 0;

typedef struct node {
	int data;
	struct node *next;
} node;

typedef struct var_node {
	int value;
	char *name;
	struct var_node *next;
}  var_node;


var_node *var_list = NULL;
node *list = NULL;
typedef struct inforec {
	ID_TYPE id;
	int field_type;
	char *name;
	char **list;
	struct inforec *sub;
} inforec;


int is_abbrev(char *arg1, char *arg2);
int string_lookup(char *name, char *fields[]);
int string_table(char *names, char *fields[], s16 *A);
void bitvector(u32 *v, char *names, char *list[]);
unsigned long bitvector1(char *names, char *list[]);
int str_cmp(char *arg1, char *arg2);
int block_parser(char *block, void(*handle)(FILE *, char *, char *, inforec *), FILE *fp, inforec *fields);

int virtual;

#define SOR 255
#define EOL 254

extern char *race_list[];
extern char *class_list[];
extern char *trap_eff_flags[];
extern char *weapon_types[];
extern char *damage_types[];
extern char *spells[];
extern char *skills[];
extern char *immunity_names[];
extern char *action_bits[];
extern char *affected_bits[];
extern char *drinks[];
extern char *container_bits[];
extern char *extra_bits[];
extern char *wear_bits[];
extern char *material_types[];
extern char *item_types[];
extern char *sector_types[];
extern char *dirs[];
extern char *room_bits[];
extern char *exit_bits[];

#define STR 1 /* string */
#define U8  2 /* unsigned 8 bit */
#define S8  3 /* signed 8 bit */
#define S16 4 /* signed 16 bit */
#define U16 5 /* unsigned 16 bit */
#define S32 6 /* signed 32 bit */
#define U32 7 /* unsigned 32 bit */
#define E32 8 /* Element from a list (32 bit) */
#define L16 9 /* array of elements from a list */
#define IDV 10 /* infitite dimension vector */
#define T32 11 /* two, signed 32 bit numbers */
#define BIT 12 /* 32 bit bitvector */
#define DIC 13 /* dice format... 3 32bit signed */
#define SUB 14 /* sublist */
#define HOL 15 /* place holder */
#define A32 16 /* Array of 32 bit values */
#define DBL 17 /* double precision float */
#define RVR 18 /* for river flow */
#define TEL 19 /* for teleport */
#define ROM 20 /* for rooms to resolve references */
#define EXI 21 /* for quick exit */
#define E8  22 /* element from a list (8 bit) */

inforec extra_desc[] = {
	{ 1, STR, "keywords" },
	{ 2, STR, "desc" },
	{ -1, -1, "", 0 }
};

inforec exit_field[] = {
	{ 1, EXI, "to" },
	{ 2, STR, "description" },
	{ 2, STR, "desc" },
	{ 3, STR, "keywords" },
	{ 4, BIT, "info", exit_bits },
	{ 5, U32, "key" },
	{ -1, -1, "", 0 }
};

inforec room_field[] = {
	{ 1, E8, "sector", sector_types },
	{ 2, RVR, "river" },
	{ 3, TEL, "teleport" },
	{ 3, TEL, "tele" },
	{ 4, STR, "name" },
	{ 5, STR, "description" },
	{ 5, STR, "desc" },
	{ 6, SUB, "extra", NULL, extra_desc },
	{ 7, SUB, "exits", NULL, exit_field },
	{ 9, BIT, "flags", room_bits },
	{ -1, -1, "" },
};


inforec shop_messages[] = {
	{ 1, STR, "dont_have" },
	{ 2, STR, "no_item" },
	{ 3, STR, "dont_buy" },
	{ 4, STR, "shop_cant_afford" },
	{ 5, STR, "player_cant_afford" },
	{ 6, STR, "buy" },
	{ 7, STR, "sell" },
	{ -1, -1, "" } };

inforec shp_field[] = {
	{ 0, S32, "shopkeeper" },
	{ 0, S32, "keeper" },
	{ 1, S32, "location" },
	{ 2, L16, "buy_type", item_types },
	{ 3, A32, "sell" },
	{ 6, DBL, "sell_mult" },
	{ 7, DBL, "buy_mult" },
	{ 8, T32, "hours" },
	{ 9, SUB, "messages", NULL, shop_messages },
	{ 10, U32, "temper_player" },
	{ 11, U32, "temper_attacked" },
	{ 12, A32, "buy" },
	{ -1, -1, "" } };

inforec mob_field[] = {
	{ 0, STR, "name" },
	{ 1, STR, "namelist" },
	{ 2, STR, "shortdesc" },
	{ 2, STR, "short" },
	{ 3, STR, "longdesc" },
	{ 3, STR, "long" },
	{ 4, E32, "race", race_list },
	{ 5, U32, "mana" },
	{ 6, S16, "height" },
	{ 7, S16, "weight" },
	{ 8, S32, "level" },
	{ 9, S32, "alignment" },
	{ 10, U32, "gold" },
	{ 11, S32, "hit_bonus" },
	{ 12, S32, "dam_bonus" },
	{ 13, S32, "attacks_per_round" },
	{ 14, L16, "skills", skills },
	{ 16, U8, "strength" },
	{ 17, U8, "dexterity" },
	{ 18, U8, "constitution" },
	{ 19, U8, "intelligence" },
	{ 20, U8, "wisdom" },
	{ 21, U8, "charisma" },
	{ 22, U8, "luck" },
	{ 23, T32, "head" },
	{ 24, T32, "body" },
	{ 25, T32, "arms" },
	{ 27, T32, "legs" },
	{ 28, T32, "feet" },
	{ 29, BIT, "immune", immunity_names },
	{ 30, BIT, "resistant", immunity_names },
	{ 31, BIT, "susceptible", immunity_names },
	{ 32, S32, "rod" },
	{ 33, S32, "spell" },
	{ 34, S32, "breath" },
	{ 35, S32, "petrification" },
	{ 36, S32, "paralyzation" },
	{ 37, DIC, "hp" },
	{ 38, U32, "move" },
	{ 39, S32, "experience" },
	{ 40, DIC, "damage" },
	{ 41, IDV, "affected", affected_bits },
	{ 42, BIT, "act", action_bits },
	{ 43, U8, "caster" },
	{ 44, L16, "spells", spells },
	{ 45, U32, "power" },
	{ 46, E8, "class", class_list },
	{ -1, -1, "", 0 } };

inforec _light[] = {
	{ 3, S32, "duration" },
	{ -1, -1, "", 0 }
};

inforec _scroll[] = {
	{ 1, S32, "level" },
	{ 2, E32, "spell1", spells },
	{ 3, E32, "spell2", spells },
	{ 4, E32, "spell3", spells },
	{ -1, -1, "", 0 }
};

inforec _wand[] = {
	{ 1, S32, "level" },
	{ 2, S32, "max_charges" },
	{ 3, S32, "charges" },
	{ 4, E32, "spell", spells },
	{ -1, -1, "", 0 }
};

inforec _treasure[] = {
	{ 1, S32, "value" },
	{ -1, -1, "", 0 }
};

inforec _container[] = {
	{ 1, S32, "max_hold" },
	{ 2, BIT, "flags", container_bits },
	{ 3, S32, "key" },
	{ 4, S32, "timer" },
	{ -1, -1, "", 0 }
};

inforec _drinkcon[] = {
	{ 1, S32, "max_units" },
	{ 2, S32, "amount" },
	{ 3, E32, "type", drinks },
	{ 4, S32, "poisoned" },
	{ -1, -1, "", 0 }
};

inforec _armor[] = {
	{ 1, S32, "effective_ac" },
	{ 2, S32, "force" },
	{ 3, S32, "stopping" },
	{ 4, S32, "absorb" },
	{ -1, -1, "", 0 }
};

inforec _weapon[] = {
	{ 1, E32, "wtype", weapon_types },
	{ 2, S32, "no_dice" },
	{ 3, S32, "size_dice" },
	{ 4, E32, "type", damage_types },
	{ -1, -1, "", 0 }
};

inforec _food[] = {
	{ 1, S32, "fullness" },
	{ 4, S32, "poisoned" },
	{ -1, -1, "", 0 }
};

inforec _audio[] = {
	{ 1, S32, "frequency" },
	{ -1, -1, "", 0 }
};

inforec _trap[] = {
	{ 1, BIT, "effect_type", trap_eff_flags },
	{ 2, S32, "damage_type" },
	{ 3, S32, "level" },
	{ 4, S32, "charges" },
	{ -1, -1, "", 0 }
};

inforec _component[] = {
	{ 1, S32, "uses" },
	{ -1, -1, "", 0 }
};

inforec item_fields[] = {
	{ ITEM_LIGHT, SUB, "light", 0, _light },
	{ ITEM_SCROLL, SUB, "scroll", 0, _scroll },
	{ ITEM_WAND, SUB, "wand", 0, _wand },
	{ ITEM_STAFF, SUB, "staff", 0, _wand },
	{ ITEM_TREASURE, SUB, "treasure", 0, _treasure },
	{ ITEM_POTION, SUB, "potion", 0, _scroll },
	{ ITEM_CONTAINER, SUB, "container", 0, _container },
	{ ITEM_SPELL_POUCH, SUB, "pouch", 0, _container },
	{ ITEM_DRINKCON, SUB, "drink_container", 0, _drinkcon },
	{ ITEM_MONEY, SUB, "money", 0, _treasure },
	{ ITEM_ARMOR, SUB, "armor", 0, _armor },
	{ ITEM_WEAPON, SUB, "weapon", 0, _weapon },
	{ ITEM_FOOD, SUB, "food", 0, _food },
	{ ITEM_AUDIO, SUB, "audio", 0, _audio },
	{ ITEM_OTHER, HOL, "other" },
	{ ITEM_BOOK, HOL, "book" },
	{ ITEM_BOARD, HOL, "board" },
	{ ITEM_FIREWEAPON, HOL, "fireweapon" },
	{ ITEM_MISSILE, HOL, "missile" },
	{ ITEM_NOTE, HOL, "note" },
	{ ITEM_PEN, HOL, "pen" },
	{ ITEM_WORN, HOL, "worn" },
	{ ITEM_BOAT, HOL, "boat" },
	{ ITEM_TRASH, HOL, "trash" },
	{ ITEM_KEY, HOL, "key" },
	{ ITEM_TRAP, SUB, "trap", 0, _trap },
	{ ITEM_COMPONENT, SUB, "component", 0, _component },
	{ -1, -1, "", 0 } };

inforec apply_fields[] = {
	{ APPLY_STR, S32, "strength" },
	{ APPLY_DEX, S32, "dexterity" },
	{ APPLY_INT, S32, "intelligence" },
	{ APPLY_WIS, S32, "wisdom" },
	{ APPLY_CON, S32, "constitution" },
	{ APPLY_SEX, S32, "sex" },
	{ APPLY_CLASS, S32, "class" },
	{ APPLY_LEVEL, S32, "level" },
	{ APPLY_AGE, S32, "age" },
	{ APPLY_CHAR_WEIGHT, S32, "weight" },
	{ APPLY_CHAR_HEIGHT, S32, "height" },
	{ APPLY_MANA, S32, "mana" },
	{ APPLY_HIT, S32, "hp" },
	{ APPLY_MOVE, S32, "move" },
	{ APPLY_GOLD, S32, "gold" },
	{ APPLY_EXP, S32, "experience" },
	{ APPLY_ALL_AC, S32, "all_armor" },
	{ APPLY_HITROLL, S32, "hitroll" },
	{ APPLY_DAMROLL, S32, "damroll" },
	{ APPLY_SAVING_PARA, S32, "save_paralysis" },
	{ APPLY_SAVING_ROD, S32, "save_rod" },
	{ APPLY_SAVING_PETRI, S32, "save_petrification" },
	{ APPLY_SAVING_BREATH, S32, "save_breath" },
	{ APPLY_SAVING_SPELL, S32, "save_spell" },
	{ APPLY_SAVE_ALL, S32, "save_all" },
	{ APPLY_IMMUNE, BIT, "immunity", immunity_names },
	{ APPLY_M_IMMUNE, BIT, "resistance", immunity_names },
	{ APPLY_SUSC, BIT, "susceptibility", immunity_names },
	{ APPLY_SPELL, E32, "spell_affect", affected_bits },
	{ APPLY_WEAPON_SPELL, E32, "weapon_spell_affect", spells },
	{ APPLY_EAT_SPELL, E32, "eat_spell", spells },
	{ APPLY_BACKSTAB, S32, "backstab" },
	{ APPLY_KICK, S32, "kick" },
	{ APPLY_SNEAK, S32, "sneak" },
	{ APPLY_HIDE, S32, "hide" },
	{ APPLY_BASH, S32, "bash" },
	{ APPLY_PICK, S32, "pick" },
	{ APPLY_STEAL, S32, "steal" },
	{ APPLY_TRACK, S32, "track" },
	{ APPLY_HITNDAM, S32, "hitndamage" },
	{ APPLY_FEET_AC, S32, "ac_feet" },
	{ APPLY_LEGS_AC, S32, "ac_legs" },
	{ APPLY_ARMS_AC, S32, "ac_arms" },
	{ APPLY_BODY_AC, S32, "ac_body" },
	{ APPLY_HEAD_AC, S32, "ac_head" },
	{ APPLY_ALL_STOPPING, S32, "stopping_all" },
	{ APPLY_FEET_STOPPING, S32, "stopping_feet" },
	{ APPLY_LEGS_STOPPING, S32, "stopping_legs" },
	{ APPLY_ARMS_STOPPING, S32, "stopping_arms" },
	{ APPLY_BODY_STOPPING, S32, "stopping_body" },
	{ APPLY_HEAD_STOPPING, S32, "stopping_head" },
	{ APPLY_POWER, S32, "power" },
	{ -1, -1, "", 0 }
};

inforec obj_fields[] = {
	{ 0, STR, "name" },
	{ 1, STR, "namelist" },
	{ 2, STR, "longdesc" },
	{ 2, STR, "long" },
	{ 3, STR, "special" },
	{ 3, STR, "action" },
	{ 7, S32, "weight" },
	{ 43, S32, "value" },
	{ 44, S32, "rent" },
	{ 45, SUB, "type", 0, item_fields },
	{ 46, SUB, "extras", 0, extra_desc },
	{ 47, BIT, "bits", extra_bits },
	{ 48, BIT, "wear", wear_bits },
	{ 49, SUB, "apply", 0, apply_fields },
	{ 50, S32, "min_level" },
	{ 51, S32, "max_exist" },
	{ 52, E32, "material", material_types },
	{ 53, S32, "mass" },
	{ 54, S32, "volume" },
	{ 55, S32, "length" },
	{ -1, -1, "", 0 } };


void error(char* fmt, ...) {
	va_list args;
	struct char_data *ch;
	char s[65536];

	va_start(args, fmt);
	vsprintf(s, fmt, args);
	va_end(args);
	printf("Virtual=%d\n%s\n", virtual - OFFSET, s);
	if (OFFSET) printf("Offset=%d\n", OFFSET);
	exit(1);
}

int var_lookup(char *name)
{
	var_node *p;
	for (p = var_list; p; p = p->next)
	if (!strcmp(p->name, name)) return (p->value);
	printf("Undefined variable %s\n", name);
	exit(1);
}

int var_insert(char *name, int value)
{
	var_node *t, *p;

	if (!(t = (var_node *)malloc(sizeof(var_node)))) {
		perror("malloc");
		exit(1);
	}
	t->name = mystrdup(name);
	t->value = value;
	t->next = NULL;
	if (!var_list) {
		var_list = t;
		return(1);
	}
	if (var_list->value == value) {
		free(t);
		return(0);
	}
	if (value < var_list->value) {
		t->next = var_list;
		var_list = t;
		return(1);
	}
	for (p = var_list; p->next && p->next->value < value; p = p->next);
	if (p->next && p->next->value == value) {
		free(t);
		return(0);
	}
	t->next = p->next;
	p->next = t;
	return(1);
}


char *get_arg(char *arg, char *buf, char *stop)
{
	int i;
	char *p;

	if (!arg) {
		*buf = '\0';
		return(0);
	}
	for (i = 0; i < strlen(stop); i++)
	if (strchr(arg, stop[i])) break;
	p = strchr(arg, stop[i]);
	for (; arg != p; arg++, buf++)
		*buf = *arg;
	*buf = '\0';
	return(arg);
}

int resolve_room(char *arg)
{
	int x, offset = 0;
	char name[256];

	for (; isspace(*arg); arg++);
	if (strchr(arg, ':')) {
		arg = get_arg(arg, name, ":");
		if (*arg) arg++;
		offset = var_lookup(name);
	}
	x = atoi(arg);
	if (x) return(x + (offset ? offset : OFFSET));
	else if (isdigit(*arg)) return(offset ? offset : OFFSET);
	error("Rooms must be referred to AREA:offset notation.");
}

void handle(FILE *fp, char *token, char *param, inforec *field)
{
	ID_TYPE t;
	int temp;
	int i, x;
	int count;
	unsigned char u8;
	u32 v[8];
	signed short int s16;
	signed short int A16[100];

	for (; *param && isspace(*param); param++);
	i = strlen(param);
	if (!*param) error("No paramater to %s", token);
	while (i > 1 && isspace(param[i - 1])) i--;
	param[i] = '\0';
	i = 0;
	while (*field[i].name && str_cmp(field[i].name, token)) {
		i++;
	}
	if (!*field[i].name) {
		printf("Valid tokens here:\n");
		for (i = 0; *field[i].name; i++)
			printf("  %s\n", field[i].name);
		error("Unrecognized token: '%s', param: %s", token, param);
	}
	switch (field[i].field_type) {
	case SUB: {
				  ID(field[i].id);
				  block_parser(param, handle, fp, field[i].sub);
				  ID(EOL);
	} break;
	case HOL: ID(field[i].id); break;
	case STR:
		if (param[0] == '"') param++;
		if (param[x = (strlen(param) - 1)] == '"')
			param[x] = '\0';
		WriteS(field[i].id, param); break;
	case E8:  u8 = string_lookup(param, field[i].list);
		if (temp < 0)
			error("Not a legal param:\ntoken: %s, param: %s", token, param);
		Write(field[i].id, u8);
		break;
	case E32: temp = string_lookup(param, field[i].list);
		if (temp < 0)
			error("Not a legal param:\ntoken: %s, param: %s", token, param);
		x = temp;
		Write(field[i].id, x);
		break;
	case RVR: {
				  char dir[256];
				  param = get_arg(param, dir, ",");
				  u8 = string_lookup(dir, dirs);
				  ID(field[i].id);
				  WriteF(u8);
				  for (; isspace(*param) || *param == ','; param++);
				  x = atoi(param);
				  WriteF(x);
	} break;
	case ROM:
		x = resolve_room(param);
		Write(field[i].id, x);
		break;
	case EXI: {
				  char buf[256];
				  param = get_arg(param, buf, ",");
				  if (!*param)
					  error("Exit requires a direction and room");
				  param++;
				  ID(field[i].id);
				  u8 = string_lookup(buf, dirs);
				  WriteF(u8);
				  x = resolve_room(param);
				  WriteF(x);
	} break;
	case TEL: {
				  char buf[256];
				  if (!isdigit(*param)) error("NAN (tele)");
				  param = get_arg(param, buf, ",");
				  x = atoi(buf);
				  Write(field[i].id, x);
				  if (!*param)
					  error("Requires 3 paramters for teleport");
				  param = get_arg(++param, buf, ",");
				  if (!*param)
					  error("Requires 3 paramters for teleport");
				  u8 = atoi(buf);
				  WriteF(u8);
				  x = resolve_room(++param);
				  WriteF(x);
	} break;
	case BIT: x = bitvector1(param, field[i].list); Write(field[i].id, x); break;
	case U32:
	case S32: x = atoi(param);
		if (x == 0 && !isdigit(*param)) {
			error("%s takes an integer parameter.  You put '%s'",
				field[i].name, param);
		}
		Write(field[i].id, x); break;
	case S16:
		if (!isdigit(*param) && *param != '-' && *param != '+')
			error("Not a number: %s\ntoken: %s", param, token);
		s16 = atoi(param);
		Write(field[i].id, s16);
		break;
	case L16:
		count = string_table(param, field[i].list, A16);
		WriteA(field[i].id, A16, count, unsigned short int);
		break;
	case IDV:   bitvector(v, param, field[i].list);
		Write(field[i].id, v);
		break;
	case U8:
		if (!isdigit(*param)) {
			printf("NAN (%s)\n", field[i].name);
			exit(1);
		}
		u8 = atoi(param);
		Write(field[i].id, u8);
		break;
	case T32:
	case DIC:
		if (!isdigit(*param) && *param != '-' && *param != '+')
			error("Not a number: %s\ntoken: %s", param, token);
		x = atoi(param);
		while (*param && *param != ',') param++;
		if (!*param)
			error("Requires two paramaters: %s", token);
		while (*param && *param != '-' && *param != '+' && !isdigit(*param))
			param++;
		if (!isdigit(*param) && *param != '-' && *param != '+')
			error("Not a number: %s\ntoken: %s", param, token);
		ID(field[i].id);
		WriteF(x);
		x = atoi(param);
		WriteF(x);
		if (field[i].field_type != DIC)
			break;
		while (*param && *param != ',') param++;
		if (!*param)
			error("Requires three paramaters: %s", token);
		while (*param && *param != '-' && *param != '+' && !isdigit(*param))
			param++;
		if (!isdigit(*param) && *param != '-' && *param != '+')
			error("Not a number: %s\ntoken: %s", param, token);
		x = atoi(param);
		WriteF(x);
		break;
	case A32:{
				 int nel = 0;
				 signed long int *A;

				 A = (signed long int *)malloc(sizeof(signed long int));
				 if (!A)
					 error("malloc failure");
				 do {
					 if (!isdigit(*param))
						 error("Not a number %s\ntoken: %s", param, token);
					 A[nel] = atoi(param);
					 A = (signed long int *)realloc(A, sizeof(signed long int)*(++nel + 1));
					 while (*param && isdigit(*param)) param++;
					 while (*param && !isdigit(*param)) param++;
				 } while (*param);
				 WriteA(field[i].id, A, nel, signed long int);
				 free(A);
	} break;
	case DBL: {
				  double d;
				  d = atof(param);
				  Write(field[i].id, d);
	} break;
	default:
		error("Id=%d Token=%s Field=%s\n",
			field[i].id,
			token, param);
		exit(1);
	}
}

int ReadBlock(FILE *f, char *buf)
{
	int ch;
	int nesting;
	int done;

	while ((ch = fgetc(f)) != EOF && !done) done = (ch == '{');
	if (ch == EOF) return(0);
	nesting = 1;
	while (nesting && (ch = fgetc(f)) != EOF) {
		if (ch == '{') nesting++;
		else if (ch == '}') nesting--;
		*(buf++) = (char)ch;
		if (ch == '\n') {
			ch = '\r';
			*(buf++) = (char)ch;
		}
	}
	if (ch == EOF) *buf = '\0';
	else *(--buf) = '\0';
	return(ch != EOF);
}

char *SubBlock(char *main, char *sub)
{
	int nesting;

	for (; *main && *main != '{'; main++);
	main++;
	nesting = 1;
	while (nesting) {
		if (*main == '{') nesting++;
		else if (*main == '}') nesting--;
		if (nesting)
			*(sub++) = *(main++);
	}
	*sub = '\0';
	return(main + 1);
}

typedef struct macrorec {
	char name[100];
	char *text;
} macrorec;

macrorec macro[1000];
int nmacros = 0;

char *arg_subst(char *s, int argc, char **argv)
{
	static char ret[10000];
	int num;
	char *q;

	q = ret;
	while (*s) {
		if (*s == '$') {
			s++;
			num = 0;
			while (isdigit(*s))
				num = num * 10 + *(s++) - '0';
			if (num > argc) {
				abort();
			}
			strcpy(q, argv[num - 1]);
			q += strlen(argv[num - 1]);
		}
		else *(q++) = *(s++);
	}
	*q = '\0';
	return(ret);
}

void macro_expand(char *s)
{
	char temp[10000];
	char macname[100];
	int argc = 0;
	char *argv[100];
	char *p, *q, *r;
	int i, mac;

	r = s;
	strcpy(temp, s);
	p = temp;
	mac = 0;
	while (*p) {
		while (*p && *p != '@')
			*(s++) = *(p++);
		if (*p) {
			p++;
			q = macname;
			while (*p && !isspace(*p) && (*p != '('))
				*(q++) = *(p++);
			*q = '\0';
			if (*p == '(') {
				char temp[1000];
				p++;
				argc = 0;
				q = temp;
				while (*p != ')') {
					if (*p != ',') {
						*(q++) = *(p++);
					}
					else {
						p++;
						*q = '\0';
						argv[argc++] = mystrdup(temp);
						q = temp;
					}
				}
				*q = '\0';
				p++;
				argv[argc++] = mystrdup(temp);
			}
			mac = 0;
			for (i = 0; i < nmacros; i++) {
				if (!str_cmp(macname, macro[i].name)) {
					char temp[10000];
					mac = 1;
					if (strchr(macro[i].text, '$'))
						strcpy(temp, arg_subst(macro[i].text, argc, argv));
					else
						strcpy(temp, macro[i].text);
					strcpy(s, temp);
					s = s + strlen(temp);
				}
			}
			if (!mac) {
				printf("\n\n\r%s unknown macro\n\n", macname);
				exit(1);
			}
		}
	}
	*s = '\0';
	if (strchr(r, '@')) macro_expand(r);
}

int block_parser(char *block, void(*handle)(FILE *, char *, char *, inforec *), FILE *fp, inforec *fields)
{
	char token[100];
	char field[16384];
	char *cursor, *q;

	cursor = block;
	while (*cursor) {
		for (; *cursor && isspace(*cursor); cursor++);
		if (!*cursor) break;
		q = token;
		while (isalnum(*cursor) || *cursor == '_')
			*(q++) = *(cursor++);
		*q = '\0';
		cursor = SubBlock(cursor, field);
		handle(fp, token, field, fields);
	}
}

int insert(int data)
{
	node *t, *p;

	if (!(t = (node *)malloc(sizeof(node)))) {
		perror("malloc");
		exit(1);
	}
	t->data = data;
	t->next = NULL;
	if (!list) {
		list = t;
		return(1);
	}
	if (list->data == data) {
		free(t);
		return(0);
	}
	if (data < list->data) {
		t->next = list;
		list = t;
		return(1);
	}
	for (p = list; p->next && p->next->data < data; p = p->next);
	if (p->next && p->next->data == data) {
		free(t);
		return(0);
	}
	t->next = p->next;
	p->next = t;
	return(1);
}

void tran_file(FILE *inp, FILE *outp)
{
	char s[65536];
	char *p, *q;
	ID_TYPE t;
	unsigned long vnum;

	while (1) {
		do {
			if (!fgets(s, 1025, inp)) {
				fclose(inp);
				return;
			}
		} while (*s == '\n');
		s[strlen(s) - 1] = '\0';
#ifdef SUBMIT
		printf("%s\n",s);
#else
		if (isdigit(*s))
			fprintf(stderr, "%s\r", s);
		else
			fprintf(stderr, "%s\n", s);
#endif
		if (*s == '@') { /* macro def */
			strcpy(macro[nmacros].name, s + 1);
			if (!ReadBlock(inp, s)) {
				printf("Nesting error on macro definition %s\n", macro[nmacros].name);
				printf("%s\n", s);
				exit(1);
			}
			macro[nmacros].text = malloc(strlen(s) + 1);
			strcpy(macro[nmacros++].text, s);
			continue;
		}
		else if (*s == '#') { /* directive */
			FILE *include;
			char buf[16384];
			char filename[255];
			p = s + 1;
			if (!*p) continue;
			p = get_arg(p, buf, " ");
			if (strcmp(buf, "include") == 0) {
#ifndef SUBMIT
				for (; isspace(*p) || *p == '<' || *p == '"'; p++);
				p = get_arg(p, filename, ">\"");
				include = fopen(filename, "r");
				if (!include) {
					printf("Couldn't open %s\n", filename);
					exit(1);
				}
				tran_file(include, outp);
#endif
			}
			else if (strcmp(buf, "offset") == 0) {
				for (; isspace(*p); p++);
				if (isdigit(*p)) OFFSET = atoi(p);
				else OFFSET = var_lookup(p);
			}
			else if (strcmp(buf, "define") == 0) {
				char name[255]; int value;
				for (; isspace(*p); p++);
				p = get_arg(p, name, " =");
				value = atoi(p);
				if (!var_insert(name, value)) {
					printf("Duplicate identifier %s\n", name);
					exit(1);
				}
			}
			continue;
		}
		if (!isdigit(*s)) continue;

		virtual = vnum = atoi(s) + OFFSET;
		if (!insert(virtual))
			error("Duplicate virtual number.");
		t = SOR;
		fwrite(&t, sizeof(t), 1, outp);
		fwrite(&vnum, sizeof(vnum), 1, outp);

		if (!ReadBlock(inp, s)) {
			printf("Nesting error.\nViolating block:");
			printf("%s\n", s);
			exit(1);
		}

		if (strchr(s, '@'))
			macro_expand(s);
		switch (tran_type) {
		case TRAN_OBJ: block_parser(s, handle, outp, obj_fields); break;
		case TRAN_MOB: block_parser(s, handle, outp, mob_field); break;
		case TRAN_SHP: block_parser(s, handle, outp, shp_field); break;
		case TRAN_ROM: block_parser(s, handle, outp, room_field); break;
		}
	}
}


void usage()
{
	printf("Usage: tran obj|mob|shop|room|zone <infile> <outfile>\n");
	printf("  where infile is - if input should be taken from stdin\n");
	printf("  and outfile is /dev/null for a syntax check\n");
	exit(1);
}

void main(int argc, char **argv)
{
	FILE *inp, *outp;

	if (argc < 4) usage();

	if (strcmp(argv[1], "obj") == 0) tran_type = TRAN_OBJ;
	else if (strcmp(argv[1], "mob") == 0) tran_type = TRAN_MOB;
	else if (strcmp(argv[1], "shop") == 0) tran_type = TRAN_SHP;
	else if (strcmp(argv[1], "room") == 0) tran_type = TRAN_ROM;
	else if (strcmp(argv[1], "zone") == 0) tran_type = TRAN_ZON;
	else usage();
	if (strcmp(argv[2], "-") == 0) inp = stdin;
	else inp = fopen(argv[2], "r");
	if (!inp) {
		printf("Could not open %s\n", argv[1]);
		exit(1);
	}
	outp = fopen(argv[3], "w+b");
	if (!outp) {
		printf("Cound not open %s\n", argv[3]);
		exit(1);
	}
	tran_file(inp, outp);
	fclose(outp);
	printf("Done.\n");
	exit(0);
}
