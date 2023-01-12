/*********************************************************************
 **
 * Module: V3USC32
 **
 * File Name: v3mipi2c.c
 **
 * Authors: Phil Sikora
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
 * $Revision: 1.1 $	$Date: 1999/04/22 14:46:04 $Revision: 1.1 $	$Date: 1999/04/22 14:46:04 $sbddelay(10) changed to (5)
 * $Revision: 1.1 $	$Date: 1999/04/22 14:46:04 $I2C_Delay now calls sbddelay()
 * $Revision: 1.1 $	$Date: 1999/04/22 14:46:04 $
 * $NoKeywords: $
 **
 * Description:
 **
 * This code has been adapted from James Flynn article in Embedded
 * Systems November 1997.  See www.embedded.com web site.
 **
 * For porting to other Operating Systems see the following three
 * routines at the end of this file, I2C_ReadRegSystem, I2C_WriteRegSystem
 * and the I2C_Delay routines.
 **
 ********************************************************************/

/*********************************************************************
 *
 * Include files that this module depends on.
 *
 ********************************************************************/

/* #include <windows.h>	*/
/* #include <winbase.h>	*/
#define GLOBAL_VARIABLES
#include "mips.h"
#include "sbd.h"
#include "v3mipusc.h"
#include "v3uscreg.h"
#include "v3mipi2c.h"



#define V3USC32_WriteRegByte(x,y,z) 	V3USC32_WriteRegByte_mips( x, y, z)
#define V3USC32_ReadRegByte(x,y)		V3USC32_ReadRegByte_mips( x, y )


BOOL fConfigCycle;
 


/*!
 *********************************************************************
 **
 * Function: void I2C_Init(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 **
 * Return:
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
 **
 ********************************************************************/
void I2C_Init(V3USCHANDLE hV3)
{
	static bOneTimeInit = FALSE;

	if(!bOneTimeInit)
	{
		/* Unlock the system register */
		I2C_UnLock(hV3);
		
		/* Ensure that the I2C clock and data lines are in a known state */
		I2C_Stop(hV3);
		I2C_Delay(hV3);
	
		/* Lock the system register */
		I2C_Lock(hV3);
	
		/* ensure once only */
		bOneTimeInit = TRUE;
	}
}

/*!
 *********************************************************************
 **
 * Function: BOOL I2C_EEPROMWrite(V3USCHANDLE hV3, BYTE bSlaveAddr BYTE bAddr, BYTE bData, BOOL fUseConfigCycle)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 * BYTE bSlaveAddr - 3 bit address of device been selected
 * BYTE bAddr - address of EEPROM location to be read from
 * BYTE bData - value to be written to EEPROM 
 * BOOL fUseConfigCycle - Use PCI configuration cycles to access EEPROM
 **
 * Return:
 * BOOL fStatus - TRUE if read successful
 **
 * Description:
 * This routine writes a byte to the slave device's address specified.
 * This routine uses the Write Byte method from the Atmel documentation.
 **
 ********************************************************************/
BOOL I2C_EEPROMWrite(V3USCHANDLE hV3, BYTE bSlaveAddr, BYTE bAddr, BYTE bData, BOOL fUseConfigCycle)
{
	BOOL fStatus = TRUE;

	/* Set Global flag to use PCI configuration cycles */
	fConfigCycle = fUseConfigCycle;

	/* Ensure the I2C bus is initialized */
	I2C_Init(hV3);

	/* Set upper device address */
	bSlaveAddr |= I2C_1010;

	/* Unlock the system register */
	I2C_UnLock(hV3);

	/* Send device Address and wait for Ack, bAddr is sent as a write */
	if ((I2C_PollAckStart(hV3, bSlaveAddr, I2C_WRITE)) != I2C_OK)
		fStatus = FALSE;
	else

		/* Send bAddr as the address to write */
		if (I2C_Write8(hV3, bAddr))
			fStatus = FALSE;
		else

			/* Send the value to write */
			if (I2C_Write8(hV3, bData))
				fStatus = FALSE;

	/* Clean up the bus with a stop condition */
	I2C_Stop(hV3);

	/* Lock the system register */
	I2C_Lock(hV3);
	return (fStatus);
}

