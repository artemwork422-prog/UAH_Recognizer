#include <Arduino.h>
#include "CameraHandler.h"
#include "InferenceHandler.h"

String global_result = "Ready";
int error_count = 0;
const int MAX_ERRORS = 10;
const int CAPTURE_INTERVAL_MS = 3000;  // Capture every 3 seconds
unsigned long last_capture_time = 0;

void printSystemInfo() {
    Serial.println("\n========================================");
    Serial.println("    UAH Banknote Scanner v2.0");
    Serial.println("    Automatic AI Recognition (No WiFi)");
    Serial.println("========================================");
    Serial.printf("ESP32 Chip Model: %d\n", ESP.getChipModel());
    Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / 1024 / 1024);
    Serial.printf("Free Heap: %u bytes\n", esp_get_free_heap_size());
    Serial.printf("PSRAM Size: %u bytes\n", ESP.getPsramSize());
    Serial.printf("Capture Interval: %d ms\n\n", CAPTURE_INTERVAL_MS);
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
    
    Serial.println("\n[READY] ✓ System ready!");
    Serial.println("[INFO] Automatic scanning enabled - camera captures every %d ms", CAPTURE_INTERVAL_MS);
    Serial.println("[INFO] Results output to Serial Monitor only\n");
    
    last_capture_time = millis();
}

void loop() {
    unsigned long current_time = millis();
    
    // Automatic capture every CAPTURE_INTERVAL_MS
    if (current_time - last_capture_time >= CAPTURE_INTERVAL_MS) {
        last_capture_time = current_time;
        
        camera_fb_t* fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("[ERROR] Failed to get camera frame");
            error_count++;
            global_result = "Frame error";
        } else {
            Serial.println("\n[FRAME] =====================================");
            Serial.println("[INFERENCE] Starting AI processing...");
            Serial.printf("[TIME] Captured at: %lu ms\n", current_time);
            
            String inference_result = runInference(fb);
            
            esp_camera_fb_return(fb);
            
            Serial.print("[RESULT] ");
            Serial.println(inference_result);
            
            global_result = inference_result;
            error_count = 0;  // Reset error count on success
            Serial.println("[INFO] Free Heap: %u bytes", esp_get_free_heap_size());
            Serial.println("[FRAME] =====================================\n");
        }
        
        // Check for too many errors
        if (error_count >= MAX_ERRORS) {
            Serial.println("[CRITICAL] Too many consecutive errors!");
            Serial.println("[SYSTEM] Recommend to restart ESP32");
            error_count = 0;
        }
    }
    
    delay(100);  // Small delay to prevent watchdog issues
}
