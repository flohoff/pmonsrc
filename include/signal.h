/* $Id: signal.h,v 1.2 1996/01/16 14:17:26 chris Exp $ */
#ifndef _SIGNAL_
#define _SIGNAL_

/* request types */
#define SIGINT 2

/* args & returns */
#define	SIG_ERR		(void (*)())-1
#define	SIG_DFL		(void (*)())0
#define	SIG_IGN		(void (*)())1
#define	SIG_HOLD	(void (*)())3

typedef	void (*sig_t) (int);
void	(*signal (int, void (*) (int))) (int);

#endif /* _SIGNAL_ */
