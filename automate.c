/*+--------------------------------------------------------------+
  | SYSTEM INCLUDES                                              |
  +--------------------------------------------------------------+*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


/*+--------------------------------------------------------------+
  | SPECIFIC INCLUDES                                            |
  +--------------------------------------------------------------+*/
#include "debug.h"		//file where all debug const are defined
#include "automate.h"
#include "nexstar.h"
#include "stellarium.h"
#include "gps.h"
#include "screen.h"


/*+--------------------------------------------------------------+
  | LOCAL CONSTANTS                                              |
  +--------------------------------------------------------------+*/
#define DEBUG 0




#define NEXSTAR_CMD_ECHO	"KA"	//Echo : useful to check communication  | cmd : “K” & chr(x) | reply : chr(x) & “#” 
#define NEXSTAR_ECHO		'A'

typedef  enum
{
	T_INIT,
	T_TELESCOPE_NOT_LINKED,
	T_WAIT_TELESCOPE_LINKED,
	T_WAIT_GPS_LOCKED,
	T_SET_NEXSTAR_LOC_TIME,
	T_NEXSTAR_WAIT_FOR_ALIGNMENT,
	T_NEXSTAR_ALIGNMENT_OK,
	
	T_LAST
} T_ETAT ;


/*+--------------------------------------------------------------+
  | GLOBAL VARIABLES                                             |
  +--------------------------------------------------------------+*/




/*+--------------------------------------------------------------+
  | LOCAL VARIABLES                                             |
  +--------------------------------------------------------------+*/

static T_ETAT etat ;










/*+--------------------------------------------------------------+
  | Function name :  automate_init
  | Parameters    :  none                                        |
  | Description   :  the automate for the
  * Description   :                     |
  +--------------------------------------------------------------+*/
void automate_init()
{
	etat = T_INIT ;

}




/*+--------------------------------------------------------------+
  | Function name :  automate                                    |
  | Parameters    :  none                                        |
  | Description   :  the automate for the      |
  | Description   :                     |
  +--------------------------------------------------------------+*/
void automate()
{
	switch (etat)
	{
		case T_INIT :
		{ // nothing to do
			// program is starting, step to "TEST" state
			nexstar_init(); // TODO consider erro on open port
			etat = T_TELESCOPE_NOT_LINKED ;
#if DEBUG			
			printf("automate|init\n");
#endif
		} // nothing to do
		break ;
		
		case T_TELESCOPE_NOT_LINKED :
		{
			trace("automate|telescope not linked") ;
			etat = T_WAIT_TELESCOPE_LINKED ;
		}
		break ;
		
		case T_WAIT_TELESCOPE_LINKED :
		{ // test if communication with telecope is good
			nexstar_get_version() ;

			if (nexstar_echo()) 
			{
				etat = T_WAIT_GPS_LOCKED ;

				trace("automate|link with telescope ok") ;
				trace("automate|waiting for GPS locked...");

			}
		} // test if communication with telecope is good
		break ;

		case T_WAIT_GPS_LOCKED :
		{ // wait for GPS locked
			if (gps_is_locked())
			{
				etat = T_SET_NEXSTAR_LOC_TIME ;
#if DEBUG			
				printf("automate|GPS is locked\n");
#endif		
			}	
		} // wait for GPS locked
		break ;

		case T_SET_NEXSTAR_LOC_TIME :
		{ // set Telescope location and time
			
			nexstar_set_time(day, month, year, hour, minute, second) ;
			
			nexstar_get_time() ;
			nexstar_set_location(lattitude_degre, lattitude_minute, lattitude_seconde, north_south, longitude_degre, longitued_minute, longitued_seconde, east_west) ;

			nexstar_get_location() ;
			//nexstar_get_tracking_mode() ;
			//nexstar_set_tracking_mode(TRACKING_OFF) ;
			etat = T_NEXSTAR_WAIT_FOR_ALIGNMENT ;
			
			trace("automate|Telescope settings is done");
			trace("automate|Waiting for telescope alignment...");
		
	
		} // set Telescope location and time
		break ;

		case T_NEXSTAR_WAIT_FOR_ALIGNMENT :
		{ //
			// if telescope link is broken, go back for waiting telescope linked
			if (!nexstar_echo()) etat = T_TELESCOPE_NOT_LINKED ;
			
			if (nexstar_is_alignment_complete())
			{
				etat = T_NEXSTAR_ALIGNMENT_OK ;

				trace("automate|Telescope alignment is done");
				trace("automate|telescope is ready !") ;
		
			}	
		} // set Telescope location and time
		break ;

		case T_NEXSTAR_ALIGNMENT_OK :
		{ // telescope ready
			// if telescope link is broken, go back for waiting telescope linked
			if (!nexstar_echo()) etat = T_TELESCOPE_NOT_LINKED ;

			int ra, dec ;
			
			nexstar_get_precise_ra_dec(&ra, &dec) ;
			stellarium_sc_message(ra, dec) ;

	
		} // telescope ready
		break ;

		default :
		{
			printf("nexstar_ERROR : bad value for etat : %d\n", etat);
		}
		break ;
	}
}
