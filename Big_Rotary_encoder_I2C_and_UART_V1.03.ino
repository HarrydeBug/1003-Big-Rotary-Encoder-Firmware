//Software for Zachtek "Big Rotary Encoder V1R1" 
//I2C Slave arduino that counts pulses in from an optical encoder and reports it over I2C wenever asked by a
//an I2C Master (typically another Arduino)

// To use this program you need run it on the Zachtek "Big Rotary Encoder V1R1" and then connect another arduino over I2C (SDA, SCL and GND should be conencted between the two arduinos.)
//On the other Arduino call on adress "I2CSlaveAddr" and ask for 6 bytes back.
//The optical encoder counts since last call will be returned.
//Encoder input pulses make use Interrupt on the Arduino to make sure no pulses are missed
//An I2C query will trigger an event that reports back a number positive or negative of the number of pulses that havebenn generated since last I2C query
//The Number of pulses will also be sent out on the UART every 50mS
  
//I2C Portions built upon the example I2C code "Wire Master Reader" by Nicholas Zambetti <http://www.zambetti.com>
//Rotary encoder portion built upon the example code by carnevaledaniele http://playground.arduino.cc/Main/RotaryEncoders#Example7
//ZachTec 2016-2017

//The Version of this software is stored in the constant "softwareversion" and is displayed on the Serialport att startup
//For Arduino Pro Mini.

#include <Wire.h>//Use I2C library

//PIN's definition
#define encoder0PinA  2
#define encoder0PinB  3

#define I2CSlaveAddr 8  //I2C adress to act as a slave on CHANGE THIS ADRESS IF YOU WANT TO USE MULTIPLE ENCODERS ON THE SAME I2C BUS.


volatile int encoder0Pos = 0;  //The current count of input pulses from Encoder on Pins encoder0PinA and encoder0PinB
volatile boolean PastA = 0;
volatile boolean PastB = 0;

int encoder0lastvalue =0; //The old I2C value, used for checking changes of incoming pulses between I2C calls
char enc0ChangeSt[6]; //String to hold the number of pulses since last polled by I2C
int UARTlastvalue=0; //The old UART value, used for checking changes of incoming pulses between polled checks reported by UART 
                     //This is differnet from the encoder0lastvalue used by I2C as it is calculates changes between I2C calls 
boolean I2Cquery=false;

const char softwareversion[] = "1.03" ; //Version of this program, sent to serialport at startup


void setup() 
{
  //Set up Optical encoder pins and set up Interrupt calls
  pinMode(encoder0PinA, INPUT_PULLUP);//the Encoder used is an Opencollector device
  pinMode(encoder0PinB, INPUT_PULLUP); 
  // encoder A channel on interrupt 0 (arduino's pin 2 on Mini Pro)
  attachInterrupt(0, doEncoderA, RISING);
  // encoder B channel pin on interrupt 1 (arduino's pin 3 on Mini Pro)
  //attachInterrupt(1, doEncoderB, CHANGE); 

  pinMode (13,OUTPUT); //Set LED Pin to output;

  //Set up I2C as a slave and set up Interrups call for incoming data
  Wire.begin(I2CSlaveAddr);      // join I2C bus with address #I2CSlaveAddr
  Wire.onRequest(doI2CEvent);    // register event to handle incoming I2C 
  Serial.begin(9600);
  delay (200);

  Serial.print("Zachtek Big Rotary Encoder Software, Version: ");
  Serial.println(softwareversion);
  
  //Startup blink
  digitalWrite(13,HIGH); 
  delay(100);
  digitalWrite(13,LOW);
  delay(100);  
  digitalWrite(13,HIGH); 
  delay(100);
  digitalWrite(13,LOW);
}

void loop() {  
int pulsechange; 
  
  if (UARTlastvalue!=encoder0Pos) {
    pulsechange=UARTlastvalue-encoder0Pos;
    UARTlastvalue=encoder0Pos;
    Serial.println (pulsechange);
    delay(50);//Only report 20 times a second to keep down UART speed.
  }
  
  //Pulse LED if we are queried by I2C
  if (I2Cquery) {
    digitalWrite(13,HIGH); 
    delay(5);
    digitalWrite(13,LOW);
    I2Cquery=false; 
  }
}

