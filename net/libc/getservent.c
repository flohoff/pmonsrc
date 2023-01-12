/* $Id: getservent.c,v 1.5 2001/10/31 11:48:36 chris Exp $ */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/endian.h>

struct servtab {
    const char	*name;
    const char	*alias;
    short	port;
    short	proto;
};

#define TCP	0
#define UDP	1

static const struct servtab servtab[] = {
    {"echo",	0,	7,	TCP},
    {"echo",	0,	7,	UDP},
    {"discard", "sink",	9,	TCP},
    {"discard",	"sink",	9,	UDP},
    {"daytime", 0,	13,	TCP},
    {"daytime",	0,	13,	UDP},
    {"ftp",	0,	21,	TCP},
    {"telnet",	0,	23,	TCP},
    {"time", 	0,	37,	TCP},
    {"time",	0,	37,	UDP},
    {"bootps",	0,	67,	UDP},
    {"bootpc",	0,	68,	UDP},
    {"tftp",	0,	69,	UDP},
    {"sunrpc",	0,	111,	TCP},
    {"sunrpc",	0,	111,	UDP},
    /* Unix specials */
    {"exec",	0,	512,	TCP},
    {"login",	0,	513,	TCP},
    {"shell",	"cmd",	514,	TCP},
    {"talk",	0,	518,	UDP},
    {"bfs",	0,	2201,	UDP},	/* MIPS boot file server */
    {"gdb",	"rdbg",	2202,	UDP},	/* GDB remote debugging */
    {"gdbremote",0,	2159,	UDP},	/* official */
    {"gdbremote",0,	2159,	TCP},	/* official */
    {0}
};

static int servidx;
int _serv_stayopen;

void setservent(x)
{
    servidx = 0;
}

void endservent()
{
    servidx = -1;
}    

struct servent *
getservent()
{
    register const struct servtab *st;
    static struct servent sv;
    static char *aliases[2];

    if (servidx < 0)
      return (0);

    st = &servtab[servidx];
    if (!st->name)
      return (0);

    sv.s_name = (char *)st->name;
    aliases[0] = (char *)st->alias; aliases[1] = 0;
    sv.s_aliases = aliases;
    sv.s_port = htons(st->port);
    sv.s_proto = (st->proto == TCP) ? "tcp" : "udp";

    servidx++;
    return (&sv);
}
