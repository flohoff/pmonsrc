/* $Id: boot.c,v 1.8 1998/09/20 23:53:53 chris Exp $ */
#include <sys/types.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <mips.h>
#include <netio.h>
#include <../pmon/pmon.h>
#include <../pmon/loadfn.h>

extern char *heaptop;

static int	bootseg;
static int	bootscheme;
static char	*bootpath;
static int	bootfd;
static int	bootbigend;

static unsigned long tablebase;

static int 
bootread (void *addr, int size)
{
    unsigned long limit;
    int i;

    if (bootscheme) {
	char addrbuf[100];
	sprintf (addrbuf, "%-4d 0x%08x/%d", bootseg++, addr, size);
	fprintf (stderr, "\r%-26s", addrbuf);
    }
    else {
	if (bootseg++ > 0)
	    fprintf (stderr, "\b + ");
	fprintf (stderr, "0x%x/%d ", addr, size);
    }

    if (!dl_checkloadaddr (addr, size, 1))
      return (-1);

    i = netread (bootfd, addr, size);
    if (i < size) {
	if (i >= 0)
	  fprintf (stderr, "\nread failed (corrupt object file?)");
	else
	  perror ("\nsegment read");
	return (-1);
    }
    return size;
}


static int 
bootclear (void *addr, int size)
{
    unsigned long limit;

    if (bootseg++ > 0)
      fprintf (stderr, "\b + ");
    fprintf (stderr, "0x%x/%d ", addr, size);

    if (!dl_checkloadaddr (addr, size, 1))
      return (-1);

    if (size > 0)
	bzero (addr, size);
    return size;
}


static void *
gettable (int offs, int size, char *name)
{
    extern char end[];
    unsigned long base;

    /* temporarily grab some of the top of memory to hold a table */
    base = (tablebase - size) & ~7;
    if (base < K0_TO_PHYS(dl_maxaddr)) {
	fprintf (stderr, "\nnot enough memory for %s table", base);
	return 0;
    }
    tablebase = base;

    if (IS_K0SEG (end))
      base = PHYS_TO_K0 (base);
    else
      base = PHYS_TO_K1 (base);

    if (netlseek (bootfd, offs, SEEK_SET) != offs || 
	netread (bootfd, (void *)base, size) != size) {
	fprintf (stderr, "\ncannot read %s table", name);
	return 0;
    }

    return (void *) base;
}



#include "./ecoff.h"
#include "./sym.h"
#include "./symconst.h"

static int
ecoffreadsyms (struct filehdr *fh)
{
    char *strtab;
    HDRR hdrr;
    EXTR extr;
    long offs;
    int i;

    if (netlseek (bootfd, fh->f_symptr, SEEK_SET) != fh->f_symptr ||
	netread (bootfd, &hdrr, sizeof(hdrr)) != sizeof(hdrr)) {
	perror ("\nsym header");
	return (0);
    }

    if (hdrr.magic != magicSym) {
	fprintf (stderr, "\nbad sym header");
	return (0);
    }

    if (bootseg++ > 0)
      fprintf (stderr, "\b + ");
    fprintf (stderr, "%d syms ", hdrr.iextMax);

    if (!(strtab = gettable (hdrr.cbSsExtOffset, hdrr.issExtMax, "string")))
      return (0);

    offs = hdrr.cbExtOffset;
    if (netlseek (bootfd, offs, SEEK_SET) != offs) {
	perror ("\nsymbol table");
	return (0);
    }

    for (i = 0; i < hdrr.iextMax; i++) {
	if (netread (bootfd, &extr, sizeof(extr)) != sizeof(extr)) {
	    perror ("\nsymbol table");
	    return (0);
	}
	if (extr.asym.iss >= hdrr.issExtMax) {
	    fprintf (stderr, "\ncorrupt string pointer");
	    return (0);
	}
	if (!newsym (strtab + extr.asym.iss, extr.asym.value)) {
	    fprintf (stderr, "\nonly room for %d symbols", i);
	    return (0);
	}
    }

    return (1);
}


