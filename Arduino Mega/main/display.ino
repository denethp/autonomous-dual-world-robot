#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void initOLED() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}

void showText(String text, int size = 2, int x = 0, int y = 20) {
  display.clearDisplay();
  display.setTextSize(size);
  display.setTextColor(WHITE);
  display.setCursor(x, y);
  display.print(text);
  display.display();
}

void showTwoLines(String line1, String line2, int size = 2) {
  display.clearDisplay();

  display.setTextSize(size);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print(line1);

  display.setCursor(0, 20);  
  display.print(line2);

  display.display();
}
