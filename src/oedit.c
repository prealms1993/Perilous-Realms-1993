/*****************************************************************************
**									    **
**			OEdit.c--By Adam Caldwell			    **
**									    **
*****************************************************************************/

#include <ctype.h>

#include <fcns.h>
#include <ticks.h>
#include <utils.h>
#include <oedit.h>

#define STATE(ch) ch->build.state
#define BUILD ch->build.obj_build
#define STRING(FIELD,ARG) BUILD->FIELD=(char *)malloc(strlen(ARG)+1);\
			strcpy(BUILD->FIELD,ARG)
#define STR_ADD(FIELD,ARG)\
	if (BUILD->FIELD) {\
	    BUILD->FIELD=(char *)realloc(strlen(ARG)+strlen(BUILD->FIELD)+1);\
	    strcat(BUILD->FIELD,ARG);\
	} else {\
	    BUILD->FIELD=(char *)malloc(strlen(ARG)+1);\
	    strcpy(BUILD->FIELD,ARG);\
	}

#define O_NAME 1
#define O_DESCRIPTION 2
#define O_SHORT 3
#define O_ACTION 4
#define O_TYPE 5
#define O_WEIGHT 7
#define O_COST 8
#define O_RENT 9
#define O_WEAR 10
#define O_EXTRA 11
#define O_BITVECTOR 12
#define O_AFFECT1 13
#define O_AFFECT2 14
#define O_AFFECT3 15
#define O_AFFECT4 16
#define O_AFFECT5 17
#define O_VALUE0 18
#define O_VALUE1 19
#define O_VALUE2 20
#define O_VALUE3 21

#define STATE_HDR(nme) static void nme(struct char_data *ch, char *arg)
#define PRE_HDR(nme) static void nme(struct char_data *ch)
#define SEND(string) send_to_char(string,ch);

int my_search(char *arg,char **list)
{
  int i;

  if (!*arg) return(-1);
  i=0;
  while (*list[i]!='\n' && !is_abbrev(arg,list[i])) i++;
  if (*list[i]=='\n')
    return(-1);
  else return(i+1);
}

int range_check_list(struct char_data *ch,int n, char **list)
{
  int i;
  char buf[180];

  i=0;
  while (*list[i]!='\n') i++;
  if (n<1 || n>i) {
    for (n=0; n<i; n++) {
	sprintf(buf,"%-19s%s",list[n],((n+1) % 4)?"":"\n");
	SEND(buf);
    }
    SEND("\n");
    return(0);
  }
  return(1);
}

