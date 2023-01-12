/*			V3pmon/pmon/pmon/hurrican/
 *
 * hurrican/sbdreset.S - low level board dependent startup code for PMON
 *
 * a36 16/12/1998 11:00a rh -delay 0.01 Sec before any access to SDRAM after set up.
 * a35 15/12/1998  4:30p rh -Add new code to do memory probing.
 * a34 04/12/1998  6:00p rh -Debug single bank DIMM.
 * a33 01/12/1998  9:30a rh -rid of debug inserts.
 * a32 30/11/1998 10:30a rh -@ spdGet(),check for valid operating frequency range.
 * a31 27/11/1998  4:30p rh -knit in Column Row Mux and total DIMM memory code.
 * a30 27/11/1998  9:30a rh -debug sbdreset(): do SPD,,write to BANKX.
 * a29 26/11/1998  6:30p rh -debug sbdreset(): do SPD,,write to BANKX.
 * a28 25/11/1998  9:30a rh -debug sbdreset(): do SPD,,write to BANKX.
 * a27 24/11/1998  9:00a rh -debug sbdreset(): skip SPD,do defaults,write to BANK3.
 * a26 23/11/1998  7:15p rh -debug sbdreset(): skip SPD,do defaults.
 * a25 23/11/1998 11:30a rh -debug sbdreset(): skip writing to the BANKX registers.
 * a24 20/11/1998 10:30a rh -debug sbdreset(): corrected offset fields in BANKX registers.
 * a23 19/11/1998  4:30p rh -debug sbdreset(). 5:00p
 * a22 19/11/1998  3:30p rh -fix restoration of final return address back to mips.S
 * a21 19/11/1998 12:50p rh -debug sbdreset(),problem around V3DISPLAY(0x2B)
 * a20 17/11/1998  3:30p rh -debug SPD for SDRAM,rid of loop reading SPD EEPROM,but keep pause.
 * a19 16/11/1998  4:30p rh -debug SPD for SDRAM
 * a19 16/11/1998  3:30p rh -debug SDRAM set up
 * a18 13/11/1998 11:30a rh -debug EEPROM access
 * a17 13/11/1998 10:00a rh -debug EEPROM access,try reading USER EEPROM right after writing USC
 * a16 12/11/1998  3:15p rh -put V3XDEBUG9811 loop at 2 places in spdGet()
 * a15 11/11/1998  5:00p rh -put V3DEBUG9811 loop at spdGet()
 * a14 11/11/1998  3:00p rh -Fixed section in spdGet
 * a13 11/11/1998 12:15p rh -knit in ROW/COLUMN determination and writes to LB_SDRAM_BASE of USC
 * a12 10/11/1998 12:00p rh -knit in SPD top level processing code.
 * a11 06/11/1998 12:20p rh -added #include "sdramSpd.S" to end of file
 * a10 28/10/1998  3:00p rh -added SPD(Serial Presence Detect) for SDRAM on DIMM.
 * a09 18/09/1998  9:00a rh -change SDRAMREFRESHTIME TO 0x06400000 FROM 0x02000000.
 * a08 15/09/1998 11:00a rh -omit write to USC IOS, take out RDELAY().
 * a07 14/09/1998  4:15p rh -use 32bit read/write,lw,sw in SDRAM_CFG.
 * a06 10/09/1998  2:45p rh -remove most of RDELAY().
 * a05 09/09/1998 12:00p rh -added RDELAY(),RLOOP() to add delay to better follow program execution
 * a04 08/09/1998  4:15p rh -added sdramInit: Initialize SDRAM controller in V320USC
 * a03 08/09/1998 		 rh -change #ifdef RTC to #ifdef HRTC
 * a02 05/09/1998 		 rh -added #ifdef/#endif RTC,crwrite() body empty, "not t9" in V3DISPLAY macro.
 * a01 26/08/1998 		 rh	-added macro V3DISPLAY( ),change all occurrence of DISPLAY()
 *                       	 redefined macro RTC_LOAD, RTC_STORE,
 */

#include <mips.h>
#include <pmon.h>
#include <sbd.h>
#include "v3uscreg.h"

#define DISPLAY(d0,d1,d2,d3)			\
	li	t8,PA_TO_KVA1(LED_BASE);	\
	li	t9,d0; 				\
	sw	t9,LED(0)(t8); 			\
	li	t9,d1; 				\
	sw	t9,LED(1)(t8);	 		\
	li	t9,d2;	 			\
	sw	t9,LED(2)(t8);	 		\
	li	t9,d3;	 			\
	sw	t9,LED(3)(t8)

  
#define RDELAYVALUE 100000

#define	RLOOP  \
3:	b	3b 	\
	nop

#define	RDELAY(dvalue)  \
	.set	noreorder;	.set	noat;	\
	li	AT,(dvalue);	\
1:	bnez	AT,1b;	\
	subu	AT,1;	\
	.set	at;	.set	reorder

#define	RDELAY2(dvalue)  \
	.set	noat;	\
	li	AT,(dvalue);	\
1:	bnez	AT,1b;	\
	subu	AT,1;	\
	.set	at

#define V3DISPLAY(d0)			\
	li	t8,PA_TO_KVA1(LED_BASE);	\
	li	t9,d0; 		\
	not	t9;			\
	sw	t9,(t8) 
				
#define V3DDISPLAY(d0)			\
	li	a1,PA_TO_KVA1(LED_BASE);	\
	li	a0,d0; 		\
	not	a0;			\
	sw	a0,(a1) 			

#define V3XDISPLAY(x)			\
	li	a1,PA_TO_KVA1(LED_BASE);	\
	move a0,x; 		\
	not	a0;		\
	sw	a0,(a1) 			

#define MEG	0x100000		

/* 
 * Include standard memory test/clear code.
 */
#define BUSWIDTH	4		/* 32-bit, non-interleaved */
#define MEMSTART	(32*MEG)	/* simm1 start */
#define MEMINCR		(8*MEG)		/* simm1 quanta */
#include "meminit.s"

#define	rtcptr	s2
#define	crptr	s3
#define	crval	s4
#define msize	s5
#define	rasave	s6
#define	RELOC	s8

#define RTC_STORE(rtcoffs,reg) \
 /*	li	t8,rtcoffs; \
	sw	t8,RTC_ADDR(rtcptr); \
	sw	reg,RTC_DATA(rtcptr)
  */
	.set noreorder
	nop
	nop
	.set reorder

