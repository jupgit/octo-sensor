/*  OLEDMeter was written to utilize any 128x64 display. I have only seen marginal attempts to
 *  animate meters and I hope this one will set a standard. Please feel free to modify and share
 *  this code for any 128x64 LCD or OLED. OLEDMeter sketch was written for use with I2C SH1106.
 *  This code must be modified to work with other display devices.
 *  
 *  Working portion of code was taken from Adafruit Example Sound Level Sketch for the
 *  Adafruit Microphone Amplifier
 *  https://learn.adafruit.com/adafruit-microphone-amplifier-breakout/measuring-sound-levels
 *  
 *  Remaining code was written by Greg Stievenart with no claim to or any images or information 
 *  provided in this code. Freely published May 26, 2016.
 *  
 *  Software to convert background mask to 128x64 at: http://www.ablab.in/image2glcd-software/
 *  
 *  IMPORTANT: Sound source must be grounded to the Arduino or other MCU's to work. Usually the
 *  base sleeve contact on TRS or TRRS connector is the ground.
 */

/*
 * 
 *
 */

 
#include <SPI.h>
#include <Wire.h>

//i2c comm with Nano
#define PAYLOAD_SIZE 2 // how many bytes to expect from each I2C salve node
#define NODE_MAX 1 // maximum number of slave nodes (I2C addresses) to probe
#define START_NODE 1 // The starting I2C address of slave nodes
#define NODE_READ_DELAY 200 // Some delay between I2C node reads

int nodePayload[PAYLOAD_SIZE];



#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// POT
// Potentiometer is connected to GPIO 34 (Analog ADC1_CH6) 
const int potPin = 34;

// variable for storing the potentiometer value
int potValue = 0;

int MeterValue = 512;
int hMeter = 65;                      // horizontal center for needle animation
int vMeter = 85;                      // vertical center for needle animation (outside of dislay limits)
int rMeter = 80;                      // length of needle animation or arch of needle travel

// VU meter background mask image:
static const unsigned char PROGMEM VUMeter[] = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x00, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x09, 0x04, 0x80, 0x21, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x98, 0x08, 0x06, 0x03, 0x80, 0x21, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xA4, 0x10, 0x09, 0x00, 0x80, 0x21, 0x20, 0x07, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xA4, 0x10, 0x06, 0x03, 0x00, 0x20, 0xC0, 0x00, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x71, 0x80, 0xA4, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0A, 0x40, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3C, 0x00, 0x00,
  0x00, 0x00, 0x3A, 0x40, 0x00, 0x00, 0x02, 0x01, 0x00, 0x40, 0x80, 0x07, 0x00, 0x20, 0x00, 0x00,
  0x00, 0x00, 0x42, 0x40, 0x00, 0x08, 0x02, 0x01, 0x08, 0x40, 0x80, 0x00, 0x00, 0x38, 0x00, 0x00,
  0x00, 0x00, 0x79, 0x80, 0x04, 0x08, 0x02, 0x01, 0x08, 0x81, 0x10, 0x00, 0x00, 0x04, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x04, 0x08, 0x02, 0x01, 0x08, 0x81, 0x11, 0x04, 0x00, 0x38, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x02, 0x01, 0x08, 0x81, 0x21, 0x04, 0x00, 0x00, 0x08, 0x00,
  0x00, 0x00, 0x00, 0x84, 0x02, 0x04, 0x0F, 0xFF, 0xFF, 0xC3, 0xE2, 0x04, 0x00, 0x00, 0x08, 0x00,
  0x00, 0x00, 0x00, 0xC2, 0x01, 0x07, 0xF0, 0x00, 0x00, 0x3B, 0xFE, 0x08, 0x40, 0x40, 0x08, 0x00,
  0x00, 0xFE, 0x00, 0x62, 0x01, 0xF8, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xE8, 0x40, 0x80, 0x7F, 0x00,
  0x00, 0x00, 0x00, 0x21, 0x1E, 0x00, 0x04, 0x00, 0x80, 0x00, 0x7F, 0xFE, 0x80, 0x80, 0x08, 0x00,
  0x00, 0x00, 0x03, 0x31, 0xE0, 0x00, 0x04, 0x00, 0x80, 0x04, 0x01, 0xFF, 0xC1, 0x00, 0x08, 0x00,
  0x00, 0x00, 0x07, 0x1E, 0x00, 0x40, 0x00, 0x00, 0x00, 0x04, 0x00, 0x1F, 0xFA, 0x00, 0x08, 0x00,
  0x00, 0x00, 0x07, 0xF0, 0x00, 0x40, 0x3B, 0x07, 0x60, 0x00, 0x00, 0x01, 0xFF, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x34, 0x81, 0x90, 0xCC, 0xC0, 0x00, 0x3F, 0xC0, 0x00, 0x00,
  0x00, 0x00, 0x0C, 0x00, 0x03, 0x30, 0x0C, 0x82, 0x90, 0x53, 0x20, 0x00, 0x07, 0xF8, 0x00, 0x00,
  0x00, 0x00, 0x70, 0x40, 0x00, 0xC8, 0x3B, 0x02, 0x60, 0x53, 0x20, 0x00, 0x00, 0xFE, 0x00, 0x00,
  0x00, 0x01, 0x80, 0x20, 0x01, 0xC8, 0x00, 0x00, 0x00, 0x4C, 0xC0, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x06, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00,
  0x00, 0x08, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFC, 0x00,
  0x00, 0x30, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00,
  0x00, 0x00, 0x40, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
  0x00, 0x00, 0xA0, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x02, 0x02, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x03, 0x06, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x8C, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x70, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void setup(){


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  //display.invertDisplay(1);

  // Draw a single pixel in white
  display.drawPixel(63, 31, SSD1306_WHITE);

    Serial.begin(9600);  
  Serial.println("MASTER READER NODE");
  Serial.print("Maximum Slave Nodes: ");
  Serial.println(NODE_MAX);
  Serial.print("Payload size: ");
  Serial.println(PAYLOAD_SIZE);
  Serial.println("***********************");
  
  Wire.begin();        // Activate I2C link                         
}

void loop(){
  
    //request sensor data from Nano
    Wire.requestFrom(1, PAYLOAD_SIZE);    // request data from node#
    if(Wire.available() == PAYLOAD_SIZE) {  // if data size is avaliable from nodes
      for (int i = 0; i < PAYLOAD_SIZE; i++) nodePayload[i] = Wire.read();  // get nodes data
      //for (int j = 0; j < PAYLOAD_SIZE; j++) Serial.println(nodePayload[j]);   // print nodes data   
      //Serial.println("*************************");      
      }
    delay(NODE_READ_DELAY);

     for (int j = 0; j < PAYLOAD_SIZE; j++) Serial.println(nodePayload[j]);   // print nodes data   

 

  potValue = analogRead(potPin);
  MeterValue = map(potValue, 0, 4096, 0, 80);
  
  MeterValue = MeterValue - 34;                            // shifts needle to zero position
  display.clearDisplay();                                  // refresh display for next step
  display.drawBitmap(0, 0, VUMeter, 128, 64, WHITE);       // draws background
  int a1 = (hMeter + (sin(MeterValue / 57.296) * rMeter)); // meter needle horizontal coordinate
  int a2 = (vMeter - (cos(MeterValue / 57.296) * rMeter)); // meter needle vertical coordinate
  display.drawLine(a1, a2, hMeter, vMeter, WHITE);         // draws needle
  display.display();

  
}
