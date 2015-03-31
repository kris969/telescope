/*
 * Example :
 * $GPGGA,064951.000,2307.1256,N,12016.4438,E,1,8,0.95,39.9,M,17.8,M,,*65
*/
 

extern	int day ;
extern	int month ;
extern	int year;

extern	int	hour ;
extern	int minute ;
extern	int second ;
extern	int msecond ;
	
extern	int lattitude_degre;
extern	int lattitude_minute;
extern	int lattitude_seconde;
//extern	int lacs;
extern	int north_south ;
extern	int longitude_degre ;
extern	int longitued_minute ;
extern	int longitued_seconde ;
//extern	int locs ;
extern	int east_west ;


int gps_is_locked(void);
void *p_gps_server(void * arg);