#define RTC_LOAD(rtcoffs,reg) \
 /*	li	t8,rtcoffs; \
	sw	t8,RTC_ADDR(rtcptr); \
	lw	reg,RTC_DATA(rtcptr); \
	and	reg,0xff
  */
	.set noreorder
	nop
	nop
	.set reorder

LEAF(sbdreset)
	/* move	rasave,ra */
    /*	move RELOC,ra */

	move k1,ra

	/* if launched by ITROM, leave Config alone */
#ifndef ITBASE	
	/* set config register for 32b/32b cachelines, kseg0 cacheable */
	mfc0	t1,C0_CONFIG
	and	t1,~0x3f		# set bits 5..0 only
	or	t1,CFG_IB | CFG_DB | CFG_C_NONCOHERENT
	/* On the R4300 we also have control over the endianness */
#ifdef MIPSEB
	or	t1,CFG_BE	
#else
	and	t1,~CFG_BE	
#endif
	/* for a 4100 we also clear CFG_AD (other processors will ignore this write) */
	and	t1,~CFG_AD
	/* set DDDD rate for those processors that aren't configured by hardware */
	and	t1,~CFG_EPMASK
	or	t1,CFG_EPD
	
	mtc0	t1,C0_CONFIG
#endif	/* ITBASE */

	/* Turn OFF all LEDs */

	V3DISPLAY(0x00)
	RDELAY(RDELAYVALUE)

	/*
	 * slight amount of kludgery here to stop RAM resident
	 * program from overwriting itself...
	 */
	li	v1,0x1fc00000		/* check return address is in ROM */
	and	v0,ra,v1
	bne	v0,v1,.noinit

#ifdef ALGRP4032Q
	/* initialise board control register */
	li	crptr,PA_TO_KVA1(BCR_BASE)
	
	/* toggle V3 reset and make sure everything else is disabled */
	li	crval, BCR_V96X_ENABLE
	bal	crwrite
	move	crval, zero
	bal	crwrite
	
#ifdef MIPSEB
	li	crval,BCR_LED_ON|BCR_V96X_ENABLE
#else
	li	crval,BCR_LED_ON|BCR_V96X_ENABLE|BCR_IO_LE
#endif
	bal	crwrite

	li	rtcptr,PA_TO_KVA1(RTC_BASE)
	RTC_STORE(RTC_BCR, crval)

	/* initialise I/O registers */
1:	la	a0,reginittab
	addu	a0,RELOC
1:	lw	v0,0(a0)		# reg addr
	lw	v1,4(a0)		# reg data
	beqz	v0,2f
	sw	v1,0(v0)
	addu	a0,8
	b	1b
2:

#endif  /* ALGRP4032Q */

	/* skip memory size/clear if a soft reset */
	mfc0	t1,C0_STATUS
	and	t1,SR_SR
	bnez	t1,.noinit

	/*
	 * Determine DRAM bank arrangement and size
	 */
#define aces	t0
#define fives	t1
#define base	t2
#define tval	t3
#define simmmap	t4
#define   SIMM0_0	0x01	
#define   SIMM0_1	0x02
#define   SIMM1_0	0x04	
#define   SIMM1_1	0x08	
#define msize0	v0
#define msize1	v1


	
#ifdef V3USC

#define FREQDEFAULT50M 0x32

	.set noreorder	
spdGet:

				/* Get local processor operating frequency */
	li	s7,7			# init pseudo stack
	li	t8,0x4;			# User EEPROM, system device number 4
	li	s0,10			# location 10 in User EEPROM
	jal	I2C_EEPROMRead_amips
	nop

					/* check for valid frequency range */
	bgtu v0,49,1f		 # check for lower frequency limit 50(MHz)
	nop
	b	spdfreqdefault
	nop
1:
	bltu v0,76,spdfreqOK  # check for upper freqency limit of 75(MHz)
	nop
spdfreqdefault:
	li	v0,FREQDEFAULT50M
spdfreqOK:	
	move t7,v0			# save operating frequency

	li	s7,7
	li	t8,0x4
	li	s0,10
	move s1,v0
	jal	I2C_EEPROMWrite_amips
	nop

spdpeek:
				/* Read SPD EEPROM on DIMM */
	li	s7,7			# init pseudo stack
	li	t8,0x0			# system device number 0
	li	s0,2			#location 2 in SPD EEPROM
	jal	I2C_EEPROMRead_amips
	nop					# v0,returned with EEPROM data
						# t4,returned: 1(TRUE) if ACK received from EEPROM,
						#              0(FALSE) if ACK not received from EEPROM.


				/* check for ACK from EEPROM on DIMM */
				/* ACK not received, skip SPD,use default to set USC */
   	beqz t4,memSetDefaults  
	nop	
  
 /*	b	memSetDefaults
	nop	 
  */
					   		/* ACK received, go do SPD */
	jal SPD_mset_amips
	nop							# v0,returned with bits stuffed
	move t8,v0					# save stuffed bits

	b	writeUsc				/* SPD done, go update USC */
	li	t7,0xff					  #	t7 not zero, specifies that SPD was done
	.set reorder
/*		
 *
 *	sdramInit		V3 Semiconductor Sept,08,1998  Raymond H.
 *
 *	Initializations of the on-chip SDRAM Controller in V320USC
 *
 *	 incorporated into sbdreset.S (The board specific startup file for PMON)
 *
 */
 				/* Default: SPD EEPROM not found, set up USC with defaults */		
memSetDefaults:

#define	V3USC_SDRAM_MA_W	0x88		/* USCREG  SDRAM Bank select */
#define	V3USC_SDRAM_CMD_W	0x8a		/* USCREG; SDRAM Command */
#define	V3USC_SDRAM_CFG		0x8c		/* USCREG; SDRAM Configure */
#define	V3USC_IOS			0xf0		/* USCREG; IOS  */

#define USCERRATA			0x00554000	/* USC ERRATA value */
#define	SDRAMMA10SELECT		0x0400		/* SDRAM MA10 SELECTED */
#define	SDRAMCMDPRECHRG		0x00bf		/* 0x00b1,SDRAM COMMAND, CS3 */
#define	SDRAMMACASCYCLE		0x0020		/* SDRAM COMMAND, MA CAS CYCLE */
#define	SDRAMCMDAUTOREFR	0x00d1		/* SDRAM COMMAND, AUTO REFRESH */
#define	SDRAMCMDMODE		0x00ff		/* 0x00f1 SDRAM COMMAND, MODE */
#define	SDRAMREFRESHTIME	0x06400000	/* SDRAM REFRESH TIME VALUE */
#define	SDRAMAUTOLOOPCNT	0x08		/* SDRAM auto-refresh count */

