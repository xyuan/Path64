;/* ============================================================
;Copyright (c) 2005 Advanced Micro Devices, Inc.
;
;All rights reserved.
;
;Redistribution and  use in source and binary  forms, with or
;without  modification,  are   permitted  provided  that  the
;following conditions are met:
;
;
;+ Redistributions  of source  code  must  retain  the  above
;  copyright  notice,   this  list  of   conditions  and  the
;  following disclaimer.
;
;+ Redistributions  in binary  form must reproduce  the above
;  copyright  notice,   this  list  of   conditions  and  the
;  following  disclaimer in  the  documentation and/or  other
;  materials provided with the distribution.
;
;+ Neither the  name of Advanced Micro Devices,  Inc. nor the
;  names  of  its contributors  may  be  used  to endorse  or
;  promote  products  derived   from  this  software  without
;  specific prior written permission.
;
;THIS  SOFTWARE  IS PROVIDED  BY  THE  COPYRIGHT HOLDERS  AND
;CONTRIBUTORS "AS IS" AND  ANY EXPRESS OR IMPLIED WARRANTIES,
;INCLUDING,  BUT NOT  LIMITED TO,  THE IMPLIED  WARRANTIES OF
;MERCHANTABILITY  AND FITNESS  FOR A  PARTICULAR  PURPOSE ARE
;DISCLAIMED.  IN  NO  EVENT  SHALL  ADVANCED  MICRO  DEVICES,
;INC.  OR CONTRIBUTORS  BE LIABLE  FOR ANY  DIRECT, INDIRECT,
;INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL  DAMAGES
;(INCLUDING,  BUT NOT LIMITED  TO, PROCUREMENT  OF SUBSTITUTE
;GOODS  OR  SERVICES;  LOSS  OF  USE, DATA,  OR  PROFITS;  OR
;BUSINESS INTERRUPTION)  HOWEVER CAUSED AND ON  ANY THEORY OF
;LIABILITY,  WHETHER IN CONTRACT,  STRICT LIABILITY,  OR TORT
;(INCLUDING NEGLIGENCE  OR OTHERWISE) ARISING IN  ANY WAY OUT
;OF  THE  USE  OF  THIS  SOFTWARE, EVEN  IF  ADVISED  OF  THE
;POSSIBILITY OF SUCH DAMAGE.
;
;In  the  redistribution and use of this software, each party
;shall  at  all times comply with all applicable governmental
;laws,  statutes, ordinances, rules, regulations, orders, and
;other   requirements,   including  without  limitation  such
;governmental   requirements   applicable   to  environmental
;protection,  health,  safety, wages, hours, equal employment
;opportunity,  nondiscrimination,  working conditions, import
;or export control, and transportation.  Without limiting the
;foregoing,  each  party  shall  adhere  to  the  U.S. Export
;Administration   Regulations  (EAR),  currently   found   at
;15 C.F.R. Sections 730  through  744,  and,  unless properly
;authorized  by  the  U.S. Government,  shall not (1) export,
;re-export  or  release  restricted  technology, software, or
;source  code  to  a  national of a country in Country Groups
;D:1 or E:1,  or  (2) export to Country Groups D:1 or E:1 the
;direct  product  of  such  technology  or  software, if such
;foreign  produced  direct  product  is  subject  to national
;security controls as identified on the Commerce Control List
;(currently  found  in  Supplement 1  to Section 774 of EAR).
;These  export  requirements  shall survive any expiration or
;termination of this agreement.
;============================================================ */

#
# vrd2log10.s
#
# An implementation of the log10 libm function.
#
# Prototype:
#
#     __m128d __vrd2_log10(__m128d x);
#
#   Computes the natural log10 of x.
#   Returns proper C99 values, but may not raise status flags properly.
#   Less than 1 ulp of error.  Runs 120-130 cycles for valid inputs.
#
#

# define local variable storage offsets
.equ	p_x,0			# temporary for error checking operation
.equ	p_idx,0x010		# index storage

.equ	stack_size,0x028



    .text
    .align 16
    .p2align 4,,15
.globl __vrd2_log10
    .type   __vrd2_log10,@function