static long
load_ecoff (char *buf, int *n, int flags)
{
    struct mipsexec {
	struct filehdr fh;
	struct aouthdr ah;
    } *mp;
    long seekto;
    
    mp = (struct mipsexec *)buf;
    if (sizeof (*mp) < *n) {
	if (netread (bootfd, buf+*n, sizeof(*mp) - *n) != sizeof(*mp) - *n)
	  return (-1);
	*n = sizeof (*mp);
    }
    
    if (!MIPS_OK(mp->fh) || N_BADMAG(mp->ah))
      return (-1);
    fprintf (stderr, "(ecoff)\n");

    if (flags & FFLAG) {
	switch (mp->fh.f_magic) {
	case MIPSEBMAGIC:
	case MIPS2EBMAGIC:
	case MIPS3EBMAGIC:
	    bootbigend = 1;
	    break;
	case MIPSELMAGIC:
	case MIPS2ELMAGIC:
	case MIPS3ELMAGIC:
	    bootbigend = 0;
	    break;
	}
	fprintf (stderr, "Cannot load flash from this format (yet)\n");
	return -2;
    }

    if (!(flags & YFLAG)) {
	seekto = N_TXTOFF(mp->fh, mp->ah);
	if (netlseek (bootfd, seekto, SEEK_SET) != seekto) {
	    fprintf (stderr, "seek failed (corrupt object file?)\n");
	    return (-1);
	}
	
	if (bootread((void *)mp->ah.text_start, mp->ah.tsize) != mp->ah.tsize)
	  return (-1);
	
	if (bootread((void *)mp->ah.data_start, mp->ah.dsize) != mp->ah.dsize)
	  return (-1);
	
	bootclear ((void *)mp->ah.bss_start, mp->ah.bsize);
    }

    if (!(flags & NFLAG) && mp->fh.f_symptr > seekto)
      ecoffreadsyms (&mp->fh);
    
    return mp->ah.entry;
}

#include "./elf.h"

static Elf32_Shdr *
elfgetshdr (Elf32_Ehdr *ep)
{
    Elf32_Shdr *shtab;
    unsigned size = ep->e_shnum * sizeof(Elf32_Shdr);

    shtab = (Elf32_Shdr *) malloc (size);
    if (!shtab) {
	fprintf (stderr,"\nnot enough memory to read section headers");
	return (0);
    }

    if (netlseek (bootfd, ep->e_shoff, SEEK_SET) != ep->e_shoff ||
	netread (bootfd, shtab, size) != size) {
	perror ("\nsection headers");
	free (shtab);
	return (0);
    }

    return (shtab);
}


static int
elfreadsyms (Elf32_Ehdr *eh, Elf32_Shdr *shtab)
{
    Elf32_Shdr *sh, *strh;
    Elf32_Sym *sym;
    char *strtab;
    int nsym, offs, size, i;

    for (sh = shtab, i = 0; i < eh->e_shnum; sh++, i++)
      if (sh->sh_type == SHT_SYMTAB)
	break;
    if (i >= eh->e_shnum)
      return (0);
    strh = &shtab[sh->sh_link];

    nsym = (sh->sh_size / sh->sh_entsize) - sh->sh_info;
    offs = sh->sh_offset + (sh->sh_info * sh->sh_entsize);
    size = nsym * sh->sh_entsize;

    if (bootseg++ > 0)
      fprintf (stderr, "\b + ");
    fprintf (stderr, "%d syms ", nsym);

    if (offs < strh->sh_offset)
      if (!(sym = gettable (offs, size, "symbol")))
	return (0);
    
    if (!(strtab = gettable (strh->sh_offset, strh->sh_size, "string")))
	return (0);

    if (offs > strh->sh_offset)
      if (!(sym = gettable (offs, size, "symbol")))
	return (0);

    for (i = 0; i < nsym; i++, sym++) {
	int type;

	dotik (4000, 0);
	if (sym->st_shndx == SHN_UNDEF || sym->st_shndx == SHN_COMMON)
	  continue;

	type = ELF_ST_TYPE (sym->st_info);
	if (type == STT_SECTION || type == STT_FILE)
	  continue;

	/* only use globals and functions */
	if (ELF_ST_BIND (sym->st_info) == STB_GLOBAL || type == STT_FUNC)
	  if (sym->st_name >= strh->sh_size) {
	      fprintf (stderr, "\ncorrupt string pointer");
	      return (0);
	  }
	  if (!newsym (strtab + sym->st_name, sym->st_value)) {
	      fprintf (stderr, "\nonly room for %d symbols", i);
	      return (0);
	  }
    }

    return (1);
}


