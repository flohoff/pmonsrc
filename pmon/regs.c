/* $Id: regs.c,v 1.21 2000/03/28 00:20:59 nigel Exp $ */
#include "string.h"
#include "mips.h"
#include "pmon.h"
#include "termio.h"

#define loop for (;;)
#define BADREG "error: arg2 [%s] bad register name\n"
#define BADFIELD "error: arg3: [%s] bad field name\n"
#define BADBASE "error: arg%d: [%s] base %d value expected\n"
#define BADVALUE "error: arg%d: [%s] max value %d expected\n"
#define BADSVALUE "error: arg4: [%s] bad symbolic value\n"
#define REG_RO "Register is readonly\n"

/* forward declarations */
static int	disp_reg (char *, RegList *, int, ureg_t, int);
static int      dispTlb (char *, int *, int);
static int	disp_Gprs (int);
static int	dispReg (int [], int, int *, int);
static void	prcentre (char *, int);
static int	fieldsz (const RegSpec *);
static void	setfield (ureg_t *, int, int, ureg_t);

char           *btoa ();
extern char    *regs_sw[];
extern RegList  reglist[];
extern char    *excodes[];
#ifdef R4000
extern char    *excodes_r4650[];
#endif


#ifdef R4000
int regsize = REGSZ_32;
#else
#define regsize REGSZ_32
#endif
    
const Optdesc         r_opts[] =
{
    {"*", "display all registers"},
    {"t*", "display all registers starting with t"},
    {"reg value", "set specified register"},
    {"reg field value", "set specified field"},
#ifdef FLOATINGPT
    {"f*", "display all fp registers"},
#endif
    {0}};

/*************************************************************
 *  registers(ac,av) 
 *      the 'r' (display registers) command
 */
