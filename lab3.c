/* C Language Code created by Jack Webster for the purpose of 
   Lab 3 in ELEC371. This code has been adapted from template 
   code provided by N. Manjikian. 
   The program is designed to use 3 timers set at intervals 
   of 0.2s, 0.25s and 0.5s.
   The first timer shifts and wraps around a pair of leds
   The second timer prints 0, 1, 2, 3, 0... on rightmost 
   hex display. If switch 2 is active, A, b, C, d, A... is 
   printed instead.
   The third timer alternates between printing '-' and '|'
   to the JTAG_UART interface
   The Altera monitor program was used to visualize the 
   program on a FGPA board*/

#ifndef _NIOS2_CONTROL_H_
#define _NIOS2_CONTROL_H_

#define NIOS2_WRITE_STATUS(value)  (__builtin_wrctl (0, value))

#define NIOS2_READ_IENABLE()	   (__builtin_rdctl (3))

#define NIOS2_WRITE_IENABLE(value) (__builtin_wrctl (3, value))

#define NIOS2_READ_IPENDING()	   (__builtin_rdctl (4))

#endif /* _NIOS2_CONTROL_H_ */


#ifndef _TIMER_H_
#define _TIMER_H_

#define TIMER1_STATUS	((volatile unsigned int *) 0x10004020)

#define TIMER1_CONTROL	((volatile unsigned int *) 0x10004024)

#define TIMER1_START_LO	((volatile unsigned int *) 0x10004028)

#define TIMER1_START_HI	((volatile unsigned int *) 0x1000402C)

#define TIMER2_STATUS	((volatile unsigned int *) 0x10004040)

#define TIMER2_CONTROL	((volatile unsigned int *) 0x10004044)

#define TIMER2_START_LO	((volatile unsigned int *) 0x10004048)

#define TIMER2_START_HI	((volatile unsigned int *) 0x1000404C)

#define TIMER3_STATUS	((volatile unsigned int *) 0x10004060)

#define TIMER3_CONTROL	((volatile unsigned int *) 0x10004064)

#define TIMER3_START_LO	((volatile unsigned int *) 0x10004068)

#define TIMER3_START_HI	((volatile unsigned int *) 0x1000406C)
	
#define JTAG_UART_DATA  ((volatile unsigned int *) 0x10001000)

#define JTAG_UART_STATUS	((volatile unsigned int *) 0x10001004)
	
#define HEX_DISPLAY		((volatile unsigned int *) 0x10000020)

#define SWITCH			((volatile unsigned int *) 0x10000040)


#endif /* _TIMER_H_ */


#ifndef _LEDS_H_
#define _LEDS_H_

#define LEDS	((volatile unsigned int *) 0x10000010)


#endif /* _LEDS_H_ */


/* define global program variables here */
unsigned int curr_JTAG = '-'; //initalize the print out to this

void PrintChar(unsigned int ch);

void interrupt_handler(void)
{
	unsigned int ipending;

	/* read current value in ipending register */
	ipending = NIOS2_READ_IPENDING();
	/* do one or more checks for different sources using ipending value */
	if ((ipending & 0x4000) == 0x4000) {
		/* remember to clear interrupt sources */
		*TIMER1_STATUS = 0x0;
		
		unsigned int curr_led_state = *LEDS ;
		if (curr_led_state == 0x3) {
			curr_led_state = 0x300;
		} else {
			curr_led_state = curr_led_state >> 2;
		}
		*LEDS = curr_led_state;
	}
	
	if ((ipending & 0x8000) == 0x8000) {
		*TIMER2_STATUS = 0x0;
		unsigned int curr_hex_state = *HEX_DISPLAY;
		unsigned int curr_switch_state = *SWITCH; //is switch 2 on or off 
		if (curr_switch_state == 0x4) { //switch is on
			if (curr_hex_state == 0x77 || curr_hex_state == 0x3F) { //from 0 or A to b
				curr_hex_state = 0x7C;
			} else if (curr_hex_state == 0x7C || curr_hex_state == 0x6) { //1 or A to C
				curr_hex_state = 0x39;
			} else if (curr_hex_state == 0x39 || curr_hex_state == 0x5B) { //2 or c to d
				curr_hex_state = 0x5E;
			} else {
				curr_hex_state = 0x77; //3 or d to A
			}
		} else { //switch is off
			if (curr_hex_state == 0x77 || curr_hex_state == 0x3F) { //0 to 1
				curr_hex_state = 0x6;
			} else if (curr_hex_state == 0x7C || curr_hex_state == 0x6) { //1 to 2
				curr_hex_state = 0x5B;
			} else if (curr_hex_state == 0x39 || curr_hex_state == 0x5B) { //2 to 3
				curr_hex_state = 0x4F;
			} else {
				curr_hex_state = 0x3F; //3 to 0 
			}
		}
		*HEX_DISPLAY = curr_hex_state;
		
	if ((ipending & 0x10000) == 0x10000) { //alternate between printing a - or a |
		*TIMER3_STATUS = 0x0;
		if (curr_JTAG == '-') {
			curr_JTAG = '|';
		} else {
			curr_JTAG = '-';	
		}
		PrintChar(curr_JTAG);
	}     
}
}

