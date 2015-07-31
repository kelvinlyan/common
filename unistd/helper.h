#ifndef _UNISTD_HELPER_H
#define _UNISTD_HELPER_H

#include <unistd.h>
#include <sys/signal.h>
#include <stdlib.h>

namespace nUnistd
{
	typedef void(*sig_handler)(int);

	sig_handler signal(int signo, sig_handler handler = NULL, sigset_t* mask = NULL)
	{
		if(handler == NULL)
		{
			struct sigaction oact;
			sigaction(signo, NULL, &oact);
			return oact.sa_handler;
		}

		struct sigaction act, oact;
		act.sa_handler = handler;
		if(mask != NULL)
			act.sa_mask = *mask;	
		else
			sigemptyset(&act.sa_mask);
		if(signo == SIGALRM)
		{
	#ifdef SA_INTERRUPT
			//printf("SET INTERRUPT\n");
			act.sa_flags |= SA_INTERRUPT;
	#endif
		}
		else
		{
	#ifdef SA_RESTART
			//printf("SET RESTART\n");
			act.sa_flags |= SA_RESTART;
	#endif
		}
		sigaction(signo, &act, &oact);
		return (oact.sa_handler);
	}
}

#endif
