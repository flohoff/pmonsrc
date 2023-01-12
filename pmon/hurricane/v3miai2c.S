/*********************************************************************
 **
 * Module: 
 **
 * File Name: v3miAi2c.S
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
 * $Revision: 1.1 $   $Date: 1999/04/22 14:46:03 $Edit I2C_PollAckStart_amips()
 * $Revision: 1.1 $   $Date: 1999/04/22 14:46:03 $Edit I2C_Read8_amips(),I2C_EEPROMRead_amips(),I2C_StartSlave_amips()
 *											use move  ,SLAVEADDR statements.
 * $Revision: 1.1 $   $Date: 1999/04/22 14:46:03 $Edit I2C_Read8_amips(),I2C_EEPROMRead_amips(),
 *											I2C_StartSlave_amips()
 * $Revision: 1.1 $   $Date: 1999/04/22 14:46:03 $Edit I2C_Write8_amips()
 * $Revision: 1.1 $   $Date: 1999/04/22 14:46:03 $Edit I2C_SDAIn_amips()
 *										   $Edit rastack_pop(),rastack_push(), use s7,not v0.	
 * $Revision: 1.1 $   $Date: 1999/04/22 14:46:03 $Edit I2C_PollAckStart_amips(),save a1 in a3,move label 4 to proper place.
 * $Revision: 1.1 $   $Date: 1999/04/22 14:46:03 $rastack_push(),rastack_pop(),push/pop return address of called routine in v1
 *											all routines that uses rastack_push() must save ra in v1 first, 
 *											 and restore ra from v1 after call to rastack_pop()
 * $Revision: 1.1 $	$Date: 1999/04/22 14:46:03 $Commented out #define SLAVEADDR 4
 * $Revision: 1.1 $	$Date: 1999/04/22 14:46:03 $Added rastack_push(),rastack_pop()
 * $Revision: 1.1 $	$Date: 1999/04/22 14:46:03 $Edit I2C_Delay_amips()
 * $Revision: 1.1 $	$Date: 1999/04/22 14:46:03 $Created
 * $NoKeywords: $
 **
 * Description:
 **
 * This code has been adapted from Phil Sikora file "v3mipi2c.c" on I2C
 * 
 **
 * The following is written in assembly for MIPS processor.
 * 
 * 
 **
 ********************************************************************/

/*********************************************************************
 *
 * Include files that this module depends on.
 *
 ********************************************************************/
							 /* I2C device address 4 for testing now */
/* #define SLAVEADDR	4  */		

#include "v3I2Ca.h" 


/*
#include <mips/asm.h>
#include <mips/regdef.h>
#include <mips/cpu.h>
#include <mips/endian.h>
#include <mips/r5kc0.h>
#include "sbd.h"
 */

/* #define MIPSASM	*/
#include "sbd.h"
#include "v3uscreg.h" 
 
/*!
 *********************************************************************
 **
 * Function: void rastack_push()
 **
 * Parameters:
 * v1	-return address that the called routine must go back to.
 **
 * Registers used:
 * 
 * s7	-value represents the lowest s register used for storing ra	register
 * s6-s1
 **
 * Return: NONE 
 **
 * Description:
 *  Push the current ra register onto the pseudo stack using the s registers
 *
 **
 ********************************************************************/

SLEAF(rastack_push)
	.set noreorder
	/* move v0,s7  */
6:	blt	s7,0x07,5f
	nop
	li	s7,0x6
	b	8f
	move s6,v1
5:	blt	s7,0x06,4f
	nop
	li	s7,0x5
	b	8f
	move s5,v1
4:	blt	s7,0x05,3f
	nop
	li	s7,0x4
	b	8f
	move s4,v1
3:	blt	s7,0x04,2f
	nop
	li	s7,0x3
	b	8f
	move s3,v1
2:	blt	s7,0x03,1f
	nop
	li	s7,0x2
	b	8f
	move s2,v1
1:	blt	s7,0x02,8f
	nop
	li	s7,0x1
	move s1,v1

	.set reorder