registers (ac, av)
     int             ac;
     char           *av[];
{
    int             i, j, len, lmargin[2], flag, wild, l, siz;
    unsigned int    w;
    const RegSpec  *p;
    char            buf[80], tmp1[20], tmp2[20];
    word           *pr;
    sreg_t          rn;
    word            n;
    int out;
    static int saved_place = 0;	/* For repeating the 'r' command remembers where we last were */

    ioctl (STDIN, CBREAK, NULL);
#ifdef R4000
    regsize = matchenv ("regsize");
#endif

#ifdef FLOATINGPT
    if (ac > 1 && (av[1][0] == 'f' || av[1][0] == 'F')) {
	if (initial_sr & SR_CU1)
	  return (fregs (ac, av));
	printf (BADREG);
	return (1);
    }
#endif

    prnbuf[0] = 0;
    l = siz = moresz;

    lmargin[0] = lmargin[1] = 0;
    for (j = 0; reglist[j].reg != 0; j++) {
	len = strlen (reglist[j].name);
	if (reglist[j].spec) {
	    if (len > lmargin[1])
		lmargin[1] = len;
	} else {
	    if (len > lmargin[0])
		lmargin[0] = len;
	}
    }

    switch (ac) {
    case 1:			/* displ GP registers */
	disp_Gprs (1);
	break;
    case 2:			/* displ selected register(s) */
	if (strequ (av[1], "*")) {	/* all regs */
	    disp_Gprs (0);
	    l -= 4;
	    out = 0;		/* Flag to indicate more has said to exit */
	    for (j = 0; reglist[j].reg != 0; j++) {
		if (reglist[j].flags & F_GPR)
		    continue;
		if (!in_machtype (reglist[j].flags))
		    continue;
		if (dispReg (lmargin, j, &l, siz)) {
		    out = 1;
		    break;
		}
	    }
	    if( out )
		break;		/* more() has said to quit */
	    if (strlen (prnbuf))
		more (prnbuf, &l, siz);
	} else {		/* display reg(s) by name */
	    flag = 0;
	    if (strchr (av[1], '*') || strchr (av[1], '?'))
		wild = 1;
	    else
		wild = 0;
	    strtoupper (av[1]);
	    if( repeating_cmd )
	    {
		/*
		 * We're repeating the 'r' command with an argument.  Bump
		 * to next legal register for display (do wrap).
		 */

		for( ;; )
		{
		    saved_place++;
		    if( !reglist[saved_place].reg )
			saved_place = 0;
		    if( in_machtype( reglist[saved_place].flags  ) )
			break;
		}

		if( dispReg( lmargin, saved_place, &l, siz ) )
		    break;
		flag = 1;
	    }
	    else
	    {
	    for (j = 0; reglist[j].reg != 0; j++) {
		if (!in_machtype (reglist[j].flags))
		    continue;
		strcpy (tmp1, reglist[j].name);
		if (wild)
		    *tmp2 = 0;
		else
		    strcpy (tmp2, reglist[j].aname);
		strtoupper (tmp1);
		strtoupper (tmp2);
		if (strpat (tmp1, av[1]) || strpat (tmp2, av[1])) {
			saved_place = j;
		    if (dispReg (lmargin, j, &l, siz))
			break;
		    flag = 1;
		}
	    }
	    }
	    
	    if (strlen (prnbuf))
		more (prnbuf, &l, siz);
	    if (flag == 0)
		printf (BADREG, av[1]);
	}
	break;
    case 3: 			/* set entire register */
	if (!get_rsa_reg (&rn, av[2]))
	  return (-1);
	flag = 0;
	if (strchr (av[1], '*') || strchr (av[1], '?'))
	  wild = 1;
	else
	  wild = 0;
	strtoupper (av[1]);
	for (j = 0; reglist[j].reg != 0; j++) {
	    if (!in_machtype (reglist[j].flags))
	      continue;
	    strcpy (tmp1, reglist[j].name);
	    if (wild)
	      *tmp2 = 0;
	    else
	      strcpy (tmp2, reglist[j].aname);
	    strtoupper (tmp1);
	    strtoupper (tmp2);
	    if (strpat (tmp1, av[1]) || strpat (tmp2, av[1])) {
		if (reglist[j].flags & F_RO)
		  printf (REG_RO);
		else if (reglist[j].flags & F_CF)
		  ((Func *) reglist[j].reg) (1, reglist[j].regnum, rn);
#if __mips >= 3
		else if (reglist[j].flags & F_64 && regsize == REGSZ_64) {
		    /* watch out for confusing mistake */
		    if ((rn >> 31) != (rn << 32 >> 63))
		      printf ("warning: 0x%016llx is an invalid 32-bit value\n", rn);
		    if (reglist[j].spec) {
			for (p = reglist[j].spec; p->name; p++) {
			    reg_t mask;
			    if (!in_machtype (p->flags) || p->ro)
				continue;
			    mask = ((1LL << p->size) - 1) << p->lsb;
			    *reglist[j].reg = (*reglist[j].reg & ~mask) | (rn & mask);
			}
		    }
		    else
			*reglist[j].reg = rn;
		}
#endif
		else {
		    if (reglist[j].spec) {
			for (p = reglist[j].spec; p->name; p++) {
			    word mask;
			    if (!in_machtype (p->flags) || p->ro)
				continue;
			    mask = ((1 << p->size) - 1) << p->lsb;
			    *reglist[j].reg = (*reglist[j].reg & ~mask) |
				((word) rn & mask);
			}
		    }
		    else
			*reglist[j].reg = (word) rn;
		}
		flag = 1;
	    }
	}
	if (flag == 0)
	  printf (BADREG, av[1]);
	break;
    case 4:			/* set register field */
	for (j = 0; reglist[j].reg != 0; j++) {
	    if (!in_machtype (reglist[j].flags))
		continue;
	    if (striequ (av[1], reglist[j].name))
		break;
	    if (striequ (av[1], reglist[j].aname))
		break;
	}
	if (reglist[j].reg == 0) {
	    printf (BADREG, av[1]);
	    return (-1);
	}
	/* found the reg, now find the field */
	p = reglist[j].spec;
	for (i = 0; p[i].name != 0; i++) {
	    if (in_machtype(p[i].flags) && striequ (av[2], p[i].name))
		break;
	}
	if (p[i].name == 0) {
	    printf (BADFIELD, av[2]);
	    return (-1);
	}
	/* found the field, now find the value */
	if (p[i].ro) {
	    printf (REG_RO);
	    return (1);
	}
	/* first check to see if should be symbolic */
	if (p[i].base == 0) {
	    for (n = 0; p[i].values[n] != 0; n++) {
		if (striequ (av[3], p[i].values[n]))
		    break;
	    }
	    if (p[i].values[n] == 0) {
		printf (BADSVALUE, av[3]);
		return (1);
	    }
	} else {
	    if (!atob (&n, av[3], p[i].base)) {
		printf (BADBASE, 4, av[3], p[i].base);
		return (-1);
	    }
	    if (n >= (1 << p[i].size)) {
		printf (BADVALUE, 4, av[3], (1 << p[i].size) - 1);
		return (-1);
	    }
	}
	if (reglist[j].flags & F_CF) {
	    ureg_t v = ((Func *) reglist[j].reg) (0, reglist[j].regnum);
	    setfield (&v, p[i].size, p[i].lsb, n);
	    ((Func *) reglist[j].reg) (1, reglist[j].regnum, v);
	} else
	    setfield (reglist[j].reg, p[i].size, p[i].lsb, n);
#ifdef SETBS
	if (reglist[j].reg == (word *) CFGREG
	    && strequ (p[i].name, "IBS"))
	    setibs (n);
	if (reglist[j].reg == (word *) CFGREG
	    && strequ (p[i].name, "DBS"))
	    setdbs (n);
#endif
	break;
    }
    return (0);
}

