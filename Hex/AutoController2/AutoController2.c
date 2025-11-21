/*
Nintendo Switch Auto Controller 2
	brianuuuSonic
	2025-11-17

Based on the LUFA library's Low-Level Joystick Demo
	(C) Dean Camera
Based on the HORI's Pokken Tournament Pro Pad design
	(C) HORI

This project implements a modified version of HORI's Pokken Tournament Pro Pad
USB descriptors to allow for the creation of custom controllers for the
Nintendo Switch. This also works to a limited degree on the PS3.

Since System Update v3.0.0, the Nintendo Switch recognizes the Pokken
Tournament Pro Pad as a Pro Controller. Physical design limitations prevent
the Pokken Controller from functioning at the same level as the Pro
Controller. However, by default most of the descriptors are there, with the
exception of Home and Capture. Descriptor modification allows us to unlock
these buttons for our use.

This project is based on auto-controller code written by brianuuuuSonic
*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "../Config/uart.h"
#include "../Joystick.h"

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define CHECK_BIT(var,pos) (var & (1UL << pos))
#define VERSION 1

// Main entry point.
int main(void) {
	// We'll start by performing hardware and peripheral setup.
	SetupHardware();
	// We'll then enable global interrupts for our use.
	GlobalInterruptEnable();
	// Then we'll initialize the serial communications with the external computer
	
	// Once that's done, we'll enter an infinite loop.
	for (;;)
	{
		// We need to run our task to process and deliver data for our IN and OUT endpoints.
		HID_Task();
		// We also need to run the main USB management task.
		USB_USBTask();
	}
}

// Configures hardware and peripherals, such as the USB peripherals.
void SetupHardware(void) {
	// We need to disable watchdog if enabled by bootloader/fuses.
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// We need to disable clock division before initializing the USB hardware.
	//clock_prescale_set(clock_div_1);
	// We can then initialize our hardware and peripherals, including the USB stack.
	
	CPU_PRESCALE(0);  // run at 16 MHz
	uart_init(9600);
	
	#ifdef ALERT_WHEN_DONE
	// Both PORTD and PORTB will be used for the optional LED flashing and buzzer.
	#warning LED and Buzzer functionality enabled. All pins on both PORTB and \
PORTD will toggle when printing is done.
	DDRD  = 0xFF; //Teensy uses PORTD
	PORTD =  0x0;
                  //We'll just flash all pins on both ports since the UNO R3
	DDRB  = 0xFF; //uses PORTB. Micro can use either or, but both give us 2 LEDs
	PORTB =  0x0; //The ATmega328P on the UNO will be resetting, so unplug it?
	#endif
	// The USB stack should be initialized last.
	USB_Init();
}

// Fired to indicate that the device is enumerating.
void EVENT_USB_Device_Connect(void) {
	// We can indicate that we're enumerating here (via status LEDs, sound, etc.).
}

// Fired to indicate that the device is no longer connected to a host.
void EVENT_USB_Device_Disconnect(void) {
	// We can indicate that our device is not ready (via status LEDs, sound, etc.).
}

// Fired when the host set the current configuration of the USB device after enumeration.
void EVENT_USB_Device_ConfigurationChanged(void) {
	bool ConfigSuccess = true;

	// We setup the HID report endpoints.
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);

	// We can read ConfigSuccess to indicate a success or failure at this point.
}

// Process control requests sent to the device from the USB host.
void EVENT_USB_Device_ControlRequest(void) {
	// We can handle two control requests: a GetReport and a SetReport.

	// Not used here, it looks like we don't receive control request from the Switch.
}

#define ECHOES 5
int echoes = ECHOES;

uint32_t button_flag = 0;
bool waiting_input = false;
bool should_spam = false;

// Process and deliver data from IN and OUT endpoints.
void HID_Task(void) {
	// If the device isn't connected and properly configured, we can't do anything here.
	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	// We'll start with the OUT endpoint.
	Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);
	// We'll check to see if we received something on the OUT endpoint.
	if (Endpoint_IsOUTReceived())
	{
		// If we did, and the packet has data, we'll react to it.
		if (Endpoint_IsReadWriteAllowed())
		{
			// We'll create a place to store our data received from the host.
			USB_JoystickReport_Output_t JoystickOutputData;
			// We'll then take in that data, setting it up in our storage.
			while(Endpoint_Read_Stream_LE(&JoystickOutputData, sizeof(JoystickOutputData), NULL) != ENDPOINT_RWSTREAM_NoError);
			// At this point, we can react to this data.

			// However, since we're not doing anything with this data, we abandon it.
		}
		// Regardless of whether we reacted to the data, we acknowledge an OUT packet on this endpoint.
		Endpoint_ClearOUT();
	}

	// We'll then move on to the IN endpoint.
	Endpoint_SelectEndpoint(JOYSTICK_IN_EPADDR);
	// We first check to see if the host is ready to accept data.
	if (Endpoint_IsINReady())
	{
		// We'll create an empty report.
		USB_JoystickReport_Input_t JoystickInputData;
		// We'll then populate this report with what we want to send to the host.
		GetNextReport(&JoystickInputData);
		// Once populated, we can output this data to the host. We do this by first writing the data to the control stream.
		while(Endpoint_Write_Stream_LE(&JoystickInputData, sizeof(JoystickInputData), NULL) != ENDPOINT_RWSTREAM_NoError);
		// We then send an IN packet on this endpoint.
		Endpoint_ClearIN();
	}
	
	// Check first byte in the queue, if not the mode we want, clear everything
	if (!waiting_input && uart_available() > 0)
	{
		echoes = 0;
		button_flag = 0;
		
		if (uart_getchar() == 0xFF)
		{
			waiting_input = true;
		}
		else
		{
			while (uart_available() > 0)
			{
				uart_getchar();
			}
		}
	}
	else if (waiting_input && uart_available() >= 4)
	{
		button_flag = 0;
		for (uint8_t i = 0; i < 4; i++)
		{
			button_flag |= ((uint32_t)(uart_getchar()) << (8UL * i));
		}
		
		// Discard the rest
		while (uart_available() > 0)
		{
			uart_getchar();
		}
		
		uart_putchar((char)VERSION);
		waiting_input = false;
	}
}

USB_JoystickReport_Input_t last_report;

// Prepare the next report for the host.
void GetNextReport(USB_JoystickReport_Input_t* const ReportData) {

	// Prepare an empty report
	memset(ReportData, 0, sizeof(USB_JoystickReport_Input_t));
	ReportData->LX = STICK_CENTER;
	ReportData->LY = STICK_CENTER;
	ReportData->RX = STICK_CENTER;
	ReportData->RY = STICK_CENTER;
	ReportData->HAT = HAT_CENTER;
	
	// Repeat ECHOES times the last report
	if (echoes > 0)
	{
		memcpy(ReportData, &last_report, sizeof(USB_JoystickReport_Input_t));
		echoes--;
		return;
	}
	
	// check if this frame should spam button or not
	bool has_button_input = true;
	if (CHECK_BIT(button_flag, 26))
	{
		should_spam = !should_spam;
		has_button_input = should_spam;
	}
	
	bool up, down, left, right;
	if (has_button_input)
	{
		// Buttons
		if (CHECK_BIT(button_flag, 0))
			ReportData->Button |= SWITCH_A;
		if (CHECK_BIT(button_flag, 1))
			ReportData->Button |= SWITCH_B;
		if (CHECK_BIT(button_flag, 2))
			ReportData->Button |= SWITCH_X;
		if (CHECK_BIT(button_flag, 3))
			ReportData->Button |= SWITCH_Y;
		if (CHECK_BIT(button_flag, 4))
			ReportData->Button |= SWITCH_L;
		if (CHECK_BIT(button_flag, 5))
			ReportData->Button |= SWITCH_R;
		if (CHECK_BIT(button_flag, 6))
			ReportData->Button |= SWITCH_ZL;
		if (CHECK_BIT(button_flag, 7))
			ReportData->Button |= SWITCH_ZR;
		if (CHECK_BIT(button_flag, 8))
			ReportData->Button |= SWITCH_PLUS;
		if (CHECK_BIT(button_flag, 9))
			ReportData->Button |= SWITCH_MINUS;
		if (CHECK_BIT(button_flag, 10))
			ReportData->Button |= SWITCH_HOME;
		if (CHECK_BIT(button_flag, 11))
			ReportData->Button |= SWITCH_CAPTURE;
		if (CHECK_BIT(button_flag, 12))
			ReportData->Button |= SWITCH_LCLICK;
		if (CHECK_BIT(button_flag, 17))
			ReportData->Button |= SWITCH_RCLICK;
		
		
		// DPad
		up = CHECK_BIT(button_flag, 22);
		down = CHECK_BIT(button_flag, 23);
		left = CHECK_BIT(button_flag, 24);
		right = CHECK_BIT(button_flag, 25);
		if (up)
		{
			ReportData->HAT = HAT_TOP;
			if (left)
			{
				ReportData->HAT = HAT_TOP_LEFT;
			}
			else if (right)
			{
				ReportData->HAT = HAT_TOP_RIGHT;
			}
		}
		else if (down)
		{
			ReportData->HAT = HAT_BOTTOM;
			if (left)
			{
				ReportData->HAT = HAT_BOTTOM_LEFT;
			}
			else if (right)
			{
				ReportData->HAT = HAT_BOTTOM_RIGHT;
			}
		}
		else if (left)
		{
			ReportData->HAT = HAT_LEFT;
		}
		else if (right)
		{
			ReportData->HAT = HAT_RIGHT;
		}
	}
	
	// L-Stick
	up = CHECK_BIT(button_flag, 13);
	down = CHECK_BIT(button_flag, 14);
	left = CHECK_BIT(button_flag, 15);
	right = CHECK_BIT(button_flag, 16);
	if (up)
	{
		ReportData->LY = STICK_MIN;
	}
	else if (down)
	{
		ReportData->LY = STICK_MAX;
	}
	if (left)
	{
		ReportData->LX = STICK_MIN;		
	}
	else if (right)
	{
		ReportData->LX = STICK_MAX;
	}
	
	// R-Stick
	up = CHECK_BIT(button_flag, 18);
	down = CHECK_BIT(button_flag, 19);
	left = CHECK_BIT(button_flag, 20);
	right = CHECK_BIT(button_flag, 21);
	if (up)
	{
		ReportData->RY = STICK_MIN;
	}
	else if (down)
	{
		ReportData->RY = STICK_MAX;
	}
	if (left)
	{
		ReportData->RX = STICK_MIN;		
	}
	else if (right)
	{
		ReportData->RX = STICK_MAX;
	}

	// Prepare to echo this report
	memcpy(&last_report, ReportData, sizeof(USB_JoystickReport_Input_t));
	
	echoes = ECHOES;
}
