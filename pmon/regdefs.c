/* $Id: regdefs.c,v 1.14 2001/08/12 14:32:30 chris Exp $ */
#include "string.h"
#include "pmon.h"

/*************************************************************
 *  regdefs.c
 *      This file contains all the register definitions used
 *      by the 'r' (display regs) command. There are two types
 *      of definitions: RegList that defines all the registers, 
 *      and RegSpec's that define the bit fields within a register.
 */

#ifdef LR33000
const char * const bsvalues[] =
{"2", "4", "8", "16", 0};
const char * const pgszvalues[] =
{"64", "128", "256", "512", "1024", "2048", "4096", "8192", 0};

const RegSpec         Cfgreg33000[] =
{
    {1, 23, "ICD", 2, 0, 0},
    {1, 22, "DCD", 2, 0, 0},
    {2, 20, "IBS", 0, bsvalues, 0},
    {2, 18, "DBS", 0, bsvalues, 0},
    {1, 17, "IW", 2, 0, 0},
    {4, 13, "IWAIT", 10, 0, 0},
    {1, 12, "PW", 2, 0, 0},
    {4, 8, "PWAIT", 10, 0, 0},
    {1, 7, "CS", 2, 0, 0},
    {1, 6, "PE", 2, 0, 0},
    {1, 5, "DGE", 2, 0, 0},
    {1, 4, "BFD", 2, 0, 0},
    {1, 3, "REN", 2, 0, 0},
    {1, 2, "RPC", 2, 0, 0},
    {1, 1, "CL", 2, 0, 0},
    {1, 0, "DCE", 2, 0, 0},
    {0}};

const RegSpec         Cfgreg33050[] =
{
    {1, 31, "WBE", 2, 0, 0},
    {1, 30, "BEN", 2, 0, 0},
    {3, 27, "PGSZ", 0, pgszvalues, 0},
    {1, 25, "IW8", 2, 0, 0},
    {1, 24, "PW8", 2, 0, 0},
    {1, 23, "ICD", 2, 0, 0},
    {1, 22, "DCD", 2, 0, 0},
    {2, 20, "IBS", 0, bsvalues, 0},
    {2, 18, "DBS", 0, bsvalues, 0},
    {1, 17, "IW", 2, 0, 0},
    {4, 13, "IWAIT", 10, 0, 0},
    {1, 12, "PW", 2, 0, 0},
    {4, 8, "PWAIT", 10, 0, 0},
    {1, 7, "CS", 2, 0, 0},
    {1, 6, "PE", 2, 0, 0},
    {1, 5, "DGE", 2, 0, 0},
    {1, 4, "BFD", 2, 0, 0},
    {1, 3, "REN", 2, 0, 0},
    {1, 2, "RPC", 2, 0, 0},
    {1, 1, "CL", 2, 0, 0},
    {1, 0, "DCE", 2, 0, 0},
    {0}};


const RegSpec         Cfgreg33020[] =
{
    {1, 30, "BEN", 2, 0, 0},
    {3, 27, "PGSZ", 0, pgszvalues, 0},
    {1, 23, "ICD", 2, 0, 0},
    {1, 22, "DCD", 2, 0, 0},
    {2, 20, "IBS", 0, bsvalues, 0},
    {2, 18, "DBS", 0, bsvalues, 0},
    {1, 17, "IW", 2, 0, 0},
    {4, 13, "IWAIT", 10, 0, 0},
    {1, 12, "PW", 2, 0, 0},
    {4, 8, "PWAIT", 10, 0, 0},
    {3, 5, "BNK", 10, 0, 0},
    {1, 4, "BFD", 2, 0, 0},
    {1, 3, "REN", 2, 0, 0},
    {1, 2, "RPC", 2, 0, 0},
    {1, 1, "CL", 2, 0, 0},
    {1, 0, "DCE", 2, 0, 0},
    {0}};

const RegSpec         c2pscomm[] =
{				/* creg25 */
    {1, 7, "IO", 2, 0, 0},
    {1, 4, "CLKINH", 2, 0, 0},
    {1, 3, "RCVINT", 2, 0, 0},
    {1, 2, "TXINT", 2, 0, 0},
    {1, 1, "TXEN", 2, 0, 0},
    {1, 0, "INTHTX", 2, 0, 0},
    {0}};

