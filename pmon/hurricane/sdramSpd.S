/*********************************************************************
 **
 * Module: 
 **
 * File Name: sdramSpd.S
 **
 * Authors: Raymond Hong
 **
 * Copyright (c) 1997-1997 V3 Semiconductor. All rights reserved.
 **
 * V3 Semiconductor makes no warranties for the use of its products.  V3 does
 * not assume any liability for errors which may appear in these files or
 * documents, however, we will attempt to notify customers of such errors.
 *
 * V3 Semiconductor retains the right to make changes to components,
 * documentation or specifications without notice.
 *
 * Please verify with V3 Semiconductor to be sure you have the latest
 * specifications before finalizing a design.
 **
 * $Revision: 1.1 $rh $Date: 1999/04/22 14:46:03 $Rid of debug inserts.
 * $Revision: 1.1 $rh $Date: 1999/04/22 14:46:03 $skip check of CAS(1)
 * $Revision: 1.1 $rh $Date: 1999/04/22 14:46:03 $rid of some pause at 0x3f and 0x5f.
 * $Revision: 1.1 $rh	$Date: 1999/04/22 14:46:03 $rid of some pause.
 * $Revision: 1.1 $rh	$Date: 1999/04/22 14:46:03 $use s2 as temp variable for stuffed bits.
 * $Revision: 1.1 $rh	$Date: 1999/04/22 14:46:03 $must load v1 properly before calling spdSaveCas().
 * $Revision: 1.1 $rh	$Date: 1999/04/22 14:46:03 $Put all SPD EEPROM reads in SPD_mset_amips().
 * $Revision: 1.1 $rh	$Date: 1999/04/22 14:46:03 $Correct calculation for CAS time value.
 * $Revision: 1.1 $rh	$Date: 1999/04/22 14:46:03 $get CAS(x) time value before calling spdCAS(),
 *											spdCAS() now just do calculation and returns.
 * $Revision: 1.1 $rh	$Date: 1999/04/22 14:46:03 $spdCAS(): reads CPU operating freq.
 * $Revision: 1.1 $rh	$Date: 1999/04/22 14:46:03 $Add pause to debug spdCAS().
 * $Revision: 1.1 $rh	$Date: 1999/04/22 14:46:03 $Add macros RDELAY2(), V3DDISPLAY(),V3XDISPLAY().
 * $Revision: 1.1 $	$Date: 1999/04/22 14:46:03 $Created
 * $NoKeywords: $
 **
 * Description:
 **
 *	The routines in this file process the information stored in the
 *  	SPD (Serial Prensence Detect) EEPROM of the DIMM (memory module).
 *	 
 * 
 * 
 **
 * The following is written in assembly for MIPS processor.
 * 
 * 	SPD_mset_amips() is the top level routine for doing the SPD.
 *	
 **
 ********************************************************************/

/*********************************************************************
 *
 * Include files that this module depends on.
 *
 ********************************************************************/

/* SPD EEPROM system device number is 0x0 */		


/*
#include <mips/asm.h>
#include <mips/regdef.h>
#include <mips/cpu.h>
#include <mips/endian.h>
#include <mips/r5kc0.h>
#include "sbd.h"
 */

/* #define MIPSASM	*/
/*#include <mips/r5kc0.h>*/
#include "sbd.h"
#include "v3uscreg.h" 


#define RDELAYVALUE 100000

#define	RDELAY2(dvalue)  \
	.set	noat;	\
	li	AT,(dvalue);	\
1:	bnez	AT,1b;	\
	subu	AT,1;	\
	.set	at

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


#define	savera		t9
#define eedevnum	t8
#define lcpufreq	t7
#define cassupport  t6
#define caslatency	t6

/*!
 *********************************************************************
 **
 * Function: void SPD_mset_amips()
 **
 * Parameters:
 *
 **
 * Registers used:
 * 
 * t9  -save return address of routine that called this function
 * t8  -EEPROM system device number[0,4]
 * t7  -local processor operating frequency
 * 	    How to get local processor operating frequency ?
 * t6  -CAS Latency supported, read from SPD EEPROM @ offset 18 
 *
 * s2  -used as temp register for gathering relavent bits for writing to USC
 *
 * v0  -value read from SPD EEPROM, or 
 *     -value returned from function spdCAS: 
 *			0 indicates CAS Latency not met,
 *			1 indicates CAS Latency met.
 * v1  -value [1,2,3] represents the CAS Latency 
 **
 * Return: NONE 
 * v0  -value read from SPD EEPROM, or 
 *     -value returned from function spdCAS: 
 *			0 indicates CAS Latency not met,
 *			1 indicates CAS Latency met.
 * v1  -value [1,2,3] represents the CAS Latency 
 *     -value [4] represents ERROR condition,DIMM too slow.
 **
 * Description:
 *  This is the top level routine for processing the SPD information.
 *  Reads SPD EEPROM @ offset 18 to see which CAS Latencies are supported by the DIMM
 *  Checks board and USC delays to find which CAS Latency [1,2,3] to use
 *  Set up USC for CAS Latency and ...
 *	If timing delays exceed CAS Latency [3], the memory is declared too slow.
 **
 ********************************************************************/

