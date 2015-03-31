/*+--------------------------------------------------------------+
  | SYSTEM INCLUDES                                              |
  +--------------------------------------------------------------+*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <pthread.h>
#include <wiringPi.h>



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
#define MAX_TIME 84
#define DHT11PIN 7







/*+--------------------------------------------------------------+
  | GLOBAL VARIABLES                                             |
  +--------------------------------------------------------------+*/
float dht11_temp ;
float dht11_humidity ;
int dht11_data_valid ;


/*+--------------------------------------------------------------+
  | LOCAL VARIABLES                                             |
  +--------------------------------------------------------------+*/

static int dht11_val[5];
static uint8_t bits[MAX_TIME] ;

static uint8_t laststate, currentstate;
static uint8_t counter;
static uint8_t i, j;
    
void dht11_read(void) ;


/*+--------------------------------------------------------------+
  | Function name 	: p_dht11
  | Parameters    	:  
  | Description   	: Driver for collecting Temp and Humidity
  | Description   	:  
  * Return			: TIMEOUT if not received character "#" after 10 seconds of waiting
  * 				: ACK     if only one character "#" has been received
  * 				: the number of characters received when frame of more than one charaxter
  +--------------------------------------------------------------+*/
void *p_dht11(void *arg)
{
	while(1) dht11_read() ;

	pthread_exit (0);

}


void dht11_read()
{

#if DEBUG
	//printf("dht11|%s", buffer) ;
#endif
			
    for(i=0;i<5;i++) dht11_val[i]=0;
    
    pinMode(DHT11PIN,OUTPUT);
    digitalWrite(DHT11PIN,LOW);
    delay(18);
    
    digitalWrite(DHT11PIN,HIGH);
    delayMicroseconds(40);
    
    pinMode(DHT11PIN,INPUT);
    laststate = HIGH ;
    for(i=0;i<MAX_TIME;i++)
    {
        counter=0;
        do
        {
            currentstate = digitalRead(DHT11PIN) ;
            delayMicroseconds(1);
            
            if(++counter==255) break;
        } while (currentstate == laststate) ;

        laststate = currentstate ;
        bits[i] = counter ;
        //if(counter==255) break;
    }
    
    // Add x us to each sample, to get real timing value
    for (i=0; i<MAX_TIME; i++) bits[i] = (bits[i] * 2 )   ; 
 
    for (i=0; i<MAX_TIME; i++) 
    {

        //if((i>=4)&&(i%2!=0)) printf("i:%d c:%d\n",i,bits[i]) ;
 
        // first 4 transistions are ignored
        if((i>=4)&&(i%2==0))
        {
            //printf("i:%02d c:%03d ",i,bits[i]) ;
            dht11_val[j/8] <<= 1 ;            
            if(bits[i]>40) dht11_val[j/8]|=1;


            j++;
        }
    }
	//printf("j: %02d\n",j);

    // verify cheksum and print the verified data
    if((j>=40)&&(dht11_val[4]==((dht11_val[0]+dht11_val[1]+dht11_val[2]+dht11_val[3])& 0xFF)))
    {
		dht11_temp = dht11_val[0] + dht11_val[1] / 100 ;
		dht11_humidity = dht11_val[0] + dht11_val[1] / 100 ;
		dht11_data_valid = 1;
//        printf("humidity-relative:%d.%d|temperature-celsius:%d.%d\n",dht11_val[0],dht11_val[1],dht11_val[2],dht11_val[3]);
    }
    else 
    {
//        printf("Invalid Data\n");
		dht11_data_valid = 0;
    }
			
	
	
}



 