static long
load_elf (char *buf, int *n, int flags)
{
    Elf32_Ehdr *ep;
    Elf32_Phdr *phtab = 0;
    Elf32_Shdr *shtab = 0;
    unsigned int nbytes;
    int i;

    ep = (Elf32_Ehdr *)buf;
    if (sizeof(*ep) > *n) {
	if (netread (bootfd, buf+*n, sizeof(*ep)-*n) != sizeof(*ep)-*n)
	  return (-1);
	*n = sizeof (*ep);
    }

    /* check header validity */
    if (ep->e_ident[EI_MAG0] != ELFMAG0 ||
	ep->e_ident[EI_MAG1] != ELFMAG1 ||
	ep->e_ident[EI_MAG2] != ELFMAG2 ||
	ep->e_ident[EI_MAG3] != ELFMAG3)
	return (-1);

    fprintf (stderr, "(elf)\n");

    {
	char *nogood = (char *)0;
	if (ep->e_ident[EI_CLASS] != ELFCLASS32)
	    nogood = "not 32-bit";
	else if (
#if BYTE_ORDER == BIG_ENDIAN
	    ep->e_ident[EI_DATA] != ELFDATA2MSB
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
	    ep->e_ident[EI_DATA] != ELFDATA2LSB
#endif
	    )
	    nogood = "incorrect endianess";
	else if (ep->e_ident[EI_VERSION] != EV_CURRENT)
	    nogood = "version not current";
	else if (
#define GREENHILLS_HACK
#ifdef GREENHILLS_HACK
	    ep->e_machine != 10 && 
#endif
	    ep->e_machine != EM_MIPS
	    )
	    nogood = "incorrect machine type";

	if (nogood) {
	    fprintf (stderr, "Invalid ELF: %s\n", nogood);
	    return -2;
	}
    }
	
    if (flags & FFLAG) {
	bootbigend = (ep->e_ident[EI_DATA] == ELFDATA2MSB);
	fprintf (stderr, "Cannot load flash from this format (yet)\n");
	return -2;
    }
    /* Is there a program header? */
    if (ep->e_phoff == 0 || ep->e_phnum == 0 ||
	ep->e_phentsize != sizeof(Elf32_Phdr)) {
	fprintf (stderr, "missing program header (not executable)\n");
	return (-2);
    }

    /* Load program header */
    nbytes = ep->e_phnum * sizeof(Elf32_Phdr);
    phtab = (Elf32_Phdr *) malloc (nbytes);
    if (!phtab) {
	fprintf (stderr,"\nnot enough memory to read program headers");
	return (-2);
    }

    if (netlseek (bootfd, ep->e_phoff, SEEK_SET) != ep->e_phoff || 
	netread (bootfd, (void *)phtab, nbytes) != nbytes) {
	perror ("program header");
	free (phtab);
	return (-2);
    }

    /*
     * From now on we've got no guarantee about the file order, 
     * even where the section header is.  Hopefully most linkers
     * will put the section header after the program header, when
     * they know that the executable is not demand paged.  We assume
     * that the symbol and string tables always follow the program 
     * segments.
     */

    /* read section table (if before first program segment) */
    if (!(flags & NFLAG) && ep->e_shoff < phtab[0].p_offset)
      shtab = elfgetshdr (ep);

    /* load program segments */
    if (!(flags & YFLAG)) {
	/* We cope with a badly sorted program header, as produced by 
	 * older versions of the GNU linker, by loading the segments
	 * in file offset order, not in program header order. */
	while (1) {
	    Elf32_Off lowest_offset = ~0;
	    Elf32_Phdr *ph = 0;

	    /* find nearest loadable segment */
	    for (i = 0; i < ep->e_phnum; i++)
	      if (phtab[i].p_type == PT_LOAD && phtab[i].p_offset < lowest_offset) {
		  ph = &phtab[i];
		  lowest_offset = ph->p_offset;
	      }
	    if (!ph)
	      break;		/* none found, finished */

	    /* load the segment */
	    if (ph->p_filesz) {
		if (netlseek (bootfd, ph->p_offset, SEEK_SET) != ph->p_offset) {
		    fprintf (stderr, "seek failed (corrupt object file?)\n");
		    if (shtab) free (shtab);
		    free (phtab);
		    return (-2);
		}
		if (bootread ((void *)ph->p_vaddr, ph->p_filesz) != ph->p_filesz) {
		    if (shtab) free (shtab);
		    free (phtab);
		    return (-2);
		}
	    }
	    if (ph->p_filesz < ph->p_memsz)
	      bootclear ((void *)ph->p_vaddr + ph->p_filesz, ph->p_memsz - ph->p_filesz);
	    ph->p_type = PT_NULL; /* remove from consideration */
	}
    }

    if (!(flags & NFLAG)) {
	/* read section table (if after last program segment) */
	if (!shtab)
	  shtab = elfgetshdr (ep);
	if (shtab) {
	    elfreadsyms (ep, shtab);
	    free (shtab);
	}
    }

    free (phtab);
    return (ep->e_entry);
}