//format a Int to a string with four digits and leading sign and zeros 
void calcChangeSt (int IntValue){
char tempstr[5];
  //truncate value to 4 digits
  if (IntValue<-9999) IntValue=-9999;
  if (IntValue> 9999) IntValue=9999; 
  itoa (IntValue,tempstr,10);
   
  //Format the encoder string to always be of four digit length with a leading sign and a zero char at the end
  enc0ChangeSt[1]='0';
  enc0ChangeSt[2]='0'; 
  enc0ChangeSt[3]='0';
  enc0ChangeSt[4]='0';
  if (IntValue<0) //Value is negative
    { 
      enc0ChangeSt[0]='-';

      if (IntValue>-10) //Single digit  
      {
        enc0ChangeSt[4]=tempstr[1];  
        enc0ChangeSt[5]='\0';
        return;
      }

      if (IntValue>-100 && IntValue<-9 ) // Two digit value
      { 
       
        enc0ChangeSt[3]=tempstr[1];
        enc0ChangeSt[4]=tempstr[2];
        enc0ChangeSt[5]='\0';
        return;
      }

      if (IntValue>-1000 && IntValue<-99 ) // Three digit value
      { 
        enc0ChangeSt[2]=tempstr[1];
        enc0ChangeSt[3]=tempstr[2];
        enc0ChangeSt[4]=tempstr[3];
        enc0ChangeSt[5]='\0';
        return;
      }

      if (IntValue<-999 ) // Four digit value
      { 
        enc0ChangeSt[1]=tempstr[1];
        enc0ChangeSt[2]=tempstr[2]; 
        enc0ChangeSt[3]=tempstr[3];
        enc0ChangeSt[4]=tempstr[4];
        enc0ChangeSt[5]='\0';
        return;
      }
    }
    
    else //Value is positive
    
    {
      enc0ChangeSt[0]='+'; 

      if (IntValue<10) // Single digit
      { 
        enc0ChangeSt[4]=tempstr[0];
        enc0ChangeSt[5]='\0';
        return;
      }

      if (IntValue<100 && IntValue>9 ) // Two digit value
      { 
        enc0ChangeSt[3]=tempstr[0];
        enc0ChangeSt[4]=tempstr[1];
        enc0ChangeSt[5]='\0';
        return;
      }

      if (IntValue<1000 && IntValue>99 ) // Three digit value
      { 
        enc0ChangeSt[2]=tempstr[0];
        enc0ChangeSt[3]=tempstr[1];
        enc0ChangeSt[4]=tempstr[2];
        enc0ChangeSt[5]='\0';
        return;
      }

      if (IntValue>999 ) // Four digit value
      { 
        enc0ChangeSt[1]=tempstr[0];
        enc0ChangeSt[2]=tempstr[1]; 
        enc0ChangeSt[3]=tempstr[2];
        enc0ChangeSt[4]=tempstr[3];
        enc0ChangeSt[5]='\0';
        return;
      }
    }
}


void doEncoderA(){ //Interrupt code for rising edge of input encoder0PinA 
     PastB = (boolean)digitalRead(encoder0PinB); //Check the  state of the encoder0PinB (direction pin)
     PastB ? encoder0Pos--:  encoder0Pos++; //Count forward or backward
}
/*
void doEncoderB(){ //Interrupt code for rising and falling edge of input encoder0PinB 
     PastB = !PastB; //Invert variable value on every input change
}
*/

// Event that executes whenever data is requested by I2C master
void doI2CEvent() {
int pulsechange;   
  pulsechange=encoder0lastvalue-encoder0Pos;
  encoder0lastvalue=encoder0Pos;
  calcChangeSt (pulsechange);
  Wire.write(enc0ChangeSt); // respond with message of 6 bytes as expected by master (Sign +4 digits + ASCII zero)
  encoder0lastvalue=encoder0Pos;
  I2Cquery=true;
}