void bit_vector(int *bitv,struct char_data *ch,char *arg, char **list)
{
  char buf[MAX_INPUT_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  int bit;

  while (arg && *arg) {
    arg=one_argument(arg,buf);
    bit=my_search(buf,list)-1;
    if (bit<0) {
	sprintf(buf1,"Not recognized: '%s'\n",buf);
	SEND(buf1);
    } else if IS_SET(*bitv,(1<<bit)) {
	CLEAR_BIT(*bitv,(1<<bit));
    } else {
	SET_BIT(*bitv,(1<<bit));
    }
  }
}

void bit_vectorl(unsigned long int *bitv, struct char_data *ch, char *arg, char **list)
{
	char buf[MAX_INPUT_LENGTH];
	char buf1[MAX_STRING_LENGTH];
	int bit;

	while (arg && *arg) {
		arg = one_argument(arg, buf);
		bit = my_search(buf, list) - 1;
		if (bit<0) {
			sprintf(buf1, "Not recognized: '%s'\n", buf);
			SEND(buf1);
		}
		else if IS_SET(*bitv, (1 << bit)) {
			CLEAR_BIT(*bitv, (1 << bit));
		}
		else {
			SET_BIT(*bitv, (1 << bit));
		}
	}
}

void print_flags(struct char_data *ch,int bitv,char **list)
{
  char buf[MAX_STRING_LENGTH];
  int cnt,i;

  strcpy(buf,"\nBits set:");
  i=cnt=0;
  while (*list[i]!='\n') {
    if (IS_SET(bitv,(1<<i))) {
	sprintf(buf+strlen(buf)," %s",list[i]);
	cnt++;
    }
    i++;
  }
  if (!cnt) strcat(buf," None");
  strcat(buf,"\n");
  SEND(buf);
}

void list_list(struct char_data *ch,char **list)
{
  int i;
  char buf[100];

  i= -1;
  while (*list[++i]!='\n') {
    sprintf(buf,"%-19s%s",list[i],((i+1)%4)?"":"\n");
    SEND(buf);
  }
  SEND("\n");
}

int generic_affect(struct char_data *ch,char *arg,int which)
{
  int i,loc;
  char buf[MAX_INPUT_LENGTH];

  if (!*arg)
    return(0);
  if (*arg=='?') {
    i= -1;
    while (*apply_list[++i].field!='\n') {
      sprintf(buf,"%-19s%s",apply_list[i].field,((i+1)%4)?"":"\n");
      SEND(buf);
    }
    SEND("\n");
    return(1);
  }
  arg=one_argument(arg,buf);
  i=0;
  while (*apply_list[i].field!='\n' && !is_abbrev(buf,apply_list[i].field)) i++;
  if (*apply_list[i].field=='\n') {
    SEND("Not Recognized.  Use ? to list legal Affect locations.\n");
    return(1);
  }
  loc=i;
  one_argument(arg,buf);
  if (loc+1==APPLY_IMMUNE) {
	i=0;
	if (!*buf || *buf=='?') { list_list(ch,immune_bits); return(1); }
	else bit_vector(&i,ch,arg,immune_bits);
  } else if (isdigit(*buf) || *buf=='+' || *buf=='-') {
    i=atoi(buf);
    if (i<apply_list[loc].min) {
      sprintf(buf,"Minimum value is %d.\n",apply_list[loc].min);
      SEND(buf);
      return(1);
    }
    if (i>apply_list[loc].max) {
      sprintf(buf,"Maximum value is %d.\n",apply_list[loc].max);
      SEND(buf);
      return(1);
    }
  } else {
    SEND("You need to specify a modifier along with that!\n");
    return(1);
  }
  BUILD->affected[which-1].location=loc+1;
  BUILD->affected[which-1].modifier=i;
  return(2);
}

PRE_HDR(pre_DONE) {
  SEND("Done! ;-)\n");
  ch->desc->connected=CON_PLYNG;
}

PRE_HDR(pre_AFFECT5) {
  SEND("Affect 5: ");
  STATE(ch)=O_AFFECT5;
}

STATE_HDR(state_AFFECT5) {
  int r;
  r=generic_affect(ch,arg,5);
  if (!r) pre_DONE(ch);
  else if (r==1) pre_AFFECT5(ch);
  else pre_DONE(ch);
} 

PRE_HDR(pre_AFFECT4) {
  SEND("Affect 4: ");
  STATE(ch)=O_AFFECT4;
}

STATE_HDR(state_AFFECT4) {
  int r;
  r=generic_affect(ch,arg,4);
  if (!r) pre_DONE(ch);
  else if (r==1) pre_AFFECT4(ch);
  else pre_AFFECT5(ch);
}

PRE_HDR(pre_AFFECT3) {
  SEND("Affect 3: ");
  STATE(ch)=O_AFFECT3;
}

STATE_HDR(state_AFFECT3) {
  int r;
  r=generic_affect(ch,arg,3);
  if (!r) pre_DONE(ch);
  else if (r==1) pre_AFFECT3(ch);
  else pre_AFFECT4(ch);
}

PRE_HDR(pre_AFFECT2) {
  SEND("Affect 2: ");
  STATE(ch)=O_AFFECT2;
}

STATE_HDR(state_AFFECT2) {
  int r;
  r=generic_affect(ch,arg,2);
  if (!r) pre_DONE(ch);
  else if (r==1) pre_AFFECT2(ch);
  else pre_AFFECT3(ch);
}

PRE_HDR(pre_AFFECT1) {
  SEND("Affect 1: ");
  STATE(ch)=O_AFFECT1;
}

STATE_HDR(state_AFFECT1) {
  int r;
  r=generic_affect(ch,arg,1);
  if (!r) pre_DONE(ch);
  else if (r==1) pre_AFFECT1(ch);
  else pre_AFFECT2(ch);
}

PRE_HDR(pre_BITVECTOR) {
  /*print_flags(ch,BUILD->obj_flags.bitvector,bitvector_bits);*/
  SEND("Player bits: ");
  STATE(ch)=O_BITVECTOR;
}

STATE_HDR(state_BITVECTOR) {
  if (*arg=='?') {
    list_list(ch,bitvector_bits);
    pre_BITVECTOR(ch);
  } else if (*arg) {
    /*bit_vector(&BUILD->obj_flags.bitvector,ch,arg,bitvector_bits);*/
    pre_BITVECTOR(ch);
  } else pre_AFFECT1(ch);
}


PRE_HDR(pre_EXTRA) {
  print_flags(ch,BUILD->obj_flags.extra_flags,extra_bits);
  SEND("Extra flags: ");
  STATE(ch)=O_EXTRA;
}

STATE_HDR(state_EXTRA) {
  if (*arg=='?') {
    list_list(ch,extra_bits);
    pre_EXTRA(ch);
  } else if (*arg) {
    bit_vectorl(&(BUILD->obj_flags.extra_flags),ch,arg,extra_bits);
    pre_EXTRA(ch);
  } else pre_BITVECTOR(ch);
}

PRE_HDR(pre_WEAR) {
  print_flags(ch,BUILD->obj_flags.wear_flags,wear_bits);
  SEND("Wear flags: ");
  STATE(ch)=O_WEAR;
}

STATE_HDR(state_WEAR) {
  if (*arg=='?') {
    list_list(ch,wear_bits);
    pre_WEAR(ch);
  } else if (*arg) {
    bit_vectorl(&BUILD->obj_flags.wear_flags,ch,arg,wear_bits);
    pre_WEAR(ch);
  } else pre_EXTRA(ch);
}

PRE_HDR(pre_RENT) {
  char buf[80];
  SEND("Rent [");
  sprintf(buf,"%d",BUILD->obj_flags.cost/10);
  SEND(buf);
  SEND("] : ");
  STATE(ch)=O_RENT;
}

STATE_HDR(state_RENT) {
  if (!*arg) {
    BUILD->obj_flags.cost_per_day=BUILD->obj_flags.cost/10;
    pre_WEAR(ch);
  } else if (isdigit(*arg)) {
    BUILD->obj_flags.cost_per_day=atoi(arg);
    pre_WEAR(ch);
  } else {
    SEND("Either hit enter to accept the default value, or type in a positive number!\n");
    pre_RENT(ch);
  }
}


PRE_HDR(pre_COST) {
  SEND("Cost: ");
  STATE(ch)=O_COST;
}

STATE_HDR(state_COST) {
  if (isdigit(*arg)) {
    BUILD->obj_flags.cost=atoi(arg);
    pre_RENT(ch);
  } else if (*arg=='?') {
    SEND(COST_HELP);
    pre_COST(ch);
  } else {
    SEND("Enter a positive number, or ? for price guidelines.\n");
    pre_COST(ch);
  }
}

PRE_HDR(pre_WEIGHT) {
  SEND("Weight: ");
  STATE(ch)=O_WEIGHT;
}

STATE_HDR(state_WEIGHT) {
  if (isdigit(*arg)) {
    BUILD->obj_flags.weight=atoi(arg);
    pre_COST(ch);
  } else if (*arg=='?') {
    SEND(WEIGHT_HELP);
    pre_WEIGHT(ch);
  } else {
    SEND("Enter a positive number, or ? for weight guidelines.\n");
    pre_WEIGHT(ch);
  }
}

PRE_HDR(pre_VALUE3) {
  switch(BUILD->obj_flags.type_flag) {
    case ITEM_SCROLL:
    case ITEM_POTION: SEND("Spell 3: "); break;
    case ITEM_WAND:
    case ITEM_STAFF:  SEND("Spell: "); break;
    case ITEM_WEAPON: SEND("Weapon type: "); break;
    case ITEM_ARMOR: SEND("Absorb (stopping power): "); break;
    case ITEM_DRINKCON:
    case ITEM_FOOD: SEND("Poisoned? "); break;
    default: pre_WEIGHT(ch); return;
  }
  STATE(ch)=O_VALUE3;
}

STATE_HDR(state_VALUE3) {
  int type,val=0;
  char buf[MAX_INPUT_LENGTH];

  type=BUILD->obj_flags.type_flag;
  if (type==ITEM_POTION || type==ITEM_SCROLL || type==ITEM_WAND ||
		type==ITEM_STAFF) {
    for (; *arg && isspace(*arg); arg++) ;
    if (*arg=='?') { list_list(ch,spells); pre_VALUE3(ch); return; }
    if (*arg) val=my_search(arg,spells);
    else { pre_WEIGHT(ch); return; }
    if (val<0) { pre_VALUE3(ch); return; }
  } else if (type==ITEM_FOOD || type==ITEM_DRINKCON) {
    for (; *arg && isspace(*arg); arg++) ;
    if (*arg=='Y' || *arg=='y') val = 1;
    else if (*arg=='N' || *arg=='n') val=0;
    else {
	SEND("Please respond with a 'Y' or an 'N\n");
	pre_VALUE3(ch);
	return;
    }
  } else if (type==ITEM_WEAPON) {
    for (; *arg && isspace(*arg); arg++) ;
    if (*arg) val=my_search(arg,hit_type)-2;
    if (val<-1) {
	list_list(ch,hit_type);
	pre_VALUE3(ch);
	return;
    }
  } else {
    one_argument(arg,buf);
    val=atoi(buf);
    if (val==0 && !isdigit(*buf)) {
      SEND("Expecting a number.\n");
      pre_VALUE3(ch);
      return;
    }
  }
/* Range Checking */
  switch (type) {
    case ITEM_ARMOR:
	if ((val<0 || val>5) && GetMaxLevel(ch)<SILLYLORD) {
		SEND("Legal values are between 0 and 5.\n");
		pre_VALUE3(ch);
		return;
	}
  }
  BUILD->obj_flags.value[3]=val;
  pre_WEIGHT(ch);
}

PRE_HDR(pre_VALUE2) {
  switch(BUILD->obj_flags.type_flag) {
    case ITEM_POTION:
    case ITEM_SCROLL: SEND("Spell 2: "); break;
    case ITEM_STAFF:
    case ITEM_WAND:  BUILD->obj_flags.value[2]=BUILD->obj_flags.value[1];
		     pre_VALUE3(ch);
		     return;
    case ITEM_LIGHT: SEND("Hours of light (-1=forever): "); break;
    case ITEM_WEAPON: SEND("Size of damage dice: "); break;
    case ITEM_ARMOR: pre_VALUE3(ch); return;
    case ITEM_CONTAINER: SEND("Key vnum (-1=not lockable): "); break;
    case ITEM_DRINKCON: SEND("Type of liquid: "); break;
    default: pre_WEIGHT(ch); return;
  }
  STATE(ch)=O_VALUE2;
}

STATE_HDR(state_VALUE2) {
  int type;
  char buf[MAX_INPUT_LENGTH];
  int val=0;

  type=BUILD->obj_flags.type_flag;
  if (type==ITEM_POTION || type==ITEM_SCROLL) {
    for (; *arg && isspace(*arg); arg++) ;
    if (*arg=='?') { list_list(ch,spells); pre_VALUE2(ch); return; }
    if (*arg) val=my_search(arg,spells);
    else { pre_VALUE3(ch); return; }
    if (val<0) { pre_VALUE2(ch); return; }
  } else if (type==ITEM_DRINKCON) {
    for (; *arg && isspace(*arg); arg++) ;
    if (*arg) val=my_search(arg,drinknames)-1;
    if (val<0) { list_list(ch,drinknames); pre_VALUE2(ch); return; }
  } else {
    one_argument(arg,buf);
    val=atoi(buf);
    if (val==0 && !isdigit(*buf)) {
      SEND("Expecting a number.\n");
      pre_VALUE2(ch);
      return;
    }
  }
/* Range Checking */
  switch (type) {
    case ITEM_WEAPON:
	if ((val<1 || val>10) && GetMaxLevel(ch)<SILLYLORD) {
		SEND("Legal values are between 1 and 10\n");
		pre_VALUE2(ch);
		return;
	}
    case ITEM_CONTAINER:
	if (val<-1) {
		SEND("Must be greater than or equal to -1.\n");
		pre_VALUE2(ch);
		return;
	}
  }
  BUILD->obj_flags.value[2]=val;
  pre_VALUE3(ch);
}

PRE_HDR(pre_VALUE1) {
  switch (BUILD->obj_flags.type_flag) {
    case ITEM_POTION:
    case ITEM_SCROLL: SEND("Spell 1: "); break;
    case ITEM_STAFF:
    case ITEM_WAND: SEND("Charges: "); break;
    case ITEM_WEAPON: SEND("Number of damage dice: "); break;
    case ITEM_ARMOR: SEND("Force of armor before destroyed: "); break;
    case ITEM_CONTAINER: SEND("Container flags: "); break;
    case ITEM_DRINKCON: SEND("Number of drink units that are left: "); break;
    default:
      pre_WEIGHT(ch);
      return;
  }
  STATE(ch)=O_VALUE1;
}

STATE_HDR(state_VALUE1) {
  int type;
  char buf[MAX_INPUT_LENGTH];
  int val;
  static char *cont_bits[] = {
	"closable","pickproof","closed","locked","\n"};

  type=BUILD->obj_flags.type_flag;
  if (type==ITEM_POTION || type==ITEM_SCROLL) {
    for (; *arg && isspace(*arg); arg++) ;
    if (*arg=='?') { list_list(ch,spells); pre_VALUE1(ch); return; }
    else if (*arg) val=my_search(arg,spells);
    else { pre_VALUE2(ch); return; }
    if (val<0) { pre_VALUE1(ch); return; }
  } else if (type==ITEM_CONTAINER) {
    val=0;
    if (*arg=='?') { list_list(ch,cont_bits); pre_VALUE1(ch); return; }
    else bit_vector(&val,ch,arg,cont_bits);
  } else {
    one_argument(arg,buf);
    val=atoi(buf);
    if (val==0 && !isdigit(*buf)) {
      SEND("Expecting a number.\n");
      pre_VALUE1(ch);
      return;
    }
  }
/* Range Checking */
  switch (type) {
    case ITEM_WAND:
    case ITEM_STAFF:
	if (val<0 || val>127) {
	  SEND("Legal values are between 0 and 127.\n");
	  pre_VALUE1(ch);
	  return;
	}
    case ITEM_WEAPON:
	if (val<1 || val>10) {
	  SEND("Legal values are between 1 and 10.\n");
	  pre_VALUE1(ch);
	  return;
        }
    case ITEM_ARMOR:
	if ((val<0 || val>5) && GetMaxLevel(ch)<SILLYLORD) {
	  SEND("Legal values are between 0 and 5.\n");
	  pre_VALUE1(ch);
	  return;
	}
    case ITEM_DRINKCON:
	if (val<0) {
	  SEND("Must be greater than 0.\n");
	  return;
	}
  }
  BUILD->obj_flags.value[1]=val;
  pre_VALUE2(ch);
}

PRE_HDR(pre_VALUE0) {
  switch (BUILD->obj_flags.type_flag) {
    case ITEM_LIGHT: pre_VALUE2(ch); return;
    case ITEM_SCROLL: SEND("Level of spells on scroll: "); break;
    case ITEM_WAND: SEND("Level of spells in wand: "); break;
    case ITEM_STAFF: SEND("Level of spells in staff: "); break;
    case ITEM_WEAPON: pre_VALUE1(ch); return;
    case ITEM_ARMOR: SEND("Effective AC: "); break;
    case ITEM_POTION: SEND("Level of spells in potion: "); break;
    case ITEM_CONTAINER: SEND("Maximum weight container can carry: "); break;
    case ITEM_NOTE: SEND("Language: "); break;
    case ITEM_DRINKCON: SEND("Maximum drink-units: "); break;
    case ITEM_KEY: SEND("Key-type (I doubt this works): "); break;
    case ITEM_FOOD: SEND("How filling (in hours): "); break;
    case ITEM_MONEY: SEND("How many coins: "); break;
    default: pre_WEIGHT(ch); return;
  }
  STATE(ch)=O_VALUE0;
}

STATE_HDR(state_VALUE0) {
  char buf[MAX_INPUT_LENGTH];
  int val;

  one_argument(arg,buf);
  val=atoi(buf);
  if (val==0 && !isdigit(*buf)) {
    SEND("Expecting a number.\n");
    pre_VALUE0(ch);
    return;
  }
/* range checking */
  switch (BUILD->obj_flags.type_flag) {
    case ITEM_SCROLL:
    case ITEM_WAND:
    case ITEM_STAFF:
    case ITEM_POTION:
	if (val<1 || val>500) {
		SEND("Legal values are between 1 and 500 inclusive.\n");
		pre_VALUE0(ch);
		return;
	}
    case ITEM_CONTAINER:
	if (val<1 || val>1000) {
		SEND("Legal values are between 1 and 1000 inclusive.\n");
		pre_VALUE0(ch);
		return;
	}
    case ITEM_ARMOR:
	if ((val<-15 || val>15) && GetMaxLevel(ch)<SILLYLORD) {
		SEND("Legal values are between -15 and 15 incluseive.\n");
		pre_VALUE0(ch);
		return;
	}
  }
  BUILD->obj_flags.value[0] = val;
  pre_VALUE1(ch);
}

PRE_HDR(pre_TYPE) {
  SEND("Type: ");
  STATE(ch)=O_TYPE;
}

STATE_HDR(state_TYPE) {
  int type;

  type=atoi(arg);
  if (!isdigit(*arg))
    type=my_search(arg,obj_type);
  if (range_check_list(ch,type,obj_type)) {
    BUILD->obj_flags.type_flag=type;
/* Set up some default flags for ceartain items */
    switch (type) {
	case ITEM_LIGHT:
		BUILD->obj_flags.wear_flags=ITEM_TAKE | ITEM_LIGHT;
		break;
	case ITEM_SCROLL:
	case ITEM_WAND:
	case ITEM_STAFF:
	case ITEM_POTION:
	case ITEM_CONTAINER:
	case ITEM_DRINKCON:
	case ITEM_KEY:
	case ITEM_FOOD:
		BUILD->obj_flags.wear_flags=ITEM_HOLD;
	case ITEM_ARMOR:
		BUILD->obj_flags.wear_flags |= ITEM_TAKE;
		break;
	case ITEM_WEAPON:
		BUILD->obj_flags.wear_flags=ITEM_TAKE | ITEM_WIELD;
		break;
    }
    pre_VALUE0(ch);
  } else pre_TYPE(ch);
}

PRE_HDR(pre_ACTION) {
  if (GetMaxLevel(ch)<IMPLEMENTOR)
    pre_TYPE(ch);
  else {
    STATE(ch)=O_ACTION;
    SEND("Action Description:\n");
  }
}

STATE_HDR(state_ACTION) {
  STRING(action_description,arg);
  pre_TYPE(ch);
}

PRE_HDR(pre_SHORT) {
  STATE(ch)=O_SHORT;
  SEND("Short description:\n");
}

STATE_HDR(state_SHORT) {
  if (*arg=='?') { SEND(SHORT_NAME_HELP); pre_SHORT(ch); return; }
  STRING(short_description,arg);
  pre_ACTION(ch);
}

PRE_HDR(pre_DESCRIPTION) {
  STATE(ch)=O_DESCRIPTION;
  SEND("Description when in room:\n");
}

STATE_HDR(state_DESCRIPTION) {
  if (*arg=='?') { SEND(DESCRIPTION_HELP); pre_DESCRIPTION(ch); return; }
  STRING(description,arg);
  pre_SHORT(ch);
}

STATE_HDR(state_NAME) {
  if (*arg=='?') { SEND(NAME_HELP); SEND("Name list: "); return; }
  STRING(name,arg);
  pre_DESCRIPTION(ch);
}


void oedit_prompt(struct char_data *ch,char *arg)
{
  char buf[MAX_INPUT_LENGTH];

  one_argument(arg,buf);
  if (!str_cmp(buf,"quit")) {
	pre_DONE(ch);
	return;
  }
  ch->desc->connected=CON_OBJECT;
  WAIT_STATE(ch,0);
  switch (STATE(ch)) {
	case O_NAME:		state_NAME(ch,arg);		break;
	case O_DESCRIPTION:	state_DESCRIPTION(ch,arg);	break;
	case O_SHORT:		state_SHORT(ch,arg);		break;
	case O_ACTION:		state_ACTION(ch,arg);		break;
	case O_TYPE:		state_TYPE(ch,arg);		break;
	case O_VALUE0:		state_VALUE0(ch,arg);		break;
	case O_VALUE1:		state_VALUE1(ch,arg);		break;
	case O_VALUE2:		state_VALUE2(ch,arg);		break;
	case O_VALUE3:		state_VALUE3(ch,arg);		break;
	case O_WEIGHT:		state_WEIGHT(ch,arg);		break;
	case O_COST:		state_COST(ch,arg);		break;
	case O_RENT:		state_RENT(ch,arg);		break;
	case O_WEAR:		state_WEAR(ch,arg);		break;
	case O_EXTRA:		state_EXTRA(ch,arg);		break;
	case O_BITVECTOR:	state_BITVECTOR(ch,arg);	break;
	case O_AFFECT1:		state_AFFECT1(ch,arg);		break;
	case O_AFFECT2:		state_AFFECT2(ch,arg);		break;
	case O_AFFECT3:		state_AFFECT3(ch,arg);		break;
	case O_AFFECT4:		state_AFFECT4(ch,arg);		break;
	case O_AFFECT5:		state_AFFECT5(ch,arg);		break;
	default:
		log("Mess up in oedit_prompt");
		ch->desc->connected=CON_PLYNG;
  }
}
