/*
 * Graduation Cap LED Matrix Controller with Animations
 */

// Blynk Configuration
#define BLYNK_TEMPLATE_ID "TMPL2E7Ie-qfs"
#define BLYNK_TEMPLATE_NAME "MatrixDisplay"
#define BLYNK_AUTH_TOKEN "IyYgJr9rcBNq0ksF6KCOGdfYo5rtnOVf"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h"
#include <FastLED.h>

// Replace with your WiFi credentials
char ssid[] = "MST-GUEST";
char pass[] = "miner2020";

// Set up Blynk
char auth[] = BLYNK_AUTH_TOKEN;

// LED Matrix Configuration
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64
#define PANELS_NUMBER 1
#define PIN_E 32

#define PANE_WIDTH PANEL_WIDTH * PANELS_NUMBER
#define PANE_HEIGHT PANEL_HEIGHT

// placeholder for the matrix object
MatrixPanel_I2S_DMA *dma_display = nullptr;

// Animation variables
uint8_t currentAnimation = 0;
uint16_t time_counter = 0;
unsigned long animTimer = 0;
uint16_t framebuffer[PANE_WIDTH][PANE_HEIGHT];

// Text parameters
String scrollText = "GRAD 2025!";
int textX = PANE_WIDTH;
int textSpeed = 50;         // Faster default speed
int textColor = 0xFFFFFF;   // Default white
unsigned long textUpdateTimer = 0;

// FastLED color palette for some effects
CRGBPalette16 currentPalette = RainbowColors_p;

// LED Matrix and brightness control
uint8_t brightness = 50;  // Default 50% brightness (range 0-255)

// ************************************************************* BEGIN Blynk Digital Console Variables ***************************************

// Receive animation style from Blynk app
BLYNK_WRITE(V1) {
  currentAnimation = param.asInt();
  Serial.print("Animation changed to: ");
  Serial.println(currentAnimation);
  
  // Reset animation counters
  time_counter = 0;
  animTimer = millis();
}

// Modify the Blynk text handler
BLYNK_WRITE(V0) {
  scrollText = param.asStr();
  textX = PANE_WIDTH; // Reset text position
  Serial.print("New text (converted): ");
  Serial.println(scrollText);
}

// Receive text color from Blynk app
BLYNK_WRITE(V2) {
  textColor = param.asInt();
  Serial.print("Text color: 0x");
  Serial.println(textColor, HEX);
}

// Receive text speed
BLYNK_WRITE(V3) {
  int newSpeed = param.asInt();
  if (newSpeed < 1) newSpeed = 1;
  textSpeed = newSpeed;
  Serial.print("Text speed: ");
  Serial.println(textSpeed);
}

// Receive brightness level from Blynk app
BLYNK_WRITE(V4) {
  brightness = param.asInt();
  // Constrain brightness to valid range
  if (brightness < 0) brightness = 0;
  if (brightness > 255) brightness = 255;
  
  // Apply the new brightness immediately
  dma_display->setBrightness8(brightness);
  
  Serial.print("Brightness set to: ");
  Serial.println(brightness);
}

// ************************************************************* END Blynk Digital Console Variables ***************************************

void setup() {
  Serial.begin(115200);
  
  Serial.println(F("*****************************************************"));
  Serial.println(F("*  Graduation Cap LED Matrix with Animations        *"));
  Serial.println(F("*****************************************************"));

  // Set up LED matrix
  HUB75_I2S_CFG mxconfig;
  mxconfig.mx_height = PANEL_HEIGHT;
  mxconfig.chain_length = PANELS_NUMBER;
  mxconfig.gpio.e = PIN_E;

  // Create matrix object
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->setBrightness8(50);  // Set to about 75% brightness

  // Initialize matrix
  if (!dma_display->begin())
    Serial.println("****** !KABOOM! I2S memory allocation failed ***********");
 
  // Apply default brightness
  dma_display->setBrightness8(brightness);
 
  // Show startup message
  dma_display->fillScreenRGB888(0, 0, 0);
  dma_display->setTextSize(1);
  dma_display->setTextColor(dma_display->color565(255, 255, 255));
  dma_display->setCursor(5, 28);
  dma_display->print("Connecting...");
  
  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);
  
  // Initialize timers
  textUpdateTimer = millis();
  animTimer = millis();
  
  // Clear display
  dma_display->fillScreenRGB888(0, 0, 0);
}

