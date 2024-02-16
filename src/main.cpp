#include "Arduino.h"
#include <WiFi.h>
#include <time.h>
#include <driver/rtc_io.h>

// brownout
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"

#if defined(SERIAL_DEBUG)
#define DBG(x) Serial.println(x)
#else
#define DBG(...)
#endif

#include "./const.h"
#include "./connect.h"
#include "./ftp.h"
#include "./cam.h"


// NTP server details
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

String TIME_STAMP;

bool getTimeStamp() {
  // Get current epoch time
  time_t now;
  time(&now);

  // Convert epoch time to struct tm
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  delay(1000);

  timeinfo.tm_isdst = daylightOffset_sec;

  // Format and print the date and time
  char strftime_buf[64];
  strftime(strftime_buf, sizeof(strftime_buf), "%Y%m%d_%H%M%S", &timeinfo);
  if (strftime_buf[0] == '1') {
    DBG("Waiting for NTP time sync: " + String(strftime_buf));
    return false;
  } else {
    TIME_STAMP = strftime_buf;
    return true;
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  pinMode(4, OUTPUT);
  // digitalWrite(4, LOW);
  rtc_gpio_hold_dis(GPIO_NUM_4);  

  Serial.begin(115200);

#ifdef SERIAL_DEBUG
  Serial.setDebugOutput(true);
#endif

  wifiInit();
  wifiConnect();

  // Initialize time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Initialize camera
  if (!cam_init_ok) {
    cam_init_ok = cameraInit();
    DBG("Camera init ok");
  }

  while (!wifi_connected) {
    DBG("Waiting for WiFi connection");
    wifiConnect();
    delay(1000);
  }

  if (wifi_connected) {
    if (cam_init_ok) {
      while (!getTimeStamp()) {
        delay(100);
      }
      DBG(TIME_STAMP);
      delay(1000);
      String fileName = TIME_STAMP + ".jpg";
      takePicture(fileName);
    }
  }

  // Go to deep sleep
  DBG("Done");
  rtc_gpio_hold_en(GPIO_NUM_4);
  esp_sleep_enable_timer_wakeup(1 * 60e6);
  esp_deep_sleep_start();

}

void loop() {}
