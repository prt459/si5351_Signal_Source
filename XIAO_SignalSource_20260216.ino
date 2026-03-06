/*  
Signal source for testing receivers etc
Written for Arduino Nano or XIAO ESP32 and si5351 breakout  
Written by Paul VK3HN (https://vk3hn.wordpress.com/) 
// I2C devices and addresses:
 
// si5351   0x60  // Multisynth PLL

Labels that need to be #define'd for your target radio/rig/project:
  Display technology     {DISPLAY_LCD, DISPLAY_OLED}

Known bugs:
- EEPROM bytes 4..7 (step_f) not being written and read properly on ESP32


*/
#define DISPLAY_OLED128x32
//#define DISPLAY_LCD


#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#define EEPROM_ADDR 0    // starting address
#define EEPROM_SIZE 512  // reserve EEPROM bytes


#include <si5351.h>     // Etherkit si3531 library from NT7S,  V2.1.4   https://github.com/etherkit/Si5351Arduino 
Si5351 si5351;          // I2C address defaults to x60 in the NT7S lib
unsigned long int  freq_hz =  14060000;  
//unsigned long int  freq_hz = 14061000;  // Set your preferred initial frequency here
unsigned long      step_f  =  1000;     // frequency step defaults to 1kHz
//unsigned long      step_f;     // frequency step, read from EEPROM 

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
  //Serial.print(fb); Serial.println('/');
 
  String s(fb);
  //Serial.print(s); Serial.println('/');
  byte k=0;
  //oled128x32.print(s.charAt(k++));
  unsigned int mhz = freq_hz/1000000;
  //Serial.print(mhz); Serial.println('!');
  if(mhz > 100) { oled128x32.print(s.charAt(k)); k++; };
  if(mhz >= 10) { oled128x32.print(s.charAt(k)); k++; };
  if(mhz >=  1) { oled128x32.print(s.charAt(k)); k++; };
 // if((freq_hz/1000000000) >= 10) oled128x32.print(s.charAt(++k));

  oled128x32.print(',');
  //if((freq_hz/1000000) < 100) oled128x32.print(',');
  //else oled128x32.print(s.charAt(++k));
  
  byte j;
  for (j=k; j<=(k+2); j++) oled128x32.print(s.charAt(j));
  oled128x32.print('.');
  oled128x32.print(s.charAt(k+3));

  // write row 2...
  oled128x32.setCursor(0, 18);
//  sprintf(stp, "%d", step_f);
//  String s2(stp);
//  for (j=0; j<=s2.length(); j++) oled128x32.print(s2.charAt(j));

  if(step_f == 1000000) oled128x32.print("1MHz");
  else if(step_f == 100000) oled128x32.print("100kHz");
  else if(step_f == 10000) oled128x32.print("10kHz");
  else if(step_f == 1000) oled128x32.print("1kHz");
  else if(step_f == 100) oled128x32.print("100Hz");
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

  // line 1
  lcd.clear();
  lcd.print("F0: ");   // Future enhancement will provide for two outputs
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
 
  f_kHz = freq_hz % 1000;
  if (f_kHz < 100) lcd.print('0');
  if (f_kHz < 10) lcd.print('0');
  lcd.print(f_kHz);
  //lcd.print('.');      Don't display values after the decimal point

  //f_Hz = freq_hz % 10;
  //if (f_Hz < 10) lcd.print('0');
  //lcd.print(f_Hz); 

// line 2
  lcd.setCursor(0, 1);  // start of line 2
  lcd.print("Step: ");
  //lcd.print(step_f);   This statement not needed 
 
  switch (step_f) {      
    case 100:
      lcd.print("100Hz");
    break;
    case 1000:
      lcd.print("1kHz");
    break;
    case 10000:
      lcd.print("10kHz");
    break;
    case 100000:
      lcd.print("100kHz");
    break;
    case 1000000:
      lcd.print("1MHz");
    break;
  }
}
#endif
//------------------


#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(__AVR_ATmega328P__)
//  Nano/ATMega328 button pinouts 
#define BUTTON_UP A0  // 'Freq up' button
#define BUTTON_DN A2  // 'Freq down' button
#define STEP_UP   A1  // 'Step' button
#define PTT       7  //   Tx enable (high) for controlling downstream stages 
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


void readMyEEPROM()
{
// Read freq and step back from EEPROM
  unsigned long int readValue = 0;
  readValue |= ((unsigned long )EEPROM.read(EEPROM_ADDR + 0) << 24);  // MSB
  readValue |= ((unsigned long )EEPROM.read(EEPROM_ADDR + 1) << 16);
  readValue |= ((unsigned long )EEPROM.read(EEPROM_ADDR + 2) <<  8);
  readValue |=  EEPROM.read(EEPROM_ADDR + 3);                       // LSB
  Serial.print("Read from x00:");  Serial.println((unsigned long)readValue );
  freq_hz = readValue; 
  if(readValue == 0) freq_hz = 7032000; 

  readValue |= ((unsigned long )EEPROM.read(EEPROM_ADDR + 4) << 24);  // MSB
  readValue |= ((unsigned long )EEPROM.read(EEPROM_ADDR + 5) << 16);
  readValue |= ((unsigned long )EEPROM.read(EEPROM_ADDR + 6) <<  8);
  readValue |=  EEPROM.read(EEPROM_ADDR + 7);                       // LSB
  Serial.print("Read from x03:");  Serial.println((unsigned long)readValue);
//  step_f = readValue; 
//  if(step_f == 0) step_f = 1000;  
}


