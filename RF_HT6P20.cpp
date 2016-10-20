/**************************************************************************
			       LIBRARY DECODER FOR CHIP HT6P20           
***************************************************************************
AUTOR: LUCAS MAZIERO - Eletric engineer                                                   
E-MAIL: lucas.mazie.ro@hotmail.com 									  
CIDADE: Santa Maria - Rio Grande do Sul - Brasil                                       
***************************************************************************
Vers�o: 2.0                                                             
Data: 23/03/2015                                                        
Modificado: 03/04/2015                                                                                                                          
***************************************************************************
BASE DO CODIGO: https://gist.github.com/acturcato/8423241
**************************************************************************/

#include <RF_HT6P20.h>

#if (ARDUINO >= 100)
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

RF_HT6P20::RF_HT6P20()
{

}
void RF_HT6P20::beginRX(int _pinRX)
{
	pinRF_RX = _pinRX;
	pinMode(pinRF_RX,INPUT);
}
void RF_HT6P20::beginTX(int _pinTX)
{
	pinRF_TX = _pinTX;
	pinMode(pinRF_TX,OUTPUT);
	digitalWrite(pinRF_TX, LOW);
}
void RF_HT6P20::beginRXTX(int _pinRX, int _pinTX)
{
	pinRF_RX = _pinRX;
	pinMode(pinRF_RX,INPUT);
	
	pinRF_TX = _pinTX;
	pinMode(pinRF_TX,OUTPUT);
	digitalWrite(pinRF_TX, LOW);	
}

unsigned long RF_HT6P20::getCode()
{
	return(addressFull);
}
boolean RF_HT6P20::available()
{
  static boolean startbit;      //checks if start bit was identified
  static int counter;           //received bits counter: 22 of Address + 2 of Data + 4 of EndCode (Anti-Code)
  static unsigned long buffer;  //buffer for received data storage

  int dur0, dur1;  // pulses durations (auxiliary)

  if (!startbit)
  {// Check the PILOT CODE until START BIT;
    dur0 = pulseIn(pinRF_RX, LOW);  //Check how long DOUT was "0" (ZERO) (refers to PILOT CODE)

    //If time at "0" is between 9200 us (23 cycles of 400us) and 13800 us (23 cycles of 600 us).
    if((dur0 > 9200) && (dur0 < 13800) && !startbit)
    {    
      //calculate wave length - lambda
      lambda_RX = dur0 / 23;

      //Reset variables
      dur0 = 0;
      buffer = 0;
      counter = 0;

      startbit = true;
    }
  }

  //If Start Bit is OK, then starts measure os how long the signal is level "1" and check is value is into acceptable range.
  if (startbit && counter < 28)
  {
    ++counter;

    dur1 = pulseIn(pinRF_RX, HIGH);

    if((dur1 > 0.5 * lambda_RX) && (dur1 < (1.5 * lambda_RX)))  //If pulse width at "1" is between "0.5 and 1.5 lambda", means that pulse is only one lambda, so the data é "1".
    {
      buffer = (buffer << 1) + 1;   // add "1" on data buffer
    }
    else if((dur1 > 1.5 * lambda_RX) && (dur1 < (2.5 * lambda_RX)))  //If pulse width at "1" is between "1.5 and 2.5 lambda", means that pulse is two lambdas, so the data é "0".
    {
      buffer = (buffer << 1);       // add "0" on data buffer
    }
    else
    {
      //Reset the loop
      startbit = false;
    }
  }

  //Check if all 28 bits were received (22 of Address + 2 of Data + 4 of Anti-Code)
  if (counter==28) 
  { 
    // Check if Anti-Code is OK (last 4 bits of buffer equal "0101")
    if ((bitRead(buffer, 0) == 1) && (bitRead(buffer, 1) == 0) && (bitRead(buffer, 2) == 1) && (bitRead(buffer, 3) == 0))
    {     
      counter = 0;
      startbit = false;

      //Get ADDRESS COMPLETO from Buffer
      addressFull = buffer;
	  
      //If a valid data is received, return OK
      return true;
    }
    else
    {
      //Reset the loop
      startbit = false;
    }
  }
  
  //If none valid data is received, return NULL and FALSE values 
  addressFull = 0; 

  return false;
}

void RF_HT6P20::myDelay(unsigned int t)
{
  delayMicroseconds(t);
}

void RF_HT6P20::sendData(char data)
{
  int pulse = (int)lambda_TX;
  
  if(data == '0')
  {
    digitalWrite(pinRF_TX, LOW);
    myDelay(pulse);
  
    digitalWrite(pinRF_TX, HIGH);
    myDelay(2 * pulse);
  }
  
  if(data == '1')
  {
    digitalWrite(pinRF_TX, LOW);
    myDelay(2 * pulse);
  
    digitalWrite(pinRF_TX, HIGH);
    myDelay(pulse);
  }
}
void RF_HT6P20::sendPilotCode()
{
  //Keep pinRF on HIGH for little time
  digitalWrite(pinRF_TX, HIGH);
  delayMicroseconds(500);
  
  //Set pinRF on LOW for 23 Lambdas
  digitalWrite(pinRF_TX, LOW);
  myDelay(23 * lambda_TX);
  
  //Set pinRF on HIGH for one Lambda
  digitalWrite(pinRF_TX, HIGH);
  myDelay(lambda_TX);
}
void RF_HT6P20::sendCode(unsigned long addressCodeHEX)
{
 String addressCodeBIN = "0000000000000000000000000000" + String(addressCodeHEX, BIN);
 
 addressCodeBIN = addressCodeBIN.substring(addressCodeBIN.length()-28, addressCodeBIN.length());
 
 if(!addressCodeHEX == 0)
 {
	//Send PILOTE CODE (details: http://acturcato.wordpress.com/2014/01/04/decoder-for-ht6p20b-encoder-on-arduino-board-english/)
	sendPilotCode();
 
	//Send all bits for Address Code
	for(int i=0; i<28; i++)
	{
		char data = addressCodeBIN[i];
		sendData(data);
	}
 }
 //Disables transmissions
 digitalWrite(pinRF_TX, LOW);
}