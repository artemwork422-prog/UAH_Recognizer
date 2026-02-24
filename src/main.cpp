#include <Arduino.h>
#include "CameraHandler.h"
#include "WebServerHandler.h"
#include "InferenceHandler.h"
#include "StreamHandler.h"

String global_result = "Ready";
volatile bool scan_request = false;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n========================================");
    Serial.println("    UAH Banknote Scanner v2.0");
    Serial.println("    Live Streaming + AI Recognition");
    Serial.println("========================================\n");

    // Ініціалізація камери
    if (!initCamera()) {
        Serial.println("[ERROR] Camera initialization failed!");
        while(1);
    }
    Serial.println("[OK] Camera initialized");

    // Ініціалізація grayscale буфера
    if (!ei_camera_init()) {
        Serial.println("[ERROR] Buffer allocation failed!");
        while(1);
    }
    Serial.println("[OK] Grayscale buffer allocated");

    // Ініціалізація класифікатора
    run_classifier_init();
    Serial.println("[OK] Classifier initialized");

    // Ініціалізація потокового передавання (мінімальне використання памяті)
    if (!StreamHandler::initStream()) {
        Serial.println("[WARNING] Stream handler init failed");
    }
    Serial.println("[OK] Stream handler initialized");

    // WiFi AP
    WiFi.softAP("UAH_Scanner", "12345678");
    Serial.print("[OK] WiFi AP started - IP: ");
    Serial.println(WiFi.softAPIP());

    // Веб-сервер
    startWebServer();
    Serial.println("[OK] WebServer started");
    
    StreamHandler::printMemoryStats();
    Serial.println("\n[READY] System ready for scanning!");
}

void loop() {
    if (scan_request) {
        camera_fb_t* fb = esp_camera_fb_get();
        if (fb) {
            Serial.println("\n[FRAME] Processing frame...");
            global_result = runInference(fb);
            esp_camera_fb_return(fb);
            
            Serial.print("[RESULT] ");
            Serial.println(global_result);
        } else {
            global_result = "Frame error";
            Serial.println("[ERROR] Failed to get camera frame");
        }
        
        StreamHandler::printMemoryStats();
        scan_request = false;
    }
    delay(10);
}
