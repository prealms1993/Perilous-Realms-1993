#include <stdio.h>
#include <string.h>

#include <fcns.h>
#include <externs.h>
#include <utils.h>
#include <comm.h>
#include <interpreter.h>
#include <handler.h>
#include <spells.h>
#include <limits.h>
#include <race.h>


int BitCount(unsigned long n)
{
  int i,tot;

  for (tot=0,i=0; i<32; i++) if (n&(1<<i)) tot++;
  return(tot);
}
