/* $Id: machine.c,v 1.4 1997/09/01 14:46:14 nigel Exp $ */
#include "mips.h"
#include "pmon.h"

const DISTBL distbl[] =
{
    /* special aliases for certain instructions */
    {"nop", 0xffffffffL, 0x00000000L, NONE},		/* sll 0,0,0 */
    {"li", 0xffe00000L, 0x24000000L, RT_SIMM},		/* addiu rd,0,simm */
    {"li", 0xffe00000L, 0x34000000L, RT_IMM},		/* ori rd,0,imm*/
    {"move", 0xfc1f07ffL, 0x00000021L, RD_RS}, 		/* addu rd,0,rs */
    {"move", 0xfc1f07ffL, 0x00000025L, RD_RS}, 		/* or rd,0,rs */
    {"neg", 0xffe007ffL, 0x00000022L, RD_RT}, 		/* sub rd,rt,0 */
    {"negu", 0xffe007ffL, 0x00000023L, RD_RT}, 		/* subu rd,rt,0 */
    {"dmove", 0xfc1f07ffL, 0x0000002dL, RD_RS}, 	/* daddu rd,0,rs */
    {"dneg", 0xffe007ffL, 0x0000002eL, RD_RT}, 		/* dsub rd,rt,0 */
    {"dnegu", 0xffe007ffL, 0x0000002fL, RD_RT}, 	/* dsubu rd,rt,0 */
    {"not", 0xfc1f07ffL, 0x00000027L, RD_RS}, 		/* nor rd,0,rs */
    {"b", 0xffff0000L, 0x10000000L, OFF},		/* beq 0,0 */
    {"b", 0xffff0000L, 0x04010000L, OFF},		/* bgez 0 */
    {"bal", 0xffff0000L, 0x04110000L, OFF},		/* bgezal 0 */

    /* machine insns */
    {"add", 0xfc0007ffL, 0x00000020L, RD_RS_RT},
    {"addi", 0xfc000000L, 0x20000000L, RT_RS_SIMM},
    {"addiu", 0xfc000000L, 0x24000000L, RT_RS_SIMM},
    {"addu", 0xfc0007ffL, 0x00000021L, RD_RS_RT},

    {"dadd", 0xfc0007ffL, 0x0000002cL, RD_RS_RT},
    {"daddi", 0xfc000000L, 0x60000000L, RT_RS_SIMM},
    {"daddiu", 0xfc000000L, 0x64000000L, RT_RS_SIMM},
    {"daddu", 0xfc0007ffL, 0x0000002dL, RD_RS_RT},

    {"and", 0xfc0007ffL, 0x00000024L, RD_RS_RT},
    {"andi", 0xfc000000L, 0x30000000L, RT_RS_IMM},

    {"bc0f", 0xffff0000L, 0x41000000L, CP_OFF},
    {"bc1f", 0xffff0000L, 0x45000000L, CP_OFF},
    {"bc2f", 0xffff0000L, 0x49000000L, CP_OFF},
    {"bc3f", 0xffff0000L, 0x4d000000L, CP_OFF},
    {"bc0t", 0xffff0000L, 0x41010000L, CP_OFF},
    {"bc1t", 0xffff0000L, 0x45010000L, CP_OFF},
    {"bc2t", 0xffff0000L, 0x49010000L, CP_OFF},
    {"bc3t", 0xffff0000L, 0x4d010000L, CP_OFF},

    {"bc0fl", 0xffff0000L, 0x41020000L, CP_OFF},
    {"bc1fl", 0xffff0000L, 0x45020000L, CP_OFF},
    {"bc2fl", 0xffff0000L, 0x49020000L, CP_OFF},
    {"bc3fl", 0xffff0000L, 0x4d020000L, CP_OFF},
    {"bc0tl", 0xffff0000L, 0x41030000L, CP_OFF},
    {"bc1tl", 0xffff0000L, 0x45030000L, CP_OFF},
    {"bc2tl", 0xffff0000L, 0x49030000L, CP_OFF},
    {"bc3tl", 0xffff0000L, 0x4d030000L, CP_OFF},

    {"beq",    0xfc000000L, 0x10000000L, RS_RT_OFF},
    {"bne",    0xfc000000L, 0x14000000L, RS_RT_OFF},
    {"blez",   0xfc1f0000L, 0x18000000L, RS_OFF},
    {"bgtz",   0xfc1f0000L, 0x1c000000L, RS_OFF},

    {"beql",   0xfc000000L, 0x50000000L, RS_RT_OFF},
    {"bnel",   0xfc000000L, 0x54000000L, RS_RT_OFF},
    {"blezl",  0xfc1f0000L, 0x58000000L, RS_OFF},
    {"bgtzl",  0xfc1f0000L, 0x5c000000L, RS_OFF},

    {"bltz",   0xfc1f0000L, 0x04000000L, RS_OFF},
    {"bgez",   0xfc1f0000L, 0x04010000L, RS_OFF},

    {"bltzl",  0xfc1f0000L, 0x04020000L, RS_OFF},
    {"bgezl",  0xfc1f0000L, 0x04030000L, RS_OFF},

    {"bltzal", 0xfc1f0000L, 0x04100000L, RS_OFF},
    {"bgezal", 0xfc1f0000L, 0x04110000L, RS_OFF},

    {"bltzall",0xfc1f0000L, 0x04120000L, RS_OFF},
    {"bgezall",0xfc1f0000L, 0x04130000L, RS_OFF},

    {"break", 0xfc00003fL, 0x0000000dL, BPCODE},

    {"cache", 0xfc000000L, 0xbc000000L, CACHE_OP},

    {"cfc0", 0xffe007ffL, 0x40400000L, RT_RD},
    {"cfc1", 0xffe007ffL, 0x44400000L, RT_CC1},
#ifdef LR33000
    {"cfc2", 0xffe007ffL, 0x48400000L, RT_CC2},
#else
    {"cfc2", 0xffe007ffL, 0x48400000L, RT_CN},
#endif
    {"cfc3", 0xffe007ffL, 0x4c400000L, RT_CN},
    {"tlbp", 0xffffffffL, 0x42000008L, NONE},
    {"tlbr", 0xffffffffL, 0x42000001L, NONE},
    {"tlbwi", 0xffffffffL, 0x42000002L, NONE},
    {"tlbwr", 0xffffffffL, 0x42000006L, NONE},
    {"rfe", 0xffffffffL, 0x42000010L, NONE},
    {"eret", 0xffffffffL, 0x42000018L, NONE},
    {"cop0", 0xfe000000L, 0x42000000L, COFUN},

    {"add.s", 0xfee0003fL, 0x46000000L, FT_FS_FD_S},
    {"add.d", 0xfee0003fL, 0x46200000L, FT_FS_FD_D},
    {"sub.s", 0xfee0003fL, 0x46000001L, FT_FS_FD_S},
    {"sub.d", 0xfee0003fL, 0x46200001L, FT_FS_FD_D},
    {"mul.s", 0xfee0003fL, 0x46000002L, FT_FS_FD_S},
    {"mul.d", 0xfee0003fL, 0x46200002L, FT_FS_FD_D},
    {"div.s", 0xfee0003fL, 0x46000003L, FT_FS_FD_S},
    {"div.d", 0xfee0003fL, 0x46200003L, FT_FS_FD_D},
    {"abs.s", 0xfee0003fL, 0x46000005L, FS_FD_S},
    {"abs.d", 0xfee0003fL, 0x46200005L, FS_FD_D},
    {"mov.s", 0xfee0003fL, 0x46000006L, FS_FD_S},
    {"mov.d", 0xfee0003fL, 0x46200006L, FS_FD_D},
    {"neg.s", 0xfee0003fL, 0x46000007L, FS_FD_S},
    {"neg.d", 0xfee0003fL, 0x46200007L, FS_FD_D},
    {"sqrt.d", 0xfee0003fL, 0x46200004, FS_FD_D},
    {"sqrt.s", 0xfee0003fL, 0x46000004, FS_FD_S},

    {"c.f.s", 0xfee0003fL, 0x46000030L, FT_FS_S},
    {"c.f.d", 0xfee0003fL, 0x46200030L, FT_FS_D},
    {"c.un.s", 0xfee0003fL, 0x46000031L, FT_FS_S},
    {"c.un.d", 0xfee0003fL, 0x46200031L, FT_FS_D},
    {"c.eq.s", 0xfee0003fL, 0x46000032L, FT_FS_S},
    {"c.eq.d", 0xfee0003fL, 0x46200032L, FT_FS_D},
    {"c.ueq.s", 0xfee0003fL, 0x46000033L, FT_FS_S},
    {"c.ueq.d", 0xfee0003fL, 0x46200033L, FT_FS_D},
    {"c.olt.s", 0xfee0003fL, 0x46000034L, FT_FS_S},
    {"c.olt.d", 0xfee0003fL, 0x46200034L, FT_FS_D},
    {"c.ult.s", 0xfee0003fL, 0x46000035L, FT_FS_S},
    {"c.ult.d", 0xfee0003fL, 0x46200035L, FT_FS_D},
    {"c.ole.s", 0xfee0003fL, 0x46000036L, FT_FS_S},
    {"c.ole.d", 0xfee0003fL, 0x46200036L, FT_FS_D},
    {"c.ule.s", 0xfee0003fL, 0x46000037L, FT_FS_S},
    {"c.ule.d", 0xfee0003fL, 0x46200037L, FT_FS_D},
    {"c.sf.s", 0xfee0003fL, 0x46000038L, FT_FS_S},
    {"c.sf.d", 0xfee0003fL, 0x46200038L, FT_FS_D},
    {"c.ngle.s", 0xfee0003fL, 0x46000039L, FT_FS_S},
    {"c.ngle.d", 0xfee0003fL, 0x46200039L, FT_FS_D},
    {"c.seq.s", 0xfee0003fL, 0x4600003aL, FT_FS_S},
    {"c.seq.d", 0xfee0003fL, 0x4620003aL, FT_FS_D},
    {"c.ngl.s", 0xfee0003fL, 0x4600003bL, FT_FS_S},
    {"c.ngl.d", 0xfee0003fL, 0x4620003bL, FT_FS_D},
    {"c.lt.s", 0xfee0003fL, 0x4600003cL, FT_FS_S},
    {"c.lt.d", 0xfee0003fL, 0x4620003cL, FT_FS_D},
    {"c.nge.s", 0xfee0003fL, 0x4600003dL, FT_FS_S},
    {"c.nge.d", 0xfee0003fL, 0x4620003dL, FT_FS_D},
    {"c.le.s", 0xfee0003fL, 0x4600003eL, FT_FS_S},
    {"c.le.d", 0xfee0003fL, 0x4620003eL, FT_FS_D},
    {"c.ngt.s", 0xfee0003fL, 0x4600003fL, FT_FS_S},
    {"c.ngt.d", 0xfee0003fL, 0x4620003fL, FT_FS_D},

    {"cvt.s.w", 0xfee0003fL, 0x46800020L, FS_FD_W},
    {"cvt.s.l", 0xfee0003fL, 0x46a00020L, FS_FD_L},
    {"cvt.s.d", 0xfee0003fL, 0x46200020L, FS_FD_D},
    {"cvt.d.s", 0xfee0003fL, 0x46000021L, FS_FD_S},
    {"cvt.d.w", 0xfee0003fL, 0x46800021L, FS_FD_W},
    {"cvt.d.l", 0xfee0003fL, 0x46a00021L, FS_FD_L},
    {"cvt.w.d", 0xfee0003fL, 0x46200024L, FS_FD_D},
    {"cvt.w.s", 0xfee0003fL, 0x46000024L, FS_FD_S},
    {"cvt.l.d", 0xfee0003fL, 0x46200025L, FS_FD_D},
    {"cvt.l.s", 0xfee0003fL, 0x46000025L, FS_FD_S},

    {"ceil.l.d", 0xffff003fL, 0x4620000aL, FS_FD_L},
    {"ceil.l.s", 0xffff003fL, 0x4600000aL, FS_FD_L},
    {"ceil.w.d", 0xffff003fL, 0x4620000eL, FS_FD_W},
    {"ceil.w.s", 0xffff003fL, 0x4600000eL, FS_FD_W},
    {"floor.l.d", 0xffff003fL, 0x4620000bL, FS_FD_L},
    {"floor.l.s", 0xffff003fL, 0x4600000bL, FS_FD_L},
    {"floor.w.d", 0xffff003fL, 0x4620000fL, FS_FD_W},
    {"floor.w.s", 0xffff003fL, 0x4600000fL, FS_FD_W},
    {"round.l.d", 0xffff003fL, 0x46200008L, FS_FD_L},
    {"round.l.s", 0xffff003fL, 0x46000008L, FS_FD_L},
    {"round.w.d", 0xffff003fL, 0x4620000cL, FS_FD_W},
    {"round.w.s", 0xffff003fL, 0x4600000cL, FS_FD_W},
    {"trunc.l.d", 0xffff003fL, 0x46200009L, FS_FD_L},
    {"trunc.l.s", 0xffff003fL, 0x46000009L, FS_FD_L},
    {"trunc.w.d", 0xffff003fL, 0x4620000dL, FS_FD_W},
    {"trunc.w.s", 0xffff003fL, 0x4600000dL, FS_FD_W},

    {"cop1", 0xfe000000L, 0x46000000L, COFUN},

#ifdef LR33020
    {"sstep", 0xfeffffffL, 0x4a00ffffL, NONE},
    {"sbstep", 0xfeffffffL, 0x4a40ffffL, NONE},
    {"wstep", 0xfeffffffL, 0x4a80ffffL, NONE},
    {"wstep_l", 0xfeffffffL, 0x4a88ffffL, NONE},
    {"wstep_r", 0xfeffffffL, 0x4a84ffffL, NONE},
    {"wstep_l_r", 0xfeffffffL, 0x4a8cffffL, NONE},
    {"wstep_s", 0xfeffffffL, 0x4aa0ffffL, NONE},
    {"wstep_s_l", 0xfeffffffL, 0x4aa8ffffL, NONE},
    {"wstep_s_r", 0xfeffffffL, 0x4aa4ffffL, NONE},
    {"wstep_s_l_r", 0xfeffffffL, 0x4aacffffL, NONE},
    {"wstep_sb", 0xfeffffffL, 0x4ab0ffffL, NONE},
    {"wstep_sb_l", 0xfeffffffL, 0x4ab8ffffL, NONE},
    {"wstep_sb_r", 0xfeffffffL, 0x4ab4ffffL, NONE},
    {"wstep_sb_l_r", 0xfeffffffL, 0x4abcffffL, NONE},
    {"wstep_four", 0xfeffffffL, 0x4a82ffffL, NONE},
    {"wstep_bfour", 0xfeffffffL, 0x4a81ffffL, NONE},
    {"bstep_bfour", 0xfeffffffL, 0x4ac1ffffL, NONE},
    {"bstep", 0xfeffffffL, 0x4ac0ffffL, NONE},
    {"bstep_l", 0xfeffffffL, 0x4ac8ffffL, NONE},
    {"bstep_r", 0xfeffffffL, 0x4ac4ffffL, NONE},
    {"bstep_l_r", 0xfeffffffL, 0x4accffffL, NONE},
    {"bstep_s", 0xfeffffffL, 0x4ae0ffffL, NONE},
    {"bstep_s_l", 0xfeffffffL, 0x4ae8ffffL, NONE},
    {"bstep_s_r", 0xfeffffffL, 0x4ae4ffffL, NONE},
    {"bstep_s_l_r", 0xfeffffffL, 0x4aecffffL, NONE},
    {"bstep_sb", 0xfeffffffL, 0x4af0ffffL, NONE},
    {"bstep_sb_l", 0xfeffffffL, 0x4af8ffffL, NONE},
    {"bstep_sb_r", 0xfeffffffL, 0x4af4ffffL, NONE},
    {"bstep_sb_l_r", 0xfeffffffL, 0x4afcffffL, NONE},
#endif				/* LR33020 */
    {"cop2", 0xfe000000L, 0x4a000000L, COFUN},
    {"cop3", 0xfe000000L, 0x4e000000L, COFUN},

    {"ctc0", 0xffe007ffL, 0x40c00000L, RT_RD_TO},
    {"ctc1", 0xffe007ffL, 0x44c00000L, RT_CC1_TO},
#ifdef LR33000
    {"ctc2", 0xffe007ffL, 0x48c00000L, RT_CC2},
#else
    {"ctc2", 0xffe007ffL, 0x48c00000L, RT_CN_TO},
#endif
    {"ctc3", 0xffe007ffL, 0x4cc00000L, RT_CN_TO},

    {"div", 0xfc00ffffL, 0x0000001aL, RS_RT},
    {"divu", 0xfc00ffffL, 0x0000001bL, RS_RT},
    {"ddiv", 0xfc00ffffL, 0x0000001eL, RS_RT},
    {"ddivu", 0xfc00ffffL, 0x0000001fL, RS_RT},

    {"j", 0xfc000000L, 0x08000000L, TARGET},
    {"jal", 0xfc000000L, 0x0c000000L, TARGET},
    {"jalr", 0xfc1f07ffL, 0x00000009L, JALR},
    {"jr", 0xfc1fffffL, 0x00000008L, JR},

    {"lui", 0xfc000000L, 0x3c000000L, RT_IMM},

    {"lb", 0xfc000000L, 0x80000000L, LOAD_STORE},
    {"lbu", 0xfc000000L, 0x90000000L, LOAD_STORE},
    {"lh", 0xfc000000L, 0x84000000L, LOAD_STORE},
    {"lhu", 0xfc000000L, 0x94000000L, LOAD_STORE},
    {"lw", 0xfc000000L, 0x8c000000L, LOAD_STORE},
    {"lwl", 0xfc000000L, 0x88000000L, LOAD_STORE},
    {"lwr", 0xfc000000L, 0x98000000L, LOAD_STORE},
    {"lwu", 0xfc000000L, 0x9c000000L, LOAD_STORE},
    {"ldl", 0xfc000000L, 0x68000000L, LOAD_STORE},
    {"ldr", 0xfc000000L, 0x6c000000L, LOAD_STORE},

    {"ll", 0xfc000000L, 0xc0000000L, LOAD_STORE},
    {"lwc1", 0xfc000000L, 0xc4000000L, LDSTC1},
    {"lwc2", 0xfc000000L, 0xc8000000L, LDSTCN},
    {"lwc3", 0xfc000000L, 0xcc000000L, LDSTCN},

    {"lld", 0xfc000000L, 0xd0000000L, LOAD_STORE},
    {"ldc1", 0xfc000000L, 0xd4000000L, LDSTC1},
    {"ldc2", 0xfc000000L, 0xd8000000L, LDSTCN},
    {"ld", 0xfc000000L, 0xdc000000L, LOAD_STORE},
    
    {"mfc0", 0xffe007ffL, 0x40000000L, RT_C0},
    {"mfc1", 0xffe007ffL, 0x44000000L, RT_C1},
#ifdef LR33000
    {"mfc2", 0xffe007ffL, 0x48000000L, RT_C2},
#else
    {"mfc2", 0xffe007ffL, 0x48000000L, RT_CN},
#endif
    {"mfc3", 0xffe007ffL, 0x4c000000L, RT_CN},

    {"dmfc0", 0xffe007ffL, 0x40200000L, RT_C0},
    {"dmfc1", 0xffe007ffL, 0x44200000L, RT_C1},
    {"dmfc2", 0xffe007ffL, 0x48200000L, RT_CN},
    {"dmfc3", 0xffe007ffL, 0x4c200000L, RT_CN},

    {"mtc0", 0xffe007ffL, 0x40800000L, RT_C0_TO},
    {"mtc1", 0xffe007ffL, 0x44800000L, RT_C1_TO},
#ifdef LR33000
    {"mtc2", 0xffe007ffL, 0x48800000L, RT_C2},
#else
    {"mtc2", 0xffe007ffL, 0x48800000L, RT_CN_TO},
#endif
    {"mtc3", 0xffe007ffL, 0x4c800000L, RT_CN_TO},

    {"dmtc0", 0xffe007ffL, 0x40a00000L, RT_C0_TO},
    {"dmtc1", 0xffe007ffL, 0x44a00000L, RT_C1_TO},
    {"dmtc2", 0xffe007ffL, 0x48a00000L, RT_CN_TO},
    {"dmtc3", 0xffe007ffL, 0x4ca00000L, RT_CN_TO},

    {"mfhi", 0xffff07ffL, 0x00000010L, RD},
    {"mflo", 0xffff07ffL, 0x00000012L, RD},
    {"mthi", 0xfc1fffffL, 0x00000011L, RS},
    {"mtlo", 0xfc1fffffL, 0x00000013L, RS},

    {"mult", 0xfc00ffffL, 0x00000018L, RS_RT},
    {"multu", 0xfc00ffffL, 0x00000019L, RS_RT},
    {"dmult", 0xfc00ffffL, 0x0000001cL, RS_RT},
    {"dmultu", 0xfc00ffffL, 0x0000001dL, RS_RT},

    /* R4100: */
    {"madd16",  0xfc00ffff, 0x00000028, RS_RT},
    {"dmadd16",  0xfc00ffff, 0x00000020, RS_RT},
    {"standby", ~0, 0x42000021, NONE},
    {"suspend", ~0, 0x42000022, NONE},
    {"hibernate", ~0, 0x42000023, NONE},

    {"nor", 0xfc0007ffL, 0x00000027L, RD_RS_RT},
    {"or", 0xfc0007ffL, 0x00000025L, RD_RS_RT},
    {"ori", 0xfc000000L, 0x34000000L, RT_RS_IMM},

    {"sb", 0xfc000000L, 0xa0000000L, STORE},
    {"sh", 0xfc000000L, 0xa4000000L, STORE},
    {"swl", 0xfc000000L, 0xa8000000L, STORE},
    {"sw", 0xfc000000L, 0xac000000L, STORE},
    {"sdl", 0xfc000000L, 0xb0000000L, STORE},
    {"sdr", 0xfc000000L, 0xb4000000L, STORE},
    {"swr", 0xfc000000L, 0xb8000000L, STORE},

    {"sc", 0xfc000000L, 0xe0000000L, STORE},
    {"swc1", 0xfc000000L, 0xe4000000L, STOREC1},
    {"swc2", 0xfc000000L, 0xe8000000L, STORECN},
    {"swc3", 0xfc000000L, 0xec000000L, STORECN},

    {"scd", 0xfc000000L, 0xf0000000L, STORE},
    {"sdc1", 0xfc000000L, 0xf4000000L, STOREC1},
    {"sdc2", 0xfc000000L, 0xf8000000L, STORECN},
    {"sd", 0xfc000000L, 0xfc000000L, STORE},

    {"sll", 0xffe0003fL, 0x00000000L, RD_RT_SFT},
    {"sllv", 0xfc0007ffL, 0x00000004L, RD_RT_RS},
    {"dsll", 0xffe0003fL, 0x00000038L, RD_RT_SFT},
    {"dsllv", 0xfc0007ffL, 0x00000014L, RD_RT_RS},
    {"dsll32", 0xfc0007ffL, 0x0000003cL, RD_RT_RS},

    {"slt", 0xfc0007ffL, 0x0000002aL, RD_RS_RT},
    {"slti", 0xfc000000L, 0x28000000L, RT_RS_SIMM},
    {"sltiu", 0xfc000000L, 0x2c000000L, RT_RS_SIMM},
    {"sltu", 0xfc0007ffL, 0x0000002bL, RD_RS_RT},

    {"sra", 0xffe0003fL, 0x00000003L, RD_RT_SFT},
    {"srav", 0xfc0007ffL, 0x00000007L, RD_RT_RS},
    {"dsra", 0xffe0003fL, 0x0000003bL, RD_RT_SFT},
    {"dsrav", 0xfc0007ffL, 0x00000017L, RD_RT_RS},
    {"dsra32", 0xfc0007ffL, 0x0000003fL, RD_RT_RS},

    {"srl", 0xffe0003fL, 0x00000002L, RD_RT_SFT},
    {"srlv", 0xfc0007ffL, 0x00000006L, RD_RT_RS},
    {"dsrl", 0xffe0003fL, 0x0000003aL, RD_RT_SFT},
    {"dsrlv", 0xfc0007ffL, 0x00000016L, RD_RT_RS},
    {"dsrl32", 0xfc0007ffL, 0x0000003eL, RD_RT_RS},

    {"sub", 0xfc0007ffL, 0x00000022L, RD_RS_RT},
    {"subu", 0xfc0007ffL, 0x00000023L, RD_RS_RT},
    {"dsub", 0xfc0007ffL, 0x0000002eL, RD_RS_RT},
    {"dsubu", 0xfc0007ffL, 0x0000002fL, RD_RS_RT},

    {"teqi", 0xfc1f0000L, 0x040c0000L, RS_SIMM},
    {"teq", 0xfc00003fL, 0x00000034L, RS_RT},
    {"tgei", 0xfc1f0000L, 0x04080000L, RS_SIMM},
    {"tge", 0xfc00003fL, 0x00000030L, RS_RT},
    {"tgeiu", 0xfc1f0000L, 0x04090000L, RS_SIMM},
    {"tgeu", 0xfc00003fL, 0x00000031L, RS_RT},
    {"tlti", 0xfc1f0000L, 0x040a0000L, RS_SIMM},
    {"tlt", 0xfc00003fL, 0x00000032L, RS_RT},
    {"tltiu", 0xfc1f0000L, 0x040b0000L, RS_SIMM},
    {"tltu", 0xfc00003fL, 0x00000033L, RS_RT},
    {"tnei", 0xfc1f0000L, 0x040e0000L, RS_SIMM},
    {"tne", 0xfc00003fL, 0x00000036L, RS_RT},

    {"sync", 0xffffffffL, 0x0000000fL, NONE},
    {"syscall", 0xffffffffL, 0x0000000cL, NONE},
    {"xor", 0xfc0007ffL, 0x00000026L, RD_RS_RT},
    {"xori", 0xfc000000L, 0x38000000L, RT_RS_IMM},

	/* must be last !! never be move/remove */
    {".word", 0x00000000L, 0x00000000L, WORD}
};