SLEAF(SPD_mset_amips)
	.set noreorder
		/* save ra the return address of caller routine*/
	move savera,ra
		/* get local processor operating frequency */
	li	s7,7					# init pseudo stack
	li	eedevnum,0x4		    # USDER EEPROM device number
	li	s0,10
	jal	I2C_EEPROMRead_amips
	nop							
	move lcpufreq,v0			# save processor operating frequency 

		/* Which CAS Latency is supported ? */
	li  s7,7					# init pseudo stack
	li	eedevnum,0x0			# SPD EEPROM device number
	li	s0,18			# SPD EEPROM offset #18
	jal	I2C_EEPROMRead_amips
	nop
	move cassupport,v0			# save CAS Latency Support 

						# skip check for CAS(1)
	b	2f
	nop

1:	and	t1,v0,0x1		# does it support CAS(1) ? 
	beqz t1,2f			#  no (bit0=0)
	nop

	li	s0,26			#  yes (bit0=1),CAS(1) supported 
	li	v1,0x1
	bal	spdCAS			# go check timing for CAS(1)
	nop

	beqz v0,2f				# CAS(1) met ?
	nop
	b	spdSave1				# yes,go set USC
	nop
								# no,check next CAS Latency
2:	and t1,cassupport,0x2		# does it support CAS(2) ?
	beqz t1,3f			#  no (bit1=0)
	nop

					   #  yes (bit1=1),CAS(2) supported
	li	s0,24				   	# get CAS(2) time value,@ location 24
	li  s7,7						# init pseudo stack
	li	eedevnum,0x0				# SPD EEPROM device number
	jal	I2C_EEPROMRead_amips		# get CAS(2) time value
	nop								# v0 returns with time value

	bal	spdCAS					# go check timing for CAS(3
	nop

	beqz v0,3f				# CAS(2) met ?
	nop
	b	spdSave1				# yes,go set USC
	li	v1,0x2					
	
								# no,check next CAS Latency
3:	and t1,cassupport,0x4		# does it support CAS(3) ?
	beqz t1,4f			#  no (bit2=0)
	nop
						#  yes (bit2=1),CAS(3) supported
	li	s0,10					# get CAS(3) time value,@ location 10
	li  s7,7						# init pseudo stack
	li	eedevnum,0x0				# SPD EEPROM device number
	jal	I2C_EEPROMRead_amips		# get CAS(3) time value
	nop								# v0 returns with time value

	bal	spdCAS					# go check timing for CAS(3)
	nop

	beqz v0,4f				# CAS(3) met ?
	nop
	b	spdSave1				# yes,go do next SPD parameter
	li	v1,0x3
								# no, Error-memory is too slow
4:	li	v0,0			# v0 = 0, CAS latency not met
	b 	spddone			# ERROR, memory too slow
	li	v1,0x4

		/* CAS Latency found (Latency number in v1), save it */
