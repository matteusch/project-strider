#pragma once

#include <Arduino.h>

class ImgManip
{
    private:
        uint8_t * _Img;
        uint8_t * _matY;
        uint8_t * _matU;
        uint8_t * _matV;
        uint8_t * _result;
        uint8_t * _buf;
        uint8_t * _labels;

        int _width;
        int _height;

        int CoordinatestoIndex(int x, int y);

    public:
        ImgManip();
        ~ImgManip();

        void init();
        void setImg(uint8_t * Img, int width, int height);

        uint8_t * getMatY();
        uint8_t * getMatU();
        uint8_t * getMatV();
        uint8_t * getResult();

        void separateChannels();
        void detectColors(uint8_t minU, uint8_t maxU, uint8_t minV, uint8_t maxV);
        void dilation(bool * StructElem);
        void erosion(bool * StructElem);
        int label();
};