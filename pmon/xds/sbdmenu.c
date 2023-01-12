/* $Id: sbdmenu.c,v 1.3 1996/01/17 13:46:36 chris Exp $ */
#include "mips.h"
#include "pmon.h"
#include "stdio.h"
#include "termio.h"
#include "string.h"
#include "signal.h"

#include "sys/types.h"
#include "sys/syslog.h"
#include "netinet/in.h"
#include "arpa/inet.h"

#include "sbd.h"

#define BEL	0x07
#define FF	0x0c

struct menu {
    char	*m_name;		/* menu string */
    char	*m_env;			/* corresponding environment value */
    char	*m_dval;		/* default value */
    char	*m_cval;		/* current value */
    char	*m_oval;		/* original value */
    int		m_key;			/* menu hot key */
    void	(*m_func) (struct menu *mp);	/* menu function */
};

static jmp_buf		menujmpb;

static void media_handler (struct menu *mp);
static void boothost_handler (struct menu *mp);
static void bootfile_handler (struct menu *mp);
static void netaddr_handler (struct menu *mp);
static void mask_handler (struct menu *mp);
static void broadcast_handler (struct menu *mp);
static void ping_handler (struct menu *mp);
static void undo_handler (struct menu *mp);
static void quit_handler (struct menu *mp);
static void exit_handler (struct menu *mp);

struct menu menu[] = {
     {"Boot media",	"media",	"ethernet", NULL, NULL,
     'A',
      media_handler},
    {"Boot host",	"boothost",	"193.117.190.130", NULL, NULL,
     'B', boothost_handler},
    {"Boot file",	"bootfile",	"/tftpboot/xds", NULL, NULL,
     'C', bootfile_handler},
    {"IP Address",	"netaddr",	"193.117.190.250", NULL, NULL,
     'D', netaddr_handler},
    {"Netmask",		"netmask",	"255.255.255.0", NULL, NULL,
     'E', mask_handler},
    {"Broadcast",	"broadcast",	"193.117.190.255", NULL, NULL,
     'F', broadcast_handler},
    {"Ping host",	NULL,		NULL, NULL, NULL,
     'G', ping_handler},
    {"Undo changes",	NULL,		NULL, NULL, NULL,
     'H', undo_handler},
    {"Quit to PMON",	NULL,		NULL, NULL, NULL,
     'Q', quit_handler},
    {"Continue",	NULL,		NULL, NULL, NULL,
     'X', exit_handler},
    {NULL}
};

static char *
strdup (char *s)
{
    int l;
    char *d = NULL;
    if (s) {
	l = strlen (s);
	d = (char *)malloc (l + 1);
	if (d)
	    strcpy (d, s);
    }
    return (d);
}

static int
isgraph (c) {
    return (isprint (c) && (c != ' '));
}

static void
menubuild (void)
{
    struct menu *mp;
    char *s;
    for (mp = menu; mp->m_name; mp++) {
	if (mp->m_env) {
	    s = getenv (mp->m_env);
	    if (s == NULL) {
		s = mp->m_dval;
		setenv (mp->m_env, s);
	    }
	    mp->m_oval = strdup (s);
	    mp->m_cval = strdup (s);
	}
	else
	    mp->m_cval = mp->m_oval = NULL;
    }
}

static void
menutitle (void)
{
    printf ("-------------------------------------------------------------------------------\n");
    printf ("TERMA Elektronik AS\n");
    printf ("XDS Boot Menu Revision 1.0\n");
    printf ("-------------------------------------------------------------------------------\n"); 
}

static void
menuclear (void)
{
    printf ("%c", FF);
}

static void
menubell (void)
{
    printf ("%c", BEL);
}

static void
menuerror (char *s)
{
    menubell ();
    printf ("Error: %s\n", s);
}


static
void menuchoices (void)
{
    struct menu *mp;
    for (mp = menu; mp->m_name; mp++) {
	if (mp->m_cval) 
	    printf ("%c - %-20s %s\n", mp->m_key, mp->m_name, mp->m_cval);
	else
	    printf ("%c - %s\n", mp->m_key, mp->m_name);
    }
}

static void
menudisplay (void)
{
    menuclear ();
    menutitle ();
    menuchoices ();
}

int
menukey (void)
{
    int             c;
    struct termio   tbuf;

    ioctl (STDIN, SETNCNE, &tbuf);
    c = getchar ();
    if (islower (c))
	c = toupper (c);
    ioctl (STDIN, TCSETAW, &tbuf);
    return (c);
}

static void
menuupdate (void)
{
    struct menu *mp;
    for (mp = menu; mp->m_name; mp++) {
	if (mp->m_env && mp->m_cval)
	    setenv (mp->m_env, mp->m_cval);
    }
}

static void
menurun ()
{
    struct menu *mp;
    int c;

    menubuild ();
    if (setjmp (jmpb) == 0) {
	for (;;) {
	    menudisplay ();
	    c = menukey ();
	    for (mp = menu; mp->m_name; mp++) {
		if (mp->m_key == c)
		    break;
	    }
	    if (!mp->m_name) {
		menubell ();
		continue;
	    }
	    (mp->m_func) (mp);
	}
    }
}