// ************************************************************ Default Scrolling Text ************************************************

// Animation 0: Scrolling Text
void drawScrollingText() {
  // Clear the display
  dma_display->fillScreenRGB888(0, 0, 0);
  
  // Set text properties
  dma_display->setTextSize(2);
  dma_display->setTextWrap(false);
  
  // Extract RGB from textColor
  uint8_t r = (textColor >> 16) & 0xFF;
  uint8_t g = (textColor >> 8) & 0xFF;
  uint8_t b = textColor & 0xFF;
  
  dma_display->setTextColor(dma_display->color565(r, g, b));
  dma_display->setCursor(textX, 24);
  dma_display->print(scrollText);
  
  // Update text position
  unsigned long scrollDelay = 150 - (textSpeed * 1.5);
  if (scrollDelay < 5) scrollDelay = 5;
  
  if (millis() - textUpdateTimer >= scrollDelay) {
    textX--;
    
    // Reset text position when it's completely off-screen
    int16_t textWidth = scrollText.length() * 12;
    if (textX < -textWidth) {
      textX = PANE_WIDTH;
    }
    
    textUpdateTimer = millis();
  }
}

// ********************************************** Draw pixel helper function to fix error ***************************************

void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  dma_display->drawPixelRGB888(x, y, r, g, b);
  if (x >= 0 && x < PANE_WIDTH && y >= 0 && y < PANE_HEIGHT) {
    // Convert RGB888 to RGB565 and store in framebuffer
    uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    framebuffer[x][y] = color;
  }
}

//******************************************************** Animations Begin ***************************************************

// Animation 1: Plasma Effect
void drawPlasmaAnimation() {
  for (int x = 0; x < PANE_WIDTH; x++) {
    for (int y = 0; y < PANE_HEIGHT; y++) {
      int16_t v = 0;
      uint8_t wibble = sin8(time_counter);
      v += sin16(x * wibble * 3 + time_counter);
      v += cos16(y * (128 - wibble) + time_counter);
      v += sin16(y * x * cos8(-time_counter) / 8);

      CRGB rgb = ColorFromPalette(currentPalette, (v >> 8) + 127);
      dma_display->drawPixelRGB888(x, y, rgb.r, rgb.g, rgb.b);
    }
  }
  
  // Update animation state
  if (millis() - animTimer >= 30) {  // 30ms delay for ~33fps
    time_counter++;
    animTimer = millis();
  }
}

// Animation 2: Graduation Cap Icon
void drawGraduationCap(int tasselOffset = 0) {
  // Clear screen
  dma_display->fillScreenRGB888(0, 0, 0);

  // Extract RGB from textColor
  uint8_t r = (textColor >> 16) & 0xFF;
  uint8_t g = (textColor >> 8) & 0xFF;
  uint8_t b = textColor & 0xFF;
  uint16_t capColor = dma_display->color565(r, g, b);

  // Coordinates for the cap top (diamond)
  int centerX = 32;
  int centerY = 20;

  // Diamond points
  int capTop[4][2] = {
    {centerX, centerY - 10}, // Top point
    {centerX + 20, centerY}, // Right point
    {centerX, centerY + 10}, // Bottom point
    {centerX - 20, centerY}  // Left point
  };

  // Draw the cap top (diamond)
  dma_display->fillTriangle(capTop[0][0], capTop[0][1], capTop[1][0], capTop[1][1], capTop[2][0], capTop[2][1], capColor);
  dma_display->fillTriangle(capTop[0][0], capTop[0][1], capTop[2][0], capTop[2][1], capTop[3][0], capTop[3][1], capColor);

  // Draw the button on top
  dma_display->fillCircle(centerX, centerY - 10, 2, capColor);

  // Draw the hanging tassel
  int tasselX = centerX + tasselOffset;
  dma_display->drawLine(centerX, centerY - 10, tasselX, centerY + 10, capColor);
  dma_display->fillCircle(tasselX, centerY + 12, 2, capColor); // end of tassel
}


