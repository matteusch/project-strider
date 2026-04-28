#include "ImgManip.hh"

ImgManip::ImgManip()
{
    _Img = nullptr;
    _matY = nullptr;
    _matU = nullptr;
    _matV = nullptr;
    _result = nullptr;
    _buf = nullptr;

    _width = 0;
    _height = 0;

    _labels_num = 0;
}

ImgManip::~ImgManip()
{

}

void ImgManip::init()
{

}

void ImgManip::setImg(uint8_t *Img, int width, int height)
{
    _Img = Img;

    if(_matY == nullptr)
    {
        _matY = (uint8_t *)ps_malloc(width * height);
        _matU = (uint8_t *)ps_malloc(width * height);
        _matV = (uint8_t *)ps_malloc(width * height);
        _result = (uint8_t *)ps_malloc(width * height);
        _buf = (uint8_t *)ps_malloc(width * height);
        _labels = (uint8_t *)ps_malloc(width * height);
    }
    else if((_width != width) || (_height != height))
    {
        _matY = (uint8_t *)ps_realloc(_matY, width * height);
        _matU = (uint8_t *)ps_realloc(_matU, width * height);
        _matV = (uint8_t *)ps_realloc(_matV, width * height);
        _result = (uint8_t *)ps_realloc(_matV, width * height);
        _buf = (uint8_t *)ps_realloc(_matV, width * height);
        _labels = (uint8_t *)ps_realloc(_matV, width * height);
    }

    _width = width;
    _height = height;

    _labels_info.clear();
}

void ImgManip::separateChannels()
{
    for(int i=0; i<_width*_height; i+=2)
    {
        _matY[i] = _Img[2*i];
        _matY[i+1] = _Img[2*i+2];

        _matU[i] = _Img[2*i+1];
        _matV[i] = _Img[2*i+3];

        _matU[i+1] = _Img[2*i+1];
        _matV[i+1] = _Img[2*i+3];

        _result[i] = _matY[i];
        _result[i+1] = _matY[i+1];
    }
}

void ImgManip::detectColors(uint8_t minU, uint8_t maxU, uint8_t minV, uint8_t maxV)
{
    for(int i=0; i<_width*_height; i++)
    {
        if( _matU[i] >= minU && _matU[i] <= maxU && _matV[i] >= minV && _matV[i] <= maxV) _result[i] = 255;
        else _result[i] = 0;
    }
}

void ImgManip::dilation(bool *StructElem)
{
    memset(_buf, 0, _width * _height);

    for(int y=1; y<_height-1; y++)
    {
        for(int x=1; x<_width-1; x++)
        {
            int i = CoordinatestoIndex(x,y);

            if(_result[i] == 255)
            {
                _buf[i] = 255;
                if(StructElem[0])  _buf[CoordinatestoIndex(x-1,y-1)] = 255;
                if(StructElem[1])  _buf[CoordinatestoIndex(x,y-1)]   = 255;
                if(StructElem[2])  _buf[CoordinatestoIndex(x+1,y-1)] = 255;
                if(StructElem[3])  _buf[CoordinatestoIndex(x-1,y)]   = 255;
                if(StructElem[5])  _buf[CoordinatestoIndex(x+1,y)]   = 255;
                if(StructElem[6])  _buf[CoordinatestoIndex(x-1,y+1)] = 255;
                if(StructElem[7])  _buf[CoordinatestoIndex(x,y+1)]   = 255;
                if(StructElem[8])  _buf[CoordinatestoIndex(x+1,y+1)] = 255;
            }
        }
    }

    memcpy(_result, _buf, _width * _height);
}

void ImgManip::erosion(bool *StructElem)
{
    memset(_buf, 0, _width * _height);
    bool t = true;

    for(int y=1; y<_height-1; y++)
    {
        for(int x=1; x<_width-1; x++)
        {
            int i = CoordinatestoIndex(x,y);
            _buf[i] = 0;
            t=true;

            if(StructElem[0] && _result[CoordinatestoIndex(x-1,y-1)] == 0) t=false;
            if(StructElem[1] && _result[CoordinatestoIndex(x,y-1)] == 0)   t=false;
            if(StructElem[2] && _result[CoordinatestoIndex(x+1,y-1)] == 0) t=false;
            if(StructElem[3] && _result[CoordinatestoIndex(x-1,y)] == 0)   t=false;
            if(StructElem[4] && _result[CoordinatestoIndex(x,y)] == 0)     t=false;
            if(StructElem[5] && _result[CoordinatestoIndex(x+1,y)] == 0)   t=false;
            if(StructElem[6] && _result[CoordinatestoIndex(x-1,y+1)] == 0) t=false;
            if(StructElem[7] && _result[CoordinatestoIndex(x,y+1)] == 0)   t=false;
            if(StructElem[8] && _result[CoordinatestoIndex(x+1,y+1)] == 0) t=false;

            if(t==true) _buf[i]=255;
        }
    }

    memcpy(_result, _buf, _width * _height);
}

