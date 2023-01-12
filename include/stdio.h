/* $Id: stdio.h,v 1.3 1998/06/17 00:49:19 chris Exp $ */
#ifndef _STDIO_
#define _STDIO_

typedef struct FILE {
	int fd;
	int valid;
	int ungetcflag;
	int ungetchar;
	} FILE;

#ifdef __STDC__

/* get __va_list definition */
#define __need___va_list
#include <stdarg.h>

/* --- Inclusions --- */
#include <stddef.h>		/* bogus gets offsetof */

int	fclose	(FILE *);
FILE	*fopen	(const char *, const char *);

int	fgetc	(FILE *);
char	*fgets	(char *, int , FILE *);
size_t	fread	(void *, size_t, size_t, FILE *);
int	fscanf	(FILE *, const char *, ...);
int	getc	(FILE *);
int	getchar	(void);
char	*gets	(char *);
int	scanf	(const char *, ...);
int	sscanf	(const char *, const char *, ...);
int	ungetc	(int, FILE *);

int	fprintf	(FILE *, const char *, ...);
int	fputc	(int , FILE *);
int	fputs	(const char *, FILE *);
size_t	fwrite	(const void *, size_t, size_t, FILE *);
int	printf	(const char *, ...);
int	putc	(int, FILE *);
int	putchar	(int);
int	puts	(const char *);
int	sprintf	(char *, const char *, ...);

int	vfprintf (FILE *, const char *, __va_list);
int	vprintf	(const char *, __va_list);
int	vsprintf (char *, const char *, __va_list);

void	clearerr (FILE *);
int	feof	(FILE *);
int	ferror	(FILE *);
int	fflush	(FILE *);
int	fseek	(FILE *, long int, int);

#else

FILE *fopen();
char *fgets();
char *gets();

#endif

extern FILE _iob[];

#define stdin	(&_iob[0])
#define stdout	(&_iob[1])
#define stderr	(&_iob[2])

#ifdef OPEN_MAX
#undef OPEN_MAX
#endif
#define OPEN_MAX 8

#define MAXLN 256

#ifndef NULL
#define NULL 0
#endif

#define EOF  (-1)

typedef int iFunc();
typedef int *Addr;
#endif /* _STDIO_ */
