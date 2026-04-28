#include "OV2640.hh"
#include "ImgManip.hh"
#include "driver/uart.h"

OV2640 Camera;
ImgManip Manipulator;

bool structElem[9] = {1,1,1,1,1,1,1,1,1};

uint32_t measureTime();
void sendFrame(uint8_t* data, size_t len, uint32_t interval, float fps, uint8_t labels);

void setup() {
  esp_log_level_set("*", ESP_LOG_NONE);

  Serial.begin(115200);
  Serial.flush();
  uart_set_pin(UART_NUM_0, 26, 27, -1, -1);

  Serial2.setTxBufferSize(20000);
  Serial2.begin(1000000, SERIAL_8N1, 3, 1);
  while(!Serial2) delay(10);

  Camera.init();
  Manipulator.init();
}

void loop() {
  uint32_t interval = measureTime();
  float fps = 1000.0 / (float)interval;

  uint8_t * Img = Camera.capture();

  Manipulator.setImg(Img, Camera.getWidth(), Camera.getHeight());
  Manipulator.separateChannels();
  Manipulator.detectColors(0,128,150,255);

  //Zamknięcie
  Manipulator.dilation(structElem);
  Manipulator.erosion(structElem);

  //Otwarcie
  Manipulator.erosion(structElem);
  Manipulator.dilation(structElem);

  uint8_t labels = Manipulator.label();

  if (Serial2.availableForWrite() >= Camera.getWidth()*Camera.getHeight() + 17) sendFrame(Manipulator.getResult(), Camera.getWidth() * Camera.getHeight(), interval, fps, labels);
}

uint32_t measureTime() {
    static uint32_t last_call_time = 0;
    uint32_t current_time = millis();
    uint32_t interval;
    
    if (last_call_time > 0) interval = current_time - last_call_time; 

    last_call_time = current_time;

    return interval;
}

void sendFrame(uint8_t* data, size_t len, uint32_t interval, float fps, uint8_t labels) {
    const uint8_t header[] = {0xDE, 0xAD, 0xBE, 0xEF};
    
    Serial2.write(header, 4);
    Serial2.write((uint8_t *)&interval, 4);
    Serial2.write((uint8_t *)&fps,4);
    Serial2.write((uint8_t *)&labels, 1);
    Serial2.write((uint8_t*)&len, 4);
    Serial2.write(data, len);
}