__vrd2_log10:
	sub		$stack_size,%rsp

	movdqa	%xmm0,p_x(%rsp)	# save the input values


#      /* Store the exponent of x in xexp and put
#         f into the range [0.5,1) */

#
# compute the index into the log10 tables
#
	pxor	%xmm1,%xmm1
	movdqa	%xmm0,%xmm3
	psrlq	$52,%xmm3
	psubq	.L__mask_1023(%rip),%xmm3
	packssdw	%xmm1,%xmm3
	cvtdq2pd	%xmm3,%xmm6			# xexp
	movdqa	%xmm0,%xmm2
	xor		%rax,%rax
	subpd	.L__real_one(%rip),%xmm2

	movdqa	%xmm0,%xmm3
	andpd	.L__real_notsign(%rip),%xmm2
	pand	.L__real_mant(%rip),%xmm3
	movdqa	%xmm3,%xmm4
	movapd	.L__real_half(%rip),%xmm5							# .5

	cmppd	$1,.L__real_threshold(%rip),%xmm2
	movmskpd	%xmm2,%r10d
	cmp		$3,%r10d
	jz		.Lall_nearone

#/* Now  x = 2**xexp  * f,  1/2 <= f < 1. */
	psrlq	$45,%xmm3
	movdqa	%xmm3,%xmm2
	psrlq	$1,%xmm3
	paddq	.L__mask_040(%rip),%xmm3
	pand	.L__mask_001(%rip),%xmm2
	paddq	%xmm2,%xmm3

	packssdw	%xmm1,%xmm3
	cvtdq2pd	%xmm3,%xmm1
	xor		%rcx,%rcx
	movq	 %xmm3,p_idx(%rsp)

# reduce and get u
	por		.L__real_half(%rip),%xmm4
	movdqa	%xmm4,%xmm2


	mulpd	.L__real_3f80000000000000(%rip),%xmm1		# f1 = index/128


	lea	.L__np_ln_lead_table(%rip),%rdx
	mov	p_idx(%rsp),%eax

	subpd	%xmm1,%xmm2				# f2 = f - f1
	mulpd	%xmm2,%xmm5
	addpd	%xmm5,%xmm1

	divpd	%xmm1,%xmm2				# u

# do error checking here for scheduling.  Saves a bunch of cycles as
# compared to doing this at the start of the routine.
#  if NaN or inf
	movapd	%xmm0,%xmm3
	andpd	.L__real_inf(%rip),%xmm3
	cmppd	$0,.L__real_inf(%rip),%xmm3
	movmskpd	%xmm3,%r8d
	xorpd	%xmm1,%xmm1

	cmppd	$2,%xmm1,%xmm0
	movmskpd	%xmm0,%r9d

# get z
	movlpd	 -512(%rdx,%rax,8),%xmm0		# z1
	mov		p_idx+4(%rsp),%ecx
	movhpd	 -512(%rdx,%rcx,8),%xmm0		# z1
# solve for ln(1+u)
	movapd	%xmm2,%xmm1				# u
	mulpd	%xmm2,%xmm2				# u^2
	movapd	%xmm2,%xmm5
	movapd	.L__real_cb3(%rip),%xmm3
	mulpd	%xmm2,%xmm3				#Cu2
	mulpd	%xmm1,%xmm5				# u^3
	addpd	.L__real_cb2(%rip),%xmm3 		#B+Cu2

	mulpd	%xmm5,%xmm2				# u^5
	movapd	.L__real_log2_lead(%rip),%xmm4

	mulpd	.L__real_cb1(%rip),%xmm5 		#Au3
	addpd	%xmm5,%xmm1				# u+Au3
	mulpd	%xmm3,%xmm2				# u5(B+Cu2)

	addpd	%xmm2,%xmm1				# poly
# recombine		
	mulpd	%xmm6,%xmm4				# xexp * log2_lead
	addpd	%xmm4,%xmm0				#r1
	movapd  %xmm0,%xmm2
	lea		.L__np_ln_tail_table(%rip),%rdx
	movlpd	 -512(%rdx,%rax,8),%xmm4		#z2	+=q
	movhpd	 -512(%rdx,%rcx,8),%xmm4		#z2	+=q
	addpd	%xmm4,%xmm1

	mulpd	.L__real_log2_tail(%rip),%xmm6

	addpd	%xmm6,%xmm1				#r2
	
