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

//#define F_CPU 16000000UL  // 16 MHz

#include "main.h"
#include <util/usa_dst.h>

#if BACK_BOARD_PRESENT == 0
#define LED1_PORT			PORTB
#define LED1_PIN_NUM		7
#define LED1_DDR			DDRB
#define LED1_PIN			PINB

#define LED2_PORT			PORTE
#define LED2_PIN_NUM		6
#define LED2_DDR			DDRE
#define LED2_PIN			PINE

#define LED_GREEN				1
#define LED_BLUE				2
#define LED_RED					3

#else
//Use these in the code to turn on LEDs
#define LED_RED					1
#define LED_GREEN				2
#define LED_BLUE				3
//Used internally by the LED function
#define LED_RED_BIT				0x02
#define LED_GREEN_BIT			0x08
#define LED_BLUE_BIT			0x04
#define LED_MASK				0x0E
#endif

#define HVPSU_PORT			PORTF
#define HVPSU_PIN_NUM		3
#define HVPSU_DDR			DDRF

#define IO_EXPANDER_INT_PORT		PORTB
#define IO_EXPANDER_INT_PIN_NUM		7
#define IO_EXPANDER_INT_DDR			DDRB
#define IO_EXPANDER_INT_PIN			PINB //TODO: do I need this?

#define IO_EXPANDER_RESET_PORT			PORTF
#define IO_EXPANDER_RESET_PIN_NUM		6
#define IO_EXPANDER_RESET_DDR			DDRF
#define IO_EXPANDER_RESET_PIN			PINF //TODO: do I need this?

#define IO_EXPANDER_BUTTON_MASK		0xF1

#define GPS_PWR_PORT			PORTF
#define GPS_PWR_PIN_NUM			7
#define GPS_PWR_DDR				DDRF
#define GPS_PWR_PIN				PINF	//TODO: do I need this?

void HandleButtonPress(void);
static inline void InitDigits(void);
void SetDigit_Int(uint8_t Digit, uint8_t NumberToDisplay, uint8_t UpdateReg);

//Non-volatile variables saved in EEPROM
uint8_t EEMEM NV_TimeZone;			//Timezone to apply to the UTC time. Expressed in hours to be added to the UTC time
uint32_t EEMEM NV_LatInSeconds;		//Latitude in arcseconds
uint32_t EEMEM NV_LongInSeconds;	//Longitude in arcseconds
uint8_t EEMEM NV_DimVal;			//Dimming value TODO: how to apply this??
uint8_t EEMEM NV_ClockSetup;		//Bits to set up clock. TODO: Use these somewhere for AM/PM, leading zero, etc..



uint8_t OldButtonState;		//Global variable to save the old button state
uint8_t ButtonStatus;		//Set a bit to one in this register to tell the button handler to handle the corresponding button

volatile uint8_t UpdateDisplayRequest;

volatile uint8_t BlinkDP;
volatile uint8_t TimeMode24;

volatile uint16_t GlobalDimmingVal;
volatile uint16_t CrossfadeVal;



//Status of the Digits
// 0 - Fixed digits, can be updated by the user
// 1 - Automatic updating, showing the time and stuff
volatile uint8_t DigitStatus;


//Global status bits. These are private to this function, and should only be changed using functions.
volatile uint8_t ClockStatus;
volatile uint8_t SystemStatus;
volatile uint8_t DisplaySetup;

//TODO: Make this an array?
volatile uint8_t Digit1State;
volatile uint8_t Digit2State;
volatile uint8_t Digit3State;
volatile uint8_t Digit4State;
volatile uint8_t DP1State;
volatile uint8_t DP2State;

//volatile uint8_t Digit1Statei;
//volatile uint8_t Digit2Statei;
//volatile uint8_t Digit3Statei;
//volatile uint8_t Digit4Statei;

