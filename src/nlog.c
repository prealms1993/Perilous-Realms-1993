#include <stdio.h>
//#include <varargs.h>
#include <stdarg.h>
#include <time.h>

/* don't include special proto's */
#define _MISC_PROTO_
#include <fcns.h>
#include <externs.h>
#include <utils.h>


void sendf(struct char_data * ch, char * fmt, ...) {
  char s[MAX_STRING_LENGTH*4];
  va_list args;

  va_start(args, fmt);
  vsprintf(s,fmt,args);
  va_end(args);
  send_to_char(s,ch);
}

//void nlog(va_alist) va_dcl {
void nlog(char* fmt, ...) {
  va_list args;
  char s[MAX_STRING_LENGTH];

  va_start(args, fmt);
  vsprintf(s, fmt, args);
  va_end(args);
  log(s);
}

static char *log_chars[] = {
  "[**]", /* 0 - Connection */
  "<**>", /* 1 - Mortal Spy */
  ">**<", /* 2 - Immortal Spy */
  "!!!!", /* 3 - Urgent */
  "?  ?", /* 4 - Debug */
  "]**[", /* 5 - reject */
  "|--|", /* 6 - misc */
  "#**#", /* 7 - Death */
  "/**/", /* 8 */
  "/**/", /* 9 */
  "/**/", /* 10 */
  "/**/", /* 11*/
  "/**/", /* 12 */
  "/**/", /* 13 */
  "/**/", /* 14 */
  "/**/", /* 15 */
  "/**/", /* 16 */
  "/**/", /* 17 */
  "/**/", /* 18 */
  "/**/", /* 19 */
  "/**/", /* 20 */
  "/**/", /* 21 */
  "/**/", /* 22 */
  "/**/", /* 23 */
  "/**/", /* 24 */
  "/**/", /* 25 */
  "/**/", /* 26 */
  "/**/", /* 27 */
  "/**/", /* 28 */
  "/**/", /* 29 */
  "/**/", /* 30 */
  "/**/", /* 31 */
  "/**/"
};

void vlog(u32 type, char* fmt, ...){
  va_list args;
  char s[MAX_STRING_LENGTH];
  long ct;
  int bit;
  char *tmstr, buf[MAX_STRING_LENGTH];
  struct connection_data *i;


  va_start(args, fmt);
  vsprintf(s, fmt, args);
  va_end(args);

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  fprintf(stderr, "%s :: %s\n", tmstr, s);

  if (!type) type=LOG_URGENT;
  for (bit=0; ((1<<bit)&type)==0; bit++);

  sprintf(buf,"%c%c %s %c%c\n",
	log_chars[bit][0],log_chars[bit][1],
	s,
	log_chars[bit][2],log_chars[bit][3]);

  for (i = connection_list; i; i = i->next)
    if (IS_CONN(i) && LOG(i->character,type) &&
	!IS_SET(i->character->specials.act,PLR_VEG))
      write_to_q(buf, &i->output,0);
}


void sports_cast(char * fmt, ...) {
  va_list args;
  char s[MAX_STRING_LENGTH];
  struct connection_data *i;

  va_start(args, fmt);
  vsprintf(s, fmt, args);
  va_end(args);
  for (i = connection_list; i ; i = i->next)
    if(!i->connected && !IS_SET(i->character->specials.act, PLR_NOSPORTS))
      sendf(i->character,"SPORTS NEWS: %s\n",s);
}

void logl(int level,char *dl,char *str)
{
  long ct;
  char *tmstr, buf[MAX_STRING_LENGTH];
  struct connection_data *i;


  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  fprintf(stderr, "%s :: %s\n", tmstr, str);


  if (str)
     sprintf(buf,"%c* %s *%c\n",dl[0],str,dl[1]);
  for (i = connection_list; i; i = i->next)
    if (IS_CONN(i) && GetMaxLevel(i->character)>=level &&
	!IS_SET(i->character->specials.act,PLR_VEG) &&
	!IS_SET(i->character->specials.act, PLR_NOVIEWLOG))
      write_to_q(buf, &i->output,0);
}

void llog(int level, char* fmt, ...) {
  static char dl[]="?\?//223344556677{}[]";
  va_list args;
  char s[MAX_STRING_LENGTH];

  va_start(args, fmt);
  vsprintf(s,fmt,args);
  va_end(args);
  if (level<MAX_MORT) log(s);
  else if (level>IMPLEMENTOR) logl(IMPLEMENTOR,"!!",s);
  else logl(level,(char *)(dl+2*(level-LOW_IMMORTAL)),s);
}

void sb_printf(struct string_block * sb, char * fmt, ...) {
  va_list args;
  char s[MAX_STRING_LENGTH*4];

  va_start(args, fmt);
  vsprintf(s,fmt,args);
  va_end(args);
  append_to_string_block(sb,s);
}
static FILE *player_log;
void open_player_log()
{
  player_log=fopen("PLAYER.LOG","a+");
  if (!player_log) {
    nlog("Error, could not open PLAYER.LOG");
    exit(1);
  }
}

void log_action(struct char_data *ch, char *arg)
{
  char prefix[5];

  if (IS_MOB(ch)) return;
  if (GetMaxLevel(ch)>=IMMORTAL)
    strcpy(prefix,"IMM:");
  else
    strcpy(prefix,"PLR:");
  fprintf(player_log,"%s%s:%s\n",prefix,GET_NAME(ch),arg);
  if (IS_SET(ch->specials.act,PLR_LOG) && strlen(arg)>2) {
    if (GET_LEVEL(ch)>=IMMORTAL)
      vlog(LOG_IMMSPY,"%s:%s",GET_NAME(ch),arg);
    else
      vlog(LOG_MORTSPY,"%s:%s",GET_NAME(ch),arg);
  }
  fflush(player_log);
}

void close_player_log()
{
  fclose(player_log);
}
