#include <stdio.h>
//#include <varargs.h>
#include <stdarg.h>

/* don't include prototype for schedule */
#define _MISC_PROTO1_
#include <fcns.h>
#include <utils.h>
#include <externs.h>

static char *event_types[] = {
  "None",
  "Room Flag",
  "Char Data",
  "Function Call",
  "Object",
  "\n"
};

static int current_tick=0;

typedef struct ev_room_flag {
  u32 room;
  u32 bitv;
} ev_room_flag;

typedef struct ev_char_data {
  struct char_data *ch;
  u32 data;
} ev_char_data;

typedef struct ev_object {
  struct obj_data *obj;
} ev_object;

typedef struct event {
  u32 id;
  u32 tick;
  void (*func)();
  int type;
  struct event *next;
  union {
    struct ev_room_flag room_flag; /* func(room,bitv) */
    struct ev_char_data char_dat;  /* func(ch,data) */
    struct ev_object    object;    /* func(object) */
  } x;
} event;

static event *event_list=NULL;

//void schedule(va_alist) va_dcl {
void schedule(int in_ticks, void * func, int type, ...) {
  //int in_ticks;
  event *ev,*i;
  //void *func;
  //int type;
  static int next_id=1;
  va_list args;

  CREATE(ev,event,1);
  bzero(ev,sizeof(event));

  //in_ticks=va_arg(args,int);
  //func=va_arg(args,void *);
  //type=va_arg(args,int);
  ev->id = next_id++;
  ev->tick=current_tick+in_ticks;
  ev->func=func;
  ev->type=type;
  
  va_start(args, type);

  switch (type) {
    case EV_FUNC: break;
    case EV_ROOM_FLAG:
	ev->x.room_flag.room=va_arg(args,u32);
	ev->x.room_flag.bitv=va_arg(args,u32);
	break;
    case EV_CHAR_DATA:
	ev->x.char_dat.ch=va_arg(args,struct char_data *);
	ev->x.char_dat.data=va_arg(args,u32);
	break;
    case EV_OBJECT:
	ev->x.object.obj=va_arg(args,struct obj_data *);
	break;
    default:
	vlog(LOG_URGENT,"Event type %d illegal in schedule.",type);
  }
  va_end(args);

  if (DEBUG) vlog(LOG_DEBUG,"Schedued event type %d for %d",
			ev->type,ev->tick);

/* insert event into list... Keep it sorted by tick number */
  if (!event_list) {
    event_list=ev;
    return;
  }
  if (ev->tick<event_list->tick) { /* prepend to list */
    ev->next=event_list;
    event_list=ev;
    return;
  }
  for (i=event_list; i->next; i=i->next)
    if (ev->tick < i->next->tick) break;
  ev->next=i->next;
  i->next=ev;
}

int validate_char(struct char_data *ch)
{
  struct char_data *i;
  for (i=character_list; i; i=i->next) if (i==ch) return(1);
  return(0);
}

int validate_obj(struct obj_data *obj)
{
  struct obj_data *i;
  for (i=object_list; i; i=i->next) if (i==obj) return(1);
  return(0);
}

void event_tick()
{
  event *ev;
  current_tick++;
  if (DEBUG>1) vlog(LOG_DEBUG,"Tick %d",current_tick);
  if (!event_list) return;
  while (event_list->tick<=current_tick) {
    ev=event_list;
    if (DEBUG) vlog(LOG_DEBUG,"Event type %d",ev->type);
    event_list=event_list->next;
    switch (ev->type) {
      case EV_FUNC:
	ev->func();
	break;
      case EV_ROOM_FLAG:
	ev->func(ev->x.room_flag.room,ev->x.room_flag.bitv);
	break;
      case EV_CHAR_DATA:
	if (validate_char(ev->x.char_dat.ch)) /* check if ch still valid */
	  ev->func(ev->x.char_dat.ch,ev->x.char_dat.data);
	break;
      case EV_OBJECT:
	if (validate_obj(ev->x.object.obj)) /* check if obj still valid */
	  ev->func(ev->x.object.obj);
	break;
    }
    FREE(ev);
    if (!event_list) return;
  }
}

void do_events(struct char_data *ch, char *arg,int cmd)
{
  event *ev;
  struct string_block sb;

  if (!event_list) {
    sendf(ch,"No events pending.\n");
    return;
  }

  init_string_block(&sb);
  sb_printf(&sb,"Current tick is %d.\n",current_tick);
  for (ev=event_list; ev; ev=ev->next) {
    sb_printf(&sb,"ID: %d Tick: %d Type: %s\n",
	ev->id,
	ev->tick,
	event_types[ev->type]);
  }
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
}

void do_cancel(struct char_data *ch, char *arg, int cmd)
{
  int id;
  event *ev,*last;
  id=atoi(arg);
  if (id<=0) {
    sendf(ch,"Syntax: cancel <event_number>\n");
    return;
  }
  last=NULL;
  for (ev=event_list; ev; ev=ev->next)
    if (ev->id==id) break;
    else last=ev;
  if (!ev) {
    sendf(ch,"Couldn't find event #%d\n",id);
    return;
  }
  if (ev!=event_list)
    last->next=ev->next;
  else
    event_list=ev->next;
  FREE(ev);
  sendf(ch,"Ok.\n");
}
