
void screen_init(void);
void trace(char string[]);


void screen_gps_status(char status[]) ;
void screen_set_date(int day, int month, int year);
void screen_set_time(int hour, int minute, int second) ;
void screen_gps_lattitude(char lattitude[]);
void screen_gps_longitude(char longitude[]);
void screen_telescope_status(char status[]);

void *p_screen_display_server(void * arg);
