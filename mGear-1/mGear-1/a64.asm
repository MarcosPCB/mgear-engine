.MODEL FLAT

.CODE

_mSqrt64 PROC
movd rax, xmm0
	fld dword ptr [rax]
	fsqrt
	ret

_mSqrt64 ENDP
END