const RegSpec         c2psstat[] =
{				/* creg24 */
    {1, 7, "FERR", 2, 0, 0},
    {1, 6, "PAR", 2, 0, 0},
    {1, 5, "RXIN", 2, 0, 0},
    {1, 4, "RXBF", 2, 0, 0},
    {1, 3, "TXBE", 2, 0, 0},
    {1, 2, "TXIN", 2, 0, 0},
    {1, 1, "CLKX", 2, 0, 0},
    {1, 0, "CLK", 2, 0, 0},
    {0}};

const RegSpec         c2config[] =
{				/* creg9 */
    {1, 19, "HCUR", 2, 0, 0},
    {1, 18, "DRAM", 2, 0, 0},
    {1, 17, "SAM", 2, 0, 0},
    {1, 16, "VRAM", 2, 0, 0},
    {1, 15, "D3", 2, 0, 0},
    {3, 12, "IORV3", 10, 0, 0},
    {4, 8, "IOWAIT3", 10, 0, 0},
    {1, 7, "D2", 2, 0, 0},
    {3, 4, "IORV2", 10, 0, 0},
    {4, 0, "IOWAIT2", 10, 0, 0},
    {0}};

const RegSpec         c2vhwconfig[] =
{				/* dreg23 */
    {1, 18, "CROSSINV", 2, 0, 0},
    {1, 17, "CROSSDAT", 2, 0, 0},
    {1, 16, "CROSSEN", 2, 0, 0},
    {1, 15, "CSYNCEN", 2, 0, 0},
    {1, 14, "OVRSCN", 2, 0, 0},
    {1, 13, "VEND", 2, 0, 0},
    {1, 12, "VSYNCOUTEN", 2, 0, 0},
    {1, 11, "VSYNCINEN", 2, 0, 0},
    {1, 10, "HWCRSR", 2, 0, 0},
    {2, 8, "SHFT", 10, 0, 0},
    {1, 7, "VSPOS", 2, 0, 0},
    {1, 6, "HSPOS", 2, 0, 0},
    {1, 5, "VSINT", 2, 0, 0},
    {1, 4, "HLINT", 2, 0, 0},
    {1, 1, "VLINTEN", 2, 0, 0},
    {1, 0, "HLINTEN", 2, 0, 0},
    {0}};

const RegSpec         c2gcpcntrl[] =
{				/* creg4 */
    {2, 13, "SPCLWE", 10, 0, 0},
    {1, 12, "YDIR", 2, 0, 0},
    {1, 11, "XDIR", 2, 0, 0},
    {1, 9, "WO", 2, 0, 0},
    {1, 8, "MW", 2, 0, 0},
    {1, 6, "MASK", 2, 0, 0,},
    {1, 4, "EXPND", 2, 0, 0,},
    {1, 3, "TRAN", 2, 0, 0,},
    {3, 0, "PIXSIZ", 10, 0, 0},
    {0}};

#endif /* LR33000 */

const char * const rmvalues[] =
{"RN", "RZ", "RP", "RM", 0};

const RegSpec         cp1_csr[] =
{
#ifdef R4000
    {1, 24, "FS", 2, 0, 0},
#endif
    {1, 23, "C", 2, 0, 0},
    {1, 17, "CE", 2, 0, 0},
    {1, 16, "CV", 2, 0, 0},
    {1, 15, "CZ", 2, 0, 0},
    {1, 14, "CO", 2, 0, 0},
    {1, 13, "CU", 2, 0, 0},
    {1, 12, "CI", 2, 0, 0},
    {1, 11, "EV", 2, 0, 0},
    {1, 10, "EZ", 2, 0, 0},
    {1, 9, "EO", 2, 0, 0},
    {1, 8, "EU", 2, 0, 0},
    {1, 7, "EI", 2, 0, 0},
    {1, 6, "FV", 2, 0, 0},
    {1, 5, "FZ", 2, 0, 0},
    {1, 4, "FO", 2, 0, 0},
    {1, 3, "FU", 2, 0, 0},
    {1, 2, "FI", 2, 0, 0},
    {2, 0, "RM", 0, rmvalues, 0},
    {0}};

