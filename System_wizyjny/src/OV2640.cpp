#include "OV2640.hh"

OV2640::OV2640()
{
    _width = 0;
    _height = 0;
}

OV2640::~OV2640() {}

bool OV2640::init()
{
    camera_config_t config;

    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = CAM_PIN_D0;
    config.pin_d1 = CAM_PIN_D1;
    config.pin_d2 = CAM_PIN_D2;
    config.pin_d3 = CAM_PIN_D3;
    config.pin_d4 = CAM_PIN_D4;
    config.pin_d5 = CAM_PIN_D5;
    config.pin_d6 = CAM_PIN_D6;
    config.pin_d7 = CAM_PIN_D7;
    config.pin_xclk = CAM_PIN_XVCLK;
    config.pin_pclk = CAM_PIN_PCLK;
    config.pin_vsync = CAM_PIN_VSYNC;
    config.pin_href = CAM_PIN_HREF;
    config.pin_sccb_sda = CAM_PIN_SIOD;
    config.pin_sccb_scl = CAM_PIN_SIOC;
    config.pin_pwdn = CAM_PIN_PWDN;
    config.pin_reset = CAM_PIN_RESET;
    config.xclk_freq_hz = CAM_XVCLK_FREQ;
    config.pixel_format = PIXFORMAT_YUV422;
    config.frame_size = FRAMESIZE_QQVGA; 
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;

    esp_err_t e = esp_camera_init(&config);
    if(e != ESP_OK) return false;

    return true;
}

uint8_t * OV2640::capture()
{
    camera_fb_t * fb = esp_camera_fb_get();
    if(!fb) return nullptr;

    _width = fb->width;
    _height = fb->height;

    uint8_t * Img = fb->buf;
    
    esp_camera_fb_return(fb);
    vTaskDelay(pdMS_TO_TICKS(1));

    return Img;
}

int OV2640::getWidth()
{
    return _width;
}

int OV2640::getHeight()
{
    return _height;
}
