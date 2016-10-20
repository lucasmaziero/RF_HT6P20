#include <RF_HT6P20.h>

RF_HT6P20 RF;

int pinRF_RX = 2;

void setup() 
{
  RF.beginRX(pinRF_RX);
  Serial.begin(9600);
}

void loop() 
{
  if(RF.available())
  {
    Serial.print("Address Full: ");
    Serial.println(RF.getCode(),HEX);
  }
  
}
