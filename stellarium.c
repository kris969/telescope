/*
 * sans titre.c
 * 
 * Copyright 2013  <pi@raspberrypi>
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


#define DEBUG 0
#define P_RATIO 4294967296L

#define FALSE 0
#define TRUE 1



/*+--------------------------------------------------------------+
  | SYSTEM INCLUDES                                              |
  +--------------------------------------------------------------+*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>


/*+--------------------------------------------------------------+
  | SPECIFIC INCLUDES                                            |
  +--------------------------------------------------------------+*/
#include "debug.h"		//file where all debug const are defined
#include "stellarium.h"
#include "nexstar.h"
#include "screen.h"



#define PORT	8085
#define BUFLEN	100		// TODO definir la bonne valeur de cette k
#define SOCKET_ERROR -1 // TODO to see why it is unknowed ???
#define INVALID_SOCKET -1 // TODO to see why it is unknowed ???


static 	ST_SSTELLARIUM	buffer_to_send_to_stellarium ; // buffer data to transmit to stellarium

struct sockaddr_in   addr_server, addr_client;
unsigned int addr_client_size ;

int     ss_id;	// Socket id du server
int		sc_id;	// Socket id du client

static	char buf[BUFLEN];



/*+--------------------------------------------------------------+
  | Function name :  stellarium_begin                            |
  | Parameters    :  none                                        |
  | Description   :  initialize tcp server and start waiting     |
  | Description   :  connexion from stellarium                   |
  +--------------------------------------------------------------+*/
void *p_stellarium_server(void * arg)
{
    
	//Create a new TCP socket
	if ((ss_id=socket(AF_INET, SOCK_STREAM, 0))==-1)
	{
		perror("p_stellarium_server|socket");
		pthread_exit(0);
	}
	else
	{
		{
			char tmp_str[80] ;
			
			sprintf(tmp_str,"p_stellarium_server| open socket %d ==> ok", ss_id) ;
			trace(tmp_str) ;
		}
	}
	
	
	// Bind the socket with the port
	memset((char *) &addr_server, 0, sizeof(addr_server));
	
	addr_server.sin_family       =  AF_INET;
	addr_server.sin_port         =  htons(PORT);
	addr_server.sin_addr.s_addr  =  htonl(INADDR_ANY);
	
/*
 * 	TODO work on this for not been in condition of faulty restart
	if (setsockopt(ss_id, SOL_SOCKET, SO_REUSEADDR, (void*)NULL, (socklen_t)NULL) == -1 )
	{
		perror("p_stellarium_server|setsockopt");
	}
*/
	int bind_nok = TRUE ;
	do
	{
		if (bind(ss_id, (const struct sockaddr *)&addr_server, sizeof(addr_server))==-1)
		{
			sleep(2);
		}
		else
		{
			bind_nok = FALSE ;
		}
	} while (bind_nok) ;

	/* socket listen */
		{
			char tmp_str[80] ;
			
			sprintf(tmp_str,"p_stellarium_server| listen on port %d",PORT) ;
			trace(tmp_str) ;
		}


	if (listen(ss_id,1) == SOCKET_ERROR) 
	{
		perror("p_stellarium_server|Listen");
		pthread_exit (0);
	}


	addr_client_size = sizeof(addr_client);
	// infinite loop waiting for data.
	while(1)
	{
		{
			char tmp_str[80] ;
			
			sprintf(tmp_str,"p_stellarium_server| waiting for connexion port %d...", PORT) ;
			trace(tmp_str) ;
		}

		sc_id = accept(ss_id,(struct sockaddr *)&addr_client, &addr_client_size);
		if (sc_id == INVALID_SOCKET	)
		{
			perror("p_stellarium_server|accept");
			pthread_exit (0);
			break ;
		}
		else
		{
			char tmp_str[80] ;
			
			sprintf(tmp_str,"p_stellarium_server| client connected on socket %d", sc_id) ;
			trace(tmp_str) ;
		}
		
		 
		 
		while (1)
		{
			memset(buf, 0, sizeof(buf));
			int nb = recv(sc_id, buf, sizeof(buf), 0);
			if (nb >0)
			{

#if DEBUG				
				printf("p_stellarium_server| %d bytes received on client socket:%d\n", nb, sc_id) ;
#endif
				int i;
				for (i=0; i<nb; i++)
				{
					printf("%02X ",buf[i]) ;
				}
				printf("\n") ;
				stellarium_cs_message(buf) ;
			}
		}
	}
	
	// release the socket.
	close(ss_id);
#if DEBUG
	printf("p_stellarium_server| close the socket %d\n", ss_id) ;
#endif
	
	pthread_exit (0);
}



