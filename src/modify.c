/* ************************************************************************
*  file: modify.c                                         Part of DIKUMUD *
*  Usage: Run-time modification (by users) of game variables              *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************ */


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include <fcns.h>
#include <utils.h>
#include <interpreter.h>
#include <handler.h>
#include <db.h>
#include <comm.h>
#include <externs.h>

#define REBOOT_AT    10  /* 0-23, time of optional reboot if -e lib/reboot */


#define TP_MOB    0
#define TP_OBJ	   1
#define TP_ERROR  2


struct room_data *world;              /* dyn alloc'ed array of rooms     */


char *string_fields[] =
{
	"name",
	"short",
	"long",
	"description",
	"title",
	"delete-description",
	"\n"
};


/* ************************************************************************
*  modification of malloc'ed strings                                      *
************************************************************************ */

/* Add user input to the 'current' string (as defined by d->str) */

void string_add(struct connection_data *d, char *str)
{
	char *scan;
	char ch;
	int terminator = 0;

	/* determine if this is the terminal string, and truncate if so */
	for (scan = str; *scan; scan++)
	   if ((terminator = (*scan == '@')))  {
		ch=*(scan+1);
		if (ch!='\0' && ch!='\n') {
		  terminator=0;
		  continue;
		}
		*scan = '\0';
		break;
	   }
	
	if (!(*d->str))	{
		if (strlen(str) > d->max_str)  	{
			send_to_char("String too long - Truncated.\n",
			   d->character);
			*(str + d->max_str) = '\0';
			terminator = 1;
		}
		CREATE(*d->str, char, strlen(str) + 3);
		strcpy(*d->str, str);
	} else	{
		if (strlen(str) + strlen(*d->str) > d->max_str)	{
			send_to_char("String too long. Last line skipped.\n",
			   d->character);
			terminator = 1;
		} else 	{
			if (!(*d->str = (char *) realloc(*d->str, strlen(*d->str) + 
		   	strlen(str) + 3)))     	{
				perror("string_add");
				exit(1);
			}
			strcat(*d->str, str);
		}
	}

	if (terminator)	{
	   d->str = 0;
	   if (d->connected == CON_EXDSCR)	{
		  SEND_TO_Q(MENU, d);
		  d->connected = CON_SLCT;
	   }
	   if (d->character->funct) {
	      d->character->funct(d->character);
	      d->character->funct=NULL;
	   }
	} else {
	   strcat(*d->str, "\n");
	}
}


#undef MAX_STR

/* interpret an argument for do_string */
void quad_arg(char *arg, int *type, char *name, int *field, char *string)
{
	char buf[MAX_STRING_LENGTH];

	/* determine type */
	arg = one_argument(arg, buf);
	if (is_abbrev(buf, "char"))
	   *type = TP_MOB;
	else if (is_abbrev(buf, "obj"))
	   *type = TP_OBJ;
	else {
		*type = TP_ERROR;
		return;
	}

	/* find name */
	arg = one_argument(arg, name);

	/* field name and number */
	arg = one_argument(arg, buf);
	if (!(*field = old_search_block(buf, 0, strlen(buf), string_fields, 0)))
	   return;

	/* string */
	for (; isspace(*arg); arg++);
	for (; (*string = *arg); arg++, string++);

	return;
}
	
	 


