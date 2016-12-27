/* CAN Bus transmit speed test. Transmit interval can be set by navigation wheel.
 *  
 * www.skpang.co.uk
 * 
 * V1.0 Dec 2016
 *  
 * For use with Teensy CAN-Bus Demo board:
 * http://skpang.co.uk/catalog/teensy-canbus-demo-board-included-teensy-32-p-1505.html
 * 
 * Make sure the fonts are installed first
 * https://github.com/PaulStoffregen/ILI9341_fonts
 * 
 * Put the font files in /hardware/teensy/avr/libraries/ILI9341_t3 folder
 * 
 * Also requres new FlexCAN libarary
 * https://github.com/collin80/FlexCAN_Library
 * 
 * 
 */
#include <FlexCAN.h>
#include <Encoder.h>
#include <Metro.h>

#include "SPI.h"
#include "ILI9341_t3.h"
#include "font_Arial.h"
#include "font_LiberationMono.h"
#include "font_CourierNew.h"

#define SCK_PIN   13  //Clock pin
#define MISO_PIN  12  //Mater in Slave output
#define MOSI_PIN  11  //Master out Slave input
#define SD_PIN    10  //pin for SD card control
#define TFT_DC 9
#define TFT_CS 2

#define P14 14

const int JOY_LEFT = 0;
const int JOY_RIGHT= 6;
const int JOY_CLICK = 5;
const int JOY_UP  = 7;
const int JOY_DOWN = 1;

const int JOG_A = 20;
const int JOG_B = 19;

IntervalTimer TX_timer;
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);
Encoder jog(JOG_A, JOG_B);
  
int led = 13;
static CAN_message_t msg,rxmsg;

volatile uint32_t can_msg_count = 0;
long jog_position  = -999;
long tx_delay  = 1000;
long new_jog;

void setup() {
  Can0.begin(500000);
  //Can0.begin(1000000); 
  pinMode(JOY_LEFT, INPUT_PULLUP);    // Enable internal pullup
  pinMode(JOY_RIGHT, INPUT_PULLUP);
  pinMode(JOY_CLICK, INPUT_PULLUP);
  pinMode(JOY_UP, INPUT_PULLUP);
  pinMode(JOY_DOWN, INPUT_PULLUP); 
  
  pinMode(P14, OUTPUT); 
  pinMode(led,OUTPUT);
  
  tft.begin();                        // Setup TFT
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setFont(LiberationMono_10);
  tft.setCursor(0, 0);
  tft.println("CAN Tx Test v1.0     skpang.co.uk 11/16");
  tft.setCursor(0, 20);
  tft.println("Center Btn:Start/Pause");
  tft.setCursor(0, 31);
  tft.println("Up:Exit    Jog:+/- delay ");
  tft.drawLine(0,46,340,46,ILI9341_DARKGREY);    //Horizontal line
  tft.setFont(LiberationMono_12);
  tft.setCursor(0, 100);
  tft.println("Interval "); 
  
  delay(1000);
  Serial.println(F("CAN Bus Tx test"));
  msg.buf[0] = 1;
  msg.buf[1] = 2;
  msg.buf[2] = 0;
  msg.buf[3] = 0;
  msg.buf[4] = 0;
  msg.buf[5] = 0;
  msg.buf[6] = 0;
  msg.len = 8;
  msg.id = 0x222;
  msg.timeout = 500;
 
  jog.write(30);
}

/* From Timer Interrupt */
void tx_CAN(void)
{
  msg.buf[4] = can_msg_count >> 24;
  msg.buf[5] = can_msg_count >> 16;
  msg.buf[6] = can_msg_count >> 8;
  msg.buf[7] = can_msg_count;
  Can0.write(msg);
  can_msg_count++;
}

void loop() {

  if(digitalRead(JOY_CLICK) == 0)
  {
     Serial.println(F("Click"));
     delay(10);
     while(digitalRead(JOY_CLICK) == 0);
      
     tx_loop();
   }
    
  if(digitalRead(JOY_UP) == 0)
  {
     Serial.println(F("up"));
     while(digitalRead(JOY_UP) == 0);
     delay(50);
     can_msg_count = 0;
  }

  if(digitalRead(JOY_RIGHT) == 0)
  {
     Serial.println(F("RIGHT"));
     while(digitalRead(JOY_UP) == 0);
     delay(50);
     
  }

  if(digitalRead(JOY_LEFT) == 0)
  {
     Serial.println(F("LEFT"));
     while(digitalRead(JOY_UP) == 0);
     delay(50);
    can_msg_count = 0;
  }
  
  if(digitalRead(JOY_DOWN) == 0)
  {
     Serial.println(F("DOWN"));
     while(digitalRead(JOY_UP) == 0);
     delay(50);

  }
  
  new_jog = jog.read();
  
  if (new_jog != jog_position ) 
  {
     update_jog(new_jog);
  }
   
} //loop

