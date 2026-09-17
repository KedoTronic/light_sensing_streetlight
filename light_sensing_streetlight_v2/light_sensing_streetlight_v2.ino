#include <Wire.h>
#include <BH1750.h>
#include <U8g2lib.h>

// Rotate the 128×64 OLED by 90° and draw using a 64×128 portrait coordinate system.
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R1, /* reset=*/ U8X8_PIN_NONE);
BH1750 lightMeter;

const int ledPin = 4; // LED connected to GPIO4
const float lightOnThreshold = 45.0;  // Turn the light on below this illuminance.
const float lightOffThreshold = 55.0; // Turn the light off above this illuminance.
bool lightOn = false;

// When the LED is on, display nighttime: a crescent moon with scattered starlight.
void drawMoonAndStars() {
  // Keep every star at least 4 pixels outside the moon's outline to avoid visual overlap with the crescent.
  const uint8_t smallStars[][2] = {
    {6, 52}, {20, 54}, {45, 51}, {57, 61}, {59, 78},
    {57, 99}, {51, 114}, {32, 121}, {12, 116}, {5, 104}
  };
  const uint8_t brightStars[][2] = {{7, 73}, {55, 88}, {3, 94}, {44, 61}, {45, 120}};

  // Subtract two offset circles to form a softly edged crescent that opens to the right.
  u8g2.drawDisc(28, 86, 19, U8G2_DRAW_ALL);
  u8g2.setDrawColor(0);
  u8g2.drawDisc(37, 79, 19, U8G2_DRAW_ALL);
  u8g2.setDrawColor(1);

  // Draw the stars last; their positions avoid the moon, keeping their outlines clear and uncovered.
  for (uint8_t i = 0; i < sizeof(smallStars) / sizeof(smallStars[0]); i++) {
    u8g2.drawDisc(smallStars[i][0], smallStars[i][1], 1, U8G2_DRAW_ALL);
  }
  for (uint8_t i = 0; i < sizeof(brightStars) / sizeof(brightStars[0]); i++) {
    uint8_t x = brightStars[i][0];
    uint8_t y = brightStars[i][1];
    u8g2.drawLine(x - 2, y, x + 2, y);
    u8g2.drawLine(x, y - 2, x, y + 2);
  }
}

// When the LED is off, display daytime: a smiling sun with twelve rounded rays in the reference-image style.
void drawSun() {
  const int8_t rayEnds[][4] = {
    {32, 61, 32, 55}, {44, 65, 49, 60}, {53, 74, 58, 71},
    {56, 86, 62, 86}, {53, 98, 58, 101}, {44, 107, 49, 112},
    {32, 111, 32, 117}, {20, 107, 15, 112}, {11, 98, 6, 101},
    {8, 86, 2, 86}, {11, 74, 6, 71}, {20, 65, 15, 60}
  };

  for (uint8_t i = 0; i < sizeof(rayEnds) / sizeof(rayEnds[0]); i++) {
    // Use a single uniform-width line to prevent the endpoints from appearing thicker than the middle.
    u8g2.drawLine(rayEnds[i][0], rayEnds[i][1], rayEnds[i][2], rayEnds[i][3]);
  }
  u8g2.drawDisc(32, 86, 17, U8G2_DRAW_ALL);

  // Use black lines on the monochrome display for closed eyes and a smile, keeping the expression clear without crowding it.
  u8g2.setDrawColor(0);
  u8g2.drawLine(23, 81, 26, 78); // Left eye: upward curve
  u8g2.drawLine(26, 78, 29, 81);
  u8g2.drawLine(35, 81, 38, 78); // Right eye: upward curve
  u8g2.drawLine(38, 78, 41, 81);
  u8g2.drawCircle(32, 88, 9, U8G2_DRAW_LOWER_LEFT | U8G2_DRAW_LOWER_RIGHT); // Smiling mouth
  u8g2.setDrawColor(1);
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
  dtostrf(lux, 6, 0, luxStr); // Convert the float to a string: total width 6, 1 digit after the decimal point
  
  // Hysteresis control: prevent repeated switching when illuminance fluctuates near the thresholds.
  if (!lightOn && lux < lightOnThreshold) {
    lightOn = true;
  } else if (lightOn && lux > lightOffThreshold) {
    lightOn = false;
  }
  digitalWrite(ledPin, lightOn ? HIGH : LOW);

  // Output the current illuminance and streetlight state to the Serial Monitor for debugging.
  Serial.print("Light: ");
  Serial.print(lux, 1);
  Serial.print(" lx, Streetlight: ");
  Serial.println(lightOn ? "ON" : "OFF");

  // Portrait OLED layout: show illuminance at the top and a day or night pattern below according to the LED state.
  u8g2.clearBuffer();
  u8g2.drawFrame(1, 1, 62, 43);
  u8g2.drawStr(14,17,"Light");
  u8g2.drawStr(3,37, luxStr); // Output the illuminance value
  u8g2.drawStr(46,37,"lx");
  if (lightOn) {
    drawMoonAndStars();
  } else {
    drawSun();
  }
  u8g2.sendBuffer();

  delay(500);
}
