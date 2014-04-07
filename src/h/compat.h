#ifndef _COMPATH_
#define _COMPATH_
#ifndef BSD
/*#define srandom(seed) srand(seed)
#define random() rand()*/
#define bcopy(b1,b2,l) memcpy(b2,b1,l)
#define bzero(s,l) memset(s,0,l)
#define index(s,c) strchr(s,c)
//#define stricmp(s1,s2) str_cmp(s1,s2)
#else
#endif
#endif
