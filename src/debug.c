#include <ctype.h>

#include <fcns.h>
#include <externs.h>
#include <utils.h>

int DEBUG=0;

int is_alpha(char *s)
{
  if (!s) return(0);
  for (;*s;s++) if (!isprint(*s)) return(0);
  return(1);
}

int valid_player(struct char_data *ch)
{
  if (!is_alpha(ch->player.name)) return(0);
  if (GetMaxLevel(ch)<1 || GetMaxLevel(ch)>2010) return(0);
  return(1);
}

int valid_object(struct obj_data *obj)
{
  return(1);
  if (!is_alpha(obj->name)) return(0);
  if (obj->equipped_by || obj->in_obj || obj->carried_by ||
	real_roomp(obj->in_room)) return(1);
  return(0);
}

#if 0
static void not_valid(char *s)
{
  nlog("ERROR: %s",s);
}
#endif

void integrity_check()
{
#if 0
  extern struct obj_data *object_list;
  extern struct char_data *character_list;
  struct char_data *ch;
  struct obj_data *obj;
  static int last=0;

  if (character_list->equipment[18] && last!=character_list->equipment[18]->virtual) {
    last=character_list->equipment[18]->virtual;
    nlog("new=%d",last);
  }

  for (ch=character_list; ch; ch=ch->next)
    if (!valid_player(ch))
	not_valid("Player/Mob");
  for (obj=object_list; obj; obj=obj->next)
    if (!valid_object(obj)) {
	nlog("Invalid object: %d %s",obj->virtual,obj->name);
	extract_obj(obj);
	obj=object_list;
    }
#endif
}
