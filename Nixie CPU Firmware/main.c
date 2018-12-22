/*
 * Nixie CPU Firmware.c
 *
 * Created: 1/13/2018 12:34:59 PM
 * Author : Pat
 *
 * Copyright 2018, Pat Satyshur
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */ 

#define F_CPU 16000000UL  // 16 MHz

#define LED1_PORT			PORTB
#define LED1_PIN_NUM		7
#define LED1_DDR			DDRB
#define LED1_PIN			PINB

#define LED2_PORT			PORTE
#define LED2_PIN_NUM		6
#define LED2_DDR			DDRE
#define LED2_PIN			PINE

#define HVPSU_PORT			PORTF
#define HVPSU_PIN_NUM		3
#define HVPSU_DDR			DDRF






#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "DigitDefs.h"
//#include "lcd.h"

//Internal Functions

void HVPower(uint8_t PWRState);
void LED(uint8_t LEDNumber, uint8_t state);
void BlankDigit(uint8_t DigitToBlank);
void SetDigit(uint8_t Digit, uint8_t NumberToDisplay);

void SetDP(uint8_t DPLocation, uint8_t DPState);

static inline void InitDigits(void);

//volatile uint32_t counts_val;
//volatile uint32_t seconds_val;

//char Line1[16];

volatile uint8_t Time10Hours;
volatile uint8_t TimeHours;
volatile uint8_t Time10Minutes;
volatile uint8_t TimeMinutes;
volatile uint8_t TimeSeconds;
volatile uint8_t AMPM;

volatile uint8_t UpdateDisplayRequest;

volatile uint8_t BlinkDP;
volatile uint8_t TimeMode24;

volatile uint16_t GlobalDimmingVal;
volatile uint16_t CrossfadeVal;