int main(void)
{
	uint8_t StatusByte;
	//uint8_t StatusByte2;
	struct tm TheTime;
	//int32_t ZoneVal;
	time_t TimeVal;


	//Needed for GPS reads. TODO: Remove these once the GPS subfunction is ready.
	//char GPGGA_STRING[155];
	//char GPRMC_STRING[155];
	//int32_t LatSec;
	//int32_t LongSec;

	Digit1State = 0;
	Digit2State = 0;
	Digit3State = 0;
	Digit4State = 0;
	
	DP1State = 1;
	DP2State = 1;
	
	//Initialize global variables - Which of these do I need???
	OldButtonState	= 0;
	ButtonStatus	= 0;
	uint8_t i;
	//uint8_t DPState_T = 0;
	DigitStatus = 0;
	BlinkDP = 0;
	TimeMode24 = TIME_MODE_AMPM;
	UpdateDisplayRequest = 5;
	
	DisplaySetup = (TIME_MODE_AMPM | CLOCK_HIDE_LEADING_ZERO);	//TODO: Make this defined in main.h or config.h

	
	
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
	
	SetSystemStatus(0x00);
	SetClockStatus(CLOCK_STATUS_INITIALIZING);
	
	//Initialize Subsystem control. These functions control GPIOs on the CPU.
	HVPower(0);		//HV power supply is off when line is low
	GPSPower(1);	//GPS power is off when line is low
	IOExpReset(1);	//IO Expander is reset when line is low. TODO: Invert this so that reset=1 means reset??
	
	//Initialize USART1 for communication
	UARTinit();					//Initialize UART1, UART number is defined in config.h
	stdout = &UART_stdout;		//Needed to use printf
	UCSR1B |= (1<<RXCIE1);		//Enable receive interrupt
	
	//Initialize TWI
	//Set up reset pin
	//TODO: add error checking and status registers for this stuff...
	InitTWI();
	
	//Enable timer 3 for dimming?
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

	//Wait for secondary stuff to initialize
	//_delay_ms(400);

	//Configure all digit pins as outputs
	InitDigits();
	
	//All digit pins low, all digits off
	BlankDigit(1);
	BlankDigit(2);
	BlankDigit(3);
	BlankDigit(4);
	
	_delay_ms(200);
	
	//Turn on HV power supply
	HVPower(1);
	
	//Delay to let power stabilize
	_delay_ms(200);
	
	//Make sure all digits light
	for(i=0;i<10;i++)
	{
		SetDigit(1, i);
		SetDigit(2, i);
		SetDigit(3, i);
		SetDigit(4, i);
		_delay_ms(500);
	}
	
	for(i=10;i>0;i--)
	{
		SetDigit(1, i-1);
		SetDigit(2, i-1);
		SetDigit(3, i-1);
		SetDigit(4, i-1);
		_delay_ms(500);
	}
	
	BlankDigit(1);
	BlankDigit(2);
	BlankDigit(3);
	BlankDigit(4);
	
	SetDP(1, 1);
	SetDP(2, 1);
	
	//Look for devices on the TWI bus. If devices are found, set the correct status in the global registers, and initialize the device.
	if(TWIDeviceCheck(DS3231M_SLA_ADDRESS) == 1)
	{
		//RTC detected. The backer board is present
		SetSystemStatus(SYSTEM_STATUS_RTC_PRESENT);
			
		//Look for valid time from the RTC
		if( DS3231M_Init() == 0 )
		{
			SetSystemStatus( GetSystemStatus() | SYSTEM_STATUS_RTC_VALID);
		}

		if(TWIDeviceCheck(TCA9538_SLA_ADDR_1) == 1)
		{
			//IO expander detected. The button board is present
			SetSystemStatus( GetSystemStatus() | SYSTEM_STATUS_BUTTON_BOARD_PRESENT);
				
			//Set up the button Board
			TCA9538_WriteReg(1, TCA9538_REG_OUTPUT, 0x00);		//Set output register to all low.
			TCA9538_WriteReg(1, TCA9538_REG_CONFIG, 0xF1);		//Set up pin 1, 2, and 3 as output, all others to input
			PCICR = (1<<0);										//Enable PCI0 interrupts from the IO expander
			PCMSK0 = (1<<7);									//Unmask PCINT7 interrupts from IO expander
		}
			
		if(TWIDeviceCheck(GPS_TWI_ADDRESS) == 1)
		{
			//GPS is present
			SetSystemStatus( GetSystemStatus() | SYSTEM_STATUS_GPS_PRESENT);
			//GPS Init code goes here...
			//TODO: GPS LOCK does not make sense as a system status. The GPS lock may be lost without the CPU knowing...
			
			
			//Set SYSTEM_STATUS_GPS_LOCK if the GPS is locked
		}
	}
	else
	{
		//No RTC present, assume the backer board is not present. If there is no backer board, there is no button board or GPS.
		SetSystemStatus(0x00);
	}
		
	//Set the status LED to indicate the device is initializing
	LED(LED_BLUE, 1);		//Turn on the blue LED, indicate that we are initalizing the device. TODO: Handle if the backer board is not installed...
		
	//Initialize the system based on the detected configuration
	//Treat the GPS as the authoritative time source if it is present
	//	-If no valid time sources are available (RTC not present, RTC present but not valid, GPS not present, GPS present but not locked)
	//		-Set a fake system time and location
	//		-Enter standby mode
	//	-If RTC is present and valid, GPS is absent, or not locked.
	//		-Set system time from RTC
	//		-Set location from default in main.h
	//		-Enter clock mode
	//	-If GPS is present and locked, RTC is not present
	//		-Set system time from GPS
	//		-Set location from GPS
	//		-Enter clock mode
	//	-if GPS is present and locked, RTC is present
	//		-Set system time from GPS
	//		-Set RTC time from GPS (do this even if the RTC data is valid)
	//		-Set location from GPS
	//		-Enter clock mode
	StatusByte = GetSystemStatus();
	
	
	
	/*if( ((StatusByte & SYSTEM_STATUS_GPS_PRESENT) == SYSTEM_STATUS_GPS_PRESENT) )//&& ((StatusByte & SYSTEM_STATUS_GPS_LOCK) == SYSTEM_STATUS_GPS_LOCK) )
	{
		//GPS is present, check for lock and set time if GPS is locked.
		//TODO: GPS LOCK does not make sense as a system status. The GPS lock may be lost without the CPU knowing...
		StatusByte = GPS_GetStrings(GPGGA_STRING, GPRMC_STRING);		//TODO: Make this a subfucntion. What happens when calling this if the GPS is not present?
		if(StatusByte == 0)
		{
			StatusByte = GPS_ValidateRMC(GPRMC_STRING, &TheTime, &LatSec, &LongSec);
			if(StatusByte == 0)
			{
				//Set the system time and RTC based on the GPS
				SetSystemTime(&TheTime, GetTimeZone(), LatSec, LongSec);
				SetSystemStatus( GetSystemStatus() | SYSTEM_STATUS_RTC_VALID);
				SetClockStatus(CLOCK_STATUS_NORMAL);		//TODO: right now this is set in SetSystemTime(). I probably want to move it out of that function.
			}
		}
	}*/
	
	if(GetTimeFromGPS() == 0)	
	{
		//Check for a valid GPS lock and sets the time from GPS if available.
		SetSystemStatus( GetSystemStatus() | SYSTEM_STATUS_RTC_VALID);
		SetClockStatus(CLOCK_STATUS_NORMAL);		//TODO: right now this is set in SetSystemTime(). I probably want to move it out of that function.
	}
	else
	{
		StatusByte = GetSystemStatus();
		if( ((StatusByte & SYSTEM_STATUS_RTC_PRESENT) == SYSTEM_STATUS_RTC_PRESENT) && ((StatusByte & SYSTEM_STATUS_RTC_VALID) == SYSTEM_STATUS_RTC_VALID) )
		{
			//RTC is present and valid
			//GPS time is not available
			//RTC time is valid. Set the internal time from the RTC time.
			DS3231M_GetTime(&TheTime);
			SetSystemTime(&TheTime, GetTimeZone(), GetLat(), GetLong());
			SetClockStatus(CLOCK_STATUS_NORMAL);	//TODO: right now this is set in SetSystemTime(). I probably want to move it out of that function.
		}
		else
		{
			//No valid time source
			//Time set to 00:00:00 on 01/01/1900
			TheTime.tm_hour		= 0;
			TheTime.tm_min		= 0;
			TheTime.tm_sec		= 0;
			TheTime.tm_mday		= 1;
			TheTime.tm_mon		= 0;
			TheTime.tm_year		= 0;
			TheTime.tm_isdst	= 0;
			SetSystemTime(&TheTime, GetTimeZone(), GetLat(), GetLong());
			SetClockStatus(CLOCK_STATUS_DEMO);		//TODO: Should I get rid of demo, and make it display the fake time??
		}
	}
	
	//Turn on green LED to indicate that the device is done initializing. TODO: Handle the backerboard not installed case. This should be done inside the LED function.
	LED(LED_BLUE, 0);
	LED(LED_GREEN, 1);
	
	if(GetClockStatus() == CLOCK_STATUS_NORMAL)
	{
		//Show the current time on the display
		UpdateDisplay();
	}
	
	//Initialize the minutes into StatusByte. This will let us make the update code only run once per update.
	time(&TimeVal);
	localtime_r(&TimeVal, &TheTime);
	StatusByte = TheTime.tm_min;

	while (1)
	{
		RunCommand();			//Look for commands from the command interpreter and run them
		HandleButtonPress();	//Look for button presses and handle them
		
		//Long time scale scheduler
		//	-Update the display every min
		//	-Check for GPS lock and update times
		//	-Sync system time with RTC or GPS
		//
		time(&TimeVal);
		localtime_r(&TimeVal, &TheTime);
		
		if( (TheTime.tm_sec == 0) && (StatusByte != TheTime.tm_min) )
		{
			StatusByte = TheTime.tm_min;
			if(GetClockStatus() == CLOCK_STATUS_NORMAL)
			{
				UpdateDisplay();
			}
			else if(GetClockStatus() == CLOCK_STATUS_DEMO)
			{
				if(GetTimeFromGPS() == 0)
				{
					//Check for a valid GPS lock and sets the time from GPS if available.
					SetSystemStatus( GetSystemStatus() | SYSTEM_STATUS_RTC_VALID);
					SetClockStatus(CLOCK_STATUS_NORMAL);		//TODO: right now this is set in SetSystemTime(). I probably want to move it out of that function.
					UpdateDisplay();
				}
			}
		}
		
		if(GetClockStatus() == CLOCK_STATUS_DEMO)
		{
			if(i<10)
			{
				i++;
			}
			else
			{
				i=0;
			}
			SetDigit(1, i);
			SetDigit(2, 10-i);
			SetDigit(3, i);
			SetDigit(4, 10-i);
			_delay_ms(400);
		}
	}
}

