#pragma once

#include <Arduino.h>
#include <stdint.h>

#pragma pack(push, 1)

typedef struct
{
  uint8_t xPos;
  uint8_t yPos;

  uint32_t Area;
} label;

typedef struct
{
  uint8_t count;
  label l[10];
}msg;

#pragma pack(pop)