void setup() {
  Serial.begin(9600);

#if defined(ESP32)
  EEPROM.begin(EEPROM_SIZE);   // must call once
#endif
// Read freq and step back from EEPROM
  unsigned long readValue = 0;
  readValue |= ((unsigned long )EEPROM.read(EEPROM_ADDR + 0) << 24);  // MSB
  readValue |= ((unsigned long )EEPROM.read(EEPROM_ADDR + 1) << 16);
  readValue |= ((unsigned long )EEPROM.read(EEPROM_ADDR + 2) <<  8);
  readValue |=  EEPROM.read(EEPROM_ADDR + 3);                       // LSB
  Serial.print("Read from x00:");  Serial.println((unsigned long)readValue );
  freq_hz = readValue; 
  if(readValue == 0) freq_hz = 7032000; 

  // read the step size
  uint8_t b = EEPROM.read(EEPROM_ADDR + 4); 
  Serial.print("Read from x04:");  Serial.println(b);
  switch(b) {
        case 1: step_f = 1000000; break; 
        case 2: step_f = 100000; break; 
        case 3: step_f = 10000; break; 
        case 4: step_f = 1000; break; 
        case 5: step_f = 100; break; 
  };
  

//readValue |= ((unsigned long )EEPROM.read(EEPROM_ADDR + 5) << 16);
// readValue |= ((unsigned long )EEPROM.read(EEPROM_ADDR + 6) <<  8);
//  readValue |=  EEPROM.read(EEPROM_ADDR + 7);                       // LSB
//  Serial.print("Read from x03:");  Serial.println((unsigned long)readValue);
  //step_f = readValue; 
  //if(step_f == 0) step_f = 1000;   
 
  //Serial.print("setup freq="); Serial.println(freq_hz);
  //Serial.print("setup step="); Serial.println(step_f);


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
  lcd.print("VK3HN 20/02/2026");
  delay(2000);
  lcd.clear();
  refresh_LCD(); 
#endif

}


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
  bool changed_f = 0, changed_s = 0;

  if(b == 'u')
  {
    // PB1 pressed
    Serial.print(" up ");
    changed_f = 1;
    freq_hz = freq_hz + step_f;
    delay(100);
  }
  else if(b == 'd')
  {
    // PB2 pressed
    Serial.print(" down ");
    changed_f = 1;
    if(freq_hz >= step_f) freq_hz = freq_hz - step_f;
    delay(100);
  }
  else if (b == '+')
  {
    // PB3 pressed
    if(step_f == 1000000) step_f = 100000;
    else if(step_f == 100000) step_f = 10000;
    else if(step_f == 10000) step_f = 1000;
    else if(step_f == 1000) step_f = 100;
    else if(step_f == 100) step_f = 1000000;
    Serial.print(" step=");
    Serial.println(step_f);
    changed_s = 1;
    delay(100);
  };

if(changed_f or changed_s)
  {
    // frequency or step changed 

    if(changed_f)
    {
      si5351.set_freq(freq_hz * SI5351_FREQ_MULT, SI5351_CLK0);
      //Serial.print(freq_hz);

      EEPROM.write(EEPROM_ADDR + 0, (freq_hz >> 24) & 0xFF);  // MSB
      EEPROM.write(EEPROM_ADDR + 1, (freq_hz >> 16) & 0xFF);
      EEPROM.write(EEPROM_ADDR + 2, (freq_hz >>  8) & 0xFF);
      EEPROM.write(EEPROM_ADDR + 3,  freq_hz        & 0xFF);  // LSB
    }
    else if(changed_s)
    {
      Serial.print(" (");
      Serial.print(step_f);
      Serial.println(')');

      uint8_t b; 
      switch(step_f) {
        case 1000000: b = 1; break; 
        case 100000: b = 2; break; 
        case 10000: b = 3; break; 
        case 1000: b = 4; break; 
        case 100: b = 5; break; 
      };
      EEPROM.write(EEPROM_ADDR + 4, b);  // MSB
      //EEPROM.write(EEPROM_ADDR + 5, (step_f >> 16) & 0xFF);
      //EEPROM.write(EEPROM_ADDR + 6, (step_f >>  8) & 0xFF);
      //EEPROM.write(EEPROM_ADDR + 7,  step_f        & 0xFF);  // LSB
    };
#if defined(ESP32)
    EEPROM.commit();  // must call to actually save to flash 
#endif

#ifdef DISPLAY_LCD
    refresh_LCD(); 
#endif
#ifdef DISPLAY_OLED128x32
    refresh_OLED128x32(); 
#endif
    delay(5);
    //readMyEEPROM();
    delay(5); 
    changed_f = 0; 
    changed_s = 0;
  } 
}