//TODO: Fix the definitions of left and right.
void HandleButtonPress(void)
{
	if(ButtonStatus > 0)
	{
		//printf("Button Status: 0x%02X\n", ButtonStatus);
		if(ButtonStatus == 0x01)
		{
			printf("Down\n");
		}
		else if(ButtonStatus == 0x10)
		{
			printf("Left\n");
		}
		else if(ButtonStatus == 0x20)
		{
			printf("Push\n");
		}
		else if(ButtonStatus == 0x40)
		{
			printf("Right\n");
		}
		else if(ButtonStatus == 0x80)
		{
			printf("Up\n");
		}
		ButtonStatus = 0;
	}
	return;
}

//Look for button presses from IO expander
ISR(PCINT0_vect)
{
	uint8_t TWI_Stat;
	uint8_t NewButtonStatus;
	
	//printf("button\n");
	TWI_Stat = TCA9538_ReadReg(1, TCA9538_REG_INPUT, &NewButtonStatus);
	
	if(TWI_Stat == 0)
	{
		//Read from I/O expander was successful.
		ButtonStatus = (NewButtonStatus & IO_EXPANDER_BUTTON_MASK);
	}
	else
	{
		//Failed to read from I/O expander, report no button presses.
		ButtonStatus = 0x00;
	}
	return;
}

