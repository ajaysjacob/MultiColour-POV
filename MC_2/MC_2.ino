#include <avr/pgmspace.h>
#include <SPI.h>

#ifdef __AVR_ATmega2560__
  const byte CLOCKOUT = 11;  // Mega 2560
#else
  const byte CLOCKOUT = 9;   // Uno, Duemilanove, etc.
#endif


const byte data[36] PROGMEM=
{0xff,0xff,0xff,0x00,0x00,0x00,
0x00,0x77,0x77,0x77,0x77,0x77,
0x77,0x77,0x77,0x77,0x77,0x77,
0x77,0x77,0x77,0x77,0x77,0x77,
0x77,0x77,0x77,0x77,0x77,0x77,
0x77,0x77,0x77,0x70,0x07,0x00};

int gslat=6; // any general purpose pin except 12, MISO
int xblank=7; //Any general purpose pin except 12, MISO
int dat=11;
const int NO_OF_ICS=1;

byte dcData [36];

volatile int flag=0;
volatile int enable=0;

void setup (){
  Serial.begin (9600);
  // set up Timer 1
  pinMode (CLOCKOUT, OUTPUT);
  pinMode (dat, OUTPUT);
  //analogWrite(dat,255);
  TCCR1A = bit (COM1A0);  // toggle OC1A on Compare Match
  TCCR1B = bit (WGM12) | bit (CS10);   // CTC, no prescaling
  OCR1A =  0;  
  Serial.println("setup works");
  pinMode (xblank, OUTPUT);
  digitalWrite (xblank, LOW);
  pinMode (gslat, OUTPUT);
  SPI.setBitOrder(MSBFIRST);//Most Significant Bit First
  SPI.setDataMode(SPI_MODE0);// Mode 0 Rising edge of data, keep clock low
  SPI.setClockDivider(SPI_CLOCK_DIV4);//Run the data in at 16MHz/4 - 4MHz
  digitalWrite (gslat, HIGH);
  DotCorrection ();
  //digitalWrite (xblank, ); //Set xblank high and gslat low for inputting GS data
  digitalWrite (gslat, LOW);
  interrupts ();
}

bool once=true;

void loop() {
  //Serial.println("Loop a okay");
  //while (flag==0 || enable==0);
  if(once){
    digitalWrite (gslat, LOW);
    send_data();
    digitalWrite (gslat, HIGH);
  }
  //flag=0;

}

void send_data()
{
  
  for(int j=0; j<NO_OF_ICS;j++)
  {
     Serial.println("sendata() accessed");
     digitalWrite (xblank, HIGH);
  for (int i=0; i<36; i++){
    SPI.transfer(data[i]);
    Serial.println("Action Completed");
  }
  digitalWrite (xblank, LOW );
  }
  once=false;
}

void DotCorrection()
{
  for(int k=0;k<24;k++) //DC-7Bits x 24Channels=168Bits=21Bytes//GBC-3bytes
  dcData[k]=0x77;
  dcData[24]=B01111000; // 7 LSB bits are Function bits. For enabling Auto
  // Display timing reset mode and 8bit GSCLK mode
  for (int k=25; k<36;k++)
  dcData[k]=0x00;
  digitalWrite(xblank,LOW);
  digitalWrite(gslat,HIGH); //gslat must be high while inputting DC values
  SPI.begin(); // If the GSLAT pin is high on the LAST EDGE of clock GSCLK,
  for(int i=0;i<NO_OF_ICS;i++)
  {
  for(int DCSin=35;DCSin>=0;DCSin--)
  {
    SPI.transfer(dcData[DCSin]);
  }
  }
  delay(10);
  digitalWrite(gslat,LOW);
  //delay(10);
  digitalWrite(gslat,HIGH); //Provides rising edge at which data is transferred
  delay(10);
}