/*!
 *********************************************************************
 **
 * Function: BOOL I2C_EEPROMWrite(V3USCHANDLE hV3, BYTE bSlaveAddr, BYTE bAddr, BYTE *bData, BOOL fUseConfigCycle)
 **
 * Input Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 * BYTE bSlaveAddr - 3 bit address of device been selected
 * BYTE bAddr - address of EEPROM location to be read from
 * BOOL fUseConfigCycle - Use PCI configuration cycles to access EEPROM
 **
 * Input Parameters:
 * BYTE *bData - EEPROM data read back (pointer to data passed)
 **
 * Return:
 * BOOL fStatus - TRUE if read successful
 **
 * Description:
 * This routine reads back a byte from the slave device's address
 * specificed.  This routine uses the Random Read method in the Atmel
 * documentation.
 **
 ********************************************************************/
BOOL I2C_EEPROMRead(V3USCHANDLE hV3, BYTE bSlaveAddr, BYTE bAddr, BYTE *bData, BOOL fUseConfigCycle)
{
	BOOL fStatus = TRUE;

	/* Set Global flag to use PCI configuration cycles */
	fConfigCycle = fUseConfigCycle;

	/* Ensure the I2C bus is initialized */
	I2C_Init(hV3);

	/* Set upper device address */
	bSlaveAddr |= I2C_1010;

	/* Unlock the system register */
	I2C_UnLock(hV3);

	/* Send device Address and wait for Ack, bAddr is sent as a write */
	if ((I2C_PollAckStart(hV3, bSlaveAddr, I2C_WRITE)) != I2C_OK)
		fStatus = FALSE;
	else

		/* Send bAddr as the random address to be read */
		if (I2C_Write8(hV3, bAddr))
			fStatus = FALSE;

		else
		/* Send a START condition and device address this time as read */
			if (I2C_StartSlave(hV3, bSlaveAddr, I2C_READ))
				fStatus = FALSE;

			else
				/* read the date back from EEPROM */
				*bData = I2C_Read8(hV3);

	/* clean up bus with a NaK and Stop condition */
	I2C_NoAck(hV3);
	I2C_Stop(hV3);

	/* Lock the system register */
	I2C_Lock(hV3);
	return (fStatus);
}

/*!
 *********************************************************************
 **
 * Function: int I2C_StartSlave(V3USCHANDLE hV3, register BYTE bAddr, bit ReadBit)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 * BYTE bAddr - 3 bit address of device been selected
 * bit ReadBit - Set if command is to be a read, reset if command is a write 
 **
 * Return:
 * I2C_OK    - read was successful
 * I2C_NOACK - if negative acknowledge was received
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
int I2C_StartSlave(V3USCHANDLE hV3, register BYTE bAddr, BYTE ReadBit)
{
	I2C_SCL(hV3, I2C_HIGH);
	I2C_Delay(hV3);
	I2C_SDA(hV3, I2C_LOW);
	I2C_Delay(hV3);
	I2C_SCL(hV3, I2C_LOW);
	I2C_Delay(hV3);
	return I2C_Write8(hV3, (BYTE) (bAddr<<1 | ReadBit));
}

/*!
 *********************************************************************
 **
 * Function: int I2C_PollAckStart(V3USCHANDLE hV3, register BYTE bAddr, bit ReadBit)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 * BYTE bAddr - 3 bit address of device been selected
 * bit ReadBit - Set if command is to be a read, reset if command is a write 
 **
 * Return:
 * I2C_BUSY  - device is not responding (retried out)
 * I2C_OK    - read was successful
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
int I2C_PollAckStart(V3USCHANDLE hV3, register BYTE bAddr, BYTE ReadBit)
{
	register BYTE bTries = I2C_RETRY;
	do
	{
		/* send start condition */
		I2C_SDA(hV3, I2C_LOW);
		I2C_Delay(hV3);
		I2C_SCL(hV3, I2C_LOW);
		I2C_Delay(hV3);
		if (I2C_Write8(hV3, (BYTE) (bAddr<<1 | ReadBit)) == I2C_OK)
			return (I2C_OK);

		/* setup for start condition if device busy */
		I2C_SCL(hV3, I2C_HIGH);
		I2C_Delay(hV3);
	}
	while(bTries--);
	return(I2C_BUSY);
}

