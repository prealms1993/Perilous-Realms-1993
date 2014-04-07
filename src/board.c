#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
  
#include <fcns.h>
#include <utils.h>
#include <comm.h>
#include <db.h>
#include <externs.h>
  
#define MAX_MSGS 100	            /* Max number of messages.          */
#define MAX_MESSAGE_LENGTH 2048     /* that should be enough            */
  
struct Board {
    char *msgs[MAX_MSGS];
    char *head[MAX_MSGS];
    char new_header[80];
    char *new_message;
    int in_use;
    int msg_num;
    int vnum;    /* Real # of object that this board hooks to */
    struct Board *next;
};

struct Board *board_list;

extern struct obj_data *object_list;

void InitBoards()
{
  board_list = 0;
}

static char *BoardFileName(int virtual)
{
  static char buf[255];

  sprintf(buf, "%d.board", virtual);
  return(buf);
}

static void load_board(struct Board *b)
{
  FILE *f;
  int len,i;

  f=fopen(BoardFileName(b->vnum),"rb");
  if (!f) {
    nlog("%s does not exist.",BoardFileName(b->vnum));
    return;
  }
  if (!fread(&len,sizeof(int),1,f)) {
    fclose(f);
    b->msg_num=0;
    return;
  }
  //2014. Handle old Boards that are big-endian
  if (len > 0x00010000) {	//detect big endian here.  should detect any old style with a msg count of 0-65535
	  len = __builtin_bswap32(len);
	  nlog("%s byteswapping load. len:%d", BoardFileName(b->vnum), len);
	  b->msg_num = len;
	  for (i = 0; i<b->msg_num; i++) {
		  fread(&len, sizeof(len), 1, f);
		  len = __builtin_bswap32(len);
		  CREATE(b->head[i], char, len + 1);
		  fread(b->head[i], sizeof(char), len + 1, f);
		  fread(&len, sizeof(len), 1, f);
		  len = __builtin_bswap32(len);
		  CREATE(b->msgs[i], char, len + 1);
		  fread(b->msgs[i], sizeof(char), len + 1, f);
	  }

  }
  else {
	  b->msg_num = len;
	  for (i = 0; i<b->msg_num; i++) {
		  fread(&len, sizeof(len), 1, f);
		  CREATE(b->head[i], char, len + 1);
		  fread(b->head[i], sizeof(char), len + 1, f);
		  fread(&len, sizeof(len), 1, f);
		  CREATE(b->msgs[i], char, len + 1);
		  fread(b->msgs[i], sizeof(char), len + 1, f);
	  }

  }
  fclose(f);
}

static void save_board(struct Board *b)
{
  FILE *f;
  int len;
  int i;

  f=fopen(BoardFileName(b->vnum),"w+b");
  if (!f) {
    vlog(LOG_URGENT,"Can't write to %s",BoardFileName(b->vnum));
    return;
  }
  fwrite(&b->msg_num, sizeof(int), 1, f);
  for (i=0; i<b->msg_num; i++) {
    len=strlen(b->head[i]);
    fwrite(&len,sizeof(len),1,f);
    fwrite(b->head[i],sizeof(char),len+1,f);
    len=strlen(b->msgs[i]);
    fwrite(&len,sizeof(len),1,f);
    fwrite(b->msgs[i],sizeof(char),len+1,f);
  }
  fclose(f);
}

static int show_board(struct char_data *ch, char *arg, struct Board *b)
{
  int i;
  char tmp[MAX_INPUT_LENGTH];
  struct string_block sb;
  
  one_argument(arg, tmp);
  
  if (!*tmp || !isname(tmp, "board bulletin"))
    return(0);
  
  act("$n studies the board.", TRUE, ch, 0, 0, TO_ROOM);

  init_string_block(&sb);
  sb_printf(&sb,
  "This is a bulletin board. Usage: READ/REMOVE <messg #>, WRITE <header>\n");
  if (!b->msg_num) {
    sb_printf(&sb,"The board is empty.\n");
  } else {
    if (b->msg_num==1)
      sb_printf(&sb,"There is 1 message on the board.\n");
    else
      sb_printf(&sb,"There are %d messages on the board.\n", b->msg_num);
    for (i = 0; i < b->msg_num; i++)
      sb_printf(&sb,"%-2d : %s\n", i + 1, b->head[i]);
  }
  
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
  
  return(1);
}
void InitABoard(struct obj_data *obj)
{
  struct Board *new, *tmp;
  
  
  if (!real_objp(obj->virtual)) {
    vlog(LOG_URGENT, "Board %d does not exist in database.",obj->virtual);
    return;
  }
  if (board_list) {
/* try to match a board with an existing board in the game */
    for (tmp = board_list; tmp; tmp = tmp->next) {
      if (tmp->vnum == obj->virtual) {
/* board has been matched, load and ignore it.  */
	return;
      }
    }
  }
  
  new = MALLOC(sizeof(struct Board),struct Board);
  if (!new) {
    perror("InitABoard(malloc)");
    exit(0);
  }
  
  bzero(new, sizeof(struct Board));
  
  new->vnum = obj->virtual;
  real_objp(new->vnum)->func = board;
  
  load_board(new);
  
  /*
   **  add our new board to the beginning of the list
   */
  
  tmp = board_list;
  new->next = tmp;
  board_list = new;
}

struct Board *FindBoardInRoom(int room)
{
  struct obj_data *o;
  struct Board *nb;
  
  if (!real_roomp(room)) return(NULL);
  