spdSave1:

	li	s2,0x0			# clear s2,temp register for gathering relavent bits
	bal	spdSaveCAS
	nop
	
		/* get RCD (RAS to CAS delay) */
	li	s0,29
	li  s7,7						# init pseudo stack
	li	eedevnum,0x0				# SPD EEPROM device number
	jal	I2C_EEPROMRead_amips		# get Minimum RAS to CAS Delay time value
	nop	 							# v0 returns with value nSec.

	bal	spdRCD
	nop
	bal	spdSaveRCD	
	nop

		/* get RP (RAS precharge) */
	li	s0,27
	li  s7,7						# init pseudo stack
	li	eedevnum,0x0				# SPD EEPROM device number
	jal	I2C_EEPROMRead_amips		# get Minimum Row Precharge time value
	nop	 							# v0 returns with value in nSec.

	bal	spdRP
	nop
	bal	spdSaveRP
	nop

		/* get RAS (RAS pulse width) */
	li	s0,30
	li  s7,7						# init pseudo stack
	li	eedevnum,0x0				# SPD EEPROM device number
	jal	I2C_EEPROMRead_amips		# get Minimun RAS Pulse Width value
	nop	 							# v0 returns with value in nSec.

	bal	spdRAS
	nop
	bal spdSaveRAS
	nop

		/* get DPL (Last data write precharge) */
	bal	spdDPL
	nop
	bal	spdSaveDPL
	nop

		/* get REF_SCALE (Refresh prescaler) */
	li	s0,12						
	li  s7,7						# init pseudo stack
	li	eedevnum,0x0				# SPD EEPROM device number
	jal	I2C_EEPROMRead_amips		# get Refresh Rate value
	nop	 							# v0 returns with coded value

	bal spdRScale
	nop
	bal spdSaveRScale
	nop

		/* get NDIV (Refresh timer divisor) */
	/* bal	spdNDIV
	   nop
	 */
	bal	spdSaveNDIV
	nop
				/* recall all the bits processed */
	move v0,s2
spddone:	
				/* restore return address in ra */
	move ra,savera
	.set reorder
	j	ra
SEND(SPD_mset_amips)

/*!
 *********************************************************************
 **
 * Function: void spdCAS()
 **
 * Parameters:
 * lcpufreq  -specifies the operating frequency [75,66,50] in MHz of the local processor
 * s0  -specifies the location [0-128] in the SPD EEPROM to be read 
 *
 **
 * Registers used:
 * 
 **
 * Return: NONE 
 * v0  -value returned: 
 *			0 indicates CAS Latency not met,
 *			1 indicates CAS Latency met.
 *
 **
 * Description:
 *  Checks board and USC delays to see if the CAS Latency is met for
 *  	the operating frequency of the local processor. 
 **
 ********************************************************************/

SLEAF(spdCAS)
	.set noreorder
			/* INPUTS: */
			/* 	t7 contains the local processor operating frequency */
			/* 	v0 contains data ( x.x nsec) read from EEPROM */

	
			/* calculate operating clock period in pico seconds */
					/* period = (100MHz/operating freq ) x 10000 pico seconds */
	li	a0,10000
	li	a1,100
	mul	t1,a0,100
	nop	
	div	t2,t1,t7
	nop

			/* sum up delays in pico seconds */
	and	t1,v0,0x0f
	and v1,v0,0xf0
	srl	v0,v1,4
	mul	v1,v0,1000
	nop
	mul	v0,t1,100		# convert CAS time value into pico seconds 
	nop
	add	t1,v1,v0
	add	t0,t1,4000		# USC setup time = 3000 psec, board delay = 1000 psec
	sub	t1,t2,t0		# operating clock period - total delays
						/* if result is less than zero,CAS Latency not met	*/
	bltz t1,1f			
	nop
	
	li	v0,0x1				
	b 	2f 
	nop
							/* result > zero, CAS Latency met  */
1:	li	v0,0				# result = zero, CAS Latency not met
	/* .set reorder	*/
2:	

	.set reorder
	j	ra

SEND(spdCAS)

SLEAF(spdSaveCAS)
	.set noreorder
			/* sp,temp register for gathering relavent bits */
			/* v1,specifies CAS Latency[3,2,1] */
	bne	v1,0x3,1f
	nop
	b	6f
	li	v1,SDRAM_CFG_TCAS_RD_3

1:	bne	v1,0x2,2f
	nop
	b	6f
	li	v1,SDRAM_CFG_TCAS_RD_2

2:	li	v1,SDRAM_CFG_TCAS_RD_1

	.set reorder
6:	move v0,s2
	or	s2,v0,v1
	j	ra

SEND(spdSaveCAS)



SLEAF(spdRCD)
	.set noreorder
			/* lcpufreq specifies the operating clock frequency[75,66,50]MHz */
			/* v0 contains data ( x nsec) read from EEPROM */

			/* calculate operating clock period in pico seconds */
			/*   period = (100MHz/operating freq ) x 10000 pico seconds */
	li	a0,10000
	li	a1,100
	mul	t1,a0,a1
	nop	
	div	t2,t1,lcpufreq
	nop
			/* calculate the number clock cycles for RCD*/
	mul t1,v0,10000		# convert RCD value into pico seconds and x10
	div	t0,t1,t2		# cycles x10 = (RCD value x10 )/ operating clock period

	blt	t0,20,1f		# cycles x 10 < 20 ?
	nop
	b	6f					# no, use 3 cycles
	li	v1,3
							# yes, try next lower cycle