/* modification of malloc'ed strings in chars/objects */
void do_string(struct char_data *ch, char *arg, int cmd)
{
  
  char name[MAX_STRING_LENGTH], string[MAX_STRING_LENGTH];
  struct extra_descr_data *ed, *tmp;
  int field, type;
  struct char_data *mob;
  struct obj_data *obj;
  extern int length[];

  if (IS_NPC(ch))
    return;
  
  quad_arg(arg, &type, name, &field, string);
  
  if (type == TP_ERROR)	{
    send_to_char(
		 "Syntax:\nstring ('obj'|'char') <name> <field> [<string>].",
		 ch);
    return;
  }
  
  if (!field)	{
    send_to_char("No field by that name. Try 'help string'.\n",
		 ch);
    return;
  }
  
  if (type == TP_MOB)	{
    /* locate the beast */
    if (!(mob = get_char_vis(ch, name))) {
      send_to_char("I don't know anyone by that name...\n", ch);
      return;
    }
    
    switch(field)	{
    case 1: 
      if (!IS_NPC(mob) && GetMaxLevel(ch) < IMPLEMENTOR) {
	send_to_char("You can't change that field for players.", ch);
	return;
      }
      if (!*string) {
	send_to_char("You have to supply a name!\n", ch);
	return;
      }
      if (str_cmp(string,"deth")==0 || str_cmp(string,"alcanzar")==0)
        return;
      ch->desc->str = &mob->player.name;
      if (IS_MOB(mob)) mob->player.name=0;
      if (!IS_NPC(mob))
	send_to_char("WARNING: You have changed the name of a player.\n", ch);
      break;
    case 2:
      if (!IS_NPC(mob)) {
	send_to_char("That field is for monsters only.\n", ch);
	return;
      }
      mob->player.short_descr=0;
      ch->desc->str = &mob->player.short_descr;
      break;
    case 3:
      if (!IS_NPC(mob)){
	send_to_char("That field is for monsters only.\n", ch);
	return;
      }
      mob->player.long_descr=0;
      ch->desc->str = &mob->player.long_descr;
      break;
    case 4:ch->desc->str = &mob->player.description;
	   if (IS_MOB(ch)) mob->player.description=0;
	   break;
    case 5:
      if (IS_NPC(mob))  {
	send_to_char("Monsters have no titles.\n",ch);
	return;
      }
      if ((GetMaxLevel(ch) > GetMaxLevel(mob)) && (ch != mob)) 
	ch->desc->str = &mob->player.title;
      else {
	send_to_char("Sorry, can't set the title of someone of higher level.\n", ch);
	return;
      }
      break;
    default:
      send_to_char("That field is undefined for monsters.\n", ch);
      return;
      break;
    }
  } else {   /* type == TP_OBJ */
    /* locate the object */
    if (!(obj = get_obj_vis(ch, name)))    	{
      send_to_char("Can't find such a thing here..\n", ch);
      return;
    }
    
    switch(field)  	{
      
    case 1: 
      if (!*string) {
	send_to_char("You have to supply a keyword.\n", ch);
	return;
      } else {
	ch->desc->str = &obj->name;
	break;
      }
      break;
    case 2: ch->desc->str = &obj->short_description; break;
    case 3: ch->desc->str = &obj->description; break;
    case 4:
      if (!*string)  	{
	send_to_char("You have to supply a keyword.\n", ch);
	return;
      }
      /* try to locate extra description */
      for (ed = obj->ex_description; ; ed = ed->next)
	if (!ed) {
	  CREATE(ed , struct extra_descr_data, 1);
	  ed->next = obj->ex_description;
	  obj->ex_description = ed;
	  CREATE(ed->keyword, char, strlen(string) + 1);
	  strcpy(ed->keyword, string);
	  ed->description = 0;
	  ch->desc->str = &ed->description;
	  send_to_char("New field.\n", ch);
	  break;
	}
	else if (!str_cmp(ed->keyword, string)) /* the field exists */
	  {
	    FREE(ed->description);
	    ed->description = 0;
	    ch->desc->str = &ed->description;
	    send_to_char(
			 "Modifying description.\n", ch);
	    break;
	  }
      ch->desc->max_str = MAX_STRING_LENGTH;
      return; /* the stndrd (see below) procedure does not apply here */
      break;
    case 6: /* deletion */
      if (!*string)  	{
	send_to_char("You must supply a field name.\n", ch);
	return;
      }
      /* try to locate field */
      for (ed = obj->ex_description; ; ed = ed->next)
	if (!ed) {
	  send_to_char("No field with that keyword.\n", ch);
	  return;
	} else if (!str_cmp(ed->keyword, string)) {
	  FREE(ed->keyword);
	  FREE(ed->description);
	  
	  /* delete the entry in the desr list */						
	  if (ed == obj->ex_description)
	    obj->ex_description = ed->next;
	  else {
	    for(tmp = obj->ex_description; tmp->next != ed; 
		tmp = tmp->next);
	    tmp->next = ed->next;
	  }
	  FREE(ed);
	  
	  send_to_char("Field deleted.\n", ch);
	  return;
	}
      break;				
    default:
      send_to_char(
		   "That field is undefined for objects.\n", ch);
      return;
      break;
    }
  }
  
  FREE(*ch->desc->str);
  
  if (*string) {   /* there was a string in the argument array */ 
    if (strlen(string) > length[field - 1])	{
      char *p;
      send_to_char("String too long - truncated.\n", ch);
      p=string;
      p+=length[field-1];
      *p = '\0';
    }
    CREATE(*ch->desc->str, char, strlen(string) + 1);
    strcpy(*ch->desc->str, string);
    ch->desc->str = 0;
    send_to_char("Ok.\n", ch);
  } else {  /* there was no string. enter string mode */
    send_to_char("Enter string. terminate with '@'.\n", ch);
    *ch->desc->str = 0;
    ch->desc->max_str = length[field - 1];
  }
}




