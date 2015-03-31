/*+--------------------------------------------------------------+
  | SYSTEM INCLUDES                                              |
  +--------------------------------------------------------------+*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringSerial.h>

/*+--------------------------------------------------------------+
  | SPECIFIC INCLUDES                                            |
  +--------------------------------------------------------------+*/
#include "debug.h"		//file where all debug const are defined
#include "const.h"
#include "automate.h"
#include "nexstar.h"
#include "stellarium.h"
#include "screen.h"

/*+--------------------------------------------------------------+
  | LOCAL CONSTANTS                                              |
  +--------------------------------------------------------------+*/
#define DEBUG 0

#define DEVICE "/dev/ttyUSB0"
#define UART_NEXSTAR_SPEED 9600

#define BUFFER_SIZE 40
#define TIMEOUT		10 

#define NEXSTAR_CMD_ECHO		"KA"	//Echo : useful to check communication  | cmd : “K” & chr(x) | reply : chr(x) & “#” 
#define NEXSTAR_ECHO			'A'

#define BINTOASCII(x) (((x)<10)?('0'+(x)):('A'+(x)-10))
#define ASCIITOBIN(x) (((x)<'A')?((x)-'0'):((x)-'A'+10))



/*+--------------------------------------------------------------+
  | GLOBAL VARIABLES                                             |
  +--------------------------------------------------------------+*/
  



/*+--------------------------------------------------------------+
  | LOCAL VARIABLES                                             |
  +--------------------------------------------------------------+*/
static int	fd ; 					//uart file descriptor
static char buffer[BUFFER_SIZE] ;
static int	flag_data_received ;


void nexstar_init()
{
	flag_data_received = FALSE ;
	
	trace("nexstar_server| starting...") ;
	{
		char tmp_str[80] ;
		sprintf(tmp_str, "nexstar_init| try to open uart on %s...", DEVICE) ;
		trace(tmp_str) ;
	}

	fd = serialOpen(DEVICE, UART_NEXSTAR_SPEED ) ;
    if (fd == -1)
    {
		perror("nexstar_init|serialOpen");
	}
	else
    { // serial port is open
#if DEBUG
		{
			char tmp_str[80] ;
			sprintf(tmp_str,"nextar_init| uart open on %s, uart_fd: %d", DEVICE, fd) ;
			trace(tmp_str) ;
		}
#endif
		serialFlush(fd) ;
    } // serial port is open
}



/*+--------------------------------------------------------------
  | Function name :  nexstar_listen
  | Parameters    :  
  | Description   :  wait for nextar telescope answer upto the '#' character
  | Description   :  
  * Return		  : 0 if nothing has been received and timeout occurs
  * 			  : 1 if only the '#' character has been received
  * 			  : x (x>1) the number of received characters including '#'
  *--------------------------------------------------------------
*/
int nexstar_listen()
{
	int ret = 0 ;
	int timeout = TIMEOUT ;
	int i_buffer = 0 ;

	
	do 
	{ // waiting for answer coming from telescope
		
		int data_length = serialDataAvail(fd) ;
		if (data_length > 0 )
		{ // some data are available

			char car ;
			do
			{
				car = serialGetchar(fd) ;
				if (car == -1) break ;
				buffer[i_buffer++] = car ;
				if (i_buffer > BUFFER_SIZE) 
				{ // to protect against buffer overflow
#if DEBUG
					printf("nextar_listen|ERROR Buffer overflow !\n") ;
#endif
					return 0 ;
				} // to protect against buffer overflow
			}
			while (--data_length) ;


			if (car == '#')
			{ // last received car is "#"
				buffer[i_buffer] = '\0' ;
				ret = i_buffer ;
#if DEBUG						
//				printf("nexstar_listen|%s\n", buffer) ;
#endif
				break ;
			} // last received car is "#"			
		} // some data are available
		//sleep(1) ;
		delay(100) ;
	} // waiting for answer coming from telescope
	while (timeout--) ;
	
	return ret ;
}




/*+--------------------------------------------------------------+
  | Function name :  nexstar_send_to_telescope
  | Parameters    :  none
  | Description   :  
  | Description   :  
  +--------------------------------------------------------------+*/
void nexstar_send_to_telescope(char* cmd)
{

#if DEBUG
	//printf("nexstar_send| %s\n", cmd) ;
#endif

	if (fd != -1)
	{
		serialPrintf (fd, "%s", cmd) ;
	}
}


/*
 --------------------------------------------------------------
  * Function name :  nexstar_get_precise_ra_dec
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * 
  * 
  * 
  * 
  * 
  *
 --------------------------------------------------------------
 */