1:	blt t0,10,2f		# cycles x 10 < 10 ?
	nop
	b	6f					# no, use 2 cycles
	li	v1,2
							# yes, use next lower cycle
2:	li	v1,1			# use 1 cycle

	.set reorder
6:	j	ra

SEND(spdRCD)

SLEAF(spdSaveRCD)
	.set noreorder

			/* s2,temp register for gathering relavent bits */
	bne	v1,0x3,1f		# cycles < 3 ?
	nop
	b	6f					# no, use 3 cycles
	li	v1,SDRAM_CFG_RCD_3
							# yes, use next lower cycle
1:	bne	v1,0x2,2f		# cycles < 2 ?				
	nop
	b	6f					# no, use 2 cycles
	li	v1,SDRAM_CFG_RCD_2
							# yes, use next lower cycle
2:	li	v1,SDRAM_CFG_RCD_1	# use 1 cycle

	.set reorder
6:	move v0,s2
	or	s2,v0,v1
	j	ra

SEND(spdSaveRCD)


SLEAF(spdRP)
	.set noreorder
			/* s0 specifies the location[0-128] in SPD EEPROM to be read */
			/* lcpufreq specifies the operating clock frequency[75,66,50]MHz */
			/* v0 contains data ( x nsec) read from EEPROM */

			/* calculate operating clock period in pico seconds */
			/*   period = (100MHz/operating freq ) x 10000 pico seconds */
	li	a0,10000
	li	a1,100
	mul	t1,a0,a1	
	div	t2,t1,lcpufreq

			/* calculate the number clock cycles for RP*/
	mul t1,v0,10000		# convert RP value into pico seconds and x10
	div	t0,t1,t2		# cycles x10 = (RP value x10 )/ operating clock period

	blt	t0,30,1f		# cycles x 10 < 30 ?
	nop
	b	6f					# no, use 4 cycles
	li	v1,4
							# yes, try next lower cycle
1:	blt	t0,20,2f		# cycles x 10 < 20 ?
	nop
	b	6f					# no, use 3 cycles
	li	v1,3
							# yes, try next lower cycle
2:	blt t0,10,3f		# cycles x 10 < 10 ?
	nop
	b	6f					# no, use 2 cycles
	li	v1,2
							# yes, use next lower cycle
3:	li	v1,1			# use 1 cycle

	.set reorder
6:	j	ra

SEND(spdRP)

SLEAF(spdSaveRP)
	.set noreorder

			/* s2,temp register for gathering relavent bits */
	bne	v1,0x4,1f		# cycles < 4 ?
	nop
	b	6f					# no, use 4 cycles
	li	v1,SDRAM_CFG_RP_4
							# yes, use next lower cycle
1:	bne	v1,0x3,2f		# cycles < 3 ?
	nop
	b	6f					# no, use 3 cycles
	li	v1,SDRAM_CFG_RP_3
							# yes, use next lower cycle
2:	bne	v1,0x2,3f		# cycles < 2 ?				
	nop
	b	6f					# no, use 2 cycles
	li	v1,SDRAM_CFG_RP_2
							# yes, use next lower cycle
3:	li	v1,SDRAM_CFG_RP_1	# use 1 cycle

	.set reorder
6:	move v0,s2
	or	s2,v0,v1
	j	ra

SEND(spdSaveRP)


SLEAF(spdRAS)
	.set noreorder
			/* lcpufreq specifies the operating clock frequency[75,66,50]MHz */
			/* v0 contains data ( x nsec) read from EEPROM */

			/* calculate operating clock period in pico seconds */
			/*   period = (100MHz/operating freq ) x 10000 pico seconds */
	li	a0,10000
	li	a1,100
	mul	t1,a0,a1	
	div	t2,t1,lcpufreq

			/* calculate the number clock cycles for RP*/
	mul t1,v0,10000		# convert RAS value into pico seconds and x10
	div	t0,t1,t2		# cycles x10 = (RAS value x10 )/ operating clock period

	blt	t0,40,1f		# cycles x 10 < 40 ?
	nop
	b	6f					# no, use 5 cycles
	li	v1,5
							# yes, try next lower cycle
1:	blt	t0,30,2f		# cycles x 10 < 30 ?
	nop
	b	6f					# no, use 4 cycles
	li	v1,4
							# yes, try next lower cycle
2:	blt	t0,20,3f		# cycles x 10 < 20 ?
	nop
	b	6f					# no, use 3 cycles
	li	v1,3
							# yes, try next lower cycle