int main(void)
{
	//uint8_t LEDState = 1;
	//uint8_t Digit1State = 0;
	//uint8_t Digit2State = 1;
	//uint8_t Digit3State = 2;
	//uint8_t Digit4State = 3;
	
	uint8_t i;
	uint8_t DPState_T = 0;
	
	
	BlinkDP = 0;
	TimeMode24 = 0;
	
	//Initialize the time
	Time10Hours		= 0;
	TimeHours		= 8;
	Time10Minutes	= 0;
	TimeMinutes		= 1;
	TimeSeconds		= 0;
	AMPM			= 1;
	UpdateDisplayRequest = 5;	
	
	//Enable timer 1 for counting seconds
	// CTC Mode
	// 256 Divider
	// Enable interrupts
	TCCR1A = 0x00;
	TCCR1B = 0x0C;		//Enable Timer 1, CTC Mode, 256 Divider
	TCCR1C = 0x00;
	OCR1AH = 0xF4;		//Set timer top value to 62,500
	OCR1AL = 0x24;
	TIMSK1 = 0x02;		//Enable interrupt on OCRA match
	
	//Enable timer 3 for counting seconds
	// CTC Mode
	// No Divider
	// Timer top value is 50,000, timer overflow frequency is 320Hz
	// Enable interrupts
	TCCR3A = 0x00;
	TCCR3B = 0x09;		//Enable Timer 3, CTC Mode, No Divider
	TCCR3C = 0x00;
	OCR3AH = 0xC3;		//Set timer top value to 50,000
	OCR3AL = 0x50;
	OCR3BH = 0x61;		//Set timer mid value to 25,000
	OCR3BL = 0xA8;
	
	TIMSK3 = 0x06;		//Enable interrupt on OCRA and OCRB match
	
	GlobalDimmingVal = 12500;
	
	sei();				//Global interrupt enable

	//Configure all digit pins as outputs
	InitDigits();
	
	//All digit pins low, all digits off
	BlankDigit(1);
	BlankDigit(2);
	BlankDigit(3);
	BlankDigit(4);

	//Initialize LEDs
	LED(1,0);
	LED(2,0);
	
	_delay_ms(1000);
	
	//Turn on HV power supply
	HVPower(1);
	LED(1,1);
	
	//Delay to let power stabilize
	_delay_ms(1000);
	
	//Setup INT0 on PD2
	//DDRD &= ~(1<<2);	//Set PD2 as input
	//PORTD = 0x00;		//No pullups on port D
	
//	EICRA = 0x03;		//Enable INT0 on rising edge
//	EIMSK = 0x01;		//Unmask INT0
	
	//Make sure all digits light
	for(i=0;i<10;i++)
	{
		SetDigit(1, i);
		SetDigit(2, i);
		SetDigit(3, i);
		SetDigit(4, i);
		_delay_ms(100);
	}
	
	
	//SetDigit(4, 5);
	SetDigit(3, 5);
	SetDigit(2, 5);
	SetDigit(1, 5);
	
	
	//SetDigit(4, Time10Hours);
	//SetDigit(3, TimeHours);
	//SetDigit(2, Time10Minutes);
	//SetDigit(1, TimeMinutes);
	//SetDP(1,1);
	//SetDP(2,1);


    while (1) 
    {
		LED(2,1);
		//OCR3BH = (GlobalDimmingVal>>8) & 0xFF;//     0x30;
		//OCR3BL = GlobalDimmingVal & 0xFF;
		_delay_ms(5000);
		LED(2,0);
		//OCR3BH = 0x92;
		//OCR3BL = 0x7C;
		_delay_ms(5000);
		
		/*if(UpdateDisplayRequest > 0)
		{
			//Blink the decimal points every second
			if(BlinkDP == 1)
			{
				if(DPState_T == 1)
				{
					SetDP(1,1);
					SetDP(2,1);
					DPState_T = 0;
				}
				else
				{
					SetDP(1,0);
					SetDP(2,0);
					DPState_T = 1;
				}
			}
			
			//Change digits if needed
			switch (UpdateDisplayRequest)
			{
				case 1:		//seconds updated
					//Nothing to do here
					break;
					
				case 5:		//Update 10 hours digit
					SetDigit(4, Time10Hours);
	
				case 4:		//Update hours digit
					SetDigit(3, TimeHours);
					
				case 3:		//Update 10 minutes digit
					SetDigit(2, Time10Minutes);
					
				case 2:		//Update minutes digit
					SetDigit(1, TimeMinutes);
					break;		
			}
			
			UpdateDisplayRequest = 0;
		}*/
		
		/*
		LED(2,LEDState);
		SetDigit(1, Digit1State);
		SetDigit(2, Digit2State);
		SetDigit(3, Digit3State);
		SetDigit(4, Digit4State);
		
		if(DPState_T == 1)
		{
			SetDP(1,1);
			SetDP(2,1);
			DPState_T = 0;
		}
		else
		{
			SetDP(1,0);
			SetDP(2,0);
			DPState_T = 1;
		}
		
		if(LEDState == 0)
		{
			LEDState = 1;
		}
		else
		{
			LEDState = 0;
		}
		
		Digit1State++;
		if(Digit1State >= 10)
		{
			Digit1State = 0;
		}
		
		Digit2State++;
		if(Digit2State >= 10)
		{
			Digit2State = 0;
		}
		
		Digit3State++;
		if(Digit3State >= 10)
		{
			Digit3State = 0;
		}	
		
		Digit4State++;
		if(Digit4State >= 10)
		{
			Digit4State = 0;
		}
		_delay_ms(1000);
		*/
    }
}

/*void LED(uint8_t state)
{
	if(state == 1)
	{
		PORTC |= 1<<5;
	}
	else
	{
		PORTC &= ~(1<<5);
	}
}*/

/*void LED_Toggle(void)
{
	PORTC ^= 1<<5;
	return;
}*/

/*void UpdateDisplay(void)
{
	uint32_t DayVal		= 0;
	uint32_t HourVal	= 0;
	uint32_t MinVal		= 0;
	uint32_t SecVal		= 0;
	
	SecVal = seconds_val;
	
	if(SecVal >= 86400)
	{
		DayVal = SecVal/86400;
		SecVal = SecVal - (DayVal*86400);
	}
	if(SecVal >= 3600)
	{
		HourVal = SecVal/3600;
		SecVal = SecVal - (HourVal*3600);
	}
	if(SecVal >= 60)
	{
		MinVal = SecVal/60;
		SecVal = SecVal - (MinVal*60);
	}
	
	lcd_clrscr(); //clear display and home cursor 
	
	sprintf(Line1, "Counts: %ld\n", counts_val);
	lcd_puts(Line1);
	
	sprintf(Line1, "%03ld:%02ld:%02ld:%02ld\n",DayVal, HourVal, MinVal, SecVal);
	lcd_puts(Line1);
}*/

