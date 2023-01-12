/*** qtc_ep.h ***/

/***
 * $Header: /vol/cvsroot/pmon/include/qtc_ep.h,v 1.1.1.1 1996/01/10 16:34:12 chris Exp $
***/

/***
 * $Log: qtc_ep.h,v $
 * Revision 1.1.1.1  1996/01/10 16:34:12  chris
 * Import of Algorithmics PMON v3
 *
 * Revision 2.0  90/06/20  16:45:53  law
 * This is the version 2.0 standard.
 * 
***/

/*** ----------------- QTC IEEE Emulation Library ----------------------- ****
****                                                                      ****
****  Copyright:                                                          ****
****      (c) 1988                                                        ****
****      (c) 1989                                                        ****
****      (c) 1990 Quantitative Technology Corporation.                   ****
****               8700 SW Creekside Place Suite D                        ****
****               Beaverton OR 97005                                     ****
****               (503) 626 3081                                         ****
****                                                                      ****
**** -------------------------------------------------------------------- ***/

/*****************************************************************************
** Change the following value to adjust the binary precision of the         **
** internal extended-precision value                                        **
*****************************************************************************/
#define NUMBER_OF_BINARY_BITS 80  /*  Maximum binary precision in bits    */

/*****************************************************************************
** Change the following values to adjust the acceptable range of exponents  **
** for an internal extended-precision value                                 **
*****************************************************************************/
#define MAX_BINARY_EXPONENT  32767
#define MIN_BINARY_EXPONENT  (-MAX_BINARY_EXPONENT)

/*****************************************************************************
** Change the following values to alter the number of decimal digits used   **
** for conversion between internal extended-precision format and ASCII      **
** string representations                                                   **
*****************************************************************************/
#define NUMBER_OF_DECIMAL_DIGITS 25
#define NUMBER_OF_EXPONENT_DIGITS 5

/*****************************************************************************
** The remainder of the values in this file should not be altered from      **
** their supplied values                                                    **
*****************************************************************************/
#define BINARY_DIGIT_LENGTH 16

#define SINGLE_EXP_UPPER_LIMIT        127
#define DOUBLE_EXP_UPPER_LIMIT        1023
#define IEEE_EXTENDED_EXP_UPPER_LIMIT 0x3FFF
#define QUAD_EXP_UPPER_LIMIT          0x3FFF

#define SINGLE_EXP_LOWER_LIMIT        -126
#define DOUBLE_EXP_LOWER_LIMIT        -1022
#define IEEE_EXTENDED_EXP_LOWER_LIMIT -0x3FFE
#define QUAD_EXP_LOWER_LIMIT          -0x3FFE

#define MANTISSA_LENGTH         ((NUMBER_OF_BINARY_BITS+BINARY_DIGIT_LENGTH-1)/BINARY_DIGIT_LENGTH)
#define NUMBER_OF_BINARY_DIGITS (2*MANTISSA_LENGTH+1)
#define NUMBER_OF_HEX_DIGITS    (4*MANTISSA_LENGTH)

#define MAX_DECIMAL_EXPONENT MAX_BINARY_EXPONENT
#define MIN_DECIMAL_EXPONENT (-MAX_DECIMAL_EXPONENT)

#define INFINITY_EXPONENT (MAX_BINARY_EXPONENT+2)
#define BINARY_OVERFLOW_EXPONENT (MAX_BINARY_EXPONENT+1)

#define SINGLE_PRECISION        0
#define DOUBLE_PRECISION        1
#define EXTENDED_PRECISION      2
#define IEEE_EXTENDED_PRECISION 3
#define QUAD_PRECISION          4

#define TRUE 1
#define FALSE 0

#define min(x,y) (((x)<=(y))?(x):(y))
#define max(x,y) (((x)>=(y))?(x):(y))

#define POSITIVE 0
#define NEGATIVE 1

#define ROUND_TO_NEAREST        0
#define ROUND_TO_MINUS_INFINITY 1
#define ROUND_TO_PLUS_INFINITY  2
#define ROUND_TO_ZERO           3

#define SINGLE_EXP_WIDTH       8
#define SINGLE_FRACTION_WIDTH 23

#define DOUBLE_EXP_WIDTH      11
#define DOUBLE_FRACTION_WIDTH 52

#define IEEE_EXTENDED_EXP_WIDTH      15
#define IEEE_EXTENDED_FRACTION_WIDTH 80

#define QUAD_EXP_WIDTH      15
#define QUAD_FRACTION_WIDTH 96

#define SINGLE_PLUS_INFINITY           0x7F800000L
#define SINGLE_MINUS_INFINITY          0xFF800000L
#define SINGLE_PLUS_INFINITY_MINUS_ONE 0x7F7FFFFFL
#define SINGLE_MINUS_INFINITY_PLUS_ONE 0xFF7FFFFFL

#define LESS_THAN        0
#define GREATER_THAN     1
#define EQUAL            2
#define UNORDERED        3

struct ep {
   unsigned int sign;
   long int exponent;
   unsigned long int mant[MANTISSA_LENGTH];
};
typedef struct ep EP;

#ifdef MSH_FIRST
#define MSH(x) (*(unsigned long int*)&x)
#define LSH(x) (*(((unsigned long int*)&x)+1))
#else
#define MSH(x) (*(((unsigned long int*)&x)+1))
#define LSH(x) (*(unsigned long int*)&x)
#endif