int nexstar_get_precise_ra_dec(int* p_ra, int* p_dec)
{
	int ra, dec ;
	
	nexstar_send_to_telescope("e") ;
	if (nexstar_listen() == 18)
	{
#if DEBUG
		//printf("nexstar_get_precise_ra_dec|%s\n", buffer) ;
#endif		
		ra = 0 ;
		dec = 0 ;

		int i ;
		for (i=0; i<6; i++)
		{
			ra += (unsigned int) ASCIITOBIN( buffer[i] );
			ra <<= 4 ;
		}
		ra <<= 4 ;

		for (i=9; i<15; i++)
		{
			dec += (unsigned int) ASCIITOBIN(buffer[i]);
			dec <<= 4 ;
		}
		dec <<= 4 ;

#if DEBUG
		//printf("nexstar_get_precise_ra_dec|ra: %08X\n", ra) ;
		//printf("nexstar_get_precise_ra_dec|ra: %08X\n", dec) ;
		//printf("nexstar_get_precise_ra_dec|ra: %d, dec: %d \n", ra, dec) ;
#endif		

	}
	*p_ra = ra ;
	*p_dec = dec ;

	return 1 ;
}


/*
 --------------------------------------------------------------
  * Function name :  nexstar_goto_precise_ra_dec
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * 
  * 
  * 
  * 
  * 
  * 
 --------------------------------------------------------------
 */
void nexstar_goto_precise_ra_dec(int ra, int dec)
{
	char cmd[20] ;
	int i ;
	
#if DEBUG	
	printf("nexstar_goto_precise_ra_dec|ra: %d, dec:%d\n", ra, dec) ;
#endif
	
	cmd[0] =  'r' ;
	ra &= 0xFFFFFF00 ;
	for (i=8; i>0; i--)
	{
		cmd[i] = BINTOASCII((unsigned int) (ra & 0x0000000F)) ;
		ra >>= 4 ;
	}
	cmd[9] =  ',' ;
	
	dec &= 0xFFFFFF00 ;
	for (i=17; i>9; i--)
	{
		cmd[i] = BINTOASCII((unsigned int) (dec & 0x0000000F)) ;
		dec >>= 4 ;
	}
	cmd[18] =  '\0' ;
	
	//printf("cmd: %s\n", cmd) ;
	nexstar_send_to_telescope((char*)cmd) ;

	// wait for the ack car '#'
	if (nexstar_listen() == 1)
	{	// ack car '#' 
#if DEBUG
		printf("nexstar_goto_precise_ra_dec|ACK received\n") ;
#endif		
	}	// ack car '#'

}


/*
 --------------------------------------------------------------
  * Function name :  nexstar_sync_precise_ra_dec
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * 
  * 
  * 
  * 
  * 
  *
 --------------------------------------------------------------
 */
void nexstar_sync_precise_ra_dec(long ra, long dec)
{
	// TOTO make the cmd
	nexstar_send_to_telescope("s xxxx") ;
	// wait for the ack car '#'
	if (nexstar_listen() == 1)
	{	// ack car '#' 
#if DEBUG
		printf("nexstar_sync_precise_ra_dec|ACK received\n") ;
#endif		
	}	// ack car '#'

}


/*
 --------------------------------------------------------------
  * Function name :  nexstar_get_tracking_mode
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * 
  * 
  * 
  * 
  * 
  *
 --------------------------------------------------------------
 */
int nexstar_get_tracking_mode()
{
	int mode = 1 ;
	// TOTO make the cmd
	nexstar_send_to_telescope("t") ;
	// wait for the ack car '#'
	if (nexstar_listen() == 2)
	{	// ack car '#' 
#if DEBUG
		printf("nexstar_get_tracking_mode|tracking: %x\n", buffer[0]) ;
#endif		
	}	// ack car '#'
	
	return mode ;
}


/*
 --------------------------------------------------------------
  * Function name :  nexstar_set_tracking_mode
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * 
  * 
  * 
  * 
  * 
  *
 --------------------------------------------------------------
 */