8:	
	j	ra
SEND(rastack_push)

/*!
 *********************************************************************
 **
 * Function: void rastack_pop()
 **
 * Parameters:
 * 
 **
 * Registers used:
 * v1
 * s7	-value represents the lowest s register used for storing ra	register
 * s6-s1
 **
 * Return:
 * v1	-return address the called routine should return to. 
 **
 * Description:
 *  Pop the ra register from the pseudo stack using the s registers
 *
 **
 ********************************************************************/

SLEAF(rastack_pop)
	.set noreorder
	/* move v0,s7 */
1:	bgt	s7,0x01,2f
	nop
	li	s7,0x2
	b	8f
	move v1,s1
2:	bgt	s7,0x02,3f
	nop
	li	s7,0x3
	b	8f
	move v1,s2
3:	bgt	s7,0x03,4f
	nop
	li	s7,0x4
	b	8f
	move v1,s3
4:	bgt	s7,0x04,5f
	nop
	li	s7,0x5
	b	8f
	move v1,s4
5:	bgt	s7,0x05,6f
	nop
	li	s7,0x6
	b	8f
	move v1,s5
6:	bgt	s7,0x06,8f
	nop
	li	s7,0x7
	move v1,s6

	.set reorder
8:	
	j	ra
SEND(rastack_pop)


/*!
 *********************************************************************
 **
 * Function: BYTE V3USC32_ReadRegByte_amips(V3USCHANDLE hV3, DWORD dwReg)
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3,pointer to base address (physical)of USC
 * a0	-DWORD dwReg - USC register @ offset from USC base address
 * 
 **
 * Registers used:
 * t1,v0
 **
 * Return: v0 -BYTE,1 data byte read from USC 
 **
 * Description:
 *  Read 1 byte from register in USC
 **
 ********************************************************************/

SLEAF(V3USC32_ReadRegByte_amips)
	.set noreorder
	li	t1,PA_TO_KVA1(V3USC_BASE + V3USC_SYSTEM_B)
	lb	v0,0(t1)
	nop
	.set reorder
	j	ra
SEND(V3USC32_ReadRegByte_amips)

/*!
 *********************************************************************
 **
 * Function: BOOL V3USC32_WriteRegByte_amips(V3USCHANDLE hV3, DWORD dwReg, BYTE bdata)
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3, pointer to base address (physical)of USC
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * a1	-BYTE bdata, data to be written
 **
 * Registers used:
 * t1,a1
 **
 * Return: NONE 
 **
 * Description:
 *  Write 1 byte to register in USC
 **
 ********************************************************************/

SLEAF(V3USC32_WriteRegByte_amips)
	.set noreorder
	li	t1,PA_TO_KVA1(V3USC_BASE + V3USC_SYSTEM_B)
	sb	a1,0(t1)
	.set reorder
	j	ra
SEND(V3USC32_WriteRegByte_amips)

/*!
 *********************************************************************
 **
 * Function: void I2C_Unlock_amips(V3USCHANDLE hV3)
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3, pointer to base address (physical)of USC
 **
 * Registers used:
 * v1	-save return address (ra)
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * a1	-BYTE bdata, data to be written or data read from
 * v0
 * 
 **
 * Return: NONE 
 **
 * Description:
 * This routine enable the sytem register for writting, and enables
 * the Serial Clock output pin.
 **
 ********************************************************************/

SLEAF(I2C_UnLock_amips)
	.set noreorder
					/* save ra, return address */
	move v1,ra
	jal	rastack_push
	nop

	li	a0,V3USC_SYSTEM_B
	bal	V3USC32_WriteRegByte_amips
	li	a1,SYSTEM_B_UNLOCK_TOKEN

	bal	V3USC32_ReadRegByte_amips
	nop

	bal	V3USC32_WriteRegByte_amips
	or	a1,v0,SYSTEM_B_SPROM_EN	
					/* restore ra */
	jal	rastack_pop
	nop
	move ra,v1	
	.set reorder
	j	ra
