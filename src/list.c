#include <fcns.h>
#include <utils.h>
#include <db.h>

struct obj_data *object_list,*obj;

#define MINUTE (60)
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)
#define WEEK (7*DAY)
#define MONTH (4*WEEK)
#define YEAR (12*MONTH)

int DEBUG=0;
void main(int argc, char **argv)
{
  struct char_data *ch;
  struct obj_data *o,*p;
  FILE *f;
  char s[80];
  int i,look,count;

  object_list=NULL;

  f=fopen("lib/players.new","r");
  if (!f) { perror("lib/players.new"); exit(1); }
  count=0;
  while (fgets(s,80,f)) {
    if (++count%100==0) fprintf(stderr,"%d",count);
    s[strlen(s)-1]='\0';
    CREATE(ch,struct char_data,1);
    bzero(ch,sizeof(struct char_data));
    if (new_load_char(ch,s)>0)
      printf("%-20s %4d %3d\n",ch->player.name,ch->player.level,(time(0)-ch->player.time.logon)/DAY);
    else printf("Couldn't load %s\n",s);
    while (object_list) {
	o=object_list->next;
	free_obj(object_list);
	object_list=o;
    }
    free_char(ch);
    ch=NULL;
  }
  fclose(f);
}
