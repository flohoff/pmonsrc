/*
 * memcmp.c : string function
 * Copyright (c) 1992 Algorithmics Ltd.
 */

int memcmp(const void *src1,const void *src2,unsigned long n)
{
    while(n-- > 0) {
	int c = *(unsigned char*)src1++ - *(unsigned char *)src2++;
	if(c != 0)
	  return c;
    }
    return 0;
}
