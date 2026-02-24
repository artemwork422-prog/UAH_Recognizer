#include <Arduino.h>
#include "CameraHandler.h"
#include "WebServerHandler.h"
#include "InferenceHandler.h"

String global_result = "Ready";
volatile bool scan_request = false;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== UAH Banknote Scanner ===");
    
    // Ініціалізуємо камеру
    if (!initCamera()) {
        Serial.println("ERROR: Camera init failed");
        while(1);
    }
    Serial.println("Camera initialized");
    
    // Ініціалізуємо grayscale буфер
    if (!ei_camera_init()) {
        Serial.println("ERROR: Buffer allocation failed");
        while(1);
    }
    Serial.println("Grayscale buffer allocated");
    
    // Ініціалізуємо класифікатор
    run_classifier_init();
    
    // WiFi AP
    WiFi.softAP("UAH_Scanner", "12345678");
    Serial.print("WiFi IP: ");
    Serial.println(WiFi.softAPIP());
    
    // Вебсервер
    startWebServer();
    Serial.println("WebServer started");
    Serial.println("Ready for scanning!");
}

void loop() {
    if (scan_request) {
        camera_fb_t* fb = esp_camera_fb_get();
        if (fb) {
            Serial.println("Processing frame...");
            global_result = runInference(fb);
            esp_camera_fb_return(fb);
            Serial.print("Result: ");
            Serial.println(global_result);
        } else {
            global_result = "Frame error";
            Serial.println("ERROR: Failed to get frame");
        }
        scan_request = false;
    }
    delay(10);
}