#   loge to log10
	movapd  %xmm1,%xmm3
	mulpd 	.L__real_log10e_tail(%rip),%xmm1
	mulpd 	.L__real_log10e_tail(%rip),%xmm0
	addpd   %xmm1,%xmm0
	mulpd 	.L__real_log10e_lead(%rip),%xmm3
	addpd 	%xmm3,%xmm0
	mulpd 	.L__real_log10e_lead(%rip),%xmm2
# check for nans/infs
	test	$3,%r8d
	addpd	%xmm2,%xmm0

	jnz		.L__log_naninf
.L__vlog1:
# check for negative numbers or zero
	test		$3,%r9d
	jnz		.L__z_or_n 

.L__finish:
# see if we have a near one value
	test		 $3,%r10d
	jnz		.L__near_one
.L__finishn1:
	add		$stack_size,%rsp
	ret

	.align	16
.Lall_nearone:
# saves 10 cycles
#      r = x - 1.0;
	movapd	.L__real_two(%rip),%xmm2
	subpd	.L__real_one(%rip),%xmm0	   # r
#      u          = r / (2.0 + r);
	addpd	%xmm0,%xmm2
	movapd	%xmm0,%xmm1
	divpd	%xmm2,%xmm1			# u
	movapd	.L__real_ca4(%rip),%xmm4	  #D
	movapd	.L__real_ca3(%rip),%xmm5	  #C
#      correction = r * u;
	movapd	%xmm0,%xmm6
	mulpd	%xmm1,%xmm6		# correction
#      u          = u + u;
	addpd	%xmm1,%xmm1		#u
	movapd	%xmm1,%xmm2
	mulpd	%xmm2,%xmm2		#v =u^2
#      r2 = (u * v * (ca_1 + v * (ca_2 + v * (ca_3 + v * ca_4))) - correction);
	mulpd	%xmm1,%xmm5		# Cu
	movapd	%xmm1,%xmm3
	mulpd	%xmm2,%xmm3		# u^3
	mulpd	.L__real_ca2(%rip),%xmm2	#Bu^2
	mulpd	%xmm3,%xmm4		#Du^3

	addpd	.L__real_ca1(%rip),%xmm2	# +A
	movapd	%xmm3,%xmm1
	mulpd	%xmm1,%xmm1		# u^6
	addpd	%xmm4,%xmm5		#Cu+Du3
#	subsd	%xmm6,%xmm0		; -correction

	mulpd	%xmm3,%xmm2		#u3(A+Bu2)
	mulpd	%xmm5,%xmm1		#u6(Cu+Du3)
	addpd	%xmm1,%xmm2
	subpd	%xmm6,%xmm2		# -correction

#	loge to log10	
	movapd 	%xmm0,%xmm3		#r1 = r
	pand	.L__mask_lower(%rip),%xmm3
	subpd	%xmm3,%xmm0
	addpd 	%xmm0,%xmm2		#r2 = r2 + (r - r1);
	
	movapd 	%xmm3,%xmm0
	movapd	%xmm2,%xmm1
	
	mulpd 	.L__real_log10e_tail(%rip),%xmm2
	mulpd 	.L__real_log10e_tail(%rip),%xmm0
	mulpd 	.L__real_log10e_lead(%rip),%xmm1
	mulpd 	.L__real_log10e_lead(%rip),%xmm3
	addpd 	%xmm2,%xmm0
	addpd 	%xmm1,%xmm0
	addpd	%xmm3,%xmm0	
	
#      return r + r2;
#	addpd	%xmm2,%xmm0
	jmp		.L__finishn1

	.align	16
.L__near_one:
	test	$1,%r10d
	jz		.L__lnn12

#	movapd	%xmm0,%xmm6		; save the inputs
	movlpd	p_x(%rsp),%xmm0
	call	.L__ln1
#	shufpd	xmm0,$2,%xmm6