void nexstar_set_tracking_mode(int mode)
{
	char cmd[3] ;
	
	cmd[0] = 'T' ;
	cmd[1] = '0' ;
	nexstar_send_to_telescope(cmd) ;
	// wait for the ack car '#'
	if (nexstar_listen() == 1)
	{	// ack car '#' 
#if DEBUG
		printf("nexstar_set_tracking_mode|ACK received\n") ;
#endif		
	}	// ack car '#'

}

 
/*
 --------------------------------------------------------------
  * Function name :  nexstar_get_location
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * The format of the location commands is: ABCDEFGH, where:
  * A is the number of degrees of latitude.
  * B is the number of minutes of latitude.
  * C is the number of seconds of latitude.
  * D is 0 for north and 1 for south.
  * E is the number of degrees of longitude.
  * F is the number of minutes of longitude.
  * G is the number of seconds of longitude.
  * H is 0 for east and 1 for west.
  * 
  * 
  * 
  * 
  * 21  32  29  0  76  14  11  1  23
 --------------------------------------------------------------
 */
int nexstar_get_location()
{
	int ret = FALSE ;

	
	nexstar_send_to_telescope("w") ;
	if (nexstar_listen() == 9)
	{	// ack car '#'
#if DEBUG	
		printf("nexstar_get_location|lattitude: %02d° %02d' %02d\" %c\n", buffer[0], buffer[1], buffer[2], (buffer[3]?'S':'N')) ;
		printf("nexstar_get_location|longitude: %02d° %02d' %02d\" %c\n", buffer[4], buffer[5], buffer[6], (buffer[7]?'W':'E')) ;
#endif
		ret = TRUE ;
#if DEBUG
		printf("nexstar_get_location|ACK received\n") ;
#endif		
	}	// ack car '#'
	
	return ret ;
}

 
/*
 --------------------------------------------------------------
  * Function name :  nexstar_set_location
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * 
  * 
  * 
  * 
  * 
  *
 --------------------------------------------------------------
 */
void nexstar_set_location(int lad, int lam, int las, int ns, int lod, int lom, int los, int ew)
{
	char cmd[10] ;
#ifdef DEBUG
	//printf("nexstar_set_location|lattitude: %02d° %02d' %02d\" %c\n",lad, lam, las, (ns?'S':'N')) ;
	//printf("nexstar_set_location|longitude: %02d° %02d' %02d\" %c\n",lod, lom, los, (ew?'W':'E')) ;
#endif

	cmd[0] = 'W' ;
	cmd[1] = lad ;
	cmd[2] = lam ;
	cmd[3] = las ;
	cmd[4] = ns ;
	cmd[5] = lod ;
	cmd[6] = lom ;
	cmd[7] = los ;
	cmd[8] = ew ;

	int i ;
	for (i=0; i<9; i++) serialPutchar(fd, cmd[i]) ;

#if DEBUG3					
			printf("nexstar_set_location|") ;
			for (i=0; i<9; i++)
			{
				printf(" %02x ", cmd[i]);
			}
			printf("\n");
#endif				
	
	// wait for the ack car '#'
	if (nexstar_listen() == 1)
	{	// ack car '#' 
#if DEBUG
//		printf("nexstar_set_location|ACK received\n") ;
#endif		
	}	// ack car '#'

}


/*
 --------------------------------------------------------------
  * Function name :  nexstar_get_time
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * 
  * 
  * 
  * 
  * 
  *
 --------------------------------------------------------------
 */
int nexstar_get_time()
{
	int ret = FALSE ;
	
	nexstar_send_to_telescope("h") ;
	if (nexstar_listen() == 9)
	{	// ack car '#' 
#if DEBUG
		printf("nexstar_get_time|%02d/%02d/20%02d %02d:%02d:%02d GMT+%d %s\n", buffer[4], buffer[3], buffer[5], buffer[0], buffer[1], buffer[2], buffer[6], (buffer[7]?"Daylight Savings":"Standard Time")) ;
#endif		
	}	// ack car '#'

	return ret ;
}


/*
 --------------------------------------------------------------
  * Function name :  nexstar_set_time
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * The format of the time commands is: QRSTUVWX, where:
  * Q is the hour (24 hour clock).
  * R is the minutes.
  * S is the seconds.
  * T is the month.
  * U is the day.
  * V is the year (century assumed as 20).
  * W is the offset from GMT for the time zone. Note: if zone is negative, use 256-zone.
  * X is 1 to enable Daylight Savings and 0 for Standard Time.
  * 
  * 
  * 
  * 
  *
 --------------------------------------------------------------
 */
