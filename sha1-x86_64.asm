OPTION	DOTNAME
.text$	SEGMENT ALIGN(64) 'CODE'

PUBLIC	sha1_block_data_order

ALIGN	16
sha1_block_data_order	PROC PUBLIC
	mov	QWORD PTR[8+rsp],rdi	;WIN64 prologue
	mov	QWORD PTR[16+rsp],rsi
	mov	rax,rsp
$L$SEH_begin_sha1_block_data_order::
	mov	rdi,rcx
	mov	rsi,rdx
	mov	rdx,r8


	push	rbx
	push	rbp
	push	r12
	push	r13
	mov	r11,rsp
	mov	r8,rdi
	sub	rsp,72
	mov	r9,rsi
	and	rsp,-64
	mov	r10,rdx
	mov	QWORD PTR[64+rsp],r11
$L$prologue::

	mov	esi,DWORD PTR[r8]
	mov	edi,DWORD PTR[4+r8]
	mov	r11d,DWORD PTR[8+r8]
	mov	r12d,DWORD PTR[12+r8]
	mov	r13d,DWORD PTR[16+r8]

ALIGN	4
$L$loop::
	mov	edx,DWORD PTR[r9]
	bswap	edx
	mov	DWORD PTR[rsp],edx
	mov	eax,r11d
	mov	ebp,DWORD PTR[4+r9]
	mov	ecx,esi
	xor	eax,r12d
	bswap	ebp
	rol	ecx,5
	lea	r13d,DWORD PTR[05a827999h+r13*1+rdx]
	and	eax,edi
	mov	DWORD PTR[4+rsp],ebp
	add	r13d,ecx
	xor	eax,r12d
	rol	edi,30
	add	r13d,eax
	mov	eax,edi
	mov	edx,DWORD PTR[8+r9]
	mov	ecx,r13d
	xor	eax,r11d
	bswap	edx
	rol	ecx,5
	lea	r12d,DWORD PTR[05a827999h+r12*1+rbp]
	and	eax,esi
	mov	DWORD PTR[8+rsp],edx
	add	r12d,ecx
	xor	eax,r11d
	rol	esi,30
	add	r12d,eax
	mov	eax,esi
	mov	ebp,DWORD PTR[12+r9]
	mov	ecx,r12d
	xor	eax,edi
	bswap	ebp
	rol	ecx,5
	lea	r11d,DWORD PTR[05a827999h+r11*1+rdx]
	and	eax,r13d
	mov	DWORD PTR[12+rsp],ebp
	add	r11d,ecx
	xor	eax,edi
	rol	r13d,30
	add	r11d,eax
	mov	eax,r13d
	mov	edx,DWORD PTR[16+r9]
	mov	ecx,r11d
	xor	eax,esi
	bswap	edx
	rol	ecx,5
	lea	edi,DWORD PTR[05a827999h+rdi*1+rbp]
	and	eax,r12d
	mov	DWORD PTR[16+rsp],edx
	add	edi,ecx
	xor	eax,esi
	rol	r12d,30
	add	edi,eax
	mov	eax,r12d
	mov	ebp,DWORD PTR[20+r9]
	mov	ecx,edi
	xor	eax,r13d
	bswap	ebp
	rol	ecx,5
	lea	esi,DWORD PTR[05a827999h+rsi*1+rdx]
	and	eax,r11d
	mov	DWORD PTR[20+rsp],ebp
	add	esi,ecx
	xor	eax,r13d
	rol	r11d,30
	add	esi,eax
	mov	eax,r11d
	mov	edx,DWORD PTR[24+r9]
	mov	ecx,esi
	xor	eax,r12d
	bswap	edx
	rol	ecx,5
	lea	r13d,DWORD PTR[05a827999h+r13*1+rbp]
	and	eax,edi
	mov	DWORD PTR[24+rsp],edx
	add	r13d,ecx
	xor	eax,r12d
	rol	edi,30
	add	r13d,eax
	mov	eax,edi
	mov	ebp,DWORD PTR[28+r9]
	mov	ecx,r13d
	xor	eax,r11d
	bswap	ebp
	rol	ecx,5
	lea	r12d,DWORD PTR[05a827999h+r12*1+rdx]
	and	eax,esi
	mov	DWORD PTR[28+rsp],ebp
	add	r12d,ecx
	xor	eax,r11d
	rol	esi,30
	add	r12d,eax
	mov	eax,esi
	mov	edx,DWORD PTR[32+r9]
	mov	ecx,r12d
	xor	eax,edi
	bswap	edx
	rol	ecx,5
	lea	r11d,DWORD PTR[05a827999h+r11*1+rbp]
	and	eax,r13d
	mov	DWORD PTR[32+rsp],edx
	add	r11d,ecx
	xor	eax,edi
	rol	r13d,30
	add	r11d,eax
	mov	eax,r13d
	mov	ebp,DWORD PTR[36+r9]
	mov	ecx,r11d
	xor	eax,esi
	bswap	ebp
	rol	ecx,5
	lea	edi,DWORD PTR[05a827999h+rdi*1+rdx]
	and	eax,r12d
	mov	DWORD PTR[36+rsp],ebp
	add	edi,ecx
	xor	eax,esi
	rol	r12d,30
	add	edi,eax
	mov	eax,r12d
	mov	edx,DWORD PTR[40+r9]
	mov	ecx,edi
	xor	eax,r13d
	bswap	edx
	rol	ecx,5
	lea	esi,DWORD PTR[05a827999h+rsi*1+rbp]
	and	eax,r11d
	mov	DWORD PTR[40+rsp],edx
	add	esi,ecx
	xor	eax,r13d
	rol	r11d,30
	add	esi,eax
	mov	eax,r11d
	mov	ebp,DWORD PTR[44+r9]
	mov	ecx,esi
	xor	eax,r12d
	bswap	ebp
	rol	ecx,5
	lea	r13d,DWORD PTR[05a827999h+r13*1+rdx]
	and	eax,edi
	mov	DWORD PTR[44+rsp],ebp
	add	r13d,ecx
	xor	eax,r12d
	rol	edi,30
	add	r13d,eax
	mov	eax,edi
	mov	edx,DWORD PTR[48+r9]
	mov	ecx,r13d
	xor	eax,r11d
	bswap	edx
	rol	ecx,5
	lea	r12d,DWORD PTR[05a827999h+r12*1+rbp]
	and	eax,esi
	mov	DWORD PTR[48+rsp],edx
	add	r12d,ecx
	xor	eax,r11d
	rol	esi,30
	add	r12d,eax
	mov	eax,esi
	mov	ebp,DWORD PTR[52+r9]
	mov	ecx,r12d
	xor	eax,edi
	bswap	ebp
	rol	ecx,5
	lea	r11d,DWORD PTR[05a827999h+r11*1+rdx]
	and	eax,r13d
	mov	DWORD PTR[52+rsp],ebp
	add	r11d,ecx
	xor	eax,edi
	rol	r13d,30
	add	r11d,eax
	mov	eax,r13d
	mov	edx,DWORD PTR[56+r9]
	mov	ecx,r11d
	xor	eax,esi
	bswap	edx
	rol	ecx,5
	lea	edi,DWORD PTR[05a827999h+rdi*1+rbp]
	and	eax,r12d
	mov	DWORD PTR[56+rsp],edx
	add	edi,ecx
	xor	eax,esi
	rol	r12d,30
	add	edi,eax
	mov	eax,r12d
	mov	ebp,DWORD PTR[60+r9]
	mov	ecx,edi
	xor	eax,r13d
	bswap	ebp
	rol	ecx,5
	lea	esi,DWORD PTR[05a827999h+rsi*1+rdx]
	and	eax,r11d
	mov	DWORD PTR[60+rsp],ebp
	add	esi,ecx
	xor	eax,r13d
	rol	r11d,30
	add	esi,eax
	mov	edx,DWORD PTR[rsp]
	mov	eax,r11d
	mov	ecx,esi
	xor	edx,DWORD PTR[8+rsp]
	xor	eax,r12d
	rol	ecx,5
	xor	edx,DWORD PTR[32+rsp]
	and	eax,edi
	lea	r13d,DWORD PTR[05a827999h+r13*1+rbp]
	xor	edx,DWORD PTR[52+rsp]
	xor	eax,r12d
	rol	edx,1
	add	r13d,ecx
	rol	edi,30
	mov	DWORD PTR[rsp],edx
	add	r13d,eax
	mov	ebp,DWORD PTR[4+rsp]
	mov	eax,edi
	mov	ecx,r13d
	xor	ebp,DWORD PTR[12+rsp]
	xor	eax,r11d
	rol	ecx,5
	xor	ebp,DWORD PTR[36+rsp]
	and	eax,esi
	lea	r12d,DWORD PTR[05a827999h+r12*1+rdx]
	xor	ebp,DWORD PTR[56+rsp]
	xor	eax,r11d
	rol	ebp,1
	add	r12d,ecx
	rol	esi,30
	mov	DWORD PTR[4+rsp],ebp
	add	r12d,eax
	mov	edx,DWORD PTR[8+rsp]
	mov	eax,esi
	mov	ecx,r12d
	xor	edx,DWORD PTR[16+rsp]
	xor	eax,edi
	rol	ecx,5
	xor	edx,DWORD PTR[40+rsp]
	and	eax,r13d
	lea	r11d,DWORD PTR[05a827999h+r11*1+rbp]
	xor	edx,DWORD PTR[60+rsp]
	xor	eax,edi
	rol	edx,1
	add	r11d,ecx
	rol	r13d,30
	mov	DWORD PTR[8+rsp],edx
	add	r11d,eax
	mov	ebp,DWORD PTR[12+rsp]
	mov	eax,r13d
	mov	ecx,r11d
	xor	ebp,DWORD PTR[20+rsp]
	xor	eax,esi
	rol	ecx,5
	xor	ebp,DWORD PTR[44+rsp]
	and	eax,r12d
	lea	edi,DWORD PTR[05a827999h+rdi*1+rdx]
	xor	ebp,DWORD PTR[rsp]
	xor	eax,esi
	rol	ebp,1
	add	edi,ecx
	rol	r12d,30
	mov	DWORD PTR[12+rsp],ebp
	add	edi,eax
	mov	edx,DWORD PTR[16+rsp]
	mov	eax,r12d
	mov	ecx,edi
	xor	edx,DWORD PTR[24+rsp]
	xor	eax,r13d
	rol	ecx,5
	xor	edx,DWORD PTR[48+rsp]
	and	eax,r11d
	lea	esi,DWORD PTR[05a827999h+rsi*1+rbp]
	xor	edx,DWORD PTR[4+rsp]
	xor	eax,r13d
	rol	edx,1
	add	esi,ecx
	rol	r11d,30
	mov	DWORD PTR[16+rsp],edx
	add	esi,eax
	mov	ebp,DWORD PTR[20+rsp]
	mov	eax,r11d
	mov	ecx,esi
	xor	ebp,DWORD PTR[28+rsp]
	xor	eax,edi
	rol	ecx,5
	lea	r13d,DWORD PTR[1859775393+r13*1+rdx]
	xor	ebp,DWORD PTR[52+rsp]
	xor	eax,r12d
	add	r13d,ecx
	xor	ebp,DWORD PTR[8+rsp]
	rol	edi,30
	add	r13d,eax
	rol	ebp,1
	mov	DWORD PTR[20+rsp],ebp
	mov	edx,DWORD PTR[24+rsp]
	mov	eax,edi
	mov	ecx,r13d
	xor	edx,DWORD PTR[32+rsp]
	xor	eax,esi
	rol	ecx,5
	lea	r12d,DWORD PTR[1859775393+r12*1+rbp]
	xor	edx,DWORD PTR[56+rsp]
	xor	eax,r11d
	add	r12d,ecx
	xor	edx,DWORD PTR[12+rsp]
	rol	esi,30
	add	r12d,eax
	rol	edx,1
	mov	DWORD PTR[24+rsp],edx
	mov	ebp,DWORD PTR[28+rsp]
	mov	eax,esi
	mov	ecx,r12d
	xor	ebp,DWORD PTR[36+rsp]
	xor	eax,r13d
	rol	ecx,5
	lea	r11d,DWORD PTR[1859775393+r11*1+rdx]
	xor	ebp,DWORD PTR[60+rsp]
	xor	eax,edi
	add	r11d,ecx
	xor	ebp,DWORD PTR[16+rsp]
	rol	r13d,30
	add	r11d,eax
	rol	ebp,1
	mov	DWORD PTR[28+rsp],ebp
	mov	edx,DWORD PTR[32+rsp]
	mov	eax,r13d
	mov	ecx,r11d
	xor	edx,DWORD PTR[40+rsp]
	xor	eax,r12d
	rol	ecx,5
	lea	edi,DWORD PTR[1859775393+rdi*1+rbp]
	xor	edx,DWORD PTR[rsp]
	xor	eax,esi
	add	edi,ecx
	xor	edx,DWORD PTR[20+rsp]
	rol	r12d,30
	add	edi,eax
	rol	edx,1
	mov	DWORD PTR[32+rsp],edx
	mov	ebp,DWORD PTR[36+rsp]
	mov	eax,r12d
	mov	ecx,edi
	xor	ebp,DWORD PTR[44+rsp]
	xor	eax,r11d
	rol	ecx,5
	lea	esi,DWORD PTR[1859775393+rsi*1+rdx]
	xor	ebp,DWORD PTR[4+rsp]
	xor	eax,r13d
	add	esi,ecx
	xor	ebp,DWORD PTR[24+rsp]
	rol	r11d,30
	add	esi,eax
	rol	ebp,1
	mov	DWORD PTR[36+rsp],ebp
	mov	edx,DWORD PTR[40+rsp]
	mov	eax,r11d
	mov	ecx,esi
	xor	edx,DWORD PTR[48+rsp]
	xor	eax,edi
	rol	ecx,5
	lea	r13d,DWORD PTR[1859775393+r13*1+rbp]
	xor	edx,DWORD PTR[8+rsp]
	xor	eax,r12d
	add	r13d,ecx
	xor	edx,DWORD PTR[28+rsp]
	rol	edi,30
	add	r13d,eax
	rol	edx,1
	mov	DWORD PTR[40+rsp],edx
	mov	ebp,DWORD PTR[44+rsp]
	mov	eax,edi
	mov	ecx,r13d
	xor	ebp,DWORD PTR[52+rsp]
	xor	eax,esi
	rol	ecx,5
	lea	r12d,DWORD PTR[1859775393+r12*1+rdx]
	xor	ebp,DWORD PTR[12+rsp]
	xor	eax,r11d
	add	r12d,ecx
	xor	ebp,DWORD PTR[32+rsp]
	rol	esi,30
	add	r12d,eax
	rol	ebp,1
	mov	DWORD PTR[44+rsp],ebp
	mov	edx,DWORD PTR[48+rsp]
	mov	eax,esi
	mov	ecx,r12d
	xor	edx,DWORD PTR[56+rsp]
	xor	eax,r13d
	rol	ecx,5
	lea	r11d,DWORD PTR[1859775393+r11*1+rbp]
	xor	edx,DWORD PTR[16+rsp]
	xor	eax,edi
	add	r11d,ecx
	xor	edx,DWORD PTR[36+rsp]
	rol	r13d,30
	add	r11d,eax
	rol	edx,1
	mov	DWORD PTR[48+rsp],edx
	mov	ebp,DWORD PTR[52+rsp]
	mov	eax,r13d
	mov	ecx,r11d
	xor	ebp,DWORD PTR[60+rsp]
	xor	eax,r12d
	rol	ecx,5
	lea	edi,DWORD PTR[1859775393+rdi*1+rdx]
	xor	ebp,DWORD PTR[20+rsp]
	xor	eax,esi
	add	edi,ecx
	xor	ebp,DWORD PTR[40+rsp]
	rol	r12d,30
	add	edi,eax
	rol	ebp,1
	mov	DWORD PTR[52+rsp],ebp
	mov	edx,DWORD PTR[56+rsp]
	mov	eax,r12d
	mov	ecx,edi
	xor	edx,DWORD PTR[rsp]
	xor	eax,r11d
	rol	ecx,5
	lea	esi,DWORD PTR[1859775393+rsi*1+rbp]
	xor	edx,DWORD PTR[24+rsp]
	xor	eax,r13d
	add	esi,ecx
	xor	edx,DWORD PTR[44+rsp]
	rol	r11d,30
	add	esi,eax
	rol	edx,1
	mov	DWORD PTR[56+rsp],edx
	mov	ebp,DWORD PTR[60+rsp]
	mov	eax,r11d
	mov	ecx,esi
	xor	ebp,DWORD PTR[4+rsp]
	xor	eax,edi
	rol	ecx,5
	lea	r13d,DWORD PTR[1859775393+r13*1+rdx]
	xor	ebp,DWORD PTR[28+rsp]
	xor	eax,r12d
	add	r13d,ecx
	xor	ebp,DWORD PTR[48+rsp]
	rol	edi,30
	add	r13d,eax
	rol	ebp,1
	mov	DWORD PTR[60+rsp],ebp
	mov	edx,DWORD PTR[rsp]
	mov	eax,edi
	mov	ecx,r13d
	xor	edx,DWORD PTR[8+rsp]
	xor	eax,esi
	rol	ecx,5
	lea	r12d,DWORD PTR[1859775393+r12*1+rbp]
	xor	edx,DWORD PTR[32+rsp]
	xor	eax,r11d
	add	r12d,ecx
	xor	edx,DWORD PTR[52+rsp]
	rol	esi,30
	add	r12d,eax
	rol	edx,1
	mov	DWORD PTR[rsp],edx
	mov	ebp,DWORD PTR[4+rsp]
	mov	eax,esi
	mov	ecx,r12d
	xor	ebp,DWORD PTR[12+rsp]
	xor	eax,r13d
	rol	ecx,5
	lea	r11d,DWORD PTR[1859775393+r11*1+rdx]
	xor	ebp,DWORD PTR[36+rsp]
	xor	eax,edi
	add	r11d,ecx
	xor	ebp,DWORD PTR[56+rsp]
	rol	r13d,30
	add	r11d,eax
	rol	ebp,1
	mov	DWORD PTR[4+rsp],ebp
	mov	edx,DWORD PTR[8+rsp]
	mov	eax,r13d
	mov	ecx,r11d
	xor	edx,DWORD PTR[16+rsp]
	xor	eax,r12d
	rol	ecx,5
	lea	edi,DWORD PTR[1859775393+rdi*1+rbp]
	xor	edx,DWORD PTR[40+rsp]
	xor	eax,esi
	add	edi,ecx
	xor	edx,DWORD PTR[60+rsp]
	rol	r12d,30
	add	edi,eax
	rol	edx,1
	mov	DWORD PTR[8+rsp],edx
	mov	ebp,DWORD PTR[12+rsp]
	mov	eax,r12d
	mov	ecx,edi
	xor	ebp,DWORD PTR[20+rsp]
	xor	eax,r11d
	rol	ecx,5
	lea	esi,DWORD PTR[1859775393+rsi*1+rdx]
	xor	ebp,DWORD PTR[44+rsp]
	xor	eax,r13d
	add	esi,ecx
	xor	ebp,DWORD PTR[rsp]
	rol	r11d,30
	add	esi,eax
	rol	ebp,1
	mov	DWORD PTR[12+rsp],ebp
	mov	edx,DWORD PTR[16+rsp]
	mov	eax,r11d
	mov	ecx,esi
	xor	edx,DWORD PTR[24+rsp]
	xor	eax,edi
	rol	ecx,5
	lea	r13d,DWORD PTR[1859775393+r13*1+rbp]
	xor	edx,DWORD PTR[48+rsp]
	xor	eax,r12d
	add	r13d,ecx
	xor	edx,DWORD PTR[4+rsp]
	rol	edi,30
	add	r13d,eax
	rol	edx,1
	mov	DWORD PTR[16+rsp],edx
	mov	ebp,DWORD PTR[20+rsp]
	mov	eax,edi
	mov	ecx,r13d
	xor	ebp,DWORD PTR[28+rsp]
	xor	eax,esi
	rol	ecx,5
	lea	r12d,DWORD PTR[1859775393+r12*1+rdx]
	xor	ebp,DWORD PTR[52+rsp]
	xor	eax,r11d
	add	r12d,ecx
	xor	ebp,DWORD PTR[8+rsp]
	rol	esi,30
	add	r12d,eax
	rol	ebp,1
	mov	DWORD PTR[20+rsp],ebp
	mov	edx,DWORD PTR[24+rsp]
	mov	eax,esi
	mov	ecx,r12d
	xor	edx,DWORD PTR[32+rsp]
	xor	eax,r13d
	rol	ecx,5
	lea	r11d,DWORD PTR[1859775393+r11*1+rbp]
	xor	edx,DWORD PTR[56+rsp]
	xor	eax,edi
	add	r11d,ecx
	xor	edx,DWORD PTR[12+rsp]
	rol	r13d,30
	add	r11d,eax
	rol	edx,1
	mov	DWORD PTR[24+rsp],edx
	mov	ebp,DWORD PTR[28+rsp]
	mov	eax,r13d
	mov	ecx,r11d
	xor	ebp,DWORD PTR[36+rsp]
	xor	eax,r12d
	rol	ecx,5
	lea	edi,DWORD PTR[1859775393+rdi*1+rdx]
	xor	ebp,DWORD PTR[60+rsp]
	xor	eax,esi
	add	edi,ecx
	xor	ebp,DWORD PTR[16+rsp]
	rol	r12d,30
	add	edi,eax
	rol	ebp,1
	mov	DWORD PTR[28+rsp],ebp
	mov	edx,DWORD PTR[32+rsp]
	mov	eax,r12d
	mov	ecx,edi
	xor	edx,DWORD PTR[40+rsp]
	xor	eax,r11d
	rol	ecx,5
	lea	esi,DWORD PTR[1859775393+rsi*1+rbp]
	xor	edx,DWORD PTR[rsp]
	xor	eax,r13d
	add	esi,ecx
	xor	edx,DWORD PTR[20+rsp]
	rol	r11d,30
	add	esi,eax
	rol	edx,1
	mov	DWORD PTR[32+rsp],edx
	mov	ebp,DWORD PTR[36+rsp]
	mov	eax,r11d
	mov	ebx,r11d
	xor	ebp,DWORD PTR[44+rsp]
	and	eax,r12d
	mov	ecx,esi
	xor	ebp,DWORD PTR[4+rsp]
	xor	ebx,r12d
	lea	r13d,DWORD PTR[08f1bbcdch+r13*1+rdx]
	rol	ecx,5
	xor	ebp,DWORD PTR[24+rsp]
	add	r13d,eax
	and	ebx,edi
	rol	ebp,1
	add	r13d,ebx
	rol	edi,30
	mov	DWORD PTR[36+rsp],ebp
	add	r13d,ecx
	mov	edx,DWORD PTR[40+rsp]
	mov	eax,edi
	mov	ebx,edi
	xor	edx,DWORD PTR[48+rsp]
	and	eax,r11d
	mov	ecx,r13d
	xor	edx,DWORD PTR[8+rsp]
	xor	ebx,r11d
	lea	r12d,DWORD PTR[08f1bbcdch+r12*1+rbp]
	rol	ecx,5
	xor	edx,DWORD PTR[28+rsp]
	add	r12d,eax
	and	ebx,esi
	rol	edx,1
	add	r12d,ebx
	rol	esi,30
	mov	DWORD PTR[40+rsp],edx
	add	r12d,ecx
	mov	ebp,DWORD PTR[44+rsp]
	mov	eax,esi
	mov	ebx,esi
	xor	ebp,DWORD PTR[52+rsp]
	and	eax,edi
	mov	ecx,r12d
	xor	ebp,DWORD PTR[12+rsp]
	xor	ebx,edi
	lea	r11d,DWORD PTR[08f1bbcdch+r11*1+rdx]
	rol	ecx,5
	xor	ebp,DWORD PTR[32+rsp]
	add	r11d,eax
	and	ebx,r13d
	rol	ebp,1
	add	r11d,ebx
	rol	r13d,30
	mov	DWORD PTR[44+rsp],ebp
	add	r11d,ecx
	mov	edx,DWORD PTR[48+rsp]
	mov	eax,r13d
	mov	ebx,r13d
	xor	edx,DWORD PTR[56+rsp]
	and	eax,esi
	mov	ecx,r11d
	xor	edx,DWORD PTR[16+rsp]
	xor	ebx,esi
	lea	edi,DWORD PTR[08f1bbcdch+rdi*1+rbp]
	rol	ecx,5
	xor	edx,DWORD PTR[36+rsp]
	add	edi,eax
	and	ebx,r12d
	rol	edx,1
	add	edi,ebx
	rol	r12d,30
	mov	DWORD PTR[48+rsp],edx
	add	edi,ecx
	mov	ebp,DWORD PTR[52+rsp]
	mov	eax,r12d
	mov	ebx,r12d
	xor	ebp,DWORD PTR[60+rsp]
	and	eax,r13d
	mov	ecx,edi
	xor	ebp,DWORD PTR[20+rsp]
	xor	ebx,r13d
	lea	esi,DWORD PTR[08f1bbcdch+rsi*1+rdx]
	rol	ecx,5
	xor	ebp,DWORD PTR[40+rsp]
	add	esi,eax
	and	ebx,r11d
	rol	ebp,1
	add	esi,ebx
	rol	r11d,30
	mov	DWORD PTR[52+rsp],ebp
	add	esi,ecx
	mov	edx,DWORD PTR[56+rsp]
	mov	eax,r11d
	mov	ebx,r11d
	xor	edx,DWORD PTR[rsp]
	and	eax,r12d
	mov	ecx,esi
	xor	edx,DWORD PTR[24+rsp]
	xor	ebx,r12d
	lea	r13d,DWORD PTR[08f1bbcdch+r13*1+rbp]
	rol	ecx,5
	xor	edx,DWORD PTR[44+rsp]
	add	r13d,eax
	and	ebx,edi
	rol	edx,1
	add	r13d,ebx
	rol	edi,30
	mov	DWORD PTR[56+rsp],edx
	add	r13d,ecx
	mov	ebp,DWORD PTR[60+rsp]
	mov	eax,edi
	mov	ebx,edi
	xor	ebp,DWORD PTR[4+rsp]
	and	eax,r11d
	mov	ecx,r13d
	xor	ebp,DWORD PTR[28+rsp]
	xor	ebx,r11d
	lea	r12d,DWORD PTR[08f1bbcdch+r12*1+rdx]
	rol	ecx,5
	xor	ebp,DWORD PTR[48+rsp]
	add	r12d,eax
	and	ebx,esi
	rol	ebp,1
	add	r12d,ebx
	rol	esi,30
	mov	DWORD PTR[60+rsp],ebp
	add	r12d,ecx
	mov	edx,DWORD PTR[rsp]
	mov	eax,esi
	mov	ebx,esi
	xor	edx,DWORD PTR[8+rsp]
	and	eax,edi
	mov	ecx,r12d
	xor	edx,DWORD PTR[32+rsp]
	xor	ebx,edi
	lea	r11d,DWORD PTR[08f1bbcdch+r11*1+rbp]
	rol	ecx,5
	xor	edx,DWORD PTR[52+rsp]
	add	r11d,eax
	and	ebx,r13d
	rol	edx,1
	add	r11d,ebx
	rol	r13d,30
	mov	DWORD PTR[rsp],edx
	add	r11d,ecx
	mov	ebp,DWORD PTR[4+rsp]
	mov	eax,r13d
	mov	ebx,r13d
	xor	ebp,DWORD PTR[12+rsp]
	and	eax,esi
	mov	ecx,r11d
	xor	ebp,DWORD PTR[36+rsp]
	xor	ebx,esi
	lea	edi,DWORD PTR[08f1bbcdch+rdi*1+rdx]
	rol	ecx,5
	xor	ebp,DWORD PTR[56+rsp]
	add	edi,eax
	and	ebx,r12d
	rol	ebp,1
	add	edi,ebx
	rol	r12d,30
	mov	DWORD PTR[4+rsp],ebp
	add	edi,ecx
	mov	edx,DWORD PTR[8+rsp]
	mov	eax,r12d
	mov	ebx,r12d
	xor	edx,DWORD PTR[16+rsp]
	and	eax,r13d
	mov	ecx,edi
	xor	edx,DWORD PTR[40+rsp]
	xor	ebx,r13d
	lea	esi,DWORD PTR[08f1bbcdch+rsi*1+rbp]
	rol	ecx,5
	xor	edx,DWORD PTR[60+rsp]
	add	esi,eax
	and	ebx,r11d
	rol	edx,1
	add	esi,ebx
	rol	r11d,30
	mov	DWORD PTR[8+rsp],edx
	add	esi,ecx
	mov	ebp,DWORD PTR[12+rsp]
	mov	eax,r11d
	mov	ebx,r11d
	xor	ebp,DWORD PTR[20+rsp]
	and	eax,r12d
	mov	ecx,esi
	xor	ebp,DWORD PTR[44+rsp]
	xor	ebx,r12d
	lea	r13d,DWORD PTR[08f1bbcdch+r13*1+rdx]
	rol	ecx,5
	xor	ebp,DWORD PTR[rsp]
	add	r13d,eax
	and	ebx,edi
	rol	ebp,1
	add	r13d,ebx
	rol	edi,30
	mov	DWORD PTR[12+rsp],ebp
	add	r13d,ecx
	mov	edx,DWORD PTR[16+rsp]
	mov	eax,edi
	mov	ebx,edi
	xor	edx,DWORD PTR[24+rsp]
	and	eax,r11d
	mov	ecx,r13d
	xor	edx,DWORD PTR[48+rsp]
	xor	ebx,r11d
	lea	r12d,DWORD PTR[08f1bbcdch+r12*1+rbp]
	rol	ecx,5
	xor	edx,DWORD PTR[4+rsp]
	add	r12d,eax
	and	ebx,esi
	rol	edx,1
	add	r12d,ebx
	rol	esi,30
	mov	DWORD PTR[16+rsp],edx
	add	r12d,ecx
	mov	ebp,DWORD PTR[20+rsp]
	mov	eax,esi
	mov	ebx,esi
	xor	ebp,DWORD PTR[28+rsp]
	and	eax,edi
	mov	ecx,r12d
	xor	ebp,DWORD PTR[52+rsp]
	xor	ebx,edi
	lea	r11d,DWORD PTR[08f1bbcdch+r11*1+rdx]
	rol	ecx,5
	xor	ebp,DWORD PTR[8+rsp]
	add	r11d,eax
	and	ebx,r13d
	rol	ebp,1
	add	r11d,ebx
	rol	r13d,30
	mov	DWORD PTR[20+rsp],ebp
	add	r11d,ecx
	mov	edx,DWORD PTR[24+rsp]
	mov	eax,r13d
	mov	ebx,r13d
	xor	edx,DWORD PTR[32+rsp]
	and	eax,esi
	mov	ecx,r11d
	xor	edx,DWORD PTR[56+rsp]
	xor	ebx,esi
	lea	edi,DWORD PTR[08f1bbcdch+rdi*1+rbp]
	rol	ecx,5
	xor	edx,DWORD PTR[12+rsp]
	add	edi,eax
	and	ebx,r12d
	rol	edx,1
	add	edi,ebx
	rol	r12d,30
	mov	DWORD PTR[24+rsp],edx
	add	edi,ecx
	mov	ebp,DWORD PTR[28+rsp]
	mov	eax,r12d
	mov	ebx,r12d
	xor	ebp,DWORD PTR[36+rsp]
	and	eax,r13d
	mov	ecx,edi
	xor	ebp,DWORD PTR[60+rsp]
	xor	ebx,r13d
	lea	esi,DWORD PTR[08f1bbcdch+rsi*1+rdx]
	rol	ecx,5
	xor	ebp,DWORD PTR[16+rsp]
	add	esi,eax
	and	ebx,r11d
	rol	ebp,1
	add	esi,ebx
	rol	r11d,30
	mov	DWORD PTR[28+rsp],ebp
	add	esi,ecx
	mov	edx,DWORD PTR[32+rsp]
	mov	eax,r11d
	mov	ebx,r11d
	xor	edx,DWORD PTR[40+rsp]
	and	eax,r12d
	mov	ecx,esi
	xor	edx,DWORD PTR[rsp]
	xor	ebx,r12d
	lea	r13d,DWORD PTR[08f1bbcdch+r13*1+rbp]
	rol	ecx,5
	xor	edx,DWORD PTR[20+rsp]
	add	r13d,eax
	and	ebx,edi
	rol	edx,1
	add	r13d,ebx
	rol	edi,30
	mov	DWORD PTR[32+rsp],edx
	add	r13d,ecx
	mov	ebp,DWORD PTR[36+rsp]
	mov	eax,edi
	mov	ebx,edi
	xor	ebp,DWORD PTR[44+rsp]
	and	eax,r11d
	mov	ecx,r13d
	xor	ebp,DWORD PTR[4+rsp]
	xor	ebx,r11d
	lea	r12d,DWORD PTR[08f1bbcdch+r12*1+rdx]
	rol	ecx,5
	xor	ebp,DWORD PTR[24+rsp]
	add	r12d,eax
	and	ebx,esi
	rol	ebp,1
	add	r12d,ebx
	rol	esi,30
	mov	DWORD PTR[36+rsp],ebp
	add	r12d,ecx
	mov	edx,DWORD PTR[40+rsp]
	mov	eax,esi
	mov	ebx,esi
	xor	edx,DWORD PTR[48+rsp]
	and	eax,edi
	mov	ecx,r12d
	xor	edx,DWORD PTR[8+rsp]
	xor	ebx,edi
	lea	r11d,DWORD PTR[08f1bbcdch+r11*1+rbp]
	rol	ecx,5
	xor	edx,DWORD PTR[28+rsp]
	add	r11d,eax
	and	ebx,r13d
	rol	edx,1
	add	r11d,ebx
	rol	r13d,30
	mov	DWORD PTR[40+rsp],edx
	add	r11d,ecx
	mov	ebp,DWORD PTR[44+rsp]
	mov	eax,r13d
	mov	ebx,r13d
	xor	ebp,DWORD PTR[52+rsp]
	and	eax,esi
	mov	ecx,r11d
	xor	ebp,DWORD PTR[12+rsp]
	xor	ebx,esi
	lea	edi,DWORD PTR[08f1bbcdch+rdi*1+rdx]
	rol	ecx,5
	xor	ebp,DWORD PTR[32+rsp]
	add	edi,eax
	and	ebx,r12d
	rol	ebp,1
	add	edi,ebx
	rol	r12d,30
	mov	DWORD PTR[44+rsp],ebp
	add	edi,ecx
	mov	edx,DWORD PTR[48+rsp]
	mov	eax,r12d
	mov	ebx,r12d
	xor	edx,DWORD PTR[56+rsp]
	and	eax,r13d
	mov	ecx,edi
	xor	edx,DWORD PTR[16+rsp]
	xor	ebx,r13d
	lea	esi,DWORD PTR[08f1bbcdch+rsi*1+rbp]
	rol	ecx,5
	xor	edx,DWORD PTR[36+rsp]
	add	esi,eax
	and	ebx,r11d
	rol	edx,1
	add	esi,ebx
	rol	r11d,30
	mov	DWORD PTR[48+rsp],edx
	add	esi,ecx
	mov	ebp,DWORD PTR[52+rsp]
	mov	eax,r11d
	mov	ecx,esi
	xor	ebp,DWORD PTR[60+rsp]
	xor	eax,edi
	rol	ecx,5
	lea	r13d,DWORD PTR[3395469782+r13*1+rdx]
	xor	ebp,DWORD PTR[20+rsp]
	xor	eax,r12d
	add	r13d,ecx
	xor	ebp,DWORD PTR[40+rsp]
	rol	edi,30
	add	r13d,eax
	rol	ebp,1
	mov	DWORD PTR[52+rsp],ebp
	mov	edx,DWORD PTR[56+rsp]
	mov	eax,edi
	mov	ecx,r13d
	xor	edx,DWORD PTR[rsp]
	xor	eax,esi
	rol	ecx,5
	lea	r12d,DWORD PTR[3395469782+r12*1+rbp]
	xor	edx,DWORD PTR[24+rsp]
	xor	eax,r11d
	add	r12d,ecx
	xor	edx,DWORD PTR[44+rsp]
	rol	esi,30
	add	r12d,eax
	rol	edx,1
	mov	DWORD PTR[56+rsp],edx
	mov	ebp,DWORD PTR[60+rsp]
	mov	eax,esi
	mov	ecx,r12d
	xor	ebp,DWORD PTR[4+rsp]
	xor	eax,r13d
	rol	ecx,5
	lea	r11d,DWORD PTR[3395469782+r11*1+rdx]
	xor	ebp,DWORD PTR[28+rsp]
	xor	eax,edi
	add	r11d,ecx
	xor	ebp,DWORD PTR[48+rsp]
	rol	r13d,30
	add	r11d,eax
	rol	ebp,1
	mov	DWORD PTR[60+rsp],ebp
	mov	edx,DWORD PTR[rsp]
	mov	eax,r13d
	mov	ecx,r11d
	xor	edx,DWORD PTR[8+rsp]
	xor	eax,r12d
	rol	ecx,5
	lea	edi,DWORD PTR[3395469782+rdi*1+rbp]
	xor	edx,DWORD PTR[32+rsp]
	xor	eax,esi
	add	edi,ecx
	xor	edx,DWORD PTR[52+rsp]
	rol	r12d,30
	add	edi,eax
	rol	edx,1
	mov	DWORD PTR[rsp],edx
	mov	ebp,DWORD PTR[4+rsp]
	mov	eax,r12d
	mov	ecx,edi
	xor	ebp,DWORD PTR[12+rsp]
	xor	eax,r11d
	rol	ecx,5
	lea	esi,DWORD PTR[3395469782+rsi*1+rdx]
	xor	ebp,DWORD PTR[36+rsp]
	xor	eax,r13d
	add	esi,ecx
	xor	ebp,DWORD PTR[56+rsp]
	rol	r11d,30
	add	esi,eax
	rol	ebp,1
	mov	DWORD PTR[4+rsp],ebp
	mov	edx,DWORD PTR[8+rsp]
	mov	eax,r11d
	mov	ecx,esi
	xor	edx,DWORD PTR[16+rsp]
	xor	eax,edi
	rol	ecx,5
	lea	r13d,DWORD PTR[3395469782+r13*1+rbp]
	xor	edx,DWORD PTR[40+rsp]
	xor	eax,r12d
	add	r13d,ecx
	xor	edx,DWORD PTR[60+rsp]
	rol	edi,30
	add	r13d,eax
	rol	edx,1
	mov	DWORD PTR[8+rsp],edx
	mov	ebp,DWORD PTR[12+rsp]
	mov	eax,edi
	mov	ecx,r13d
	xor	ebp,DWORD PTR[20+rsp]
	xor	eax,esi
	rol	ecx,5
	lea	r12d,DWORD PTR[3395469782+r12*1+rdx]
	xor	ebp,DWORD PTR[44+rsp]
	xor	eax,r11d
	add	r12d,ecx
	xor	ebp,DWORD PTR[rsp]
	rol	esi,30
	add	r12d,eax
	rol	ebp,1
	mov	DWORD PTR[12+rsp],ebp
	mov	edx,DWORD PTR[16+rsp]
	mov	eax,esi
	mov	ecx,r12d
	xor	edx,DWORD PTR[24+rsp]
	xor	eax,r13d
	rol	ecx,5
	lea	r11d,DWORD PTR[3395469782+r11*1+rbp]
	xor	edx,DWORD PTR[48+rsp]
	xor	eax,edi
	add	r11d,ecx
	xor	edx,DWORD PTR[4+rsp]
	rol	r13d,30
	add	r11d,eax
	rol	edx,1
	mov	DWORD PTR[16+rsp],edx
	mov	ebp,DWORD PTR[20+rsp]
	mov	eax,r13d
	mov	ecx,r11d
	xor	ebp,DWORD PTR[28+rsp]
	xor	eax,r12d
	rol	ecx,5
	lea	edi,DWORD PTR[3395469782+rdi*1+rdx]
	xor	ebp,DWORD PTR[52+rsp]
	xor	eax,esi
	add	edi,ecx
	xor	ebp,DWORD PTR[8+rsp]
	rol	r12d,30
	add	edi,eax
	rol	ebp,1
	mov	DWORD PTR[20+rsp],ebp
	mov	edx,DWORD PTR[24+rsp]
	mov	eax,r12d
	mov	ecx,edi
	xor	edx,DWORD PTR[32+rsp]
	xor	eax,r11d
	rol	ecx,5
	lea	esi,DWORD PTR[3395469782+rsi*1+rbp]
	xor	edx,DWORD PTR[56+rsp]
	xor	eax,r13d
	add	esi,ecx
	xor	edx,DWORD PTR[12+rsp]
	rol	r11d,30
	add	esi,eax
	rol	edx,1
	mov	DWORD PTR[24+rsp],edx
	mov	ebp,DWORD PTR[28+rsp]
	mov	eax,r11d
	mov	ecx,esi
	xor	ebp,DWORD PTR[36+rsp]
	xor	eax,edi
	rol	ecx,5
	lea	r13d,DWORD PTR[3395469782+r13*1+rdx]
	xor	ebp,DWORD PTR[60+rsp]
	xor	eax,r12d
	add	r13d,ecx
	xor	ebp,DWORD PTR[16+rsp]
	rol	edi,30
	add	r13d,eax
	rol	ebp,1
	mov	DWORD PTR[28+rsp],ebp
	mov	edx,DWORD PTR[32+rsp]
	mov	eax,edi
	mov	ecx,r13d
	xor	edx,DWORD PTR[40+rsp]
	xor	eax,esi
	rol	ecx,5
	lea	r12d,DWORD PTR[3395469782+r12*1+rbp]
	xor	edx,DWORD PTR[rsp]
	xor	eax,r11d
	add	r12d,ecx
	xor	edx,DWORD PTR[20+rsp]
	rol	esi,30
	add	r12d,eax
	rol	edx,1
	mov	DWORD PTR[32+rsp],edx
	mov	ebp,DWORD PTR[36+rsp]
	mov	eax,esi
	mov	ecx,r12d
	xor	ebp,DWORD PTR[44+rsp]
	xor	eax,r13d
	rol	ecx,5
	lea	r11d,DWORD PTR[3395469782+r11*1+rdx]
	xor	ebp,DWORD PTR[4+rsp]
	xor	eax,edi
	add	r11d,ecx
	xor	ebp,DWORD PTR[24+rsp]
	rol	r13d,30
	add	r11d,eax
	rol	ebp,1
	mov	DWORD PTR[36+rsp],ebp
	mov	edx,DWORD PTR[40+rsp]
	mov	eax,r13d
	mov	ecx,r11d
	xor	edx,DWORD PTR[48+rsp]
	xor	eax,r12d
	rol	ecx,5
	lea	edi,DWORD PTR[3395469782+rdi*1+rbp]
	xor	edx,DWORD PTR[8+rsp]
	xor	eax,esi
	add	edi,ecx
	xor	edx,DWORD PTR[28+rsp]
	rol	r12d,30
	add	edi,eax
	rol	edx,1
	mov	DWORD PTR[40+rsp],edx
	mov	ebp,DWORD PTR[44+rsp]
	mov	eax,r12d
	mov	ecx,edi
	xor	ebp,DWORD PTR[52+rsp]
	xor	eax,r11d
	rol	ecx,5
	lea	esi,DWORD PTR[3395469782+rsi*1+rdx]
	xor	ebp,DWORD PTR[12+rsp]
	xor	eax,r13d
	add	esi,ecx
	xor	ebp,DWORD PTR[32+rsp]
	rol	r11d,30
	add	esi,eax
	rol	ebp,1
	mov	DWORD PTR[44+rsp],ebp
	mov	edx,DWORD PTR[48+rsp]
	mov	eax,r11d
	mov	ecx,esi
	xor	edx,DWORD PTR[56+rsp]
	xor	eax,edi
	rol	ecx,5
	lea	r13d,DWORD PTR[3395469782+r13*1+rbp]
	xor	edx,DWORD PTR[16+rsp]
	xor	eax,r12d
	add	r13d,ecx
	xor	edx,DWORD PTR[36+rsp]
	rol	edi,30
	add	r13d,eax
	rol	edx,1
	mov	DWORD PTR[48+rsp],edx
	mov	ebp,DWORD PTR[52+rsp]
	mov	eax,edi
	mov	ecx,r13d
	xor	ebp,DWORD PTR[60+rsp]
	xor	eax,esi
	rol	ecx,5
	lea	r12d,DWORD PTR[3395469782+r12*1+rdx]
	xor	ebp,DWORD PTR[20+rsp]
	xor	eax,r11d
	add	r12d,ecx
	xor	ebp,DWORD PTR[40+rsp]
	rol	esi,30
	add	r12d,eax
	rol	ebp,1
	mov	edx,DWORD PTR[56+rsp]
	mov	eax,esi
	mov	ecx,r12d
	xor	edx,DWORD PTR[rsp]
	xor	eax,r13d
	rol	ecx,5
	lea	r11d,DWORD PTR[3395469782+r11*1+rbp]
	xor	edx,DWORD PTR[24+rsp]
	xor	eax,edi
	add	r11d,ecx
	xor	edx,DWORD PTR[44+rsp]
	rol	r13d,30
	add	r11d,eax
	rol	edx,1
	mov	ebp,DWORD PTR[60+rsp]
	mov	eax,r13d
	mov	ecx,r11d
	xor	ebp,DWORD PTR[4+rsp]
	xor	eax,r12d
	rol	ecx,5
	lea	edi,DWORD PTR[3395469782+rdi*1+rdx]
	xor	ebp,DWORD PTR[28+rsp]
	xor	eax,esi
	add	edi,ecx
	xor	ebp,DWORD PTR[48+rsp]
	rol	r12d,30
	add	edi,eax
	rol	ebp,1
	mov	eax,r12d
	mov	ecx,edi
	xor	eax,r11d
	lea	esi,DWORD PTR[3395469782+rsi*1+rbp]
	rol	ecx,5
	xor	eax,r13d
	add	esi,ecx
	rol	r11d,30
	add	esi,eax
	add	esi,DWORD PTR[r8]
	add	edi,DWORD PTR[4+r8]
	add	r11d,DWORD PTR[8+r8]
	add	r12d,DWORD PTR[12+r8]
	add	r13d,DWORD PTR[16+r8]
	mov	DWORD PTR[r8],esi
	mov	DWORD PTR[4+r8],edi
	mov	DWORD PTR[8+r8],r11d
	mov	DWORD PTR[12+r8],r12d
	mov	DWORD PTR[16+r8],r13d

	sub	r10,1
	lea	r9,QWORD PTR[64+r9]
	jnz	$L$loop

	mov	rsi,QWORD PTR[64+rsp]
	mov	r13,QWORD PTR[rsi]
	mov	r12,QWORD PTR[8+rsi]
	mov	rbp,QWORD PTR[16+rsi]
	mov	rbx,QWORD PTR[24+rsi]
	lea	rsp,QWORD PTR[32+rsi]
