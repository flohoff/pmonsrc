/* $Id: bootparams.h,v 1.2 1996/01/16 14:19:32 chris Exp $ */
struct bootparams {
    unsigned short	need;
    unsigned short	have;
    struct in_addr	addr;
    struct in_addr	mask;
    struct in_addr	broadaddr;
    struct in_addr	gateip;
    struct in_addr	dnsip;
    struct in_addr	bootip;
    char		hostname[256];
    char		domainname[256];
    char		bootfile[129];
};

#define	BOOT_ADDR		0x001
#define	BOOT_MASK		0x002
#define	BOOT_GATEIP		0x004
#define	BOOT_DNSIP		0x008
#define	BOOT_BOOTIP		0x010
#define BOOT_HOSTNAME		0x020
#define BOOT_DOMAINNAME		0x040
#define BOOT_BOOTFILE		0x080
#define BOOT_BROADADDR		0x100