/*ISR(INT0_vect)
{
	counts_val++;
	UpdateDisplayRequest = 1;	//Tell main to update the LCD
	return;
}*/

ISR(TIMER1_COMPA_vect)
{
	//Update the time registers
	TimeSeconds++;
	if(TimeSeconds >= 60)
	{
		TimeMinutes++;
		TimeSeconds = 0;
		if(TimeMinutes >= 10)
		{
			Time10Minutes++;
			TimeMinutes = 0;
			if(Time10Minutes >= 6)
			{
				TimeHours++;
				Time10Minutes = 0;
				if(TimeHours >= 10)
				{
					Time10Hours++;
					TimeHours = 0;
					if(TimeMode24 == 1)
					{
						//24 Hour mode
						if(Time10Hours >= 3)
						{
							Time10Hours = 0;
						}
					}
					else
					{
						//AM/PM mode
						if(Time10Hours >= 2)
						{
							Time10Hours = 0;
							TimeHours = 1;
							if(AMPM == 1)
							{
								AMPM = 0;
							}
							else
							{
								AMPM = 1;
							}
						}
					}
					UpdateDisplayRequest = 5;	//Tell main to update 10 hours digit
					return;
				}
				UpdateDisplayRequest = 4;	//Tell main to update hours digit
				return;
			}
			UpdateDisplayRequest = 3;	//Tell main to update 10 minutes digit
			return;
		}
		UpdateDisplayRequest = 2;	//Tell main to update minutes digit
		return;
	}
	UpdateDisplayRequest = 1;	//Tell main to update seconds
	return;
}

ISR(TIMER3_COMPA_vect)
{
	SetDigit(4, 5);
	return;
}

ISR(TIMER3_COMPB_vect)
{
	SetDigit(4, 10);
	
	GlobalDimmingVal = GlobalDimmingVal+100;
	if(GlobalDimmingVal > 45000)
	{
		GlobalDimmingVal = 1000;
	}
	
	OCR3BH = (GlobalDimmingVal>>8) & 0xFF;
	OCR3BL = GlobalDimmingVal & 0xFF;
	return;
}

static inline void InitDigits(void)
{
	//Configure all digit pins as outputs
	
	//Digit 1
	DIGIT_1_0_DDR |= 1<<DIGIT_1_0_NUM;
	DIGIT_1_1_DDR |= 1<<DIGIT_1_1_NUM;
	DIGIT_1_2_DDR |= 1<<DIGIT_1_2_NUM;
	DIGIT_1_3_DDR |= 1<<DIGIT_1_3_NUM;
	DIGIT_1_4_DDR |= 1<<DIGIT_1_4_NUM;
	DIGIT_1_5_DDR |= 1<<DIGIT_1_5_NUM;
	DIGIT_1_6_DDR |= 1<<DIGIT_1_6_NUM;
	DIGIT_1_7_DDR |= 1<<DIGIT_1_7_NUM;
	DIGIT_1_8_DDR |= 1<<DIGIT_1_8_NUM;
	DIGIT_1_9_DDR |= 1<<DIGIT_1_9_NUM;
	
	//Digit 2
	DIGIT_2_0_DDR |= 1<<DIGIT_2_0_NUM;
	DIGIT_2_1_DDR |= 1<<DIGIT_2_1_NUM;
	DIGIT_2_2_DDR |= 1<<DIGIT_2_2_NUM;
	DIGIT_2_3_DDR |= 1<<DIGIT_2_3_NUM;
	DIGIT_2_4_DDR |= 1<<DIGIT_2_4_NUM;
	DIGIT_2_5_DDR |= 1<<DIGIT_2_5_NUM;
	DIGIT_2_6_DDR |= 1<<DIGIT_2_6_NUM;
	DIGIT_2_7_DDR |= 1<<DIGIT_2_7_NUM;
	DIGIT_2_8_DDR |= 1<<DIGIT_2_8_NUM;
	DIGIT_2_9_DDR |= 1<<DIGIT_2_9_NUM;
	
	//Digit 3
	DIGIT_3_0_DDR |= 1<<DIGIT_3_0_NUM;
	DIGIT_3_1_DDR |= 1<<DIGIT_3_1_NUM;
	DIGIT_3_2_DDR |= 1<<DIGIT_3_2_NUM;
	DIGIT_3_3_DDR |= 1<<DIGIT_3_3_NUM;
	DIGIT_3_4_DDR |= 1<<DIGIT_3_4_NUM;
	DIGIT_3_5_DDR |= 1<<DIGIT_3_5_NUM;
	DIGIT_3_6_DDR |= 1<<DIGIT_3_6_NUM;
	DIGIT_3_7_DDR |= 1<<DIGIT_3_7_NUM;
	DIGIT_3_8_DDR |= 1<<DIGIT_3_8_NUM;
	DIGIT_3_9_DDR |= 1<<DIGIT_3_9_NUM;
	
	//Digit 4
	DIGIT_4_0_DDR |= 1<<DIGIT_4_0_NUM;
	DIGIT_4_1_DDR |= 1<<DIGIT_4_1_NUM;
	DIGIT_4_2_DDR |= 1<<DIGIT_4_2_NUM;
	DIGIT_4_3_DDR |= 1<<DIGIT_4_3_NUM;
	DIGIT_4_4_DDR |= 1<<DIGIT_4_4_NUM;
	DIGIT_4_5_DDR |= 1<<DIGIT_4_5_NUM;
	DIGIT_4_6_DDR |= 1<<DIGIT_4_6_NUM;
	DIGIT_4_7_DDR |= 1<<DIGIT_4_7_NUM;
	DIGIT_4_8_DDR |= 1<<DIGIT_4_8_NUM;
	DIGIT_4_9_DDR |= 1<<DIGIT_4_9_NUM;
	
	return;
}