int 
is_branch (adr)
     word            adr;
{
    const DISTBL   *pt;
    uword           inst;

    inst = load_word (adr);
    pt = get_distbl (inst);
    switch (pt->type) {
    case OFF:
    case RS_RT_OFF:
    case RS_OFF:
    case CP_OFF:
    case TARGET:
    case JALR:
    case JR:
	return (1);
    default:
	return (0);
    }
}

int 
is_conditional_branch (adr)
     word            adr;
{
    const DISTBL   *pt;
    uword           inst;

    inst = load_word (adr);
    pt = get_distbl (inst);
    switch (pt->type) {
    case RS_RT_OFF:
    case RS_OFF:
    case CP_OFF:
	return (1);
    default:
	return (0);
    }
}

is_jr (adr)
     unsigned long   adr;
{
    const DISTBL   *pt;
    uword           inst;

    inst = load_word (adr);
    pt = get_distbl (inst);
    return (pt->type == JR);
}

word 
branch_target_address (adr)
     word            adr;
{
    const DISTBL   *pt;
    word            val;
    uword           inst;

    inst = load_word (adr);
    pt = get_distbl (inst);
    switch (pt->type) {
    case OFF:
    case RS_RT_OFF:
    case RS_OFF:
    case CP_OFF:
	val = inst & 0xffff;
	if (val & 0x8000)
	    val |= 0xffff0000;
	return (adr + 4 + (val << 2));
    case TARGET:
	val = inst & 0x3ffffff;
	return (((adr + 4) & 0xf0000000) | (val << 2));
    case JALR:
    case JR:
	val = RS_ (inst);
	return (Gpr[val]);
    default:
	return (0);
    }
}

