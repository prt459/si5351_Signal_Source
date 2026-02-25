/*  
Signal source fortesting receivers etc
Written for Arduino Nano or XIAO ESP32 and si5351 breakout  
Written by Paul VK3HN (https://vk3hn.wordpress.com/) 
// I2C devices and addresses:
 
// si5351   0x60  // Multisynth PLL

Labels that need to be #define'd for your target radio/rig/project:
  Display technology     {DISPLAY_LCD, DISPLAY_OLED}

*/
#define DISPLAY_OLED128x32
//#define DISPLAY_LCD


#include <SPI.h>
#include <Wire.h>

#include <si5351.h>     // Etherkit si3531 library from NT7S,  V2.1.4   https://github.com/etherkit/Si5351Arduino 
Si5351 si5351;          // I2C address defaults to x60 in the NT7S lib
unsigned long int  freq_hz = 144060000;  
//unsigned long int  freq_hz = 14061000;  // Set your preferred initial frequency here
unsigned long int  step_f  =  1000;     // frequency step defaults to 1kHz

// Displays and display code

#ifdef DISPLAY_OLED128x32
// Adafruit OLED 128x32 0.91"OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 oled128x32(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void refresh_OLED128x32()
{
  char fb[16], stp[16];

  oled128x32.clearDisplay();
  delay(1);
  oled128x32.setTextSize(2);
  oled128x32.setTextColor(SSD1306_WHITE);  // Draw white text
  oled128x32.setCursor(0, 0);

  sprintf(fb, "%d", freq_hz);
  // Serial.print(fb); Serial.println('/');
 
  String s(fb);
  //Serial.print(s); Serial.println('/');
  byte k=0;
  oled128x32.print(s.charAt(k));
  if((freq_hz/1000000) > 100) oled128x32.print(s.charAt(++k));
  if((freq_hz/1000000) >= 10) oled128x32.print(s.charAt(++k));
  if((freq_hz/1000000000) >= 10) oled128x32.print(s.charAt(++k));

  oled128x32.print(',');
  //if((freq_hz/1000000) < 100) oled128x32.print(',');
  //else oled128x32.print(s.charAt(++k));
  
  byte j;
  for (j=k+1; j<=(k+3); j++) oled128x32.print(s.charAt(j));
  oled128x32.print('.');
  oled128x32.print(s.charAt(j+1));

  // write row 2...
  oled128x32.setCursor(0, 18);
//  sprintf(stp, "%d", step_f);
//  String s2(stp);
//  for (j=0; j<=s2.length(); j++) oled128x32.print(s2.charAt(j));

  if(step_f == 1000000) oled128x32.print("1MHz");
  else if(step_f == 100000) oled128x32.print("100kHz");
  else if(step_f == 10000) oled128x32.print("10kHz");
  else if(step_f == 1000) oled128x32.print("1kHz");
  oled128x32.display();
  delay(1);
};
#endif


#ifdef DISPLAY_LCD
// HD7044 16x2 Liquid Crystal Display

#define LCD_RS    8  // Register Select is LCD pin 4
#define LCD_E     9  // Enable/clock LCD pin 6
#define LCD_D4   10  // LCD D4
#define LCD_D5   11  // LCD D5
#define LCD_D6   12  // LCD D6
#define LCD_D7   13  // LCD D7  

#include <LiquidCrystal.h>
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

void refresh_LCD() {
// Update the LCD  
  uint16_t f_MHz, f_kHz, f_Hz, f;
  uint32_t vfo_l;

  lcd.setCursor(0, 0);
  f = freq_hz / 1000000;  
  if(f<900) lcd.print(' ');
  if(f<10) lcd.print(' ');
  lcd.print(f);
  lcd.print(',');

  f_MHz = (freq_hz % 1000000) / 1000;
  if (f_MHz < 100) lcd.print('0');
  if (f_MHz < 10) lcd.print('0');
  lcd.print(f_MHz);
  lcd.print(',');
 
  f_kHz = freq_h % 1000;
  if (f_kHz < 100) lcd.print('0');
  if (f_kHz < 10) lcd.print('0');
  lcd.print(f_kHz);
  lcd.print('.');

  f_Hz = freq_h % 10;
  if (f_Hz < 10) lcd.print('0');
  lcd.print(f_Hz);
 
//  lcd.print(" ");

// line 2
  lcd.setCursor(0, 1);  // start of line 2
  lcd.print("Step: ");
  lcd.print(step_hz);  
 
  switch (step_hz) {      
    case 100:
      lcd.print("100Hz");
    break;
    case 1000:
      lcd.print("1kHz");
    break;
    case 10000:
      lcd.pint("10kHz");
    break;
    case 100000:
      lcd.pint("100kHz");
    break;
    case 1000000:
      lcd.pint("1MHz");
    break;
  }
}
#endif
//------------------


#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(__AVR_ATmega328P__)
//  Nano/ATMega328 button pinouts 
#define BUTTON_UP 2  // A2 to 'Freq up' button
#define BUTTON_DN 4  // D4 to 'Freq down' button
#define STEP_UP   5  // D5 to 'Step' button
#define PTT       7  // D7 is Tx enable (high) for controlling downstream stages 
#endif