void HVPower(uint8_t PWRState)
{
	HVPSU_DDR |= 1<<HVPSU_PIN_NUM;
	if(PWRState == 1)
	{
		HVPSU_PORT |= 1<<HVPSU_PIN_NUM;
	}
	else
	{
		HVPSU_PORT &= ~(1<<HVPSU_PIN_NUM);
	}
	return;
}

void LED(uint8_t LEDNumber, uint8_t state)
{
	LED1_DDR |= 1<<LED1_PIN_NUM;
	LED2_DDR |= 1<<LED2_PIN_NUM;
	
	if(LEDNumber == 1)
	{
		if(state == 0)
		{
			LED1_PORT &= ~(1<<LED1_PIN_NUM);
		}
		else if (state == 1)
		{
			LED1_PORT |= 1<<LED1_PIN_NUM;
		}
		else
		{
			LED1_PORT |= 1<<LED1_PIN_NUM;
		}
	}
	else if(LEDNumber == 2)
	{
		if(state == 0)
		{
			LED2_PORT &= ~(1<<LED2_PIN_NUM);
		}
		else if (state == 1)
		{
			LED2_PORT |= 1<<LED2_PIN_NUM;
		}
		else
		{
			LED2_PORT |= 1<<LED2_PIN_NUM;
		}
	}
	else if(LEDNumber == 3)
	{
		//All on
		LED1_PORT |= 1<<LED1_PIN_NUM;
		LED2_PORT |= 1<<LED2_PIN_NUM;
		
	}
	else
	{
		LED1_PORT &= ~(1<<LED1_PIN_NUM);
		LED2_PORT &= ~(1<<LED2_PIN_NUM);
	}
	return;
}

