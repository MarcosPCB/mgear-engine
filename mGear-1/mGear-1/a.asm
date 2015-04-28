.586P

.MODEL FLAT
.DATA

t1 SDWORD ?
t2 SDWORD ?
valx db 0
valy db 0

x SDWORD ?
y SDWORD ?
sizex SDWORD ?
sizey SDWORD ?
dimx SDWORD ?
dimy SDWORD ?
cosv SDWORD ?
sinv SDWORD ?

.STACK

.CODE

PUBLIC _CheckBounds
_CheckBounds proc near

	push ebp
	mov ebp, esp
	sub esp, 32
	mov eax, [ebp+8]
	mov x, eax
	mov eax, [ebp+12]
	mov y, eax
	mov eax, [ebp+16]
	mov sizex, eax
	mov eax, [ebp+20]
	mov sizey, eax
	mov eax, [ebp+24]
	mov dimx, eax
	mov eax, [ebp+28]
	mov dimy, eax
	mov eax, [ebp+32]
	mov cosv, eax
	mov eax, [ebp+36]
	mov sinv, eax


	mov edx, 0
	mov eax, sizex
	mov ecx, 2
	idiv ecx
	mov ebx, x
	mov t1, ebx
	sub t1, eax
	mov ebx, x
	sub t1, ebx

	mov edx, 0
	mov eax, sizey
	mov ecx, 2
	idiv ecx
	mov ebx, y
	mov t2, ebx
	sub t2, eax
	mov ebx, y
	sub t2, ebx

	mov eax, t1
	mov ebx,cosv
	imul ebx
	mov ecx, 100
	idiv ecx
	mov t1, eax

	mov eax, t2
	mov ebx,sinv
	imul ebx
	mov ecx, 100
	idiv ecx
	mov t2, eax

	mov eax, t1
	mov ebx, t2
	mov ecx, x
	add eax, ebx
	add ecx, eax

	mov eax, dimx
	cmp ecx, eax
	ja VALL1
	jna P2
VALL1:
	inc valx
P2:
		mov edx, 0
		mov eax, sizex
		mov ecx, 2
		idiv ecx
		mov ebx, x
		mov t1, ebx
		sub t1, eax
		mov ebx, x
		sub t1, ebx

		mov edx, 0
		mov eax, sizey
		mov ecx, 2
		idiv ecx
		mov ebx, y
		mov t2, ebx
		sub t2, eax
		mov ebx, y
		sub t2, ebx

		mov eax, t1
		mov ebx,sinv
		imul ebx
		mov ecx, 100
		idiv ecx
		mov t1, eax

		mov eax, t2
		mov ebx,cosv
		imul ebx
		mov ecx, 100
		idiv ecx
		mov t2, eax

		mov eax, t1
		mov ebx, t2
		mov ecx, y
		add eax, ebx
		add ecx, eax

		mov eax, dimy
		cmp ecx, eax
		ja VALL2
		jna P3
VALL2:
		inc valy
P3:
		mov edx, 0
		mov eax, sizex
		mov ecx, 2
		idiv ecx
		mov ebx, x
		mov t1, ebx
		add t1, eax
		mov ebx, x
		sub t1, ebx

		mov edx, 0
		mov eax, sizey
		mov ecx, 2
		idiv ecx
		mov ebx, y
		mov t2, ebx
		sub t2, eax
		mov ebx, y
		sub t2, ebx

		mov eax, t1
		mov ebx,cosv
		imul ebx
		mov ecx, 100
		idiv ecx
		mov t1, eax

		mov eax, t2
		mov ebx,sinv
		imul ebx
		mov ecx, 100
		idiv ecx
		mov t2, eax

		mov eax, t1
		mov ebx, t2
		mov ecx, x
		add eax, ebx
		add ecx, eax

		mov eax, dimx
		cmp ecx, eax
		ja VALL3
		jna P4
VALL3:
		inc valx
P4:
		mov edx, 0
		mov eax, sizex
		mov ecx, 2
		idiv ecx
		mov ebx, x
		mov t1, ebx
		add t1, eax
		mov ebx, x
		sub t1, ebx

		mov edx, 0
		mov eax, sizey
		mov ecx, 2
		idiv ecx
		mov ebx, y
		mov t2, ebx
		sub t2, eax
		mov ebx, y
		sub t2, ebx

		mov eax, t1
		mov ebx,sinv
		imul ebx
		mov ecx, 100
		idiv ecx
		mov t1, eax

		mov eax, t2
		mov ebx,cosv
		imul ebx
		mov ecx, 100
		idiv ecx
		mov t2, eax

		mov eax, t1
		mov ebx, t2
		mov ecx, y
		add eax, ebx
		add ecx, eax

		mov eax, dimy
		cmp ecx, eax
		ja VALL4
		jna P5
