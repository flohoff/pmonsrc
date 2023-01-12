/* $Id: dis.c,v 1.6 1999/12/08 12:08:49 nigel Exp $ */
#include "mips.h"
#include "pmon.h"
#include "stdio.h"
#include "termio.h"
#include "string.h"

#ifdef SABLE
#define TIMEOUT 10
#else
#define TIMEOUT 1000
#endif

#define comma()		strcat(dest,",")
#define rd()		strcat(dest,regname[(int)RD_(inst)])
#define rs()		strcat(dest,regname[(int)RS_(inst)]), rsvalue = DBGREG[(int)RS_(inst)]
#define rt()		strcat(dest,regname[(int)RT_(inst)]), rtvalue = DBGREG[(int)RT_(inst)]
#define fd()		strcat(dest,c1reg[(int)FD_(inst)])
#define fs()		strcat(dest,c1reg[(int)FS_(inst)])
#define ft()		strcat(dest,c1reg[(int)FT_(inst)])
#define c0()		strcat(dest,c0reg[(int)RD_(inst)])
#define c1()		strcat(dest,c1reg[(int)RD_(inst)])
#ifdef LR33020
#define c2()		strcat(dest,regs_c2d[(int)RD_(inst)])
#else
#define c2()		strcat(dest,regs_hw[(int)RD_(inst)])
#endif
#define cn()		strcat(dest,regs_hw[(int)RD_(inst)])
#define c0ft()		strcat(dest,c0reg[(int)RT_(inst)])
#define c1ft()		strcat(dest,c1reg[(int)RT_(inst)])
#define cnft()		strcat(dest,regs_hw[(int)RT_(inst)])
#define cc1()		strcat(dest,regs_hw[(int)RD_(inst)])
#ifdef LR33020
#define cc2()		strcat(dest,regs_c2c[(int)RD_(inst)])
#else
#define cc2()		strcat(dest,regs_hw[(int)RD_(inst)])
#endif

long            inst, cur_loc;

const char         * const *regname;	/* pointer to either regs_sw or regs_hw */

/* software register names */
const char           * const regs_sw[] =
{
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "s8", "ra"
};

/* hardware register names */
const char           * const regs_hw[] =
{
    "$0", "$1", "$2", "$3", "$4", "$5", "$6", "$7",
    "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15",
    "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
    "$24", "$25", "$26", "$27", "$28", "$29", "$30", "$31"
};

const char * const *  c0reg;		/* pointer to either regs_c0 or c1reg */

const char           * const regs_c0[] =
{
#ifdef R4000
    "C0_INX", "C0_RAND", "C0_TLBLO0", "C0_TLBLO1",
    "C0_CTEXT", "C0_PGMASK", "C0_WIRED", "$7",
    "C0_BADADDR", "C0_COUNT", "C0_TLBHI", "C0_COMPARE",
    "C0_SR", "C0_CAUSE", "C0_EPC", "C0_PRID",
    "C0_CONFIG", "C0_LLADDR", "C0_WATCHLO", "C0_WATCHHI", 
    "$20", "$21", "$22", "$23", "$24", "$25", 
    "C0_ECC", "C0_CACHERR", "C0_TAGLO", "C0_TAGHI", "C0_ERRPC", "$31"
#else
    "C0_INX", "C0_RAND", "C0_TLBLO", "C0_BPC",
    "C0_CTEXT", "C0_BDA", "$6", "C0_DCIC",
    "C0_BADADDR", "$9", "C0_TLBHI", "$11",
    "C0_SR", "C0_CAUSE", "C0_EPC", "C0_PRID",
    "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
    "$24", "$25", "$26", "$27", "$28", "$29", "$30", "$31"
#endif
};

const char           * const c1reg[] =
{
    "$f0", "$f1", "$f2", "$f3", "$f4", "$f5", "$f6", "$f7",
    "$f8", "$f9", "$f10", "$f11", "$f12", "$f13", "$f14", "$f15",
    "$f16", "$f17", "$f18", "$f19", "$f20", "$f21", "$f22", "$f23",
    "$f24", "$f25", "$f26", "$f27", "$f28", "$f29", "$f30", "$f31"
};