void BlankDigit(uint8_t DigitToBlank)
{
	if(DigitToBlank == 1)
	{
		DIGIT_1_0_PORT &= ~(1<<DIGIT_1_0_NUM);
		DIGIT_1_1_PORT &= ~(1<<DIGIT_1_1_NUM);
		DIGIT_1_2_PORT &= ~(1<<DIGIT_1_2_NUM);
		DIGIT_1_3_PORT &= ~(1<<DIGIT_1_3_NUM);
		DIGIT_1_4_PORT &= ~(1<<DIGIT_1_4_NUM);
		DIGIT_1_5_PORT &= ~(1<<DIGIT_1_5_NUM);
		DIGIT_1_6_PORT &= ~(1<<DIGIT_1_6_NUM);
		DIGIT_1_7_PORT &= ~(1<<DIGIT_1_7_NUM);
		DIGIT_1_8_PORT &= ~(1<<DIGIT_1_8_NUM);
		DIGIT_1_9_PORT &= ~(1<<DIGIT_1_9_NUM);
	}
	else if(DigitToBlank == 2)
	{
		DIGIT_2_0_PORT &= ~(1<<DIGIT_2_0_NUM);
		DIGIT_2_1_PORT &= ~(1<<DIGIT_2_1_NUM);
		DIGIT_2_2_PORT &= ~(1<<DIGIT_2_2_NUM);
		DIGIT_2_3_PORT &= ~(1<<DIGIT_2_3_NUM);
		DIGIT_2_4_PORT &= ~(1<<DIGIT_2_4_NUM);
		DIGIT_2_5_PORT &= ~(1<<DIGIT_2_5_NUM);
		DIGIT_2_6_PORT &= ~(1<<DIGIT_2_6_NUM);
		DIGIT_2_7_PORT &= ~(1<<DIGIT_2_7_NUM);
		DIGIT_2_8_PORT &= ~(1<<DIGIT_2_8_NUM);
		DIGIT_2_9_PORT &= ~(1<<DIGIT_2_9_NUM);
	}
	else if(DigitToBlank == 3)
	{
		DIGIT_3_0_PORT &= ~(1<<DIGIT_3_0_NUM);
		DIGIT_3_1_PORT &= ~(1<<DIGIT_3_1_NUM);
		DIGIT_3_2_PORT &= ~(1<<DIGIT_3_2_NUM);
		DIGIT_3_3_PORT &= ~(1<<DIGIT_3_3_NUM);
		DIGIT_3_4_PORT &= ~(1<<DIGIT_3_4_NUM);
		DIGIT_3_5_PORT &= ~(1<<DIGIT_3_5_NUM);
		DIGIT_3_6_PORT &= ~(1<<DIGIT_3_6_NUM);
		DIGIT_3_7_PORT &= ~(1<<DIGIT_3_7_NUM);
		DIGIT_3_8_PORT &= ~(1<<DIGIT_3_8_NUM);
		DIGIT_3_9_PORT &= ~(1<<DIGIT_3_9_NUM);
	}
	else if(DigitToBlank == 4)
	{
		DIGIT_4_0_PORT &= ~(1<<DIGIT_4_0_NUM);
		DIGIT_4_1_PORT &= ~(1<<DIGIT_4_1_NUM);
		DIGIT_4_2_PORT &= ~(1<<DIGIT_4_2_NUM);
		DIGIT_4_3_PORT &= ~(1<<DIGIT_4_3_NUM);
		DIGIT_4_4_PORT &= ~(1<<DIGIT_4_4_NUM);
		DIGIT_4_5_PORT &= ~(1<<DIGIT_4_5_NUM);
		DIGIT_4_6_PORT &= ~(1<<DIGIT_4_6_NUM);
		DIGIT_4_7_PORT &= ~(1<<DIGIT_4_7_NUM);
		DIGIT_4_8_PORT &= ~(1<<DIGIT_4_8_NUM);
		DIGIT_4_9_PORT &= ~(1<<DIGIT_4_9_NUM);
	}
	return;
}

