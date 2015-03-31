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
 
 
 #include <sys/time.h>
 



/******************
 * The format of the time commands is: QRSTUVWX, where:
Q is the hour (24 hour clock).
R is the minutes.
S is the seconds.
T is the month.
U is the day.
V is the year (century assumed as 20).
W is the offset from GMT for the time zone. Note: if zone is negative, use 256-zone.
X is 1 to enable Daylight Savings and 0 for Standard Time.
 * ******************/



/*
 * Scheme for server→client message

LENGTH (2 bytes, integer): length of the message
TYPE (2 bytes, integer): 0
TIME (8 bytes, integer): current time on the server computer in microseconds
    since 1970.01.01 UT. Currently unused.
RA (4 bytes, unsigned integer): right ascension of the telescope (J2000)
    a value of 0x100000000 = 0x0 means 24h=0h,
    a value of 0x80000000 means 12h
DEC (4 bytes, signed integer): declination of the telescope (J2000)
    a value of -0x40000000 means -90degrees,
    a value of 0x0 means 0degrees,
    a value of 0x40000000 means 90degrees
STATUS (4 bytes, signed integer): status of the telescope, currently unused.
    status=0 means ok, status<0 means some error

Scheme for client→server message

LENGTH (2 bytes, integer): length of the message
TYPE (2 bytes, integer): 0
TIME (8 bytes, integer): current time on the server computer in microseconds
    since 1970.01.01 UT. Currently unused.
RA (4 bytes, unsigned integer): right ascension of the telescope (J2000)
    a value of 0x100000000 = 0x0 means 24h=0h,
    a value of 0x80000000 means 12h
DEC (4 bytes, signed integer): declination of the telescope (J2000)
    a value of -0x40000000 means -90degrees,
    a value of 0x0 means 0degrees,
    a value of 0x40000000 means 90degrees
 */
	


// type définissant tous les paramètres d'un transfert (sauvegarde ou restauration)
typedef struct
{
	short message_length	__attribute__((packed)) ;	// Paquet size
	short type				__attribute__((packed));	// Always set to 0
	double time 			__attribute__((packed)) ; 	//(8 bytes, integer): current time on the server computer in microseconds
														// since 1970.01.01 UT. Currently unused.
	int ra 					__attribute__((packed));	// ascension droite
	int dec 				__attribute__((packed)) ;	// déclinaison
	
}	ST_CSTELLARIUM, *PST_CSTELLARIUM ;


typedef struct
{
	short message_length 	__attribute__((packed)) ;	// Paquet size
	short type 				__attribute__((packed));	// Always set to 0
	double time 			__attribute__((packed)) ; 	//(8 bytes, integer): current time on the server computer in microseconds
														// since 1970.01.01 UT. Currently unused.
	int ra 					__attribute__((packed));	// ascension droite
	int dec 				__attribute__((packed)) ;	// déclinaison	unsigned int	dec ;			// déclinaison
	int status 				__attribute__((packed)); 	// status of the telescope, currently unused.
}	ST_SSTELLARIUM, *PST_SSTELLARIUM ;

typedef struct
{
	double ra ;
	double dec ;
}	ST_RPI, *PST_RPI ;


void *p_stellarium_server(void * arg);
void stellarium_cs_message(char* buffer);
void stellarium_sc_message(int ra, int dec);

void short_swap(short* p_short);
void int_swap(int* p_int);
void double_swap(double* p_double);

//float raw_to_degrees(long raw);
//long degrees_to_raw(float degrees);
//int degrees_to_hour(float rad);






