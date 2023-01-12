/* $Id: netio.h,v 1.2 1996/01/16 14:17:22 chris Exp $ */
typedef struct netfh *netfh_t;

struct netfileops {
    netfh_t	(*fo_open)	(const char *, int);
    int		(*fo_read)	(netfh_t, void *, int);
    int		(*fo_write)	(netfh_t, const void *, int);
    long	(*fo_lseek)	(netfh_t, long, int);
    int		(*fo_ioctl)	(netfh_t, int, void *);
    int		(*fo_close)	(netfh_t);
};

extern int	netopen (const char *, int);
extern int	netread (int, void *, int);
extern int	netwrite (int, const void *, int);
extern long	netlseek (int, long, int);
extern int	netioctl (int, int, void *);
extern int	netclose (int);