#define SPD_CAS_LATENCY_MASK	0x3
#define SPD_CAS_LATENCY_SHIFT	0x4
#define	SDRAM_CFG_REF_NDIV_24	0x06000000	/* SDRAM refresh timer value of 0x18 shifted left 22 bits  */
#define	SDRAM_CFG_REF_NDIV_32	0x08000000	/* SDRAM refresh timer value of 0x20 shifted left 22 bits  */
#define	SDRAM_CFG_REF_NDIV_36	0x0c000000	/* SDRAM refresh timer value of 0x24 shifted left 22 bits  */


#define	SDRAM_BANKX_OFFSET_32M		0x06000000
#define	SDRAM_BANKX_OFFSET_64M		0x04000000
#define	SDRAM_BANKX_OFFSET_96M		0x02000000

#define	SDRAM_1G		1024
#define	SDRAM_512M		512
#define	SDRAM_256M		256
#define	SDRAM_128M		128
#define	SDRAM_64M		64
#define	SDRAM_32M		32
#define	SDRAM_16M		16
#define	SDRAM_8M		8
#define	SDRAM_4M		4
#define	SDRAM_2M		2
#define	SDRAM_1M		1
#define	SDRAM_512K		1

#define	SDRAM_BANKX_OFFSET1_256M	0x10000000
#define	SDRAM_BANKX_OFFSET1_128M	0x08000000
#define	SDRAM_BANKX_OFFSET1_64M		0x04000000
#define	SDRAM_BANKX_OFFSET1_32M		0x02000000
#define	SDRAM_BANKX_OFFSET1_16M		0x01000000
#define	SDRAM_BANKX_OFFSET1_8M		0x00800000
#define	SDRAM_BANKX_OFFSET1_4M		0x00400000
#define	SDRAM_BANKX_OFFSET1_2M		0x00200000
#define	SDRAM_BANKX_OFFSET1_1M		0x00100000
#define	SDRAM_BANKX_OFFSET1_512K	0x00080000


#define SDRAM_CFGREG_50M	\
	SDRAM_CFG_TCAS_RD_2 | \
	SDRAM_CFG_RCD_2 | \
	SDRAM_CFG_RP_2 | \
	SDRAM_CFG_RAS_3 | \
	SDRAM_CFG_DPL_1 & \
	~SDRAM_CFG_REF_SCALE_MASK | \
	SDRAM_CFG_REF_NDIV_24

#define SDRAM_CFGREG_66M	\
	SDRAM_CFG_TCAS_RD_2 | \
	SDRAM_CFG_RCD_2 | \
	SDRAM_CFG_RP_2 | \
	SDRAM_CFG_RAS_3 | \
	SDRAM_CFG_DPL_1 & \
	~SDRAM_CFG_REF_SCALE_MASK | \
	SDRAM_CFG_REF_NDIV_32

#define SDRAM_CFGREG_75M	\
	SDRAM_CFG_TCAS_RD_3 | \
	SDRAM_CFG_RCD_3 | \
	SDRAM_CFG_RP_3 | \
	SDRAM_CFG_RAS_4 | \
	SDRAM_CFG_DPL_1 & \
	~SDRAM_CFG_REF_SCALE_MASK | \
	SDRAM_CFG_REF_NDIV_36

	.set noreorder

f75Mhz:
	blt	t7,75,f66Mhz	
	nop

	/* li	t9,SDRAM_BASENSIZE */
	/* li	t6,SDRAM_BLKX_ROWCOLN */

	li	t8,SDRAM_CFGREG_75M		# get Default value for 75MHz
	li	t7,0					# specifies Default case
	b	writeUsc				# go initialize SDRAM and USC
	nop

f66Mhz:
	blt	t7,66,f50Mhz
	nop

	li	t8,SDRAM_CFGREG_66M		# get Default value for 66MHz
	li	t7,0					# specifies Default case
	b	writeUsc				# go initialize SDRAM and USC
	nop

f50Mhz:
	/*li	t8,SDRAM_CFGREG_50M	*/	# get Default value (0x06000541) for 50MHz 
	/* li	t8,0x06000541  */			# CAS(2),works for 64M,128M DIMM
	/* li	t8,0x06000942  */			# CAS(3),works for 32M,128M DIMM
	/* li	t8,0x06000941  */			# CAS(2),works for 128M DIMM
	/* li	t8,0x06000940  */			# CAS(1),none works
	li	t8,0x06000541				# CAS(2),RAS(3cycles)
	li	t7,0					# specifies Default case

	b	writeUsc				# go initialize SDRAM and USC
	nop

	.set reorder

	/*
	 * SDRAM Controller Initialization in USC320
	 *
	 */

	/* V3DISPLAY(0x20) */
	

	/* 
	 * The required parameters before executing the code under
	 *  label writeUsc: are: 
	 *		t6: (byte3) size of DIMM (read from SPD EEPROM @ byte #5 )
	 *			(byte2) number of column address( SPD @ byte #4)
	 *			(byte1) number of row    address( SPD @ byte #3)
	 *			(byte0) memory type				( SPD @ byte #2)
	 *		t8: bits stuffed according to requirements by USC SDRAM_CFG register
	 *		
	 *		t7: 0, defaults used,no SPD.
	 *			1, SPD used.
	 */
				/* First the SDRAM is initialized via the USC, */
				/*  then the relavent SDRAM control registers in the USC are setup. */
writeUsc:

	li	a1,PA_TO_KVA1(V3USC_BASE)	# get USC base address

	lw	t0,V3USC_LB_SDRAM_BASE(a1)	    # read LB_SDRAM_BASE reg of USC (@ offset 0x78)
	or	v0,t0,LB_SDRAM_BASE_ENABLE		# set Enable bit (bit0=1)
	sw	v0,V3USC_LB_SDRAM_BASE(a1)		# update LB_SDRAM_BASE reg; enables CPU access to SDRAM


				/* Write PreCharge command via SDRAM_MA and SDRAM_CMD */
					/* Select all banks with MA10 -0000 0100 0000 0000  */

	li	a0,SDRAMMA10SELECT			# get SDRAM bank select value 
	sh	a0,V3USC_SDRAM_MA_W(a1)		# write 16bits to MA__ register 
	
	.set noreorder

	li	a0,SDRAMCMDPRECHRG			# PreCharge command 
	sh	a0,V3USC_SDRAM_CMD_W(a1)	# write to Command Register (16bits) 
