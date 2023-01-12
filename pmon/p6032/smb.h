/* 
 * p6032/smb.h: PIIX4 SMB for P-6032
 *
 * Copyright (c) 2000 Algorithmics Ltd - all rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the "Free MIPS" License Agreement, a copy of 
 * which is available at:
 *
 *  http://www.algor.co.uk/ftp/pub/doc/freemips-license.txt
 *
 * You may not, however, modify or remove any part of this copyright 
 * message if this program is redistributed or reused in whole or in
 * part.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * "Free MIPS" License for more details.  
 */
int _sbd_smbread(int addr, int protocol, int cmd, void *buf, int *len);
int _sbd_smbwrite(int addr, int protocol, int cmd, const void *buf, int len);


#define SMB_QRW			(0<<2)
#define SMB_BRW			(1<<2)
#define SMB_BDRW		(2<<2)
#define SMB_WDRW		(3<<2)
#define SMB_BKRW		(5<<2)
