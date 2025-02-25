#include "app_camera.hpp"
#include "esp_log.h"
#include "esp_system.h"
#include "bsp/esp-bsp.h"

const static char *TAG = "App/Camera";

/**
 * @brief Constructor for the AppCamera class.
 * 
 * Initializes the camera module with the specified pixel format, frame size, 
 * and framebuffer count.
 * Configures the camera pins, initializes the camera, and applies sensor settings.
 * 
 * @param pixel_format  Pixel format to be used by the camera 
 * @param frame_size    Frame size (resolution) for the camera 
 * @param fb_count      Number of frame buffers to allocate for the camera
 * @param queue_o       Queue handle to which captured frames will be sent
 */
AppCamera::AppCamera(const pixformat_t pixel_fromat,
                     const framesize_t frame_size,
                     const uint8_t fb_count)
{
    // ESP_LOGI(TAG, "Camera module is %s", CAMERA_MODULE_NAME);

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_xclk = BSP_CAMERA_XCLK;
    config.pin_pclk = BSP_CAMERA_PCLK;
    config.pin_vsync = BSP_CAMERA_VSYNC;
    config.pin_href = BSP_CAMERA_HSYNC;
    config.pin_d0 = BSP_CAMERA_D0;
    config.pin_d1 = BSP_CAMERA_D1;
    config.pin_d2 = BSP_CAMERA_D2;
    config.pin_d3 = BSP_CAMERA_D3;
    config.pin_d4 = BSP_CAMERA_D4;
    config.pin_d5 = BSP_CAMERA_D5;
    config.pin_d6 = BSP_CAMERA_D6;
    config.pin_d7 = BSP_CAMERA_D7;
    config.pin_sccb_sda = -1;
    config.pin_sccb_scl = BSP_I2C_SCL;
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    config.xclk_freq_hz = XCLK_FREQ_HZ;
    config.pixel_format = pixel_fromat;
    config.frame_size = frame_size;
    config.jpeg_quality = 12;
    config.fb_count = fb_count;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.sccb_i2c_port = BSP_I2C_NUM;
    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 0); // flip it back
    s->set_hmirror(s,1);
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID)
    {
        s->set_brightness(s, 1);  // up the blightness just a bit
        s->set_saturation(s, -2); // lower the saturation
    }
    s->set_sharpness(s, 2);
    s->set_awb_gain(s, 2);

}

/**
 * @brief Task function to capture and queue camera frames.
 * 
 * Continuously captures frames from the camera and sends them to a queue for processing.
 * 
 * @param self Pointer to the AppCamera instance
 */
static void task(AppCamera *self)
{
#if 0
    ESP_LOGI(TAG, "Start");
    while (true)
    {
        if (self->queue_o == nullptr)
        {
            ESP_LOGE(TAG,"QUEUE FAIL");
            vTaskDelay(portMAX_DELAY);
        }
        // xSemaphoreTake(self->cam_mtx, portMAX_DELAY);
        camera_fb_t *frame = esp_camera_fb_get();
        // xSemaphoreGive(self->cam_mtx);
        if (frame)
            xQueueSend(self->queue_o, &frame, portMAX_DELAY);

        vTaskDelay(1);
    }
    ESP_LOGE(TAG, "Stop");
#endif
}

/**
 * @brief Starts the camera capture task.
 * 
 * Creates a task to run the `task` function, which continuously 
 * captures frames from the camera.
 */
void AppCamera::run()
{
#if 0
    xTaskCreatePinnedToCore((TaskFunction_t)task, TAG, 3 * 1024, this, 5, &CAM_Handle, 0);
#endif
}

/**
 * @brief Destructor for the AppCamera class.
 * 
 * Cleans up resources by deleting the task and deinitializing the camera.
 */
AppCamera::~AppCamera()
{    
    esp_camera_deinit();
}