.L__lnn12:
	test	$2,%r10d		# second number?
	jz		.L__lnn1e
	movlpd	%xmm0,p_x(%rsp)
	movlpd	p_x+8(%rsp),%xmm0
	call	.L__ln1
	movlpd	%xmm0,p_x+8(%rsp)
	movapd	p_x(%rsp),%xmm0
#	shufpd	xmm6,$0,%xmm0
#	movapd	%xmm6,%xmm0

.L__lnn1e:
	jmp		.L__finishn1

.L__ln1:
# saves 10 cycles
#      r = x - 1.0;
	movlpd	.L__real_two(%rip),%xmm2
	subsd	.L__real_one(%rip),%xmm0	# r
#      u          = r / (2.0 + r);
	addsd	%xmm0,%xmm2
	movsd	%xmm0,%xmm1
	divsd	%xmm2,%xmm1			# u
	movlpd	.L__real_ca4(%rip),%xmm4	#D
	movlpd	.L__real_ca3(%rip),%xmm5	#C
#      correction = r * u;
	movsd	%xmm0,%xmm6
	mulsd	%xmm1,%xmm6		# correction
#      u          = u + u;
	addsd	%xmm1,%xmm1		#u
	movsd	%xmm1,%xmm2
	mulsd	%xmm2,%xmm2		#v =u^2
#      r2 = (u * v * (ca_1 + v * (ca_2 + v * (ca_3 + v * ca_4))) - correction);
	mulsd	%xmm1,%xmm5		# Cu
	movsd	%xmm1,%xmm3
	mulsd	%xmm2,%xmm3		# u^3
	mulsd	.L__real_ca2(%rip),%xmm2	#Bu^2
	mulsd	%xmm3,%xmm4		#Du^3

	addsd	.L__real_ca1(%rip),%xmm2	# +A
	movsd	%xmm3,%xmm1
	mulsd	%xmm1,%xmm1		# u^6
	addsd	%xmm4,%xmm5		#Cu+Du3
#	subsd	%xmm6,%xmm0		; -correction

	mulsd	%xmm3,%xmm2		#u3(A+Bu2)
	mulsd	%xmm5,%xmm1		#u6(Cu+Du3)
	addsd	%xmm1,%xmm2
	subsd	%xmm6,%xmm2		# -correction
	
#	loge to log10
	movsd 	%xmm0,%xmm3		#r1 = r
	pand	.L__mask_lower(%rip),%xmm3
	subsd	%xmm3,%xmm0
	addsd 	%xmm0,%xmm2		#r2 = r2 + (r - r1);
	
	movsd 	%xmm3,%xmm0
	movsd	%xmm2,%xmm1
	
	mulsd 	.L__real_log10e_tail(%rip),%xmm2	
	mulsd 	.L__real_log10e_tail(%rip),%xmm0
	mulsd 	.L__real_log10e_lead(%rip),%xmm1
	mulsd 	.L__real_log10e_lead(%rip),%xmm3
	addsd 	%xmm2,%xmm0
	addsd 	%xmm1,%xmm0
	addsd	%xmm3,%xmm0
	
	
		
#      return r + r2;
#	addsd	%xmm2,%xmm0
	ret

	.align	16

# at least one of the numbers was a nan or infinity
.L__log_naninf:
	test		$1,%r8d		# first number?
	jz		.L__lninf2

	movapd	%xmm0,%xmm1		# save the inputs
	mov		p_x(%rsp),%rdx
	movlpd	p_x(%rsp),%xmm0
	call	.L__lni
	shufpd	$2,%xmm1,%xmm0

.L__lninf2:
	test		$2,%r8d		# second number?
	jz		.L__lninfe
	movapd	%xmm0,%xmm1		# save the inputs
	mov	p_x+8(%rsp),%rdx
	movlpd	p_x+8(%rsp),%xmm0
	call	.L__lni
	shufpd	$0,%xmm0,%xmm1
	movapd	%xmm1,%xmm0

.L__lninfe:
	
	cmp		$3,%r8d		# both numbers?
	jz		.L__finish	# return early if so
	jmp		.L__vlog1	# continue processing if not