1:	lh	a0,V3USC_SDRAM_CMD_W(a1)	# read Command Register 
	and	a0,0x0001					# isolate bit 0 
	bnez a0,1b						# loop until bit 0 is zero 
	nop								# delay byte

				/* Write auto refresh command, send 8 times */
	
	li	a2,SDRAMAUTOLOOPCNT			# loop count, 8
2:	li	a0,SDRAMCMDAUTOREFR			# load auto refresh command
	sh	a0,V3USC_SDRAM_CMD_W(a1)	# write to SDRAM controller Command register

1:	lh	a0,V3USC_SDRAM_CMD_W(a1)	# read command register
	and	a0,0x0001					# isolate bit 0 
	bnez a0,1b						# loop until bit 0 is zero 
	nop								# delay byte

	subu	a2,1					# decrement loop count
	bnez	a2,2b					# repeat if loop count not zero
	nop

				/* Write CAS Latency to SDRAM MODE register */
  						
	and v0,t8,SPD_CAS_LATENCY_MASK	# extract the CAS Latency number from t8(bit 1,0)
	add v0,0x1						# formatted as required by MODE register
	sll	a0,v0,SPD_CAS_LATENCY_SHIFT	# shift left 4 bits for MODE register
	sh	a0,V3USC_SDRAM_MA_W(a1)		# MA CAS cycle

	li	a0,SDRAMCMDMODE				# MODE command
	sh	a0,V3USC_SDRAM_CMD_W(a1)	# write to SDRAM controller Command register

1:	lh	a0,V3USC_SDRAM_CMD_W(a1)	# read command register
	and	a0,0x0001					# isolate bit 0 
	bnez a0,1b						# loop until bit 0 is zero 
	nop								# delay byte

				/* Write SDRAM_CFG and start refresh timer */
  	
	sw	t8,V3USC_SDRAM_CFG(a1)		# write to SDRAM_CFG register of USC

				/* Skip if defaults used */
  
	beqz t7,memSetDefaultend	 
	/* b  memSetDefaultend		 */
	/* b	spdSdramBase		 */
	nop 

				/*  SPD path, continue setting up USC */
				/*    Update SDRAM_BLK3-0 regs (offset @ 0x90,0x94,0x98,0x9c) */
				/*    with ROW/COLUMN address multiplexing and its Enable bit set. */

spdRowColumn:
	li	s7,7				# init pseudo stack
	li	t8,0				# SPD EEPROM system device number 0
	li	s0,3				# SPD EEPROM byte #3,number ROW addresses
	jal	I2C_EEPROMRead_amips
	nop

	move t6,v0				# stuff into byte 0 of t6

	li	s0,4				# SPD EEPROM byte #4,number COLUMN addresses
	jal	I2C_EEPROMRead_amips
	nop


	sll	v1,v0,8				# stuff into byte 1 of t6
	or	t0,v1,t6
	move t6,t0

	li	s0,5				# SPD EEPROM byte #5,number of physical banks on DIMM
	jal	I2C_EEPROMRead_amips
	nop

	sll	v1,v0,16			# stuff into byte 2 of t6
	or	t0,v1,t6
	move t6,t0

	li	s7,7				# init pseudo stack
	li	t8,0				# SPD EEPROM system device number 0
	li	s0,31				# SPD EEPROM byte #31,module bank density
	jal	I2C_EEPROMRead_amips
	nop

	sll	v1,v0,24			# stuff into byte 3 of t6
	or	t0,v1,t6
	move t6,t0

					/* FIND Column Row Mux Code */
	and	v0,t6,0xff00		# retrieve number of column address (offset 4)
	srl	a0,v0,8

1:	add	t3,a0,2				# (t3): iFirstRowAddress = iColumn +2

2:	and	a0,t6,0xff			# retrieve number of row address (offset 3)
	add	t2,t3,a0			# (t2): iBS0Address = iFirstRowAddress + iRow

						/* find Column mode */

	add	v0,t2,-19			# iBS0Address -19
	sll	t7,v0,12			# t7  <- shift result left by 12

						/* find Row mode */

	add	v0,t3,-10			# iFirstRowAddress -10
	sll v1,v0,1				# multiply by 3
	add	t0,v1,v0			#  t0  <- partial 1

	sub	v1,t2,t3			# iBS0Address - iFirstRowAddress
	add v0,v1,-11			#  subtract 11
							#  v0  <- partial 2

	add	v1,t0,v0			# partial 1 + partial 2
	add	v0,v1,2				#  +2

	sll	t0,v0,8				# t0  <- shift result left by 8

					/* t7 contains Column Row Mux code */
	or	t7,t7,t0	


6:
					/* find SIZE and OFFSET values */ 
						/* FIND total memory on DIMM */
	and t0,t6,0xff0000		# retrieve number of banks on DIMM
	srl	v1,t0,16			# shift down to LSByte

	and v0,t6,0xff000000	# retrieve byte #31
	srl	t0,v0,24			# shift down to LSByte

	sll	v0,t0,2				# multiply by 4;each bit position is 4MBytes
	add	t0,v1,-1
	sll	t3,v0,t0			# multiply again by number of banks on DIMM

							/* t3 contains the total memory on the DIMM */

				/* should save total DIMM memory into USER EEPROM location #12 */

					/* set the LB_SDRAM_BASE register,with ENABLE bit=0 */
bnksdrambase:
	bne t3,SDRAM_1G,1f
	nop
	b	6f
	li	v0,LB_SDRAM_BASE_SIZE_1G

1:	
	bne	t3,SDRAM_512M,2f
	nop
	b	6f
	li	v0,LB_SDRAM_BASE_SIZE_512M

2:	
	bne	t3,SDRAM_256M,3f
	nop
	b	6f
	li	v0,LB_SDRAM_BASE_SIZE_256M

3:	
	bne	t3,SDRAM_128M,4f
	nop
	b	6f
	li	v0,LB_SDRAM_BASE_SIZE_128M

4:	
	bne	t3,SDRAM_64M,6f
	nop
	b	6f
	li	v0,LB_SDRAM_BASE_SIZE_64M

6:
	li	a1,PA_TO_KVA1(V3USC_BASE)	# get USC base address
	sw	v0,V3USC_LB_SDRAM_BASE(a1)	# write to LB_SDRAM_BASE register in USC	
	srl	t0,t3,v1			# divide total DIMM memory by number of banks on DIMM */	

bnksize:
	bne	t0,SDRAM_256M,1f
	nop
	or	v0,t7,SDRAM_BANKX_SIZE_256M		# set BANKX size bits
	or	t0,v0,SDRAM_BANKX_ENABLE		# set BANKX enable bit
	or	v0,t0,SDRAM_BANKX_OFFSET1_256M	# set BANK1 offset bit
	b	bnkwrt
	nop
