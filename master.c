/*
 * PWM Control of an LED Matrix layer.
 */

#include <msp430g2553.h>		//must include so compiler knows what each variable means

const int led_num_cycles = 12;
const char col_array[5] = {BIT0, BIT3, 0x00, BIT6, BIT7};
const char row_array[5] = {BIT0, BIT1, BIT2, BIT3, BIT4};
char pwm_array[5][5] = {
		{10,		0,		0,		0,		0},
		{0,			10,		0,		0,		0},
		{0,			0,		 10, 	0,		0},
		{0,			0,		 0,		10,		0},
		{0,			0,		 0,		0,		10}
}; //row,col. Elements go from 0 to led_num_cycles

void set_column(int c, int j);
void clear_all();

void main(void){

	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	P1OUT &= (BIT0 + BIT3 + BIT6 + BIT7);
	P1DIR |= (BIT0 + BIT3 + BIT6 + BIT7);

	P2OUT &= (BIT5 + BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
	P2DIR |= (BIT5 + BIT0 + BIT1 + BIT2 + BIT3 + BIT4);

	BCSCTL3 |= LFXT1S_2;					// Set clock source to VLO
	BCSCTL2 |= SELM_3 + SELS;          		// SMCLK  = MCLK = VLO = 12KHz

	P1SEL  =   BIT2    |   BIT4;	//enable UCA0 transmit and clock
	P1SEL2 =   BIT2    |   BIT4;

	UCA0CTL1   =   UCSWRST;
	UCA0CTL0   |=  UCCKPH  +   UCMSB   +   UCMST   +   UCSYNC + UCMODE0; //  3-pin,  8-bit   SPI master
	UCA0CTL1   |=  UCSSEL_1;   //  ACLK
	UCA0BR0    |=  0x00;   //don't prescale Baud rate
	UCA0BR1    =   0;  //don't prescale Baud rate
	UCA0MCTL   =   0;  //  No  modulation
	UCA0CTL1   &=  ~UCSWRST;   //  **Initialize    USCI    state   machine**

	while(1){

		int c; // global intensity setting, later will be an array with 25 (5x5) elements
		int r; // cycle index running from 0 to 15
		int k;

		// set the control array
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

//		pwm_array[0][1] = 10 - pwm_array[0][1];
		// set the led matrix to a state for 1000 iterations

		int index;
		for (index=0; index<5;index++){
			for (k=0;k<1; k++){
				for (c=0; c<5; c++){
					for (r=0; r<5; r++){
						//PWM all lights

						while  (!(IFG2 &   UCA0TXIFG));    //  USCI_A0 TX  buffer  ready?
						UCA0TXBUF  =   pwm_array[r][c];   //  Send data byte  over SPI to  Slave. Would use UCA0RXBUF to receive.

		//				_BIS_SR(LPM3_bits);	//enter low-power mode.
					}
				}
			}
		}
	}

}