#ifdef LR33000
const RegSpec         TimerCnl[] =
{
    {1, 2, "CE", 2, 0, 0},
    {1, 1, "IE", 2, 0, 0},
    {1, 0, "INT", 2, 0, 0},
    {0}};
#endif /* LR33000 */

#ifdef R4000
const char * const ksuvalues[] =
{"kern", "supv", "user", "????", 0};
#endif

const RegSpec         Stat[] =
{
#ifdef R4000
    {1, 31, "XX", 2, 0, 0, F_5000|F_7000|F_5400},
#endif
    {4, 28, "CU", 2, 0, 0},
#ifdef R4000
    {1, 27, "RP", 2, 0, 0},
    {1, 26, "FR", 2, 0, 0},
    {1, 25, "RE", 2, 0, 0},
    {1, 24, "ITS", 2, 0, 0, F_ALL & ~F_4650},
    {1, 24, "DL", 2, 0, 0, F_4650},
    {1, 23, "IL", 2, 0, 0, F_4650},
    {1, 22, "BEV", 2, 0, 0},
    {1, 21, "TS", 2, 0, 1, F_ALL & ~F_4650},
    {1, 20, "SR", 2, 0, 0},
    {1, 18, "CH", 2, 0, 1},
    {1, 17, "CE", 2, 0, 0},
    {1, 16, "DE", 2, 0, 0},
    {8, 8, "IM", 2, 0, 0},
    {1, 7, "KX", 2, 0, 0},
    {1, 6, "SX", 2, 0, 0},
    {1, 5, "UX", 2, 0, 0},
    {2, 3, "KSU", 0, ksuvalues, 0},
    {1, 2, "ERL", 2, 0, 1},
    {1, 1, "EXL", 2, 0, 1},
    {1, 0, "IE", 2, 0, 0},
#else
    {1, 25, "RE", 2, 0, 0},
    {1, 22, "BEV", 2, 0, 0},
    {1, 21, "TS", 2, 0, 1},
    {1, 20, "PE", 2, 0, 1},
    {1, 19, "CM", 2, 0, 0},
    {1, 18, "PZ", 2, 0, 0},
    {1, 17, "SWC", 2, 0, 0},
    {1, 16, "ISC", 2, 0, 0},
    {8, 8, "IM&SW", 2, 0, 0},
    {1, 5, "KUo", 2, 0, 0},
    {1, 4, "IEo", 2, 0, 0},
    {1, 3, "KUp", 2, 0, 0},
    {1, 2, "IEp", 2, 0, 0},
    {1, 1, "KUc", 2, 0, 0},
    {1, 0, "IEc", 2, 0, 0},
#endif
    {0}};

const char * const excodes[] =
{
    "Int",  "MOD",  "TLBL", "TLBS",
    "AdEL", "AdES", "IBE",  "DBE",
    "Sys",  "Bp",   "RI",   "CpU", 
    "Ovf", 
#ifdef R4000
            "Trap", "VCEI", "FPE", 
    "Cp2",  "Resv", "Resv", "Resv",
    "Resv", "Resv", "Resv", "Wtch",
    "Resv", "Resv", "Resv", "Resv",
    "Resv", "Resv", "Resv", "VCED",
#else
            "Resv", "Resv", "Resv", 
#endif
    0};


#ifdef R4000
const char * const excodes_r4650[] =
{
    "Int",  "Resv", "IBnd", "DBnd",
    "AdEL", "AdES", "IBE",  "DBE",
    "Sys",  "Bp",   "RI",   "CpU", 
    "Ovf",  "Trap", "Resv", "FPE", 
    "Resv", "Resv", "Resv", "Resv",
    "Resv", "Resv", "Resv", "Wtch",
    "Resv", "Resv", "Resv", "Resv",
    "Resv", "Resv", "Resv", "Resv",
    0};
#endif

const RegSpec         Caus[] =
{
    {1, 31, "BD", 2, 0, 1},
    {2, 28, "CE", 10, 0, 1},
    {1, 25, "DW", 2, 0, 1, F_4650},
    {1, 24, "IW", 2, 0, 1, F_4650},
    {1, 23, "IV", 2, 0, 0, F_4650},
    {1, 26, "W2", 2, 0, 1, F_7000},
    {1, 25, "W1", 2, 0, 1, F_7000},
    {1, 24, "IV", 2, 0, 0, F_7000},
    {16, 8, "IP", 2, 0, 0, F_7000},
    {8, 8, "IP", 2, 0, 0, F_ALL & ~F_7000},
    {4, 2, "EXCODE", 0, excodes, 1, F_ALL & ~F_4650},
#ifdef R4000
    {4, 2, "EXCODE", 0, excodes_r4650, 1, F_4650},
#endif
    {0}};

