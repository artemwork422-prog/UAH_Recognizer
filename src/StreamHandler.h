#ifndef _STREAM_HANDLER_H_
#define _STREAM_HANDLER_H_

#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>

// Мінімальне использование памяти - потокове передавання JPEG
class StreamHandler {
private:
    static const uint16_t STREAM_CHUNK_SIZE = 2048;  // 2KB chunks (reduced from 4KB)
    static const uint16_t MAX_FRAME_SIZE = 65536;    // Максимальний розмір кадру
    
public:
    // Ініціалізація потокового відео
    static bool initStream() {
        Serial.println("Stream handler initialized (minimal memory mode)");
        return true;
    }
    
    // Потокове передавання кадру до HTTP клієнта (MJPEG)
    // Використовується для дистанційного відеоспостереження
    static void streamFrame(WiFiClient* client, camera_fb_t* fb) {
        if (!client || !fb) return;
        
        // MJPEG boundary
        client->write((uint8_t*)"--frame\r\n", strlen("--frame\r\n"));
        
        // Header
        String header = String("Content-Type: image/jpeg\r\n") +
                       "Content-Length: " + String(fb->len) + "\r\n\r\n";
        client->write((uint8_t*)header.c_str(), header.length());
        
        // Передавання по чанках для економії RAM
        size_t offset = 0;
        while (offset < fb->len) {
            size_t chunk_size = min((size_t)STREAM_CHUNK_SIZE, fb->len - offset);
            client->write(fb->buf + offset, chunk_size);
            offset += chunk_size;
        }
        
        // Завершення frame
        client->write((uint8_t*)"\r\n", 2);
    }
    
    // Компактна трансляція з мінімальним буферуванням
    static void streamCompact(WiFiClient* client, camera_fb_t* fb) {
        if (!client || !fb) return;
        
        // Без додатків, просто JPEG data
        client->write(fb->buf, fb->len);
    }
    
    // Отримання статистики використання пам'яті
    static void printMemoryStats() {
        Serial.print("Free heap: ");
        Serial.print(esp_get_free_heap_size());
        Serial.println(" bytes");
    }
};

#endif
