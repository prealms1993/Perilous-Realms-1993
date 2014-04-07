#include <fcns.h>
#include <db.h>

struct obj_data *object_list,*obj;

#define MINUTE (60)
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)
#define WEEK (7*DAY)
#define MONTH (4*WEEK)
#define YEAR (12*MONTH)

unsigned long time_frame;

int should_delete(struct char_data *ch)
{
  ch->specials.last_logon=ch->player.time.logon;
  if (time(0)-ch->specials.last_logon < DAY) return(0);
  if (ch->player.level<=10 && !ch->carrying) return(1);
  if (ch->player.level>2010) return(1);
  if (ch->player.level>2000) return(0);
  if (time(0)-ch->specials.last_logon> time_frame) return(1);
  return(0);
}

int DEBUG=0;
void main(int argc, char **argv)
{
  struct char_data ch;
  struct obj_data *o,*p;
  FILE *f,*g;
  char s[80];
  int i,look,count;

  object_list=NULL;

  if (argc!=2) {
	fprintf(stderr,"purge <min days>\n");
	exit(1);
  }
  time_frame=atoi(argv[1])*DAY;
  if (time_frame<30*DAY) {
	fprintf(stderr,"purge <min days>\n\nmin days must be>=30.\n");
	exit(1);
  }
  f=fopen("players.new","r");
  if (!f) { perror("players.new"); exit(1); }
  g=fopen("immortals","w");
  if (!g) { perror("immortals"); exit(1); }
  count=0;
  while (fgets(s,80,f)) {
    s[strlen(s)-1]='\0';
    fprintf(stderr, "%d:%s%c[K",count++,s,27);
    bzero(&ch,sizeof(struct char_data));

    if (new_load_char(&ch,s)>0) {
      if (should_delete(&ch)) {
	printf("%-20s %4d %3d\n",s,ch.player.level,
		(time(0)-ch.specials.last_logon)/DAY);
	unlink(name_to_path(s));
      } else if (ch.player.level>=2000)
        fprintf(g,"%-20s %4d %7d %7d\n",ch.player.name,ch.player.level,
                ch.build_lo, ch.build_hi);
      while (object_list) {
	struct obj_data *o;
	o=object_list->next;
	free_obj(object_list);
	object_list=o;
      }
    } else {
	printf("Couldn't load %s\n",s);
	unlink(name_to_path(s));
    }
  }
  fclose(f);
  fclose(g);
}