# a subroutine to treat one number for nan/infinity
# the number is expected in rdx and returned in the low
# half of xmm0
.L__lni:
	mov		$0x0000FFFFFFFFFFFFF,%rax
	test	%rax,%rdx
	jnz		.L__lnan	# jump if mantissa not zero, so it's a NaN
# inf
	rcl		$1,%rdx
	jnc		.L__lne2	# log(+inf) = inf
# negative x
	movlpd	.L__real_nan(%rip),%xmm0
	ret

#NaN
.L__lnan:	
	mov		$0x00008000000000000,%rax	# convert to quiet
	or		%rax,%rdx
.L__lne:	
	movd	%rdx,%xmm0
.L__lne2:	
	ret

	.align	16

# at least one of the numbers was a zero, a negative number, or both.
.L__z_or_n:
	test		$1,%r9d		# first number?
	jz		.L__zn2

#	movapd	%xmm0,%xmm1		# save the inputs
	mov		p_x(%rsp),%rax
	call	.L__zni
#	shufpd	$2,%xmm1,%xmm0

.L__zn2:
	test		$2,%r9d		# second number?
	jz		.L__zne
	movapd	%xmm0,%xmm1		# save the inputs
	mov		p_x+8(%rsp),%rax
	call	.L__zni
	shufpd	$0,%xmm0,%xmm1
	movapd	%xmm1,%xmm0

.L__zne:
	jmp		.L__finish		

# a subroutine to treat one number for zero or negative values
# the number is expected in rax and returned in the low
# half of xmm0
.L__zni:
	shl		$1,%rax
	jnz		.L__zn_x		 # if just a carry, then must be negative
	movlpd	.L__real_ninf(%rip),%xmm0  # C99 specs -inf for +-0
	ret
.L__zn_x:
	movlpd	.L__real_nan(%rip),%xmm0
	ret




	.data
	.align	16

.L__real_one:			.quad 0x03ff0000000000000	# 1.0
				.quad 0x03ff0000000000000
.L__real_two:			.quad 0x04000000000000000	# 2.0
				.quad 0x04000000000000000
.L__real_ninf:			.quad 0x0fff0000000000000	# -inf
				.quad 0x0fff0000000000000
.L__real_inf:			.quad 0x07ff0000000000000	# +inf
				.quad 0x07ff0000000000000
.L__real_nan:			.quad 0x07ff8000000000000	# NaN
				.quad 0x07ff8000000000000

.L__real_sign:			.quad 0x08000000000000000	# sign bit
				.quad 0x08000000000000000
.L__real_notsign:		.quad 0x07ffFFFFFFFFFFFFF	# ^sign bit
				.quad 0x07ffFFFFFFFFFFFFF
.L__real_threshold:		.quad 0x03FB082C000000000	# .064495086669921875 Threshold
				.quad 0x03FB082C000000000
.L__real_qnanbit:		.quad 0x00008000000000000	# quiet nan bit
				.quad 0x00008000000000000
.L__real_mant:			.quad 0x0000FFFFFFFFFFFFF	# mantissa bits
				.quad 0x0000FFFFFFFFFFFFF
.L__real_3f80000000000000:	.quad 0x03f80000000000000	# /* 0.0078125 = 1/128 */
				.quad 0x03f80000000000000
.L__mask_1023:			.quad 0x000000000000003ff	# 
				.quad 0x000000000000003ff
.L__mask_040:			.quad 0x00000000000000040	# 
				.quad 0x00000000000000040
.L__mask_001:			.quad 0x00000000000000001	# 
				.quad 0x00000000000000001

.L__real_ca1:			.quad 0x03fb55555555554e6	# 8.33333333333317923934e-02
				.quad 0x03fb55555555554e6
.L__real_ca2:			.quad 0x03f89999999bac6d4	# 1.25000000037717509602e-02
				.quad 0x03f89999999bac6d4
.L__real_ca3:			.quad 0x03f62492307f1519f	# 2.23213998791944806202e-03
				.quad 0x03f62492307f1519f
.L__real_ca4:			.quad 0x03f3c8034c85dfff0	# 4.34887777707614552256e-04
				.quad 0x03f3c8034c85dfff0