#ifdef LR33000
const char           * const regs_c2d[] =
{
    "C2_SCRSTART", "C2_SCRPITCH", "C2_HWCRSRSTART", "C2_HWCRSRCURR",
    "C2_SAMEXTENT", "C2_NXTDISP", "C2_CURRDISP", "C2_LINECOUNT",
    "C2_VBLKSIZE", "C2_SRCLINE", "C2_SRCCURR", "C2_SRCPITCH",
    "C2_DESTLINE", "C2_DESTCURR", "C2_DESTPITCH", "C2_GBLKSIZE",
    "C2_SERPULS", "C2_HLINTR", "C2_BLANKS", "C2_SYNCS",
    "C2_SYNCRESET", "C2_BLANKE", "C2_HWCRSR", "C2_VHWCONFIG",
    "C2_PSTXB", "C2_PSRCVB", "$26", "C2_PSTXBWE", "C2_SRCDATA",
    "C2_DESTDATA", "C2_LEFTMASK", "C2_RIGHTMASK"
};

const char           * const regs_c2c[] =
{
    "C2_SRCSKEW", "C2_SRCSHIFT", "C2_COLOR0", "C2_COLOR1",
    "C2_GCPCONTROL", "C2_RASTEROP", "C2_PLANEMASK", "$7", "$8",
    "C2_CONFIG", "$10", "$11", "$12", "$13", "$14", "$15", "$16",
    "$17", "$18", "$19", "$20", "$21", "$22", "$23", "C2_PSSTAT",
    "C2_PSCOMM", "$26", "C2_PSCOMMWE", "$28", "$29", "$30", "$31"
};

#endif

const Optdesc         l_opts[] =
{
    {"-b", "list only branches"},
    {"-c", "list only calls"},
    {"-t", "list trace buffer"},
    {"-r", "show register values with trace"},
    {0}};

/*************************************************************
 *  dis(ac,av)
 *      the 'l' (disassemble) command
 */
int rflag;			/* Wanting effective addresses for load/store instructions */
int rsvalue;			/* Computed by rs() macro for displaying load/store effective addrs */
int rtvalue;
int do_rt;
int do_rs;
  
dis (ac, av)
     int             ac;
     char           *av[];
{
    word            adr, siz, l;
    char            v;
    char            instr[20], *p;
    const char * const * q;
    int             bflag, cflag, i, j, n, tflag;
    static word last_adr;	/* For repeat of l command */
    word prev_adr;		/* For figuring print of a0-a3 after jal type inst. */

    rflag = bflag = cflag = n = tflag = 0;

    siz = moresz;
    adr = Epc;

    for (i = 1; i < ac; i++) {
	if (av[i][0] == '-') {
	    for (j = 1; av[i][j] != 0; j++) {
		switch (av[i][j]) {
		case 'b':
		    bflag = 1;
		    break;
		case 'c':
		    cflag = 1;
		    break;
		case 't':
		    tflag = 1;
		    n++;
		    break;
		case 'r':
		    rflag = 1;
		    break;
		default:
		    printf ("%c: unknown option\n", av[i][j]);
		    return (-1);
		}
	    }
	} else {
	    switch (n) {
	    case 0:
		if (!get_rsa (&adr, av[i]))
		    return (-1);
		break;
	    case 1:
		if (!get_rsa (&siz, av[i]))
		    return (-1);
		break;
	    default:
		printf ("%s: unknown option\n", av[i]);
		return (-1);
	    }
	    n++;
	}
    }

    if (repeating_cmd )
      adr = last_adr - 4;
    
    if (matchenv ("regstyle")) {
	regname = regs_sw;
	c0reg = regs_c0;
    } else {
	regname = regs_hw;
	c0reg = regs_hw;
    }

    ioctl (STDIN, CBREAK, NULL);

    if (tflag) {
	dispchist (n, siz);
	rflag = 0;
	return (0);
    }

    if (adr < K0BASE || adr >= K2BASE || (adr & 3)) {
	printf ("%08x: illegal instruction address\n", adr);
	rflag = 0;
	return (1);
    }

    if (n > 1 && siz == 1 && is_branch (adr))
      siz = 2;			/* include branch delay slot */
    l = siz;

    if (cflag || bflag)
	printf ("%s", searching);
    while (1) {
	/* Enable this if you want a 'label' at the start of each
	 * procedure.
	 * if (adr2sym(prnbuf,adr)) {
	 * strcat(prnbuf,":");
	 * if (more(prnbuf,&l,(n>1)?0:siz)) break;
	 * }
	 */
	if (cflag || bflag) {
	    int match;
	    if (cflag)
	      match = is_jal (adr);
	    else
	      match = is_branch (adr);
	    if (!match) {
		dotik (128, 0);
		adr += 4L;
		continue;
	    }
	    era_line (searching);
	}
	prev_adr = adr;
	adr = disasm (prnbuf, adr, load_word (adr));
	last_adr = adr;
	if (more (prnbuf, &l, (n > 1) ? 0 : siz))
	    break;
	if (rflag && ( is_jal( prev_adr ) ) )
	{
	    /*
	     * We're showing registers and we just displayed
	     * a JAL type instruction.  Show a0-a3
	     */

	    printf( "\t\t\t# a0=0x%x a1=0x%x a2=0x%x a3=0x%x\n",
		   (word)DBGREG[R_A0], (word)DBGREG[R_A1], (word)DBGREG[R_A2], (word)DBGREG[R_A3]);
	}
	
	if (cflag || bflag)
	    printf ("%s", searching);
    }
    rflag = 0;
    return (0);
}

