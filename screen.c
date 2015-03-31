/*+--------------------------------------------------------------+
  | SYSTEM INCLUDES                                              |
  +--------------------------------------------------------------+*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h> 



/*+--------------------------------------------------------------+
  | SPECIFIC INCLUDES                                            |
  +--------------------------------------------------------------+*/
#include "debug.h"		//file where all debug const are defined
#include "screen.h"

/*+--------------------------------------------------------------+
  | LOCAL CONSTANTS                                              |
  +--------------------------------------------------------------+*/
#define MAX_STR_DEBUG 80
#define NB_LG_DEBUG	20

#define ESCAPE 27 
#define CLEAR "[2J" 


/*+--------------------------------------------------------------+
  | GLOBAL VARIABLES                                             |
  +--------------------------------------------------------------+*/
  



/*+--------------------------------------------------------------+
  | LOCAL VARIABLES                                             |
  +--------------------------------------------------------------+*/

char str_date[10] ;
char str_time[10] ;
char str_telescope_status[10] ;
char str_gps_status[10] ;
char str_gps_longitude[40] ;
char str_gps_lattitude[40] ;




int row_debug ;
int index_message ;
char ld[NB_LG_DEBUG][MAX_STR_DEBUG] ;

/*+--------------------------------------------------------------+
  | Function name :  screen_debug
  | Parameters    :  none
  | Description   :  
  | Description   :                     |
  +--------------------------------------------------------------+*/
void trace(char string[])
{
	int i ;
	char log_string[80] ;


	index_message++;

	if (row_debug < NB_LG_DEBUG) 
	{
		row_debug++ ;
	}
	else
	{
		for (i=0; i<NB_LG_DEBUG-1; i++)
		{
			strncpy(ld[i], ld[i+1], MAX_STR_DEBUG) ;
		}
	}
	sprintf(log_string, "%s|%02d|%s",str_time,index_message, string) ;
	strncpy(ld[row_debug-1], log_string, MAX_STR_DEBUG) ; 
	

}


/*+--------------------------------------------------------------+
  | Function name :  screen_init
  | Parameters    :  none
  | Description   :  
  | Description   :                     |
  +--------------------------------------------------------------+*/
void screen_init()
{
	int i ;
	for (i=0; i<NB_LG_DEBUG; i++)
	{
		ld[i][0] = '\0' ;
	}
	row_debug = 0 ;
    index_message = 0 ;
    
	sprintf (str_gps_status, "Unlocked") ;
	sprintf (str_date, "--/--/--") ;
	sprintf (str_time, "--:--:--") ;

}



/*+--------------------------------------------------------------+
  | Function name :  screen_set_date
  | Parameters    :  none
  | Description   :  
  | Description   :                     |
  +--------------------------------------------------------------+*/
void screen_set_date(int day, int month, int year)
{
	sprintf (str_date, "%02d/%02d/%02d", day, month, year) ;
	
}


/*+--------------------------------------------------------------+
  | Function name :  screen_set_time
  | Parameters    :  none
  | Description   :  
  | Description   :                     |
  +--------------------------------------------------------------+*/
void screen_set_time(int hour, int minute, int second)
{
	sprintf (str_time, "%02d:%02d:%02d", hour, minute, second) ;
}


/*+--------------------------------------------------------------+
  | Function name :  screen_gps_status
  | Parameters    :  none
  | Description   :  
  | Description   :                     |
  +--------------------------------------------------------------+*/
void screen_gps_status(char status[])
{
	strncpy(str_gps_status, status, sizeof(str_gps_status)) ;
	
}

void screen_gps_lattitude(char lattitude[])
{
	strncpy(str_gps_lattitude, lattitude, sizeof(str_gps_lattitude)) ;
	
}

void screen_gps_longitude(char longitude[])
{
	strncpy(str_gps_longitude, longitude, sizeof(str_gps_longitude)) ;
	
}

/*+--------------------------------------------------------------+
  | Function name :  screen_gps_status
  | Parameters    :  none
  | Description   :  
  | Description   :                     |
  +--------------------------------------------------------------+*/
void screen_telescope_status(char status[])
{
	strncpy(str_telescope_status, status, sizeof(str_telescope_status)) ;
	
}


/*+--------------------------------------------------------------+
  | Function name :  screen_init
  | Parameters    :  none
  | Description   :  
  | Description   :                     |
  +--------------------------------------------------------------+*/
void *p_screen_display_server(void * arg)
{

	while(1)
	{ // update screen forever

		// new page
		printf("%c%s", ESCAPE,CLEAR); 

		// line n°1
		printf ("GPS: %s\t\t\t\t| Telescope link: %s\n", str_gps_status,str_telescope_status) ;
		// line n°2
		printf ("date: %s  time: %s UTC\t|       Humidity: %02f \%\n", str_date, str_time,0) ;
		// line n°3
		printf ("%s\t\t|    Temperature: %02f °C\n",str_gps_lattitude,0) ;
		printf ("%s\t\t|         DewNot: %03d\ %\n",str_gps_longitude,0) ;
		// last line
		printf ("----------------------------------------------------------------\n") ;

		int i ;
		for (i=0; i<NB_LG_DEBUG; i++)
		{
			printf("%02d   %s\n",i,ld[i]) ;
		}
		sleep(1) ;
		//delay(1000) ;
	} // update screen forever
	pthread_exit (0);

}

