#include <RF_HT6P20.h>

RF_HT6P20 RF;

#define HT6P20buttonCode 0xBD6DAD5 //code button
int pinRF_RX = 2;
int pinRelay = 13;

void setup() 
{
  RF.beginRX(pinRF_RX);
  Serial.begin(9600);
  pinMode(pinRelay,OUTPUT);
}

void loop() 
{
  if(RF.available())
  {
    Serial.print("Address Full: ");
    Serial.println(RF.getCode(),HEX);
  }
  if(RF.getCode()== HT6P20buttonCode)
  {
    digitalWrite(pinRelay,!digitalRead(pinRelay));
    delay(300);
  }
  
}
