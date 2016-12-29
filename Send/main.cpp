#include "mbed.h" //mbed library 
#include "xbee.h" //xbee library
#include "C12832.h" //lcd library 
#include "MMA7660.h" //acceleromter library 
#include "hcsr04.h" //ultrasonic library
#include <stdio.h>
#include <string.h>


HCSR04  usensor(PTD4,PTD5);//Initalise ultrasonic (trig pin,echo pin)
xbee xbee1(D1,D0,D3); //Initalise xbee_lib  varName(rx,tx,reset)
DigitalOut rst1(D3); // xbee reset
MMA7660 MMA(D14,D15); //accelerometer(SCL, SDA) 
C12832 shld_lcd (D11, D13, D12, D7, D10);  //LCD 
Serial pc(USBTX, USBRX); //Initalise serial 


unsigned int dist; //ultrasonic distance
int changedPosition; //changed position from accelermoter 


#define g(x,y,z) sqrt(x * x + y * y + z * z) //gforce calculation 


//get position of device 
void checkPosition()
{
    changedPosition = 0;
    
    float low = 0.87f;
    float high = 1.098f;
    
    float x = MMA.x();
    float y = MMA.y();
    float z = MMA.z();
    double gf = g(x,y,z);
           
    if( gf < low || gf > high)
    {
        changedPosition = 1; 
        wait(0.5);
    }
    else
    {
        changedPosition =0;
        wait(0.5);
    }
}
int main()
{

    // reset the xbees 
    rst1 = 0;
    wait_ms(1); 
    rst1 = 1;
    wait_ms(1);
    
    //Setup LCD screen
    shld_lcd.cls();      
    shld_lcd.locate(0,1);
    
    char send_data[202]; //Xbee buffer size is 202 bytes
        
    checkPosition();
    
    //infinite loop for whichever position it is in 
    for(;;) 
    {
                    
            //if device position has changed the ultrasonic is activated
            //data will then be sent to the receive mbed
            if(changedPosition == 1)
            {
                 //start ultrasonic; get data 
                usensor.start();
                wait_ms(500); 
                dist=usensor.get_dist_cm();
                  
                
                sprintf(send_data, "%u", dist); //convert to string 
                    
               
                //xbee only receiving 4 characters so make sure it is always 4
				//will be taken out on receving end 
                if((unsigned)strlen(send_data) > 2 && (unsigned)strlen(send_data) <4)
                {
                    send_data[3] = 'C';
                    xbee1.SendData(send_data); //Send data to XBee 
                    shld_lcd.printf("You said:%s \n",send_data);
                }
				
                else if((unsigned)strlen(send_data) > 1 && (unsigned)strlen(send_data) <3)
                {
                    send_data[2] = 'C';
                    send_data[3] = 'M';
                    xbee1.SendData(send_data); //Send data to XBee 
                    shld_lcd.printf("You said:%s \n",send_data);
                }
                    
                else if((unsigned)strlen(send_data) < 2 )
                {
                    send_data[1] = 'C';
                    send_data[2] = 'M';
                    send_data[3] = 'u';
                    xbee1.SendData(send_data); //Send data to XBee 
                    shld_lcd.printf("You said:%s \n",send_data);
                }
                    
                else
                {         
                    xbee1.SendData(send_data); //Send data to XBee 
                    shld_lcd.printf("You said:%s \n",send_data);    
                }
                    
                checkPosition();

            }
                
        //if nothing has changed      
        else if(changedPosition == 0)
            {       
                checkPosition(); 
            }   
    }
}