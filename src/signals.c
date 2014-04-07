/* ************************************************************************
*  file: signals.c , trapping of signals from Unix.       Part of DIKUMUD *
*  Usage : Signal Trapping.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */
//#define __USE_BSD

#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>

#include <fcns.h>
#include <utils.h>
#include <externs.h>
#include <ticks.h>

#ifndef BSD
#define SIG_RET int
#define SIG_RET_VAL (0)
#else
#define SIG_RET void
#define SIG_RET_VAL
#endif


void signal_fatal(int sig)
{
  signal(sig,SIG_DFL);
  nlog("A fatal signal (%d) was caught.", sig);
  if(crash_save){
    close_sockets(0);
    nlog("All sockets were closed.");
    kill(getpid(),SIGABRT);
  }
  /*return SIG_RET_VAL;*/
}


#ifdef DEADLOCK
static SIG_RET checkpointing()
{
	extern int tics;
	
	if (!(TICS))
	{
		log("CHECKPOINT shutdown: tics not updated");
		abort();
	}
	else
		TICS = 0;
  return SIG_RET_VAL;
}

#endif


void wakeup_request()
{
  extern int errno;

  signal(SIGUSR2, wakeup_request);
  signal(SIGALRM, wakeup_request);
  log("Received wakeup request");
  errno=EINTR;
  /*return SIG_RET_VAL;*/
}



/* kick out players etc */
void hupsig()
{
  log("Received SIGHUP, SIGINT, or SIGTERM. Shutting down");
  exit(0);   /* something more elegant should perhaps be substituted */
}



void logsig(int sig)
{
  signal(sig,logsig);
  nlog("Signal %d recieved and ignored.",sig);
  /*return SIG_RET_VAL;*/
}

void signal_setup()
{
#ifdef DEADLOCK
	struct itimerval itime;
	struct timeval interval;
#endif


	signal(SIGUSR2, wakeup_request);
	signal(SIGALRM, wakeup_request);

	/* just to be on the safe side: */

	signal(SIGHUP, hupsig);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, hupsig);
	signal(SIGTERM, hupsig);

	if (crash_save) {
	  signal(SIGABRT, signal_fatal);
	  signal(SIGFPE,signal_fatal);
	  signal(SIGBUS,signal_fatal);
	  signal(SIGSEGV,signal_fatal);
#ifdef SIGPWR
	  signal(SIGPWR,signal_fatal);
#endif
	  /*signal(SIGSYS,signal_fatal);*/
	}


	/* set up the deadlock-protection */

#ifdef DEADLOCK
	interval.tv_sec = 900;    /* 15 minutes */
	interval.tv_usec = 0;
	itime.it_interval = interval;
	itime.it_value = interval;
	setitimer(ITIMER_VIRTUAL, &itime, 0);
	signal(SIGVTALRM, checkpointing);
#endif
}

