/**************************************************************************
 * TRIGGERS TWO 2 CHANNEL RECORDED AT EEPROM                           
 ***************************************************************************
 * AUTOR: LUCAS MAZIERO - Eletrical Engineer                                                    
 * E-MAIL: lucas.mazie.ro@hotmail.com 									  
 * CIDADE: Santa Maria - Rio Grande do Sul - Brasil                                       
 ***************************************************************************
 * Versão: 1.0                                                             
 * Data: 17/04/2016                                                        
 * Modificado: --/--/----                          
 ***************************************************************************
 * PROGRAMA BASEADO DO CHIP ATMEGA 328                                      
 * KIT DE DESENVOLVIMENTO ARDUINO PRO MINI                                  
 **************************************************************************/

/**************************************************************************
 * DESCRIÇÃO DO PROJETO
 ***************************************************************************
 * PROJETO DESENVOLVIDO COM ARDUINO + ESP8266 + RF433
 **************************************************************************/

/**************************************************************************
 * BIBLIOTECAS AUXILIARES
 **************************************************************************/
#include <RF_HT6P20.h> // Biblioteca RF 433
#include <EEPROM.h> // Biblioteca EEPROM
#include <Metro.h> // Biblioteca dos Time's

/**************************************************************************
 * DECLARACAO DE OBJETOS
 **************************************************************************/
RF_HT6P20 RF;

Metro LearningMode = Metro(2000);
Metro eepromReset = Metro(8000);

/**************************************************************************
 * VARIAVEIS E DEFINIÇÕES
 **************************************************************************/
#define pinRF_RX 2 //Pino receptor do modulo RF433

/*************************** Pinos de Entrada ****************************/
#define pinLearnButton 3 // Pino que seleciona modo apreender da cental
#define pinSelecaoCanal 4 // Pino que seleciona o canal da cental

#define buttonCanal1 A2 // Botao que aciona o canal 1
#define buttonCanal2 A3 // Botao que aciona o canal 2

/**************************** Pinos de Saida *****************************/
#define pinCanal1 A0 // Pino que aciona canal 1
#define pinCanal2 A1 // Pino que aciona canal 2

#define pinLearnLED 13 // Led do pino 13 que indica que a cental esta em modo apreender 

#define EEPROMSizeATmega328   1024 // Tamanho da eeprom do arduino

#define myDelay 200 // Delay anti-repique

long codeHt6p20; // Armazema codigo do controle recebido
long address; // Armazema valor do ultima endereco da eeprom gravado

int EEPROMSize = EEPROMSizeATmega328 / 4;// eeprom tem 1024bytes como estamos gravando dados de 32bits=4bytes temos que dividir por 4

int selecao_canal = 0;

const int numCanais = 2; // Numeros de canais da central
const int releAciona = HIGH; //rele aciona com LOW ou HIGH?

/**************************************************************************
 * ESQUEMA BASE DAS LIGACOES(MAP DE PINOS)
 **************************************************************************/
//        |->Porta tranca Motor|->(10)
//        |                    
//        |                |->(11)
//Saidas->|->Lampadas Reles|->(12)
//        |                |->(A0)
//        |                |->(A1)
//        |->LED Aprender->(13)
/////////////////////////////////////////
///////////////////////////////////////
//                     |->Modo Apreender->(3)
//         |->Apreender|
//         |           |->Selecao de canal->(4)
//         |                 |->(5)
//Entrada->|->Lampadas Botoes|->(6)
//         |                 |->(7)
//         |                 |->(8)
//         |                 |->(9)
//         |
//         |->Fim de Curso Parta Aberta FeedBack ServoMotor ->(A4)
//         |                    
//         |->Sinal RF->(2)

/**************************************************************************
 * FUNÇÃO SETUP
 **************************************************************************/
void setup()
{ 
  /*************************** pino receptor *******************************/
  RF.beginRX(pinRF_RX); // Declara pino do receptor RF433

  /*************************** Pinos de Entrada ****************************/
  pinMode(pinLearnButton, INPUT_PULLUP);
  pinMode(pinSelecaoCanal, INPUT_PULLUP);

  pinMode(buttonCanal1, INPUT_PULLUP);
  pinMode(buttonCanal2, INPUT_PULLUP);

  /**************************** Pinos de Saida *****************************/
  pinMode(pinLearnLED, OUTPUT);

  pinMode(pinCanal1, OUTPUT);
  pinMode(pinCanal2, OUTPUT);


  digitalWrite(pinCanal1, !releAciona);
  digitalWrite(pinCanal2, !releAciona);

  //Serial.begin(115200); // Inicia comunicacao serial

  address = EEPROMReadlong(0) + 1; // Armazema o ultimo + 1 endereco do eeprom

  delay(myDelay);// Espera as tensoes dos modulos estabizar
}
/**************************************************************************
 * FUNÇÃO LOOP
 **************************************************************************/
