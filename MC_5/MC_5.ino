/* PORTC- 12 to 19 GSSIN 9 to 16
PORTC- 2 to 9----> 16 GPI output pins for data output GSSIN 9 to 16
PORTC- C1- external interrupt-- digital pin D22
PORTC- 2-D34 PORTC- 12-D51
3-D35 13-D50
4-D36 14-D49
5-D37 15-D48
6-D38 16-D47
7-D39 17-D46
8-D40 18-D45
9-D41 19-D44
REG_PIOC_ODSR=byte1<<2|byte2<<12 (For sending 1 bit to each IC)
GSCLKR/G/B signal from Analog pin 1.
GSSCK signal--> Digital 15 ---> PORT D Pin 4
GSLAT signal--> Digital 14 ---> PORT D Pin 5
XBLANK signal--> Digital 16 ---> PORT A Pin 13
*/
#include <avr/pgmspace.h>

# define noOperation(x) for(int i=0;i<x;i++) asm volatile ("\n\t")
# define gslat_low() REG_PIOD_ODSR&=~1<<4
# define gslat_high() REG_PIOD_ODSR|=1<<4
# define gssck_low() REG_PIOD_ODSR&=~1<<5
# define gssck_high() REG_PIOD_ODSR|= 1<<5

byte greenData[36]=
{
0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff
};

int gslat=14; // D4
int xblank=16; // A13
int gssck=15; // D5
volatile boolean enable=false;

const uint16_t num_leds=8;
const uint16_t num_bitsPWM=8;
const uint16_t led_channels=3;
const uint16_t num_extraBits=4;

volatile unsigned long int rc=5000;
byte dcData[216];

void setup() {
  /* CONFIGURE ALL THE REGISTERS OF ATMEGA CHIP*/
  //**********************************************************************
  // TO DISABLE WRITE PROTECT MODE//
  REG_PIOA_WPMR=0x50494F00;
  REG_PIOB_WPMR=0x50494F00; // Writes WPKEY at required value of
  REG_PIOC_WPMR=0x50494F00; //Ox50494F to change WPEN bit, Write
  REG_PIOD_WPMR=0x50494F00; // Protect Enable bit, and clears the bit
  // as well (0th bit).
  //-------------------------------------------------------------------------//
  /* PARALLEL INPUT OUTPUT (PIO) REGISTERS*/
  // Set Data direction bits for PORTC
  // making C.2-C.9 and C.12-C.19 as outputs
  REG_PIOC_OER|=0b11111111<<2|(0b11111111<<12); // OER- Output Enable Register. C2-9 and 12-19 as Output
  REG_PIOC_PER|=0b11111111<<2|(0b11111111<<12); // Enable PIO control, disable peripheral control
  /* For Clock output through pin Analog 1, Frequency=24 MHz*/
  REG_PIOA_PDR|=1<<24; // Enable peripheral control on PA24= Analogue 1
  REG_PIOA_ABSR|=1<<24;
  
  pinMode(xblank,OUTPUT);
  digitalWrite(xblank,LOW);
  pinMode(gslat,OUTPUT);
  gslat_high();
  pinMode(gssck,OUTPUT);
  gssck_low();
  DotCorrection();
  //Set xblank high and gslat low for inputting GS data
  gslat_low();
  noInterrupts();
  pmc_set_writeprotect(false);
  REG_PMC_SCER|=1<<9; //Enable PCK1
  REG_PMC_PCK|=1<<5|1<<4|1; // Clock= Main Clock, prescalar=4
  // Freq= 84/4=21MHz
  // For GSCKB/R/G signal from A1
  pmc_enable_periph_clk(ID_TC4);
  pmc_enable_periph_clk(ID_TC3);
  TC_Configure(TC1, 1, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK4);
  TC_Configure(TC1, 0, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC |TC_CMR_TCCLKS_TIMER_CLOCK1);
  TC_SetRC(TC1, 0, rc);
  TC_Start(TC1, 1);
  TC_Start(TC1, 0);
  TC1->TC_CHANNEL[0].TC_IER=TC_IER_CPCS;
  TC1->TC_CHANNEL[0].TC_IDR=~TC_IER_CPCS;
  NVIC_EnableIRQ(TC3_IRQn);
  
  all_green(); // Start with all LEDs turned green
  digitalWrite(xblank,HIGH); // Turn all LEDs on. (XBLANK =LOW makes all LEDs forcibly go off)
  interrupts(); // Enable timer and IR interrupts now
  
}
void loop() {
  //all_green();
}

void DotCorrection()
{
//Serial.println("In Dot correction mode");
for(int dcByte=0;dcByte<199;dcByte++)
dcData[dcByte]=0xff; // 7 bits*24 LEDs=21Bytes + 3 Bytes for GBC
// 192-198 7 bits FC all 1
for(int dcByte=199;dcByte<216;dcByte++)
dcData[dcByte]=0x00; // Rest all bits not needed
for(int dcByte=0;dcByte<216;dcByte++)
{
REG_PIOC_ODSR=dcData[dcByte]<<2|dcData[dcByte]<<12;
gssck_high();
noOperation(3);
gssck_low();
}
noOperation(10);
gslat_low();
noOperation(10);
gslat_high(); //Provides rising edge at which data is transferred into DC 216 bit
noOperation(10);

}

void all_green()
{
  //for(int i=0;i<8;i++) // Number of LEDs
    for(int j=0;j<36;j++) // Data for 1 LED=36 bits
    {
      REG_PIOC_ODSR=greenData[j]<<2|greenData[j]<<12;
      gssck_high();
      noOperation(3);
      gssck_low();
      noOperation(3);
    }
      gslat_high();
      noOperation(10);
      gslat_low();
      noOperation(10);
}