ISR(USART1_RX_vect)
{
	CommandGetInputChar(UDR1);
	return;
}

//This ISR keeps track of the time
ISR(TIMER1_COMPA_vect, ISR_NAKED)
{
	system_tick();
	reti();
}

ISR(TIMER3_COMPA_vect)
{
	//if(ClockStatus == CLOCK_STATUS_STANDBY)
	//{
		if(Digit1State < 10) SetDigit_Int(1, Digit1State, 0);
		if(Digit2State < 10) SetDigit_Int(2, Digit2State, 0);
		if(Digit3State < 10) SetDigit_Int(3, Digit3State, 0);
		if(Digit4State < 10) SetDigit_Int(4, Digit4State, 0);
		SetDP(1, DP1State);
		SetDP(2, DP2State);
		
		
	//}
	
	//Turn digits on
	/*if(Digit1State < 10)
	{
		SetDigit_Int(1, Digit1State, 0);
	}
	
	if(Digit2State < 10)
	{
		SetDigit_Int(2, Digit2State, 0);
	}
	
	if(Digit3State < 10)
	{
		SetDigit_Int(3, Digit3State, 0);
	}
	
	if(Digit4State < 10)
	{
		SetDigit_Int(4, Digit4State, 0);
	}*/
	
	 //Digit1State;
	// Digit2State;
	 //Digit3State;
	// Digit4State;
	
	//if(DigitStatus == 1)
	//{
	//	SetDigit(4, 5);
	//}
	return;
}

