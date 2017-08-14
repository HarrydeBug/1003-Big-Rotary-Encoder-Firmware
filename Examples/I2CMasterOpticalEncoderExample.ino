//Software for an Arduino that is connected to a Zachtek "Big Rotary Encoder V1R1" on an I2C Bus.
//The "Big Rotary Encoder V1R1" is an I2C Slave that counts pulses in from an optical encoder and reports it over I2C wenever asked by a
//this example software that runs on another Arduino (E.g like an Arduino UNO or so)
//This software is the I2C Master
//SDA, SCL and GND should be connected between the two arduinos
//The optical encoder counts since last call will be printed on the serial monitor as a value
//Portion of this program built upon the example I2C code "Wire Master Reader" by Nicholas Zambetti <http://www.zambetti.com>
//ZachTec 2016-2017

#include <Wire.h>
char RotaryStringIn[6]={'d','a','t','a','\0'}; //String to hold Rotary Encoder input
char KeyStringIn[3] ;                             // String to hold received Keyboard input

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
}

  
void loop() {
int EncValue;  

int charcount;
unsigned long I2CTimeOut; 

  WipeBuffers ();
  
  //Check Rotary Encoder
  charcount=0;
  Wire.requestFrom(8, 6);    // request 6 bytes from slave device #8 (Optical Encoder)
  I2CTimeOut=millis()+100;  //Timeout if not all 5 bytes are received in 100mS
  while (charcount<6 && millis()<I2CTimeOut ) {//untill all data is received or TimeOut is reached.
    while (Wire.available()) { // call may return with less than requested at so keep on filling buffer
      RotaryStringIn[charcount] = Wire.read(); // receive a byte as character  
      charcount++;
    }
  }
  if (millis ()>=I2CTimeOut) {
    Serial.println("Rotary Encoder I2C TIME OUT");
  }
  else
  {//We recevied data from I2C slave
    EncValue= atoi (RotaryStringIn); //Convert to Integer
    if (EncValue != 0) { //If the encoder was moved it will hold a value that is non-zero
      Serial.print ("Rotary Encoder ");
      Serial.println(EncValue); //Print the received data
    }  
  }
  
  delay (50); //Query 20 times per second.
}

void WipeBuffers ()
{
  //RotaryStringIn={'d','a','t','a','\0'}; //String to hold Rotary Encoder input
  KeyStringIn[0]='\0';
  KeyStringIn[1]='\0';
  KeyStringIn[2]='\0';
}