void bisect_arg(char *arg, int *field, char *string)
{
  char buf[MAX_INPUT_LENGTH];
  extern char *room_fields[];
  
  /* field name and number */
  arg = one_argument(arg, buf);
  if (!(*field = old_search_block(buf, 0, strlen(buf), room_fields, 0)))
    return;
  
  /* string */
  for (; isspace(*arg); arg++);
  for (; (*string = *arg); arg++, string++);
  
  return;
}



/* db stuff *********************************************** */


/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */

char *one_word(char *argument, char *first_arg )
{
	int found, begin, look_at;

	found = begin = 0;

	do
	{
		for ( ;isspace(*(argument + begin)); begin++);

		if (*(argument+begin) == '\"') {  /* is it a quote */

			begin++;

			for (look_at=0; (*(argument+begin+look_at) >= ' ') && 
			    (*(argument+begin+look_at) != '\"') ; look_at++)
				*(first_arg + look_at) = LOWER(*(argument + begin + look_at));

			if (*(argument+begin+look_at) == '\"')
				begin++;

		} else {

			for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)
				*(first_arg + look_at) = LOWER(*(argument + begin + look_at));

		}

		*(first_arg + look_at) = '\0';
		begin += look_at;
	}
	while (fill_word(first_arg));

	return(argument+begin);
}


struct help_index_element *build_help_index(FILE *fl, int *num)
{
	int nr = -1, issorted, i;
	struct help_index_element *list = 0, mem;
	char buf[81], tmp[81], *scan;
	long pos;

	for (;;)
	{
		pos = ftell(fl);
		fgets(buf, 81, fl);
		*(buf + strlen(buf) - 1) = '\0';
		scan = buf;
		for (;;)
		{
			/* extract the keywords */
			scan = one_word(scan, tmp);

			if (!*tmp)
				break;

			if (!list)
			{
				CREATE(list, struct help_index_element, 1);
				nr = 0;
			}
			else
				RECREATE(list, struct help_index_element, ++nr + 1);

			list[nr].pos = pos;
			CREATE(list[nr].keyword, char, strlen(tmp) + 1);
			strcpy(list[nr].keyword, tmp);
		}
		/* skip the text */
		do
			fgets(buf, 81, fl);
		while (*buf != '#');
		if (*(buf + 1) == '~')
			break;
	}
	/* we might as well sort the stuff */
	do
	{
		issorted = 1;
		for (i = 0; i < nr; i++)
			if (str_cmp(list[i].keyword, list[i + 1].keyword) > 0)
			{
				mem = list[i];
				list[i] = list[i + 1];
				list[i + 1] = mem;
				issorted = 0;
			}
	}
	while (!issorted);

	*num = nr;
	return(list);
}



void page_string(struct connection_data *d, char *str, int keep_internal)
{
	if (!d)
		return;

	if (keep_internal)
	{
		CREATE(d->showstr_head, char, strlen(str) + 1);
		strcpy(d->showstr_head, str);
		d->showstr_point = d->showstr_head;
	}
	else
		d->showstr_point = str;

	show_string(d, "");
}



void show_string(struct connection_data *d, char *input)
{
	char buffer[MAX_STRING_LENGTH], buf[MAX_INPUT_LENGTH];
	register char *scan, *chk;
	int lines = 0;

	one_argument(input, buf);

	if (*buf)
	{
		FREE(d->showstr_head);
		d->showstr_point = 0;
		return;
	}

	/* show a chunk */
	for (scan = buffer;; scan++, d->showstr_point++)
	  if ((*scan = *d->showstr_point) == '\n')
	    lines++;
	  else if (!*scan || (lines >= 22)) {
	  *scan = '\0';
	  SEND_TO_Q(buffer, d);

	  /* see if this is the end (or near the end) of the string */
	  for (chk = d->showstr_point; isspace(*chk); chk++);
	  if (!*chk) {
	    FREE(d->showstr_head);
	    d->showstr_point = 0;
	  }
	  return;
	}
}

