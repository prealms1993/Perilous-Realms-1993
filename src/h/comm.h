/* ************************************************************************
*  file: comm.h , Communication module.                   Part of DIKUMUD *
*  Usage: Prototypes for the common functions in comm.c                   *
************************************************************************* */

#ifndef _COMMH_
#define _COMMH_

#define TO_ROOM    0
#define TO_VICT    1
#define TO_NOTVICT 2
#define TO_CHAR    3

#define SEND_TO_Q(messg, desc)  write_to_q((messg), &(desc)->output,0)
#endif
