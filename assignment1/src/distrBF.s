 	.syntax unified
 	.cpu cortex-m3
 	.thumb
 	.align 2
 	.global	distrBF
 	.thumb_func

@ EE2024 Assignment 1, Sem 1, AY 2017/18
@ (c) Siidheesh Theivasigamani 2017

@R0: T
@R1: dij
@R2: Dj
@R3: n_res
@R4: N
@R5: RETRIEVE_arr
@R6: Ti
@R7: j
@R8: RETRIEVE_retval
@R9: current dij
@R10: smallest current Di for current timestep
@R11: j for which smallest Di found

distrBF:
	push {r4-r11, r14}			@ push callee-save registers and lr
	cbz r0, end					@ input validation: T > 0
	ldr r4, [r3]				@ N = n_res[0][0]
	cbz r4, end					@ input validation: N > 0
	mov r6, #0					@ Ti = 0
	loopStep:					@ loop through steps
		mov r7, #0				@ j = 0
		ldr r10, =#0x7fffffff	@ set to largest signed 32-bit value as starting minimum
		loopNode:				@ loop through nodes
			mov r5, r1			@ set R5 to dij
			bl RETRIEVE			@ set R8 to dij[Ti][j]
			mov r9, r8			@ save to R9
			mov r5, r2			@ set R5 to Dj
			bl RETRIEVE			@ set R8 to Dj[Ti][j]
			//ldr r8, [r1], #4  @ RETRIEVE sub and the above 5 lines can be replaced with these 2 lines
			//ldr r9, [r2], #4
			add r8, r9			@ add dij[Ti][j] and Dj[Ti][j] as R8
			add r7, #1			@ j++
			cmp r10, r8			@ compare current min with new result
			itt gt				@ signed R10 > R8
			movgt r10, r8		@ save new min val
			movgt r11, r7		@ save new min pos
			cmp r4, r7			@ compare N with current j
			bgt loopNode		@ branch if N > j
		stm r3!, {r10, r11}		@ post-incremental multiple store of best val and pos to n_res
		add r6, #1				@ Ti++
		cmp r0, r6				@ compare T with current Ti
		bgt loopStep			@ branch if T > Ti
	end:
	pop {r4-r11, r14}			@ pop callee-save registers and lr
	bx lr						@ branch to address in LR (return)

@ Subroutine RETRIEVE
RETRIEVE:
	mla r8, r4, r6, r7			@ (N*t)+j into R8
	ldr r8, [r5, r8, lsl #2]	@ LDR from [R5, offset R8<<2] into R8
	bx lr						@ return

	nop
	.end
