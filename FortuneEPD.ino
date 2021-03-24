/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include "epd_driver.h"
#include "opensans18b.h"

#define USE_SERIAL Serial

WiFiMulti wifiMulti;
const char* WIFI_SSID = "Senzafibre";
const char* WIFI_PWD = "gatt0kak0ne";
char* URL = "http://fotoni.it/cgi-bin/fortune.cgi";

GFXfont currentFont;
enum alignment {LEFT, RIGHT, CENTER};
uint8_t *framebuffer;
int cursor_x = 8;
int cursor_y = 4;
char pageBuffer[0xffff];

void setFont(GFXfont const & font) {
  currentFont = font;
}

void InitialiseDisplay() {
  epd_init();
  framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
  if (!framebuffer) Serial.println("Memory alloc failed!");
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
  setFont(OpenSans18B);
}

void setup() {
  USE_SERIAL.begin(115200);
  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(200);
  }

  wifiMulti.addAP(WIFI_SSID, WIFI_PWD);
  InitialiseDisplay();
}

String getPage() {
  String payload;
  // wait for WiFi connection
  if ((wifiMulti.run() == WL_CONNECTED)) {

    HTTPClient http;

    USE_SERIAL.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(URL); //HTTP

    USE_SERIAL.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
        USE_SERIAL.println(payload);
      }
    } else {
      USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    return payload;
  }
}

void edp_update() {
  epd_poweron();      // Switch on EPD display
  epd_clear();
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
  cursor_x = 8;
  cursor_y = 4;
  write_string((GFXfont *)&OpenSans18B, pageBuffer, &cursor_x, &cursor_y, framebuffer);
  epd_draw_grayscale_image(epd_full_screen(), framebuffer); // Update the screen
  epd_poweroff_all(); // Switch off all power to EPD
}

void loop() {
  String page = getPage();
  int plength = page.length();
  page.toCharArray(pageBuffer, min(plength, 0xffff));
  edp_update();
  delay(60000);
}
