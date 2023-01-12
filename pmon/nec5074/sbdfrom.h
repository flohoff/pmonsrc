/*
 * NEC5074L/sbdfrom.h: onboard FROM support for NEC DDB-Vrc5074
 * Copyright (c) 1999 Algorithmics Ltd.
 */

/* Only Intel devices on this board */
#define FROM_FAMILY_INTEL	1

/* Fixed device geometry on this board */
#define FROM_WIDTH		2	/* dev width = word */
#define FROM_NCOLS		2	/* 2 devices in parallel */
#define FROM_GROUP		2	/* program 2 in parallel */

/* "Variable" device geometry */
#define FROM_NBANKS(fcp)	1
/*#define FROM_SECSIZE(fcp)*/
/*#define FROM_DEVSIZE(fcp)*/
/*#define FROM_DEVSIZE_LOG2(fcp)*/

/* Define to enable debug messages */
/*#define FROM_DEBUG*/

/* Define if code (in particular flash programming code) 
   will never executed be from the flash rom. */
/*#define FROM_NEVER_SELF_PROGRAM*/

