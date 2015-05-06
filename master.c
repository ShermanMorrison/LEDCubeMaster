/*
 * PWM Control of an LED Matrix layer.
 */

#include <msp430g2553.h>		//must include so compiler knows what each variable means

const int led_num_cycles = 12;
const char col_array[5] = {BIT0, BIT3, 0x00, BIT6, BIT7};
const char row_array[5] = {BIT0, BIT1, BIT2, BIT3, BIT4};
char pwm_array[5][5] = {
		{10,		0,		0,		0,		0},
		{0,		10,		0,		0,		0},
		{0,		0,		 10, 	0,		0},
		{0,		0,		 0,		10,		0},
		{0,		0,		 0,		0,		10}
}; //Layer 0 row,col. Elements go from 0 to led_num_cycles

char pwm_array_2[5][5] = {
		{0,			0,		0,		0,		10},
		{0,			0,		0,		10,		0},
		{0,			0,		 10, 	0,		0},
		{0,			10,		 0,		0,		0},
		{10,			0,		 0,		0,		0}
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

	UCA0CTL0 |= UCCKPL + UCMSB + UCMST + UCSYNC + UCMODE_1;  // 3-pin, 8-bit SPI master
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 |= 0x02;                          // /2
	UCA0BR1 = 0;                              //
	UCA0MCTL = 0;                             // No modulation
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	//	  IE2 |= UCA0RXIE;                          // Enable USCI0 RX interrupt

	int count = 0;	//index of the layer frame we are on in the sequence
	while(1){

		int c; // global intensity setting, later will be an array with 25 (5x5) elements
		int r; // cycle index running from 0 to 15

		// set the control array

		//downwards rotate layer 0 array columns
		if (count % 2== 0){
			int row,col;
			int nextRow [5];
			for (col=0;col<5;col++)
				nextRow[col] = pwm_array[4][col];
			for (row=4;row>0;row--){
				for (col=0;col<5;col++)
					pwm_array[row][col] = pwm_array[row-1][col];
			}
			for (col=0;col<5;col++)
				pwm_array[0][col] = nextRow[col];
		}
		//downwards rotate layer 0 array columns
		if (count % 2== 1){
			int row,col;
			int nextRow [5];
			for (col=0;col<5;col++)
				nextRow[col] = pwm_array_2[4][col];
			for (row=4;row>0;row--){
				for (col=0;col<5;col++)
					pwm_array_2[row][col] = pwm_array_2[row-1][col];
			}
			for (col=0;col<5;col++)
				pwm_array_2[0][col] = nextRow[col];
		}

		//Set slave select pin
		if (count % 2 == 0){
			P2OUT |= BIT0;
			P2OUT &= ~BIT1;
		}
		else{
			P2OUT |= BIT1;
			P2OUT &= ~BIT0;
		}

		//Send layer frame
		for (r=0; r<5; r++){
			for (c=0; c<5; c++){
				//PWM all lights
				while  (!(IFG2 &   UCA0TXIFG));    //  USCI_A0 TX  buffer  ready?
				if (count % 2 == 1){
					UCA0TXBUF = pwm_array_2[r][c];	// send layer 1
				}
				else{
					UCA0TXBUF = pwm_array[r][c];	// send layer 0
				}

				__delay_cycles(1000);				// delay before writing new byte to transmit buffer
			}
		}

		if (count % 2 == 1){
			__delay_cycles(10);					// delay between Cube Frames
		}

		count = (count + 1) % 20;					// increment frame index

	}

}




