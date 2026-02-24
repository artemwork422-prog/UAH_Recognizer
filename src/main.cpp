#include <Arduino.h>
#include "CameraHandler.h"
#include "WebServerHandler.h"
#include "InferenceHandler.h"
#include "StreamHandler.h"

String global_result = "Ready";
volatile bool scan_request = false;
volatile bool inference_active = false;  // Флаг для паузи стрімінгу під час AI
int error_count = 0;
const int MAX_ERRORS = 10;

void printSystemInfo() {
    Serial.println("\n========================================");
    Serial.println("    UAH Banknote Scanner v2.0");
    Serial.println("    Live Streaming + AI Recognition");
    Serial.println("========================================");
    Serial.printf("ESP32 Chip Model: %d\n", ESP.getChipModel());
    Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / 1024 / 1024);
    Serial.printf("Free Heap: %u bytes\n", esp_get_free_heap_size());
    Serial.printf("PSRAM Size: %u bytes\n\n", ESP.getPsramSize());
}

void setup() {
    Serial.begin(115200);
    delay(1500);
    
    printSystemInfo();
    
    // Ініціалізація камери з перевіркою
    Serial.println("[SETUP] Initializing camera...");
    int camera_attempts = 0;
    while (!initCamera() && camera_attempts < 3) {
        camera_attempts++;
        Serial.printf("[SETUP] Camera init attempt %d/3 failed, retrying...\n", camera_attempts);
        delay(500);
    }
    
    if (camera_attempts >= 3) {
        Serial.println("[FATAL] Camera initialization failed after 3 attempts!");
        Serial.println("[DIAGNOSTIC] Possible causes:");
        Serial.println("  - Camera not connected or defective");
        Serial.println("  - GPIO pins incorrect or damaged");
        Serial.println("  - I2C (SCCB) communication failure");
        Serial.println("  - Camera firmware corrupted");
        while(1) {
            delay(1000);
            Serial.println("[SYSTEM] Waiting for reset...");
        }
    }
    Serial.println("[OK] ✓ Camera initialized");

    // Ініціалізація grayscale буфера
    Serial.println("[SETUP] Allocating grayscale buffer...");
    if (!ei_camera_init()) {
        Serial.println("[FATAL] Failed to allocate grayscale buffer!");
        Serial.printf("[DIAGNOSTIC] Available heap: %u bytes\n", esp_get_free_heap_size());
        while(1) delay(1000);
    }
    Serial.println("[OK] ✓ Grayscale buffer allocated");

    // Ініціалізація класифікатора
    Serial.println("[SETUP] Initializing classifier...");
    run_classifier_init();
    Serial.println("[OK] ✓ Classifier initialized");

    // Ініціалізація потокового передавання
    Serial.println("[SETUP] Initializing stream handler...");
    if (!StreamHandler::initStream()) {
        Serial.println("[WARNING] Stream handler initialization had issues");
    }
    Serial.println("[OK] ✓ Stream handler initialized");

    // WiFi AP
    Serial.println("[SETUP] Starting WiFi AP...");
    if (!WiFi.softAP("UAH_Scanner", "12345678")) {
        Serial.println("[ERROR] Failed to start WiFi AP!");
    } else {
        Serial.print("[OK] ✓ WiFi AP started - IP: ");
        Serial.println(WiFi.softAPIP());
    }

    // Веб-сервер
    Serial.println("[SETUP] Starting web server...");
    startWebServer();
    Serial.println("[OK] ✓ WebServer started");
    
    StreamHandler::printMemoryStats();
    Serial.println("\n[READY] ✓ System ready for scanning!");
    Serial.println("[INFO] Connect to WiFi: 'UAH_Scanner' (no password)");
    Serial.println("[INFO] Open browser: http://192.168.4.1\n");
}

void loop() {
    if (scan_request) {
        scan_request = false;
        
        camera_fb_t* fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("[ERROR] Failed to get camera frame");
            error_count++;
            global_result = "Frame error";
        } else {
            Serial.println("\n[FRAME] =====================================");
            Serial.println("[INFERENCE] Starting AI processing - pausing stream...");
            
            // КРИТИЧНО: Встановити флаг ДО інференції, щоб зупинити стрім
            inference_active = true;
            
            String inference_result = runInference(fb);
            
            // Встановити флаг ПІСЛЯ інференції, щоб відновити стрім
            inference_active = false;
            
            esp_camera_fb_return(fb);
            
            Serial.print("[RESULT] ");
            Serial.println(inference_result);
            
            global_result = inference_result;
            error_count = 0;  // Reset error count on success            Serial.println("[INFERENCE] AI processing complete - resuming stream...");            
            Serial.println("[FRAME] =====================================\n");
        }
        
        StreamHandler::printMemoryStats();
        
        // Check for too many errors
        if (error_count >= MAX_ERRORS) {
            Serial.println("[CRITICAL] Too many consecutive errors!");
            Serial.println("[SYSTEM] Recommend to restart ESP32");
            error_count = 0;
        }
    }
    delay(10);
}