/*************************************************************
 *  dispReg(lmargin,j,ln,siz)
 */
static int
dispReg (lmargin, j, ln, siz)
     int             lmargin[], j, *ln, siz;
{
    char            tmp[80];
    sreg_t          value;
    int		    state;

    if (reglist[j].flags & F_WO) {
	if (lmargin[0] + strlen (prnbuf) + 10 > 79)
	    if (more (prnbuf, ln, siz))
		return (1);
	sprintf (tmp, "%*s=WRTEONLY ", lmargin[0], reglist[j].name);
	strcat (prnbuf, tmp);
	return (0);
    }
    if (reglist[j].flags & F_CF)
	value = ((Func *) reglist[j].reg) (0, reglist[j].regnum);
    else
	value = *reglist[j].reg;

    if( strcmp( reglist[j].name, "C0_TLB" ) == 0 )
	dispTlb( prnbuf, ln, siz);
    else if (reglist[j].spec) {
	if (strlen (prnbuf))
	    if (more (prnbuf, ln, siz))
		return (1);
	state = 0;
	while (disp_reg (prnbuf, &reglist[j], lmargin[1], value, state++)) {
	    if (more (prnbuf, ln, siz))
		return (1);
	}
    } else if (reglist[j].flags & F_GPR && regsize == REGSZ_32) {
	if (lmargin[0] + strlen (prnbuf) + 10 > 79)
	    if (more (prnbuf, ln, siz))
		return (1);
	sprintf (tmp, "%*s=%s=%08x ", lmargin[0] - 3, reglist[j].aname,
		 reglist[j].name, (word)value);
	strcat (prnbuf, tmp);
    } else if (reglist[j].flags & F_GPR && regsize == REGSZ_64) {
	if (lmargin[0] + strlen (prnbuf) + 18 > 79)
	    if (more (prnbuf, ln, siz))
		return (1);
	sprintf (tmp, "%*s=%s=%016llx ", lmargin[0] - 3, reglist[j].aname,
		 reglist[j].name, value);
	strcat (prnbuf, tmp);
    } else if (reglist[j].flags & F_64 && regsize == REGSZ_64) {
	if (lmargin[0] + strlen (prnbuf) + 18 > 79)
	    if (more (prnbuf, ln, siz))
		return (1);
	sprintf (tmp, "%*s=%016llx ", lmargin[0],
		 reglist[j].name, value);
	strcat (prnbuf, tmp);
    } else {
/*	if (lmargin[0] + strlen (prnbuf) + 10 > 79)*/
	if( strlen( prnbuf ) )
	    if (more (prnbuf, ln, siz))
		return (1);
	sprintf (tmp, "%*s=%08x ", lmargin[0], reglist[j].name, (word)value);
	strcat (prnbuf, tmp);
    }
    return (0);
}


static void
prcentre (str, width)
    char *str;
{
    int len = strlen(str);
    int isp = (width - len) / 2;
    int i;

    for (i = isp; i > 0; i--)
      putchar (' ');
    printf ("%s", str);
    for (i = width - (isp + len); i > 0; i--)
      putchar (' ');
}

/*************************************************************
 *  disp_Gprs(n)
 */
static int
disp_Gprs (n)
     int             n;
{
    int             i, regno;
    int		    cols = 8;
    int		    width = 8;

    if (regsize == REGSZ_64)
      cols = 4, width= 16;

    for (regno = 0; regno < 32; regno += cols) {
	if (n == 1) {
	    printf ("    ");
	    for (i = 0; i < cols; i++) {
		printf (" "); prcentre (regs_sw[regno + i], width);
	    }
	    printf ("\n");
	}

	printf ("$%02d-", regno);
	for (i = 0; i < cols; i++)
	  if (regsize == REGSZ_64)
	    printf (" %016llx", Gpr[regno + i]);
	  else
	    printf (" %08x", (word) Gpr[regno + i]);
	printf ("\n");
    }
}

