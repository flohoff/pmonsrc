/* $Id: sdeelf.sc,v 1.2 1996/01/16 14:17:06 chris Exp $ */

OUTPUT_ARCH(mips)
ENTRY(_start)

__DYNAMIC  =  0;

MEMORY 	
{
k0ram (RWX):	ORIGIN=0x80000000 LENGTH=0x20000000
k1ram (RWX):	ORIGIN=0xa0000000 LENGTH=0x20000000
k2ram (RWX):	ORIGIN=0xc0000000 LENGTH=0x40000000
rom (RX): 	ORIGIN=0xbfc00000 LENGTH=0x00200000
}

SECTIONS
{
  .text 0xa0000200:
  {
    _ftext  =  .;	/* FIXME mipsism */
    *(.init)
    *(.text)
    *(.fini)
    *(.rdata)
    *(.rodata)
    *(.rodata1)
    . = ALIGN(4);
    *(.lit4)
    . = ALIGN(8);
    *(.lit8)
    CONSTRUCTORS
    _etext  =  .;
    etext  =  .;	/* FIXME mipsism */
  }
  .data .:
  {
    _fdata   =  .;	/* FIXME mipsism */
    *(.data)
    *(.data1)
    _gp   =  . + 0x7ff0;
    *(.sdata)
    edata   =  .;	/* FIXME mipsism */
    _edata  =  .;
  }
  .bss:
  {
    _fbss   = .;	/* FIXME mipsism */
    *(.sbss)
    [SCOMMON]
    *(.bss)
    [COMMON]
    _end    = .;
    end	    = .;	/* FIXME mipsism */
  }
  .debug 0 ( INFO ) : 
	{ *(.debug) }
  .debug_sfnames 0 ( INFO ) : 
	{ *(.debug_sfnames) }
  .debug_pubnames 0 ( INFO ) : 
	{ *(.debug_pubnames) }
  .debug_aranges 0 ( INFO ) : 
	{ *(.debug_aranges) }
  .debug_srcinfo 0 ( INFO ) : 
	{ *(.debug_srcinfo) }
  .line  0 ( INFO ) : { *(.line) }
}
