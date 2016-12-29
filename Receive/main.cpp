#include "mbed.h" //mbed library
#include "xbee.h" // xbee library
#include "C12832.h" // LCD library
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

xbee xbee1(D1,D0,D3); //Initalise xbee_lib varName(rx,tx,reset)
DigitalOut rst1(D3);//xbee reset 
Serial pc(USBTX, USBRX); //Initalise serial 
C12832 shld_lcd (D11, D13, D12, D7, D10); // LCD 


 
//will receive data from sending xbee 
int main()
{
    // reset the xbees (at least 200ns)
    rst1 = 0;
    wait_ms(1); 
    rst1 = 1;
    wait_ms(1);
    
    //Establish a variable to receive data from End Device
    //Max buffer is 202
    char read_data[202]; //Xbee buffer size is 202 bytes
	char tmp[256];

    //Setup LCD screen
    shld_lcd.cls();      
    shld_lcd.locate(0,1);
    
    while(1) {
        //Recieve data from xbee
        
        xbee1.RecieveData(read_data,0);
        
        shld_lcd.printf("Recieved data %s \n");
        //convert back to integer (remove extra data)
        int x;
        tmp[0]='\0';
        while (sscanf(read_data,"%[^0123456789]%s",tmp,read_data)>1||sscanf(read_data,"%d%s",&x,read_data))
            {
                if (tmp[0]=='\0')
                    {
                        printf("%d\r\n",x);
                        shld_lcd.printf("Sent over serial\n");
                    }
                 tmp[0]='\0';
            }
        
    }
}
