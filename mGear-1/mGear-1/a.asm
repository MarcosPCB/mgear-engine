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

_mXOR32 PROC
	push ebp
	mov ebp, esp
	mov eax, [ebp+8]
	xor eax, [ebp+12]
	mov esp, ebp
	pop ebp
	ret
_mXOR32 ENDP

END