#ifdef R4000
/* RM7000 interrupt control register */
const RegSpec         Icrspec[] =
{
    {8, 8, "IM", 2, 0, 0},
    {1, 7, "TE", 1, 0, 0},
    {5, 0, "VS", 16, 0, 0},
    {0}};

const RegSpec         Infospec[] =
{
    {1, 28, "BM", 2, 0, 1, 0},
    {2, 26, "DS", 10, 0, 1, 0},
    {1, 25, "NP", 2, 0, 1, 0},
    {2, 23, "WP", 10, 0, 1, 0},
    {6, 17, "DW", 10, 0, 1, 0},
    {6, 11, "IW", 10, 0, 1, 0},
    {6,  5, "SW", 10, 0, 1, 0},
    {4,  1, "SS", 10, 0, 1, 0},
    {1,  0, "AE", 2, 0, 1, 0},
    {0}};

#endif

const RegSpec         PRid[] =
{
    {8, 8, "IMP", 10, 0, 1},
    {8, 0, "Rev", 10, 0, 1},
    {0}};

#ifdef LR33000
const RegSpec         DCIC[] =
{
    {1, 31, "TR", 2, 0, 0},
    {1, 30, "UD", 2, 0, 0},
    {1, 29, "KD", 2, 0, 0},
    {1, 28, "TE", 2, 0, 0},
    {1, 27, "DW", 2, 0, 0},
    {1, 26, "DR", 2, 0, 0},
    {1, 25, "DAE", 2, 0, 0},
    {1, 24, "PCE", 2, 0, 0},
    {1, 23, "DE", 2, 0, 0},
    {1, 13, "D", 2, 0, 0},
    {1, 12, "I", 2, 0, 0},
    {1, 5, "T", 2, 0, 0},
    {1, 4, "W", 2, 0, 0},
    {1, 3, "R", 2, 0, 0},
    {1, 2, "DA", 2, 0, 0},
    {1, 1, "PC", 2, 0, 0},
    {1, 0, "DG", 2, 0, 0},
    {0}};
#endif /* LR33000 */

#ifdef R4000
const char * const epvalues[] = {
    "DD", "DDx", "DDxx", "DxDx", "DDxxx", "DDxxxx", "DxxDxx", "DDxxxxx",
    "DxxxDxxx", "??", "??", "??", "??", "??", "??", "??", 0};

const char * const ssvalues_5000[] = {
    "512Kb", "1Mb", "2Mb", "None", 0
};

const char * const coherencyvalues[] = {
    "wthr/na", "wthr/a", "uncd", "!chrnt",
    "excl", "excl/w", "updt", "rsvd",
    0};

const RegSpec         Cfg[] =
{
    {1, 31, "CM", 2, 0, 1, F_ALL & ~(F_5000|F_7000)},
    {1, 31, "SC", 2, 0, 1, F_7000},
    {3, 28, "EC", 10, 0, 1, F_ALL},
    {4, 24, "EP", 0, epvalues, 1, F_ALL},
    {2, 22, "SB", 10, 0, 1, F_ALL & ~F_4100},
    {1, 23, "AD", 10, 0, 1, F_4100},
    {1, 21, "SS", 2, 0, 1, F_ALL & ~(F_5000 | F_7000)},
    {1, 20, "SW", 2, 0, 1, F_ALL & ~(F_5000 | F_7000)},
    {2, 20, "SS", 0, ssvalues_5000, 1, F_5000},
    {2, 20, "PI", 10, 0, 1, F_7000},
    {2, 18, "EW", 10, 0, 1, F_ALL},
    {1, 17, "SC", 2, 0, 1, F_ALL & ~F_7000},
    {1, 17, "TC", 2, 0, 1, F_7000},
    {1, 16, "SM", 2, 0, 1, F_ALL},
    {1, 15, "BE", 2, 0, 1, F_ALL},
    {1, 14, "EM", 2, 0, 1, F_ALL},
    {1, 13, "EB", 2, 0, 1, F_ALL},
    {1, 12, "CS", 2, 0, 1, F_4100},
    {1, 12, "SE", 2, 0, 1, F_5000},
    {1, 12, "TE", 2, 0, 1, F_7000},
    {3, 9, "IC", 10, 0, 1, F_ALL},
    {3, 6, "DC", 10, 0, 1, F_ALL},
    {1, 5, "IB", 2, 0, 1, F_ALL},
    {1, 4, "DB", 2, 0, 1, F_ALL},
    {1, 3, "CU", 2, 0, 0, F_ALL & ~(F_7000 | F_5000)},
    {1, 3, "SE", 2, 0, 1, F_7000},
    {3, 0, "K0", 16, 0, 0, F_ALL},
    {0}};