SEND(I2C_UnLock_amips)

/*!
 *********************************************************************
 **
 * Function: void I2C_Lock_amips(V3USCHANDLE hV3)
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3, pointer to base address (physical)of USC
 **
 * Registers used:
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * a1	-BYTE bdata, data to be written or data read from
 * v0
 * 
 **
 * Return: NONE 
 **
 * Description:
 * This routine disables the Serial Clock pin, and sets the system
 * lock bit.
 **
 ********************************************************************/

SLEAF(I2C_Lock_amips)
	.set noreorder
					/* save ra, return address */
	move v1,ra
	jal	rastack_push
	nop

	bal	V3USC32_ReadRegByte_amips
	li	a0,V3USC_SYSTEM_B

	and	a1,v0,~SYSTEM_B_SPROM_EN	
	bal	V3USC32_WriteRegByte_amips
	nop

	bal	V3USC32_WriteRegByte_amips
	or	a1,v0,SYSTEM_B_LOCK	
					/* restore ra */
	jal	rastack_pop
	nop
	move ra,v1		
	.set reorder
	j	ra
SEND(I2C_Lock_amips)

/*!
 *********************************************************************
 **
 * Function: void I2C_SDA_amips(V3USCHANDLE hV3, int State)
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3,pointer to base address (physical)of USC
 * a1	-int State - what state the SDA to be set to
 * 
 **
 * Registers used:
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * a1	-over-written with intermediate data
 * v0
 * 
 **
 * Return: NONE
 **
 * Description:
 * This routine sets the Serial Data line to the desired state.
 **
 ********************************************************************/

SLEAF(I2C_SDA_amips)
	.set noreorder
					/* save ra, return address */
	move v1,ra
	jal	rastack_push
	nop

	li	a0,V3USC_SYSTEM_B
	bal	V3USC32_ReadRegByte_amips
	nop

	beqz a1,1f
	nop
	or	a1,v0,SYSTEM_B_SDA_OUT	
	b	2f
	nop

1:	and	a1,v0,~SYSTEM_B_SDA_OUT	

2:	bal	V3USC32_WriteRegByte_amips
	nop
					/* restore ra */
	jal	rastack_pop
	nop
	move ra,v1
	.set reorder
	j	ra
SEND(I2C_SDA_amips)


/*!
 *********************************************************************
 **
 * Function: void I2C_SDAIn_amips(V3USCHANDLE hV3)
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3,pointer to base address (physical)of USC
 * 
 **
 * Registers used:
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * a1	-over-written with intermediate data
 * v0
 * 
 **
 * Return: 
 * v0 	-BYTE bData, state of the Serial Data line
 **
 * Description:
 * This routine returns the state of the Serial Data line.
 **
 ********************************************************************/

SLEAF(I2C_SDAIn_amips)
	.set noreorder
					/* save ra, return address */
	move v1,ra
	jal	rastack_push
	nop

	bal	V3USC32_ReadRegByte_amips
	li	a0,V3USC_SYSTEM_B

	/* and	a1,v0,SYSTEM_B_SDA_IN  */
	/* srl	a1,SYSTEM_B_SDA_IN_SHIFT */
	/* move v0,a1 */

	and	v1,v0,SYSTEM_B_SDA_IN
	and v0,v1,0xff
	srl a0,v0,SYSTEM_B_SDA_IN_SHIFT
	and	v1,a0,0xff
	move v0,v1
					/* restore ra */
	jal	rastack_pop
	nop
	move ra,v1
	.set reorder	
	j	ra
SEND(I2C_SDAIn_amips)


/*!
 *********************************************************************
 **
 * Function: void I2C_SCL_amips(V3USCHANDLE hV3, int State)
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3,pointer to base address (physical)of USC
 * a1	-int State - what state the SDA to be set to
 * 
 **
 * Registers used:
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * a1	-over-written with intermediate data
 * v0
 * 
 **
 * Return: NONE
 **
 * Description:
 * This routine sets the Serial Clock line to the desired state.
 **
 ********************************************************************/