/*****
** Make sure either UNDERFLOW_AFTER_ROUNDING or UNDERFLOW_BEFORE_ROUNDING
** is defined
*****/
#ifndef UNDERFLOW_AFTER_ROUNDING
#ifndef UNDERFLOW_BEFORE_ROUNDING
#define UNDERFLOW_BEFORE_ROUNDING
#endif
#endif

#define LONG(x) (*(unsigned long int*)&x)

#ifndef INTEGER_ONLY
#define DOUBLE(x) (*(double*)x)
#define FLOAT(x) (*(float*)&x)
#endif

struct internal_decimal_format {
     unsigned int sign;                            /* Sign               */
     long int exponent;                            /* Exponent           */
     unsigned int digit[NUMBER_OF_DECIMAL_DIGITS]; /* Significand Digits */
     } ;

struct internal_binary_format {
     unsigned int sign;                           /* Sign                  */
     long int exponent;                           /* Exponent              */
     unsigned int digit[NUMBER_OF_BINARY_DIGITS]; /* Bytes of Significand  */
     } ;

#define DECIMAL_BUFFER_LENGTH (NUMBER_OF_DECIMAL_DIGITS + \
                               NUMBER_OF_EXPONENT_DIGITS + 5)
#define DECIMAL_BUFFER_END    (DECIMAL_BUFFER_LENGTH-1)

#define BINARY_BUFFER_LENGTH ( NUMBER_OF_BINARY_DIGITS + 4 )
#define BINARY_BUFFER_END    (BINARY_BUFFER_LENGTH-1)

typedef struct decimal_buffer DECIMAL_BUFFER;
typedef struct binary_buffer BINARY_BUFFER;

struct decimal_buffer {
     int head;
     int tail;
     long int exponent;
     unsigned int digit[DECIMAL_BUFFER_LENGTH];
     } ;

struct binary_buffer {
     int head;
     int tail;
     long int exponent;
     unsigned int digit[BINARY_BUFFER_LENGTH];
     } ;

typedef struct internal_decimal_format INTERNAL_DECIMAL_FORMAT;

typedef struct internal_binary_format INTERNAL_BINARY_FORMAT;

typedef struct integer_input_params INTEGER_INPUT_PARAMS;

#define FUNCTION

#ifndef INTEGER_ONLY
extern void extended_to_ieee_dp();
extern void extended_to_ieee_ep();
extern void extended_to_ieee_qp();
extern void extended_to_ieee_sp();
#endif

extern int ascii_to_decimal();
extern int ascii_to_extended();
extern int binary_increment();
extern int binary_width();
extern int extended_compare();
extern int extended_compare_less_than();
extern int extended_magnitude_compare();
extern int is_ascii_infinity_or_nan();
extern int is_binary_infinity_or_nan();
extern int is_extended_NaN();
extern int is_extended_infinity();
extern int is_extended_positive();
extern int is_extended_sNaN();
extern int is_extended_zero();

extern void binary_overflow();
extern void binary_right_shift();
extern void binary_round();
extern void binary_times_10();
extern void binary_times_2();
extern void binary_to_decimal();
extern void binary_to_extended();
extern void binary_underflow();
extern void convert_to_extended_qNaN();
extern void create_extended_infinity();
extern void create_extended_qNaN();
extern void create_extended_zero();
extern void decimal_overflow();
extern void decimal_right_shift();
extern void decimal_round();
extern void decimal_times_2();
extern void decimal_to_ascii();
extern void decimal_to_binary();
extern void decimal_underflow();
extern void extended_copy();
extern void extended_left_shift();
extern void extended_right_shift();
extern void extended_subtract();
extern void extended_to_ascii();
extern void extended_to_binary();
extern void extended_to_integer ();
extern void ieee_dp_to_extended();
extern void ieee_ep_to_extended();
extern void ieee_qp_to_extended();
extern void ieee_sp_to_extended ();
extern void integer_to_extended ();
extern void qtc_ep_add();
extern void qtc_ep_div();
extern void qtc_ep_mul();
extern void qtc_ep_postprocess();
extern void qtc_ep_sub();
extern void round_to_double();
extern void round_to_extended();
extern void round_to_IEEE_extended();
extern void round_to_integer();
extern void round_to_quad();
extern void round_to_single();
extern void set_Rounding_mode();
extern void set_Target();
extern void signal_divide_by_zero();
extern void signal_domain_error();
extern void signal_inexact();
extern void signal_invalid_operation();
extern void signal_overflow();
extern void signal_reserved_operand();
extern void signal_underflow();
extern void zero_internal_buffers();

extern INTERNAL_DECIMAL_FORMAT decimal_value;
extern INTERNAL_BINARY_FORMAT  binary_value;
extern char output_buffer[1024];
extern unsigned int hex_buffer[NUMBER_OF_HEX_DIGITS];
extern DECIMAL_BUFFER dec_buffer;
extern BINARY_BUFFER  bin_buffer;
extern int Rounding_mode;
extern int Rounding_precision;
extern int Target_exp_upper_limit;
extern int Target_exp_lower_limit;
extern int Conversion_underflow_flag;
extern int Invalid_operation_flag;
extern int Reserved_operand_flag;
extern int Underflow_flag;
extern int Overflow_flag;
extern int Inexact_flag;
extern int Divide_by_zero_flag;
extern int Domain_error_flag;

#ifdef NEWFP
#undef NEWFP
#endif
#include "cop1.h"
/*** end of qtc_ep.h ***/
