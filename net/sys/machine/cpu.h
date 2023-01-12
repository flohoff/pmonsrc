/* $Id: cpu.h,v 1.3 1996/12/09 20:11:00 nigel Exp $ */
#if defined(R4000)
#include "r4000.h"
#elif defined(R3000)
#include "r3000.h"
#else
#error You must define CPU_R3000 or CPU_R4000
#endif