/*************************************************************
 *  disp_reg(pb,q,lmargin,value,state)
 */
static int
disp_reg (pb, q, lmargin, value, state)
     char           *pb;
     RegList        *q;
     int             lmargin, state;
     ureg_t          value;
{
    int             i, val, len, width, col, n;
    char            buf[80], tmp[80];
    const RegSpec  *p;
    static int      posn;

    if (lmargin == 0)
	lmargin = strlen (q->name);

    if (q->spec == 0) {
	printf ("disp_reg: spec==0\n");
	return (1);
    }
    if (state == 0)
	posn = 0;
    p = q->spec;
    if (p[posn].size == 0)
	return (0);

    *pb = 0;
    if ((state & 1) == 0) {	/* print field names */
	sprintf (pb, "%*s:%08x\n %*s", lmargin, q->name, (int)value, lmargin, " " );
	col = lmargin + 2;
	for (i = posn; p[i].size != 0; i++) {
	    if (p[i].name == 0)
		continue;
	    if (!in_machtype (p[i].flags))
		continue;
	    width = fieldsz (&p[i]);
	    if (col + width + 1 > 76)
		break;
	    strcpy (buf, p[i].name);
	    str_fmt (buf, width, FMT_CENTER);
	    strcat (pb, buf);
	    strcat (pb, " ");
	    col += width + 1;
	}
    } else {			/* print field values */
	for (n = lmargin + 1; n > 0; n--)
	    strcat (pb, " ");	/* +1 for ":" */
	col = lmargin + 2;
	for (i = posn; p[i].size != 0; i++) {
	    if (!in_machtype (p[i].flags))
		continue;
	    width = fieldsz (&p[i]);
	    if (col + width + 1 > 76)
		break;
	    val = getfield (value, p[i].size, p[i].lsb);
	    if (p[i].base == 0)
		strcpy (buf, p[i].values[val]);
	    else {
		btoa (buf, val, p[i].base);
		switch (p[i].base) {
		case 2:
		    str_fmt (buf, p[i].size, FMT_RJUST0);
		    break;
		case 8:
		    str_fmt (buf, (p[i].size + 2) / 3, FMT_RJUST0);
		    break;
		case 16:
		    str_fmt (buf, (p[i].size + 3) / 4, FMT_RJUST0);
		    break;
		}
	    }
	    str_fmt (buf, width, FMT_CENTER);
	    strcat (pb, buf);
	    strcat (pb, " ");
	    col += width + 1;
	}
	posn = i;
    }
    return (1);
}

/*************************************************************
 *  fieldsz(p)
 */
static int
fieldsz (p)
     const RegSpec  *p;
{
    int             nmsz, valsz, len;
    const char * const *q;

    nmsz = strlen (p->name);
    switch (p->base) {
    case 0:
	valsz = 0;
	for (q = p->values; *q; q++) {
	    len = strlen (*q);
	    if (len > valsz)
		valsz = len;
	}
	break;
    case 2:
	valsz = p->size;
	break;
    case 8:
	valsz = (p->size + 2) / 3;
	break;
    case 10:
	valsz = (p->size + 2) / 3 + 1;
	break;
    case 16:
	valsz = (p->size + 3) / 4;
	break;
    }
    return (nmsz > valsz) ? nmsz : valsz;
}

/*************************************************************
 *  setfield(dstp,size,lsb,value)
 */
static void
setfield (dstp, size, lsb, value)
    ureg_t	*dstp, value;
    int         size, lsb;
{
    ureg_t	msk, v;

    msk = (((ureg_t)1 << size) - 1) << lsb;
    v = *dstp;
    v &= ~msk;
    v |= value << lsb;
    *dstp = v;
}

/*************************************************************
 *  getreg(vp,p)
 */
getreg (vp, p)
     sreg_t         *vp;
     char           *p;
{
    sreg_t	    *adr;

    if (!getregadr (&adr, p))
	return (0);
    *vp = load_reg (adr);
    return (1);
}

/*************************************************************
 *  getregadr(vp,p)
 */
getregadr (vp, p)
     sreg_t         **vp;
     char           *p;
{
    unsigned long   j;

    if (isdigit (*p)) {
	if (!atob (&j, p, 10) || j > 31)
	    return (0);
	if (vp)
	  *vp = &Gpr[j];
    } else if ((j = regno (p)) != -1) {
	if (vp)
	  *vp = &Gpr[j];
    } else {
	for (j = 0; reglist[j].reg != 0; j++) {
	    if (striequ (p, reglist[j].name))
		break;
	    if (striequ (p, reglist[j].aname))
		break;
	}
	if (!reglist[j].reg)
	    return (0);
	if (vp)
	  *vp = reglist[j].reg;
    }
    return (1);
}

