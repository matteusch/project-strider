#pragma once

#include <Arduino.h>
#include <stdint.h>

//#pragma pack(push, 1)

typedef struct __attribute__((packed)) {
  uint8_t posX;
  uint8_t posY;
  uint32_t area;
} CameraObject;

typedef struct __attribute__((packed)) {
  uint8_t objNum;
  CameraObject objList[10];
} DataFrame;

//#pragma pack(pop)