void animateGraduationCap() {
  int angle = 0;
  int direction = 1;

  for (int i = 0; i < 100; i++) { // Adjust 100 to control how long it runs
    drawGraduationCap(sin(radians(angle)) * 5); // Swing 5 pixels left/right
    delay(30); // Control speed of animation
    angle += direction * 5;
    if (angle > 30 || angle < -30) {
      direction = -direction;
    }
  }
}

// Animation 3: Fireworks
void drawFireworks() {
  // Fade existing pixels
  for (int y = 0; y < PANE_HEIGHT; y++) {
    for (int x = 0; x < PANE_WIDTH; x++) {
      uint16_t color = framebuffer[x][y];
      uint8_t r = ((color >> 11) & 0x1F) * 8;  // Extract red (5 bits)
      uint8_t g = ((color >> 5) & 0x3F) * 4;   // Extract green (6 bits)
      uint8_t b = (color & 0x1F) * 8;          // Extract blue (5 bits)
      
      // Fade pixel
      r = r > 10 ? r - 10 : 0;
      g = g > 10 ? g - 10 : 0;
      b = b > 10 ? b - 10 : 0;
      
      setPixel(x, y, r, g, b);
    }
  }
  
  // Occasionally add a new firework
  if (random8() < 5) {  // ~2% chance per frame
    // Firework origin at bottom of display
    int origin_x = random16(PANE_WIDTH);
    
    // Firework explosion coordinates
    int explosion_x = origin_x + random16(21) - 10;  // ±10 pixels from origin
    int explosion_y = random16(PANE_HEIGHT/2);       // Upper half of the display
    
    // Color for this firework
    uint8_t hue = random8();
    
    // Draw explosion particles
    for (int i = 0; i < 30; i++) {  // 30 particles per firework
      int dx = random16(21) - 10;  // ±10 pixels
      int dy = random16(21) - 10;  // ±10 pixels
      
      int px = explosion_x + dx;
      int py = explosion_y + dy;
      
      // Make sure pixel is within bounds
      if (px >= 0 && px < PANE_WIDTH && py >= 0 && py < PANE_HEIGHT) {
        // Slightly vary hue for each particle
        CRGB rgb = CHSV(hue + random8(30), 200, 255);
        setPixel(px, py, rgb.r, rgb.g, rgb.b);
      }
    }
  }
  
  // Update animation state
  if (millis() - animTimer >= 30) {
    time_counter++;
    animTimer = millis();
  }
}

// Animation 4: Waving Flag (school colors)
void drawWavingFlag() {
  // Use school colors for stripes
  // Assuming gold and black as example school colors - change to your school colors!
  uint16_t color1 = dma_display->color565(21, 71, 52);  // Forest Green
  uint16_t color2 = dma_display->color565(206, 184, 136);      // Gold
  
  for (int y = 0; y < PANE_HEIGHT; y++) {
    // Calculate wave offset for this row
    float wave = sin16((y * 128) + (time_counter * 200)) / 2048.0;
    int offset = wave * 10;  // Wave amplitude
    
    for (int x = 0; x < PANE_WIDTH; x++) {
      // Determine which color stripe this pixel is in
      int stripeWidth = 8;  // Width of each color stripe
      int adjustedX = x + offset;
      int stripeNum = (adjustedX / stripeWidth) % 2;
      
      // Alternate between school colors
      uint16_t pixelColor = (stripeNum == 0) ? color1 : color2;
      dma_display->drawPixel(x, y, pixelColor);
    }
  }
  
  // Update animation state
  if (millis() - animTimer >= 40) {
    time_counter++;
    animTimer = millis();
  }
}

//******************************************************** Animations End ***************************************************

void loop() {
  // Run Blynk
  Blynk.run();
  
  // Run the current animation
  switch (currentAnimation) {
    case 0:
      drawScrollingText();
      break;
    case 1:
      drawPlasmaAnimation();
      break;
    case 2:
      animateGraduationCap();
      break;
    case 3:
      drawFireworks();
      break;
    case 4:
      drawWavingFlag();
      break;
    default:
      drawScrollingText();  // Default to text scrolling
      break;
  }
  
  // Brief delay to prevent overwhelming the ESP32
  delay(5);
}