/*************************************************************
 *  regno(p)
 */
regno (p)
     char           *p;
{
    int             i;

    for (i = 0; i < 32; i++) {
	if (striequ (p, regs_sw[i]))
	    return (i);
    }
    return (-1);
}

/*************************************************************
 *  char *getexcname(n)
 */
char           *
getexcname (n)
     int             n;
{
#ifdef R4000
    if (machtype == 4640 || machtype == 4650)
	return (excodes_r4650[n >> 2]);
#endif
    return (excodes[n >> 2]);
}


#ifdef FLOATINGPT

/*
 * check for denormalised floating point
 */
int spdenorm(sp)
    struct IEEEsp *sp;
{
    return (sp->exp == 0 && sp->man);
}


/*
 * check for denormalised floating point
 */
int dpdenorm(dp)
    struct IEEEdp *dp;
{
    return (dp->exp == 0 && (dp->manh | dp->manl));
}


int spnan(sp)
     struct IEEEsp *sp;
{
  return( ( sp->exp == 0xff ) && sp->man );
}

int dpnan(dp)
     struct IEEEdp *dp;
{
  return( ( dp->exp == 0x7ff ) && dp->manh );
}

/*************************************************************
 *  fregs(ac,av)
 */
fregs (ac, av)
     int             ac;
     char           *av[];
{
    dword	   dv;
    word	   sv;
    double         *dp;
    float          *sp;
    unsigned long  i;
    int 	   fp_inc;
    int		   cnt;
    int            show_fmt;
    show_fmt = matchenv("fpfmt" );
    if( ( show_fmt < 0 ) || ( show_fmt > 3 ) )
	show_fmt = 0;
      

    sp = (float *)&sv;
    dp = (double *)&dv;
#if __mips >= 3
    fp_inc = (Status & SR_FR) ? 1 : 2;
#else
    fp_inc = 2;
#endif
    cnt = moresz;

    if (ac == 2) {
	if (av[1][1] == '*') {
	    for (i = 0; i < 32; i += fp_inc) {
		char *b = prnbuf;
		sv = Fpr[i];
#if __mips >= 3
		dv = Fpr[i];
#else
		dv = Fpr[i + 1];
		dv = (dv << 32) | sv;
#endif
		/*
		 * show_fmt == 0 -> show both double and single
		 * show_fmt == 1 -> show double
		 * show_fmt == 2 -> show single
		 */

		if (machtype == 4640 || machtype == 4650)
		    b += sprintf (b, "$f%-2d = %08x", i, sv);
		else {
		  if (show_fmt == 0 || show_fmt == 1 || show_fmt == 3)
  		    b += sprintf (b, "$f%-2d = %08x/%08x", i,
				  (word)(dv >> 32), sv);
		  else
 		    b += sprintf (b, "$f%-2d = %08x", i, sv);
		  if (show_fmt == 0 || show_fmt == 2) {
		    if (spdenorm ((struct IEEEsp *)&sv))
			b += sprintf (b, " sp=0.0 (denorm)");
		    else if (spnan((struct IEEEsp *)&sv))
			b += sprintf (b, " sp=Nan");
		    else
			b += sprintf (b, " sp=%e", *sp);
		  }
		  if (show_fmt == 0 || show_fmt == 1) {
		    if (dpdenorm ((struct IEEEdp *)&dv))
		      sprintf (b, " dp=0.0 (denorm)");
		    else if (dpnan((struct IEEEdp *)&dv))
		      b += sprintf (b, " dp=Nan");
		    else
		      sprintf (b, " dp=%e", *dp);
		  }
		}
		if (more (prnbuf, &cnt, moresz))
		  return;
	    }
	    disp_as_reg (&Fcr, "C1_CSR", &cnt);
	    disp_as_reg (&Fid, "C1_FRID", &cnt);
	} else {
	    if (atob (&i, &av[1][1], 10) && i < 32) {
		i &= ~(fp_inc - 1);	/* clear LSB */
		sv = Fpr[i];
#if __mips >= 3
		dv = Fpr[i];
#else
		dv = Fpr[i + 1];
		dv = (dv << 32) | sv;
#endif
		if (machtype == 4640 || machtype == 4650) {
		    printf ("$f%-2d = %08x ", i, sv);
		}
		else {
		  /*
		   * show_fmt == 0 -> show both double and single
		   * show_fmt == 1 -> show double
		   * show_fmt == 2 -> show single
		   */
 
		  if (show_fmt == 0 || show_fmt == 1)
		    printf ("$f%-2d = %08x/%08x", i, (word)(dv >> 32), sv);
		  else
		    printf ("$f%-2d = %08x", i, sv);

		  if (show_fmt == 0 || show_fmt == 2) {
		    if (spdenorm ((struct IEEEsp *)&sv))
			printf (" sp=0.0 (denorm)");
		    else if (spnan ((struct IEEEsp *)&sv))
			printf (" sp=Nan");
		    else
			printf (" sp=%e", *sp);
		  }
		  if (show_fmt == 0 || show_fmt == 1) {
		    if (dpdenorm ((struct IEEEdp *)&dv))
		      printf (" dp=0.0 (denorm)");
 		    else if (dpnan ((struct IEEEdp *)&dv))
		      printf (" dp=Nan");
		    else
		      printf (" dp=%e", *dp);
		  }
		}
		printf ("\n");
	    } else {
		printf ("bad register number\n");
		return (-1);
	    }
	}
    } else if (ac == 3) {
	ureg_t n;
	get_rsa_reg (&n, av[2]);
	if (atob (&i, &av[1][1], 10) && i < 32)
	    Fpr[i] = n;
	else {
	    printf ("bad register number\n");
	    return (-1);
	}
    } else {
	printf ("bad arg count\n");
	return (-1);
    }
    return (0);
}
#endif

