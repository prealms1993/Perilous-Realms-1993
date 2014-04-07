#ifndef _MISC_PROTO_
#define _MISC_PROTO_

#ifndef _MISC_PROTO1_
void schedule(int in_ticks,void (*func)(),int type,...);
#endif

void sendf(struct char_data *ch, char *fmt,...);
void vlog(int level,char *fmt,...);
void nlog(char *fmt,...);
void llog(int level, char *fmt, ...);
void sports_cast(char *fmt,...);
void sb_printf(struct string_block *sb,char *fmt, ...);

char *mystrdup(char *s);
time_t time(time_t *tloc);
/*char *crypt(char *key, char *salt);*/

/*void getitimer(void *,void *);*/
int getdtablesize(void);
//void setdtablesize(int size);
//int select(int,void *, void *, void *,struct timeval *);
//void gethostname(char *name, int namelen);
int socket(int af,int type, int protocol);
/*int setsockopt(int,int,int,char *,int);
int bind(int s,void *name,int namelen);
int listen(int s, int backlog);
int getsockname(int,void *,int *);
int accept(int,void *,int *);*/

typedef int pid_t;
pid_t getppid();
int kill();
pid_t getpid();

#endif