void tcp_sc_message(char* p_buffer, int size)
{
	int nb_bytes_sent = send(sc_id,p_buffer,size,0 );
	if (nb_bytes_sent == SOCKET_ERROR) 
	{
//		perror("tcp_sc_message|send");
		
		// traitement de l'erreur
	}
	else
	{
		//printf("tcp_sc_message| %d bytes sent\n",nb_bytes_sent);
	}

}	


/*
If the Get RA/DEC command returns 34AB,12CE then the DEC value is 12CE in hexadecimal. As a percentage
of a revolution, this is 4814/65536 = 0.07346. To calculate degrees, simply multiply by 360, giving a value of
26.4441 degrees.


1° 11930465
18,446,744,073,709,551,615
* 

*/



/*
 * 
 * 
 */ 
void stellarium_cs_message(char* buffer)
{
	PST_CSTELLARIUM	p_stellarium ; // pointeur sur bloc data reçu de stellarium

	
	p_stellarium = (PST_CSTELLARIUM) buffer ;
#if DEBUG 	
	printf("Sizeof ST_CSTELLARIUM : %d\n", sizeof(ST_CSTELLARIUM)) ;
	
	printf("size offset : %d taille: %d\n", (void*)&p_stellarium->message_length - (void*)p_stellarium, sizeof(p_stellarium->message_length)) ;
	printf("offset type : %d taille: %d\n", (void*)&p_stellarium->type - (void*)p_stellarium, sizeof(p_stellarium->type)) ;
	printf("offset time : %d taille: %d\n", (void*)&p_stellarium->time - (void*)p_stellarium, sizeof(p_stellarium->time)) ;
	printf("offset ra   : %d taille: %d\n", (void*)&p_stellarium->ra - (void*)p_stellarium, sizeof(p_stellarium->ra)) ;
	printf("offset dec  : %d taille: %d\n", (void*)&p_stellarium->dec - (void*)p_stellarium, sizeof(p_stellarium->dec)) ;


	int i ;
	char* buf;
	buf = (char*)p_stellarium ;
	for (i=0; i<20; i++)
	{
		printf("%02X ",buf[i]) ;
	}
	printf("\n");
	

	
	printf("--------------------------\n") ;
	short size = p_stellarium->message_length ;
	printf("Taille paquet : %d\n", size) ;
	printf("--------------------------\n") ;
#endif
	
	nexstar_goto_precise_ra_dec(p_stellarium->ra, p_stellarium->dec) ;
}


	
void stellarium_sc_message(int ra, int dec)
{	
	
	// les valeurs données en entrée sont en degrés
	
	buffer_to_send_to_stellarium.message_length = sizeof(ST_SSTELLARIUM) ;
	buffer_to_send_to_stellarium.type = 0 ;
	buffer_to_send_to_stellarium.time = 0 ;
	buffer_to_send_to_stellarium.ra = ra ;
	buffer_to_send_to_stellarium.dec = dec ;
	buffer_to_send_to_stellarium.status = 0 ;
	
	tcp_sc_message((char*) &buffer_to_send_to_stellarium, sizeof(ST_SSTELLARIUM)) ;
	
}







/*
 * convert a raw value as it commes from stellarium in data buffre
 * to degrees
 * 
 * 
 */
float raw_to_degrees(long raw)
{	

	long x = raw ;
	// k is the constant to use to get the percentage of a revolution
	double k = 4294967296L ;	


	float y = x * 360 ; 
	float z = y / k ;

	
#if DEBUG	
	//printf("DEBUG | raw: %0.0f ==> %0.2f degrees\n", x, z) ;			
#endif
	return z ;
}


/*
 * convert a raw value as it commes from stellarium in data buffre
 * to degrees
 * 
 * 
 */
long degrees_to_raw(float degrees)
{	

	long x = degrees ;
	// k is the constant to use to get the percentage of a revolution
	double k = 4294967296L ;	

	float y = x * k ;
	float z = y / 360 ;
	
#if DEBUG	
	//printf("DEBUG | %0.2f degrees ==> raw: %0.0f\n", x, z) ;			
#endif
	return (long) z ;
}



/*
 * convert a value in degrees to hour(s)
 * 
 * 
 * 
 */

int degrees_to_hour(float rad)
{
	int hour ;
		
	hour = (rad / 360) * 24 ;
	
	return hour ;	
}