void nexstar_set_time(int day, int month, int year, int hour, int minute, int second)
{
	char cmd[10] ;

#ifdef DEBUG
	//printf("nexstar_set_time|%02d/%02d/%02d %02d:%02d:%02d\n",day, month, year, hour, minute, second) ;
#endif

	cmd[0] = 'H' ;
	cmd[1] = hour ;
	cmd[2] = minute ;
	cmd[3] = second ;
	cmd[4] = month ;
	cmd[5] = day ;
	cmd[6] = year ;
	cmd[7] = 0 ;
	cmd[8] = 0 ;
	
	int i ;
	for (i=0; i<9; i++) serialPutchar(fd, cmd[i]) ;
#if DEBUG3					
			printf("nexstar_set_time|") ;
			for (i=0; i<9; i++)
			{
				printf(" %02x ", cmd[i]);
			}
			printf("\n");
#endif				

	// wait for the ack car '#'
	if (nexstar_listen() == 1)
	{	// ack car '#' 
#if DEBUG
		printf("nexstar_set_time|ACK received\n") ;
#endif		
	}	// ack car '#'

}

/*
 --------------------------------------------------------------
  * Function name :  nexstar_get_version
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * 
  * 
  * 
  * 
  * 
  * 
 --------------------------------------------------------------
 */
void nexstar_get_version()
{
	
	nexstar_send_to_telescope("V") ;
	
	if (nexstar_listen() == 3)	
	{	// nexstar version received 
#if DEBUG
		printf("nexstar_get_version| %d.%d\n",buffer[0],buffer[1]) ;
#endif		
	}	// nexstar version received 

}

/*
 --------------------------------------------------------------
  * Function name :  nexstar_get_device_version
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * 
  * 
  * 
  * 
  * 
  *
 --------------------------------------------------------------
 */
void nexstar_get_device_version(int device)
{
	// TOTO make the cmd
	nexstar_send_to_telescope("V xxxx") ;
	// wait for the ack car '#'
	if (nexstar_listen() == 3)
	{	// version received 
#if DEBUG
		printf("nexstar_get_device_version|ACK received\n") ;
#endif		
	}	// version received

}

/*
 --------------------------------------------------------------
  * Function name :  nexstar_get_model
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * 
  * 
  * 
  * 
  * 
  *
 --------------------------------------------------------------
 */
int nexstar_get_model()
{
	int ret ;
	// TOTO make the cmd
	nexstar_send_to_telescope("m") ;

	if (nexstar_listen() == 2)
	{	//
		ret = buffer[0] ;
#if DEBUG
		printf("nexstar_get_model| %d\n", ret) ;
#endif		
	}	// 

	return ret ;

}

 
/*
 --------------------------------------------------------------
  * Function name :  nexstar_echo
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * 
  * 
  * 
  * 
  * 
  *
 --------------------------------------------------------------
 */
int nexstar_echo()
{
	
	int ret = FALSE ;

	nexstar_send_to_telescope("Kz") ;
	// wait for the ack car '#'
	if (nexstar_listen() == 2)
	{	// echo response received 

		if (buffer[0] == 'z')
		{
			ret = TRUE ;
			screen_telescope_status("OK") ;	
#if DEBUG
			printf("nexstar_echo|ACK received\n") ;
#endif
		}
		
	}	// echo response received
	else
	{
		screen_telescope_status("--") ;
	}
	
	return ret ;
}


/*
 --------------------------------------------------------------
  * Function name :  nexstar_is_alignment_complete
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * 
  * 
  * 
  * 
  * 
  *
 --------------------------------------------------------------
 */
int nexstar_is_alignment_complete()
{
	int ret ;

	nexstar_send_to_telescope("J") ;

	if (nexstar_listen() == 2)
	{	//  
#if DEBUG
//		printf("nexstar_is_alignment_complete|%s\n",buffer[0]?"Yes":"No") ;
#endif		
		ret = buffer[0] ;
	}	// 

	return ret ;
} 


/*
 --------------------------------------------------------------
  * Function name :  nexstar_is_goto_in_progress
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * 
  * 
  * 
  * 
  * 
  *
 --------------------------------------------------------------
 */
int nexstar_is_goto_in_progress()
{
	int ret = FALSE ;

	nexstar_send_to_telescope("L") ;
	
	if (nexstar_listen() == 2)
	{	// ack car '#' 
#if DEBUG
		printf("nexstar_is_goto_in_progress|ACK received\n") ;
#endif		
	}	// ack car '#'

	return ret ;
}



/*
 --------------------------------------------------------------
  * Function name :  nexstar_cancel_goto
  * Parameters    :  none                                        
  * Description   :                                              
  * Description   :                                              
  * 
  * 
  * 
  * 
  * 
  * 
  *
 --------------------------------------------------------------
 */
void nexstar_cancel_goto()
{
	nexstar_send_to_telescope("M") ;
	
	if (nexstar_listen() == 1)
	{	// ack car '#' 
#if DEBUG
		printf("nexstar_cancel_goto|ACK received\n") ;
#endif		
	}	// ack car '#'
} 
