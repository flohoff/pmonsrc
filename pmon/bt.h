/*
 * Boot package information
 */

#define NPKG		8	/* number of supported packages */
#define PKGSHIFT	3	/* log2(NPKG) */

#ifdef BYTE_ORDER
#define BTMAGIC		((0xcafe << 16)|BYTE_ORDER)
#else
#ifdef MIPSEB
#define BTMAGIC	((0xcafe << 16)|4321)
#else
#define BTMAGIC	((0xcafe << 16)|1234)
#endif
#endif

#define oMAGIC		0
#define oSTART		4
#define oEND		8
#define oSUM		12
#define oENTRY		16

#ifndef __ASSEMBLER__
struct package {
  unsigned int    magic;
  unsigned int   *start;
  unsigned int   *end;
  unsigned int    sum;
  unsigned int    entry;
  unsigned int:   32;
  unsigned int:   32;
  unsigned int:   32;
};

#endif

/*
 * where boot package information is stored
 */
#define PACKAGEINFO	PA_TO_KVA1(0x1fc00400)
	
/* well known packages */
#define PKG_ITROM	6
#define PKG_PMON	7
