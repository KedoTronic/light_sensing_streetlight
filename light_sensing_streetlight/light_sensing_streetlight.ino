#include <Wire.h>
#include <BH1750.h>
#include <U8g2lib.h>

// Rotate the 128×64 OLED by 90° and draw using a 64×128 portrait coordinate system.
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R1, /* reset=*/ U8X8_PIN_NONE);
BH1750 lightMeter;

const int ledPin = 4; // LED connected to GPIO4
const float lightOnThreshold = 45.0;  // Turn on the light below this illuminance.
const float lightOffThreshold = 55.0; // Turn off the light above this illuminance.
bool lightOn = false;

// Draw a reference-style bulb: circular outline, internal highlight, surrounding rays, and a segmented threaded base.
void drawLampIcon(uint8_t x, uint8_t y, bool isOn) {
  // Fill the bulb when on; when off, draw a thick outline with two concentric circles.
  if (isOn) {
    u8g2.drawDisc(x, y, 18, U8G2_DRAW_ALL);
  } else {
    u8g2.drawCircle(x, y, 18, U8G2_DRAW_ALL);
    u8g2.drawCircle(x, y, 17, U8G2_DRAW_ALL);
  }
  u8g2.setDrawColor(0);
  u8g2.drawBox(x - 19, y + 8, 39, 14); // Erase the bottom of the circle to form a narrowed bulb base.
  u8g2.setDrawColor(1);
  if (isOn) {
    // The narrowed connection below the bulb also glows, so fill it as a solid trapezoid.
    u8g2.drawTriangle(x - 16, y + 8, x + 16, y + 8, x - 9, y + 18);
    u8g2.drawTriangle(x + 16, y + 8, x + 9, y + 18, x - 9, y + 18);
  }
  u8g2.drawLine(x - 16, y + 8, x - 9, y + 18);
  u8g2.drawLine(x + 16, y + 8, x + 9, y + 18);
  u8g2.drawFrame(x - 9, y + 18, 19, 9); // Upper edge of the base
  u8g2.drawRFrame(x - 10, y + 29, 21, 7, 3); // First thread segment
  u8g2.drawRFrame(x - 8, y + 38, 17, 7, 3); // Second thread segment
  u8g2.drawRBox(x - 5, y + 46, 11, 4, 2); // Bottom contact

  if (isOn) {
    // Seven surrounding light rays from the reference image
    u8g2.drawLine(x, y - 26, x, y - 21);
    u8g2.drawLine(x - 20, y - 20, x - 16, y - 16);
    u8g2.drawLine(x + 20, y - 20, x + 16, y - 16);
    u8g2.drawLine(x - 26, y, x - 21, y);
    u8g2.drawLine(x + 21, y, x + 26, y);
    u8g2.drawLine(x - 21, y + 16, x - 17, y + 13);
    u8g2.drawLine(x + 21, y + 16, x + 17, y + 13);

  }
}

void setup(){
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB10_tr); // Set the font
  lightMeter.begin();
  pinMode(ledPin, OUTPUT);
}

void loop(){
  float lux = lightMeter.readLightLevel();
  char luxStr[10]; // Character buffer for the illuminance string
  dtostrf(lux, 6, 0, luxStr); // Convert float to string: total width 6, 1 decimal place
  
  // Hysteresis control: prevent repeated switching when illuminance fluctuates near the thresholds.
  if (!lightOn && lux < lightOnThreshold) {
    lightOn = true;
  } else if (lightOn && lux > lightOffThreshold) {
    lightOn = false;
  }
  digitalWrite(ledPin, lightOn ? HIGH : LOW);

  // Print the current illuminance and streetlight state to the serial monitor for debugging.
  Serial.print("Light: ");
  Serial.print(lux, 1);
  Serial.print(" lx, Streetlight: ");
  Serial.println(lightOn ? "ON" : "OFF");

  // Portrait OLED: show illuminance in the top panel and a complete large bulb below.
  u8g2.clearBuffer();
  u8g2.drawFrame(1, 1, 62, 43);
  u8g2.drawStr(14,17,"Light");
  u8g2.drawStr(3,37, luxStr); // Display the illuminance value
  u8g2.drawStr(46,37,"lx");
  drawLampIcon(32, 77, lightOn);
  u8g2.sendBuffer();

  delay(500);
}