VALL4:
		inc valy
P5:
		mov edx, 0
		mov eax, sizex
		mov ecx, 2
		idiv ecx
		mov ebx, x
		mov t1, ebx
		add t1, eax
		mov ebx, x
		sub t1, ebx

		mov edx, 0
		mov eax, sizey
		mov ecx, 2
		idiv ecx
		mov ebx, y
		mov t2, ebx
		add t2, eax
		mov ebx, y
		sub t2, ebx

		mov eax, t1
		mov ebx,cosv
		imul ebx
		mov ecx, 100
		idiv ecx
		mov t1, eax

		mov eax, t2
		mov ebx,sinv
		imul ebx
		mov ecx, 100
		idiv ecx
		mov t2, eax

		mov eax, t1
		mov ebx, t2
		mov ecx, x
		add eax, ebx
		add ecx, eax

		mov eax, dimx
		cmp ecx, eax
		ja VALL5
		jna P6
VALL5:
		inc valx
P6:
		mov edx, 0
		mov eax, sizex
		mov ecx, 2
		idiv ecx
		mov ebx, x
		mov t1, ebx
		add t1, eax
		mov ebx, x
		sub t1, ebx

		mov edx, 0
		mov eax, sizey
		mov ecx, 2
		idiv ecx
		mov ebx, y
		mov t2, ebx
		add t2, eax
		mov ebx, y
		sub t2, ebx

		mov eax, t1
		mov ebx,sinv
		imul ebx
		mov ecx, 100
		idiv ecx
		mov t1, eax

		mov eax, t2
		mov ebx,cosv
		imul ebx
		mov ecx, 100
		idiv ecx
		mov t2, eax

		mov eax, t1
		mov ebx, t2
		mov ecx, y
		add eax, ebx
		add ecx, eax

		mov eax, dimy
		cmp ecx, eax
		ja VALL6
		jna P7
VALL6:
		inc valy
P7:
		mov edx, 0
		mov eax, sizex
		mov ecx, 2
		idiv ecx
		mov ebx, x
		mov t1, ebx
		sub t1, eax
		mov ebx, x
		sub t1, ebx

		mov edx, 0
		mov eax, sizey
		mov ecx, 2
		idiv ecx
		mov ebx, y
		mov t2, ebx
		add t2, eax
		mov ebx, y
		sub t2, ebx

		mov eax, t1
		mov ebx,cosv
		imul ebx
		mov ecx, 100
		idiv ecx
		mov t1, eax

		mov eax, t2
		mov ebx,sinv
		imul ebx
		mov ecx, 100
		idiv ecx
		mov t2, eax

		mov eax, t1
		mov ebx, t2
		mov ecx, x
		add eax, ebx
		add ecx, eax

		mov eax, dimx
		cmp ecx, eax
		ja VALL7
		jna P8

VALL7:
		inc valx
P8:
		mov edx, 0
		mov eax, sizex
		mov ecx, 2
		idiv ecx
		mov ebx, x
		mov t1, ebx
		sub t1, eax
		mov ebx, x
		sub t1, ebx

		mov edx, 0
		mov eax, sizey
		mov ecx, 2
		idiv ecx
		mov ebx, y
		mov t2, ebx
		add t2, eax
		mov ebx, y
		sub t2, ebx

		mov eax, t1
		mov ebx,sinv
		imul ebx
		mov ecx, 100
		idiv ecx
		mov t1, eax

		mov eax, t2
		mov ebx,cosv
		imul ebx
		mov ecx, 100
		idiv ecx
		mov t2, eax

		mov eax, t1
		mov ebx, t2
		mov ecx, y
		add eax, ebx
		add ecx, eax

		mov eax, dimy
		cmp ecx, eax
		ja VALL8
		jna CHECKVALX
VALL8:
		inc valy
CHECKVALX:
		mov al, 4
		cmp valx, al
		je NODRAW
		jne CHECKVALY
NODRAW:
		mov esp, ebp
		pop ebp
		mov eax, 1
		ret
CHECKVALY:
		mov al, 4
		cmp valy, al
		je NODRAW
		jne DRAW
DRAW:
		mov esp, ebp
		pop ebp
		mov eax, 0
		ret


_CheckBounds ENDP

END