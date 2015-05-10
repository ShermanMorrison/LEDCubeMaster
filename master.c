/*
 * PWM Control of an LED Matrix layer.
 */

#include <msp430g2553.h>		//must include so compiler knows what each variable means


#define PI 3.14159
const int led_num_cycles = 12;
const char col_array[5] = {BIT0, BIT3, 0x00, BIT6, BIT7};
const char row_array[5] = {BIT0, BIT1, BIT2, BIT3, BIT4};
int pass(int x, int y, int z, int t);
int pass2(int x, int y, int z, int t);
int wave(int x, int y, int z, int t);
int cosine(int v);
int count = 0;
int which_layer = 0;
const int LIMIT_COUNT = 10;
int flag = 1;

char pwm_array[5][5] = {
		{0,		0,		0,	    0,		0},
		{0,		0,		0,		0,		0},
		{0,		0,		 1, 	0,		1},
		{0,		0,		 0,		0,		0},
		{0,		0,		 1,		0,		0}
}; //Layer 0 row,col. Elements go from 0 to led_num_cycles

char pwm_array_2[5][5] = {
		{10,			0,		0,		0,		0},
		{0,			0,		0,		0,		0},
		{0,			0,		 0, 	0,		0},
		{0,			0,		 0,		0,		0},
		{0,			0,		 0,		0,		0}
}; //Layer 1

void main(void){

	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	BCSCTL3 |= LFXT1S_2;					// Set clock source to VLO (only option-- don't have external oscillator)
	BCSCTL1 |= DIVA_0;						// ACLK is sourced on VLO by default. Run at 12KHz.
	BCSCTL2 |= SELM_3 + SELS + DIVM_0;   	// SMCLK  = MCLK = VLO = 12KHz

	P1SEL  =   BIT1 	|BIT2    |   BIT4;	//enable UCA0 transmit and clock
	P1SEL2 =   BIT1 	|	BIT2    |   BIT4;

	P1DIR |= BIT0;
	P2DIR |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);

	P1REN |= BIT6;
	P1OUT |= BIT6;
	P1IE  |= BIT6;
	P1IES = 0;
	P1IFG = 0;

	UCA0CTL0 |= UCCKPL + UCMSB + UCMST + UCSYNC + UCMODE_1;  // 3-pin, 8-bit SPI master
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 |= 0x02;                          // /2
	UCA0BR1 = 0;                              //
	UCA0MCTL = 0;                             // No modulation
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	//	  IE2 |= UCA0RXIE;                          // Enable USCI0 RX interrupt

	count = 0;	//index of the layer frame we are on in the sequence
	while(1){

		int c; // global intensity setting, later will be an array with 25 (5x5) elements
		int r; // cycle index running from 0 to 15

		//Set the control array
		//Set slave select pin

		int SS = count % 5;
		if (SS==0){
			P2OUT |= BIT0;
			P2OUT &= ~(BIT1 + BIT2 + BIT3 + BIT4);
//			P2OUT = BIT1;
		}
		else if (SS==1){
			P2OUT |= BIT1;
			P2OUT &= ~(BIT0 + BIT2 + BIT3 + BIT4);
//			P2OUT = BIT1;
		}
		else if (SS==2){
			P2OUT |= BIT2;
			P2OUT &= ~(BIT1 + BIT0 + BIT3 + BIT4);
		}
		else if (SS == 3){
			P2OUT = BIT3;
		}
		else if (SS == 4){
			P2OUT = BIT4;
		}
		//Send layer frame
		for (r=0; r<5; r++){
			for (c=0; c<5; c++){
				//PWM all lights
				while  (!(IFG2 &   UCA0TXIFG));    //  USCI_A0 TX  buffer  ready?
				if (1){
					UCA0TXBUF = wave(r,c,SS,count % 25);
				}
				else if (!flag){
					UCA0TXBUF = pass(r,c,SS,count % 25);
				}
				else{
					UCA0TXBUF = pass2(r,c,SS,count % 25);
				}
			}
		}
		__delay_cycles(3000);				// delay before writing new byte to transmit buffer

		count++;
		if (count == 3 * 25){

			if ( ((P1IN & BIT6) == 0)){
				flag = 1 - flag;
			}
//			__delay_cycles(4000);					// delay between Cube Frames
			which_layer = (which_layer + 1) % 3;
			count = 0;
		}



	}

}


int cosine(int v){
	return 1 - (v*v)/2 + (v*v*v*v)/4;
}


int wave(int x, int y, int z, int t){

	if ((2*t/5) - 2 > ((2-x)*(2*((2-x)>0) - 1) + (2-y)*(2*((2-y)>0) - 1) + (1-z)*(2*((1-z)>0) - 1))){
		return 10;
	}
	return 0;
}

int pass(int x, int y, int z, int t){

	if (t - 2 > (x+y)){
		return 10;
	}
	return 0;
}

int pass2(int x, int y, int z, int t){

	if ((8 - (t-2)) < (x+y)){
		return 10;
	}
	return 0;
}


