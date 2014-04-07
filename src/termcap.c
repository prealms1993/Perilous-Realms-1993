#include <fcns.h>
#include <utils.h>

static char *type_names[] = {
  "cm",
  "cs",
  ""
};

static char *color_names[] = {
  "c0",
  "c1",
  "c2",
  "c3",
  "c4",
  "c5",
  "c6",
  "c7",
  "c8",
  "c9",
  ""
};

#define MAX_TERMS 20

termcap_entry termlist[MAX_TERMS];

void reload_terms()
{
  int i,j;
  for (i=0; i<MAX_TERMS; i++) {
    FREE(termlist[i].name);
    for (j=0; j<MAX_COLORS; j++)
      FREE(termlist[i].color[j]);
    for (j=0; j<MAX_CAPS; j++)
      FREE(termlist[i].cap[j]);
  }
  init_terms();
}

void init_terms()
{
  FILE *f;
  char s[1024];
  char *p;
  int count=-1;
  int cap;

  bzero(termlist,sizeof(termlist));

  f=fopen("termcap","r");
  if (!f) {
    perror("termcap");
    exit(1);
  }
  while (fgets(s,sizeof(s),f)!=NULL) {
    if (strchr(s,':')) {
      count++;
      s[strlen(s)-2]='\0'; /* chop :\n */
      termlist[count].name=mystrdup(s);
      continue;
    }
    if (!strchr(s,'=')) continue;
    for (p=s; isspace(*p); p++);
    p[strlen(p)-1]='\0'; /* chop \n */
    p[2]='\0'; /* chop off the '=' for now */
    for (cap=0; *type_names[cap]; cap++)
      if (strcmp(type_names[cap],p)==0) break;
    if (!*type_names[cap]) {
      for (cap=0; *color_names[cap]; cap++)
	if (strcmp(color_names[cap],p)==0) break;
      if (!*color_names[cap]) {
	vlog(LOG_URGENT,"termcap: %s not a legal capability\n",p);
	continue;
      }
      termlist[count].color[cap]=mystrdup(p+3);
      continue;
    }
    termlist[count].cap[cap]=mystrdup(p+3);
  }
  fclose(f);
}
