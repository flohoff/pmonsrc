/* $Id: netio.c,v 1.2 1996/01/16 14:19:35 chris Exp $ */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>

#include <netio.h>

struct netfile {
    const struct netfileops	*ops;
    netfh_t			handle;
};

#define NFILE	10
static struct netfile netfile[NFILE];

extern struct netfileops udpttyops;
extern const struct netfileops tftpops;

static const struct netfileops * const netops[] = {
    &udpttyops,
    &tftpops,
    NULL
};


static struct netfile *
getfh (int fd)
{
    struct netfile *nfp;

    if (fd < 0 || fd >= NFILE) {
	errno = EINVAL;
	return NULL;
    }
    nfp = &netfile[fd];
    if (!nfp->handle) {
	errno = EINVAL;
	return NULL;
    }
    return nfp;
}


int netopen (const char *path, int flags)
{
    struct netfile *nfp;
    const struct netfileops * const *ops;

    for (nfp = netfile; nfp < &netfile[NFILE] && nfp->handle; nfp++)
	continue;

    if (nfp >= &netfile[NFILE]) {
	errno = EMFILE;
	return (-1);
    }

    for (ops = netops; *ops; ops++)
	if (nfp->handle = ((*ops)->fo_open) (path, flags))
	    break;

    if (!nfp->handle) {
	errno = ENOENT;
	return (-1);
    }

    nfp->ops = *ops;
    return (nfp - netfile);
}


int netread (int fd, void *buf, int nb)
{
    struct netfile *nfp;

    if (!(nfp = getfh (fd)))
      return (-1);
    return (nfp->ops->fo_read) (nfp->handle, buf, nb);
}

int netwrite (int fd, const void *buf, int nb)
{
    struct netfile *nfp;

    if (!(nfp = getfh (fd)))
      return (-1);
    return (nfp->ops->fo_write) (nfp->handle, buf, nb);
}

long netlseek (int fd, long offs, int how)
{
    struct netfile *nfp;

    if (!(nfp = getfh (fd)))
      return (-1);
    return (nfp->ops->fo_lseek) (nfp->handle, offs, how);
}

int netioctl (int fd, int op, void *argp)
{
    struct netfile *nfp;

    if (!(nfp = getfh (fd)))
      return (-1);
    return (nfp->ops->fo_ioctl) (nfp->handle, op, argp);
}

int netclose (int fd)
{
    struct netfile *nfp;

    if (!(nfp = getfh (fd)))
      return (-1);
    (nfp->ops->fo_close) (nfp->handle);
    nfp->handle = NULL;
    return 0;
}