void Init (void)
{
	
	/* initialize software variables */
	*TIMER1_START_LO = 0x9680;
	*TIMER1_START_HI = 0x0098;
	*TIMER1_CONTROL = 0x7;
	*TIMER2_START_LO = 0xBC20;
	*TIMER2_START_HI = 0x00BE;
	*TIMER2_CONTROL = 0x7;
	*TIMER3_START_LO = 0x7840;
	*TIMER3_START_HI = 0x017D;
	*TIMER3_CONTROL = 0x7;
	*LEDS = 0x300;
	*HEX_DISPLAY = 0x3F;

	/* set up each hardware interface */
	
	/* set up ienable */
	NIOS2_WRITE_IENABLE(0x1C000);
	NIOS2_WRITE_STATUS(0x1);
	/* enable global recognition of interrupts in procr. status reg. */
}


/* place additional functions here */
void PrintChar(unsigned int ch){
	unsigned int st;
	do {
		st = *JTAG_UART_STATUS;
		st = st & 0xFFFF0000;
	} while(st == 0);
	*JTAG_UART_DATA = ch;
	
}

int main (void)
{
	Init ();	/* perform software/hardware initialization */

	while (1)
	{
		/* fill in body of infinite loop */
		int count = 1;
	}

	return 0;	/* never reached, but main() must return a value */
}

/*-----------------------------------------------------------------*/
/*              end of application-specific code                   */
/*-----------------------------------------------------------------*/



/*-----------------------------------------------------------------*/


/* 
   exception_handler.c

   This file is a portion of the original code supplied by Altera.

   It has been adapted by N. Manjikian for use in ELEC 371 laboratory work.

   Various unnecessary or extraneous elements have been excluded. For
   example, declarations in C for external functions called from asm()
   instructions are not required because any reference to external names
   in asm() instructions is embedded directly in the output written to
   the assembly-language .s file without any other checks by the C compiler.

   There is one particularly important change: on _reset_, the jump must be
   to the >> _start << location in order to properly initialize the stack
   pointer and to perform other crucial initialization tasks that ensure
   proper C semantics for variable initialization are enforced. The Altera
   version of the code jumped to main(), which will _not_ perform these
   crucial initialization tasks correctly.

   Finally, a reference to control register 'ctl4' in the asm() sequence
   has been replaced with the more meaningful alias 'ipending' for clarity.

   Other than the changes described above, the file contents have also been
   reformatted to fit in 80 columns of text, and comments have been edited.
*/


/* The assembly language code below handles processor reset */
void the_reset (void) __attribute__ ((section (".reset")));

/*****************************************************************************
 * Reset code. By giving the code a section attribute with the name ".reset" *
 * we allow the linker program to locate this code at the proper reset vector*
 * address. This code jumps to _startup_ code for C program, _not_ main().   *
 *****************************************************************************/

void the_reset (void)
{
  asm (".set noat");         /* the .set commands are included to prevent */
  asm (".set nobreak");      /* warning messages from the assembler */
  asm ("movia r2, _start");  /* jump to the C language _startup_ code */
  asm ("jmp r2");            /* (_not_ main, as in the original Altera file) */
}

/* The assembly language code below handles exception processing. This
 * code should not be modified; instead, the C language code in the normal
 * function interrupt_handler() [which is called from the code below]
 * can be modified as needed for a given application.
 */

void the_exception (void) __attribute__ ((section (".exceptions")));

/*****************************************************************************
 * Exceptions code. By giving the code a section attribute with the name     *
 * ".exceptions" we allow the linker program to locate this code at the      *
 * proper exceptions vector address. This code calls the interrupt handler   *
 * and later returns from the exception to the main program.                 *
 *****************************************************************************/