void loop()
{
  /**************************** Reseta Timer,s *****************************/
  if (digitalRead(pinLearnButton))
  {
    LearningMode.reset();
    eepromReset.reset();
  }

  /************************* Ativa Modo Apreender **************************/
  if (LearningMode.check() && !digitalRead(pinLearnLED))
  {
    digitalWrite(pinLearnLED, HIGH);
  }

  /***************************** Reseta EEPROM *****************************/
  if (eepromReset.check())
  {
    digitalWrite(pinLearnLED, LOW);
    EEPROMreset();
  }

  /**************************** Selecao de Canal ****************************/
  if (!digitalRead(pinSelecaoCanal) && digitalRead(pinLearnLED))
  {
    selecao_canal++;
    selecao_canal = selecao_canal % (numCanais + 2);
    delay(myDelay);
  }
  if (selecao_canal == 0 && digitalRead(pinLearnLED))
  {
    digitalWrite(pinCanal1, !releAciona);
    digitalWrite(pinCanal2, !releAciona);
  }
  if (selecao_canal == 1)
  {
    digitalWrite(pinCanal1, releAciona);
  }
  if (selecao_canal == 2)
  {
    digitalWrite(pinCanal1, !releAciona);
    digitalWrite(pinCanal2, releAciona);
  }

  if (selecao_canal == (numCanais + 1)) selecao_canal = 1; // Retorna ao inicio da selecao

  /**************************** Botao Aciona Canal ***************************/
  if (!digitalRead(buttonCanal1))
  {
    digitalWrite(pinCanal1, !digitalRead(pinCanal1));
    delay(myDelay);
  }
  if (!digitalRead(buttonCanal2))
  {
    digitalWrite(pinCanal2, !digitalRead(pinCanal2));
    delay(myDelay);
  }
  /**************************** Recepcao do Sinal ****************************/
  if (RF.available())
  {
    codeHt6p20 = RF.getCode();

    if (digitalRead(pinLearnLED) && address <= EEPROMSize  && selecao_canal != 0)
    {
      switch (selecao_canal)
      {
      case 1:
        codeHt6p20 = ((codeHt6p20 << 4) | 0x1);// concatena entereco com canal
        break;

      case 2:
        codeHt6p20 = ((codeHt6p20 << 4) | 0x2);
        break;
      }

      EEPROMWritelong(address, codeHt6p20);
      EEPROMWritelong(0, address);
      codeHt6p20 = NULL;
      selecao_canal = 0;
      address++;

      digitalWrite(pinLearnLED, LOW);
    }
  }

  /********************* Verifica Se o Codigo Esta Na EEPROM ******************/
  if ((codeHt6p20 != NULL))
  {
    for (int i = 1; i <= address; i++)
    {
      if (codeHt6p20 == ((EEPROMReadlong(i) >> 4) & 0xFFFFFFF))
      {
        if ((EEPROMReadlong(i) & 0xF) == 1)
        {
          digitalWrite(pinCanal1, !digitalRead(pinCanal1));
          delay(myDelay);
          break;
        }
        if ((EEPROMReadlong(i) & 0xF) == 2)
        {
          digitalWrite(pinCanal2, !digitalRead(pinCanal2));
          delay(myDelay);
          break;
        }
      }
    }
    codeHt6p20 = NULL;
  }
}

/**************************************************************************
 * FUNCAO DE MANIPULACAO DE EEPROM PARA DADOS DE 32 BITS (LEITUTA/ESCRITA)
 **************************************************************************/
///////////////////////////////////////////////////////////////////////////////////
//This function will write a 4 byte (32bit) long to the eeprom at
//the specified address to adress + 3.
void EEPROMWritelong(long address, long value)
{
  address = address * 4; //desloca 4bits na eeprom.
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}
///////////////////////////////////////////////////////////////////////////////////
//This function will return a 4 byte (32bit) long from the eeprom
//at the specified address to adress + 3.
long EEPROMReadlong(long address)
{
  address = address * 4; //desloca 4bits na eeprom.
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

/**************************************************************************
 * FUNCAO QUE APAGA TODOS OS CONTROLES GRAVADOS NA CENTAL
 **************************************************************************/
void EEPROMreset()
{
  for (int i = 0; i <= EEPROMSize; i++)
  {
    EEPROMWritelong(i, 0);
  }
  address = EEPROMReadlong(0) + 1;
}




























































