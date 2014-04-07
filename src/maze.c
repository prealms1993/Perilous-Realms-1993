#include <stdio.h>

#include <fcns.h>
#include <utils.h>

#define MAZE_NORTH 1
#define MAZE_EAST  2
#define MAZE_SOUTH 4
#define MAZE_WEST  8

typedef unsigned char cell_type;

typedef struct node {
  int cell_id;
  struct node *next;
} node;

static node *used_stack;
static node *free_stack=NULL;

static cell_type *maze;
static int max_cell;
static int maxh,maxv;

#define num(lo,hi) ((random() % (hi-lo+1)) + lo)

static void push(int id)
{
  node *new;

  if (free_stack) {
    new=free_stack;
    free_stack=free_stack->next;
  } else {
    new=(node *)malloc(sizeof(node));
  }
  new->next=used_stack;
  used_stack=new;
  new->cell_id=id;
}

static int pop(void)
{
  node *n;

  n=used_stack;
  used_stack=used_stack->next;
  n->next=free_stack;
  return(n->cell_id);
}

static cell_type legal_dirs(int pos)
{
  cell_type legal=0;

  if (pos>0 && !maze[pos-1]) legal |= MAZE_WEST;
  if (pos+1<max_cell && !maze[pos+1]) legal |= MAZE_EAST;
  if (pos+maxh+1<max_cell && !maze[pos+maxh]) legal |=MAZE_SOUTH;
  if (pos-maxh>0 && !maze[pos-maxh]) legal |= MAZE_NORTH;
  return(legal);
}

void create_maze(int room,int h,int v)
{
  cell_type dirs;
  cell_type move;
  int pos;

  max_cell=h*v;
  maze = (cell_type *)malloc(max_cell*sizeof(cell_type));
  if (!maze) {
    fprintf(stderr,"Insufficent memory.\n");
    exit(1);
  }
  maxh=h; maxv=v;
  memset(maze,0,max_cell*sizeof(cell_type));


  pos = num(0,max_cell-1);

  while (1) {
    push(pos);
    do {
      dirs=legal_dirs(pos);
      if (!dirs && used_stack)
        pos=pop();
    } while (used_stack && !dirs);
    if (!dirs && !used_stack) break;
    do {
      move=(1<<num(0,3));
    } while (!(move & dirs));
    maze[pos] |= move;

    if (move == MAZE_WEST) { pos--; maze[pos] |= MAZE_EAST; }
    else if (move == MAZE_EAST) { pos++; maze[pos] |= MAZE_WEST; }
    else if (move == MAZE_NORTH) { pos-=maxh; maze[pos] |= MAZE_SOUTH; }
    else if (move == MAZE_SOUTH) { pos+=maxh; maze[pos] |= MAZE_NORTH; }
  }

  while (free_stack) {
    used_stack=free_stack;
    free_stack=free_stack->next;
    FREE(used_stack);
  }
  free_stack=NULL;
  used_stack=NULL;

  for (pos=0; pos<max_cell; pos++) {
    struct room_data *rp;
    struct char_data *ch;
    char dir[10];
    char buf[MAX_STRING_LENGTH];
    struct room_direction_data *d;
    int i;

    rp=real_roomp(pos+room);
    strcpy(buf,"");
    for (i=0; i<4; i++) {
      if (maze[pos] & (1<<i)) {
	CREATE(d,struct room_direction_data,1);
	memset(d,0,sizeof(struct room_direction_data));
	rp->dir_option[i]=d;
	d->exit_info=0;
	d->keyword=NULL;
	d->key=-1;
	switch (1<<i) {
	  case MAZE_NORTH: d->to_room=room+pos-maxh;
		strcpy(dir,"north");
		break;
	  case MAZE_SOUTH: d->to_room=room+pos+maxh;
		strcpy(dir,"south");
		break;
	  case MAZE_WEST: d->to_room=room+pos-1;
		strcpy(dir,"west");
		break;
	  case MAZE_EAST: d->to_room=room+pos+1;
		strcpy(dir,"east");
		break;
	}
	if (rp->people)
	  sprintf(buf+strlen(buf),
		"A building daemon knocks a hole in the %s wall.\n",dir);
      }
    }
    for (ch=rp->people; ch; ch=ch->next_in_room)
	sendf(ch,buf);
  }
  FREE(maze);
}
