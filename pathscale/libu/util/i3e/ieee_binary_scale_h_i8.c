/*
 * Copyright (C) 2007. PathScale, LLC. All Rights Reserved.
 */
/*
 * Copyright 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

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


#pragma ident "@(#) libu/util/i3e/ieee_binary_scale_h_i8.c	92.1	07/07/99 14:37:09"

#include <fenv.h>
#include <fp.h>
#include "i3eintrin.h"

extern int _leadz(int);
extern float _IEEE_BINARY_SCALE_H_I8(float x, int n);

/* BINARY_SCALE - computes x * 2**n efficiently by not computing 2**n
 *	INPUT:   32 bit float x
 *	         64 bit integer n
 *	OUTPUT:  32 bit float result
 */

#pragma _CRI duplicate _IEEE_BINARY_SCALE_H_I8 as _IEEE_BINARY_SCALE_4_I8
float
_IEEE_BINARY_SCALE_H_I8(float x, int n)
{  
	/* Union defined to work with IEEE 32-bit floating point. */
	union _ieee_float {
		float	dword;
		int	lword;
		unsigned int	ulword;
		struct {
#if defined(_LITTLE_ENDIAN)
			unsigned int mantissa	: IEEE_32_MANT_BITS;
			unsigned int exponent	: IEEE_32_EXPO_BITS;
			unsigned int sign	: 1;
#else
			unsigned int sign	: 1;
			unsigned int exponent	: IEEE_32_EXPO_BITS;
			unsigned int mantissa	: IEEE_32_MANT_BITS;
#endif
		} parts;
	};

	int fp_class	= _fpclassifyf(x);

	if (fp_class==FP_NORMAL) {
		union _ieee_float x_val;
		long exponent;
		x_val.dword = x;

		exponent = x_val.parts.exponent + n;

		if (exponent <= 0) {
			int j;
			long full_mantissa = x_val.parts.mantissa;
			/* need to emit underflow exception */
			j	= FE_UNDERFLOW;
			feraiseexcept(j);

			/* add implicit bit to mantissa */
			full_mantissa |=  IEEE_32_IMPLICIT_BIT;

			/* shift mantissa over by exponent remaining */
			full_mantissa >>= -exponent + 1;

			/* return denormal number */
			x_val.parts.exponent = 0;
			x_val.parts.mantissa = full_mantissa;

		} else if ((exponent>>IEEE_32_EXPO_BITS) != 0) {
			int j;
			unsigned int k	= x_val.parts.sign;

			/* need to emit overflow exception */
			j	= FE_OVERFLOW;
			feraiseexcept(j);

			/* return infinity */
			x_val.ulword = IEEE_32_INFINITY;
			x_val.parts.sign = k;
		} else {
			x_val.parts.exponent = exponent;
		}
		return(x_val.dword);

	} else if (fp_class==FP_SUBNORMAL) {
		union _ieee_float x_val;
		x_val.dword = x;

		if (n <= 0) {
			x_val.parts.mantissa = x_val.parts.mantissa >> (-n);
		} else {
			long mantissa_bits_to_fill =
				_leadz(x_val.parts.mantissa);

			if (mantissa_bits_to_fill >= n) {
				x_val.parts.mantissa =
					x_val.parts.mantissa << n;
			} else {

				/* Return a normal number.  Shift out
				 * implicit bit.
				 */
				x_val.parts.mantissa = x_val.parts.mantissa <<
					(mantissa_bits_to_fill + 1);
				x_val.parts.exponent = n -
					mantissa_bits_to_fill + 1;
			}
		}

		return(x_val.dword);
	} else { /* NAN, INFINITE, or ZERO */
		return(x);
	}
}