/* read up to '\n' */
int
fdgets (char *buf, int *n, int maxn)
{
    int nb = 0;

    for (;;) {
	if (netread (bootfd, buf, 1) != 1)
	    break;
	if (*buf == '\n')
	    break;
	buf++;
	if (++nb >= maxn)
	    break;
    }
    *buf = '\0';
    if (n)
	*n += nb;
    return nb;
}


static long
load_cebin (char *buf, int *n, int flags)
{
    struct cesig {
	unsigned char sig[7];
    } *cep;
    struct imghdr {
	unsigned long addr;
	unsigned long size;
    } imghdr;
    struct rechdr {
	unsigned long addr;
	unsigned long size;
	unsigned long csum;
    } rechdr;
    unsigned long csum;
    unsigned char *s;

#if BYTE_ORDER == BIG_ENDIAN
    /* Will Microsoft ever produce a big endian OS? */
    return (-1);
#endif

    cep = (struct cesig *)buf;
    if (sizeof (*cep) > *n) {
	if (netread (bootfd, buf+*n, sizeof(*cep) - *n) != sizeof(*cep) - *n) {
	    return (-1);
	}
	*n = sizeof (*cep);
    }
    
    if (strncmp (cep->sig, "B000FF\n", 7) != 0) {
	return (-1);
    }

    fprintf (stderr, "(cebin)\n");

    if (flags & FFLAG) {
	bootbigend = 0;
	fprintf (stderr, "Cannot load flash from this format (yet)\n");
	return -2;
    }

    if (!(flags & YFLAG)) {

	if (netread(bootfd, (void *)&imghdr, sizeof(imghdr)) != sizeof(imghdr)) {
	    fprintf (stderr, " [imghdr]");
	    return (-1);
	}
	if (!dl_checkloadaddr ((void *)imghdr.addr, imghdr.size, 1))
	    return (-1);

	/* CE binaries consist of loads of little segments... */
	bootscheme = 1;
	for (;;) {
	    if (netread(bootfd, (void *)&rechdr, sizeof(rechdr)) != sizeof(rechdr)) {
		fprintf (stderr, " [rechdr]");
		return (-1);
	    }

	    if (rechdr.addr == 0)
		break;
	    
	    if (bootread((void *)rechdr.addr, rechdr.size) != rechdr.size) {
		fprintf (stderr, " [record]");
		return (-1);
	    }

	    csum = 0;
	    for (s = (unsigned char *)rechdr.addr;
		 s < (unsigned char *)(rechdr.addr+rechdr.size); 
		 s++)
		csum += *s;

	    if (csum != rechdr.csum) {
		fprintf (stderr, "\nchecksum failure: 0x%08x:0x%08x want 0x%x got 0x%x",
			 rechdr.addr, rechdr.addr+rechdr.size,
			 rechdr.csum, csum);
		return (-1);
	    }
	}
    }
    
    return rechdr.size;
}

