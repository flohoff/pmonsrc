/*********************************************************************
 **
 * Module: V3USC
 **
 * File Name: v3mipusc.h
 **
 * Authors: Raymond Hong
 **
 * Copyright (c) 1998 V3 Semiconductor. All rights reserved.
 *
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
 * $Revision: 1.1 $	$Date: 1999/04/22 14:46:05 $NoKeywords: $
 **
 * Description:
 **
 ********************************************************************/
 /*
#ifndef _V3MEM_H_
#define _V3MEM_H_
  */
/* #ifdef __cplusplus
extern "C" {
#endif
 */
/*
 * If the global variables are to be instantiated in a specific module
 * then GLOBAL_VARIABLES is define just prior to including this file.
 * The GLOBAL_VARIABLES variable is promptly undefined to prevent other
 * modules from instantiating their global variables.
 * The _GLOBAL_VARIABLES_V3MEM_H_ define can then be used to istantiate
 * and global variables.
 * LOCAL_EXTERN is a macro for extern or nothing depending on how this include
 * file is called the variable will be accessible.
 */
/*
#undef LOCAL_EXTERN
#ifdef GLOBAL_VARIABLES
	#undef GLOBAL_VARIABLES
	#define _GLOBAL_VARIABLES_V3MEM_H_
	#define LOCAL_EXTERN
#else /* GLOBAL VARIABLES */
/*
	#define LOCAL_EXTERN extern
#endif /* GLOBAL_VARIABLES */

/*********************************************************************
 **
 * Include files that this include file depends on.
 **
 ********************************************************************/


/*********************************************************************
 **
 * Equates
 **
 ********************************************************************/

#define TRUE 1
#define FALSE 0

/*********************************************************************
 **
 * Data Structures
 **
 ********************************************************************/

typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef unsigned short WORD;
#ifndef _Windows_
typedef int BOOL;
#endif

typedef DWORD *	 V3USCHANDLE;


/*********************************************************************
 **
 * Equates 2
 **
 ********************************************************************/


/*********************************************************************
 **
 * Global Variables with scope of only the module that defined GLOBAL_VARIABLES
 **
 ********************************************************************/

#ifdef _GLOBAL_VARIABLES_V3MEM_H_
#endif /* _GLOBAL_VARIABLES_V3MEM_H_ */

/*********************************************************************
 **
 * Global Variables to be exported from this module 
 **
 ********************************************************************/


/*********************************************************************
 **
 * Prototypes
 **
 ********************************************************************/

void V3USC_WriteRegByte_mips(V3USCHANDLE hV3, DWORD dwReg, BYTE data);
BYTE V3USC_ReadRegByte_mips(V3USCHANDLE hV3, DWORD dwReg);
void V3USC_WriteConfigByte_mips(V3USCHANDLE hV3, DWORD dwReg, BYTE data);
BYTE V3USC_ReadConfigByte_mips(V3USCHANDLE hV3, DWORD dwReg);
void v3eeput( unsigned int offs, unsigned short val);
unsigned short v3eeget( unsigned int offs );

 
