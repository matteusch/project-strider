#pragma once

#include <Arduino.h>

#include "esp_camera.h"
#include "OV2640_Pinout.hh"

class OV2640
{
    private:
        uint32_t _width;
        uint32_t _height;

    public:
        OV2640();
        ~OV2640();

        bool init();
        uint8_t * capture();

        uint32_t getWidth();
        uint32_t getHeight();
};