1:
	bne	t0,SDRAM_128M,2f
	nop
	or	v0,t7,SDRAM_BANKX_SIZE_128M		# set BANKX size bits
	or	t0,v0,SDRAM_BANKX_ENABLE		# set BANKX enable bit
	or	v0,t0,SDRAM_BANKX_OFFSET1_128M	# set BANK1 offset bit
	b	bnkwrt
	nop
2:
	bne	t0,SDRAM_64M,3f
	nop
	or	v0,t7,SDRAM_BANKX_SIZE_64M		# set BANKX size bits
	or	t0,v0,SDRAM_BANKX_ENABLE		# set BANKX enable bit
	or	v0,t0,SDRAM_BANKX_OFFSET1_64M	# set BANK1 offset bit
	b	bnkwrt
	nop
3:
	bne	t0,SDRAM_32M,4f
	nop
	or	v0,t7,SDRAM_BANKX_SIZE_32M		# set BANKX size bits
	or	t0,v0,SDRAM_BANKX_ENABLE		# set BANKX enable bit
	or	v0,t0,SDRAM_BANKX_OFFSET1_32M	# set BANK1 offset bit
	b	bnkwrt
	nop
4:
	bne	t0,SDRAM_16M,5f
	nop
	or	v0,t7,SDRAM_BANKX_SIZE_16M		# set BANKX size bits
	or	t0,v0,SDRAM_BANKX_ENABLE		# set BANKX enable bit
	or	v0,t0,SDRAM_BANKX_OFFSET1_16M	# set BANK1 offset bit
	b	bnkwrt
	nop
5:
	bne	t0,SDRAM_8M,6f
	nop
	or	v0,t7,SDRAM_BANKX_SIZE_8M		# set BANKX size bits
	or	t0,v0,SDRAM_BANKX_ENABLE		# set BANKX enable bit
	or	v0,t0,SDRAM_BANKX_OFFSET1_8M	# set BANK1 offset bit
	b	bnkwrt
	nop
6:
	bne	t0,SDRAM_4M,7f
	nop
	or	v0,t7,SDRAM_BANKX_SIZE_4M		# set BANKX size bits
	or	t0,v0,SDRAM_BANKX_ENABLE		# set BANKX enable bit
	or	v0,t0,SDRAM_BANKX_OFFSET1_4M	# set BANK1 offset bit
	b	bnkwrt
	nop
7:
	bne	t0,SDRAM_2M,8f
	nop
	or	v0,t7,SDRAM_BANKX_SIZE_2M		# set BANKX size bits
	or	t0,v0,SDRAM_BANKX_ENABLE		# set BANKX enable bit
	or	v0,t0,SDRAM_BANKX_OFFSET1_2M	# set BANK1 offset bit
	b	bnkwrt
	nop
8:
	bne	t0,SDRAM_1M,9f
	nop
	or	v0,t7,SDRAM_BANKX_SIZE_1M		# set BANKX size bits
	or	t0,v0,SDRAM_BANKX_ENABLE		# set BANKX enable bit
	or	v0,t0,SDRAM_BANKX_OFFSET1_1M	# set BANK1 offset bit
	b	bnkwrt
	nop
9:
	or	v0,t7,SDRAM_BANKX_SIZE_512K		# set BANKX size bits
	or	t0,v0,SDRAM_BANKX_ENABLE		# set BANKX enable bit
	or	v0,t0,SDRAM_BANKX_OFFSET1_512K	# set BANK1 offset bit

bnkwrt:
	beq	v1,0x2,2f
	nop

	beq v1,0x1,1f
	nop
				/* 2 chip selects */
1:
	li	a1,PA_TO_KVA1(V3USC_BASE)	# get USC base address
	
	or	t7,v0,SDRAM_BANKX_ENABLE
	sw	t7,V3USC_SDRAM_BANK2(a1)	# write BANK2 register of USC

	and	v0,t7,~SDRAM_BANKX_OFFSET_MASK	# clear OFFSET field bits
	sw	v0,V3USC_SDRAM_BANK0(a1)	# write BANK0 register of USC
	
	li	v0,0x0
	sw	v0,V3USC_SDRAM_BANK1(a1)	# write 0 to BANK1 register of USC
	sw	v0,V3USC_SDRAM_BANK3(a1)	# write 0 to BANK3 register of USC

	b	6f
	nop
				/* 4 chip selects */
2:
	li	a1,PA_TO_KVA1(V3USC_BASE)	# get USC base address
	
	or	t7,v0,SDRAM_BANKX_ENABLE
	sw	t7,V3USC_SDRAM_BANK1(a1)	# write BANK1 register of USC

	and	v1,t7,SDRAM_BANKX_OFFSET_MASK	# v1 contains only the BANK1 OFFSET field bits
	move t0,v1
	not	t0
	and t7,t0,t7
	sll	v0,v1,1							# v0 contains only the BANK2 OFFSET field bits
	or	t0,v0,t7						
	sw	t0,V3USC_SDRAM_BANK2(a1)	# write BANK2 register of USC

	or	t0,v1,v0						# t0 contains only the BANK3 OFFSET field bits
	or	v0,t0,t7						
	sw	v0,V3USC_SDRAM_BANK3(a1)	# write BANK3 register of USC

	and	v0,t7,~SDRAM_BANKX_OFFSET_MASK	# clear OFFSET FIELD bits
	sw	v0,V3USC_SDRAM_BANK0(a1)	# write BANK0 register of USC

6:
	li	a1,PA_TO_KVA1(V3USC_BASE)	# get USC base address
	
	lw	t0,V3USC_LB_SDRAM_BASE(a1)	    # read LB_SDRAM_BASE reg of USC (@ offset 0x78)
	or	v0,t0,LB_SDRAM_BASE_ENABLE		# set Enable bit (bit0=1)
	sw	v0,V3USC_LB_SDRAM_BASE(a1)		# update LB_SDRAM_BASE reg; enables CPU access to SDRAM

		
				/* Finished setting up SDRAM controller in USC */
	b	memSetdone
	nop
				/* For DEFAULT case, just set Enable bit in LB_SDRAM_BASE reg in USC */
