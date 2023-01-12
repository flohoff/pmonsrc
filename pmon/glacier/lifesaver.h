/*
 * lifesaver.h: Lifesaver ASIC
 */

#ifdef __ASSEMBLER__

/* offsets from base register */
#if #endian(little)
#define LSVRW(x)		(x)
#define LSVRH(x)		(x)
#define LSVRB(x)		(x)
#else
#define LSVRW(x)		(x)
#define LSVRH(x)		((x)^2)
#define LSVRB(x)		((x)^3)
#endif

#else /* !__ASSEMBLER */

/* offsets from base pointer, this construct allows optimisation */
#define pLSVR	char * const _lsvrp = PA_TO_KVA1(LIFESAVER_BASE)

#if #endian(little)
#define LSVRW(x)		*(volatile unsigned long *)(_lsvrp + (x))
#define LSVRH(x)		*(volatile unsigned short *)(_lsvrp + (x))
#define LSVRB(x)		*(volatile unsigned char *)(_lsvrp + (x))
#else
#define LSVRW(x)		*(volatile unsigned long *)(_lsvrp + (x))
#define LSVRH(x)		*(volatile unsigned short *)(_lsvrp + ((x)^2))
#define LSVRB(x)		*(volatile unsigned char *)(_lsvrp + ((x)^3))
#endif

#endif /* __ASSEMBLER__ */

#define	LSVR_BIU_ID		LSVRW(0x00000)
#define LSVR_BIU_CFG		LSVRW(0x00004)
#define LSVR_BIU_FAULTSTAT	LSVRW(0x00008)
#define LSVR_BIU_FAULTADDR	LSVRW(0x0000c)
#define LSVR_BIU_TRACEPOIINT	LSVRW(0x00010)
#define LSVR_BIU_SWAN1		LSVRW(0x00020)
#define LSVR_BIU_SWAN2		LSVRW(0x00024)

#define LSVR_BIU_CFG_Q		0x80000000
#define LSVR_BIU_CFG_PCI	0x40000000
#define LSVR_BIU_CFG_V		0x20000000

#define	LSVR_MIU_BANKCFG	LSVRW(0x40000)
#define LSVR_MIU_BANKPROF	LSVRW(0x40040)
#define LSVR_MIU_CBAVAL		LSVRW(0x40080)
#define LSVR_MIU_REFCFG		LSVRW(0x40084)
#define LSVR_MIU_CBAINIT	LSVRW(0x40088)
#define LSVR_MIU_WATCH		LSVRW(0x40090)

#define LSVR_PCI_VENDOR		LSVRH(0x80000)
#define LSVR_PCI_DEVICE		LSVRH(0x80002)
#define LSVR_PCI_CMD		LSVRH(0x80004)
#define LSVR_PCI_STAT		LSVRH(0x80006)
#define LSVR_PCI_CC_REV		LSVRW(0x80008)
#define LSVR_PCI_HDR_CFG	LSVRW(0x8000c)	
#define LSVR_PCI_BASE0		LSVRW(0x80010)
#define LSVR_PCI_BASE1		LSVRW(0x80014)
#define LSVR_PCI_BASE2		LSVRW(0x80018)
#define LSVR_PCI_BASE3		LSVRW(0x8001c)
#define LSVR_PCI_BASE4		LSVRW(0x80020)
#define LSVR_PCI_BASE5		LSVRW(0x80024)
#define LSVR_PCI_BPARAM		LSVRW(0x8003c)


#define LSVR_PCI_CMD_FBB_EN		0x0200
#define LSVR_PCI_CMD_SERR_EN		0x0100
#define LSVR_PCI_CMD_WAITCYC_EN		0x0080
#define LSVR_PCI_CMD_PAR_EN		0x0040
#define LSVR_PCI_CMD_WRITEINV_EN	0x0010
#define LSVR_PCI_CMD_PAR_EN		0x0040
#define LSVR_PCI_CMD_WRITEINV_EN	0x0010
#define LSVR_PCI_CMD_SPECIAL_EN		0x0008
#define LSVR_PCI_CMD_MASTER_EN		0x0004
#define LSVR_PCI_CMD_MEM_EN		0x0002
#define LSVR_PCI_CMD_IO_EN		0x0001

#define LSVR_PCI_STAT_PAR_ERR		0x8000
/*#define LSVR_PCI_STAT_SYS_ERR		0x4000*/
#define LSVR_PCI_STAT_CONF_DONE		0x4000
#define LSVR_PCI_STAT_M_ABORT		0x2000
#define LSVR_PCI_STAT_T_ABORT		0x1000
#define LSVR_PCI_STAT_DEVSEL		0x0600
#define LSVR_PCI_STAT_PAR_REP		0x0100
#define LSVR_PCI_STAT_FAST_BACK		0x0080
#define LSVR_PCI_STAT_UDF		0x0040
#define LSVR_PCI_STAT_66MHZ		0x0020

#define LSVR_PCI_CC_REV_BASE_CLASS	0xff000000
#define LSVR_PCI_CC_REV_SUB_CLASS	0x00ff0000
#define LSVR_PCI_CC_REV_PROG_IF		0x0000ff00
#define LSVR_PCI_CC_REV_UREV		0x000000f0
#define LSVR_PCI_CC_REV_VREV		0x0000000f

#define LSVR_PCI_HDR_CFG_LT		0x0000ff00
#define LSVR_PCI_HDR_CFG_LT_SHIFT	8
#define LSVR_PCI_HDR_CFG_CLS		0x000000ff
#define LSVR_PCI_HDR_CFG_CLS_SHIFT	0


#define	LSVR_PCICONF_ADDR	LSVRW(0xf8)
#define LSVR_PCICONF_DATA	LSVRW(0xfc)


#define LSVR_PCICONF_ADDR_E	0x80000000