$L$epilogue::
	mov	rdi,QWORD PTR[8+rsp]	;WIN64 epilogue
	mov	rsi,QWORD PTR[16+rsp]
	DB	0F3h,0C3h		;repret
$L$SEH_end_sha1_block_data_order::
sha1_block_data_order	ENDP

DB	83,72,65,49,32,98,108,111,99,107,32,116,114,97,110,115
DB	102,111,114,109,32,102,111,114,32,120,56,54,95,54,52,44
DB	32,67,82,89,80,84,79,71,65,77,83,32,98,121,32,60
DB	97,112,112,114,111,64,111,112,101,110,115,115,108,46,111,114
DB	103,62,0
ALIGN	16
EXTERN	__imp_RtlVirtualUnwind:NEAR

ALIGN	16
se_handler	PROC PRIVATE
	push	rsi
	push	rdi
	push	rbx
	push	rbp
	push	r12
	push	r13
	push	r14
	push	r15
	pushfq
	sub	rsp,64

	mov	rax,QWORD PTR[120+r8]
	mov	rbx,QWORD PTR[248+r8]

	lea	r10,QWORD PTR[$L$prologue]
	cmp	rbx,r10
	jb	$L$in_prologue

	mov	rax,QWORD PTR[152+r8]

	lea	r10,QWORD PTR[$L$epilogue]
	cmp	rbx,r10
	jae	$L$in_prologue

	mov	rax,QWORD PTR[64+rax]
	lea	rax,QWORD PTR[32+rax]

	mov	rbx,QWORD PTR[((-8))+rax]
	mov	rbp,QWORD PTR[((-16))+rax]
	mov	r12,QWORD PTR[((-24))+rax]
	mov	r13,QWORD PTR[((-32))+rax]
	mov	QWORD PTR[144+r8],rbx
	mov	QWORD PTR[160+r8],rbp
	mov	QWORD PTR[216+r8],r12
	mov	QWORD PTR[224+r8],r13

