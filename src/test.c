#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>

#define CONN_NAME 0
#define CONN_PASS 1
#define CONN_CHAT 2

#define IBUF_SIZE 512

typedef struct conn_t {
   int           fd;
   int           state;
   char          name[80];
   char          title[80];
   char          ibuf[IBUF_SIZE];
   int           ipos;
   struct conn_t *next;
} conn_t;

conn_t *conn_list = NULL;

/* functions */
int  init_socket(int);
void new_conn(int);
void input_conn(conn_t *);
void look_conn(conn_t *);
void output_except(char *, conn_t *);
void output_conn(conn_t *, char *);
void close_conn(conn_t *);

int main(int argc, char *argv[], char *envp[])
{
   int    fds, s, quit = 0;
   fd_set input_set, exc_set;
   conn_t *conn, *next_conn;

   s = init_socket(2151);
   fds = getdtablesize();

   while(!quit){
      FD_ZERO(&input_set);
      FD_ZERO(&exc_set);
      FD_SET(s, &input_set);
      for(conn = conn_list; conn; conn = conn->next){
         FD_SET(conn->fd, &input_set);
         FD_SET(conn->fd, &exc_set);
      }
      if(select(fds, &input_set, NULL, &exc_set, NULL) == -1){
         perror("select");
         exit(1);
      }
      if(FD_ISSET(s, &input_set)){
         new_conn(s);
      }
      for(conn = conn_list; conn; conn = next_conn){
         next_conn = conn->next;
         if(FD_ISSET(conn->fd, &exc_set)){
            FD_CLR(conn->fd, &input_set);
            close_conn(conn);
         }
      }
      for(conn = conn_list; conn; conn = next_conn){
         next_conn = conn->next;
         if(FD_ISSET(conn->fd, &input_set)){
            input_conn(conn);
         }
      }
   }
}

int init_socket(int port)
{
   int s;
   int opt = 1;
   char hostname[512];
   struct sockaddr_in sa;
   struct hostent *hp;
   struct linger ld;

   memset(&sa, 0, sizeof(struct sockaddr_in));
   gethostname(hostname, 512);
   hp = gethostbyname(hostname);
   if (hp == NULL) {
      perror("gethostbyname");
      exit(1);
   }
   sa.sin_family = hp->h_addrtype;
   sa.sin_port = htons(port);
   s = socket(AF_INET, SOCK_STREAM, 0);
   if (s < 0) {
      perror("Init-socket");
      exit(1);
   }
   if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
	 (char *) &opt, sizeof(opt)) < 0) {
      perror("setsockopt REUSEADDR");
      exit(1);
   }

   ld.l_onoff = 0;
   ld.l_linger = 0;
   if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0) {
      perror("setsockopt LINGER");
      exit(1);
   }
   if (bind(s, &sa, sizeof(sa), 0) < 0) {
      perror("bind");
      close(s);
      exit(1);
   }
   listen(s, 3);
   return (s);
}

void new_conn(int s)
{
   struct sockaddr_in isa;
   conn_t *conn;
   int i;

   if(!(conn = (conn_t *)malloc(sizeof(conn_t)))){
      return;
   }

   i = sizeof(isa);
   getsockname(s, &isa, &i);

   if((conn->fd = accept(s, &isa, &i)) == -1){
      perror("accept");
      return;
   }
   if(fcntl(s, F_SETFL, O_NDELAY) == -1){
      perror("fcntl");
      return;
   }

   conn->state    = CONN_NAME;
   conn->ipos     = 0;
   conn->name[0]  = '\0';
   conn->title[0] = '\0';
   /* link it in */
   conn->next     = conn_list;
   conn_list      = conn;

   output_conn(conn, "Welcome to the Perilous Realms chat service, available for use while Perilous\nRealms looks for a new home.  To see who is on use \"look\" or \"who\".  To send\nsomeone a private message, use \"tell\".  To send a global message use \"say\".\nTo emote use \"emote\".  To change your title use \"title\".  To quit, use\n\"quit\".\n\n");
   output_conn(conn, "Name: ");
}

