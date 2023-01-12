/*
 * TEMIC/sbdfrom.h: onboard FROM support for Algor/Temic module
 * Copyright (c) 1999 Algorithmics Ltd.
 */

/* Board only supports AMD-style devices */
#define FROM_FAMILY_AMD		1
#define FROM_FAMILY_AM29DL	1

/* Fixed device geometry on this board */
#define FROM_WIDTH		2	/* dev width = word (16 bits) */
#define FROM_NCOLS		2	/* 2 devices in parallel (32 bits) */
#define FROM_GROUP		2	/* program 2 in parallel (32 bits) */

/* "Variable" device geometry */
/*#define FROM_SECSIZE(fcp)*/
/*#define FROM_DEVSIZE(fcp)*/
/*#define FROM_DEVSIZE_LOG2(fcp)*/
/*#define FROM_NBANKS(fcp)*/

/* Define to enable debug messages */
/*#define FROM_DEBUG*/

/* Defined if code (in particular flash programming code) 
   will never executed be from the flash rom. */
/*#define FROM_NEVER_SELF_PROGRAM*/
