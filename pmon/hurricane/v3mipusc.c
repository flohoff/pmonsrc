/*********************************************************************
 **
 * Module: V3USC
 **
 * File Name: v3mipusc.c
 **
 * Authors: Raymond Hong
 **
 * Copyright (c) 1998 V3 Semiconductor. All rights reserved.
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
 * a05 30/11/1998  6:00p rh -EDIT V3USC_WriteRegByte_mips(),V3USC_ReadRegByte_mips().
 * a04 23/11/1998 10:00a rh -EDIT V3USC_WriteRegByte_mips().
 * a03 13/11/1998  3:45p rh -hard coded 0x73 in V3USC_WriteRegByte_mips(),V3USC_ReadRegByte_mips().
 * a02 01/oct/1998 12:00p rh -v3eeput() delay 1msec between byte writes.
 * $Revision: 1.1 $	$Date: 1999/04/22 14:46:04 $Create
 * $NoKeywords: $
 **
 * Description:
 **
 * This module contains the interface routines to the serial EEPROM.
 **
 ********************************************************************/

/*********************************************************************
 *
 * Include files that this module depends on.
 *
 ********************************************************************/

/* #define GLOBAL_VARIABLES	  */
#include "sbd.h"
#include "v3mipusc.h"

#define I2CSLAVEADR	4
#define NONCONFIGCYCLE 0

/*!
 *********************************************************************
 **
 * Function: void V3USC_WriteRegByte_mips(V3USCHANDLE hV3, DWORD dwReg, BYTE data)
 **
 * Parameters:
 * V3USCHANDLE hV3 - pointer to USC base address
 * DWORD dwReg - USC register @ offset from USC base address
 * BYTE data - data to be written to USC register
 **
 * Return:
 **
 * Description:
 *  Write a byte to a USC register
 **
 ********************************************************************/

void V3USC32_WriteRegByte_mips(V3USCHANDLE hV3, DWORD dwReg, BYTE data)
{
	/* volatile BYTE *uscptr = PA_TO_KVA1((DWORD)hV3) + dwReg; */	 /* dwReg */
	volatile BYTE *uscptr = (BYTE *)dwReg; 	 /* dwReg */

	*uscptr = data;	

															 /* V3DEBUG, 01,oct,1998 */
	/* printf("dwReg = 0x%08x    data = 0x%08x\n", dwReg, data);   */
}
			

/*!
 *********************************************************************
 **
 * Function: BYTE V3USC_ReadRegByte_mips(V3USCHANDLE hV3, DWORD dwReg)
 **
 * Parameters:
 * V3USCHANDLE hV3 - pointer to base address of USC
 * DWORD dwReg - USC register @ offset from USC base address
 * 
 **
 * Return: data.
 **
 * Description:
 *  Read 1 byte from register in USC
 **
 ********************************************************************/

BYTE V3USC32_ReadRegByte_mips(V3USCHANDLE hV3, DWORD dwReg)
{
	BYTE data;
	/* volatile BYTE *uscptr = PA_TO_KVA1((DWORD)hV3 + 0x73); */	/* dwReg */
	volatile BYTE *uscptr = (BYTE *)dwReg; 	/* dwReg */

	data = *uscptr; 

	return( data);
}

/*!
 *********************************************************************
 **
 * Function: void V3USC_WriteConfigByte_mips(V3USCHANDLE hV3, DWORD dwReg, BYTE data)
 **
 * Parameters:
 * V3USCHANDLE hV3 - pointer to base address of USC
 * DWORD dwReg - USC register @ offset from USC base address
 * BYTE data -  data (1 byte) to be written 
 **
 * Return:
 **
 * Description:
 *  Write 1 byte to USC using configuration cycles.
 **
 ********************************************************************/

void V3USC32_WriteConfigByte(V3USCHANDLE hV3, DWORD dwReg, BYTE data)
{
  /* presently not required by the local processor */
}

