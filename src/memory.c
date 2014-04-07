#include <stdio.h>
#include <malloc.h>
#define MAX_MALLOC 10000

char *ptrs[MAX_MALLOC];
extern int DEBUG;

char *mallocx(int size,char *file)
{
  char *p;
  int i;

  p=malloc(size);
  if (!p) return (p);
  if (DEBUG>10) {
    i=0;
    while (ptrs[i]) i++;
    assert(i<MAX_MALLOC);
    ptrs[i]=p;
  }
  return(p);
}

void freex(void *p,char *file)
{
  int i;

  if (DEBUG>0) {
    assert(p);
    i=0;
    while (i<MAX_MALLOC && ptrs[i]!=(char *)p) i++;
    if (i<MAX_MALLOC)
      ptrs[i]=NULL;
  }
  free(p);
}

char *reallocx(void *old,int new_size,char *file)
{
  int i;
  if (!old)
    return(mallocx(new_size));
  if (DEBUG>0) {
    while (i<MAX_MALLOC && ptrs[i]!=(char *)old) i++;
    i=0;
    if (i<MAX_MALLOC) {
      ptrs[i]=realloc(old,new_size);
      return(ptrs[i]);
    }
  }
  return(realloc(old,new_size));
}

static int is_string(char *s)
{
  if (s[0]>=32 && s[0]<=127)
   return(1);
  return(0);
}

void malloc_dump(int fd)
{
  int i;
  for (i=0; i<MAX_MALLOC; i++) if (ptrs[i]) {
    if (is_string(ptrs[i]))
      fprintf(stderr,"%d:%s\n",i,ptrs[i]);
    else
	fprintf(stderr,"%d:NAS\n",i);
  }
}