const RegSpec         CErr[] =
{
    {1, 31, "ER", 2, 0, 1},
    {1, 30, "EC", 2, 0, 1},
    {1, 29, "ED", 2, 0, 1},
    {1, 28, "ET", 2, 0, 1},
    {1, 27, "ES", 2, 0, 1},
    {1, 26, "EE", 2, 0, 1},
    {1, 25, "EB", 2, 0, 1},
    {1, 24, "EI", 2, 0, 1},
    {1, 23, "EW", 2, 0, 1},
    {22, 0, "Sidx", 16, 0, 1},	/* cheat lsb to get unshifted address */
    {3, 0, "Pidx", 16, 0, 1},
    {0}};
#endif

#ifdef R3081
const RegSpec         Cfg[] =
{
    {1, 31, "LOCK", 2, 0, 0},
    {1, 30, "SLOW", 2, 0, 0},
    {1, 29, "DBR", 2, 0, 0},
    {3, 26, "FPINT", 10, 0, 0},
    {1, 25, "HALT", 2, 0, 0},
    {1, 24, "RF", 2, 0, 0},
    {1, 23, "AC", 2, 0, 0},
    {0}};
#endif

#ifdef R3041
const RegSpec         CCfg[] =
{
    {1, 31, "LOCK", 2, 0, 0},
    {1, 29, "DBR", 2, 0, 0},
    {1, 19, "FDM", 2, 0, 0},
    {0}};

const char * const ctl_values[] = {
    "high", "wr", "rd", "rdwr", 0};

const char * const bta_values[] = {
    "0.5", "1.5", "2.5", "3.5", 0};

const RegSpec         BCtrl[] =
{
    {1, 31, "LOCK", 2, 0, 0},
    {2, 26, "MEM", 0, ctl_values, 0},
    {2, 24, "ED", 0, ctl_values, 0},
    {2, 22, "IO", 0, ctl_values, 0},
    {1, 19, "BE16", 2, 0, 0},
    {2, 14, "BTA", 0, bta_values, 0},
    {1, 13, "DMA", 2, 0, 0},
    {1, 12, "TC", 2, 0, 0},
    {1, 12, "BR", 2, 0, 0},
    {0}};

const char * const psz_values[] = {
    "32", "8", "16", "?", 0};

const RegSpec         PSize[] =
{
    {2, 28, "K2B", 0, psz_values, 0},
    {2, 26, "K2A", 0, psz_values, 0},
    {2, 24, "KUD", 0, psz_values, 0},
    {2, 22, "KUC", 0, psz_values, 0},
    {2, 20, "KUB", 0, psz_values, 0},
    {2, 18, "KUA", 0, psz_values, 0},
    {2, 14, "K1H", 0, psz_values, 0},
    {2, 12, "K1G", 0, psz_values, 0},
    {2, 10, "K1F", 0, psz_values, 0},
    {2, 8, "K1E", 0, psz_values, 0},
    {2, 6, "K1D", 0, psz_values, 0},
    {2, 4, "K1C", 0, psz_values, 0},
    {2, 2, "K1B", 0, psz_values, 0},
    {2, 0, "K1A", 0, psz_values, 0},
    {0}};
#endif

const RegSpec         EntHi[] =
{
#ifdef R4000
    {27, 13, "VPN2", 16, 0, 0},
    {8, 0, "ASID", 10, 0, 0},
#else
    {20, 12, "VPN", 16, 0, 0},
    {6, 6, "ASID", 10, 0, 0},
#endif
    {0}};


