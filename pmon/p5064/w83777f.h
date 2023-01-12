/*
 * w83777f.h: Winbond PC combi i/o chip
 * W83787F W83777F W83877F
 */

#define EFER_PORT	0x250
#define EFIR_PORT	0x251
#define EFDR_PORT	0x252

#define EFER_ENABLE	0x89
#define EFER_DISABLE	0x00

#define CR0_IDEEN	0x80	/* W837[78]7 only */
#define CR0_HADSEL	0x40	/* W837[78]7 only */
#define CR0_FDCEN	0x20	/* W837[78]7 only */
#define CR0_FADSEL	0x10	/* W837[78]7 only */
#define CR0_PRTMODS1	0x08
#define CR0_PRTMODS0	0x04
#define CR0_APD		0x02
#define CR0_IPD		0x01

#define CR1_ABCHG	0x80
#define CR1_PRTAS1	0x20	/* W837[78]7 only */
#define CR1_PRTAS0	0x10	/* W837[78]7 only */
#define CR1_URBS1	0x08	/* W837[78]7 only */
#define CR1_URAS1	0x04	/* W837[78]7 only */
#define CR1_URBS0	0x02	/* W837[78]7 only */
#define CR1_URAS0	0x01	/* W837[78]7 only */

#define CR2_CEA		0x01

#define CR3_PRTBEN	0x80	/* W837[78]7 only */
#define CR3_GMENL	0x40
#define CR3_EPPVER	0x20
#define CR3_GMODS	0x10
#define CR3_URAS2	0x08	/* W837[78]7 only */
#define CR3_URBS2	0x04	/* W837[78]7 only */
#define CR3_SUAMIDI	0x02
#define CR3_SUBMIDI	0x01

#define CR4_PRTPWD	0x80
#define CR4_GMPWD	0x40
#define CR4_URAPWD	0x20
#define CR4_URBPWD	0x10
#define CR4_PRTTRI	0x08
#define CR4_GMTRI	0x04
#define CR4_URATRI	0x02
#define CR4_URBTRI	0x01

#define CR6_OSCS2	0x40
#define CR6_SEL4FDD	0x20
#define CR6_FIPURDWN	0x10
#define CR6_FDCPWD	0x08
#define CR6_IDEPWD	0x04
#define CR6_FDCTRI	0x02
#define CR6_IDETRI	0x01

#define CR8_APDTMS1	0x80
#define CR8_APDTMS0	0x40
#define CR8_DISFDDWR	0x20
#define CR8_SWWP	0x10
#define CR8_MEDIA1	0x08
#define CR8_MEDIA0	0x04
#define CR8_BOOT1	0x02
#define CR8_BOOT0	0x01

#define CR9_PRTMODS2	0x80
#define CR9_LOCKREG	0x40
#define CR9_EN3MODE	0x20
#define CR9_CHIPID	0x0f
#define  CHIPID_W83777F	 0x7
#define  CHIPID_W83787F	 0x8
#define  CHIPID_W83877F	 0xa

#define CRA_PFDCACT	0x80
#define CRA_PEXTACT	0x40
#define CRA_PDIRHISOP	0x20
#define CRA_PDCHACT	0x10
#define CRA_PEXTADP	0x08
#define CRA_PEXTEPP	0x04
#define CRA_PEXTECP	0x02
#define CRA_PEXTECPP	0x01

#define CRB_ENIFCHG	0x08
#define CRB_IDENT	0x04
#define CRB_MFM		0x02
#define CRB_INVERTZ	0x01

#define CRC_TURA	0x80
#define CRC_TURB	0x40
#define CRC_HEFERE	0x20
#define CRC_URIRSEL	0x08
#define CRC_RX2INV	0x02
#define CRC_TX2INV	0x01

#define CRD_SIRTX1	0x80
#define CRD_SIRTX0	0x40
#define CRD_SIRRX1	0x20
#define CRD_SIRRX0	0x10
#define CRD_HDUPLX	0x08
#define CRD_IRMODE2	0x04
#define CRD_IRMODE1	0x02
#define CRD_IRMODE0	0x01

#define CR17_PRIOQOD	0x10

/* etc */






