#include "ImgManip.hh"

ImgManip::ImgManip()
{
    _Img = nullptr;
    _matY = nullptr;
    _matU = nullptr;
    _matV = nullptr;
    _result = nullptr;
    _buf = nullptr;
    _edges = nullptr;
    _distance = nullptr;

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
        _edges = (uint8_t *)ps_malloc(width * height);
        _distance = (uint8_t *)ps_malloc(width * height);
    }
    else if((_width != width) || (_height != height))
    {
        _matY = (uint8_t *)ps_realloc(_matY, width * height);
        _matU = (uint8_t *)ps_realloc(_matU, width * height);
        _matV = (uint8_t *)ps_realloc(_matV, width * height);
        _result = (uint8_t *)ps_realloc(_result, width * height);
        _buf = (uint8_t *)ps_realloc(_buf, width * height);
        _labels = (uint8_t *)ps_realloc(_labels, width * height);
        _edges = (uint8_t *)ps_realloc(_edges, width * height);
        _distance = (uint8_t *)ps_realloc(_distance, width * height);
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

void ImgManip::detectEdges()
{
    int8_t Sx[9] = {-1, 0, 1, -2, 0, 2, -1, 0 , 1};
    int8_t Sy[9] = {-1, -2, -1, 0, 0, 0, 1, 2 , 1};

    memset(_edges, 0, _width * _height);

    for(int y=1; y<_height-1; y++)
    {
        for(int x=1; x<_width-1; x++)
        {
            int Gx = 0, Gy = 0;
            int idx = 0;

            for(int a = -1; a <= 1; a++) for(int b = -1; b <=1; b++)
            {
                Gx += _matY[CoordinatestoIndex(x+a,y+b)] * Sx[idx];
                Gy += _matY[CoordinatestoIndex(x+a,y+b)] * Sy[idx];
                idx++;
            }

            int G = abs(Gx) + abs(Gy);
            if(G >= 150) _edges[CoordinatestoIndex(x,y)] = 255;
            else _edges[CoordinatestoIndex(x,y)] = 0;
        }
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

void ImgManip::filterEdges()
{
    for(int y=0; y<_height; y++)
    {
        for(int x=0; x<_width; x++)
        {
            if(!(_edges[CoordinatestoIndex(x,y)] && isAround(x,y,10))) _edges[CoordinatestoIndex(x,y)] = 0;
        }
    }
}

void ImgManip::distanceTransform()
{
    memset(_distance, 0, _width * _height);

    for(int y=1; y<_height; y++)
    {
        for(int x=1; x<_width; x++)
        {
            if(_result[CoordinatestoIndex(x,y)]) 
                _distance[(CoordinatestoIndex(x,y))] = 1 + min(_distance[CoordinatestoIndex(x-1,y)], _distance[CoordinatestoIndex(x,y-1)]);
        }
    }

    for(int y=_height-2; y>=0; y--)
    {
        for(int x=_width-2; x>=0; x--)
        {
            if(_result[CoordinatestoIndex(x,y)]) 
                _distance[(CoordinatestoIndex(x,y))] = (uint8_t)min((int)_distance[CoordinatestoIndex(x,y)], 1+ min(_distance[CoordinatestoIndex(x+1,y)], _distance[CoordinatestoIndex(x,y+1)]));
        }
    }

    uint8_t max_value = 0;
    for(int y=0; y<_height; y++)
    {
        for(int x=0; x<_width; x++)
        {
            uint8_t buf = _distance[CoordinatestoIndex(x,y)];
            if(buf > max_value) max_value = buf;
        }
    }

    if(max_value == 0) return;

    for(int y=0; y<_height; y++)
    {
        for(int x=0; x<_width; x++)
        {
            _distance[CoordinatestoIndex(x,y)] = (uint8_t)((float)_distance[CoordinatestoIndex(x,y)]*255 / (float)max_value);
        }
    }
}

void ImgManip::separateOverlaps()
{
    if(_labels_info.empty()) return;

    std::vector<Blob> blobBuf;

    int next_index = _labels_info.back().index + 1;

    for(Blob & b: _labels_info)
    {
        uint8_t max_value = 0;
        float x1 = b.x;
        float y1 = b.y;

        for(int y=0; y<_height; y++)
        {
            for(int x=0; x<_width; x++)
            {
                uint8_t buf = _distance[CoordinatestoIndex(x,y)];
                if(_labels[CoordinatestoIndex(x,y)] == b.index && buf > max_value)
                {
                    max_value = buf;
                    x1 = x;
                    y1 = y;
                }
            }
        }

        float dx = b.x - x1;
        float dy = b.y - y1;
        float distance_from_center = sqrt(dx*dx + dy*dy);
        float cube_width = sqrt((float)b.area);
        float thickness_ratio = (float)max_value / cube_width; 

        bool geometric_indicates_overlap = (b.roundness < 0.5f);
        bool should_split = false;

        if (distance_from_center > 0.15f * cube_width) 
        {
            should_split = true;
        }
        else if (thickness_ratio < 0.20f && geometric_indicates_overlap)
        {
            should_split = true;
        }

        if (thickness_ratio > 0.40f && b.roundness > 0.70f)
        {
            should_split = false;
        }

        if (!should_split) continue;

        float x2 = 2*b.x - x1;
        float y2 = 2*b.y - y1;
        uint32_t newArea = b.area/2;
        float newRoundness = b.roundness;

        b.x = x1;
        b.y = y1;
        b.area = newArea;
        b.roundness = newRoundness;

        Blob newBlob;
        newBlob.x = x2;
        newBlob.y = y2;
        newBlob.roundness = newRoundness;
        newBlob.area = newArea;
        newBlob.index = next_index++;

        blobBuf.push_back(newBlob);
    }

    if(!blobBuf.empty())
    {
        _labels_info.insert(_labels_info.end(), blobBuf.begin(), blobBuf.end());
        _labels_num = _labels_info.size();
    }
}

int ImgManip::CoordinatestoIndex(int x, int y)
{
    return y*_width + x;
}

bool ImgManip::isAround(int x, int y, int neighborhood)
{
    for(int i=-neighborhood; i<=neighborhood; i++)
    {
        for(int j=-neighborhood; j<=neighborhood; j++)
        {
            if(x+i < 0) break;
            if(x+i >= _width) break;
            if(y+j < 0) break;
            if(y+j >= _height) break;
            if(_result[CoordinatestoIndex(x+i,y+j)]) return true;
        }
    }

    return false;
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

uint8_t *ImgManip::getEdges()
{
    return _edges;
}

uint8_t *ImgManip::getDistance()
{
    return _distance;
}

std::vector<Blob> ImgManip::getLabelsInfo()
{
    return _labels_info;
}

int ImgManip::getLabelsNum()
{
    return _labels_num;
}