ISR(TIMER3_COMPB_vect)
{
		//if(ClockStatus == CLOCK_STATUS_STANDBY)
		//{
			SetDigit_Int(1, 12, 0);
			SetDigit_Int(2, 12, 0);
			SetDigit_Int(3, 12, 0);
			SetDigit_Int(4, 12, 0);
			SetDP(1, 0);
			SetDP(2, 0);
			
		//}
		
		/*if(Digit1State < 10)
		{
			SetDigit_Int(1, 10, 0);
		}
		
		if(Digit2State < 10)
		{
			SetDigit_Int(2, 10, 0);
		}
		
		if(Digit3State < 10)
		{
			SetDigit_Int(3, 10, 0);
		}
		
		if(Digit4State < 10)
		{
			SetDigit_Int(4, 10, 0);
		}*/
	
	
	
	
	
	/*if(DigitStatus == 1)
	{
		SetDigit(4, 10);
		
		GlobalDimmingVal = GlobalDimmingVal+100;
		if(GlobalDimmingVal > 45000)
		{
			GlobalDimmingVal = 1000;
		}
		
		OCR3BH = (GlobalDimmingVal>>8) & 0xFF;
		OCR3BL = GlobalDimmingVal & 0xFF;
	}*/
	return;
}

void SetDimming(uint16_t DimVal)
{
	OCR3BH = (DimVal>>8) & 0xFF;
	OCR3BL = DimVal & 0xFF;
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

void GPSPower(uint8_t GPSState)
{
	GPS_PWR_DDR |= 1<<GPS_PWR_PIN_NUM;
	if(GPSState == 1)
	{
		GPS_PWR_PORT |= 1<<GPS_PWR_PIN_NUM;
	}
	else
	{
		GPS_PWR_PORT &= ~(1<<GPS_PWR_PIN_NUM);
	}
	return;
}

void IOExpReset(uint8_t ResetState)
{
		IO_EXPANDER_RESET_DDR |= 1<<IO_EXPANDER_RESET_PIN_NUM;
		if(ResetState == 1)
		{
			IO_EXPANDER_RESET_PORT |= 1<<IO_EXPANDER_RESET_PIN_NUM;
		}
		else
		{
			IO_EXPANDER_RESET_PORT &= ~(1<<IO_EXPANDER_RESET_PIN_NUM);
		}
		return;
}

uint8_t PWRState(uint8_t SystemToQuery)
{
	if(SystemToQuery == HV_PWR_NUM)
	{
		if( (HVPSU_DDR & (1<<HVPSU_PIN_NUM)) == (1<<HVPSU_PIN_NUM) )
		{
			//Pin is set properly as an output
			if( (HVPSU_PORT & (1<<HVPSU_PIN_NUM)) == (1<<HVPSU_PIN_NUM) )
			{
				//Pin is driven high
				return PWR_STATUS_HIGH;
			}
			else
			{
				//Pin is driven low
				return PWR_STATUS_LOW;
			}
		}
		else
		{
			//Pin is set as an input 
			//TODO: this may not be an error, if external pullups are used. confirm this...
			return PWR_STATUS_ERROR;
		}
	}
	
	else if(SystemToQuery == GPS_PWR_NUM)
	{
		if( (GPS_PWR_DDR & (1<<GPS_PWR_PIN_NUM)) == (1<<GPS_PWR_PIN_NUM) )
		{
			//Pin is set properly as an output
			if( (GPS_PWR_PORT & (1<<GPS_PWR_PIN_NUM)) == (1<<GPS_PWR_PIN_NUM) )
			{
				//Pin is driven high
				return PWR_STATUS_HIGH;
			}
			else
			{
				//Pin is driven low
				return PWR_STATUS_LOW;
			}
		}
		else
		{
			//Pin is set as an input
			//TODO: this may not be an error, if external pullups are used. confirm this...
			return PWR_STATUS_ERROR;
		}
	}
	
	else if(SystemToQuery == IOEXP_RESET_NUM)
	{
		if( (IO_EXPANDER_RESET_DDR & (1<<IO_EXPANDER_RESET_PIN_NUM)) == (1<<IO_EXPANDER_RESET_PIN_NUM) )
		{
			//Pin is set properly as an output
			if( (IO_EXPANDER_RESET_PORT & (1<<IO_EXPANDER_RESET_PIN_NUM)) == (1<<IO_EXPANDER_RESET_PIN_NUM) )
			{
				//Pin is driven high
				return PWR_STATUS_HIGH;
			}
			else
			{
				//Pin is driven low
				return PWR_STATUS_LOW;
			}
		}
		else
		{
			//Pin is set as an input
			//TODO: this may not be an error, if external pullups are used. confirm this...
			return PWR_STATUS_ERROR;
		}
	}
	
	
	
	return 	PWR_STATUS_NOSUB;
}



//TODO: Add function to reset backer board
//TODO: Handle backer board and stuff...
void LED(uint8_t LEDNumber, uint8_t state)
{
	
	#if BACK_BOARD_PRESENT == 0
	//No backer board, use LEDs on the CPU board
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
	#else
	//Backer board is installed, use the LEDs on the button board
	//Note: The pins for the LEDs on the CPU board have other functions when the backer board is installed, the button board will not work when using the CPU board LEDs.
	//TODO: Allow multiple LEDs???
	uint8_t TWI_Stat;
	uint8_t OutputReg;
	
	//Read the current state of the output register
	TWI_Stat = TCA9538_ReadReg (1, TCA9538_REG_OUTPUT, &OutputReg);
	if(TWI_Stat != 0x00)
	{
		//Exit if we get an I2C error.
		//TODO: Make some sort of debug thing here (outputs, led calls, etc...)
		//printf_P(PSTR("I2C Error\n"));
		return;
	}
	
	if(LEDNumber == LED_RED)
	{
		if(state == 0)
		{
			OutputReg &= ~LED_RED_BIT;
		}
		else
		{
			OutputReg |= LED_RED_BIT;
		}
	}
	else if(LEDNumber == LED_GREEN)
	{
		if(state == 0)
		{
			OutputReg &= ~LED_GREEN_BIT;
		}
		else
		{
			OutputReg |= LED_GREEN_BIT;
		}
	}
	else if(LEDNumber == LED_BLUE)
	{
		if(state == 0)
		{
			OutputReg &= ~LED_BLUE_BIT;
		}
		else
		{
			OutputReg |= LED_BLUE_BIT;
		}
	}
	else if(LEDNumber == 0)	//All Off
	{
		OutputReg &= ~LED_MASK;
	}
	
	TWI_Stat = TCA9538_WriteReg (1, TCA9538_REG_OUTPUT, OutputReg);

	#endif

	return;
}

//TODO: This function should not be called from outside the interrupts...
void BlankDigit(uint8_t DigitToBlank)
{
	if(DigitToBlank == 1)
	{
		//Digit1State = 10;
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
		//Digit2State = 10;
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
		//Digit3State = 10;
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
		//Digit4State = 10;
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
	//if(NumberToDisplay < 10)
	//{
		
	
	if(Digit == 1)
	{
		Digit1State = NumberToDisplay;
	}
	else if(Digit == 2)
	{
		Digit2State = NumberToDisplay;
	}
	else if(Digit == 3)
	{
		Digit3State = NumberToDisplay;
	}
	else if(Digit == 4)
	{
		Digit4State = NumberToDisplay;
	}
	//}
	//SetDigit_Int(Digit, NumberToDisplay, 1);
	return;
}

void SetDigit_Int(uint8_t Digit, uint8_t NumberToDisplay, uint8_t UpdateReg)
{
	switch (Digit)
	{
		//Digit 1
		case 1:
		BlankDigit(1);
		//if(UpdateReg == 1)
		//{
		//	Digit1State = NumberToDisplay;
		//}
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
		//if(UpdateReg == 1)
		//{
		//	Digit2State = NumberToDisplay;
		//}
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
		//if(UpdateReg == 1)
		//{
		//	Digit3State = NumberToDisplay;
		//}
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
		//if(UpdateReg == 1)
		//{
		//	Digit4State = NumberToDisplay;
		//}
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

uint8_t GetSystemStatus(void)
{
	return SystemStatus;
}

uint8_t GetClockStatus(void)
{
	return ClockStatus;
}

void SetSystemStatus(uint8_t StatusToSet)
{
	SystemStatus = StatusToSet;
	return;
}

void SetClockStatus(uint8_t StatusToSet)
{
	ClockStatus = StatusToSet;
	
	//printf("Stat: %d\n", StatusToSet);
	
	if(StatusToSet == CLOCK_STATUS_NORMAL)
	{
		//Turn on display, show time
		UpdateDisplay();
		//printf("on\n");
	}
	else if(StatusToSet == CLOCK_STATUS_STANDBY)
	{
		SetDigit(4,10);
		SetDigit(3,10);
		SetDigit(2,10);
		SetDigit(1,10);
		//BlankDigit(1);
		//BlankDigit(2);
		//BlankDigit(3);
		//BlankDigit(4);
		//printf("blank\n");
	}
	return;
}

//System time is set in GMT. Time zone and location are used to get local time and sunrise/sunset times.
uint8_t SetSystemTime(struct tm *TimeToSet, int8_t TimeZoneToSet, int32_t LatInSeconds, int32_t LongInSeconds)
{
	TimeToSet->tm_isdst = 0;
	set_system_time(mk_gmtime(TimeToSet));
	
	set_zone(TimeZoneToSet * ONE_HOUR);		//Set time zone to CST
	set_position(LatInSeconds, LongInSeconds);
	set_dst(usa_dst);
	SetClockStatus(CLOCK_STATUS_NORMAL);		//TODO: remove this here??
	
	
	SetTimeZone(TimeZoneToSet);
	SetLat(LatInSeconds);
	SetLong(LongInSeconds);
	//TODO: set the RTC also...
	
	DS3231M_SetTime(TimeToSet);
	
	return 0;
}

//Update the display with the current system time
void UpdateDisplay(void)
{
	struct tm TheTime;
	time_t TimeVal;
	uint8_t HoursValue;
	
	time(&TimeVal);
	localtime_r(&TimeVal, &TheTime);

	if( (DisplaySetup & TIME_MODE_24_HOURS) != TIME_MODE_24_HOURS)
	{
		if(TheTime.tm_hour > 12)
		{
			HoursValue = TheTime.tm_hour - 12;
		}
		else if(TheTime.tm_hour == 0)
		{
			//An hour of 0 should be displayed as 12:xx
			HoursValue = 12;
		}
		else
		{
			HoursValue = TheTime.tm_hour;
		}
	}
	else
	{
		HoursValue = TheTime.tm_hour;
	}

	if( ((DisplaySetup & CLOCK_SHOWN_LEADING_ZERO) != CLOCK_SHOWN_LEADING_ZERO) && (HoursValue < 10) )
	{
		//SetDigit with values greater than 9 will turn off the digit
		SetDigit(4, 10);
	}
	else
	{
		SetDigit(4, HoursValue / 10);
	}
	
	SetDigit(3, HoursValue % 10);
	SetDigit(2, TheTime.tm_min / 10);
	SetDigit(1, TheTime.tm_min % 10);

	return;
}

uint8_t GetDigitStatus(uint8_t DigitToQuery)
{
	if(DigitToQuery == 1)
	{
		return Digit1State;
	}
	else if(DigitToQuery == 2)
	{
		return Digit2State;
	}
	else if(DigitToQuery == 3)
	{
		return Digit3State;
	}
	else if(DigitToQuery == 4)
	{
		return Digit4State;
	}
	
	return 0xFF;
}


int8_t GetTimeZone(void)
{
	return (int8_t)eeprom_read_byte(&NV_TimeZone);
}

int32_t GetLat(void)
{
	return (int32_t)eeprom_read_dword(&NV_LatInSeconds);
}

int32_t GetLong(void)
{
	return (int32_t)eeprom_read_dword(&NV_LongInSeconds);
}

void SetTimeZone(int8_t TimeZoneToSet)
{
	set_zone(TimeZoneToSet * ONE_HOUR);
	eeprom_update_byte(&NV_TimeZone, (uint8_t)TimeZoneToSet);
}

void SetLat(int32_t LatToSet)
{
	eeprom_update_dword(&NV_LatInSeconds, (int32_t)LatToSet);
}

void SetLong(int32_t LongToSet)
{
	eeprom_update_dword(&NV_LongInSeconds, (int32_t)LongToSet);
}

int8_t GetTimeFromGPS(void)
{
	uint8_t StatusByte = 0;
	char GPGGA_STRING[155];
	char GPRMC_STRING[155];
	int32_t LatSec;
	int32_t LongSec;
	struct tm GPSTime;
	
	StatusByte = GetSystemStatus();
	if( ((StatusByte & SYSTEM_STATUS_GPS_PRESENT) == SYSTEM_STATUS_GPS_PRESENT) )
	{
		//GPS is present, check for lock and set time if GPS is locked.
		StatusByte = GPS_GetStrings(GPGGA_STRING, GPRMC_STRING);		
		if(StatusByte == 0)
		{
			StatusByte = GPS_ValidateRMC(GPRMC_STRING, &GPSTime, &LatSec, &LongSec);
			if(StatusByte == 0)
			{
				//Set the system time and RTC based on the GPS
				SetSystemTime(&GPSTime, GetTimeZone(), LatSec, LongSec);
				return 0;
				//SetSystemStatus( GetSystemStatus() | SYSTEM_STATUS_RTC_VALID);		//TODO: Do i set status here or outside this fucntion?
				//SetClockStatus(CLOCK_STATUS_NORMAL);		//TODO: right now this is set in SetSystemTime(). I probably want to move it out of that function.
			}
		}
	}
	else
	{
		return 0xFF;	//Return code if GPS not present.
	}
	
	return StatusByte;
}