int ImgManip::label()
{
    uint8_t ConnectionArray[256];
    for(int i=0; i<256; i++) ConnectionArray[i] = i;
    
    memset(_labels, 0, _width * _height);

    int count=0;

    for(int y=1; y<_height-1; y++)
    {
        for(int x=1; x<_width-1; x++)
        {
            int i = CoordinatestoIndex(x,y);
            if(_result[i]==255)
            {
                int neighbors[4] = {
                    _labels[CoordinatestoIndex(x-1, y-1)],
                    _labels[CoordinatestoIndex(x,   y-1)],
                    _labels[CoordinatestoIndex(x+1, y-1)],
                    _labels[CoordinatestoIndex(x-1, y)]
                };

                int min = 0;
                for(int n : neighbors) if(n > 0) if(min == 0 || n < min) min = n;

                if(min == 0)
                {
                    if(count == 255) return -1;
                    count++;
                    _labels[i] = count;
                }
                else
                {
                    _labels[i] = min;
                    for(int j=0; j<4; j++) if(neighbors[j] > 0 && neighbors[j] != min)
                    {
                        int rootNeighbor = neighbors[j];
                        while(ConnectionArray[rootNeighbor] != rootNeighbor) rootNeighbor = ConnectionArray[rootNeighbor];

                        int rootMin = min;
                        while(ConnectionArray[rootMin] != rootMin) rootMin = ConnectionArray[rootMin];

                        if(rootNeighbor != rootMin) 
                        {
                            if(rootNeighbor < rootMin) ConnectionArray[rootMin] = rootNeighbor;
                            else ConnectionArray[rootNeighbor] = rootMin;
                        }
                    }
                }
            }
        }
    }

    int nextNewLabel = 1;

    for (int i = 1; i <= count; i++) 
    {
        if (ConnectionArray[i] == i) 
        {
            ConnectionArray[i] = nextNewLabel;
            nextNewLabel++;
        } 
        else ConnectionArray[i] = ConnectionArray[ConnectionArray[i]];
    }

    for(int i = 0; i < _width * _height; i++) if(_labels[i] > 0) _labels[i] = ConnectionArray[_labels[i]];

    _labels_num = nextNewLabel - 1;

    for(int i=1; i<=_labels_num; i++) _labels_info.push_back({(uint8_t)i,0,0,0,0});

    return _labels_num;
}

void ImgManip::countArea()
{
    for(Blob & b: _labels_info)
    {
        for(int i=0; i<_width*_height; i++) 
        {
            if(_labels[i] == b.index) b.area++;
        }
    }
}

void ImgManip::findCenter()
{
    for(Blob & b: _labels_info)
    {
        int sumX = 0, sumY = 0;

        for(int y=0; y<_height; y++)
        {
            for(int x=0; x<_width; x++)
            {
                if(_labels[CoordinatestoIndex(x,y)] == b.index)
                {
                    sumX += x;
                    sumY += y;
                }
            }
        }
        
        b.x = sumX / (float)b.area;
        b.y = sumY / (float)b.area;
    }
}

void ImgManip::countRoundness()
{
    for(Blob & b: _labels_info)
    {
        uint32_t Rmax2 = 0;

        for(int y=0; y<_height; y++)
        {
            for(int x=0; x<_width; x++)
            {
                if(_labels[CoordinatestoIndex(x,y)] == b.index)
                {
                    int buf1 = x - b.x;
                    int buf2 = y - b.y;
                    float R2 = buf1*buf1 + buf2*buf2;

                    if(R2 > Rmax2) Rmax2 = R2;
                }
            }
        }
        
        b.roundness = (float)b.area/(3.14* (float)Rmax2);
    }
}

int ImgManip::CoordinatestoIndex(int x, int y)
{
    return y*_width + x;
}

uint8_t * ImgManip::getMatY()
{
    return _matY;
}

uint8_t * ImgManip::getMatU()
{
    return _matU;
}

uint8_t * ImgManip::getMatV()
{
    return _matV;
}

uint8_t * ImgManip::getResult()
{
    return _result;
}

std::vector<Blob> ImgManip::getLabelsInfo()
{
    return _labels_info;
}