#if defined(ESP32)
// XIAO ESP32-C3 button pinouts 
#define  BUTTON_UP  20  // pin 8, GPIO20, pushbutton to increment freq
#define  BUTTON_DN   8  // pin 9, GPIO8,  pushbutton to decrement freq
#define  STEP_UP   9  // pin 10, GPIO9, increment the step (radix)
#define  PTT 10  // pin 11, GPIO10, Tx enable (high) for controlling downstream stages
#endif


// general purpose variables
unsigned long t = millis(); 
#define HALF_PERIOD_MS 500        // basic unit of time
#define MARK_SPACE_RATIO 3        // ratio of Tx time to off time
bool tx = 0; 
byte i=0; 
byte n = 0;
#define LINE_LENGTH_CHARS 40


void setup() {
  Serial.begin(9600);

  // initialise and start the si5351 clocks

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);         // If using 27Mhz xtal, put 27000000 instead of 0 (0 is the default xtal freq of 25Mhz)
  si5351.set_correction(132200, SI5351_PLL_INPUT_XO);

  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);

    // set up the Carrier Oscillator (CO) for CW 
  si5351.set_freq(freq_hz*SI5351_FREQ_MULT, SI5351_CLK0);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA);   // reduce this when testing in a transceiver
  si5351.output_enable(SI5351_CLK0, 1);
 
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DN, INPUT_PULLUP);
  pinMode(STEP_UP, INPUT_PULLUP);
  pinMode(PTT, OUTPUT);
  
  // initialise display

#ifdef DISPLAY_OLED128x32
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!oled128x32.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  oled128x32.clearDisplay();  // Clear the buffer
  refresh_OLED128x32(); 
#endif

#ifdef DISPLAY_LCD
  lcd.begin(16, 2);  
  lcd.cursor();
  lcd.noBlink();  
  lcd.setCursor(0,0);
  lcd.print("si5351 SigGen");
  lcd.setCursor(0,1);
  lcd.print("VK3HN 12/07/2020");
  delay(2000);
  lcd.clear();
  refresh_LCD(); 
#endif
  //Serial.println("setup");
}


//void setVfoFrequency(unsigned long int f) {
//  si5351.set_freq(f * SI5351_FREQ_MULT, SI5351_CLK0); //  
  //Serial.print("set frequency: ");  Serial.println(frequency);
  //printSi5351Status();
//}


char readPbtns()
{
  char r = '\0';

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(__AVR_ATmega328P__)
//  Classic AVR boards like Arduino Nano (ATmega328P)
  if(!digitalRead(BUTTON_UP)) r = 'u';  // up 
  else if(!digitalRead(BUTTON_DN)) r = 'd';  // down
  else if(!digitalRead(STEP_UP)) r = '+';  // increase step_f 
  delay(200); 
#endif

#if defined(ESP32)
// read the pushbuttons on resistive divider on A0 (GPIO2) 
  unsigned int v= analogRead(2);
  // Serial.println(v);
   //delay(500); 
  // while(analogRead(2) < 4000) delay(10); 
  delay(200); 
  //Serial.print("   >"); Serial.println(v);
  if(v < 1000) r = 'u';
  else if((v > 2000) and (v < 3400)) r = 'd';
  else if((v > 3500) and (v < 4000)) r = '+';
#endif

  return r;  
}



void loop() 
{
  // pulse CLK0 carrier
  if( (millis() - t) > HALF_PERIOD_MS)
  {
    i++;
    t= millis();
    if(tx)
    {
      Serial.print('-');  
      if(i > MARK_SPACE_RATIO)
      {
       // turn CLK0 off
        si5351.output_enable(SI5351_CLK0, 0);
        tx = 0;
        i=0;
        delay(10);
        digitalWrite(PTT, 0);  // disable downstream stages
      }
      
      //
      n++;
      if(n > LINE_LENGTH_CHARS)  
      {
        Serial.println();
        n=0;
      }
    } 
    else
    {
      // turn CLK0 on
      Serial.print(' ');
      digitalWrite(PTT, 1);  // enable downstream stages
      delay(10);
      si5351.output_enable(SI5351_CLK0, 1);
      tx = 1;
    } 
  };

   // check pushbuttons
  char b = readPbtns();
  bool changed = 0;
  if(b == 'u')
  {
    // PB1 pressed
    Serial.print(" up ");
    changed = 1;
    freq_hz = freq_hz + step_f;
    delay(100);
  }
  else if(b == 'd')
  {
    // PB2 pressed
    Serial.print(" down ");
    changed = 1;
    if(freq_hz >= step_f) freq_hz = freq_hz - step_f;
    delay(100);
  }
  else if (b == '+')
  {
    // PB3 pressed
    if(step_f == 1000000) step_f = 100000;
    else if(step_f == 100000) step_f = 10000;
    else if(step_f == 10000) step_f = 1000;
    else if(step_f == 1000) step_f = 1000000;
    Serial.print(" step=");
    Serial.println(step_f);
    changed = 1;
    delay(100);
  };

  if(changed)
  {
    si5351.set_freq(freq_hz * SI5351_FREQ_MULT, SI5351_CLK0);
    Serial.print(freq_hz);
    Serial.print(" (");
    Serial.print(step_f);
    Serial.println(')');
#ifdef DISPLAY_LCD
    refresh_LCD(); 
    #endif
#ifdef DISPLAY_OLED128x32
    refresh_OLED128x32(); 
#endif
    changed = 0; 
  } 
}