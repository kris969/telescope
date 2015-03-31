/*
 * 0 = Off
 * 1 = Alt/Az
 * 2 = EQ North
 * 3 = EQ South 
*/

#define TRACKING_OFF		0
#define TRACKING_EQ_NORTH	2

void nexstar_init(void);
   
void nexstar_send_to_telescope(char* cmd);
int nextar_echo_received(void) ;
int nextar_ack_received(void);

void nexstar_get_version(void);
int nexstar_get_location(void);
int nexstar_get_time(void);
int nexstar_echo(void);
int nexstar_is_alignment_complete(void);
int nexstar_is_goto_in_progress(void);



void nexstar_cancel_goto(void);
int nexstar_get_model(void);

void nexstar_set_time(int day, int month, int year, int hour, int minute, int second);
void nexstar_set_location(int lad, int lam, int las, int ns, int lod, int lom, int los, int ew);

int nexstar_get_tracking_mode();
void nexstar_set_tracking_mode(int mode);

int nexstar_is_alignment_complete(void);

int nexstar_get_precise_ra_dec(int* p_ra, int* p_dec);
void nexstar_goto_precise_ra_dec(int ra, int dec);