void update_jog(long new_jog)
{
   if (new_jog < 3) 
   {
       jog.write(3);
       new_jog = 3;
    }
    
    jog_position = new_jog;
    tx_delay = new_jog * 50;
    tx_delay_update(tx_delay);
}

void tx_delay_update(int tx_delay)
{
  float fps;
  fps = 1/((float)tx_delay/1000000);
  tft.fillRect(90,99,190,16,ILI9341_BLACK); // Clear count area
  tft.setCursor(90, 100);
  tft.print(tx_delay,DEC);   
  tft.print(" uS  "); 
  tft.print(round(fps),DEC);
  tft.print(" f/s");
   
}

void update_count(void)
{
    tft.fillRect(60,125,100,15,ILI9341_BLACK); // Clear count area
    tft.setCursor(60, 125);
    tft.println(can_msg_count);

    tft.fillRect(60,150,235,15,ILI9341_BLACK); // Clear count area

    tft.setCursor(60, 150);
    tft.print("00");

    tft.setCursor(85, 150);
    tft.print("00");

    tft.setCursor(110, 150);
    tft.print("00");

    tft.setCursor(135, 150);
    tft.print("00");
    
    tft.setCursor(160, 150);
    tft.print((can_msg_count >> 24),HEX);
 
    tft.setCursor(185, 150);
    tft.print((can_msg_count >> 16)&0xff,HEX);
    
    tft.setCursor(210, 150);
    tft.print((can_msg_count >> 8)&0xff,HEX);

    tft.setCursor(235, 150);
    tft.print(can_msg_count&0xff,HEX);
}

/* Main Tx loop 
 * 
 */
void tx_loop(void)
{
  uint8_t exit = 0;
  can_msg_count = 0;
  
  tft.fillRect(0,225,55,15,ILI9341_BLACK); // Clear count area
  tft.setCursor(0, 225);
  
  tft.println("RUN");
  tx_delay_update(tx_delay);

  tft.setCursor(0, 100);
  tft.println("Interval ");
  tft.setCursor(0, 125);
  tft.println("Count ");
  tft.setCursor(0, 150);
  tft.println("Data ");

  TX_timer.begin(tx_CAN, tx_delay);    /* Start interrutp timer */
  
  while(1)
  {
    update_count();
    new_jog = jog.read();

    if(exit == 1)
      break;      
      
    if (new_jog != jog_position ) 
    {
      update_jog(new_jog);
     
      TX_timer.end();           /* Stop timer */
      tx_delay_update(tx_delay);
      TX_timer.begin(tx_CAN, tx_delay);    /* Start interrutp timer again */
    }
    
    if(digitalRead(JOY_CLICK) == 0)  // Check for pause
    {
      tft.fillRect(0,225,55,15,ILI9341_BLACK); // Clear count area
      tft.setCursor(0, 225);
      tft.println("PAUSE");
 
      TX_timer.end();
      while(digitalRead(JOY_CLICK) == 0);
      delay(50);
      
      while(1)  // In pause mode, wait until a key is pressed
      {
        if(digitalRead(JOY_CLICK) == 0)
        {   
          tft.fillRect(0,225,55,15,ILI9341_BLACK); // Clear count area
          tft.setCursor(0, 225);
          tft.println("RUN");
    
          while(digitalRead(JOY_CLICK) == 0);
          delay(50);
          TX_timer.begin(tx_CAN, tx_delay);
          break;
        }

        if(digitalRead(JOY_LEFT) == 0)    // Reset count
        {
           can_msg_count = 0;
           update_count();
           while(digitalRead(JOY_CLICK) == 0);
           delay(50);
        }
        
        if(digitalRead(JOY_UP) == 0)
        { 
            exit = 1;
            tft.fillRect(0,225,55,15,ILI9341_BLACK); // Clear count area
            tft.setCursor(0, 225);
            tft.println("READY");
            while(digitalRead(JOY_UP) == 0);
            delay(50);
            break;
        }
   
      }
    }
  } // while
}
