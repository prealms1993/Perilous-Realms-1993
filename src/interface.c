#include <fcns.h>
#define MAX_FIELD_LENGTH 80

typedef struct item {
  char *tag;   /* text description of this field */
  int tag_x,tag_y;     /* screen location of the tag */
  int field_x,field_y; /* location of input area */
} item;

typedef struct field {
  item info;
  char buf[MAX_FIELD_LENGTH];
  struct field *next;
} field;

field *make_item(char *tag,int x, int y, int field_x, int field_y)
{
  field *f;

  if (field_x < 0) field_x = strlen(tag)+x+1;
  if (field_t < 0) field_y = y;

  CREATE(f,field,1);
  bzero(f,sizeof(f));

  f->tag=mystrdup(tag);
  f->tag_x=x;
  f->tag_y=y;
  f->field_x=field_x;
  f->field_y=field_y;
  return(f);
}

void edit_interface()
{
  field *p, *edit_fields;

  p = edit_fields = make_item("Room name: ",1,1,-1,-1);
  p = p->next = make_item("Room num : ",1,2,-1,-1);
  p = p->next = make_item("Exit 1   : ",1,3,-1,-1);
}

void move_to(struct char_data *ch,char x, char y)
{
  sendf(ch,"\1\1%c%c",y,x);
}

void interface(field *p)
{
  
}