void SetDigit(uint8_t Digit, uint8_t NumberToDisplay)
{
	switch (Digit)
	{
		//Digit 1
		case 1:
			BlankDigit(1);
			switch (NumberToDisplay)
			{
				case 0:
					DIGIT_1_0_PORT |= (1<<DIGIT_1_0_NUM);
					break;
				
				case 1:
					DIGIT_1_1_PORT |= (1<<DIGIT_1_1_NUM);
					break;
				
				case 2:
					DIGIT_1_2_PORT |= (1<<DIGIT_1_2_NUM);
					break;
				
				case 3:
					DIGIT_1_3_PORT |= (1<<DIGIT_1_3_NUM);
					break;
				
				case 4:
					DIGIT_1_4_PORT |= (1<<DIGIT_1_4_NUM);
					break;
				
				case 5:
					DIGIT_1_5_PORT |= (1<<DIGIT_1_5_NUM);
					break;
				
				case 6:
					DIGIT_1_6_PORT |= (1<<DIGIT_1_6_NUM);
					break;
				
				case 7:
					DIGIT_1_7_PORT |= (1<<DIGIT_1_7_NUM);
					break;
				
				case 8:
					DIGIT_1_8_PORT |= (1<<DIGIT_1_8_NUM);
					break;
				
				case 9:
					DIGIT_1_9_PORT |= (1<<DIGIT_1_9_NUM);
					break;
			}
			break;
			
		//Digit 2
		case 2:
			BlankDigit(2);
			switch (NumberToDisplay)
			{
				case 0:
					DIGIT_2_0_PORT |= (1<<DIGIT_2_0_NUM);
					break;
				
				case 1:
					DIGIT_2_1_PORT |= (1<<DIGIT_2_1_NUM);
					break;
				
				case 2:
					DIGIT_2_2_PORT |= (1<<DIGIT_2_2_NUM);
					break;
				
				case 3:
					DIGIT_2_3_PORT |= (1<<DIGIT_2_3_NUM);
					break;
				
				case 4:
					DIGIT_2_4_PORT |= (1<<DIGIT_2_4_NUM);
					break;
				
				case 5:
					DIGIT_2_5_PORT |= (1<<DIGIT_2_5_NUM);
					break;
				
				case 6:
					DIGIT_2_6_PORT |= (1<<DIGIT_2_6_NUM);
					break;
				
				case 7:
					DIGIT_2_7_PORT |= (1<<DIGIT_2_7_NUM);
					break;
				
				case 8:
					DIGIT_2_8_PORT |= (1<<DIGIT_2_8_NUM);
					break;
				
				case 9:
					DIGIT_2_9_PORT |= (1<<DIGIT_2_9_NUM);
					break;
			}
			break;
			
		//Digit 3
		case 3:
			BlankDigit(3);
			switch (NumberToDisplay)
			{
				case 0:
					DIGIT_3_0_PORT |= (1<<DIGIT_3_0_NUM);
					break;
				
				case 1:
					DIGIT_3_1_PORT |= (1<<DIGIT_3_1_NUM);
					break;
				
				case 2:
					DIGIT_3_2_PORT |= (1<<DIGIT_3_2_NUM);
					break;
				
				case 3:
					DIGIT_3_3_PORT |= (1<<DIGIT_3_3_NUM);
					break;
				
				case 4:
					DIGIT_3_4_PORT |= (1<<DIGIT_3_4_NUM);
					break;
				
				case 5:
					DIGIT_3_5_PORT |= (1<<DIGIT_3_5_NUM);
					break;
				
				case 6:
					DIGIT_3_6_PORT |= (1<<DIGIT_3_6_NUM);
					break;
				
				case 7:
					DIGIT_3_7_PORT |= (1<<DIGIT_3_7_NUM);
					break;
				
				case 8:
					DIGIT_3_8_PORT |= (1<<DIGIT_3_8_NUM);
					break;
				
				case 9:
					DIGIT_3_9_PORT |= (1<<DIGIT_3_9_NUM);
					break;
			}
			break;
			
		//Digit 4
		case 4:
			BlankDigit(4);
			switch (NumberToDisplay)
			{
				case 0:
					DIGIT_4_0_PORT |= (1<<DIGIT_4_0_NUM);
					break;
				
				case 1:
					DIGIT_4_1_PORT |= (1<<DIGIT_4_1_NUM);
					break;
				
				case 2:
					DIGIT_4_2_PORT |= (1<<DIGIT_4_2_NUM);
					break;
				
				case 3:
					DIGIT_4_3_PORT |= (1<<DIGIT_4_3_NUM);
					break;
				
				case 4:
					DIGIT_4_4_PORT |= (1<<DIGIT_4_4_NUM);
					break;
				
				case 5:
					DIGIT_4_5_PORT |= (1<<DIGIT_4_5_NUM);
					break;
				
				case 6:
					DIGIT_4_6_PORT |= (1<<DIGIT_4_6_NUM);
					break;
				
				case 7:
					DIGIT_4_7_PORT |= (1<<DIGIT_4_7_NUM);
					break;
				
				case 8:
					DIGIT_4_8_PORT |= (1<<DIGIT_4_8_NUM);
					break;
				
				case 9:
					DIGIT_4_9_PORT |= (1<<DIGIT_4_9_NUM);
					break;
			}
			break;
	}
	
	return;
}

void SetDP(uint8_t DPLocation, uint8_t DPState)
{
	if(DPLocation == 1)
	{
		if(DPState == 1)
		{
			DP_TOP_PORT |= (1<<DP_TOP_NUM);
		}
		else
		{
			DP_TOP_PORT &= ~(1<<DP_TOP_NUM);
		}
	}
	
	else if(DPLocation == 2)
	{
		if(DPState == 1)
		{
			DP_BOTTOM_PORT |= (1<<DP_BOTTOM_NUM);
		}
		else
		{
			DP_BOTTOM_PORT &= ~(1<<DP_BOTTOM_NUM);
		}
	}
	
	return;
}