.L__real_cb1:			.quad 0x03fb5555555555557	# 8.33333333333333593622e-02
				.quad 0x03fb5555555555557
.L__real_cb2:			.quad 0x03f89999999865ede	# 1.24999999978138668903e-02
				.quad 0x03f89999999865ede
.L__real_cb3:			.quad 0x03f6249423bd94741	# 2.23219810758559851206e-03
				.quad 0x03f6249423bd94741
.L__real_log2_lead:  		.quad 0x03fe62e42e0000000	# log2_lead	  6.93147122859954833984e-01
				.quad 0x03fe62e42e0000000
.L__real_log2_tail: 		.quad 0x03e6efa39ef35793c	# log2_tail	  5.76999904754328540596e-08
				.quad 0x03e6efa39ef35793c

.L__real_half:			.quad 0x03fe0000000000000	# 1/2
				.quad 0x03fe0000000000000
				
.L__real_log10e_lead:	.quad 0x03fdbcb7800000000	# log10e_lead 4.34293746948242187500e-01
				.quad 0x03fdbcb7800000000
.L__real_log10e_tail:	.quad 0x03ea8a93728719535	# log10e_tail 7.3495500964015109100644e-7
				.quad 0x03ea8a93728719535
						
.L__mask_lower:			.quad 0x0ffffffff00000000
				.quad 0x0ffffffff00000000
	
	.align	16

.L__np_ln_lead_table:
	.quad	0x0000000000000000 		# 0.00000000000000000000e+00
	.quad	0x3f8fc0a800000000		# 1.55041813850402832031e-02
	.quad	0x3f9f829800000000		# 3.07716131210327148438e-02
	.quad	0x3fa7745800000000		# 4.58095073699951171875e-02
	.quad	0x3faf0a3000000000		# 6.06245994567871093750e-02
	.quad	0x3fb341d700000000		# 7.52233862876892089844e-02
	.quad	0x3fb6f0d200000000		# 8.96121263504028320312e-02
	.quad	0x3fba926d00000000		# 1.03796780109405517578e-01
	.quad	0x3fbe270700000000		# 1.17783010005950927734e-01
	.quad	0x3fc0d77e00000000		# 1.31576299667358398438e-01
	.quad	0x3fc2955280000000		# 1.45181953907012939453e-01
	.quad	0x3fc44d2b00000000		# 1.58604979515075683594e-01
	.quad	0x3fc5ff3000000000		# 1.71850204467773437500e-01
	.quad	0x3fc7ab8900000000		# 1.84922337532043457031e-01
	.quad	0x3fc9525a80000000		# 1.97825729846954345703e-01
	.quad	0x3fcaf3c900000000		# 2.10564732551574707031e-01
	.quad	0x3fcc8ff780000000		# 2.23143517971038818359e-01
	.quad	0x3fce270700000000		# 2.35566020011901855469e-01
	.quad	0x3fcfb91800000000		# 2.47836112976074218750e-01
	.quad	0x3fd0a324c0000000		# 2.59957492351531982422e-01
	.quad	0x3fd1675c80000000		# 2.71933674812316894531e-01
	.quad	0x3fd22941c0000000		# 2.83768117427825927734e-01
	.quad	0x3fd2e8e280000000		# 2.95464158058166503906e-01
	.quad	0x3fd3a64c40000000		# 3.07025015354156494141e-01
	.quad	0x3fd4618bc0000000		# 3.18453729152679443359e-01
	.quad	0x3fd51aad80000000		# 3.29753279685974121094e-01
	.quad	0x3fd5d1bd80000000		# 3.40926527976989746094e-01
	.quad	0x3fd686c800000000		# 3.51976394653320312500e-01
	.quad	0x3fd739d7c0000000		# 3.62905442714691162109e-01
	.quad	0x3fd7eaf800000000		# 3.73716354370117187500e-01
	.quad	0x3fd89a3380000000		# 3.84411692619323730469e-01
	.quad	0x3fd9479400000000		# 3.94993782043457031250e-01
	.quad	0x3fd9f323c0000000		# 4.05465066432952880859e-01
	.quad	0x3fda9cec80000000		# 4.15827870368957519531e-01
	.quad	0x3fdb44f740000000		# 4.26084339618682861328e-01
	.quad	0x3fdbeb4d80000000		# 4.36236739158630371094e-01
	.quad	0x3fdc8ff7c0000000		# 4.46287095546722412109e-01
	.quad	0x3fdd32fe40000000		# 4.56237375736236572266e-01
	.quad	0x3fddd46a00000000		# 4.66089725494384765625e-01
	.quad	0x3fde744240000000		# 4.75845873355865478516e-01
	.quad	0x3fdf128f40000000		# 4.85507786273956298828e-01
	.quad	0x3fdfaf5880000000		# 4.95077252388000488281e-01
	.quad	0x3fe02552a0000000		# 5.04556000232696533203e-01
	.quad	0x3fe0723e40000000		# 5.13945698738098144531e-01
	.quad	0x3fe0be72e0000000		# 5.23248136043548583984e-01
	.quad	0x3fe109f380000000		# 5.32464742660522460938e-01
	.quad	0x3fe154c3c0000000		# 5.41597247123718261719e-01
	.quad	0x3fe19ee6a0000000		# 5.50647079944610595703e-01
	.quad	0x3fe1e85f40000000		# 5.59615731239318847656e-01
	.quad	0x3fe23130c0000000		# 5.68504691123962402344e-01
	.quad	0x3fe2795e00000000		# 5.77315330505371093750e-01
	.quad	0x3fe2c0e9e0000000		# 5.86049020290374755859e-01
	.quad	0x3fe307d720000000		# 5.94707071781158447266e-01
	.quad	0x3fe34e2880000000		# 6.03290796279907226562e-01
	.quad	0x3fe393e0c0000000		# 6.11801505088806152344e-01
	.quad	0x3fe3d90260000000		# 6.20240390300750732422e-01
	.quad	0x3fe41d8fe0000000		# 6.28608644008636474609e-01
	.quad	0x3fe4618bc0000000		# 6.36907458305358886719e-01
	.quad	0x3fe4a4f840000000		# 6.45137906074523925781e-01
	.quad	0x3fe4e7d800000000		# 6.53301239013671875000e-01
	.quad	0x3fe52a2d20000000		# 6.61398470401763916016e-01
	.quad	0x3fe56bf9c0000000		# 6.69430613517761230469e-01
	.quad	0x3fe5ad4040000000		# 6.77398800849914550781e-01
	.quad	0x3fe5ee02a0000000		# 6.85303986072540283203e-01
	.quad	0x3fe62e42e0000000		# 6.93147122859954833984e-01
	.quad 0					# for alignment