memSetDefaultend:
	li	a1,PA_TO_KVA1(V3USC_BASE)	# get USC base address
	lw	a0,V3USC_LB_SDRAM_BASE(a1)		# read LB_SDRAM_BASE reg of USC (@ offset 0x78)
	or	a0,0x1							# set Enable bit (bit0=1)
	sw	a0,V3USC_LB_SDRAM_BASE(a1)		# update LB_SDRAM_BASE reg; enables CPU access to SDRAM

	.set reorder

	/* V3DISPLAY(0x2f) */


 /* end of SdramInit: */
	 	
#endif	/* V3USC */
					/* finished setting memory controller on USC */
memSetdone:
					/* transfer return address */
  /* move rasave,k1	 */
					
	/* initialise DCR memory speed and type from option links */
	lw	crval,PA_TO_KVA1(OPTION_BASE)
	and	crval,DCR_SIMM1_DRAM|DCR_TYPE|DCR_DRAMFAST

	/* initialise registers */
	move	msize0,zero
	move	msize1,zero
	li	aces,0xaaaaaaaa
	not	fives,aces
	li	base,PA_TO_KVA1(0)
	
	/* make sure memory has started up properly */
	/* configure for 4Mb double sided and read at 4Mb intervals */
	or	crval,DCR_4MB
	/* DISPLAY ('D','C','R','I') */
	V3DISPLAY(0x21)

	li	crptr,PA_TO_KVA1(DCR_BASE)
	bal	crwrite
	
	/* DISPLAY ('M','E','M','I') */
	V3DISPLAY(0x22)
							/* V3DEBUG9811 */
/*	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
 */
						/* delay (10 milli Sec) before accessing SDRAM */
	li	a0,10000;	 	/* 0.01 Sec delay */
	bal	sbddelay
	

	li	tval,8
.initmem:
	sw	zero,+0*MEG(base)
	sw	zero,+4*MEG(base)
	sw	zero,+8*MEG(base)
	sw	zero,12*MEG(base)
	subu	tval,1
	bnez	tval,.initmem

	/* now detect the presense of SIMMS & their sides */
	/* DISPLAY ('S','L','O','T') */
	V3DISPLAY(0x23)
	
	sw	aces,+0*MEG(base)
	sw	aces,+4*MEG(base)
	sw	aces,+8*MEG(base)
	sw	aces,12*MEG(base)

	sw	fives,+0*MEG+4(base)
	sw	fives,+4*MEG+4(base)
	sw	fives,+8*MEG+4(base)
	sw	fives,12*MEG+4(base)

	sw	zero,+0*MEG+8(base)
	sw	zero,+4*MEG+8(base)
	sw	zero,+8*MEG+8(base)
	sw	zero,12*MEG+8(base)


	/* generate map of SIMM slots/sides */		
	move	simmmap,zero
	
	lw	tval,+0*MEG(base)
	bne	tval,aces,1f
	or	simmmap,SIMM0_0
1:
	lw	tval,+4*MEG(base)
	bne	tval,aces,1f
	or	simmmap,SIMM0_1
1:
	lw	tval,+8*MEG(base)
	bne	tval,aces,1f
	or	simmmap,SIMM1_0
1:
	lw	tval,+12*MEG(base)
	bne	tval,aces,1f
	or	simmmap,SIMM1_1
1:

	/* SIMM1 cannot be double sided if SIMM0 is single sided */
	and	tval,simmmap,SIMM0_1|SIMM1_1
	bne	tval,SIMM1_1,1f
	and	simmmap,~SIMM1_1
1:	
	
	/* any memory in SIMM0 ? */
	and	tval,simmmap,SIMM0_0
	beqz	tval,_sbd_memfail

	/* SIMM0 single sided ? */
	and	tval,simmmap,SIMM0_1
	bnez	tval,1f
	or	crval,DCR_SIMM0_SGL
1:		

	/* any memory in SIMM1 ? */
	and	tval,simmmap,SIMM1_0
	beqz	tval,.confsimms

	/* do we have DRAM configured? */
	and	tval,crval,DCR_SIMM1_DRAM
	bnez	tval,1f
	/* DISPLAY('F','L','S','H') */
	V3DISPLAY(0x24)

/* 2:	b	2b	 */
1:	

	/* SIMM1 single sided ? */
	and	tval,simmmap,SIMM1_1
	bnez	tval,.confsimms   # 1f
	or	crval,DCR_SIMM1_SGL

.confsimms:		
	
/*	 BLOCK START of PMON memory probe code not used
	
	/* now determine SIMM sizes * /
.confsimms:	
	/* start with max SIMM0: 16Mb, ds = 32Mb, ss=16Mb * /
	and	crval,~DCR_SIMM0_SIZE
	or	crval,DCR_16MB
	li	crptr,PA_TO_KVA1(DCR_BASE)
	/* DISPLAY ('D','C', 'R','0') * /
	V3DISPLAY(0x25)

	bal	crwrite

	/* DISPLAY ('S','I','M','0') * /
	V3DISPLAY(0x26)

	and	tval,simmmap,SIMM0_1
	beqz	tval,.chkss
	
.chkds:
	/* write data to wraparound points (mod 32Mb) * /
	li	tval,DCR_16MB
	sw	tval,0(base)
	li	tval,DCR_8MB
	sw	tval,+2*8*MEG(base)
	li	tval,DCR_4MB
	sw	tval,+2*4*MEG(base)
	b	1f

.chkss:	
	/* write data to wraparound points (mod 32Mb) * /
	li	tval,DCR_16MB
	sw	tval,0(base)
	li	tval,DCR_8MB
	sw	tval,+8*MEG(base)
	li	tval,DCR_4MB
	sw	tval,+4*MEG(base)
	/* b	1f * /
	
1:				
	sw	aces,4(base)		# kill bus capacitance
	
	/* read SIMM0 size * /
	lw	tval,0(base)
	
	/* see which it was * /
	bne	tval,DCR_16MB,1f
	li	msize0,2*16*MEG
	b	.gotsimm0

1:	bne	tval,DCR_8MB,1f
	li	msize0,2*8*MEG
	b	.gotsimm0
	
1:	bne	tval,DCR_4MB,_sbd_memfail	# error if not one of above
	li	msize0,2*4*MEG
	#b	.gotsimm0

	/* store the new SIMM0 size * /
.gotsimm0:
	and	crval,~DCR_SIMM0_SIZE
	or	crval,tval

	/* if SIMM0 is single sided then halve the detected size * /
	and	tval,simmmap,SIMM0_1
	bnez	tval,1f
	srl	msize0,1
1:		
	/* configure size field * /
	la	crptr,PA_TO_KVA1(DCR_BASE)
	/* DISPLAY ('D','C', 'R','1') * /
	V3DISPLAY(0x27)

	bal	crwrite
	
	/* if SIMM1 is FLASH prom, then skip the SIMM1 tests * /
	and	tval,crval,DCR_SIMM1_DRAM
	beqz	tval,.alldone

	/* if we did not detect SIMM1 then skip remainder * /
	and	tval,simmmap,SIMM1_0
	beqz	tval,.alldone
	
	/* DISPLAY ('S','I','M','1') * /
	V3DISPLAY(0x28)


	/*
	 * determine 2nd SIMM size by seeing where it wraps
	 * /
	addu	base,msize0
	li	tval,32*MEG
	sw	tval,0(base)
	li	tval,16*MEG
	sw	tval,16*MEG(base)
	li	tval,8*MEG
	sw	tval,8*MEG(base)
	li	tval,4*MEG
	sw	tval,4*MEG(base)
	li	tval,2*MEG
	sw	tval,2*MEG(base)
	li	tval,1*MEG
	sw	tval,1*MEG(base)
	
	lw	msize1,0(base)

    BLOCK END of PMON memory probe code not used
 */
 
