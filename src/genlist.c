#include <fcns.h>
#include <utils.h>
#include <db.h>

int bad_vnum(int v)
{
  return(0);
  if (v<32000) return(0);
  if (v<35000) return(1);
  if (v<36000) return(0);
  if (v<45500) return(1);
  if (v<46000) return(0);
  if (v<50500) return(1);
  if (v<51000) return(0);
  return(1);
}

struct obj_data *object_list,*obj;

#define MINUTE (60)
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)
#define WEEK (7*DAY)
#define MONTH (4*WEEK)
#define YEAR (12*MONTH)

int should_delete(struct char_data *ch)
{
  if (ch->player.level[0]<5) return(1);
  if (ch->player.level[0]>2000) return(0);
  if (time(0)-ch->player.time.logon>2*MONTH) return(1);
  return(0);
}

void free_char(struct char_data *ch)
{
          FREE(ch->player.name);
          FREE(ch->player.title);
          FREE(ch->player.short_descr);
          FREE(ch->player.long_descr);
          FREE(ch->player.description);
          FREE(ch->player.sounds);
          FREE(ch->player.distant_snds);
          FREE(ch->player.level);
          FREE(ch->player.name);
          FREE(ch->player.title);
          FREE(ch->player.short_descr);
          FREE(ch->player.long_descr);
          FREE(ch->player.description);
          FREE(ch->player.sounds);
          FREE(ch->player.distant_snds);
          FREE(ch->player.level);
	  ch->affected=NULL;
	  FREE(ch);
}

int DEBUG=0;
void main(int argc, char **argv)
{
  struct char_data *ch;
  struct obj_data *o,*p;
  FILE *f,*g;
  char s[80];
  int i,look,count;

  object_list=NULL;

  f=fopen("lib2/players.new","r");
  if (!f) { perror("lib2/players.new"); exit(1); }
  g=fopen("to_delete","w+");
  count=0;
  while (fgets(s,80,f)) {
    s[strlen(s)-1]='\0';
    fprintf(stderr, "%d:%s%c[K",count++,s,27);
    bzero(&ch,sizeof(ch));
    o=object_list;
    CREATE(ch,struct char_data,1);
    if (new_load_char(ch,s)>0) {
      for (p=object_list; p!=o; p=p->next)
	if (bad_vnum(p->virtual)) printf("%s %d\n",ch->player.name,p->virtual);
      if (should_delete(ch))
	fprintf(g,"%-20s %4d %3d\n",ch->player.name,ch->player.level[0],(time(0)-ch->player.time.logon)/DAY);
      if (ch->player.level[0]>2000) {
        printf("%-20s %4d %7d %7d\n",ch->player.name,ch->player.level[0],
		ch->build_lo, ch->build_hi);
      }
      free_char(ch);
      ch=NULL;
    } else fprintf(g,"Couldn't load %s\n",s);
  }
  fclose(f);
  fclose(g);
  printf("\n\n");
  for (i=1; i<argc; i++) {
    look=atoi(argv[i]);
    obj=object_list;
    count=0;
    while (obj) {
      if (obj->virtual==look) count++;
      if (count==1) strcpy(s,obj->name);
      obj=obj->next;
    }
    if (count) fprintf(stderr,"%5d %5d %s\n",look,count,s);
  }
}
