#pragma once

#include <Arduino.h>
#include <vector>

struct Blob;

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

        int _labels_num;
        std::vector<Blob> _labels_info;

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
        std::vector<Blob> getLabelsInfo();

        void separateChannels();
        void detectColors(uint8_t minU, uint8_t maxU, uint8_t minV, uint8_t maxV);
        void dilation(bool * StructElem);
        void erosion(bool * StructElem);
        int label();
        void countArea();
        void findCenter();
        void countRoundness();
};

struct Blob
{
    uint8_t index;
    uint32_t area;
    float x;
    float y;
    float roundness;
};