void the_exception (void)
{
  asm (".set noat");         /* the .set commands are included to prevent */
  asm (".set nobreak");      /* warning messages from the assembler */
  asm ("subi sp, sp, 128");
  asm ("stw  et, 96(sp)");
  asm ("rdctl et, ipending"); /* changed 'ctl4' to 'ipending' for clarity */
  asm ("beq  et, r0, SKIP_EA_DEC");   /* Not a hardware interrupt, */
  asm ("subi ea, ea, 4");             /* so decrement ea by one instruction */ 
  asm ("SKIP_EA_DEC:");
  asm ("stw	r1,  4(sp)"); /* Save all registers */
  asm ("stw	r2,  8(sp)");
  asm ("stw	r3,  12(sp)");
  asm ("stw	r4,  16(sp)");
  asm ("stw	r5,  20(sp)");
  asm ("stw	r6,  24(sp)");
  asm ("stw	r7,  28(sp)");
  asm ("stw	r8,  32(sp)");
  asm ("stw	r9,  36(sp)");
  asm ("stw	r10, 40(sp)");
  asm ("stw	r11, 44(sp)");
  asm ("stw	r12, 48(sp)");
  asm ("stw	r13, 52(sp)");
  asm ("stw	r14, 56(sp)");
  asm ("stw	r15, 60(sp)");
  asm ("stw	r16, 64(sp)");
  asm ("stw	r17, 68(sp)");
  asm ("stw	r18, 72(sp)");
  asm ("stw	r19, 76(sp)");
  asm ("stw	r20, 80(sp)");
  asm ("stw	r21, 84(sp)");
  asm ("stw	r22, 88(sp)");
  asm ("stw	r23, 92(sp)");
  asm ("stw	r25, 100(sp)"); /* r25 = bt (r24 = et, saved above) */
  asm ("stw	r26, 104(sp)"); /* r26 = gp */
  /* skip saving r27 because it is sp, and there is no point in saving sp */
  asm ("stw	r28, 112(sp)"); /* r28 = fp */
  asm ("stw	r29, 116(sp)"); /* r29 = ea */
  asm ("stw	r30, 120(sp)"); /* r30 = ba */
  asm ("stw	r31, 124(sp)"); /* r31 = ra */
  asm ("addi	fp,  sp, 128"); /* frame pointer adjustment */

  asm ("call	interrupt_handler"); /* call normal function */

  asm ("ldw	r1,  4(sp)"); /* Restore all registers */
  asm ("ldw	r2,  8(sp)");
  asm ("ldw	r3,  12(sp)");
  asm ("ldw	r4,  16(sp)");
  asm ("ldw	r5,  20(sp)");
  asm ("ldw	r6,  24(sp)");
  asm ("ldw	r7,  28(sp)");
  asm ("ldw	r8,  32(sp)");
  asm ("ldw	r9,  36(sp)");
  asm ("ldw	r10, 40(sp)");
  asm ("ldw	r11, 44(sp)");
  asm ("ldw	r12, 48(sp)");
  asm ("ldw	r13, 52(sp)");
  asm ("ldw	r14, 56(sp)");
  asm ("ldw	r15, 60(sp)");
  asm ("ldw	r16, 64(sp)");
  asm ("ldw	r17, 68(sp)");
  asm ("ldw	r18, 72(sp)");
  asm ("ldw	r19, 76(sp)");
  asm ("ldw	r20, 80(sp)");
  asm ("ldw	r21, 84(sp)");
  asm ("ldw	r22, 88(sp)");
  asm ("ldw	r23, 92(sp)");
  asm ("ldw	r24, 96(sp)");
  asm ("ldw	r25, 100(sp)");
  asm ("ldw	r26, 104(sp)");
  /* skip r27 because it is sp, and we did not save this on the stack */
  asm ("ldw	r28, 112(sp)");
  asm ("ldw	r29, 116(sp)");
  asm ("ldw	r30, 120(sp)");
  asm ("ldw	r31, 124(sp)");

  asm ("addi	sp,  sp, 128");

  asm ("eret"); /* return from exception */

  /* Note that the C compiler will still generate the 'standard'
     end-of-normal-function code with a normal return-from-subroutine
     instruction. But with the above eret instruction embedded
     in the final output from the compiler, that end-of-function code
     will never be executed.
   */ 
}