/*************************************************************
 *  disp_as_reg(regp,p)
 *      display reg by name
 */
int
disp_as_reg (regp, p, cntp)
     reg_t    	    *regp;
     char           *p;
     int	    *cntp;
{
    ureg_t          value;
    int             j, state;
    char	   *b;

    prnbuf[0] = 0;
    for (j = 0; reglist[j].reg != 0; j++) {
	if (striequ (reglist[j].name, p) || striequ (reglist[j].aname, p)) {
	    if (regp == reglist[j].reg)
	      value = load_reg (regp);
	    else if (reglist[j].flags & F_64 && regsize == REGSZ_64)
	      value = load_dword (regp);
	    else 
	      value = load_word (regp);
	    if (reglist[j].spec) {
		state = 0;
		while (disp_reg (prnbuf, &reglist[j], 0, value, state++))
		  if (more (prnbuf, cntp, moresz))
		    break;
	    } else {
		if (reglist[j].flags & F_ANAME)
		    sprintf (prnbuf, "$%s-", reglist[j].aname);
		strcat (prnbuf, reglist[j].name);
		b = prnbuf + strlen (prnbuf);
		if (reglist[j].flags & F_64 && regsize == REGSZ_64)
		  sprintf (b, ": %016llx\n", value);
		else
		  sprintf (b, ": %08x\n", (word)value);
		(void) more (prnbuf, cntp, moresz);
	    }
	    return (1);
	}
    }
    return (0);
}

#ifdef LR33000

#ifdef SABLE
long            c2c[32], c2d[32];

cfc2 (reg)
{
    return (c2c[reg]);
}
ctc2 (reg, value)
{
    c2c[reg] = value;
}
mfc2 (reg)
{
    return (c2d[reg]);
}
mtc2 (reg, value)
{
    c2d[reg] = value;
}
cp2init ()
{
}
#endif /* SABLE */

/*************************************************************
 *  mXc2(mode,reg,value)
 */
mXc2 (mode, reg, value)
     int             mode, reg, value;
{

    if (mode == 0)		/* read */
	return (mfc2 (reg));
    else
	mtc2 (reg, value);
}

/*************************************************************
 *  cXc2(mode,reg,value)
 */
cXc2 (mode, reg, value)
     int             mode, reg, value;
{

    if (mode == 0)		/* read */
	return (cfc2 (reg));
    else
	ctc2 (reg, value);
}
#endif /* LR33000 */

/*************************************************************
 *  mXc0(mode,reg,value)
 */
mXc0 (mode, reg, value)
     int             mode, reg, value;
{

    if (mode == 0)		/* read */
	return (mfc0 (reg));
    else
	mtc0 (reg, value);
}


/*************************************************************
 *  in_machtype(n)
 *      checks to see if a particular register is in the current
 *      machine type.
 */
