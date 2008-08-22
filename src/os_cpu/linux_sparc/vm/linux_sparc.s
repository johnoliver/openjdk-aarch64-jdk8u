#
# Copyright 2005-2008 Sun Microsystems, Inc.  All Rights Reserved.
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# This code is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 only, as
# published by the Free Software Foundation.
#
# This code is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# version 2 for more details (a copy is included in the LICENSE file that
# accompanied this code).
#
# You should have received a copy of the GNU General Public License version
# 2 along with this work; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
# CA 95054 USA or visit www.sun.com if you need additional information or
# have any questions.
#

    # Prototype: int SafeFetch32 (int * adr, int ErrValue)
    # The "ld" at Fetch32 is potentially faulting instruction.
    # If the instruction traps the trap handler will arrange
    # for control to resume at Fetch32Resume.  
    # By convention with the trap handler we ensure there is a non-CTI
    # instruction in the trap shadow.  
        

    .globl  SafeFetch32, Fetch32PFI, Fetch32Resume
    .globl  SafeFetchN
    .align  32
    .type    SafeFetch32,@function
SafeFetch32:        
    mov     %o0, %g1
    mov     %o1, %o0
Fetch32PFI:
    # <-- Potentially faulting instruction
    ld      [%g1], %o0         
Fetch32Resume:
    nop
    retl
    nop

    .globl  SafeFetchN, FetchNPFI, FetchNResume
    .type    SafeFetchN,@function
    .align  32
SafeFetchN:
    mov     %o0, %g1
    mov     %o1, %o0
FetchNPFI:
    ldn     [%g1], %o0
FetchNResume:
    nop
    retl
    nop

    # Possibilities:
    # -- membar
    # -- CAS (SP + BIAS, G0, G0)
    # -- wr %g0, %asi

    .globl SpinPause
    .type   SpinPause,@function
    .align  32
SpinPause:      
    retl
    mov %g0, %o0

   .globl _Copy_conjoint_jlongs_atomic
   .type   _Copy_conjoint_jlongs_atomic,@function
_Copy_conjoint_jlongs_atomic:
        cmp     %o0, %o1
	bleu    4f
	sll     %o2, 3, %o4
        ba      2f
   1:
	subcc   %o4, 8, %o4
	std     %o2, [%o1]
	add     %o0, 8, %o0
	add     %o1, 8, %o1
   2:
	bge,a   1b
	ldd     [%o0], %o2
	ba      5f
        nop
   3:
	std     %o2, [%o1+%o4]
   4:
	subcc   %o4, 8, %o4
	bge,a   3b
	ldd     [%o0+%o4], %o2
   5:      
	retl
	nop


    .globl _flush_reg_windows
    .align 32
 _flush_reg_windows:
        ta 0x03
        retl
        mov     %fp, %o0