.L__np_ln_tail_table:
	.quad	0x00000000000000000 # 0	; 0.00000000000000000000e+00
	.quad	0x03e361f807c79f3db		# 5.15092497094772879206e-09
	.quad	0x03e6873c1980267c8		# 4.55457209735272790188e-08
	.quad	0x03e5ec65b9f88c69e		# 2.86612990859791781788e-08
	.quad	0x03e58022c54cc2f99		# 2.23596477332056055352e-08
	.quad	0x03e62c37a3a125330		# 3.49498983167142274770e-08
	.quad	0x03e615cad69737c93		# 3.23392843005887000414e-08
	.quad	0x03e4d256ab1b285e9		# 1.35722380472479366661e-08
	.quad	0x03e5b8abcb97a7aa2		# 2.56504325268044191098e-08
	.quad	0x03e6f34239659a5dc		# 5.81213608741512136843e-08
	.quad	0x03e6e07fd48d30177		# 5.59374849578288093334e-08
	.quad	0x03e6b32df4799f4f6		# 5.06615629004996189970e-08
	.quad	0x03e6c29e4f4f21cf8		# 5.24588857848400955725e-08
	.quad	0x03e1086c848df1b59		# 9.61968535632653505972e-10
	.quad	0x03e4cf456b4764130		# 1.34829655346594463137e-08
	.quad	0x03e63a02ffcb63398		# 3.65557749306383026498e-08
	.quad	0x03e61e6a6886b0976		# 3.33431709374069198903e-08
	.quad	0x03e6b8abcb97a7aa2		# 5.13008650536088382197e-08
	.quad	0x03e6b578f8aa35552		# 5.09285070380306053751e-08
	.quad	0x03e6139c871afb9fc		# 3.20853940845502057341e-08
	.quad	0x03e65d5d30701ce64		# 4.06713248643004200446e-08
	.quad	0x03e6de7bcb2d12142		# 5.57028186706125221168e-08
	.quad	0x03e6d708e984e1664		# 5.48356693724804282546e-08
	.quad	0x03e556945e9c72f36		# 1.99407553679345001938e-08
	.quad	0x03e20e2f613e85bda		# 1.96585517245087232086e-09
	.quad	0x03e3cb7e0b42724f6		# 6.68649386072067321503e-09
	.quad	0x03e6fac04e52846c7		# 5.89936034642113390002e-08
	.quad	0x03e5e9b14aec442be		# 2.85038578721554472484e-08
	.quad	0x03e6b5de8034e7126		# 5.09746772910284482606e-08
	.quad	0x03e6dc157e1b259d3		# 5.54234668933210171467e-08
	.quad	0x03e3b05096ad69c62		# 6.29100830926604004874e-09
	.quad	0x03e5c2116faba4cdd		# 2.61974119468563937716e-08
	.quad	0x03e665fcc25f95b47		# 4.16752115011186398935e-08
	.quad	0x03e5a9a08498d4850		# 2.47747534460820790327e-08
	.quad	0x03e6de647b1465f77		# 5.56922172017964209793e-08
	.quad	0x03e5da71b7bf7861d		# 2.76162876992552906035e-08
	.quad	0x03e3e6a6886b09760		# 7.08169709942321478061e-09
	.quad	0x03e6f0075eab0ef64		# 5.77453510221151779025e-08
	.quad	0x03e33071282fb989b		# 4.43021445893361960146e-09
	.quad	0x03e60eb43c3f1bed2		# 3.15140984357495864573e-08
	.quad	0x03e5faf06ecb35c84		# 2.95077445089736670973e-08
	.quad	0x03e4ef1e63db35f68		# 1.44098510263167149349e-08
	.quad	0x03e469743fb1a71a5		# 1.05196987538551827693e-08
	.quad	0x03e6c1cdf404e5796		# 5.23641361722697546261e-08
	.quad	0x03e4094aa0ada625e		# 7.72099925253243069458e-09
	.quad	0x03e6e2d4c96fde3ec		# 5.62089493829364197156e-08
	.quad	0x03e62f4d5e9a98f34		# 3.53090261098577946927e-08
	.quad	0x03e6467c96ecc5cbe		# 3.80080516835568242269e-08
	.quad	0x03e6e7040d03dec5a		# 5.66961038386146408282e-08
	.quad	0x03e67bebf4282de36		# 4.42287063097349852717e-08
	.quad	0x03e6289b11aeb783f		# 3.45294525105681104660e-08
	.quad	0x03e5a891d1772f538		# 2.47132034530447431509e-08
	.quad	0x03e634f10be1fb591		# 3.59655343422487209774e-08
	.quad	0x03e6d9ce1d316eb93		# 5.51581770357780862071e-08
	.quad	0x03e63562a19a9c442		# 3.60171867511861372793e-08
	.quad	0x03e54e2adf548084c		# 1.94511067964296180547e-08
	.quad	0x03e508ce55cc8c97a		# 1.54137376631349347838e-08
	.quad	0x03e30e2f613e85bda		# 3.93171034490174464173e-09
	.quad	0x03e6db03ebb0227bf		# 5.52990607758839766440e-08
	.quad	0x03e61b75bb09cb098		# 3.29990737637586136511e-08
	.quad	0x03e496f16abb9df22		# 1.18436010922446096216e-08
	.quad	0x03e65b3f399411c62		# 4.04248680368301346709e-08
	.quad	0x03e586b3e59f65355		# 2.27418915900284316293e-08
	.quad	0x03e52482ceae1ac12		# 1.70263791333409206020e-08
	.quad	0x03e6efa39ef35793c		# 5.76999904754328540596e-08
	.quad 0					# for alignment