in_machtype (n)
     int             n;
{
    if (n == 0)
	return (1);
#ifdef FLOATINGPT
    if ((n & F_FPU) && !(initial_sr & SR_CU1))
      return 0;
#else
    if (n & F_FPU)
      return 0;
#endif
    switch (machtype) {
    case 3041:
	return (n & F_3041);
    case 3081:
	return (n & F_3081);
    case 33000:
	return (n & F_00);
    case 33020:
	return (n & F_20);
    case 33050:
	return (n & F_50);
    case 4650:
    case 4640:
	return (n & F_4650);
    case 5000:
    case 5230:
    case 5231:
    case 5260:
    case 5261:
    case 5270:
    case 64574:
    case 64575:
	return (n & F_5000);
    case 7000:
	return (n & F_7000);
    case 5432:
    case 5464:
	return (n & F_5400);
    case 4100:
    case 4101:
    case 4102:
    case 4111:
    case 4121:
	return (n & F_4100);
    default:
	return (n & F_OTHER);
    }
}

static int
dispTlb (pb, l, siz)
  char           *pb;
  int *l;
  int siz;
{
    int i;
    struct tlbentry tlb;
    int		    ntlb = NTLBENTRIES;
    word	    va, pa0, pa1;
    unsigned long   mask, pgsize;
    char     	    pgsizebuf[8];

    switch (machtype) {
    case 4640:
    case 4650:
	/* no TLB */
	return;
    }

    switch (machtype / 100) {
    case 41:
    case 42:
    case 43:
    case 323:
	ntlb = 32; break;
    }


#ifdef R4000
    strcpy (pb, "TLB: PAGEMASK ENTRYHI  ENTRYLO0 ENTRYLO1");
    if (more (pb, l, siz))
	return 1;
    
    for (i = 0; i < ntlb; i++) {
      tlbread (i, &tlb);
      sprintf (pb, "%02d:  %08x %08x %08x %08x", i,
	       (word)tlb.tlb_mask, (word)tlb.tlb_hi,
	       (word)tlb.tlb_lo0, (word)tlb.tlb_lo1);
      if (more (pb, l, siz))
	return (1);

      if (in_machtype (F_4100)) {
	  mask = (tlb.tlb_mask & TLBMASK_4100_MASKMASK) | 0x7ff;
	  va = tlb.tlb_hi & TLBHI_4100_VPN2MASK;
	  pa0 = (tlb.tlb_lo0 & TLBLO_PFNMASK) << (10-6);
	  pa1 = (tlb.tlb_lo1 & TLBLO_PFNMASK) << (10-6);
      }
      else {
	  mask = (tlb.tlb_mask & TLBMASK_MASKMASK) | 0x1fff;
	  va = tlb.tlb_hi & TLBHI_VPN2MASK;
	  pa0 = (tlb.tlb_lo0 & TLBLO_PFNMASK) << (12-6);
	  pa1 = (tlb.tlb_lo1 & TLBLO_PFNMASK) << (12-6);
      }

      pgsize = (mask + 1) / 2;
      if (pgsize >= 1024*1024)
	  sprintf (pgsizebuf, "%dM", pgsize/(1024*1024));
      else
	  sprintf (pgsizebuf, "%dK", pgsize/1024);

      sprintf (pb, "    Page Size = %4.4s ASID=0x%02x             v=0x%08x->plo0=0x%08x %s%s%s%s",
	       pgsizebuf,
	       (word)(tlb.tlb_hi & TLBHI_ASIDMASK),
	       va, pa0,
	       (tlb.tlb_lo0 & TLBLO_V) ? "V" : "v",
	       (tlb.tlb_lo0 & TLBLO_D) ? "D" : "d",
	       (tlb.tlb_lo0 & TLBLO_G) ? "G" : "g",
	       (((tlb.tlb_lo0 & TLBLO_CALGMASK) >> 3) == CFG_C_UNCACHED) ? "U" : "c");
      
      if (more (pb, l, siz))
	return 1;

      sprintf (pb, "                                           v=0x%08x->plo1=0x%08x %s%s%s%s",
	       va + pgsize, pa1,
	       (tlb.tlb_lo1 & TLBLO_V) ? "V" : "v",
	       (tlb.tlb_lo1 & TLBLO_D) ? "D" : "d",
	       (tlb.tlb_lo1 & TLBLO_G) ? "G" : "g",
	       (((tlb.tlb_lo0 & TLBLO_CALGMASK) >> 3) == CFG_C_UNCACHED) ? "U" : "c");
      if (more (pb, l, siz))
	return 1;
    }
#else
#endif
    
    return 0;
}