static void
media_handler (struct menu *mp)
{
    if (strcmp (mp->m_cval, "Ethernet") == 0) {
 	if (mp->m_cval)
	    free (mp->m_cval);
	mp->m_cval = strdup ("VMEbus");
    }
    else {
 	if (mp->m_cval)
	    free (mp->m_cval);
	mp->m_cval = strdup ("Ethernet");
    }
}

static int
get_ip_address (struct menu *mp, char *prompt, char *buf)
{
    struct in_addr inaddr;

    printf ("Enter IP address for %s (x.x.x.x): ", prompt);
    if (mp->m_cval)
	strcpy (buf, mp->m_cval);
    else
	buf[0] = '\0';
    get_line (buf, 0);
    if (inet_aton (buf, &inaddr) == 0) {
	menuerror ("invalid IP address");
	return (0);
    }
    strcpy (inet_ntoa (inaddr), buf);
    return (1);
}

static void
boothost_handler (struct menu *mp)
{
    char buf[1024];
    if (get_ip_address (mp, "boot host", buf) == 0)
	return;
    if (mp->m_cval)
	free (mp->m_cval);
    mp->m_cval = strdup (buf);
}    

static void
bootfile_handler (struct menu *mp)
{
    char buf[1024];
    char *s;
    if (mp->m_cval)
	strcpy (buf, mp->m_cval);
    else
	buf[0] = '\0';
    printf ("Enter boot file name: ");
    get_line (buf, 0);
    for (s = buf; *s; s++) {
	if (! isgraph (*s)) {
	    menuerror ("file name contains non printable characters");
	    return;
	}
    }
    if (mp->m_cval)
	free (mp->m_cval);
    mp->m_cval = strdup (buf);
}

static void
netaddr_handler (struct menu *mp)
{
    char buf[1024];
    if (get_ip_address (mp, "XDS board", buf) == 0)
	return;
    if (mp->m_cval)
	free (mp->m_cval);
    mp->m_cval = strdup (buf);
}

static void
mask_handler (struct menu *mp)
{
    char buf[1024];
    unsigned int netmask;
    struct in_addr inaddr;
    if (get_ip_address (mp, "network mask", buf) == 0)
	return;

    /* check for validity
     * a network mask must be all ones in the top half and all zeros
     * in the bottom half
     * check this by inverting the network mask and seeing if the
     * result is == 2^n -1
     */
    inet_aton (buf, &inaddr);
    netmask = ntohl (inaddr.s_addr);	/* should give 11...1100...00 */
    netmask = ~netmask;			/* should give 00...0011...11 */
    netmask++;				/* should give 000000100...00 */
    if ((netmask & (netmask-1)) != 0) {
	menuerror ("invalid network mask");
	return;
    }

    if (mp->m_cval)
	free (mp->m_cval);
    mp->m_cval = strdup (buf);
}

static void
broadcast_handler (struct menu *mp)
{
    char buf[1024];
    struct in_addr inaddr;
    if (get_ip_address (mp, "network mask", buf) == 0)
	return;

    if (mp->m_cval)
	free (mp->m_cval);
    mp->m_cval = strdup (buf);
}

static void
ping_handler (struct menu *mp)
{
    char buf[1024];
    char cmd[1024];
    char *bp, *s;

    /* menuupdate (); */

    if (get_ip_address (mp, "host to ping", buf) == 0)
	return;

    sprintf (cmd, "ping -c 15 %s", buf);
    do_cmd (cmd);
}

static void
undo_handler (struct menu *mp)
{
    for (mp = menu; mp->m_name; mp++) {
	if (mp->m_env) {
	    if (mp->m_cval)
		free (mp->m_cval);
	    if (mp->m_oval)
		mp->m_cval = strdup (mp->m_oval);
	    else if (mp->m_dval)
		mp->m_cval = strdup (mp->m_dval);
	}
    }
}

static void
exit_handler (struct menu *mp)
{
    menuupdate ();
    for (mp = menu; mp->m_name; mp++) {
	if (mp->m_env) {
	    if (mp->m_cval) {
		setenv (mp->m_env, mp->m_cval);
		free (mp->m_cval);
		mp->m_cval = NULL;
	    }
	    if (mp->m_oval) {
		free (mp->m_oval);
		mp->m_oval = NULL;
	    }
	}
    }
    longjmp (jmpb, 1);
}


static void
quit_handler (struct menu *mp)
{
    /* a bit of a hack to allow easy access to PMON */
    main ();
}


int
sbd_main (int argc, char **argv)
{
    char cmd[1024];
    char *boothost, *bootfile;
    unsigned int switches;
    int firsttime = 1;
    
    switches = sbd_switches ();
    log (LOG_INFO, "Configuration switches 0x%02x\n", switches);

    /* just go straight to PMON if in debug mode */
    if (switches & SWITCH_DEBUG)
	main ();

    for (;;) {
	if ((boothost = getenv ("boothost")) &&
	    (bootfile = getenv ("bootfile"))) {
	    sprintf (cmd, "boot %s:%s;g", boothost, bootfile);
	    if (firsttime)
		autorun (cmd);
	    else
		do_cmd (cmd);
	}
	firsttime = 0;
	menurun ();
    }
    /* NOTREACHED */
    return (0);
}
