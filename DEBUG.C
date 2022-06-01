
#include "aes.h"

#if DEBUG

int	TRACEFLAG;

#asm
;
; Debugging code
;
		.psect	_TEXT,class=CODE
		.globl	_TRACE,_ophex,_rlr,_TRKEY,_TRCHAR,_border


_TRKEY:		mov		ah,#1
		;;int		#0x21
		;;cmp		al,#'!'
		;;jnz		trkr
		;;int		#3
trkr:	ret

_TRACE:	push	dx		;String parameter

	mov	ax,#0x3D02	;OPEN r/w
	mov	cx,#0
	mov	dx,#gemapp
	int	#0x21		;AX = file handle
	jc	fail
	mov	bx,ax		;BX = file handle
	mov	ax,#0x4202
	mov	cx,#0
	mov	dx,#0
	int	#0x21		;LSEEK to end
	pop	dx		;Address to write
	push	dx
	mov	cx,#0
trlen:	xchg	bx,dx
	mov	al,[bx]
	xchg	bx,dx
	cmp	al,#0x00	;Accept C strings
	jz	trend
	cmp	al,#0x24	;Accept function-9 style strings
	jz	trend
	inc	cx
	inc	dx
	jmp	trlen
;
trend:	pop	dx		;CX = length
	push	dx		;DX = address
	mov	ah,#0x40
	int	#0x21
	mov	ah,#0x3E
	int	#0x21

;;;		mov		dx,_rlr
;;;		call	_ophex
;;;		mov		ah,#2
;;;		mov		dl,#':'
;;;		int		#0x21
fail:		pop		dx
;;;		mov		ah,#9
;;;		int		#0x21
		ret
;
_TRCHAR:
		mov		al,dl
		mov		onech,al
		mov		dx,#onech
		call		_TRACE
		ret

;;;		mov		ah,#2
;;;		int		#0x21
;;;		ret
;
_ophex:	mov		al,dh
		call	hexa
		mov		al,dl
hexa:	push	ax
		mov		cl,#4
		rcr		al,cl
		call	hexn
		pop		ax
hexn:	and		al,#0x0F
		cmp		al,#10
		jc		hexn1
		add		al,#7
hexn1:	add		al,#48
		push	dx
		mov		dl,al
		call		_TRCHAR
;;;		mov		ah,#2
;;;		int		#0x21
		pop		dx
		ret
;
_border:			;Set border & background colour
	push	ax
	push	bx
	push	di
	mov		bx,dx
	mov		ah,#0x0B
	int		#0x10
	pop		di
	pop		bx
	pop		ax
	ret
;
	.globl	_int3

_int3:	int	#0x03
	ret

	.psect data,class=DATA
onech:	.byte	0
	.byte	0x24

gemapp:
	.byte	'E:/GEMAPP.LOG',0

#endasm

void vdi_debug(LPWORD FAR *vdipb)
{
	TRACE("VDI call ");
	ophex(vdipb[0][0]);
	TRACE(" intin[0]=");
	ophex(vdipb[1][0]);
	TRACE(" ptsin[0]=");
	ophex(vdipb[2][0]);
	TRACE("\r\n");	
}

#endif /* DEBUG */
