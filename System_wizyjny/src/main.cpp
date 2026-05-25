#include "OV2640.hh"
#include "ImgManip.hh"
#include "driver/uart.h"
#include "MessageStruct.h"

OV2640 Camera;
ImgManip Manipulator;

bool structElem[9] = {1,1,1,1,1,1,1,1,1};

uint32_t measureTime();
void sendFrame(uint8_t* data, size_t len, uint32_t interval, float fps, uint8_t labels, std::vector<Blob> labels_info);
void sendInfo(uint8_t labels, std::vector<Blob> labels_info);

void setup() {
  esp_log_level_set("*", ESP_LOG_NONE);

  Serial.begin(115200);
  Serial.flush();
  uart_set_pin(UART_NUM_0, -1, -1, -1, -1);

  Serial2.setTxBufferSize(26000);
  Serial2.begin(1000000, SERIAL_8N1, 3, 1);
  while(!Serial2) delay(10);

  Serial1.begin(115200, SERIAL_8N1, 33, 32); //Rx, Tx
  while(!Serial1) delay(10);

  Camera.init();
  Manipulator.init();
}

void loop() {
  uint32_t interval = measureTime();
  float fps = 1000.0 / (float)interval;

  uint8_t * Img = Camera.capture();

  Manipulator.detectEdges();

  Manipulator.setImg(Img, Camera.getWidth(), Camera.getHeight());
  Manipulator.separateChannels();
  Manipulator.detectColors(0,255,150,255);

  //Zamknięcie
  Manipulator.dilation(structElem);
  Manipulator.erosion(structElem);

  //Otwarcie
  Manipulator.erosion(structElem);
  Manipulator.dilation(structElem);

  //Zamknięcie
  Manipulator.dilation(structElem);
  Manipulator.erosion(structElem);

  //Otwarcie
  Manipulator.erosion(structElem);
  Manipulator.dilation(structElem);

  uint8_t labels = Manipulator.label();

  Manipulator.countArea();
  Manipulator.findCenter();
  Manipulator.countRoundness();

  Manipulator.filterEdges();

  std::vector<Blob> labels_info = Manipulator.getLabelsInfo();

  if (Serial2.availableForWrite() >= Camera.getWidth()*Camera.getHeight() + 30) 
  {
    sendFrame(
      Manipulator.getEdges(), 
      Camera.getWidth() * Camera.getHeight(), 
      interval, 
      fps, 
      labels,
      labels_info
    );
  }

  if (Serial1.availableForWrite() >= sizeof(msg)) sendInfo(labels, labels_info);
    
}

uint32_t measureTime() {
  static uint32_t last_call_time = 0;
  uint32_t current_time = millis();
  uint32_t interval;
  
  if (last_call_time > 0) interval = current_time - last_call_time; 

  last_call_time = current_time;

  return interval;
}

void sendFrame(uint8_t* data, size_t len, uint32_t interval, float fps, uint8_t labels, std::vector<Blob> labels_info) {
  const uint8_t header[] = {0xDE, 0xAD, 0xBE, 0xEF};
  
  Serial2.write(header, 4);
  Serial2.write((uint8_t *)&interval, 4);
  Serial2.write((uint8_t *)&fps,4);
  Serial2.write((uint8_t *)&labels, 1);

  for(Blob b: labels_info)
  {
    Serial2.write((uint8_t*)&b.index, 1);
    Serial2.write((uint8_t*)&b.area, 4);
    Serial2.write((uint8_t*)&b.x, 4);
    Serial2.write((uint8_t*)&b.y, 4);
    Serial2.write((uint8_t*)&b.roundness, 4);
  }

  Serial2.write((uint8_t*)&len, 4);
  Serial2.write(data, len);
}

void sendInfo(uint8_t labels, std::vector<Blob> labels_info)
{
  msg m;

  m.count = labels;

  for(int i=0; i<10; i++)
  {
    m.l[i].xPos = 0;
    m.l[i].yPos = 0;
    m.l[i].Area = 0;
  }

  int idx = 0;

  for(Blob b: labels_info)
  {
    m.l[idx].xPos = (uint8_t)b.x;
    m.l[idx].yPos = (uint8_t)b.y;
    m.l[idx].Area = b.area;

    idx++;
    if(idx>=10) break;
  }

  Serial1.write((uint8_t *)&m, sizeof(msg));
}