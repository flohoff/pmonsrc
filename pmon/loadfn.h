#define DLREC	550		/* max download record size */

#define SFLAG 0x01		/* Don't clear symbols */
#define BFLAG 0x02		/* Don't clear breakpoints */
#define EFLAG 0x04		/* Don't clear exceptions */
#define AFLAG 0x08		/* Don't add offset to symbols */
#define TFLAG 0x10		/* Load at top of memory */
#define IFLAG 0x20		/* Ignore checksum errors */
#define VFLAG 0x40		/* verbose */
#define FFLAG 0x80		/* Load into flash */
#define NFLAG 0x100		/* Don't load symbols */
#define YFLAG 0x200		/* Only load symbols */
#define WFLAG 0x400		/* Swapped endianness */

#define DL_CONT		0
#define DL_DONE		1
#define DL_BADCHAR	2
#define DL_BADLEN	3
#define DL_BADTYPE	4
#define DL_BADSUM	5
#define DL_NOSYMSPACE	6
#define DL_BADADDR	7
#define DL_IOERR	8
#define DL_MAX		9

char const	*dl_err (int code);
int		dl_checkloadaddr (void *addr, int size, int verbose);
void		dl_initialise (unsigned long offset, int flags);
void		dl_setloadsyms (void);
int		dl_s3load (char *recbuf, int recsize, int *plen, int flags);
int		dl_fastload (char *recbuf, int recsize, int *plen, int flags);

extern long	dl_entry;	/* entry address */
extern long	dl_offset;	/* load offset */
extern long	dl_minaddr;	/* minimum modified address */
extern long	dl_maxaddr;	/* maximum modified address */