/*!
 *********************************************************************
 **
 * Function: BYTE V3USC_ReadConfigByte_mips(V3USCHANDLE hV3, DWORD dwReg)
 **
 * Parameters:
 * V3USCHANDLE hV3 - pointer to base address of USC
 * DWORD dwReg - USC register @ offset from USC base address
 * 
 **
 * Return: 1 byte data.
 **
 * Description:
 *  Read 1 byte from USC using configuration cycles.
 **
 ********************************************************************/

BYTE V3USC32_ReadConfigByte(V3USCHANDLE hV3, DWORD dwReg)
{
	/* presently not required by local processor */
	return(1);
}

/*!
 *********************************************************************
 **
 * Function: unsigned short v3eeget( unsigned int offs )
 **
 * Input Parameters:
 * unsigned int offs - address offset in EEPROM where data is to be read
 * 
 **
 * Output Parameters:
 *
 **
 * Return: Integer (2 bytes) read from EEPROM  
 **
 * Description:
 *  Get integer (2 consecutive bytes) from EEPROM. 
 **
 ********************************************************************/
unsigned short v3eeget( unsigned int offs )
{
	unsigned short *iptr, ivalue;
	unsigned char rdvalue[4], *rddataptr;
	unsigned char coffs;
	V3USCHANDLE hV3 = (DWORD *)V3USC_BASE;

									/* check offset (offset in EEPROM) */ 
	if( offs <= 0xff )
		coffs = (unsigned char)offs;
	else
	{
		printf("ERROR: EEPROM offset is greater than 0xFF\n");
		printf("  Set offset to 0xFF.\n");
		coffs = 0xff;
	}

	rddataptr = rdvalue;
									/* read lower byte from EEPROM */
	I2C_EEPROMRead(hV3,I2CSLAVEADR,coffs,rddataptr,NONCONFIGCYCLE);

	rddataptr = rdvalue+1;
									/* read upper byte from EEPROM */
	I2C_EEPROMRead(hV3,I2CSLAVEADR,coffs+1,rddataptr,NONCONFIGCYCLE);

									/* combine 2 bytes into one integer */
	iptr = (unsigned short *)rdvalue;
	ivalue = *iptr;

	return(ivalue);

} /* end v3eeget() */

/*!
 *********************************************************************
 **
 * Function: void v3eeput( unsigned int offs, unsigned short val)
 **
 * Input Parameters:
 * unsigned int offs - address offset in EEPROM where data is to be written
 * unsigned short val - 16-bit data to be written. 
 * 
 **
 * Output Parameters:
 * 
 **
 * Description:
 *  Write integer (2 bytes) to EEPROM. 
 **
 ********************************************************************/
 
void v3eeput( unsigned int offs, unsigned short val)
{

	unsigned char *cary;
	unsigned char valuelower, valueupper;
	unsigned char coffs;
	V3USCHANDLE hV3 =(DWORD *)V3USC_BASE;
									/* split integer into 2 bytes */

	printf("offs = 0x%04x    val = 0x%08x\n", offs, val);	   /* V3DEBUG, 01,oct,1998 */
	cary = (unsigned char *)(&val);
	valuelower = cary[0];
	valueupper = cary[1];

	printf("valuelower = 0x%04x   valueupper = 0x%04x\n", valuelower, valueupper);	/* V3DEBUG, 01,oct,1998 */

									/* check offset (offset in EEPROM) */ 
	if( offs <= 0xff )
		coffs = (unsigned char)offs;
	else
	{
		printf("ERROR: EEPROM offset is greater than 0xFF\n");
		printf("  Set offset to 0xFF.\n");
		coffs = 0xff;
	}
									/* write lower byte to EEPROM */
	I2C_EEPROMWrite(hV3,I2CSLAVEADR,coffs,valuelower,NONCONFIGCYCLE);
									/* write upper byte to EEPROM */
	sbddelay(1500);					/* 1.5 msec */

	I2C_EEPROMWrite(hV3,I2CSLAVEADR,coffs+1,valueupper,NONCONFIGCYCLE);

} /* end v3eeput() */

