#include<SPI.h>
//#include<SD.h> 
//#include<SdFat.h>
//#include <SdFatUtil.h>
#include <avr/pgmspace.h>
/*GSSCK -> PIN13
GSSIN -> PIN11
GSCK -> PIN8 (16MHz)
GSLAT -> PIN6
XBLANK -> PIN7
DCSCK -> GND
SENSOR -> PIN2*/

const byte greendata[36] PROGMEM=
{0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xff,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0xff};

//attempt 2
//SdFat sd; 
//SdFile file; 

int gslat = 6;
int xblank = 5;
int temp=0;
byte dcData[36];
int gssck=13;
int data =11;
//int gsclk=3;
/*byte greendata[36] = 
{0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00};*/

volatile int time_period = 0;
volatile int flag = 0;
volatile int enable = 0;

void setup()
{
  //Serial.println("Work dammit");
  pinMode (xblank, OUTPUT);
  pinMode (gslat, OUTPUT);
  pinMode (gssck, OUTPUT);
  pinMode (data, OUTPUT);
  //pinMode (gsclk,OUTPUT);
  //analogWrite (data, 255);
  digitalWrite (xblank, LOW);
  //digitalWrite (gssck, LOW);
  digitalWrite (gslat, HIGH);
  
  digitalWrite (xblank, HIGH);
  digitalWrite (gslat, LOW);

  SPI.setBitOrder(MSBFIRST);//Most Significant Bit First
  SPI.setDataMode(SPI_MODE0);// Mode 0 Rising edge of data, keep clock low
  SPI.setClockDivider(SPI_CLOCK_DIV4);//Run the data in at 16MHz/4 - 4MHz

  Serial.begin (9600);
  DotCorrection();
  interrupts();
}

void loop() 
{
  //while (flag == 0 || enable == 0)
    if(temp<1){
      //Serial.println("Work dammit!");
      //digitalWrite (gslat, LOW);
      send_data();
      //digitalWrite (gslat, HIGH);
      flag = 0;  
      temp++;
    }
}

void send_data()
{
  //Serial.println("Work dammit!!");
  for (int i=0; i<36; i++)
  {
    SPI.transfer(greendata[i]);
    //SPI.transfer((uint8_t)(pgm_read_word(&greendata[i])&255));
    //SPI.transfer((uint8_t)(0x00)&255);
    //analogWrite(10, 255);
    digitalWrite (gssck, HIGH);
    digitalWrite (gssck, LOW);
  }
  Serial.println("Operation Successful!");
}

void DotCorrection()
{
  for (int k=0; k<24; k++)
    dcData[k] = 0x00;
  dcData[24] = B01111000;
  for (int k=25; k<36;k++)
  dcData[k]=0xff;
  digitalWrite(xblank,LOW);
  digitalWrite(gslat,HIGH);
  SPI.begin();
  for(int DCSin=35;DCSin>=0;DCSin--)
  {
    SPI.transfer(dcData[DCSin]);
  }
  delay(10);
  digitalWrite(gslat, LOW);
  delay(10);
  digitalWrite(gslat, HIGH);
  delay(10);
}


