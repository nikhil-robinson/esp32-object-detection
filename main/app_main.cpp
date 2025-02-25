#include "esp_log.h"
#include "imagenet_cls.hpp"
#include "bsp/esp-bsp.h"
#include <list>
#include "esp_camera.h"
#include "app_camera.hpp"

extern const uint8_t cat_jpg_start[] asm("_binary_b_jpg_start");
extern const uint8_t cat_jpg_end[] asm("_binary_b_jpg_end");
const char *TAG = "imagenet_cls";


void init_camera()
{
    static camera_config_t camera_config = {
        .pin_pwdn = -1,
        .pin_reset = -1,
        .pin_xclk = BSP_CAMERA_XCLK,
        .pin_sccb_sda = BSP_I2C_SDA,
        .pin_sccb_scl = BSP_I2C_SCL,
    
        .pin_d7 = BSP_CAMERA_D7,
        .pin_d6 = BSP_CAMERA_D6,
        .pin_d5 = BSP_CAMERA_D5,
        .pin_d4 = BSP_CAMERA_D4,
        .pin_d3 = BSP_CAMERA_D3,
        .pin_d2 = BSP_CAMERA_D2,
        .pin_d1 = BSP_CAMERA_D1,
        .pin_d0 = BSP_CAMERA_D0,
        .pin_vsync = BSP_CAMERA_VSYNC,
        .pin_href = BSP_CAMERA_HSYNC,
        .pin_pclk = BSP_CAMERA_PCLK,
    
        //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
    
        .pixel_format = PIXFORMAT_RGB565, //YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = FRAMESIZE_240X240,    //QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.
    
        .jpeg_quality = 12, //0-63, for OV series camera sensors, lower number means higher quality
        .fb_count = 1,       //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
        .fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    };

    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        vTaskDelay(portMAX_DELAY);
    }
}


void object_detectio_task(void *pvParameters)
{
    ImageNetCls *cls = new ImageNetCls();
    // bsp_i2c_init();
    init_camera();
    

    while (true)
    {
        dl::image::img_t img;

        camera_fb_t *frame = esp_camera_fb_get();
        img.data = frame->buf;
        img.width = frame->width;
        img.height = frame->height;
        img.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB565;

        auto &results = cls->run(img);
        for (const auto &res : results) {
            ESP_LOGI(TAG, "category: %s, score: %f\n", res.cat_name, res.score);
        }

        esp_camera_fb_return(frame);
    }
    
}

extern "C" void app_main(void)
{

    xTaskCreatePinnedToCore((TaskFunction_t)object_detectio_task, TAG, 3 * 1024, NULL, 5, NULL, 1);
}
