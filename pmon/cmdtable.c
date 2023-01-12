/* $Id: cmdtable.c,v 1.13 2001/08/12 12:08:02 chris Exp $ */
#include "mips.h"
#include "pmon.h"

int             setbp (), copy (), dump (), fill (), cont (), go (), help (),
                dis (), modify ();
int             registers (), search (), trace (), clrbp (), sdump (),
                load (), flush ();
#ifdef TLBREGISTERS
int		tlbregisters ();
#endif
int             dohi (), do_sym (), do_ls ();
int		do_set (), do_unset (), do_eset ();
int             stty (), debug (), memtst (), call (), no_cmd ();
int		stacktrace (), transp ();

#ifdef RTC
int		date_cmd();
#endif
#ifdef E2PROM
int		e2program();
#endif
#ifdef FLASH
int		flashprogram();
#endif
#ifdef INET
int		boot_cmd(), ping_cmd();
#endif
#ifdef SROM
int		sbd_srom();
#endif
#if defined(P5064) || defined(P6032) || defined(P6064)
int		sbd_off();
#endif
int reboot_cmd();

extern const Optdesc  load_opts[];
extern const Optdesc  m_opts[];
extern const Optdesc  b_opts[];
extern const Optdesc  debug_opts[];
extern const Optdesc  g_opts[];
extern const Optdesc  t_opts[];
extern const Optdesc  d_opts[];
extern const Optdesc  more_opts[];
extern const Optdesc  ls_opts[];
extern const Optdesc  l_opts[];
extern const Optdesc  stty_opts[];
extern const Optdesc  r_opts[];
extern const Optdesc  sh_opts[];
extern const Optdesc  mt_opts[];
extern const Optdesc  flush_opts[];
extern const Optdesc  boot_opts[];
#ifdef SROM
extern const Optdesc  srom_opts[];
#endif

const Cmd             CmdTable[] =
{
    {"h", "[*|cmd..]", 0, "on-line help", help, 1, 99, 0},
    {"hi", "[cnt]", 0, "display command history", dohi, 1, 2, 0},

    {"m", "adr [val|-s str]..", m_opts, "modify memory", modify, 2, 99, 1},
    {"r", "[reg* [val|field val]]", r_opts, "display/set register",
     registers, 1, 4, 1},
#ifdef TLBREGISTERS
    {"tlb", "[reg]", 0, "display TLB registers", tlbregisters, 1, 2, 1},
#endif

    {"d", "[-bhws] adr [cnt]", d_opts, "display memory", dump, 2, 4, 1},
    {"l", "[-bct][adr [cnt]]", l_opts, "list (disassemble) memory", dis, 1, 5, 1},

    {"copy", "from to siz", 0, "copy memory", copy, 4, 4, 1},
    {"fill", "from to {val|-s str}..", 0, "fill memory", fill, 4, 99, 1},

    {"search", "from to {val|-s str}..", 0, "search memory", search, 4, 99, 1},
    {"tr", "", 0, "transparent mode", transp, 1, 1, 1},

    {"g", "[-s][-b bpadr][-e adr][-- args]", g_opts, "start execution (go)",
     go, 1, 99, 1},
    {"c", "[bptadr]", 0, "continue execution", cont, 1, 2, 1},

    {"t", "[-vibcmMrR] [cnt]", t_opts, "trace (single step)", trace, 1, 99, 1},
    {"to", "[-vibc] [cnt]", t_opts, "trace (step over)", trace, 1, 99, 1},

    {"b", "[-drwx] [-l len] [-s cmd] [adr]..", b_opts, "set break point(s)", setbp, 1, 99, 1},
    {"db", "[numb|*]..", 0, "delete break point(s)", clrbp, 1, 99, 1},

#ifndef NO_SERIAL
    {"load", "[-beastif][-u baud][-o offs][-c cmd][-h port]", load_opts,
     "load memory from hostport", load, 1, 16, 0},
    {"dump", "[-B] adr siz [port]", 0, "dump memory to hostport", sdump, 3, 6, 0},
#endif

    {"set", "[name [value]]", 0, "display/set variable", do_set, 1, 3, 1},
    {"eset", "name ...", 0, "edit variable(s)", do_eset, 2, 99, 1},

    {"unset", "name* ...", 0, "unset variable(s)", do_unset, 2, 99, 1},
#ifdef RTC
    {"date", "[yymmddhhmm.ss]", 0, "get/set date and time", date_cmd, 1, 2, 1},
#endif
#ifdef E2PROM
    {"e2prog", "addr size offset", 0, "program E2PROM and <optionally> reboot", e2program, 4, 4, 0},
#endif
#ifdef FLASH
    {"flash", "[-w|eb|el] addr size offset", 0, "program flash prom", flashprogram, 4, 5, 0},
#endif

    {"debug", "[-svV][-h port][-- args]", debug_opts, "remote debug mode", debug, 1, 99, 0},
    {"stty", "[device] [opts]", stty_opts, "set terminal options", stty, 1, 99, 1},

    {"sym", "name value", 0, "define symbol", do_sym, 3, 5, 1},
    {"ls", "[-ln sym*|-va adr]", ls_opts, "list symbols", do_ls, 1, 99, 1},

    {"flush", "[-di]", flush_opts, "flush caches", flush, 1, 3, 1},
    {"mt", "[-c][[addr] size]", mt_opts, "memory test", memtst, 1, 4, 1},

    {"call", "addr [val|-s str]..", 0, "call function", call, 2, 99, 1},
    {"bt", "[-v] [cnt]", 0, "stack backtrace", stacktrace, 1, 3, 1},

#ifdef INET
    {"boot", "[-bens] [[host:]file]", boot_opts, "load network file", boot_cmd, 1, 99, 1},
    {"ping", "[-fqrv] [-c cnt] [-s sz] host", 0, "ping remote host", ping_cmd, 1, 99, 1},
#endif

    {"sh", "", sh_opts, "command shell", no_cmd, 1, 99, 1},
    {"more", "", more_opts, "paginator", no_cmd, 1, 99, 1},
    {"reboot", "", 0, "reboot PMON", reboot_cmd, 1, 99, 0},
#ifdef SROM
    {"srom", "[-n] [from to size]", srom_opts, "load and go softROM", sbd_srom, 1, 5, 1},
#endif
#if defined(P5064) || defined(P6032) || defined(P6064)
    {"off", "", 0, "turn off", sbd_off, 1, 99, 1},
#endif
    {0}};
