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


/* USMID @(#) libfi/mpp_reduce/all_s.h	92.0	10/08/98 14:37:14 */


#define GLUE(X,Y) X ## Y
#define XGLUE(X,Y) GLUE(X,Y)

void
XGLUE(_ALL_LS,RANK)(DopeVectorType * result,
	 DopeVectorType * source,
	 long * dim)
{
	long i, src_extents[RANK], res_extents[RANK-1];
	long dim_bcnt[RANK-1], blkcnts[RANK];
	void *source_sdd_ptr, *result_base_ptr;

        /*
         * If the source array is not distributed, then call
         * the current Y-MP version of the intrinsic.
         */

	if (!_is_shared_pointer(source->base_addr.a.ptr)) {
	    _ALL(result, source, dim);
	} else {

	    /*
	     * If any of the extents for the source array argument is
	     * zero, then return to the caller. Since we allocate
	     * the result array, there is no need to check that.
	     */

	    EXTENT_CHK(source);

	    /*
	     * Check to insure that DIM contains a legal value from
	     * 1 through the extent of the source array.
	     */

	    DIM_CHK(dim);

	    /*
	     *  Allocate the result temporary and fill in the dope
	     *  vector.
	     */

	    _malloc_reduced_result(result, source, dim, &result_base_ptr);

	    /*
	     * Set up local pointers to the source and result arrays.
	     */

	    source_sdd_ptr = source->base_addr.a.ptr;

	    /*
	     * Set up arrays of extents for the source and result array.
	     * Macro SETUP_EXTENTS uses variables result, source, src_extents,
	     * res_extents, and dim.
	     */

	    SETUP_EXTENTS();

	    /*
	     * Initialize an array of block counts which contain the
	     * number of blocks of the distributed source array on MY_PE
	     * for each dimension. Macro SETUP_BLKCNTS() uses variables
	     * source, blkcnts, dim, and dim_bcnt.
	     */

	    SETUP_BLKCNTS();

	    /*
	     * Call the Fortran work routine.
	     */

	    XGLUE(XGLUE(ALL_I_LS,RANK),@)(result_base_ptr, source_sdd_ptr,
	    		dim, res_extents, src_extents, blkcnts, dim_bcnt);
	}

}