SLEAF(I2C_SCL_amips)
	.set noreorder
					/* save ra, return address */
	move v1,ra
	jal	rastack_push
	nop

	li	a0,V3USC_SYSTEM_B
	bal	V3USC32_ReadRegByte_amips
	nop

	beqz a1,1f
	nop
	or	a1,v0,SYSTEM_B_SCL	
	b	2f
	nop

1:	and	a1,v0,~SYSTEM_B_SCL	

2:	bal	V3USC32_WriteRegByte_amips
	nop
					/* restore ra */
	jal	rastack_pop
	nop
	move ra,v1
	.set reorder
	j	ra
SEND(I2C_SCL_amips)

/*!
 *********************************************************************
 **
 * Function: void I2C_Ack_amips(V3USCHANDLE hV3)
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3,pointer to base address (physical)of USC
 * 
 **
 * Registers used:
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * a1	-over-written with intermediate data
 * 
 * 
 **
 * Return: NONE
 **
 * Description:
 * Acknowledge is a software convention used to indicate
 * sucessful data transfers.  The transmitting device, either 
 * master or slave, will release the bus after transmitting 
 * eight bits.  During the ninth clock cycle the receiver will 
 * pull the SDA line low to acknowledge that it received the 
 * eight bits of data.
 **
 ********************************************************************/

SLEAF(I2C_Ack_amips)

	.set noreorder
					/* save ra, return address */
	move v1,ra
	jal	rastack_push
	nop

	bal	I2C_SDA_amips
	li	a1,I2C_LOW

	bal	I2C_Delay_amips
	nop

	bal	I2C_SCL_amips
	li	a1,I2C_HIGH

	bal	I2C_Delay_amips
	nop

	bal	I2C_SCL_amips
	li	a1,I2C_LOW
					/* restore ra */
	jal	rastack_pop
	nop
	move ra,v1
	.set reorder
	j	ra
SEND(I2C_Ack_amips)

/*!
 *********************************************************************
 **
 * Function: void I2C_NoAck_amips(V3USCHANDLE hV3)
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3,pointer to base address (physical)of USC
 * 
 **
 * Registers used:
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * a1	-over-written with intermediate data
 * 
 * 
 **
 * Return: NONE
 **
 * Description:
 * The No-Acknowledge is a software convention used to indicate
 * unsucessful data transfers.  The transmitting device, either 
 * master or slave, will release the bus after transmitting 
 * eight bits.  During the ninth clock cycle the receiver will 
 * pull the SDA line high to indicate that it did not received the 
 * eight bits of data.
 **
 ********************************************************************/

SLEAF(I2C_NoAck_amips)

	.set noreorder
					/* save ra, return address */
	move v1,ra
	jal	rastack_push
	nop

	bal	I2C_SDA_amips
	li	a1,I2C_HIGH

	bal	I2C_Delay_amips
	nop

	bal	I2C_SCL_amips
	li	a1,I2C_HIGH

	bal	I2C_Delay_amips
	nop

	bal	I2C_SCL_amips
	li	a1,I2C_LOW
					/* restore ra */
	jal	rastack_pop
	nop
	move ra,v1
	.set reorder
	j	ra
SEND(I2C_NoAck_amips)

/*!
 *********************************************************************
 **
 * Function: void I2C_Stop_amips(V3USCHANDLE hV3)
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3,pointer to base address (physical)of USC
 * 
 **
 * Registers used:
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * a1	-over-written with intermediate data
 * 
 * 
 **
 * Return: NONE
 **
 * Description:
 * All communications must be terminated by a stop condition which
 * is a low to high transition of SDA while SCL is high.  A stop 
 * condition can only be issued after the transmitting device has 
 * released the bus.
 **
 ********************************************************************/

