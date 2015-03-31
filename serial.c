/*
 * untitled.c
 * 
 * Copyright 2014  <pi@raspberrypi>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART


//-------------------------
//---- MODULE VARIABLE ----
//-------------------------
int uart_id = -1;


//-------------------------
//----- SETUP UART 0 -----
//-------------------------
//At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively

	
//OPEN THE UART
//The flags (defined in fcntl.h):
//	Access modes (use 1 of these):
//		O_RDONLY - Open for reading only.
//		O_RDWR - Open for reading and writing.
//		O_WRONLY - Open for writing only.
//
//	O_NDELAY / O_NONBLOCK (same function) 
//      Enables nonblocking mode. 
//      When set read requests on the file can return immediately with a failure status
//		if there is no input immediately available (instead of blocking). 
//      Likewise, write requests can also return immediately with a failure status
//      if the output can't be written immediately.
//
//	O_NOCTTY
//      When set and path identifies a terminal device, 
//      open() shall not cause the terminal device to become the controlling terminal
//      for the process.
// 		"/dev/ttyAMA0"
extern int uart_open(char* device)
{
	//Open in non blocking read/write mode
	int mode = O_RDWR | O_NOCTTY | O_NDELAY ;
	uart_id = open(device, mode);
			
	if (uart_id == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}
	return uart_id ;
}	


//CONFIGURE THE UART
//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
//	CSIZE:- CS5, CS6, CS7, CS8
//	CLOCAL - Ignore modem status lines
//	CREAD - Enable receiver
//	IGNPAR = Ignore characters with parity errors
//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
//	PARENB - Parity enable
//	PARODD - Odd parity (else even)
void uart_configure(int uart_id)
{
	struct termios options;
	tcgetattr(uart_id, &options);
//	options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_cflag = B9600 | CS8 | CLOCAL ;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart_id, TCIFLUSH);
	tcsetattr(uart_id, TCSANOW, &options);
}

// Transmitting Bytes
int uart_tx(int uart_id, char* buffer, int size)
{
	int count = 0 ;
	//----- TX BYTES -----

	
	if (uart_id != -1)
	{
		// Filestream, bytes to write, number of bytes to write
		count = write(uart_id, buffer, size);		
		if (count < 0)
		{
			printf("UART TX error\n");
		}
	}
	return count ;
}

//Receiving Bytes
//Because O_NDELAY has been used this will exit if there are no receive bytes waiting (non blocking read),
//so if you want to hold waiting for input simply put this in a while loop
int uart_rx(int uart_id, char* buffer, int size)
{
	int rx_length = 0 ;
	unsigned char rx_buffer[256];

	//----- CHECK FOR ANY RX BYTES -----
	if (uart_id != -1)
	{
		// Read up to 255 characters from the port if they are there
		// Filestream, buffer to store in, number of bytes to read (max)
		rx_length = read(uart_id, (void*)rx_buffer, sizeof(rx_buffer));		
		if (rx_length < 0)
		{
			//An error occured (will occur if there is no byte)
		}
		else if (rx_length == 0)
		{
			//No data waiting
		}
		else
		{
			//Bytes received
//			rx_buffer[rx_length] = '\0';
//			printf("%i bytes read : %s\n", rx_length, rx_buffer);
		}
	}
	return rx_length ;
	
}

//Closing the UART if no longer needed
//----- CLOSE THE UART -----
void uart_close(int uart_id)
{
	close(uart_id);
}
