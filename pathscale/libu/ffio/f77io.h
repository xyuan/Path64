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


/* USMID @(#) libu/ffio/f77io.h	92.0	10/08/98 14:57:41 */

#ifndef _F77IO_H
#define _F77IO_H
#include <cray/portdefs.h>

#define TR_UX_MIPS	3	/* UNIX tm blocking MIPS */
#define TR_UX_INTEL	4	/* UNIX tm blocking Intel */


/*
 * Useful constants.
 */
#define WPBLOCK		512			/* words per 512 wd disk block*/
#define BYTPBLOCK	4096			/* bytes per 512 wd disk block*/
#define BITPBLOCK	32768			/* bits per 512 wd disk block*/
#define F77_BUFSIZ	8			/* dflt bufsize in 512-wd blks*/

#define ATEOR		1
/*
 * Macro to swap bytes for VAX formats.  Works on low 32 bits only.
 */
#define SWAPB(jj)		{					\
				jj = ((jj >> 16) & 0xFFFF) | jj << 16;	\
				jj =	((jj >> 8) & 0x00FF00FF) |	\
					((jj << 8) & 0xFF00FF00);	\
				}



#ifndef __FF_OFFT
#define __FF_OFFT
#ifdef __mips
typedef off64_t ff_offt;
#else
typedef off_t ff_offt;
#endif
#endif
/*
 *	Private info structure
 */
#define RDWLEN 4			/* segment descriptor length in bytes */
struct f77_xf			/* X blocking */
	{

	size_t	rembytes;	/* For read: data bytes left in this record */
				/* that are not in the current buffer */
	ff_offt	last_lrdwaddr;	/* last leading rec desc file pointer */
				/* File offset of control word at start of */
				/* last record. This is a byte offset */
	ff_offt	lrdwaddr;	/* leading record descriptor file pointer */
				/* File offset of control word at start of */
				/* this record. This is a byte offset. */
	char	*_ptr;		/* Pointer to next data in buffer. */
	char	*_base;		/* Pointer to base of buffer. */
	long	_cnt;		/* Byte count. Bytes left in buffer on read */
				/* Bytes accumulated on write. */
	long	_ffbufsiz;	/* Byte size of buffer */
	size_t  recbytes;	/* Byte size of record. Includes control words. */
	size_t  maxrecsize;	/* Maximum size of record in bytes */
	int	cwwritten;	/* Beginning control word has been inserted */
	int	flag;
	unsigned do_sylistio:1;	/* 1 if we can issue simple listio calls */
	};

extern ssize_t _f77_xread(struct fdinfo *fio, bitptr bufptr, size_t nbytes,
        struct ffsw *retstat, int fulp, int *ubc);
extern ssize_t _f77_xwrite(struct fdinfo *fio, bitptr bufptr, size_t nbytes,
        struct ffsw *retstat, int fulp, int *ubc);
extern _ffseek_t _f77_xseek(struct fdinfo *fio, off_t pos, int whence,
        struct ffsw *stat);
extern _ffopen_t _f77_xopen(const char *name, int flags, mode_t mode, 
	struct fdinfo *fio, union spec_u *spec, struct ffsw *stat, long cbits,
	int cblks, struct gl_o_inf *oinf);
extern int _f77_xbksp(struct fdinfo *fio, struct ffsw *stat);
extern int _f77skip2eor(struct fdinfo *fio, struct ffsw *stat);

#endif
