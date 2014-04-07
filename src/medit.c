/*****************************************************************************
**									    **
**			OEdit.c--By Adam Caldwell			    **
**									    **
*****************************************************************************/

#include <fcns.h>
#include <ticks.h>
#include <utils.h>
#include <medit.h>

#define STATE(ch) ch->build.state
#define BUILD ch->build.mob_build
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

#define M_NAME 1
#define M_SHORT 2
#define M_LONG 3
#define M_DESCRIPTION 4

#define STATE_HDR(nme) static void nme(struct char_data *ch, char *arg)
#define PRE_HDR(nme) static void nme(struct char_data *ch)
#define SEND(string) send_to_char(string,ch);

PRE_HDR(pre_DONE) {
  SEND("Done! ;-)\n");
  ch->desc->connected=CON_PLYNG;
}

PRE_HDR(pre_DESCRIPTION) {
  STATE(ch)=M_DESCRIPTION;
  SEND("Description when in room:\n");
}

STATE_HDR(state_DESCRIPTION) {
  if (*arg=='?') { SEND(DESCRIPTION_HELP); pre_DESCRIPTION(ch); return; }
  STRING(player.description,arg);
  pre_DONE(ch);
}

PRE_HDR(pre_SHORT) {
  STATE(ch)=M_SHORT;
  SEND("Short description:\n");
}

STATE_HDR(state_SHORT) {
  if (*arg=='?') { SEND(SHORT_NAME_HELP); pre_SHORT(ch); return; }
  STRING(player.short_descr,arg);
  pre_DESCRIPTION(ch);
}

STATE_HDR(state_NAME) {
  if (*arg=='?') { SEND(NAME_HELP); SEND("Name list: "); return; }
  STRING(player.name,arg);
  pre_SHORT(ch);
}


void medit_prompt(struct char_data *ch,char *arg)
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
	case M_NAME:		state_NAME(ch,arg);		break;
	case M_DESCRIPTION:	state_DESCRIPTION(ch,arg);	break;
	case M_SHORT:		state_SHORT(ch,arg);		break;
	case M_LONG:		state_SHORT(ch,arg);		break;
	default:
		log("Mess up in oedit_prompt");
		ch->desc->connected=CON_PLYNG;
  }
}