const RegSpec         EntLo[] =
{
#ifdef R4000
    {24, 6, "PFN", 16, 0, 0},
    {3, 3, "C", 0, coherencyvalues, 0},
    {1, 2, "D", 2, 0, 0},
    {1, 1, "V", 2, 0, 0},
    {1, 0, "G", 2, 0, 0},
#else
    {20, 12, "PFN", 16, 0, 0},
    {1, 11,  "N", 2, 0, 0},
    {1, 10,  "D", 2, 0, 0},
    {1, 9,   "V", 2, 0, 0},
    {1, 8,   "G", 2, 0, 0},
#endif
    {0}};


const
RegList         reglist[] =
{
    {&Gpr[0], 0, "zero", "0", 0, (F_ALL | F_ANAME | F_GPR | F_64 | F_RO)},
    {&Gpr[1], 0, "at", "1", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[2], 0, "v0", "2", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[3], 0, "v1", "3", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[4], 0, "a0", "4", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[5], 0, "a1", "5", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[6], 0, "a2", "6", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[7], 0, "a3", "7", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[8], 0, "t0", "8", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[9], 0, "t1", "9", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[10], 0, "t2", "10", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[11], 0, "t3", "11", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[12], 0, "t4", "12", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[13], 0, "t5", "13", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[14], 0, "t6", "14", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[15], 0, "t7", "15", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[16], 0, "s0", "16", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[17], 0, "s1", "17", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[18], 0, "s2", "18", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[19], 0, "s3", "19", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[20], 0, "s4", "20", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[21], 0, "s5", "21", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[22], 0, "s6", "22", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[23], 0, "s7", "23", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[24], 0, "t8", "24", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[25], 0, "t9", "25", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[26], 0, "k0", "26", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[27], 0, "k1", "27", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[28], 0, "gp", "28", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[29], 0, "sp", "29", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[30], 0, "s8", "30", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Gpr[31], 0, "ra", "31", 0, (F_ALL | F_ANAME | F_GPR | F_64)},
    {&Hi, 0, "HI", "HI", 0, F_ALL|F_64},
    {&Lo, 0, "LO", "LO", 0, F_ALL|F_64},
    {&Status, Stat, "C0_SR", "SR", 0, (F_ALL)},
    {&Cause, Caus, "C0_CAUSE", "CAUSE", 0, (F_ALL)},
    {&Prid, PRid, "C0_PRID", "PRID", 0, (F_ALL)},
    {&Epc, 0, "C0_EPC", "EPC", 0, (F_ALL|F_64)},
#ifdef LR33000
    {&Bpc, 0, "C0_BPC", "BPC", 0, (F_ALL)},
    {&Bda, 0, "C0_BDA", "BDA", 0, (F_ALL)},
    {&Dcic, DCIC, "C0_DCIC", "DCIC", 0, (F_ALL)},
    {(word *) M_CFGREG, Cfgreg33000, "M_CFG", "CFG", 0, (F_00)},
    {(word *) M_CFGREG, Cfgreg33020, "M_CFG", "CFG", 0, (F_20)},
    {(word *) M_CFGREG, Cfgreg33050, "M_CFG", "CFG", 0, (F_50)},
    {(word *) M_TC1, TimerCnl, "M_TC1", "TC1", 0, (F_00 | F_50)},
    {(word *) M_TIC1, 0, "M_TIC1", "TIC1", 0, (F_00 | F_50)},
    {(word *) M_TC2, TimerCnl, "M_TC2", "TC2", 0, (F_ALL)},
    {(word *) M_TIC2, 0, "M_TIC2", "TIC2", 0, (F_ALL)},
    {(word *) M_RTIC, 0, "M_RTIC", "RTIC", 0, (F_ALL)},
#else
#ifdef R4000
#if 0
    {&C0Count, 0, "C0_COUNT", "COUNT", 0, (F_ALL)},
    {&C0Compare, 0, "C0_COMPARE", "COMPARE", 0, (F_ALL)},
#endif
    {&ErrEpc, 0, "C0_ERRPC", "ERRPC", 0, (F_ALL|F_64)},
    {&WatchLo, 0, "C0_WATCHLO", "WATCHLO", 0, (F_ALL & ~(F_4650 | F_7000))},
    {&WatchHi, 0, "C0_WATCHHI", "WATCHHI", 0, (F_ALL & ~(F_4650 | F_7000))},
    {&LLAddr, 0, "C0_LLADDR", "LLADDR", 0, (F_ALL & ~F_4650)},
    {&LLAddr, 0, "C0_CALG", "CALG", 0, (F_4650)},
    {&WatchLo, 0, "C0_IWATCH", "IWATCH", 0, (F_4650)},
    {&WatchHi, 0, "C0_DWATCH", "DWATCH", 0, (F_4650)},
    {&WatchLo, 0, "C0_WATCH1", "WATCH1", 0, (F_7000)},
    {&WatchHi, 0, "C0_WATCH2", "WATCH2", 0, (F_7000)},
    {&WatchMask, 0, "C0_WATCHMASK", "WATCHMASK", 0, F_7000},
    {&Ecc, 0, "C0_ECC", "ECC", 0, (F_ALL & ~F_4100)},
    {&Ecc, 0, "C0_PERR", "PERR", 0, (F_4100)},
    {&CacheErr, CErr, "C0_CACHERR", "CACHERR", 0, (F_ALL|F_RO)},
    {&TagLo, 0, "C0_TAGLO", "TAGLO", 0, (F_ALL)},
    {&TagHi, 0, "C0_TAGHI", "TAGHI", 0, (F_ALL & ~F_4650)},
    {&Wired, 0, "C0_WIRED", "WIRED", 0, (F_ALL & ~F_4650)},
    {&PgMask, 0, "C0_PGMASK", "PGMASK", 0, (F_ALL & ~F_4650)},
    {&Entrylo0, EntLo, "C0_ENTRYLO0", "ENTRYLO0", 0, (F_ALL & ~F_4650)},
    {&Entrylo1, EntLo, "C0_ENTRYLO1", "ENTRYLO1", 0, (F_ALL & ~F_4650)},
    {&Entrylo0, 0, "C0_DBASE", "DBASE", 0, F_4650},
    {&Entrylo1, 0, "C0_DBOUND", "DBOUND", 0, F_4650},
#else
    {&Entrylo, EntLo, "C0_ENTRYLO", "ENTRYLO", 0, (F_ALL & ~F_4650)},
#endif
    {&Entryhi, EntHi, "C0_ENTRYHI", "ENTRYHI", 0, F_64 | (F_ALL & ~F_4650)},
    {&Badva, 0, "C0_BADVA", "BADVA", 0, F_64 | (F_ALL & ~F_4650)},
    {&Context, 0, "C0_CONTEXT", "CONTEXT", 0, F_64 | (F_ALL & ~F_4650)},
#if __mips >= 3
    {&XContext, 0, "C0_XCONTEXT", "XCONTEXT", 0, F_64 | (F_ALL & ~F_4650)},
#endif
    {&Index, 0, "C0_INDEX", "INDEX", 0, (F_ALL & ~F_4650)},
    {&Random, 0, "C0_RANDOM", "RANDOM", 0, (F_ALL & ~F_4650)},
    {&Index, 0, "C0_IBASE", "IBASE", 0, F_4650},
    {&Random, 0, "C0_IBOUND", "IBOUND", 0, F_4650},
#ifdef R4000
    {&Config, Cfg, "C0_CONFIG", "CONFIG", 0, (F_ALL)},
    {&Info, Infospec, "C0_INFO", "INFO", 0, F_7000},
    {&Icr, Icrspec, "C0C_ICR", "ICR", 0, F_7000},
    {&IplLo, 0, "C0C_IPLLO", "IPLLO", 0, F_7000},
    {&IplHi, 0, "C0C_IPLHI", "IPLHI", 0, F_7000},
#endif
#endif				/* LR33000 */
#if 0
    {0, 0, "C0_TLB", "TLB", 0, (F_ALL | F_RO)},
#endif
#ifdef FLOATINGPT
    {&Fcr, cp1_csr, "C1_CSR", "CSR", 0, (F_ALL | F_FPU)},
    {&Fid, PRid, "C1_FRID", "FRID", 0, (F_ALL | F_FPU | F_RO)},
#endif
#ifdef R3081
    {(word *) mXc0, Cfg,	"C0_CONFIG",	"CONFIG",  3, (F_3081 | F_CF)},
#endif
#ifdef R3041
    {(word *) mXc0, BCtrl,	"C0_BUSCTRL",	"BUSCTRL",  2, (F_3041 | F_CF)},
    {(word *) mXc0, CCfg,	"C0_CACHECFG",	"CACHECFG", 3, (F_3041 | F_CF)},
    {(word *) mXc0, PSize,	"C0_PORTSIZE",	"PORTSIZE", 10, (F_3041 | F_CF)},
    {(word *) mXc0, 0,		"C0_COUNT",	"COUNT", 9, (F_3041 | F_CF)},
    {(word *) mXc0, 0, 		"C0_COMPARE",	"COMPARE",11, (F_3041 | F_CF)},
#endif
#ifdef LR33000
    {(word *) cXc2, 0, "C2_COLOR0", "COLOR0", 2, (F_20 | F_CF)},
    {(word *) cXc2, 0, "C2_COLOR1", "COLOR1", 3, (F_20 | F_CF)},
    {(word *) cXc2, c2config, "C2_CONFIG", "CONFIG", 9, (F_20 | F_CF)},
    {(word *) cXc2, 0, "C2_PLANEMASK", "PLANEMASK", 6, (F_20 | F_CF)},
    {(word *) cXc2, 0, "C2_RASTEROP", "RASTEROP", 5, (F_20 | F_CF)},
    {(word *) cXc2, 0, "C2_SRCSHIFT", "SRCSHIFT", 1, (F_20 | F_CF)},
    {(word *) cXc2, 0, "C2_SRCSKEW", "SRCSKEW", 0, (F_20 | F_CF)},
    {(word *) cXc2, c2gcpcntrl, "C2_GCPCNTRL", "GCPCNTRL", 4, (F_20 | F_CF)},
    {(word *) cXc2, c2psstat, "C2_PSSTAT", "PSSTAT", 24, (F_20 | F_CF | F_RO)},
    {(word *) cXc2, c2pscomm, "C2_PSCOMM", "PSCOMM", 25, (F_20 | F_CF)},
    {(word *) cXc2, c2pscomm, "C2_PSCOMMWE", "PSCOMMWE", 27, (F_20 | F_CF | F_WO)},
    {(word *) mXc2, 0, "C2_BLANKE", "BLANKE", 21, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_BLANKS", "BLANKS", 18, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_CURDISP", "CURDISP", 6, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_DESTCURR", "DESTCURR", 13, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_DESTDATA", "DESTDATA", 29, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_DESTLINE", "DESTLINE", 12, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_DESTPITCH", "DESTPITCH", 14, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_GBLKSIZE", "GBLKSIZE", 15, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_HWCRSR", "HWCRSR", 22, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_HLINTR", "HLINTR", 17, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_HWCRSRSTART", "HWCRSRSTART", 2, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_HWCRSRCURR", "HWCRSRCURR", 3, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_LEFTMASK", "LEFTMASK", 30, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_LINECOUNT", "LINECOUNT", 7, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_NXTDISP", "NXTDISP", 5, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_PSRCVB", "PSRCVB", 25, (F_20 | F_CF | F_RO)},
    {(word *) mXc2, 0, "C2_PSTXB", "PSTXB", 24, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_PSTXBWE", "PSTXBWE", 27, (F_20 | F_CF | F_WO)},
    {(word *) mXc2, 0, "C2_RIGHTMASK", "RIGHTMASK", 31, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_SAMEXTENT", "SAMEXTENT", 4, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_SCRSTART", "SCRSTART", 0, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_SCRPITCH", "SCRPITCH", 1, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_SERPULS", "SERPULS", 16, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_SRCDATA", "SRCDATA", 26, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_SRCCURR", "SRCCURR", 10, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_SRCLINE", "SRCLINE", 9, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_SRCPITCH", "SRCPITCH", 11, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_SYNCS", "SYNCS", 19, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_SYNCRESET", "SYNCRESET", 20, (F_20 | F_CF)},
    {(word *) mXc2, 0, "C2_VBLKSIZE", "VBLKSIZE", 8, (F_20 | F_CF)},
    {(word *) mXc2, c2vhwconfig, "C2_VHWCONFIG", "VHWCONFIG", 23, (F_20 | F_CF)},
#endif
    {0}};
