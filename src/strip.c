#include <stdio.h>
#include <string.h>

void strip_file(char *fn, FILE *out)
{
  FILE *f;
  char s[1024],*ss;
  int i;

  f=fopen(fn,"r");
  if (!f)
    return;

  fprintf(out,"\n\n/* File: %s */\n\n",fn);

  while (fgets(s,1024,f)) {
    ss=strchr(s,'\n');
    if (ss) {
      *ss=0;
      i=strlen(s)-1;
      while(i>0&&s[i]==' ') s[i--]=0;
    }
    if ((s[0]) && (strchr("{}\t/# ",s[0])==NULL) && (s[strlen(s)-1]==')')
	&& strncmp("static",s,6))
    {
        fprintf(out,"%s;\n",s);
    }
  }
  fclose(f);
}


/****************************************************************************/

int main(int argc, char *argv[])
{
  int i,i1;
  FILE *out;

  out=fopen("h/fcns.h","w");
  fprintf(out,"#ifndef _FCNS_H_\n#define _FCNS_H_\n\n");
  fprintf(out,"#include <sys/types.h>\n");
  fprintf(out,"#include <structs.h>\n#include <hash.h>\n");
  fprintf(out,"#include <misc_proto.h>\n");

  for (i=1; i<argc; i++)
    strip_file(argv[i],out);

  fprintf(out,"\n#endif\n");
  fclose(out);
  return (0);
}
