#pragma once

#include <Arduino.h>

#include "esp_camera.h"
#include "OV2640_Pinout.hh"

class OV2640
{
    private:
        int _width;
        int _height;

    public:
        OV2640();
        ~OV2640();

        bool init();
        uint8_t * capture();

        int getWidth();
        int getHeight();
};