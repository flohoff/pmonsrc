/*
 *	v3I2Ca.h
 *
 *
 *
 *  a01 06/11/1998 12:20p rh -added #define SLAVEADDR t8
 */


#define SLAVEADDR	t8

#define TRUE 1
#define FALSE 0
/*
 * System Register
 * - Offset ??h, Size 16 bits Phil
 */


		/* I2C Bus states */
#define I2C_HIGH			(1)
#define I2C_LOW				(0)
#define I2C_OK				(0)
#define I2C_READ			(1)
#define I2C_WRITE			(0)
#define I2C_BUSY			(2)
#define I2C_NOACK			(1)
		/* number of times a master will try and write to a slave device */
#define I2C_RETRY			(0x30)
#define I2C_1010			0x50