/*!
 *********************************************************************
 **
 * Function: BYTE I2C_Read8(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 **
 * Return:
 * BYTE bData - returns a byte of data 
 **
 * Description:
 * Reads eight bits from the I2C bus.
 **
 ********************************************************************/
BYTE I2C_Read8(V3USCHANDLE hV3)
{
	register BYTE i, bData=0;

	I2C_SDA(hV3, I2C_HIGH);
	for (i=8; i; --i )
	{
		I2C_SCL(hV3, I2C_HIGH);
		I2C_Delay(hV3);
		bData = (bData<<1) | I2C_SDAIn(hV3);
		I2C_SCL(hV3, I2C_LOW);
		I2C_Delay(hV3);
	}
	return(bData);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_Write8(V3USCHANDLE hV3, register BYTE bData)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 **
 * Return:
 * BYTE bAck - returns the acknowledge bit 
 **
 * Description:
 * This routine writes out all 8 bits of data out to the I2C bus.  After 
 * writing out the data, the routine reads the ACK/NACK response back and
 * returns it to the caller.
 **
 ********************************************************************/
BYTE I2C_Write8(V3USCHANDLE hV3,register BYTE bData)
{
	register BYTE i;
	for (i=0x80; i; i >>=1 )
	{
		I2C_SDA(hV3, i & bData);
		I2C_SCL(hV3, I2C_HIGH);
		I2C_Delay(hV3);
		I2C_SCL(hV3, I2C_LOW);
		I2C_Delay(hV3);
	}
	I2C_SDA(hV3, I2C_HIGH);
	I2C_SCL(hV3, I2C_HIGH);
	I2C_Delay(hV3);
	i = I2C_SDAIn(hV3);
 	I2C_SCL(hV3, I2C_LOW);
	I2C_Delay(hV3);

	return(i);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_Stop(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 **
 * Return:
 **
 * Description:
 * All communications must be terminated by a stop condition which
 * is a low to high transition of SDA while SCL is high.  A stop 
 * condition can only be issued after the transmitting device has 
 * released the bus.
 **
 ********************************************************************/
void I2C_Stop(V3USCHANDLE hV3)
{
	I2C_SDA(hV3, I2C_LOW);
	I2C_Delay(hV3);
	I2C_SCL(hV3, I2C_HIGH);
	I2C_Delay(hV3);
	I2C_SDA(hV3, I2C_HIGH);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_NoAck(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 **
 * Return:
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

void I2C_NoAck(V3USCHANDLE hV3)
{
	I2C_SDA(hV3, I2C_HIGH);
	I2C_Delay(hV3);
	I2C_SCL(hV3, I2C_HIGH);
	I2C_Delay(hV3);
	I2C_SCL(hV3, I2C_LOW);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_Ack(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 **
 * Return:
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

void I2C_Ack(V3USCHANDLE hV3)
{
	I2C_SDA(hV3, I2C_LOW);
	I2C_Delay(hV3);
	I2C_SCL(hV3, I2C_HIGH);
	I2C_Delay(hV3);
	I2C_SCL(hV3, I2C_LOW);
}

/*!
 *********************************************************************
 **
 * Function: BYTE I2C_SDAIn(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 **
 * Return:
 * BYTE bData - state of the Serial Data line
 **
 * Description:
 * This routine returns the state of the Serial Data line
 * Test the global PCI configuration flag to use PCI configuration
 * cycles to access the EEPROM.
 **
 ********************************************************************/
BYTE I2C_SDAIn(V3USCHANDLE hV3)
{
	BYTE System;

	System = I2C_ReadRegSystem(hV3);
	return ((System & SYSTEM_B_SDA_IN) >> SYSTEM_B_SDA_IN_SHIFT);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_SDA(V3USCHANDLE hV3, int State)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 * int state - what state the SDA is to be set to
 **
 * Return:
 **
 * Description:
 * This routine sets the Serial Data line to the desired state.
 * Test the global PCI configuration flag to use PCI configuration
 * cycles to access the EEPROM.
 **
 ********************************************************************/

void I2C_SDA(V3USCHANDLE hV3, int State)
{
	BYTE System;

	System = I2C_ReadRegSystem(hV3);
	if(State)
		System |= SYSTEM_B_SDA_OUT;
	else
		System &= ~SYSTEM_B_SDA_OUT;
	I2C_WriteRegSystem(hV3, System);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_SCL(V3USCHANDLE hV3, int State)
 **
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 * int state - what state the SCL is to be set to
 **
 * Return:
 **
 * Description:
 * This routine sets the Serial Clock line to the desired state.
 * Test the global PCI configuration flag to use PCI configuration
 * cycles to access the EEPROM.
 **
 ********************************************************************/

void I2C_SCL(V3USCHANDLE hV3, int State)
{
	BYTE System;

	System = I2C_ReadRegSystem(hV3);
	if(State)
		System |= SYSTEM_B_SCL;
	else
		System &= ~SYSTEM_B_SCL;
	I2C_WriteRegSystem(hV3, System);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_Lock(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 **
 * Return:
 **
 * Description:
 * This routine disables the Serial Clock pin, and sets the sytem
 * lock bit.
 * Test the global PCI configuration flag to use PCI configuration
 * cycles to access the EEPROM.
 **
 ********************************************************************/

void I2C_Lock(V3USCHANDLE hV3)
{
	BYTE System;

	System = I2C_ReadRegSystem(hV3);
	I2C_WriteRegSystem(hV3, (BYTE) (System & ~SYSTEM_B_SPROM_EN));
	I2C_WriteRegSystem(hV3, (BYTE) (System | SYSTEM_B_LOCK));
}

/*!
 *********************************************************************
 **
 * Function: void I2C_UnLock(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 **
 * Return:
 **
 * Description:
 * This routine enable the sytem register for writting, and enables
 * the Serial Clock output pin.
 * Test the global PCI configuration flag to use PCI configuration
 * cycles to access the EEPROM.
 **
 ********************************************************************/
void I2C_UnLock(V3USCHANDLE hV3)
{
	BYTE System;

	I2C_WriteRegSystem(hV3, SYSTEM_B_UNLOCK_TOKEN);
	System = I2C_ReadRegSystem(hV3);
	I2C_WriteRegSystem(hV3, (BYTE) (System |SYSTEM_B_SPROM_EN));
}

/*!
 *********************************************************************
 **
 * Function: BYTE I2C_ReadRegSystem(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 **
 * Return:
 **
 * BYTE bSystem - system register contents
 **
 * Description:
 * This routine handles the physical read of the USC system register
 * Both I/O - Memory mapped and PCI configuration cycles are supported
 * Port this routine to your Operating System.
 **
 ********************************************************************/
BYTE I2C_ReadRegSystem(V3USCHANDLE hV3)
{

	return( V3USC_SYSTEM_B ); 
}

/*!
 *********************************************************************
 **
 * Function: void I2C_WriteRegSystem(V3USCHANDLE hV3, BYTE bSystem )
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 * BYTE bSystem - contents to write to USC system register
 **
 * Return:
 **
 * Description:
 * This routine handles the physical write of the USC system register
 * Both I/O - Memory mapped and PCI configuration cycles are supported
 * Port this routine to your Operating System.
 **
 ********************************************************************/
void I2C_WriteRegSystem(V3USCHANDLE hV3, BYTE bSystem)
{

	V3USC_SYSTEM_B = bSystem;
}

/*!
 *********************************************************************
 **
 * Function: void I2C_Delay(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 **
 * Return:
 **
 * Description:
 * This routine handles the 5 microsecond delay
 * Port this routine to your Operating System.
 **
 ********************************************************************/
void I2C_Delay(V3USCHANDLE hV3)
{
	/* Timer_I2CDelay(hV3);	 */
	sbddelay(5);					/* 5 usec delay */
									/* in file sbdreset.S */
}

