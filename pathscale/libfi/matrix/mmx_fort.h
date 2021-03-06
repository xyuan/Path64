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


   !subroutine MMMM (r,a,b,blasroutine)
   !dir$ name(mmult_error="_F90_MMULT_ERROR")
   TYPE1, dimension (:,:) :: a
   TYPE2, dimension (:,:) :: b
   TYPE3, dimension (:,:), pointer :: r
   TYPE3, parameter :: ALPHA = 1, beta = 0
   ! USMID @(#) libfi/matrix/mmx_fort.h	92.1	10/28/98 22:50:56

   interface 
      subroutine blasroutine(transa,transb,m,n,k,alpha,a,lda,b,ldb,beta,c,ldc)
         character*1        transa, transb
         integer            m, n, k, lda, ldb, ldc
         TYPE3              alpha, beta
         TYPE3              a( lda,*), b(ldb,*), c(ldc,*)
         INTENT(IN) :: a,b
         INTENT(OUT) :: c
      end subroutine blasroutine
   end interface
   optional blasroutine
   
#if CONVERTT1
   TYPE3, DIMENSION (:,:), ALLOCATABLE :: AAAA
#else
#define AAAA a
#endif
   
#if CONVERTT2
   TYPE3, DIMENSION (:,:), ALLOCATABLE :: BBBB
#else
#define BBBB b
#endif
   
   INTEGER M,N,K,N1
   
   M = size(A,1)
   N = SIZE(A,2)
   K = size (B,2)
   N1 = size(B,1)
   
   if (n1.ne.n) call mmult_error
   

   allocate(R(M,K))
   if ((m.eq.0) .or. (n.eq.0) .or. (k.eq.0)) then
       r = 0
       return
   end if
   
#if !defined(AAAA)
   ALLOCATE(AAAA(m,n))
   AAAA = a
#endif
   
#if !defined(BBBB)
   ALLOCATE(BBBB(N,K))
   BBBB = b
#endif
   if (present(blasroutine)) then
       call blasroutine('n','n',m,k,n,alpha,AAAA,m,BBBB,n,beta,r,m)
   else
       call internal_mm(r)
   endif
   return

contains
   subroutine internal_mm(rr)
      TYPE3 :: rr(:,:)
      INTEGER I,J,KK
      
      rr = 0.
      UUNROLL
      do j = 1,K
         UUNROLL
         do i = 1,M
            do kk = 1,N
               rr(i,j) = rr(i,j) + AAAA(i,kk)*BBBB(kk,j)
            enddo
         enddo
      enddo
   end subroutine internal_mm
end subroutine MMMM

#undef AAAA
#undef BBBB

subroutine MMMV(r,a,b)
   !dir$ name(mmult_error="_F90_MMULT_ERROR")
   TYPE1, dimension (:,:) :: a
   TYPE2, dimension (:) :: b
   TYPE3, dimension (:), pointer :: r
   
#define AAAA a
   
#if CONVERTT2
   TYPE3, DIMENSION (:), ALLOCATABLE :: BBBB
#else
#define BBBB b
#endif
   
   INTEGER M,N,N1
   
   M = size(A,1)
   N = SIZE(A,2)
   N1 = size (B,1)
   
   if (n1.ne.n) call mmult_error
   
   allocate(R(M))
   
#if !defined(BBBB)
   ALLOCATE(BBBB(N))
   BBBB = b
#endif
   
   call internal_mm(r)
   
contains
   subroutine internal_mm(rr)
      TYPE3 :: rr(:)
      INTEGER I,KK
      
      rr = 0.
      do i = 1,M
         do kk = 1,N
            rr(i) = rr(i) + AAAA(i,kk)*BBBB(kk)
         enddo
      enddo
   end subroutine internal_mm
end subroutine MMMV

#undef AAAA
#undef BBBB

subroutine MMVM(r,a,b)
   !dir$ name(mmult_error="_F90_MMULT_ERROR")
   TYPE1, dimension (:) :: a
   TYPE2, dimension (:,:) :: b
   TYPE3, dimension (:), pointer :: r
   
#if CONVERTT1
   TYPE3, DIMENSION (:), ALLOCATABLE :: AAAA
#else
#define AAAA a
#endif
   
#define BBBB b
   
   INTEGER M,N,N1
   
   N = size (A,1)
   N1 = size(B,1)
   M = SIZE(B,2)
   
   if (n1.ne.n) call mmult_error
   
   allocate(R(M))
   
#if !defined(AAAA)
   ALLOCATE(AAAA(n))
   AAAA = a
#endif
   
   call internal_mm(r)
   
contains
   subroutine internal_mm(rr)
      TYPE3 :: rr(:)
      INTEGER I,KK
      
      rr = 0.
      do i = 1,M
         do kk = 1,N
            rr(i) = rr(i) + AAAA(kk)*BBBB(kk,i)
         enddo
      enddo
   end subroutine internal_mm
end subroutine MMVM
