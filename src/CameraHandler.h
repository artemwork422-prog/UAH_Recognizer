#ifndef _CAMERA_HANDLER_H_
#define _CAMERA_HANDLER_H_
#include "esp_camera.h"
#include <Arduino.h>

// ESP32-CAM (WROVER) GPIO Pin Configuration
// PIN connections verified for OV2640 camera module
#define PWDN_GPIO_NUM 32      // Power Down pin
#define RESET_GPIO_NUM -1     // Reset pin (disabled)
#define XCLK_GPIO_NUM 0       // Camera clock
#define SIOD_GPIO_NUM 26      // I2C SDA (camera control)
#define SIOC_GPIO_NUM 27      // I2C SCL (camera control)

// Data pins (D0-D7) for parallel data transmission
#define Y9_GPIO_NUM 35        // D9
#define Y8_GPIO_NUM 34        // D8
#define Y7_GPIO_NUM 39        // D7 (input only)
#define Y6_GPIO_NUM 36        // D6 (input only)
#define Y5_GPIO_NUM 21        // D5
#define Y4_GPIO_NUM 19        // D4
#define Y3_GPIO_NUM 18        // D3
#define Y2_GPIO_NUM 5         // D2

// Control pins
#define VSYNC_GPIO_NUM 25     // Vertical sync
#define HREF_GPIO_NUM 23      // Horizontal sync
#define PCLK_GPIO_NUM 22      // Pixel clock

bool initCamera() {
    camera_config_t config;
    
    // Clock source configuration
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    
    // Parallel data pins (D0-D7)
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    
    // Clock and sync pins
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    
    // I2C control pins
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    
    // Power and reset pins
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    
    // Clock frequency: 20MHz for OV2640
    config.xclk_freq_hz = 20000000;
    
    // Frame format - GRAYSCALE for AI inference
    config.pixel_format = PIXFORMAT_GRAYSCALE;
    
    // Frame size - 96x96 for Edge Impulse model
    config.frame_size = FRAMESIZE_96X96;
    
    // Frame buffer settings
    config.fb_count = 1;                    // Single buffer
    config.fb_location = CAMERA_FB_IN_PSRAM; // Use PSRAM if available
    
    // Optional: Gain mode settings
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    
    Serial.println("[CAMERA] Initializing camera with GPIO configuration...");
    Serial.printf("  XCLK: GPIO%d, PCLK: GPIO%d\n", XCLK_GPIO_NUM, PCLK_GPIO_NUM);
    Serial.printf("  VSYNC: GPIO%d, HREF: GPIO%d\n", VSYNC_GPIO_NUM, HREF_GPIO_NUM);
    Serial.printf("  SCCB SDA: GPIO%d, SCCB SCL: GPIO%d\n", SIOD_GPIO_NUM, SIOC_GPIO_NUM);
    Serial.printf("  Data pins: Y2-Y9 on GPIO 5,18,19,21,36,39,34,35\n");
    
    esp_err_t err = esp_camera_init(&config);
    
    if (err != ESP_OK) {
        Serial.printf("[CAMERA] ERROR: Initialization failed with code 0x%x\n", err);
        Serial.println("[CAMERA] Possible causes:");
        Serial.println("  1. GPIO pins not connected correctly");
        Serial.println("  2. Camera module is defective");
        Serial.println("  3. I2C communication failure (SCCB pins)");
        Serial.println("  4. Incorrect camera orientation");
        return false;
    }
    
    // Get sensor info after successful init
    sensor_t *s = esp_camera_sensor_get();
    if (s != NULL) {
        Serial.printf("[CAMERA] Sensor model: PID=0x%x VER=0x%x\n", 
                     s->id.pid, s->id.ver);
        Serial.println("[CAMERA] Sensor initialized successfully!");
        
        // Optional sensor settings
        s->set_brightness(s, 0);        // Brightness
        s->set_contrast(s, 0);          // Contrast
        s->set_saturation(s, 0);        // Saturation
        
        return true;
    } else {
        Serial.println("[CAMERA] ERROR: Could not get sensor after init");
        return false;
    }
}

#endif