/*************************************************************
 *  long disasm(dest,addr,bits)
 *      disassemble instruction 'bits'
 */
long 
disasm (dest, addr, bits)
     char           *dest;
     long            addr;
     long            bits;
{
    const DISTBL   *pt;
    char           *pc, tmp[40];
    long            v, v1, w;
    int             i;
#ifdef FLOATINGPT
    float *s_fs;		/* For getting at FS argument via float */
    float *s_ft;		/* For getting at FT argument via float */
    double *d_fs;		/* For getting at FS argument via double */
    double *d_ft;		/* For getting at FT argument via double */
    int *w_fs;			/* For getting at FS argument via binary fixed single */
    long long *l_fs;		/* For getting at FS argument via binary fixed long */
    int fpdis;			/* Actually show the floating point values switch */
#endif /* FLOATINGPT */

    inst = bits;
    if (regname == 0)
	regname = regs_sw;

    if (!adr2symoff (dest, addr, 12))
	sprintf (dest, "%08x", addr);
    sprintf (tmp, " %08x ", inst);
    strcat (dest, tmp);

    pt = get_distbl (inst);
    i = strlen (pt->str);
    strcat (dest, pt->str);
    do_rt = 0;
    do_rs = 0;

#ifdef FLOATINGPT
    /*
     * It is possible the floating point values are bogus for printing
     * so give user a way to not show them.
     */

    fpdis = matchenv ("fpdis");
    
    /*
     * Get pointers to various ways of looking at floating point operands
     * as part of the rflag display rather than having do do these casts
     * over and over
     */

    d_fs = (double *)&Fpr[(int)FS_(inst)];
    d_ft = (double *)&Fpr[(int)FT_(inst)];
    l_fs = (long long *)d_fs;
    s_fs = (float *)(((int *)d_fs)+1);
    s_ft = (float *)(((int *)d_ft)+1);
    w_fs = (int *)s_fs;
#endif /* FLOATINGPT */

    while (i++ < 8)
	strcat (dest, " ");
    switch (pt->type) {
    case FT_FS_FD_D:
	fd (), comma (), fs (), comma (), ft ();
#ifdef FLOATINGPT
	if (rflag )
	{
	    mkcomment( dest, "#", 0);
	    if (fpdis )
	    {
		if (dpdenorm( (struct IEEEdp *)d_fs ) )
		    sprintf (tmp, " fs=0.0 (dp denorm)");
		else if (dpnan( (struct IEEEdp *)d_fs ) )
		    sprintf (tmp, " fs=Nan");
		else
		    sprintf (tmp, " fs=%e", *d_fs);
		strcat (dest, tmp);
		if (dpdenorm( (struct IEEEdp *)d_ft ) )
		    sprintf (tmp, " ft=0.0 (dp denorm)");
		else if (dpnan( (struct IEEEdp *)d_ft ) )
		    sprintf (tmp, " ft=Nan");
		else
		    sprintf (tmp, " ft=%e", *d_ft);
	    }
	    else
	      sprintf (tmp, " fs=%llx ft=%llx", *d_fs, *d_ft);
	    strcat (dest, tmp);
	}
#endif /* FLOATINGPT */
	break;
    case FT_FS_FD_S:
	fd (), comma (), fs (), comma (), ft ();
#ifdef FLOATINGPT
	if (rflag )
	{
	    mkcomment( dest, "#", 0);
	    if (fpdis )
	    {
		if (spdenorm( (struct IEEEsp *)s_fs ) )
		    sprintf (tmp, " fs=0.0 (sp denorm)");
		else if (spnan( (struct IEEEsp *)s_fs ) )
		    sprintf (tmp, " fs=Nan");
		else
		    sprintf (tmp, " fs=%e", *s_fs);
		strcat (dest, tmp);
		if (spdenorm( (struct IEEEsp *)s_ft ) )
		    sprintf (tmp, " ft=0.0 (sp denorm)");
		if (spnan( (struct IEEEsp *)s_ft ) )
		    sprintf (tmp, " ft=Nan");
		else
		    sprintf (tmp, " ft=%e", *s_ft);
	    }		
	    else
	      sprintf (tmp, " fs=0x%x ft=0x%x", *s_fs, *s_ft);
	    strcat (dest, tmp);
	}
#endif /* FLOATINGPT */
	break;
    case FS_FD_D:
	fd (), comma (), fs ();
#ifdef FLOATINGPT
	if (rflag )
	{
	    mkcomment( dest, "#", 0);
	    if (fpdis )
		if (dpdenorm( (struct IEEEdp *)d_fs ) )
		    sprintf (tmp, " fs=0.0 (dp denorm)");
		else if (dpnan( (struct IEEEdp *)d_fs ) )
		    sprintf (tmp, " fs=Nan");
		else
		    sprintf (tmp, " fs=%e", *d_fs);
	    else
	      sprintf (tmp, " fs=%llx", *d_fs);
	    strcat (dest, tmp);
	}
#endif  /* FLOATINGPT */
	break;
    case FS_FD_S:
	fd (), comma (), fs ();
#ifdef FLOATINGPT
	if (rflag )
	{
	    mkcomment( dest, "#", 0);
	    if (fpdis )
		if (spdenorm( (struct IEEEsp *)s_fs ) )
		    sprintf (tmp, " fs=0.0 (sp denorm)");
		else if (spnan( (struct IEEEsp *)s_fs ) )
		    sprintf (tmp, " fs=Nan");
		else
		    sprintf (tmp, " fs=%e", *s_fs);
	    else
	      sprintf (tmp, " fs=0x%x", *s_fs);
	    strcat (dest, tmp);
	}
#endif /* FLOATINGPT */
	break;
    case FS_FD_W:
	fd (), comma (), fs ();
#ifdef FLOATINGPT
	if (rflag )
	{
	    mkcomment( dest, "# ", 0);
	    sprintf (tmp, "fs=0x%x", *w_fs);
	    strcat (dest, tmp);
	}
#endif /* FLOATINGPT */
	break;
    case FS_FD_L:
	fd (), comma (), fs ();
#ifdef FLOATINGPT
	if (rflag )
	{
	    mkcomment( dest, "# ", 0);
	    sprintf (tmp, "fs=%llx", *l_fs);
	    strcat (dest, tmp);
	}
#endif /* FLOATINGPT */
	break;
    case FT_FS_D:
	fs (), comma (), ft ();
#ifdef FLOATINGPT
	if (rflag )
	{
	    mkcomment( dest, "#", 0);
	    if (fpdis )
	    {
		if (dpdenorm( (struct IEEEdp *)d_fs ) )
		    sprintf (tmp, " fs=0.0 (dp denorm)");
		else if (dpnan( (struct IEEEdp *)d_fs ) )
		    sprintf (tmp, " fs=Nan");
		else
		    sprintf (tmp, " fs=%e", *d_fs);
		strcat (dest, tmp);
		if (dpdenorm( (struct IEEEdp *)d_ft ) )
		    sprintf (tmp, " ft=0.0 (dp denorm)");
		else if (dpnan( (struct IEEEdp *)d_ft ) )
		    sprintf (tmp, " ft=Nan");
		else
		    sprintf (tmp, " ft=%e", *d_ft);
	    }
	    else
	      sprintf (tmp, " fs=%llx ft=%llx", *d_fs, *d_ft);
	    strcat (dest, tmp);
	}
#endif /* FLOATINGPT */
	break;
    case FT_FS_S:
	fs (), comma (), ft ();
#ifdef FLOATINGPT
	if (rflag )
	{
	    mkcomment( dest, "#", 0);
	    if (fpdis )
	    {
		if (spdenorm( (struct IEEEsp *)s_fs ) )
		    sprintf (tmp, " fs=0.0 (sp denorm)");
		if (spnan( (struct IEEEsp *)s_fs ) )
		    sprintf (tmp, " fs=Nan");
		else
		    sprintf (tmp, " fs=%e", *s_fs);
		strcat (dest, tmp);
		if (spdenorm( (struct IEEEsp *)s_ft ) )
		    sprintf (tmp, " ft=0.0 (sp denorm)");
		else if (spnan( (struct IEEEsp *)s_ft ) )
		    sprintf (tmp, " ft=Nan");
		else
		    sprintf (tmp, " ft=%e", *s_ft);
	    }		
	    else
	      sprintf (tmp, " fs=0x%x ft=0x%x", *s_fs, *s_ft);
	    strcat (dest, tmp);
	}
#endif /* FLOATINGPT */
	break;
    case RT_RS_IMM:
	rt (), comma (), rs (), comma (), imm (dest);
	if (rflag )
	{
	    sprintf (tmp, " rs=0x%x", rsvalue);
	    strcat (dest, tmp);
	}
	break;
    case RT_RS_SIMM:
	rt (), comma (), rs (), comma (), simm (dest);
	if (rflag )
	{
	    sprintf (tmp, " rs=0x%x", rsvalue);
	    strcat (dest, tmp);
	}
	break;
    case RT_IMM:
	rt (), comma (), imm (dest);
	break;
    case RT_SIMM:
	rt (), comma (), simm (dest);
	break;
    case RS_SIMM:
	rs (), comma (), simm (dest);
	if (rflag )
	{
	    sprintf (tmp, " rs=0x%x", rsvalue);
	    strcat (dest, tmp);
	}
	break;
    case RT_RD:
	rt (), comma (), rd();
	break;
    case RT_RD_TO:
	rt (), comma (), rd();
	if (rflag )
	    mkcomment( dest, "# rt=0x%x", rtvalue);
	break;
    case RD:
	rd ();
	break;
    case RD_RS:
	rd (); comma (); rs ();
	if (rflag )
	    mkcomment( dest, "# rs=0x%x", rsvalue);
	break;
    case RT_C0:
	rt (), comma (), c0 ();
	break;
    case RT_C0_TO:
	rt (), comma (), c0 ();
	if (rflag )
	    mkcomment( dest, "# rt=0x%x", rtvalue);
	break;
    case RT_C1:
	rt (), comma (), c1 ();
	break;
    case RT_C1_TO:
	rt (), comma (), c1 ();
	if (rflag )
	    mkcomment( dest, "# rt=0x%x", rtvalue);
	break;
    case RT_C2:
	rt (), comma (), c2 ();
	break;
    case RT_CN:
	rt (), comma (), cn ();
	break;
    case RT_CN_TO:
	rt (), comma (), cn ();
	if (rflag )
	    mkcomment( dest, "# rt=0x%x", rtvalue);
	break;
    case RT_CC1:
	rt (), comma (), cc1 ();
	break;
    case RT_CC1_TO:
	rt (), comma (), cc1 ();
	if (rflag )
	    mkcomment( dest, "# rt=0x%x", rtvalue);
	break;
    case RT_CC2:
	rt (), comma (), cc2 ();
	break;
    case RD_RT_RS:
	rd (), comma (), rt (), comma (), rs();
	if (rflag )
	{
	    mkcomment( dest, "# rt=0x%x", rtvalue);
	    sprintf (tmp, " rs=0x%x", rsvalue);
	    strcat (dest, tmp);
	}
	break;
    case JR:
    case RS:
	rs ();
	if (rflag )
	    mkcomment( dest, "# rs=0x%x", rsvalue);
	break;
    case RD_RS_RT:
	rd (), comma ();
    case RS_RT:
	rs (), comma (), rt ();
	if (rflag )
	{
	    mkcomment( dest, "# rs=0x%x ", rsvalue);
	    sprintf (tmp, " rt=0x%x", rtvalue);
	    strcat (dest, tmp);
	}
	break;
    case RD_RT:
	rd (), comma (), rt ();
	if (rflag )
	    mkcomment( dest, "# rt=0x%x", rtvalue);
	break;
    case RD_RT_SFT:
	rd (), comma (), rt (), comma ();
	sprintf (tmp, "0x%x", SHAMT_ (inst));
	strcat (dest, tmp);
	mkcomment (dest, "# %d", SHAMT_ (inst));
	if (rflag )
	{
	    sprintf (tmp, " rt=0x%x", rtvalue);
	    strcat (dest, tmp);
	}
	break;
    case RS_RT_OFF:
    case RS_OFF:
	rs (), comma ();
	do_rs = 1;
	if (pt->type == RS_RT_OFF)
	{
	    rt (), comma ();
	    if (rflag )
		do_rt = 1;
	}
    case CP_OFF:
    case OFF: off:
	v = IMM_ (inst);
	if (v & (1L << 15))
	    v |= 0xffff0000L;
	v1 = addr + 4L + (v << 2);
	if (!adr2symoff (tmp, v1, 0))
	    sprintf (tmp, "%x", v1);
	strcat (dest, tmp);
	mkcomment (dest, "# 0x%08x", v1);
	if (rflag && do_rs )
	{
	    sprintf (tmp, " rs=0x%x", rsvalue);
	    strcat (dest, tmp);
	    do_rs = 0;
	}
	if (rflag && do_rt )
	{
	    sprintf (tmp, " rt=0x%x", rtvalue);
	    strcat (dest, tmp);
	    do_rt = 0;
	}
	break;
    case BPCODE:
	sprintf (tmp, "%d", (inst >> 16) & 0x3ff);
	strcat (dest, tmp);
	break;
    case COFUN:
	sprintf (tmp, "0x%x", inst & 0x01ffffffL);
	strcat (dest, tmp);
	break;
    case NONE:
	break;
    case TARGET:
	v = (inst & 0x03ffffffL) << 2;
	v |= (addr & 0xf0000000L);
	if (!adr2symoff (tmp, v, 0))
	    sprintf (tmp, "%x", v);
	strcat (dest, tmp);
	mkcomment (dest, "# 0x%08x", v);
	break;
    case JALR:
	if (RD_ (inst) != 31L)
	    rd (), comma ();
	rs ();
	if (rflag )
	    mkcomment( dest, "# rs=0x%x", rsvalue);
	break;
    case LDSTC0:
	v = IMM_ (inst);
	if (v & (1L << 15))
	    v |= 0xffff0000L;
	c0ft (), comma ();
	sprintf (tmp, "%d", v);
	strcat (dest, tmp);
	strcat (dest, "(");
	rs ();
	strcat (dest, ")");
	mkcomment (dest, "# 0x%x", v);
	if (rflag )
	{
	    /* If wanting register contents, then add this too */
	    sprintf (tmp, " addr=0x%x", (int)(v + rsvalue));
	    strcat (dest, tmp);
	}
	break;
    case LDSTC1:
    case STOREC1:
	v = IMM_ (inst);
	if (v & (1L << 15))
	    v |= 0xffff0000L;
	c1ft (), comma ();
	sprintf (tmp, "%d", v);
	strcat (dest, tmp);
	strcat (dest, "(");
	rs ();
	strcat (dest, ")");
	mkcomment (dest, "# 0x%x", v);
	if (rflag )
	{
	    /* If wanting register contents, then add this too */
	    sprintf (tmp, " addr=0x%x", (int)(v + rsvalue));
	    strcat (dest, tmp);
#ifdef FLOATINGPT
	    if (pt->type == STOREC1 )
	    {
	      if (fpdis) {
		if (dpdenorm( (struct IEEEdp *)d_ft ))
		    sprintf (tmp, " ft=0.0 (dp denorm)");
		else if (dpnan ((struct IEEEdp *)d_ft))
		    sprintf (tmp, " ft=Nan");
		else
		    sprintf (tmp, " ft=%e", *d_ft);
	      }
	      else
		sprintf (tmp, " rt=0x%llx", Fpr[(int)RT_(inst)]);
	      strcat (dest, tmp);
	    }
#endif /* FLOATINGPT */
	}
	break;
    case LDSTCN:
    case STORECN:
	v = IMM_ (inst);
	if (v & (1L << 15))
	    v |= 0xffff0000L;
	cnft (), comma ();
	sprintf (tmp, "%d", v);
	strcat (dest, tmp);
	strcat (dest, "(");
	rs ();
	strcat (dest, ")");
	mkcomment (dest, "# 0x%x", v);
	if (rflag )
	{
	    /* If wanting register contents, then add this too */
	    sprintf (tmp, " addr=0x%x", (int)(v + rsvalue));
	    strcat (dest, tmp);
	}
	break;
    case LOAD_STORE:
    case STORE:
	rt (), comma ();
    load_store:
	v = IMM_ (inst);
	if (v & (1L << 15))
	    v |= 0xffff0000L;
	sprintf (tmp, "%d", v);
	strcat (dest, tmp);
	strcat (dest, "(");
	rs ();
	strcat (dest, ")");
	if (rflag )
	{
	    /* If wanting register contents, then add this too */
	    mkcomment( dest, "# addr=0x%x", (int)(v + rsvalue));
	    if (pt->type == STORE )
	    {
		sprintf (tmp, " rt=0x%x", (int)rtvalue);
		strcat (dest, tmp);
	    }
	}
	else
	mkcomment (dest, "# 0x%x", v);
	break;
    case CACHE_OP:
	sprintf (tmp, "%d,", RT_(inst));
	strcat (dest, tmp);
	goto load_store;
    case WORD:
	sprintf (tmp, "%08x", inst);
	strcat (dest, tmp);
	strcat (dest, "      # ");
	w = addr;
	for (i = 0; i < 4; i++) {
	    v = load_byte (w++);
	    if (isprint (v))
		strccat (dest, v);
	    else
		strccat (dest, '.');
	}
	break;
    }
    return (addr + 4L);
}