$L$in_prologue::
	mov	rdi,QWORD PTR[8+rax]
	mov	rsi,QWORD PTR[16+rax]
	mov	QWORD PTR[152+r8],rax
	mov	QWORD PTR[168+r8],rsi
	mov	QWORD PTR[176+r8],rdi

	mov	rdi,QWORD PTR[40+r9]
	mov	rsi,r8
	mov	ecx,154
	DD	0a548f3fch		

	mov	rsi,r9
	xor	rcx,rcx
	mov	rdx,QWORD PTR[8+rsi]
	mov	r8,QWORD PTR[rsi]
	mov	r9,QWORD PTR[16+rsi]
	mov	r10,QWORD PTR[40+rsi]
	lea	r11,QWORD PTR[56+rsi]
	lea	r12,QWORD PTR[24+rsi]
	mov	QWORD PTR[32+rsp],r10
	mov	QWORD PTR[40+rsp],r11
	mov	QWORD PTR[48+rsp],r12
	mov	QWORD PTR[56+rsp],rcx
	call	QWORD PTR[__imp_RtlVirtualUnwind]

	mov	eax,1
	add	rsp,64
	popfq
	pop	r15
	pop	r14
	pop	r13
	pop	r12
	pop	rbp
	pop	rbx
	pop	rdi
	pop	rsi
	DB	0F3h,0C3h		;repret
se_handler	ENDP

.text$	ENDS
.pdata	SEGMENT READONLY ALIGN(4)
ALIGN	4
	DD	imagerel $L$SEH_begin_sha1_block_data_order
	DD	imagerel $L$SEH_end_sha1_block_data_order
	DD	imagerel $L$SEH_info_sha1_block_data_order

.pdata	ENDS
.xdata	SEGMENT READONLY ALIGN(8)
ALIGN	8
$L$SEH_info_sha1_block_data_order::
DB	9,0,0,0
	DD	imagerel se_handler

.xdata	ENDS
END
