/* ************************************************************************
*  file: Interpreter.h , Command interpreter module.      Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
************************************************************************* */

#ifndef _INTERPRETERH_
#define _INTERPRETERH_


typedef struct command_node {
  int cmd;
  struct command_node *left,*right;
} command_node;

#define CMDF_DISABLED  1       /* command is disabled */
#define CMDF_NODISABLE 2       /* command can not be disabled */
#define CMDF_FLUSH     4       /* command flushes lower priority commands */

struct command_info
{
  char *cmd;
  byte minimum_position;
  int minimum_level;
  void (*command_pointer) ();
  int priority;
  int flags;
};

#define MENU         \
"\n\rWelcome to Perilous Realms\n\r\n\
[0] Leave the Realms for now\n\r\
[1] Journey forth into the Realms\n\r\
[2] Tell us what you look like\n\r\
[3] Read the background story\n\r\
[4] Change your password\n\r\
[5] Journey to the abyss (delete your character)\n\r\
[6] Enter regeneration mode\n\r\
[7] Enter your finger information\n\r\n\r\
   Make your choice: "

#define ALIGN_HELP \
"\n\rAlign will determine how much exp you will lose each death. For being one\n\r\
alignment away from your choosen alignment (neutral when you choose good as\n\r\
your starting align lets say) you would lose up to 2.5 million exp, if your\n\r\
2 away (evil when you choose good as example) you can lose up to 5 million\n\r\
exp. For being in your choosen aligment you can lose a max of 1 million exp.\n\r\
These are max values obviously you wont lose 1 million exp at lvl 10 or even\n\r\
20.\n\r\
Other things alignment will affect soon is god worship. You will be allowed\n\r\
choose a god to worship after about level 10, you will get extra bonuses by\n\r\
doing this, things like extra healing or advanced stats, this will only be\n\r\
effective while your in your correct alignment.\n\r"

#define RACEHELP          \
"\n\r Allowed Races and ability adjustments.  Also classes race can be:\n\r\
            str int wis dex con cha lck\n\r\
human         0   0   0   0   0   0   0   can be ANY no multi\n\r\
elf           0   0   0  +1  -1   0   0   can be A,C,W,M,T,R W/M W/T W/M/T M/T\n\r\
dwarf         0  -1   0   0  +1  -1   0   can be W,T,C,A W/C W/T\n\r\
half-elf      0   0   0   0   0   0   0   can be C,D,W,R,M,I,T,P,A,Av,D D/W\n\r                                                 W/M W/T M/T C/M D/M W/M/T D/R\n\r\
half orc     +1  -1   0   0   0  -2   0   can be C,W,T,A  W/T\n\r\
pixie        -2  +2  +2  +1  -2   -  +1   can be M,C,I,D\n\r\
lizardman     0  +1   0  +2  -1  -2   0   can be W,B,T,A\n\r\
troll        +1  -1   0  -1  +1  -1   0   can be W,B,T W/C\n\r\
ogre         +1  -1   0   0  +1  -1   0   can be W W/M\n\r\
giant        +2  -2  -2  -1  +2   0  +1   can be W,B\n\r\
halfling     -1   0   0  +1   0   0   0   can be D,C,W,T  W/T\n\r\
gnome         0  +1  -1   0   0  -1   0   can be W,C,T,I W/T I/T\n\r\
\n\r"

#define CLASS_HELP \
"\n\rClass Help goes here\
\n\r"

#define OGREETINGS \
"\n\r           Tom Madsen, Michael Seifert, and Sebastian Hammer\n\r\
                  Hans Henrik Staerfeldt, Katja Nyboe,\n\r\
                              Created by\n\r\n\r\
                           DikuMUD I (GAMMA 0.0)\n\r\n\r"


#define WELC_MESSG \
"\n\rWelcome to the Perilous Realms. May your visit here be...Safe.\n\r\n\r"

#define STORY     \
"\n\r		In the beginning. . . the universe was filled\n\r\
            by hordes of adventurers seeking their fortunes in\n\r\
            the land of Albanian Diku.  Many great champions\n\r\
            strove for mastery within the harsh electronic bounds\n\r\
            of Judy.  For awhile, the gods smiled on the happy\n\r\
            throng, benignly aiding their flock to new sources of\n\r\
            glory and power.\n\r\
		But, there came a time when the gods looked down\n\r\
            upon the masses of electronic beings who had begun to\n\r\
            chafe at their bonds, for they had realized that there\n\r\
            must be more to life than was available within this\n\r\
            land. The gods were not pleased.  And from his high\n\r\
            place, their leader struck at the land, closing off\n\r\
            those who journeyed there from reaping it's rewards.\n\r\
            And in this cataclysm, the land was shattered into\n\r\
            many parts spread throughout this land that we all\n\r\
            share. each part known only to some of those who once\n\r\
            journeyed together the paths of electronic worship.\n\r\
		But one of these realms was different.  Its new\n\r\
            rulers strive to surpass all other worlds, to reclaim\n\r\
            the masses in a place where the bounds are virtually\n\r\
            unlimited.  And in this new realm, the gods created new\n\r\
            wonders and new perils. Soon they shall call across the\n\r\
            electronic worlds seeking a new following. Till then,\n\r\
            the world remains much a mystery, known to only a few\n\r\
            mortals, and even to them only in small part.\n\r\n\r\
\
            Welcome then to the Perilous Realms, may you thrive in\n\r\
            the new world, and help to bring it to glory.\n\r"