SLEAF(I2C_Stop_amips)

	.set noreorder
					/* save ra, return address */
	move v1,ra
	jal	rastack_push
	nop

	bal	I2C_SDA_amips
	li	a1,I2C_LOW

	bal	I2C_Delay_amips
	nop

	bal	I2C_SCL_amips
	li	a1,I2C_HIGH

	bal	I2C_Delay_amips
	nop

	bal	I2C_SDA_amips
	li	a1,I2C_HIGH
					/* restore ra */
	jal	rastack_pop
	nop
	move ra,v1
	.set reorder

	j	ra
SEND(I2C_Stop_amips)

/*!
 *********************************************************************
 **
 * Function: void I2C_Write8_amips(V3USCHANDLE hV3, register BYTE bData)
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3,pointer to base address (physical)of USC
 * a2	-register BYTE bData, data to be written out
 **
 * Registers used:
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * a1	-over-written with intermediate data
 * v0
 * t2
 **
 * Return: 
 * v0	-BYTE bAck,returns the acknowledge bit 
 **
 * Description:
 * This routine writes out all 8 bits of data out to the I2C bus.  After 
 * writing out the data, the routine reads the ACK/NACK response back and
 * returns it to the caller.
 **
 ********************************************************************/

SLEAF(I2C_Write8_amips)

	.set noreorder
					/* save ra, return address */
	move v1,ra
	jal	rastack_push
	nop

	li	t2,0x80
1:	bnez t2,2f		   # for loop, start
	nop
	b	3f
	nop
2:	bal	I2C_SDA_amips		
	and	a1,t2,a2		

	bal	I2C_SCL_amips
	li	a1,I2C_HIGH

	bal	I2C_Delay_amips
	nop

	bal	I2C_SCL_amips
	li	a1,I2C_LOW

	bal	I2C_Delay_amips
	nop

	srl	v0,t2,1			# for loop, end
	andi v1,v0,0xff
	b 	1b
	move t2,v1

3:	bal	I2C_SDA_amips
	li	a1,I2C_HIGH
	bal	I2C_SCL_amips
	li	a1,I2C_HIGH
	bal	I2C_Delay_amips
	nop

	bal	I2C_SDAIn_amips
	nop
	move t2,v0			# save v0,acknowledge bit

	bal	I2C_SCL_amips
	li	a1,I2C_LOW

	bal	I2C_Delay_amips
	nop
					/* restore ra */
	jal	rastack_pop
	nop
	move ra,v1	
	.set reorder
	move v0,t2			#retrieve acknowledge bit
	j	ra
SEND(I2C_Write8_amips)

/*!
 *********************************************************************
 **
 * Function: void I2C_Read8_amips(V3USCHANDLE hV3)
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3,pointer to base address (physical)of USC
 *
 **
 * Registers used:
 * a0	-
 * a1	-over-written with intermediate data
 * a2	-bData,formed data read from eeprom
 * v0
 * v1
 * t2
 **
 * Return: 
 * v0	-BYTE bData,returns a byte of data
 **
 * Description:
 * Reads eight bits from the I2C bus
 **
 ********************************************************************/

SLEAF(I2C_Read8_amips)

	li	a2,0
	.set noreorder
					/* save ra, return address */
	move v1,ra
	jal	rastack_push
	nop

	bal	I2C_SDA_amips
	li	a1,I2C_HIGH

	li	t2,0x08
1:	bnez t2,2f		   # for loop,start
	nop
	b	3f
	nop
2:	bal	I2C_SCL_amips	
	li	a1,I2C_HIGH		

	bal	I2C_Delay_amips
	nop
	bal	I2C_SDAIn_amips
	nop
	sll	v1,a2,1
	or	v0,v0,v1
	andi a2,v0,0xff

	bal	I2C_SCL_amips
	li	a1,I2C_LOW

	bal	I2C_Delay_amips
	nop

	addiu v1,t2,-1			# for loop, end
	andi  v0,v1,0xff
	b	  1b
	move  t2,v0

					/* restore ra */
3:	jal	rastack_pop
	nop
	move ra,v1
	.set reorder
	move v0,a2			# retrieve data read
	j	ra
SEND(I2C_Read8_amips)