3:	blt t0,10,4f		# cycles x 10 < 10 ?
	nop
	b	6f					# no, use 2 cycles
	li	v1,2
							# yes, use next lower cycle
4:	li	v1,1			# use 1 cycle

	.set reorder
6:	j	ra

SEND(spdRAS)

SLEAF(spdSaveRAS)
	.set noreorder

			/* s2,temp register for gathering relavent bits */
	bne	v1,0x5,1f		# cycles < 5 ?
	nop
	b	6f					# no, use 5 cycles
	li	v1,SDRAM_CFG_RAS_5
							# yes, use next lower cycle
1:	bne	v1,0x4,2f		# cycles < 4 ?
	nop
	b	6f					# no, use 4 cycles
	li	v1,SDRAM_CFG_RAS_4
							# yes, use next lower cycle
2:	bne	v1,0x3,3f		# cycles < 3 ?				
	nop
	b	6f					# no, use 3 cycles
	li	v1,SDRAM_CFG_RAS_3
							# yes, use next lower cycle
3:	li	v1,SDRAM_CFG_RAS_2	# use 2 cycle

	.set reorder
6:	move v0,s2
	or	s2,v0,v1
	j	ra

SEND(spdSaveRAS)


SLEAF(spdDPL)
	.set noreorder

			/* lcpufreq specifies the operating clock frequency[75,66,50]MHz */

	blt	lcpufreq,75,1f		# frequency  < 75 ?
	nop
	b	6f					# no, use 2 cycle
	li	v1,2
							# yes, try next lower cycle
1:	blt lcpufreq,66,2f		# frequency < 66 ?
	nop
	b	6f					# no, use 1 cycle
	li	v1,1
							# yes, use next lower cycle
2:	li	v1,1			# use 1 cycle

	.set reorder
6:	j	ra

SEND(spdDPL)

SLEAF(spdSaveDPL)
	.set noreorder

			/* s2,temp register for gathering relavent bits */
	bne	v1,0x2,1f		# cycles < 2 ?				
	nop
	b	6f					# no, use 2 cycles
	li	v1,SDRAM_CFG_DPL_2
							# yes, use next lower cycle
1:	li	v1,SDRAM_CFG_DPL_1	# use 1 cycle

	.set reorder
6:	move v0,s2
	or	s2,v0,v1
	j	ra

SEND(spdSaveDPL)


SLEAF(spdRScale)
	.set noreorder
			/* lcpufreq specifies the operating clock frequency[75,66,50]MHz */
			/* v0 contains time code read from EEPROM */
	and t1,v0,0xe		# rid of bit 7
	move v0,t1	

	bnez v0,1f			# if code is 0, default of 15.6us is used; 
	nop					#   that is 1 x 15.6
	li	v0,1
1:	mul	t1,v0,156		# refresh period x10 = eeprom value x 15.6us x10

	mul t2,t1,lcpufreq		# (refresh period x10) x operating clock freq

	div v1,t2,320		# divide by 32 and divide by 10 gives REF_NDIV

	blt	v1,0x3f,2f		# result < 0x3f [max value allowed for 6 bit REF_NDIV] ?
	nop					    

	div	v1,t2,128			# no, divide by 128 gives REF_NDIV
	b	6f
	li	v0,1				   # REF_SCALE = 1

2:	li	v0,0				# yes, REF_SCALE = 0	

	.set reorder
6:	j	ra				# return: v1 = REF_NDIV		
						#         v0 = REF_SCALE
SEND(spdRScale)

SLEAF(spdSaveRScale)
			/* s2,temp register for gathering relavent bits */
	.set noreorder
	bnez v0,1f
	nop

	li	v0,~SDRAM_CFG_REF_SCALE_MASK
	move t1,s2
	and	s2,t1,v0
	b	6f

1:	li	v0,SDRAM_CFG_REF_SCALE_MASK

	.set reorder	
	move t1,s2
	or	s2,t1,v0
6:	j	ra

SEND(spdSaveRScale)

SLEAF(spdSaveNDIV)
			/* s2,temp register for gathering relavent bits */
			/* v1,REF_NDIV */
	.set noreorder

	sll	t1,v1,SDRAM_CFG_REF_NDIV_SHIFT
	and	v0,t1,SDRAM_CFG_REF_NDIV_MASK

	.set reorder	
	move t1,s2
	or	s2,t1,v0

	j	ra

SEND(spdSaveNDIV)
