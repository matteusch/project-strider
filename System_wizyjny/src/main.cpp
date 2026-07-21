#include "OV2640.hh"
#include "ImgManip.hh"
#include "driver/uart.h"

#pragma pack(push, 1)
typedef struct {
  uint8_t posX;
  uint8_t posY;
  uint32_t area;
} CameraObject;

typedef struct {
  uint8_t objNum;
  CameraObject objList[10];
} DataFrame;
#pragma pack(pop)

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

  Serial1.begin(115200, SERIAL_8N1, 14, 13); 
  Serial1.setTxBufferSize(256);
  while(!Serial1) delay(10);

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

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
  Manipulator.detectColors(0,180,150,255);

  Manipulator.dilation(structElem);
  Manipulator.erosion(structElem);
  Manipulator.erosion(structElem);
  Manipulator.dilation(structElem);

  uint8_t labels = Manipulator.label();
  Manipulator.countArea();
  Manipulator.findCenter();
  Manipulator.countRoundness();
  Manipulator.checkBottomEdge();

  std::vector<Blob> labels_info = Manipulator.getLabelsInfo();

  if (Serial2.availableForWrite() >= Camera.getWidth()*Camera.getHeight() + 30) 
  {
    sendFrame(
      Manipulator.getResult(), 
      Camera.getWidth() * Camera.getHeight(), 
      interval, 
      fps, 
      labels,
      labels_info
    );
  }

  if (Serial1.availableForWrite() >= sizeof(DataFrame)) 
  {
    sendInfo(labels, labels_info);
  }

  delay(50); 
}

uint32_t measureTime() {
  static uint32_t last_call_time = 0;
  uint32_t current_time = millis();
  uint32_t interval = 0;
  
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

  uint32_t test = 30000;

  for(Blob & b: labels_info)
  {
    Serial2.write((uint8_t*)&b.index, 1);
    if(b.bottomEdge) Serial2.write((uint8_t*)&test, 4);
    else Serial2.write((uint8_t*)&b.area, 4);
    Serial2.write((uint8_t*)&b.x, 4);
    Serial2.write((uint8_t*)&b.y, 4);
    Serial2.write((uint8_t*)&b.roundness, 4);
  }

  Serial2.write((uint8_t*)&len, 4);
  Serial2.write(data, len);
}


void sendInfo(uint8_t labels, std::vector<Blob> labels_info)
{
  static bool ledState = false;
  ledState = !ledState;
  digitalWrite(2, ledState);

  DataFrame m;

  m.objNum = labels;

  
  for(int i = 0; i < 10; i++)
  {
    m.objList[i].posX = 0;
    m.objList[i].posY = 0;
    m.objList[i].area = 0;
  }

  uint32_t test = 30000;
  
  int idx = 0;
  for(Blob b : labels_info)
  {
    m.objList[idx].posX = (uint8_t)b.x;
    m.objList[idx].posY = (uint8_t)b.y;
    if(b.bottomEdge) m.objList[idx].area = test;
    else m.objList[idx].area = (uint32_t)b.area;

    idx++;
    if(idx >= 10) break;
  }


  Serial1.write((uint8_t *)&m, sizeof(DataFrame));
}