static long
load_xx (char *buf, int *n, int flags, char *rectype, char recid,
	 int (*fn)(char *, int, int *, int))
{
    extern char *strnchr (char *, char, int);
    int s, len, nb;
    char *p = NULL; int pl = 0;
    int left;

    if (p = strnchr (buf, '\n', *n)) {
	/* We already have one (or more) lines */
	*p++ = '\0';
	nb = p - buf - 1;
	left = buf + *n - p;
    }
    else {
	/* get the rest of this line */
	if (fdgets (buf + *n, n, DLREC-*n) < 0)
	    return -1;
	nb = *n;
	left = 0;
    }

    if (*buf != recid)
	return (-1);
    fprintf (stderr, "(%s)\n", rectype);

    do {
    nextline:
	s = (*fn)(buf, nb, &len, flags);
	if (s != DL_CONT)
	    break;
	if (left) {
	    memcpy (buf, p, left);
	    if (p = strnchr (buf, '\n', left)) {
		*p++ = '\0';
		nb = p - buf - 1;
		left -= nb + 1;
		goto nextline;
	    }
	    nb = left;
	}
	else 
	    nb = 0;
    } while (fdgets (buf, &nb, DLREC) >= 0);

    return s == DL_DONE ? dl_entry : -2;
}

long
load_s3 (char *buf, int *n, int flags)
{
    return load_xx (buf, n, flags, "s3", 'S', dl_s3load);
}

long
load_fast (char *buf, int *n, int flags)
{
    int s, len, nb;

    return load_xx (buf, n, flags, "fast", '/', dl_fastload);
}



static long (* const loaders[])() = {
    load_cebin,
    load_elf,
    load_ecoff,
    load_s3,
    load_fast,
    0
};


const Optdesc         boot_opts[] =
{
    {"-s", "don't clear old symbols"},
    {"-b", "don't clear breakpoints"},
    {"-e", "don't clear exception handlers"},
    {"-a", "don't add offset to symbols"},
    {"-t", "load at top of memory"},
    {"-i", "ignore checksum errors"},
    {"-f", "load into flash"},
    {"-n", "don't load symbols"},
    {"-y", "only load symbols"},
    {"-v", "verbose messages"},
    {"-w", "reverse endianness"},
    {"-o<offs>", "load offset"},

    {"host:path", "internet host name, and file name"},

    {0}};


