.586P

.MODEL FLAT

.STACK
.DATA

.CODE

_mSqrt32 PROC
	push ebp
	mov ebp, esp
	fld dword ptr [ebp+8]
	fsqrt
	mov esp, ebp
	pop ebp
	ret
_mSqrt32 ENDP

END