is_jal (adr)
     word            adr;
{
    uword           inst;

    inst = load_word (adr);
    switch (getfield (inst, 6, 26)) {
    case 0:
	/* special: jalr */
	return (getfield (inst, 6, 0) == 9);
    case 1:
	/* regimm: bal */
	return (getfield (inst, 2, 19) == 2);
    case 3:
	/* jal */
	return (1);
    }
    return (0);
}

const DISTBL *
get_distbl (bits)
     uword            bits;
{
    const DISTBL *pt = distbl;
    static const DISTBL *lastpt = 0;
    static uword lastbits;

    /* simple cache for repeated lookups */
    if (lastpt && bits == lastbits)
      return lastpt;

    while ((bits & pt->mask) != pt->code)
	++pt;
    lastpt = pt;
    lastbits = bits;
    return (pt);
}

is_writeable (adr)
     unsigned long   adr;
{
    unsigned long   x;

#ifdef SABLE
    if (adr >= 0x9fc00000 && adr <= 0x9fc3ffff)
	return (0);
    if (adr >= 0xbfc00000 && adr <= 0xbfc3ffff)
	return (0);
    return (1);
#else
    x = load_word (adr);
    store_word (adr, ~x);
    flush_cache (DCACHE);
    if (load_word (adr) != ~x)
	return (0);
    store_word (adr, x);
    return (1);
#endif
}