/*************************************************************
 *  simm(dest)
 *      signed immediate value
 */
simm (dest)
     char           *dest;
{
    char            tmp[20];
    long            v;

    v = IMM_ (inst);
    sprintf (tmp, "0x%x", v);
    strcat (dest, tmp);
    if (v & (1L << 15))
	v |= 0xffff0000L;
    mkcomment (dest, "# %d", v);
}

/*************************************************************
 *  imm(dest)
 *      unsigned immediate value
 */
imm (dest)
     char           *dest;
{
    char            tmp[20];
    long            v;

    v = IMM_ (inst);
    sprintf (tmp, "0x%x", v);
    strcat (dest, tmp);
    mkcomment (dest, "# %d", v);
}

/*************************************************************
 *  mkcomment(p,fmt,v)
 *      generate an appropriate comment
 */
mkcomment (p, fmt, v)
     char           *p, *fmt;
     long            v;
{
    char            tmp[20];
    int             n;

    for (n = 50 - strlen (p); n > 0; n--)
	strcat (p, " ");
    sprintf (tmp, fmt, v);
    strcat (p, tmp);
}

/*************************************************************
 *  dispchist(args,siz)
 *      display the pc history (trace buffer)
 */
dispchist (args, siz)
     int             args, siz;
{
    int             i, l;
    unsigned long   adr;

    l = siz;
    for (i = 0;; i++) {
	adr = getpchist (i);
	if (adr == 0)
	    break;
	disasm (prnbuf, adr, load_word (adr));
	if (more (prnbuf, &l, (args > 1) ? 0 : siz))
	    break;
    }
}
