

; Mandelbrot set plotter, by Derek Blair, May 1st 2009. SSE4 compliant or above x86 processor required.
; GCC linking function -  void mandelbrot(char* screen,float* gridx, float* gridy) __attribute__ ((regparm (3)))
; screen - byte array representing 256 x 256 8 bit bitmap data for fractal image
; gridx - 256 length array of floats representing progressing of horizontal floating points x-coordinates
; gridy - 256 length array of floats representing progressing of vertical floating points y-coordinates

%define screen [ebx+edx]
%define gridx [ebp + 4*esi]
%define gridy [edi + 4*esi]
%define shufflereg xmm4
%define crealreg xmm5
%define cimagreg xmm6
%define fourreg xmm7

segment .data

shuffledat db 0,4,8,12,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80

fourdat dd 4.0,4.0,4.0,4.0

segment .text

global _mandelbrot

_mandelbrot:

  push ebx
  push ebp
  push esi
  push edi
  
  sub esp, 8

  mov ebx,eax					
  mov ebp,edx					
  mov edi,ecx
					
  movups fourreg, [fourdat]							; load up the registers
  movups shufflereg, [shuffledat]					

  xor edx,edx										; zero traverser,dl = column,dh = row

  screenloop:

  	movzx esi,dl			
	movups crealreg, gridx							;done fetching a

  	movzx esi,dh			
	movups cimagreg, gridy							;done fetching b

	pxor xmm0,xmm0
	pxor xmm1,xmm1

	mov ecx,0xFF
  	mov eax, 0xFFFFFFFF				
  	orbitloop:										; xmm0: a , xmm1: b
  		movaps xmm2,xmm1							; xmm2 = b
  		mulps xmm1,xmm0								; b = ab
  		addps xmm1,xmm1								; b = 2ab
  		addps xmm1,cimagreg							; b = 2ab+ Cr
  		mulps xmm2,xmm2								; xmm2 = b^2
  		mulps xmm0,xmm0								; a = a^2
  		movaps xmm3,xmm0							; xmm3 = a^2
  		subps xmm0,xmm2								; a = a^2 - b^2
  		addps xmm0, crealreg						; a = a^2 - b^2 +Ci
  		addps xmm2,xmm3								; xmm2 = a^2 +b^2
  		cmpps xmm2,fourreg,1						; xmm2 = xmm2 < 4
  		pshufb xmm2,shufflereg
  		movlpd [esp],xmm2                           ; SSE4 use, pextrd esi,xmm2,0
  		mov esi, dword [esp]
  		and esi, 0x1010101							; apply bit mask
  		sub eax, esi
  		and esi,esi									; set zero flag if esi is zero
  	loopnz orbitloop
  	mov screen, eax
  	add dx,4
  jnz short screenloop
  add esp,8
  pop edi
  pop esi
  pop ebp
  pop ebx
  ret



 
