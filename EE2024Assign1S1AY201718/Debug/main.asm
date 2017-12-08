   1              		.syntax unified
   2              		.cpu cortex-m3
   3              		.fpu softvfp
   4              		.eabi_attribute 20, 1
   5              		.eabi_attribute 21, 1
   6              		.eabi_attribute 23, 3
   7              		.eabi_attribute 24, 1
   8              		.eabi_attribute 25, 1
   9              		.eabi_attribute 26, 1
  10              		.eabi_attribute 30, 6
  11              		.eabi_attribute 34, 1
  12              		.eabi_attribute 18, 4
  13              		.thumb
  14              		.file	"main.c"
  15              		.text
  16              	.Ltext0:
  17              		.cfi_sections	.debug_frame
  18              		.section	.rodata
  19              		.align	2
  20              	.LC2:
  21 0000 25642025 		.ascii	"%d %d\012\000"
  21      640A00
  22 0007 00       		.align	2
  23              	.LC0:
  24 0008 09000000 		.word	9
  25 000c 07000000 		.word	7
  26 0010 0A000000 		.word	10
  27              		.align	2
  28              	.LC1:
  29 0014 11000000 		.word	17
  30 0018 12000000 		.word	18
  31 001c 13000000 		.word	19
  32              		.section	.text.main,"ax",%progbits
  33              		.align	2
  34              		.global	main
  35              		.thumb
  36              		.thumb_func
  38              	main:
  39              	.LFB0:
  40              		.file 1 "../src/main.c"
   1:../src/main.c **** #include "stdio.h"
   2:../src/main.c **** 
   3:../src/main.c **** extern void distrBF(int T_arg, int* dij_arr, int* Dj_arr, int* n_res_arr);
   4:../src/main.c **** 
   5:../src/main.c **** // EE2024 Assignment 1, Sem 1, AY 2017/18
   6:../src/main.c **** // (c) CK Tham, ECE NUS, 2017
   7:../src/main.c **** 
   8:../src/main.c **** #define T 3
   9:../src/main.c **** #define N 1
  10:../src/main.c **** 
  11:../src/main.c **** int main(void)
  12:../src/main.c **** {
  41              		.loc 1 12 0
  42              		.cfi_startproc
  43              		@ args = 0, pretend = 0, frame = 56
  44              		@ frame_needed = 1, uses_anonymous_args = 0
  45 0000 90B5     		push	{r4, r7, lr}
  46              	.LCFI0:
  47              		.cfi_def_cfa_offset 12
  48              		.cfi_offset 14, -4
  49              		.cfi_offset 7, -8
  50              		.cfi_offset 4, -12
  51 0002 8FB0     		sub	sp, sp, #60
  52              	.LCFI1:
  53              		.cfi_def_cfa_offset 72
  54 0004 00AF     		add	r7, sp, #0
  55              	.LCFI2:
  56              		.cfi_def_cfa_register 7
  13:../src/main.c **** 	// Variable definitions
  14:../src/main.c **** 	int i;
  15:../src/main.c **** 
  16:../src/main.c **** 	// Note: different initialisation list is needed for different T, N
  17:../src/main.c **** 	int dij[T][N]={{9,5,7,8},
  57              		.loc 1 17 0
  58 0006 40F20003 		movw	r3, #:lower16:.LC0
  59 000a C0F20003 		movt	r3, #:upper16:.LC0
  60 000e 07F12804 		add	r4, r7, #40
  61 0012 93E80700 		ldmia	r3, {r0, r1, r2}
  62 0016 84E80700 		stmia	r4, {r0, r1, r2}
  18:../src/main.c **** 	               {7,9,6,9},
  19:../src/main.c **** 	               {10,8,8,7}};
  20:../src/main.c **** 
  21:../src/main.c **** 	int Dj[T][N]={{17,23,14,15},
  63              		.loc 1 21 0
  64 001a 40F20003 		movw	r3, #:lower16:.LC1
  65 001e C0F20003 		movt	r3, #:upper16:.LC1
  66 0022 07F11C04 		add	r4, r7, #28
  67 0026 93E80700 		ldmia	r3, {r0, r1, r2}
  68 002a 84E80700 		stmia	r4, {r0, r1, r2}
  22:../src/main.c **** 	              {18,19,18,14},
  23:../src/main.c **** 	              {19,22,17,16}};
  24:../src/main.c **** 
  25:../src/main.c **** 	int n_res[T][2]={{N,0},
  69              		.loc 1 25 0
  70 002e 4FF00103 		mov	r3, #1
  71 0032 7B60     		str	r3, [r7, #4]
  72 0034 4FF00003 		mov	r3, #0
  73 0038 BB60     		str	r3, [r7, #8]
  74 003a 4FF00003 		mov	r3, #0
  75 003e FB60     		str	r3, [r7, #12]
  76 0040 4FF00003 		mov	r3, #0
  77 0044 3B61     		str	r3, [r7, #16]
  78 0046 4FF00003 		mov	r3, #0
  79 004a 7B61     		str	r3, [r7, #20]
  80 004c 4FF00003 		mov	r3, #0
  81 0050 BB61     		str	r3, [r7, #24]
  26:../src/main.c **** 	                 {0,0},
  27:../src/main.c **** 	                 {0,0}};
  28:../src/main.c **** 
  29:../src/main.c **** 	// Call assembly language function
  30:../src/main.c **** 	distrBF(T,(int*)dij,(int*)Dj,(int*)n_res);
  82              		.loc 1 30 0
  83 0052 07F12801 		add	r1, r7, #40
  84 0056 07F11C02 		add	r2, r7, #28
  85 005a 07F10403 		add	r3, r7, #4
  86 005e 4FF00300 		mov	r0, #3
  87 0062 FFF7FEFF 		bl	distrBF
  31:../src/main.c **** 
  32:../src/main.c **** 	// Print out results on console
  33:../src/main.c **** 	for ( i=0; i<T ; i++ )
  88              		.loc 1 33 0
  89 0066 4FF00003 		mov	r3, #0
  90 006a 7B63     		str	r3, [r7, #52]
  91 006c 1AE0     		b	.L2
  92              	.L3:
  34:../src/main.c **** 		printf("%d %d\n", n_res[i][0], n_res[i][1]);
  93              		.loc 1 34 0 discriminator 2
  94 006e 40F20003 		movw	r3, #:lower16:.LC2
  95 0072 C0F20003 		movt	r3, #:upper16:.LC2
  96 0076 7A6B     		ldr	r2, [r7, #52]
  97 0078 4FEAC202 		lsl	r2, r2, #3
  98 007c 07F13800 		add	r0, r7, #56
  99 0080 8218     		adds	r2, r0, r2
 100 0082 52F8341C 		ldr	r1, [r2, #-52]
 101 0086 7A6B     		ldr	r2, [r7, #52]
 102 0088 4FEAC202 		lsl	r2, r2, #3
 103 008c 07F13800 		add	r0, r7, #56
 104 0090 8218     		adds	r2, r0, r2
 105 0092 52F8302C 		ldr	r2, [r2, #-48]
 106 0096 1846     		mov	r0, r3
 107 0098 FFF7FEFF 		bl	printf
  33:../src/main.c **** 	for ( i=0; i<T ; i++ )
 108              		.loc 1 33 0 discriminator 2
 109 009c 7B6B     		ldr	r3, [r7, #52]
 110 009e 03F10103 		add	r3, r3, #1
 111 00a2 7B63     		str	r3, [r7, #52]
 112              	.L2:
  33:../src/main.c **** 	for ( i=0; i<T ; i++ )
 113              		.loc 1 33 0 is_stmt 0 discriminator 1
 114 00a4 7B6B     		ldr	r3, [r7, #52]
 115 00a6 022B     		cmp	r3, #2
 116 00a8 E1DD     		ble	.L3
 117              	.L4:
  35:../src/main.c **** 
  36:../src/main.c **** 	// Enter an infinite loop, just incrementing a counter
  37:../src/main.c **** 	volatile static int loop = 0;
  38:../src/main.c **** 	while (1) {
  39:../src/main.c **** 		loop++;
 118              		.loc 1 39 0 is_stmt 1 discriminator 1
 119 00aa 40F20003 		movw	r3, #:lower16:loop.3821
 120 00ae C0F20003 		movt	r3, #:upper16:loop.3821
 121 00b2 1B68     		ldr	r3, [r3, #0]
 122 00b4 03F10102 		add	r2, r3, #1
 123 00b8 40F20003 		movw	r3, #:lower16:loop.3821
 124 00bc C0F20003 		movt	r3, #:upper16:loop.3821
 125 00c0 1A60     		str	r2, [r3, #0]
  40:../src/main.c **** 	}
 126              		.loc 1 40 0 discriminator 1
 127 00c2 F2E7     		b	.L4
 128              		.cfi_endproc
 129              	.LFE0:
 131              		.bss
 132              		.align	2
 133              	loop.3821:
 134 0000 00000000 		.space	4
 135              		.text
 136              	.Letext0:
DEFINED SYMBOLS
                            *ABS*:00000000 main.c
C:\Users\SIIDHE~1\AppData\Local\Temp\ccDRNk82.s:19     .rodata:00000000 $d
C:\Users\SIIDHE~1\AppData\Local\Temp\ccDRNk82.s:20     .rodata:00000000 .LC2
C:\Users\SIIDHE~1\AppData\Local\Temp\ccDRNk82.s:23     .rodata:00000008 .LC0
C:\Users\SIIDHE~1\AppData\Local\Temp\ccDRNk82.s:28     .rodata:00000014 .LC1
C:\Users\SIIDHE~1\AppData\Local\Temp\ccDRNk82.s:33     .text.main:00000000 $t
C:\Users\SIIDHE~1\AppData\Local\Temp\ccDRNk82.s:38     .text.main:00000000 main
C:\Users\SIIDHE~1\AppData\Local\Temp\ccDRNk82.s:133    .bss:00000000 loop.3821
C:\Users\SIIDHE~1\AppData\Local\Temp\ccDRNk82.s:132    .bss:00000000 $d
                     .debug_frame:00000010 $d

UNDEFINED SYMBOLS
distrBF
printf