#define OLDGREETINGS \
"                                  ,__.. \n\r\
             ________  Welcome   / /| :   Perilous      ,_____ \n\r\
              =|  _. \\    to...  \\_|| |    Realms      / ,____] \n\r\
              -|  | \\ \\_____,___.___| | .___. ___   __: /____. \n\r\
              -|  |./ /____/ ___ \\ || |/ .-. \\| |   | |\\___.  \\ \n\r\
               |  ||_/| __/| |  \\/ || | /   \\ \\ |   | |_.  |   | \n\r\
               |  | \\ \\____| |   | || | \\___/ / \\__/, | \\__/  /  \n\r\
              [___]  \\____/|_|  [___]__].___./ \\.__/|__]_____/  \n\r\
                ____._.               ___.            ,_____  \n\r\
                 |  || \\               | |           /  ,___] \n\r\
                -|  | \\ \\_____. ,------| |__ ___ ___/  /____. \n\r\
                -|  | / / ____// ,--! || || /,_.v,_.\\\\___.  \\ \n\r\
                 |  |/ / | __// /   | || || /  | | | |   |   | \n\r\
                 |  |\\ \\ \\____| \\___| || || |  | | | |___/  / \n\r\
                [___] \\__]___/ \\___/|__]__]__] |/  |/______/ \n\r\
===============================================================================\n\r\
                        Beginning Mods by J. Brothers\n\r\
                      Major Modifications by A. Caldwell\n\r\
               with help from C. Mohr, T. Cannon, and S. Adkins\n\r\
===============================================================================\n\r\
                        Based on DikuMUD I Created by\n\r\
        T. Madsen, M. Seifert, S. Hammer H. H. Staerfeldt and K. Nyboe\n\r\n\r"


#define GREETINGS "\n\r\
   /\\/\\/\\                \n\r\
  / /\\ \\/   Welcome to...\n\r\
 / / /\\ \\   _____        \n\r\
 \\ \\/ /\\ \\  | __ \\                      \n\r\
  \\/ /  \\ \\ | |/ / E R I L O U S               Based on DikuMUD I \n\r\
  / /\\  / / | __/\\                                 created by\n\r\
 / /\\ \\/ /  | | __\\                   \n\r\
/ /  \\ \\/   |_| | / E A L M S           T. Madsen, M. Seifert, S. Hammer,\n\r\
\\ \\  /\\ \\     | | \\                         H.H. Straefeldt, & K. Nyboe\n\r\
 \\ \\/ /\\ \\    |_|\\_\\\n\r\
  \\/ /  \\ \\\n\r\
  / /\\  / / Beginning Mods by J. Brothers            \n\r\
 / /\\ \\/ /  Major Modifications by A. Caldwell              ,---.\n\r\
/ /  \\ \\/   with help from C. Mohr, A. Cannon & S. Adkins  .\\ (, )\n\r\
\\ \\  /\\ \\  ____  ____  ____  ____  ____                  .-| \\ _/|\\  \n\r\
 \\ \\/ /\\ \\/ __ \\/ __ \\/ __ \\/ __ \\/ __ \\/\\                \\--./ _/ '\n\r\
  \\/ /  \\/ /  \\/ /  \\/ /  \\/ /  \\/ /  \\ \\ \\  /\\    /\\      \\__\\/__/\n\r\
  / /\\  / /\\  / /\\  / /\\  / /\\  / /\\  /\\ \\ \\/  \\  /  \\       ''/'\n\r\
 / /\\ \\/ /\\ \\/ /\\ \\/ /\\ \\/ /\\ \\/ /\\ \\/ /\\ \\/ /\\ \\/ /\\/   ___   |\n\r\
 \\ \\ \\ \\/  \\ \\/  \\ \\/  \\ \\/  \\ \\/  \\ \\/  \\ \\/\\/ / /      \\ -\\  /_\n\r\
  \\ \\/\\ \\__/\\ \\__/\\ \\__/\\ \\__/\\ \\__/\\ \\__/\\ \\__/ /    ___ \\__\\/\\_\\      \n\r\
   \\__/\\____/\\____/\\____/\\____/\\____/\\____/\\____/        \\___/        \n\r "
#endif
