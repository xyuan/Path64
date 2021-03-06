/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation version 2.1

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/


static const char USMID[] = "@(#) libu/util/i3e/ieee_next_after_d_d.c	92.0	10/08/98 14:57:41";

#ifndef _LD64
#include <fenv.h>
#include <float.h>
#include <fp.h>

extern long double _IEEE_NEXT_AFTER_D_D(long double x, long double y);

/* NEXT_AFTER - return the nearest different machine representable number
 * 	        in a given direction y for 128-bit values.
 *	INPUT:  128 bit float x
 *	        128 bit float y
 * 	RETURN:
 * 	        the argument x if x = y.
 * 	        the argument x if X = NaN.
 * 	        the argument y if y = NaN.
 * 	        Raise inexact for overflow or underflow and return
 *
 */
#pragma _CRI duplicate _IEEE_NEXT_AFTER_D_D as _IEEE_NEXT_AFTER_16_16
#pragma _CRI duplicate _IEEE_NEXT_AFTER_D_D as _IEEE_NEXT_AFTER_16
#pragma _CRI duplicate _IEEE_NEXT_AFTER_D_D as _IEEE_NEXT_AFTER_D
long double
_IEEE_NEXT_AFTER_D_D(long double x, long double y)
{
        /* Union defined to work with IEEE 128 bit floating point. */
        union _ieee_ldouble {
                long double     dword;
                long            lword[2];
                struct {
                   unsigned int sign         : 1;
                   unsigned int exponent     : IEEE_128_EXPO_BITS;
                   unsigned int mantissa_up  : IEEE_128_MANT_BITS_UP;
                   unsigned int mantissa_low : IEEE_128_MANT_BITS_LOW;
                } parts;
        };

	int xfpclas	= _fpclassifyl(x);
	int yfpclas	= _fpclassifyl(y);
	if (xfpclas == FP_NAN) {
		return x;
	} else if (yfpclas == FP_NAN) {
		return y;
	} else if (xfpclas == FP_ZERO && yfpclas == FP_ZERO) {
		return x;
	} else if (x==y || xfpclas == FP_INFINITE) {
		return x;
	} else if (xfpclas == FP_ZERO) {
		union _ieee_ldouble x_val;
		x_val.dword = LDBL_MIN;
		x_val.parts.sign	 = x > y;

		/* return smallest normal number */
		return(x_val.dword);
	} else {  /* first argument is normal or denormal */
		union _ieee_ldouble x_val, f;
		unsigned int add1,add2,add3;
		int j;
		int resfpclas;

		x_val.dword	= x;
		f.dword		= x;
		add3	= IEEE_128_MANTISSA_LOW;

		if (x > 0) {
			add1	= -0x1;
			add2	= 0x1;
		} else {
			add1	= 0x1;
			add2	= -0x1;
		}

		/* move one bit in the correct direction.
		 ** Because of the way the implicit bit works,
		 ** the exponent field is handled correctly.
		 */
		x_val.lword[1] += (x>y) ? add1 : add2;

		if ((f.parts.mantissa_low == add3) ||
		    (x_val.parts.mantissa_low == add3)) {
			x_val.lword[0] += (x>y) ? add1 : add2;
		}

		/* test for underflow or overflow */
		if (_isnormall(x_val.dword))
			return(x_val.dword);

		/*
		 * Raise overflow exception for infinite result and
		 * underflow exception for too small result.  Raise
		 * inexact exception for both cases. Allow subnormal
		 * values to return without exception.
		 */
		resfpclas	= _fpclassifyl(x_val.dword);
		if (resfpclas == FP_INFINITE) {
			j	= FE_OVERFLOW;
			feraiseexcept(j);
		} else if (resfpclas == FP_ZERO) {
			j	= FE_UNDERFLOW;
			feraiseexcept(j);
		} else {
			return(x_val.dword);
		}
		j	= FE_INEXACT;
		feraiseexcept(j);
		return(x_val.dword);
	}
}

#endif  /* not LD64 */