static
com_boot (argc, argv)
    int argc;
    char **argv;
{
    extern char *getenv();
    extern char *strchr();
    char *host, *file;
    int hostlen;
    char path[256];
    char buf[DLREC+1];
    long ep;
    int i, n;
    extern int optind;
    extern char *optarg;
    int c, err;
    int flags;
    unsigned long offset;
    void	    *flashbuf;
    int		    flashsize;

    flags = 0;
    optind = err = 0;
    offset = 0;
    while ((c = getopt (argc, argv, "sbeatifnyvwo:")) != EOF)
      switch (c) {
      case 's':
	  flags |= SFLAG; break;
      case 'b':
	  flags |= BFLAG; break;
      case 'e':
	  flags |= EFLAG; break;
      case 'a':
	  flags |= AFLAG; break;
      case 't':
	  flags |= TFLAG; break;
      case 'i':
	  flags |= IFLAG; break;
      case 'f':
	  flags |= FFLAG; break;
      case 'n':
	  flags |= NFLAG; break;
      case 'y':
	  flags |= YFLAG; break;
      case 'v':
	  flags |= VFLAG; break;
      case 'w':
	  flags |= WFLAG; break;
      case 'o':
	  if (!get_rsa (&offset, optarg))
	    err++;
	  break;

      default:
	  err++;
      }

    if (err || optind < argc - 1)
      return EXIT_USAGE;
    
    if (optind < argc) {
	if (file = strchr (argv[optind], ':')) {
	    host = argv[optind];
	    hostlen = file++ - host;
	} else {
	    host = NULL;
	    file = argv[optind];
	}
    }
    else {
	host = file = NULL;
    }

    if (!host || !*host) {
	host = getenv ("bootaddr");
	if (!host) {
	    fprintf (stderr, "missing host address and $bootaddr not set\n");
	    return EXIT_FAILURE;
	}
	hostlen = strlen (host);
    }

    if (!file || !*file) {
	file = getenv ("bootfile");
	if (!file) {
	    fprintf (stderr, "missing file name and $bootfile not set\n");
	    return EXIT_FAILURE;
	}
    }

    if (hostlen + strlen(file) + 2 > sizeof(path)) {
	fprintf (stderr, "remote pathname too long\n");
	return EXIT_FAILURE;
    }
    memcpy (path, host, hostlen);
    path[hostlen++] = ':';
    strcpy (&path[hostlen], file);

    if ((bootfd = netopen (path, O_RDONLY | O_NONBLOCK)) < 0) {
	perror (path);
	return EXIT_FAILURE;
    }

    bootpath = path;
    bootseg = 0;
    bootscheme = 0;
    tablebase = memorysize;

    if (flags & FFLAG) {
	sbd_flashinfo (&flashbuf, &flashsize);
	if (flashsize == 0) {
	    printf ("No flash on this target\n");
	    return 0;
	}
	/* any loaded program will be trashed... */
	flags &= ~(SFLAG | BFLAG | EFLAG);
	flags |= NFLAG;		/* don't bother with symbols */
	/*
	 * Override any offset given on command line.
	 * Addresses should be 0 based, so load them into the flash buffer.
	 */
	offset = (unsigned long) flashbuf;
#if #endian(big)
	bootbigend = 1;
#else
	bootbigend = 0;
#endif
    }

    dl_initialise (offset, flags);

    fprintf (stderr, "Loading file: %s ", bootpath);
    errno = 0;
    n = 0; 
    for (i = 0; loaders[i]; i++)
      if ((ep = (*loaders[i]) (buf, &n, flags)) != -1)
	break;

    netclose (bootfd);
    putc ('\n', stderr);

    if (ep == -1) {
	fprintf (stderr, "%s: boot failed\n", path);
	return EXIT_FAILURE;
    }

    if (ep == -2) {
	fprintf (stderr, "%s: invalid executable object file\n", path);
	return EXIT_FAILURE;
    }

    if (!(flags & (FFLAG|YFLAG))) {
	printf ("Entry address is %08x\n", ep);
	flush_cache (ICACHE);
	flush_cache (DCACHE);
	Epc = ep;
	if (!(flags & SFLAG))
	    dl_setloadsyms ();
    }
    if (flags & FFLAG) {
	extern long dl_minaddr;
	extern long dl_maxaddr;
	if (flags & WFLAG)
	    bootbigend = !bootbigend;
	sbd_flashprogram ((void *)dl_minaddr, 	   	/* address */
			  dl_maxaddr - dl_minaddr, 	/* size */
			  dl_minaddr - (long)flashbuf,  /* offset */
			  bootbigend);
    }
    return EXIT_SUCCESS;
}

#ifdef PROM
boot_cmd (argc, argv)
    int argc; char **argv;
{
    int ret;
    ret = spawn ("boot", com_boot, argc, argv);
    return (ret & ~0xff) ? 1 : (signed char)ret;
}
#endif
