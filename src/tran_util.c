#include <string.h>
#include "structs.h"
#include "utils.h"

int str_cmp(char *arg1, char *arg2)
{
  int i,chk;

  if ((!arg2) || (!arg1))
    return(1);

  for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
    if (chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))
      if (chk < 0)
        return (-1);
      else
        return (1);
  return(0);
}


int is_abbrev(char *arg1, char *arg2)
{
  if (!*arg1)
    return(0);

  for (; *arg1; arg1++, arg2++)
    if (LOWER(*arg1) != LOWER(*arg2))
      return(0);

  return(1);
}

int string_lookup(char *name,char *fields[])
{
  int i;
  i=0;
  while (*fields[i] && *fields[i]!='\n' && str_cmp(name,fields[i])) i++;
  if (str_cmp(name,fields[i])==0) return(i);
  /* try to match an abreviation */
  i=0;
  while (*fields[i] && *fields[i]!='\n' && !is_abbrev(name,fields[i])) i++;
  if (is_abbrev(name,fields[i])) return(i);
  return(-1);
}

int string_table(char *names,char *fields[], s16 *A)
{
  char temp[1000],*p;
  int count;

  count=0;

  while (*names) {
    for (;*names && isspace(*names);names++);
    p=temp;
    if (*names) {
      while (*names && *names!=',')
        *(p++)=*(names++);
      *p='\0';
      names++;
      A[count]=string_lookup(temp,fields);
      if (A[count++]<0) {
	printf("%s not recognized\n",temp);
	exit(1);
      }
    }
  }
  return(count);
}

void bitvector(u32 *v,char *names,char *list[])
{
  s16 A[1000];
  int count;
  int i;
  
  count=string_table(names,list,A);
  init_bits(v);
  for (i=0; i<count; i++)
    set_bit(v,A[i]);
}

unsigned long bitvector1(char *names,char *list[])
{
  s16 A[1000];
  int count;
  unsigned long v;
  int i;

  v=0;
  count=string_table(names,list,A);
  for (i=0; i<count; i++)
    v |= (1<<A[i]);
  return(v);
}



char *mystrdup(char *source) {
	char *new;

	if (source) {
	  CREATE(new, char, strlen(source)+1);
	  return(strcpy(new, source));
        }
	CREATE(new,char,1);
	*new='\0';
	return(new);
}