/*!
 *********************************************************************
 **
 * Function: int I2C_PollAckStart_amips(V3USCHANDLE hV3, register BYTE bAddr, BYTE ReadBit))
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3,pointer to base address (physical)of USC
 * a1	-BYTE ReadBit,Set if command is to be a read, reset if command is a write 
 * a2	-BYTE bAddr,3 bit address of device been selected
 *
 **
 * Registers used:
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * a3
 * v0
 * v1
 * t3	-BYTE,retry counter
 **
 * Return: 
 * v0	-I2C_BUSY  - device is not responding (retried out)
 * v0	-I2C_OK    - read was successful
 **
 * Description:
 * This routine checks the status of the device being written to by 
 * issuing a start condition followed by check for ACK.  If the slave
 * device is unable to communicate, the write command will not be
 * acknowledged and we should wait.  This routine will try I2C_RETRY times for 
 * an ACK before returning a I2C_BUSY indication.
 * From testing the number of retries was as high as fifteen. 
 **
 ********************************************************************/

SLEAF(I2C_PollAckStart_amips)
	li	t3,I2C_RETRY
	move a3,a1
	.set noreorder
					/* save ra, return address */
	move v1,ra
	jal	rastack_push
	nop
				/* do while(I2C_RETRY) loop start */
1:	bal	I2C_SDA_amips
	li	a1,I2C_LOW

	bal	I2C_Delay_amips
	nop

	bal	I2C_SCL_amips
	li	a1,I2C_LOW

	bal	I2C_Delay_amips
	nop
					/* check to see if device is busy */
	sll	v0,a2,0x1
	move v1,v0
	or	v1,a3
	bal	I2C_Write8_amips
	andi a2,v1,0xff	

	bnez v0,2f
	nop
					/* device not busy,return I2C_OK */
	b	4f
	li a3,I2C_OK

					/* device is busy,setup for start condition if device busy */
2:	bal	I2C_SCL_amips
	li	a1,I2C_HIGH

	bal	I2C_Delay_amips
	nop

					/* set upper device address */
	move v1,SLAVEADDR
	or	a2,v1,I2C_1010

				/* do while(I2C_RETRY) loop end */
	move v0,t3
	addiu v1,v0,-1
	andi t3,v1,0xff
	andi v0,v1,0xff
	li	v1,0xff
					/* check for loop condition */
	bne	v0,v1,1b
	nop
				/* return I2C_BUSY */
3:  li	a3,I2C_BUSY
					/* restore ra */
4:	jal	rastack_pop
	nop
	move ra,v1
	.set reorder
	move v0,a3
 	j	ra	
SEND(I2C_PollAckStart_amips)

/*!
 *********************************************************************
 **
 * Function: int I2C_StartSlave_amips(V3USCHANDLE hV3, register BYTE bAddr, BYTE ReadBit))
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3,pointer to base address (physical)of USC
 * a1	-BYTE ReadBit,Set if command is to be a read, reset if command is a write 
 * a2	-BYTE bAddr,3 bit address of device been selected
 *
 **
 * Registers used:
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * v0
 * v1
 **
 * Return: 
 * v0	-I2C_OK    - read was successful
 * v0	-I2C_NOACK - if negative acknowledge was received
 **
 * Description:
 * All commands are preceded by the start condition, which is a high
 * to low transition of SDA when SCL is high.  All I2C devices
 * continuously monitor the SDA and SCL lines for the start condition 
 * and will not respond to any command until this condition has been met.
 *
 * Once a device detects that it is being addressed it outputs an 
 * acknowledge on the SDA line.  Depending on the state of the read/write 
 * bit, the device will execute a read or write operation.
 *
 * +-----+-----+-----+-----+-----+-----+-----+-----+
 * | SA6 | SA5 | SA4 | SA3 | SA2 | SA1 | SA0 | R/W |
 * +-----+-----+-----+-----+-----+-----+-----+-----+
 *                                              |--- 1=read, 0=write
 **
 ********************************************************************/

