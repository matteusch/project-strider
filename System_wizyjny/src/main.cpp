#include "OV2640.hh"
#include "ImgManip.hh"
#include "driver/uart.h"

// Definicja struktur z jawnym upakowaniem bajtów (packed)
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

// Inicjalizacja obiektów kamery i przetwarzania obrazu
OV2640 Camera;
ImgManip Manipulator;

// Element strukturalny do operacji morfologicznych
bool structElem[9] = {1,1,1,1,1,1,1,1,1};

// Prototypy funkcji
uint32_t measureTime();
void sendFrame(uint8_t* data, size_t len, uint32_t interval, float fps, uint8_t labels, std::vector<Blob> labels_info);
void sendInfo(uint8_t labels, std::vector<Blob> labels_info);

void setup() {
  // Wyłączenie logów systemowych ESP, żeby nie śmieciły na portach
  esp_log_level_set("*", ESP_LOG_NONE);

  // Serial0 (USB do PC) - odpinamy od domyślnych pinów TX/RX, aby zwolnić je dla Serial2
  Serial.begin(115200);
  Serial.flush();
  uart_set_pin(UART_NUM_0, -1, -1, -1, -1);

  // Serial2 - wysyłanie pełnego obrazu do PC (Baudrate 1M, RX=pin 3, TX=pin 1)
  Serial2.setTxBufferSize(26000);
  Serial2.begin(1000000, SERIAL_8N1, 3, 1);
  while(!Serial2) delay(10);

  // Serial1 - komunikacja z STM32 (Baudrate 115200, RX=pin 33, TX=pin 32)
  // WAŻNE: Połącz pin 32 (TX) z pinem RX w STM32 (PA10)
  Serial1.begin(115200, SERIAL_8N1, 33, 32); 
  while(!Serial1) delay(10);

  // Inicjalizacja peryferiów kamery i algorytmów
  Camera.init();
  Manipulator.init();
}

void loop() {
  uint32_t interval = measureTime();
  float fps = 1000.0 / (float)interval;

  // Przechwycenie obrazu z matrycy
  uint8_t * Img = Camera.capture();

  Manipulator.detectEdges();
  Manipulator.setImg(Img, Camera.getWidth(), Camera.getHeight());
  Manipulator.separateChannels();
  Manipulator.detectColors(0,180,150,255);

  // Operacje morfologiczne: Zamknięcie -> Otwarcie -> Zamknięcie -> Otwarcie
  Manipulator.dilation(structElem);
  Manipulator.erosion(structElem);
  Manipulator.erosion(structElem);
  Manipulator.dilation(structElem);
  // Manipulator.dilation(structElem);
  // Manipulator.erosion(structElem);
  // Manipulator.erosion(structElem);
  // Manipulator.dilation(structElem);

  // Segmentacja i analiza obiektów (Blobów)
  uint8_t labels = Manipulator.label();
  Manipulator.countArea();
  Manipulator.findCenter();
  Manipulator.countRoundness();
  Manipulator.checkBottomEdge();
  // Manipulator.filterEdges();

  // Manipulator.distanceTransform();
  // Manipulator.separateOverlaps();

  // labels = Manipulator.getLabelsNum();

  std::vector<Blob> labels_info = Manipulator.getLabelsInfo();

  // 1. Wysyłanie pełnej klatki obrazu do PC przez Serial2 (jeśli bufor jest wolny)
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

  // 2. Wysyłanie spakowanych danych o obiektach do STM32 przez Serial1
  if (Serial1.availableForWrite() >= sizeof(DataFrame) /*millis()%1000 == 0*/) 
  {
    sendInfo(labels, labels_info);
  }

  // POPRAWKA: Hamulec bezpieczeństwa. Daje linii UART 50ms ciszy. 
  // Bez tego STM32 nigdy nie wykryje przerwania IDLE i ignoruje ramki.
  delay(50); 
}

// Funkcja mierząca czas wykonania pętli
uint32_t measureTime() {
  static uint32_t last_call_time = 0;
  uint32_t current_time = millis();
  uint32_t interval = 0;
  
  if (last_call_time > 0) interval = current_time - last_call_time; 
  last_call_time = current_time;

  return interval;
}

// Funkcja wysyłająca obraz (PC)
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

    // float val = Manipulator.getMatU()[(int)b.y * 176 + (int)b.x];
    // Serial2.write((uint8_t*)&val, 4);
  }

  Serial2.write((uint8_t*)&len, 4);
  Serial2.write(data, len);
}


void sendInfo(uint8_t labels, std::vector<Blob> labels_info)
{
  DataFrame m;

  m.objNum = labels;

  
  for(int i = 0; i < 10; i++)
  {
    m.objList[i].posX = 0;
    m.objList[i].posY = 0;
    m.objList[i].area = 0;
  }

  
  int idx = 0;
  for(Blob b : labels_info)
  {
    m.objList[idx].posX = (uint8_t)b.x;
    m.objList[idx].posY = (uint8_t)b.y;
    m.objList[idx].area = (uint32_t)b.area;

    idx++;
    if(idx >= 10) break;
  }


  Serial1.write((uint8_t *)&m, sizeof(DataFrame));
}