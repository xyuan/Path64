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


#pragma ident "@(#) libfi/mathlb/imod.c	92.1	07/09/99 11:00:36"


#include <fortran.h>
#include <math.h>

extern _f_int8 _IMOD_(_f_int8 *x,  _f_int8 *y);
extern _f_int8 _MOD_(_f_int8 *x,  _f_int8 *y);

/*
 * IMOD: integer(kind=8) - pass by address
 *    Remainder function - use X - INT(X/Y) * Y if Y > zero.
 *                         else...
 */
_f_int8
_IMOD_( _f_int8 *x,
	_f_int8 *y)
{
	_f_int8 a = *x;
	_f_int8 b = *y;
	return (a - b * (a/b));
}

/*
 * MOD: integer(kind=8) - pass by address
 *    Remainder function - use X - INT(X/Y) * Y if Y > zero.
 *                         else...
 */
_f_int8
_MOD_( _f_int8 *x,
	_f_int8 *y)
{
	_f_int8 a = *x;
	_f_int8 b = *y;
	return (a - b * (a/b));
}