/*		V3
 *  Code segment for memory probing.
 * 		With the USC, the wraparound in memory writes	
 *		are prevented; this renders the method used above
 *		to probe memory unsuitable.	The following method
 *		writes to boundry locations, then the locations
 *		are read back and compare against the values written.
 *      Data read back from locations where there are no memory
 *      will not compare with values written.
 */

#define	MEGR		(0x100000 - 4)

#define	DCR_1MBB	1
#define	DCR_4MBB	4
#define	DCR_8MBB	8
#define	DCR_12MBB	12
#define	DCR_16MBB	16
#define	DCR_24MBB	24
#define	DCR_32MBB	32
#define	DCR_40MBB	40
#define	DCR_48MBB	48
#define	DCR_56MBB	56
#define	DCR_64MBB	64
#define	DCR_72MBB	72
#define	DCR_80MBB	80
#define	DCR_88MBB	88
#define	DCR_96MBB	96
#define	DCR_104MBB	104
#define	DCR_112MBB	112
#define	DCR_120MBB	120
#define	DCR_128MBB	128

.chksdramwrt:
				/* Write data to TOP of 4M blocks from 0 to 16M */
				/*   then to TOP of 8M blocks from 24M to 128M	*/
	li	tval,DCR_1MBB
	sw	tval,0(base)
	li	tval,DCR_4MBB
	sw	tval,(4*MEG-4)(base)
	li	tval,DCR_8MBB
	sw	tval,(8*MEG-4)(base)
	li	tval,DCR_12MBB
	sw	tval,(12*MEG-4)(base)
	li	tval,DCR_16MBB
	sw	tval,(16*MEG-4)(base)
	li	tval,DCR_24MBB
	sw	tval,(24*MEG-4)(base)
	li	tval,DCR_32MBB
	sw	tval,(32*MEG-4)(base)
	li	tval,DCR_40MBB
	sw	tval,40*MEGR(base)
	li	tval,DCR_48MBB
	sw	tval,48*MEGR(base)
	li	tval,DCR_56MBB
	sw	tval,56*MEGR(base)
	li	tval,DCR_64MBB
	sw	tval,64*MEGR(base)
	li	tval,DCR_72MBB
	sw	tval,72*MEGR(base)
	li	tval,DCR_80MBB
	sw	tval,80*MEGR(base)
	li	tval,DCR_88MBB
	sw	tval,88*MEGR(base)
	li	tval,DCR_96MBB
	sw	tval,96*MEGR(base)
	li	tval,DCR_104MBB
	sw	tval,104*MEGR(base)
	li	tval,DCR_112MBB
	sw	tval,112*MEGR(base)
	li	tval,DCR_120MBB
	sw	tval,120*MEGR(base)
	li	tval,DCR_128MBB
	sw	tval,128*MEGR(base)

				/* Read back from SDRAM */
chksdramrd:
	lw	tval,0(base)
	beq	tval,DCR_1MBB,1f
	li	msize,0
	b	.gotsdram
	
1:	lw	tval,(4*MEG-4)(base)
	beq	tval,DCR_4MBB,1f
	li	msize,2
	b	.gotsdram

1:	lw	tval,(8*MEG-4)(base)
	beq	tval,DCR_8MBB,1f
	li	msize,4
	b	.gotsdram

1:	lw	tval,(12*MEG-4)(base)
	beq	tval,DCR_12MBB,1f
	li	msize,8
	b	.gotsdram

1:	lw	tval,(16*MEG-4)(base)
	beq	tval,DCR_16MBB,1f
	li	msize,12
	b	.gotsdram

1:	lw	tval,(24*MEG-4)(base)
	beq	tval,DCR_24MBB,1f
	li	msize,16
	b	.gotsdram

1:	lw	tval,(32*MEG-4)(base)
	beq	tval,DCR_32MBB,1f
	li	msize,24
	b	.gotsdram

1:	lw	tval,40*MEGR(base)
	beq	tval,DCR_40MBB,1f
	li	msize,32
	b	.gotsdram

1:	lw	tval,48*MEGR(base)
	beq	tval,DCR_48MBB,1f
	li	msize,32
	b	.gotsdram

1:	lw	tval,56*MEGR(base)
	beq	tval,DCR_56MBB,1f
	li	msize,48
	b	.gotsdram

1:	lw	tval,64*MEGR(base)
	beq	tval,DCR_64MBB,1f
	li	msize,56
	b	.gotsdram

1:	lw	tval,72*MEGR(base)
	beq	tval,DCR_72MBB,1f
	li	msize,64
	b	.gotsdram

1:	lw	tval,80*MEGR(base)
	beq	tval,DCR_80MBB,1f
	li	msize,72
	b	.gotsdram

1:	lw	tval,88*MEGR(base)
	beq	tval,DCR_88MBB,1f
	li	msize,80
	b	.gotsdram

1:	lw	tval,96*MEGR(base)
	beq	tval,DCR_96MBB,1f
	li	msize,88
	b	.gotsdram

1:	lw	tval,104*MEGR(base)
	beq	tval,DCR_104MBB,1f
	li	msize,96
	b	.gotsdram

1:	lw	tval,112*MEGR(base)
	beq	tval,DCR_112MBB,1f
	li	msize,104
	b	.gotsdram

1:	lw	tval,128*MEGR(base)
	beq	tval,DCR_128MBB,1f
	li	msize,112
	b	.gotsdram

1:	li	msize,128

.gotsdram:
	move t7,msize		/* save msize */
 		