#ifdef TLBREGISTERS

void
disp_tlbentry (int ent)
{
    struct tlbentry tlb;
    int valid;
    sreg_t vaddr;
    word pa0, pa1;
    unsigned long mask, pgsize;
    char *b, *s;

    tlbread (ent, &tlb);

    b = prnbuf;

#ifdef R4000
    /* FIXME 64bit TLB entries */
    if (in_machtype (F_4100)) {
	mask = tlb.tlb_mask | 0x7ff;
	vaddr = tlb.tlb_hi & TLBHI_4100_VPN2MASK;
	pa0 = (tlb.tlb_lo0 & TLBLO_PFNMASK) << (10-6);
	pa1 = (tlb.tlb_lo1 & TLBLO_PFNMASK) << (10-6);
    }
    else {
	mask = tlb.tlb_mask | 0x1fff;
	vaddr = tlb.tlb_hi & TLBHI_VPN2MASK;
	pa0 = (tlb.tlb_lo0 & TLBLO_PFNMASK) << (12-6);
	pa1 = (tlb.tlb_lo1 & TLBLO_PFNMASK) << (12-6);
    }

    valid = (tlb.tlb_lo0 & TLBLO_V) || (tlb.tlb_lo1 & TLBLO_V);

    b += sprintf (b, "%c%2d: vpn=0x%08x", valid ? '*' : ' ', ent, 
		  (word)vaddr);

    b += sprintf (b, " asid=0x%02x", (word)(tlb.tlb_hi & TLBHI_ASIDMASK));

    pgsize = (mask + 1) / 2;
    if (pgsize >= 1024*1024)
	b += sprintf (b, " sz=%dM", pgsize/(1024*1024));
    else
	b += sprintf (b, " sz=%dK", pgsize/1024);

    b += sprintf (b, " 0x%08x %c%c%c%c",
		  pa0,
		  tlb.tlb_lo0 & TLBLO_V ? 'V':'v',
		  tlb.tlb_lo0 & TLBLO_D ? 'D':'d',
		  tlb.tlb_lo0 & TLBLO_G ? 'G':'g',
		  ((tlb.tlb_lo0 & TLBLO_CALGMASK) >> 3) == CFG_C_UNCACHED
		  ? 'U' : 'c');

    b += sprintf (b, " 0x%08x %c%c%c%c",
		  pa1,
		  tlb.tlb_lo1 & TLBLO_V ? 'V':'v',
		  tlb.tlb_lo1 & TLBLO_D ? 'D':'d',
		  tlb.tlb_lo1 & TLBLO_G ? 'G':'g',
		  ((tlb.tlb_lo1 & TLBLO_CALGMASK) >> 3) == CFG_C_UNCACHED 
		  ? 'U' : 'c');
#else
#endif    
}

void
disp_tlbentries (int ent)
{
    int             i, l, siz;
    int		    ntlb = NTLBENTRIES;

    switch (machtype) {
    case 4640:
    case 4650:
	/* no TLB */
	printf ("Processor does not have a TLB\n");
	return;
    }

    switch (machtype / 100) {
    case 41:
    case 42:
    case 43:
    case 323:
	ntlb = 32; break;
    }

    if (ent >= ntlb) {
	printf ("Invalid tlb entry number\n");
	return;
    }

    l = siz = moresz;

    if (ent >= 0) {
	disp_tlbentry (ent);
	more (prnbuf, &l, siz);
    }
    else {
	for (i = 0; i < ntlb; i++) {
	    disp_tlbentry (i);
	    if (more (prnbuf, &l, siz))
		return;
	}
    }
}

/*************************************************************
 *  tlbregisters(ac,av) 
 *      the 'tlb' (display tlb) command
 */
tlbregisters (ac, av)
     int             ac;
     char           *av[];
{
    int flag;
    uword reg;
    reg_t rn;

    ioctl (STDIN, CBREAK, NULL);

    switch (ac) {
    case 1:			/* display TLB registers */
	disp_tlbentries (-1);
	break;
    case 2:			/* display selected register(s) */
	if (strequ (av[1], "*")) {	/* all regs */
	    disp_tlbentries (-1);
	} else {		/* display reg(s) by name */
	    flag = 0;
	    if (!get_rsa (&reg, av[1])) {
		printf ("error: arg2 bad TLB register number\n");
	        return (1);
	    }
	    disp_tlbentries (reg);
	}
	break;
    default:
	return (1);
    }
    return (0);
}


#endif