SLEAF(I2C_StartSlave_amips)
					/* start condition */
	.set noreorder
	move a3,a1
					/* save ra, return address */
	move v1,ra
	jal	rastack_push
	nop

	bal	I2C_SCL_amips
	li	a1,I2C_HIGH

	bal	I2C_Delay_amips
	nop
	bal	I2C_SDA_amips
	li	a1,I2C_LOW
	bal	I2C_Delay_amips
	nop

	bal	I2C_SCL_amips
	li	a1,I2C_LOW

	bal	I2C_Delay_amips
	nop
					/* write addr of Slave onto the I2C bus */
	sll	v0,a2,0x1
	move v1,v0
	or	v1,a3
	bal	I2C_Write8_amips
	andi a2,v1,0xff
					/* Ack bit returned in v0 */
					/* restore ra */
	jal	rastack_pop
	nop
	move ra,v1
	.set reorder
					/* acknowledge bit is returned in v0 */
 	j	ra	
SEND(I2C_StartSlave_amips)


/*!
 *********************************************************************
 **
 * Function: void I2C_Init_amips(V3USCHANDLE hV3)
 **
 * Parameters:
 * s0 	-DWORD,V3USCHANDLE hV3,pointer to base address (physical)of USC
 * 
 **
 * Registers used:
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * a1	-over-written with intermediate data
 * v0
 * t1
 **
 * Return: NONE
 **
 * Description:
 * Note: I2C_Init should be called for each board.
 * This routine will force a stop condition on the I2C bus.
 * The clock and data lines will be left high and this routine will 
 * return after a half clock.
 * This routine is only called now if EEPROM is about to be accessed
 * previously this was always called which modified the system
 * register, and assumed the EEPROM was always present.
 * Now the first EEPROM routine read/write will initialized the
 * I2C bus.
 *   The static variable bOneTimeInit is not implemented in this assembler version.
 **
 ********************************************************************/

SLEAF(I2C_Init_amips)
	.set noreorder
					/* save ra, return address */
	move v1,ra
	jal	rastack_push
	nop

	bal	I2C_UnLock_amips
	nop
	bal	I2C_Stop_amips
	nop
	bal	I2C_Delay_amips
	nop
	bal	I2C_Lock_amips
	nop
					/* restore ra */
	jal	rastack_pop
	nop
	move ra,v1
	.set reorder
	j	ra
SEND(I2C_Init_amips)

/*!
 *********************************************************************
 **
 * Function: BOOL I2C_EEPROMWrite_amips(V3USCHANDLE hV3, BYTE bSlaveAddr, BYTE bAddr, BYTE bData)
 **
 * Parameters:
 * 	-DWORD,V3USCHANDLE hV3,pointer to base address (physical)of USC
 * 	-BYTE bSlaveAddr - 3 bit address of device been selected
 * s0	-BYTE bAddr - address of EEPROM location to be read from
 * s1	-BYTE bData - value to be written to EEPROM 
 * 
 **
 * Registers used:
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * a1	-over-written with intermediate data
 * a2
 * v0
 * v1
 * t4
 **
 * Return: 
 * v0	-BOOL fStatus - TRUE if read successful
 **
 * Description:
 * This routine writes a byte to the slave device's address specified.
 * This routine uses the Write Byte method from the Atmel documentation.
 **
 ********************************************************************/

LEAF(I2C_EEPROMWrite_amips)
					/* save ra */
	.set noreorder
	move v1,ra
	jal	rastack_push
	nop

	li	t4,TRUE
					/* Ensure the I2C bus is initialized */
	bal	I2C_Init_amips
	nop
					/* unlock the system register */
	bal	I2C_UnLock_amips
	nop	
					/* set upper device address */
	move v1,SLAVEADDR
	or	a2,v1,I2C_1010
					/* send device Address,wait for Ack,bAddr is sent as a write */  
	bal	I2C_PollAckStart_amips
	li	a1,I2C_WRITE
	

	beqz v0,1f
	nop
	li	t4,FALSE
					/* send bAddr as the address to write */
