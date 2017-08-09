#include <avr/pgmspace.h>
#include <SPI.h>

#ifdef AVR_ATmega2560
  const byte CLOCKOUT = 11;  // Mega 2560
#else
  const byte CLOCKOUT = 9;   // Uno, Duemilanove, etc.
#endif

int dat=11;
int gslat=6; // any general purpose pin except 12, MISO
int xblank=7; //Any general purpose pin except 12, MISO

const int NO_OF_ICS=1;
byte dcData [36];


const byte data[36]=
{0x00,0x00,0x00,0x00,0x00,0x00,
0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff};

void setup() {
  // put your setup code here, to run once:
  
  SPI.setBitOrder(MSBFIRST);//Most Significant Bit First
  SPI.setDataMode(SPI_MODE0);// Mode 0 Rising edge of data, keep clock low
  SPI.setClockDivider(SPI_CLOCK_DIV4);//Run the data in at 16MHz/4 - 4MHz

  pinMode( CLOCKOUT, OUTPUT);
  pinMode( dat, OUTPUT);
  pinMode( xblank, OUTPUT);
  pinMode(gslat, OUTPUT);

  /*TCCR1A = bit (COM1A0);  // toggle OC1A on Compare Match
  TCCR1B = bit (WGM12) | bit (CS10);   // CTC, no prescaling
  OCR1A =  0; 
  */
  digitalWrite( xblank, LOW);
  digitalWrite( gslat, LOW);
  digitalWrite( CLOCKOUT, LOW);
  
  digitalWrite(dat,HIGH);
  DotCorrection();
  interrupts();
}

void loop() {
  send_data();
}

void send_data()
{
  digitalWrite(gslat, LOW);
  delay(10);
  digitalWrite( gslat, HIGH);
  delay(10);
  digitalWrite(gslat,LOW);
  delay(10);
  for(int i=1;i<=288;i++){
    digitalWrite(CLOCKOUT,LOW);
    delay(10);
    digitalWrite(CLOCKOUT,HIGH);
    delay(10);
  }
}
void DotCorrection()
{
  digitalWrite(xblank,LOW);
  digitalWrite(gslat,HIGH); //gslat must be high while inputting DC values
  digitalWrite(xblank,HIGH);
  for(int k=0;k<24;k++) { //DC-7Bits x 24Channels=168Bits=21Bytes//GBC-3bytes
  dcData[k]=0xff;}
  dcData[24]=B01111000; // 7 LSB bits are Function bits. For enabling Auto
  // Display timing reset mode and 8bit GSCLK mode
  for (int k=25; k<36;k++){
  dcData[k]=0x00;}
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
  delay(10);
  digitalWrite(gslat,HIGH); //Provides rising edge at which data is transferred
 }
