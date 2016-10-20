/**************************************************************************
			       LIBRARY DECODER FOR CHIP HT6P20           
***************************************************************************
AUTOR: LUCAS MAZIERO - Eletric engineer                                                   
E-MAIL: lucas.mazie.ro@hotmail.com 									  
CIDADE: Santa Maria - Rio Grande do Sul - Brasil                                       
***************************************************************************
Versão: 2.0                                                             
Data: 23/03/2015                                                        
Modificado: 03/04/2015                                                                                                                         
***************************************************************************
BASE DO CODIGO: https://gist.github.com/acturcato/8423241
**************************************************************************/
#ifndef RF_HT6P20_h
#define RF_HT6P20_h

#if(ARDUINO >= 100)
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#define lambda_TX 350 //valores de lambda pode varariar de 320 a 420 

class RF_HT6P20
{
	public:
		RF_HT6P20();
		void beginRX(int _pinRX);
		void beginTX(int _pinTX);
		void beginRXTX(int _pinRX, int _pinTX);
		unsigned long getCode();
		boolean available();
		
		void myDelay(unsigned int t);
		void sendData(char data);
		void sendPilotCode();
		void sendCode(unsigned long addressCodeHEX);
		
	
    private:
		int pinRF_RX;
		int lambda_RX;
		
		int pinRF_TX;
		//int lambda_TX;
		unsigned long addressFull;

};
#endif