void input_conn(conn_t *conn)
{
   int  i, j, l;
   char buf[512], *name;
   conn_t *other;

   l = IBUF_SIZE - conn->ipos;
   if(!l){
      /* too much input, flush buffer */
      conn->ipos = 0;
      l = IBUF_SIZE;
   }
   if((i = read(conn->fd, &conn->ibuf[conn->ipos], l)) > 0){
      conn->ipos += i;
   }
   else{
      close_conn(conn);
      return;
   }
   for(i = 0; i < conn->ipos; i++){
      if((conn->ibuf[i] == '\n') || (conn->ibuf[i] == '')){
         conn->ibuf[i] = '\0';
         for(i++; ((conn->ibuf[i] == '\n') || (conn->ibuf[i] == '')) &&
            (i < conn->ipos); i++);
         switch(conn->state){
            case CONN_NAME:
               if(!*conn->ibuf){
                  output_conn(conn, "Illegal name.\n");
                  close_conn(conn);
                  return;
               }
               for(other = conn_list; other; other = other->next){
                  if(!strcasecmp(other->name, conn->ibuf)){
                     output_conn(conn, "That name is taken already.\n");
                     close_conn(conn);
                     return;
                  }
               }
               for(j = 0; conn->ibuf[j]; j++){
                  if(!isalpha(conn->ibuf[j])){
                     output_conn(conn, "Illegal name.\n");
                     close_conn(conn);
                     return;
                  }
               }
               strncpy(conn->name, conn->ibuf, 80);
               conn->name[0] = toupper(conn->name[0]);
               conn->name[79] = '\0';
               if(!strcasecmp(conn->name, "Deth") ||
                     !strcasecmp(conn->name, "Scarrow")){
                  conn->state = CONN_PASS;
                  output_conn(conn, "Password: ");
               }
               else{
                  conn->state = CONN_CHAT;
                  sprintf(buf, "\n%s has logged in.\n: ", conn->name);
                  output_except(buf, conn);
                  look_conn(conn);
                  output_conn(conn, ": ");
               }
               break;
            case CONN_PASS:
               if(!strcasecmp(conn->name, "Deth") &&
                     strcasecmp(conn->ibuf, "x11r4")){
                  output_conn(conn, "Password incorrect.\n");
                  close_conn(conn);
                  return;
               }
               else if(!strcasecmp(conn->name, "Scarrow") &&
                     strcasecmp(conn->ibuf, "felch")){
                  output_conn(conn, "Password incorrect.\n");
                  close_conn(conn);
                  return;
               }
               else{
                  conn->state = CONN_CHAT;
                  sprintf(buf, "\n%s has logged in.\n: ", conn->name);
                  output_except(buf, conn);
                  look_conn(conn);
                  output_conn(conn, ": ");
               }
               break;
            case CONN_CHAT:
               for(j = 0; conn->ibuf[j] && !isspace(conn->ibuf[j]); j++);
               if(isspace(conn->ibuf[j])){
                  conn->ibuf[j] = '\0';
                  for(j++; isspace(conn->ibuf[j]); j++);
               }
               if(!strcasecmp(conn->ibuf, "quit")){
                  close_conn(conn);
                  return;
               }
               else if(!strcasecmp(conn->ibuf, "look") ||
                     !strcasecmp(conn->ibuf, "who")){
                  look_conn(conn);
                  output_conn(conn, ": ");
               }
               else if(!strcasecmp(conn->ibuf, "say") ||
                     !strcasecmp(conn->ibuf, "'")){
                  if(conn->ibuf[j]){
                     sprintf(buf, "\n%s says, \"%s\"\n: ", conn->name,
                        &conn->ibuf[j]);
                     output_except(buf, conn);
                     sprintf(buf, "You say, \"%s\"\n: ", &conn->ibuf[j]);
                     output_conn(conn, buf);
                  }
                  else{
                     output_conn(conn, "Did you have anything in mind?\n: ");
                  }
               }
               else if(!strcasecmp(conn->ibuf, "emote") ||
                     !strcasecmp(conn->ibuf, ":")){
                  if(conn->ibuf[j]){
                     sprintf(buf, "\n%s %s\n: ", conn->name,
                        &conn->ibuf[j]);
                     output_except(buf, conn);
                     sprintf(buf, "You emote: %s %s\n: ", conn->name,
                        &conn->ibuf[j]);
                     output_conn(conn, buf);
                  }
                  else{
                     output_conn(conn, "Did you have anything in mind?\n: ");
                  }
               }
               else if(!strcasecmp(conn->ibuf, "title")){
                  strncpy(conn->title, &conn->ibuf[j], 80);
                  conn->title[79] = '\0';
                  sprintf(buf, "Title set to: %s\n: ", conn->title);
                  output_conn(conn, buf);
               }
               else if(!strcasecmp(conn->ibuf, "tell")){
                  if(!conn->ibuf[j]){
                     output_conn(conn, "Anyone in particular?\n: ");
                  }
                  else{
                     name = &conn->ibuf[j];
                     for(; conn->ibuf[j] && !isspace(conn->ibuf[j]); j++);
                     if(conn->ibuf[j]){
                        conn->ibuf[j] = '\0';
                        for(j++; isspace(conn->ibuf[j]); j++);
                     }
                     if(conn->ibuf[j]){
                        for(other = conn_list; other; other = other->next){
                           if(!strcasecmp(other->name, name)){
                              break;
                           }
                        }
                        if(other == conn){
                           output_conn(conn, "You talk to yourself.\n: ");
                        }
                        else if(other){
                           sprintf(buf, "\n%s tells you, \"%s\"\n: ",
                              conn->name, &conn->ibuf[j]);
                           output_conn(other, buf);
                           sprintf(buf, "You tell %s, \"%s\"\n: ",
                              other->name, &conn->ibuf[j]);
                           output_conn(conn, buf);
                        }
                        else{
                           output_conn(conn, "I don't see that person.\n: ");
                        }
                     }
                     else{
                        output_conn(conn, "Tell them what?\n: ");
                     }
                  }
               }
               else if(!*conn->ibuf){
                  output_conn(conn, ": ");
               }
               else{
                  output_conn(conn, "I didn't quite understand.\n: ");
               }
               break;
         }
         break;
      }
   }
   for(j = 0; i < conn->ipos; i++, j++){
      conn->ibuf[j] = conn->ibuf[i];
   }
   conn->ipos = j;
}

void look_conn(conn_t *conn)
{
   char   buf[512];
   conn_t *other;

   for(other = conn_list; other; other = other->next){
      if((other != conn) && (other->state == CONN_CHAT)){
         if(*other->title){
            sprintf(buf, "%s %s is here.\n", other->name, other->title);
         }
         else{
            sprintf(buf, "%s is here.\n", other->name);
         }
         output_conn(conn, buf);
      }
   }
}

void output_except(char *s, conn_t *conn)
{
   conn_t *other;

   for(other = conn_list; other; other = other->next){
      if((other != conn) && (other->state == CONN_CHAT)){
         output_conn(other, s);
      }
   }
}

void output_conn(conn_t *conn, char *s)
{
   write(conn->fd, s, strlen(s));
}

void close_conn(conn_t *conn)
{
   char   buf[512];
   conn_t *prev, *other;

   sprintf(buf, "\n%s has left.\n: ", conn->name);
   output_except(buf, conn);

   close(conn->fd);

   for(prev = NULL, other = conn_list; other; other = other->next){
      if(conn == other){
         break;
      }
      prev = other;
   }
   if(prev){
      prev->next = other->next;
   }
   else{
      conn_list = other->next;
   }
   free(other);
}

