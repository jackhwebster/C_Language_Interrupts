/*Interacts with a temperature sensor (input port) and a heater
(output port) to moderate temperature.
If temp falls below 98 degrees, heater is turned on
If temp goes above 102 degrees, heater is turned off
Program will also print L, -, or H depending on what 
temperature it currently is with regard to the 2 bounds*/
#include "nios2_control.h"

#define TIMER0_STATUS (volatile unsigned int*) 0x5000
#define TIMER0_CONTROL  (volatile unsigned int*) 0x5004
#define TIMER0_START_LO (volatile unsigned int*) 0x5008
#define TIMER0_START_HI (volatile unsigned int*) 0x500C 

#define TIMER1_STATUS (volatile unsigned int*) 0x6000
#define TIMER1_CONTROL  (volatile unsigned int*) 0x6004
#define TIMER1_START_LO (volatile unsigned int*) 0x6008
#define TIMER1_START_HI (volatile unsigned int*) 0x600C 

#define INPORT_DATA (volatile unsigned int*) 0x6A00
#define OUTPORT_DATA (volatile unsigned int*) 0x6B00

#define JTAG_UART_DATA (volatile unsigned int*) 0x6C00
#define JTAG_UART_STATUS (volatile unsigned int*) 0x6C04

unsigned int timer1Flag = 0;

void Init(void){
    *TIMER0_START_HI = 50000000 >> 16;
    *TIMER0_START_LO = 50000000 & 0xFFFF;
    *TIMER0_CONTROL = 0x7;

    *TIMER1_START_HI = 12500000 >> 16;
    *TIMER1_START_LO = 12500000 & 0xFFFF;
    *TIMER1_CONTROL = 0x7;

    NIOS2_WRITE_IENABLE(0x3);
    NIOS2_WRITE_STATUS(0x1); 
}

void PrintChar(unsigned int ch){
    unsigned int st;
    do{
        st = *JTAG_UART_STATUS;
        st = st & 0xFFFF0000;
    }while (st == 0);
    *JTAG_UART_DATA = ch;
}

void HandleTimer0 (void){
    if (*INPORT_DATA < 98) *OUTPORT_DATA = 128;
    else if (*INPORT_DATA > 102) *OUTPORT_DATA = 0;
}

void HandleTimer1 (void){
    timer1Flag = 1;
}

void interrupt_handler(void){
    unsigned int ipending = NIOS2_READ_IPENDING();

    if (ipending & 0x1){
        *TIMER0_STATUS = 0;
        HandleTimer0();
    }

    if (ipending & 0x2){
        *TIMER1_STATUS = 0;
        HandleTimer1();
    }
}

int main (void){
    Init();
    PrintChar(' ');
    while(1){
        if (timer1Flag = 1){
            timer1Flag = 0;
            PrintChar('\b');
            if (*INPORT_DATA < 98) PrintChar('L');
            else if (*INPORT_DATA > 102) PrintChar('H');
            else PrintChar('-');
        }
    }
}