.alldone:	

 /*	srl	tval,msize0,20
	srl	tval,msize1,20
	addu msize, msize0, msize1
	srl	msize,20
  */
						/* save memory size found in USER EEPROM @ byte #12	*/
	.set noreorder
	li	s7,7
	li	t8,0x4
	li	s0,12
	move s1,msize
	jal	I2C_EEPROMWrite_amips
	nop
	beqz s1,_sbd_memfail
	nop
	.set reorder
	
					/* transfer return address */
  	move rasave,k1	 

#ifdef	HRTC
	/* store SIMM0 size for later */
	srl	tval,msize0,20
	RTC_STORE(RTC_SIMM0SZ, tval)
	/* store SIMM1 size for later */
	srl	tval,msize1,20
	RTC_STORE(RTC_SIMM1SZ, tval)

	/* we must have got *some* memory by now */
	addu	msize, msize0, msize1
	beqz	msize,_sbd_memfail

	/* write the new DCR value */
	bal	crwrite
	RTC_STORE(RTC_DCR, crval)

	/* store memory size in rtc sram */
	srl	msize,20
	RTC_STORE(RTC_MEMSZ, msize)
#endif  /* HRTC */

	/* we can now initialise the caches for a fast clear_mem */
	/* DISPLAY ('C','A','C','H') */
	V3DISPLAY(0x29)
							/* V3DEBUG9811 */
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)

	jal	mips_init_cache

#if 0
	/* clear all of memory (to set correct parity) */
	/* DISPLAY ('Z','M','E','M') */
	V3DISPLAY(0x2A)

	la	t0,clear_mem_range
	addu	t0,RELOC
	move	a0,msize
	la	a1,PA_TO_KVA1(0)
	jal	t0
#endif

		
.noinit:

	/* initialise tlb */
	/* DISPLAY ('I','T','L','B') */
	V3DISPLAY(0x2B)

	bal	init_tlb

	/* initialise display */
	jal	sbd_dispinit
    
	 
	V3DISPLAY(0x2D)
							/* V3DEBUG9811 S */
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
	RDELAY(RDELAYVALUE)
							/* V3DEBUG9811 E */

	/* move ra,RELOC  */

	move ra,rasave 
	j	ra
END(sbdreset)


LEAF(_sbd_memfail)
	/* DISPLAY ('!','M','E','M') */
	V3DISPLAY(0x2C)

1:	b	1b
	j	ra
END(_sbd_memfail)


/* crwrite (base, val) 
 *	Write 8-bit <val> to 8 consecutive 1-bit registers, 
 *	starting at <base>.
 * Uses:	t8, t9
 */
SLEAF(crwrite)
/*	move	t9,crval
	li	t8,8
1:	.set	noat
	and	$at,t9,1
	sw	$at,0(crptr)
	.set	at
	srl	t9,1
	subu	t8,1
	addu	crptr,4
	bnez	t8,1b

	subu	crptr,8*4	# reset pointer
 */ 
	j	ra
SEND(crwrite)

		
#define INIT(addr,val) \
	.word	PA_TO_KVA1(addr), val

	.rdata

#ifdef ALGRP4032Q

reginittab:
	/* initial magic cycle for PCI bridge */
	INIT(V96XPBC_BASE+V96X_LB_IO_BASE, V96XPBC_BASE+V96X_LB_IO_BASE)
	/* led message */
	INIT(LED_BASE+LED(0), 'P')
	INIT(LED_BASE+LED(1), '4')
	INIT(LED_BASE+LED(2), '3')
	INIT(LED_BASE+LED(3), '2')
	/* program a 32kHz square wave from the RTC */
	INIT(RTC_BASE+RTC_ADDR, RTC_STATUSA)
	INIT(RTC_BASE+RTC_DATA, RTC_OSC_32KHz|RTC_RATE_NONE)
	INIT(RTC_BASE+RTC_ADDR, RTC_STATUSB)
	INIT(RTC_BASE+RTC_DATA, RTCSB_BINARY|RTCSB_24HR|RTCSB_SQWE)
	INIT(RTC_BASE+RTC_ADDR, RTC_INTR)			
	INIT(RTC_BASE+RTC_DATA, RTCIR_32KE)
	/* disable the combi chip configuration registers */
	INIT(EFER, EFER_DISABLE)
	/* initialise the z80pio chip, B channel */
	INIT(ZPIO_BASE+ZPIO_B_CTL, ZPIO_MODE_CTRL)	# control mode
	INIT(ZPIO_BASE+ZPIO_B_CTL, ZPIOB_E2_DO)		# input mask
	INIT(ZPIO_BASE+ZPIO_B_DAT, 0)			# initial value

#endif  /* ALGRP4032Q */
	
	.word	0,0


LEAF(sbdberrenb)
	mfc0	v0,C0_SR
	li	t0,SR_DE	
	bnez	a0,1f
	or	t1,v0,t0	# disable cache/parity errors (SR_DE = 1)
	b	2f
1:	not	t1,t0		# enable cache/parity errors (SR_DE = 0)
	and	t1,v0
2:	mtc0	t1,C0_SR
	and	v0,t0		# get old SR_DE bit
	xor	v0,t0		# and invert to make it an enable bit
	j	ra
END(sbdberrenb)


LEAF(sbdberrcnt)
	move	v0,zero
	j	ra
END(sbdberrcnt)

	.lcomm	wbfltmp,4

LEAF(wbflush)
XLEAF(mips_wbflush)
	sync
	la	t0,wbfltmp
	or	t0,K1BASE
	lw	zero,0(t0)
	j	ra
END(wbflush)


LEAF(sbddelay)
	li	t1,CACHEUS
	and	t0,ra,0x20000000
	beqz	t0,1f
	li	t1,ROMUS
1:	mul	a0,t1
	subu	a0,15		# approx number of loops so far

	.set	noreorder	
	.set	nomacro
	nop
2:	bgtz	a0,2b
	subu	a0,1
	.set	macro
	.set	reorder

	j	ra
END(sbddelay)


LEAF(mips_cycle)
	.set	noreorder	
	.set	nomacro
1:	bgtz	a0,1b
	subu	a0,1
	.set	macro
	.set	reorder
	j	ra
END(mips_cycle)

SLEAF(xSPD_mset_amips)
	.set	noreorder
	move	t7,ra	#save return address
				/* do SPD and set memory controller in USC */
				
	.set reorder
	move	ra,t7	#restore return address
	j	ra
SEND(xSPD_mset_amips)

#include "v3miai2c.S"
#include "sdramSpd.S"
