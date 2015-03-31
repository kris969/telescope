// main.c
//
// 


/*+--------------------------------------------------------------+
  | SYSTEM INCLUDES                                              |
  +--------------------------------------------------------------+*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

/*+--------------------------------------------------------------+
  | SPECIFIC INCLUDES                                            |
  +--------------------------------------------------------------+*/
#include "debug.h"		//file where all debug const are defined
#include "screen.h"
#include "stellarium.h"
#include "gps.h"
#include "nexstar.h"
#include "automate.h"
#include "dht11.h"

/*+--------------------------------------------------------------+
  | LOCAL CONSTANTS                                              |
  +--------------------------------------------------------------+*/





/*+--------------------------------------------------------------+
  | GLOBAL VARIABLES                                             |
  +--------------------------------------------------------------+*/
  



/*+--------------------------------------------------------------+
  | LOCAL VARIABLES                                             |
  +--------------------------------------------------------------+*/
static pthread_t th_stellarium ;
static pthread_t th_gps ;
static pthread_t th_screen_display ;
static pthread_t th_dht11 ;
//static pthread_t th_dew_not ;
//static pthread_t th_camera ;
//static pthread_t th_focus ;
void *ret;


int main(int argc, char **argv)
{

//#if DEBUG 
// just to control data size on RPI
	printf("Sizeof short : %d\n", sizeof(short)) ;
	printf("Sizeof int : %d\n", sizeof(int)) ;
	printf("Sizeof double : %d\n", sizeof(double)) ;
//#endif


//	long ra, dec ;
//	nexstar_get_precise_ra_dec(&ra, &dec) ;
	

	screen_init();
	// screen display server is launched in a dedicated thread 
	if (pthread_create (&th_screen_display, NULL, p_screen_display_server, "1") < 0) 
	{ 
		fprintf(stderr, "pthread_create error for thread stellarium_server\n"); exit (1);
	} 
	else
	{
		printf("stellarium_server launched in a thread ==> Ok !\n");
	}
 
	// stellarium server is launched in a dedicated thread 
	if (pthread_create (&th_stellarium, NULL, p_stellarium_server, "1") < 0) 
	{ 
		fprintf(stderr, "pthread_create error for thread stellarium_server\n"); exit (1);
	} 
	else
	{
		printf("stellarium_server launched in a thread ==> Ok !\n");
	}
	
	  
	// gps server is launched in a dedicated thread 
	// stellarium server is launched in a dedicated thread 
	if (pthread_create (&th_gps, NULL, p_gps_server, "1") < 0) 
	{ 
		fprintf(stderr, "pthread_create error for thread gps_server\n"); exit (1);
	} 
	else
	{
		printf("gps_server launched in a thread ==> Ok !\n");
	}

  
  // dewnot process is launched in a dedicated thread 
  
	if (pthread_create (&th_dht11, NULL, p_dht11, "1") < 0) 
	{ 
		fprintf(stderr, "pthread_create error for thread dht11_driver\n"); exit (1);
	} 
	else
	{
		printf("dht11_driver launched in a thread ==> Ok !\n");
	}

  
  // camera process is launched in a dedicated thread 

	automate_init() ;
		
	while(1)
	{
		
		// TODO add listen for command control over ssh terminal
		//sleep(1);


		automate() ;
		
	}
	fprintf(stderr,"stellarium_server\n"); 
	
	
	(void)pthread_join (th_screen_display, &ret);
	(void)pthread_join (th_stellarium, &ret);
	(void)pthread_join (th_gps, &ret);
	(void)pthread_join (th_dht11, &ret);
	
    return 0;
}


