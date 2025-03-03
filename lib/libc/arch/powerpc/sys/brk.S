/* $OpenBSD: brk.S,v 1.16 2021/10/25 14:38:10 jca Exp $ */

/*
 * Copyright (c) 1996 Dale Rahn
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "SYS.h"

	.extern __curbrk
	.extern _C_LABEL(_end)

ENTRY(brk)
	
	/* check >= _end, if not make the call for _end */
#ifndef __PIC__
	addis 5,0,_C_LABEL(_end)@h
	ori  5,5,_C_LABEL(_end)@l		/* # 5 = &_end */
#else
	mflr	10
	bcl	20, 31, 1f
1:	mflr	9
	addis	9, 9, _GLOBAL_OFFSET_TABLE_-1b@ha
	addi	9, 9, _GLOBAL_OFFSET_TABLE_-1b@l
	mtlr	10
	lwz	5,_C_LABEL(_end)@got(9)
#endif
	cmplw	3,5
	bge+	.L_brk_call
	mr	3, 5

.L_brk_call:
	mr	7, 3
	/* call break(size) */
#ifndef __PIC__
	addis	6, 0, __curbrk@H
	ori	6, 6, __curbrk@L	/* # 6 = &__curbrk */
#else
	lwz	6,__curbrk@got(9)
#endif

	li	0, SYS_break
	sc

	/* check for error */
	cmpwi	0, 0
	beq+	.L_brk_ok	/* OK so this is stupid but I haven't read b */
	stw	0, R2_OFFSET_ERRNO(2)
	li	3, -1
	blr

	/* update, __curbrk and return */
.L_brk_ok:
	stw	7, 0(6)		/* # remember, 6=&__curbrk, 3= new value */
	mr	3, 0		/* # return 0 */
	blr
END(brk)
	.weak	brk
