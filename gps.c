/*+--------------------------------------------------------------+
  | SYSTEM INCLUDES                                              |
  +--------------------------------------------------------------+*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringSerial.h>


/*+--------------------------------------------------------------+
  | SPECIFIC INCLUDES                                            |
  +--------------------------------------------------------------+*/
#include "debug.h"		//file where all debug const are defined
#include "const.h"
#include "screen.h"



/*+--------------------------------------------------------------+
  | LOCAL CONSTANTS                                              |
  +--------------------------------------------------------------+*/
#define DEBUG 0

#define DEVICE "/dev/ttyAMA0"
#define UART_GPS_SPEED 115200
#define	TIMEOUT	10	// 10 secondes de timeout

#define BUFFER_SIZE	100		// TODO definir la bonne valeur de cette k







/*+--------------------------------------------------------------+
  | GLOBAL VARIABLES                                             |
  +--------------------------------------------------------------+*/
	int day ;
	int month ;
	int year;

	int	hour ;
	int minute ;
	int second ;
	int msecond ;
	
	int lattitude_degre;
	int lattitude_minute;
	int lattitude_seconde;
	int north_south ;
	int longitude_degre ;
	int longitued_minute ;
	int longitued_seconde ;
	int east_west ;


/*+--------------------------------------------------------------+
  | LOCAL VARIABLES                                             |
  +--------------------------------------------------------------+*/
static int fd ; //uart file descriptor
static char buffer[BUFFER_SIZE] ;
static int	gps_locked ;



int gps_is_locked()
{
	return gps_locked ;
}

/*+--------------------------------------------------------------+
  | Function name 	: gps_listen
  | Parameters    	:  
  | Description   	: wait for gps frame
  | Description   	:  
  * Return			: TIMEOUT if not received character "#" after 10 seconds of waiting
  * 				: ACK     if only one character "#" has been received
  * 				: the number of characters received when frame of more than one charaxter
  +--------------------------------------------------------------+*/
void *p_gps_server(void * arg)
{

	int i_buffer ;
	int data_length ;
	int nb_entry ;
	char cmd[8] ;

	float tmp ;
	float f_lam, f_lom ;
	char valid_frame ;
	char c_N_S ;
	char c_E_W ;

	gps_locked = FALSE ,
	trace("gps_server is starting...") ;


	fd = serialOpen(DEVICE, UART_GPS_SPEED ) ;
    if (fd == -1)
    {
		perror("gps_server|serialOpen");
		pthread_exit (0);
	}
	else
    { // serial port is open
#if DEBUG2
		printf("gps_server|uart open on %s, uart_fd: %d\n", DEVICE, fd) ;
#endif


 		// Query for version firmware
		serialPrintf (fd,"$PMTK605*31\r\n") ;		
		sleep(1) ;
		
		
		// restart GPS module 
		screen_gps_status("Restart") ;
		trace("gps_server|Restart the GPS module") ;

		serialPrintf (fd,"$PMTK101*32\r\n") ; 	// hot restart

		serialPrintf (fd, "$PMTK103*30\r\n") ; 	// cold restart
		serialPrintf (fd, "$PMTK104*37\r\n") ; 	// full cold restart
		trace("gps_server|GPS module restarting...") ;
		sleep(4) ; // lets time to module to restart

		// Set NMEA port update rate to 1Hz
#if DEBUG		
		printf("gps_server|Set NMEA port update rate to 1Hz\n") ;
#endif
		serialPrintf (fd,"$PMTK220,1000*1F\r\n") ;	
		sleep(4) ; // lets time to module to restart
		

    } // serial port is open


	while(1)
	{ // waiting for frame coming from gps
		
		data_length = serialDataAvail(fd) ;
		if (data_length > 0 )
		{ // some data are available

			i_buffer = 0 ;
			char car ;
			do
			{
				car = serialGetchar(fd) ;
				buffer[i_buffer++] = car ;
			}
			while (car != LF && i_buffer < BUFFER_SIZE-1) ;
			

			// we are looking for GPRMC frame that gives all information used for telescope
			// also consider PMTK frame taht gives information about firmware and options
			if (    (strncmp(buffer,"$PMTK",5) == 0)
			   || 	(strncmp(buffer,"$GPRMC",6) == 0)
			   )
			{ // it's a GPRMC frame
				buffer[i_buffer] = '\0' ;
#if DEBUG
				printf("gps_server|%s", buffer) ;
#endif
			}
			
			
//          $GPRMC, 110822.000, A, 4714.8839,N,00218.8194, W,1.22,4.26,100814,,,A*7A		
			if (strncmp(buffer,"$GPRMC",6) == 0)
			{
				nb_entry = sscanf(buffer,  "%6[^,]%*c%02d%02d%02d%*c%03d%*c%c%*c%02d%f%*c%c%*c%03d%f%*c%c%*c%*8[^,]%*c%*8[^,]%*c%02d%02d%02d",
									cmd,
									&hour,								
									&minute,
									&second,
									&msecond,
									&valid_frame,
									&lattitude_degre,
									&f_lam,
									&c_N_S,
									&longitude_degre,
									&f_lom,
									&c_E_W,
									&day,
									&month,
									&year);
									
						
				

					
				if ( nb_entry != 15)
				{ // trame reçue non conforme
#if DEBUG
					printf("gps_server|ERROR illegal number of entries: 19 expected, %d found !\n",nb_entry) ;
#endif
				} // trame reçue non conforme
				else
				{ // trame reçue conforme
					if (valid_frame == 'A')
					{ // GPS is locked
						screen_gps_status("Locked") ;
						
						screen_set_date(day, month, year) ;
						screen_set_time(hour, minute, second) ;
						lattitude_minute = (int) f_lam ;
							
						tmp = (f_lam - (int) f_lam) ;
						tmp = tmp * 100 ;
						tmp = (int) tmp ;
							

						lattitude_seconde = (int) 60 * (int) tmp / 100 ;
							
						longitued_minute = (int) f_lom ;
						tmp = (f_lom - (int) f_lom) ;
						tmp = tmp * 100 ;
						tmp = (int) tmp ;
						longitued_seconde = (int) 60 * (int) tmp / 100 ;

						north_south = (c_N_S =='N')?0:1 ;
						east_west = (c_E_W =='E')?0:1 ;
							
							
						if (!gps_locked)
						{ // GPS just locked
							gps_locked = TRUE ;	
							trace("gps_server|GPS is locked") ;
							
							char tmp_str[40] ;
							
							sprintf(tmp_str,"lattitude: %c %02d° %02d' %02d\"", c_N_S, lattitude_degre, (int) lattitude_minute, lattitude_seconde) ;
							screen_gps_lattitude(tmp_str);
							sprintf(tmp_str,"longitude: %c %02d° %02d' %02d\"", c_E_W, longitude_degre, (int) longitued_minute, longitued_seconde) ;
							screen_gps_longitude(tmp_str);
						} // GPS just locked
					
					}  // GPS is locked
					else
					{
						gps_locked = FALSE ;
						trace("gps_server|GPS is unlocked") ;
						screen_gps_status("Unlocked") ;
					}
				} // trame reçue conforme
#if DEBUG
				printf("--------------------------------------------------------------------------------\n") ;
#endif
			} // it's a GPRMC frame


#if DEBUG					
/*
			printf("gps_server| received (i_buffer:%d)==> ",i_buffer) ;
			int i ;
			for (i=0; i<i_buffer; i++)
			{
				printf(" %02x", buffer[i]);
			}
			printf("\n");
*/
#endif				
		} // some data are available
	} // waiting for frame coming from gps
	
	
	pthread_exit (0);
}