1:	bal	I2C_Write8_amips
	move a2,s0

	beqz v0,2f
	nop
	li	t4,FALSE
					/* send the value to write */
2:	bal	I2C_Write8_amips
	move a2,s1

	beqz v0,3f
	nop
	li	t4,FALSE
					/* clean up the bus with a stop condition */
3:	bal	I2C_Stop_amips
	nop
					/* lock the system register */
	bal	I2C_Lock_amips
	nop
					/* restore ra */
	jal	rastack_pop
	nop
	move ra,v1
	.set reorder
					/* retrieve fStatus */
	move v0,t4
	j	ra
END(I2C_EEPROMWrite_amips)

/*!
 *********************************************************************
 **
 * Function: BYTE I2C_EEPROMRead_amips(V3USCHANDLE hV3, BYTE bSlaveAddr, BYTE bAddr, BYTE bData)
 **
 * Parameters:
 *  	-DWORD,V3USCHANDLE hV3,pointer to base address (physical)of USC
 * 		-BYTE bSlaveAddr - 3 bit address of device been selected
 * s0	-BYTE bAddr - address of EEPROM location to be read from
 * 
 **
 * Registers used:
 * a0	-DWORD dwReg, USC register @ offset from USC base address
 * a1	-over-written with intermediate data
 * a2
 * v0
 * v1
 * t4
 * t5
 **
 * Return: 
 * v0	-BYTE, data read from EEPROM
 **
 * Description:
 * This routine reads back a byte from the slave device's address
 * specificed.  This routine uses the Random Read method in the Atmel
 * documentation.
 **
 ********************************************************************/

LEAF(I2C_EEPROMRead_amips)
	li	t4,TRUE
	.set noreorder
					/* save ra, return address */
	move v1,ra
	jal	rastack_push
	nop
					/* Ensure the I2C bus is initialized */
	bal	I2C_Init_amips
	nop
					/* unlock the system register */
	bal	I2C_UnLock_amips
	nop	
					/* set upper device address */
	move v1,SLAVEADDR
	or	a2,v1,I2C_1010
					/* send device Address,wait for Ack,bAddr is sent as a write */  
	bal	I2C_PollAckStart_amips
	li	a1,I2C_WRITE

	beqz v0,1f
	nop
	li	t4,FALSE
					/* send bAddr as the address to write */
1:	bal	I2C_Write8_amips
	move a2,s0

	beqz v0,2f
	nop
	li	t4,FALSE
					/* send a START condition and device address this time as read */
						/* set upper device address */
2:	move v1,SLAVEADDR
	or	a2,v1,I2C_1010
	bal	I2C_StartSlave_amips
	li	a1,I2C_READ

	beqz v0,3f
	nop
	li	t4,FALSE
					/* read the data from the EEPROM */
3:	bal	I2C_Read8_amips
	nop
	move t5,v0		/* save data read from EEPROM */
					/* clean up the bus with a Nack and stop condition */
	bal	I2C_NoAck_amips
	nop
	bal	I2C_Stop_amips
	nop
					/* lock the system register */
	bal	I2C_Lock_amips
	nop
					/* restore ra */
	jal	rastack_pop
	nop
	move ra,v1
	.set reorder
					/* retrieve data read */
	move v0,t5
	j	ra
END(I2C_EEPROMRead_amips)

/*!
 *********************************************************************
 **
 * Function: IC2_Delay_amips( unsinged long delayvalue)
 **
 * Parameters:
 *  delayvalue:     1 = 1uSec
 *				 1000 = 1 millisecond.
 * 
 **
 * Return: NONE
 **
 * Description:
 *  Delay routine, a delay constant to be passed via C routine; 1 = 1 uSec.
 *                 or a delay constant loaded into reg a0 in assembly routine; 1 = 1 uSec.
 **
 ********************************************************************/

LEAF(I2C_Delay_amips)
	li	a0,0x05
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
END(I2C_Delay_amips)
