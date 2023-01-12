#include <stdio.h>
#include <mips.h>
#include <pmon.h>

#include "sbd.h"
#include "flash.h"


void
sbd_flashinfo (void **flashbuf, int *flashsize)
{
    extern char *heaptop;
    *flashbuf = (void *)heaptop;
    *flashsize = FLASHSIZE;
    memcpy (*flashbuf, (void *)PHYS_TO_K0(FLASH_BASE), FLASHSIZE);
}

/*
 * This public function is used to program the FLASH
 */
int
sbd_flashprogram (unsigned char *data, unsigned int size, unsigned int offset)
{
    int erase;
    volatile void (*restart)(void);
    void *flash = PA_TO_KVA1(FLASH_BASE);
    unsigned char *fp = flash+offset;
    int i;
    int c;

    if ((size + offset) > FLASHSIZE) {
	printf ("Too large for FLASH (offset 0x%x size 0x%x max 0x%x)\n", offset, size, FLASHSIZE);
	return 1;
    }

    /* check to see if erase is required */
    erase = 0;
    for (i = 0; i < size; i++) {
	if (data[i] & ~fp[i]) {
	    /* 
	     * some of the changing bits are not set in the flash
	     * so erase is necessary
	     */
#if 0
	    printf ("mem[%d]=%02x fp[%d]=%02x -> erasing chip\n",
			 i, data[i], i, fp[i]);
#endif
	    erase = 1;
	    break;
	}
    }
    
    /* check to see if restart is required */
    /* we always restart because we can't tell if we've been overwritten */
    restart = (volatile void (*)(void))PHYS_TO_K1(BOOTPROM_BASE);

    printf ("Programming FLASH 0x%x-0x%x (", fp, fp+size);
    if (erase)
	printf ("erasing, ");
    printf ("writing");
    if (restart)
	printf (", rebooting");
    printf (")\nAre you sure? (y/n) ");
    fflush (stdout);

    if ((c = getchar ()) != 'Y' && c != 'y') {
	printf ("\nAbandoned\n");
	return (0);
    }

    printf ("\nStarting..."); fflush (stdout);

    if (_sbd_flashprogramchip (flash, offset, data, size, erase, restart) == FLASHSTAT_OK) {
	printf ("completed\n");
	return 0;
    }
    else {
	printf ("failed\n");
	return 1;
    }
}