  for (o = real_roomp(room)->contents; o ; o = o->next_content) {
    if (!real_objp(o->virtual)) continue;
    if (real_objp(o->virtual)->func == board) {
      for (nb = board_list; nb; nb = nb->next) {
	if (nb->vnum == o->virtual)
	  return(nb);
      }
      return(NULL);
    }
  }
  return(NULL);
}

int board(struct char_data *ch, int cmd, char *arg)
{
  struct Board *nb;
  
  nb = FindBoardInRoom(ch->in_room);
  
  if (!nb) return(FALSE);
  
  if (!ch->desc)
    return(FALSE); 
  
  switch (cmd) {
  case 15:  /* look */
    return(show_board(ch, arg, nb));
  case 149: /* write */
    write_msg(ch, arg, nb);
    return 1;
  case 63: /* read */
    return(board_display_msg(ch, arg, nb));
  case 66: /* remove */
    return(board_remove_msg(ch, arg,nb));
  default:
    return 0;
  }
}

void post_message(struct char_data *ch)
{
  struct Board *b;
  int i;

  for (b=board_list; b; b=b->next)
    if (b->vnum==ch->act_ptr) break;
  if (!b) {
    vlog(LOG_URGENT,"Serious problem: %d board",ch->act_ptr);
    ch->act_ptr=0;
    return;
  }
  ch->act_ptr=0;
  b->in_use=0;
  if (!b->new_message || !strlen(b->new_message)) return;
  for (i=++b->msg_num; i>0; i--) {
    b->head[i]=b->head[i-1];
    b->msgs[i]=b->msgs[i-1];
  }
  b->head[0]=mystrdup(b->new_header);
  b->msgs[0]=b->new_message;
  save_board(b);
  act("$n finishes writing a message.", TRUE, ch, 0, 0, TO_ROOM);
}

void write_msg(struct char_data *ch, char *arg, struct Board *b) 
{
  char *times;
  long ct;
  char buf[MAX_INPUT_LENGTH+50];
  
  if (b->msg_num > MAX_MSGS - 1) {
    send_to_char("The board is full already.\n", ch);
    return;
  }
  
  if (b->in_use) {
    sendf(ch,"I'm sorry, but someone else is currently writing.\n");
    return;
  }
  
  /* skip blanks */
  
  for(; isspace(*arg); arg++);
  
  if (!*arg) {
    send_to_char("We must have a headline!\n", ch);
    return;
  }
  
  ct=time(0);
  times = asctime(localtime(&ct));
  *(times + strlen(times) - 9) = '\0';
  sprintf(buf, "[%s] %s (%s)", times,arg,GET_NAME(ch));
  if (strlen(buf)>79) {
    sendf(ch,"Try using a shorter headline.\n");
    return;
  }
  
  strcpy(b->new_header,buf);
  
  sendf(ch,"Writing message on board.\n");
  sendf(ch,"Write your message. Terminate with an @.\n\n");
  act("$n starts to write a message.", TRUE, ch, 0, 0, TO_ROOM);
  
  b->new_message=NULL;
  ch->desc->str = &b->new_message;
  ch->desc->max_str = MAX_MESSAGE_LENGTH;
  b->in_use = 1;
  ch->funct=post_message;
  ch->act_ptr=b->vnum;
}


int board_remove_msg(struct char_data *ch, char *arg, struct Board *b) 
{
  int ind, msg;
  char buf[256], number[MAX_INPUT_LENGTH];
  
  one_argument(arg, number);
  
  if (!*number || !isdigit(*number))
    return(0);
  if (!(msg = atoi(number))) return(0);
  if (strchr(number,'.')) return(0);
  if (!b->msg_num) {
    send_to_char("The board is empty!\n", ch);
    return(1);
  }
  if (msg < 1 || msg > b->msg_num) {
    sendf(ch,"That message exists only in your imagination..\n");
    return(1);
  }
  
  if (GetMaxLevel(ch) < 500) {
    sendf(ch,"Due to misuse of the REMOVE command, only 500th level\n");
    sendf(ch,"and above can remove messages.\n");
    return(1);
  }
  
  ind = msg-1;
  FREE(b->head[ind]);
  FREE(b->msgs[ind]);
  for (; ind < b->msg_num -1; ind++) {
    b->head[ind] = b->head[ind + 1];
    b->msgs[ind] = b->msgs[ind + 1];
  }
  b->msg_num--;
  send_to_char("Message removed.\n", ch);
  sprintf(buf, "$n just removed message %d.", msg);
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  save_board(b);
  
  return(1);
}

int board_display_msg(struct char_data *ch, char *arg, struct Board *b) 
{
  char buf[512], number[MAX_INPUT_LENGTH], buffer[MAX_STRING_LENGTH];
  int msg;
  
  one_argument(arg, number);
  if (!*number || !isdigit(*number))
    return(0);
  if (strchr(number,'.')) return(0);
  if (!(msg = atoi(number))) return(0);
  if (!b->msg_num) {
    send_to_char("The board is empty!\n", ch);
    return(1);
  }
  if (msg < 1 || msg > b->msg_num) {
    sendf(ch,"That message exists only in your imagination..\n");
    return(1);
  }
  
  sprintf(buf, "$n reads message %d titled : %s.", msg, b->head[msg - 1]);
  act(buf, TRUE, ch, 0, 0, TO_ROOM); 
  
  sprintf(buffer, "Message %d : %s\n\n%s", msg, b->head[msg - 1],
	  b->msgs[msg - 1]);
  page_string(ch->desc, buffer, 1);
  return(1);
}

