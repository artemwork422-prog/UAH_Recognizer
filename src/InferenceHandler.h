#ifndef _INFERENCE_HANDLER_H_
#define _INFERENCE_HANDLER_H_

#include <Arduino.h>
#include "esp_camera.h"
#include <Robotics_Practice_inferencing.h>

#define EI_CAMERA_RAW_FRAME_BUFFER_COLS 96
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS 96
#define EI_CAMERA_RAW_FRAME_SIZE (EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS)

// Один буфер для 8-біт граyscale (96x96 = 9216 байт)
static uint8_t *gray_buffer = NULL;

// Функція для отримання даних - просто нормалізуємо от 0 до 1
static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr) {
    for (size_t i = 0; i < length; i++) {
        out_ptr[i] = (float)gray_buffer[offset + i] / 255.0f;
    }
    return 0;
}

// Ініціалізація буфера при першому запиту
bool ei_camera_init() {
    if (gray_buffer != NULL) return true;
    
    gray_buffer = (uint8_t *)ps_malloc(EI_CAMERA_RAW_FRAME_SIZE);
    if (gray_buffer == NULL) {
        Serial.println("Failed to allocate gray buffer");
        return false;
    }
    return true;
}

// Захоплення кадру - камера вже дає Grayscale, просто копіюємо
bool ei_camera_capture(camera_fb_t* fb) {
    if (!fb || !gray_buffer) return false;
    
    // Якщо камера налаштована на PIXFORMAT_GRAYSCALE, 
    // fb->buf містить 8-біт grayscale дані (9216 байт для 96x96)
    if (fb->len != EI_CAMERA_RAW_FRAME_SIZE) {
        // Якщо розмір не збігається, це може бути додатковий header
        if (fb->len >= EI_CAMERA_RAW_FRAME_SIZE) {
            memcpy(gray_buffer, fb->buf, EI_CAMERA_RAW_FRAME_SIZE);
        } else {
            return false;
        }
    } else {
        memcpy(gray_buffer, fb->buf, EI_CAMERA_RAW_FRAME_SIZE);
    }
    
    return true;
}

// Перевірка чи label є валідна UAH номінал
bool isValidUAHLabel(const String& label) {
    // Перевіряємо наявність '_UAH' у label
    if (label.indexOf("_UAH") == -1) {
        return false;
    }
    
    // Перелік допустимих номіналів
    const char* valid_labels[] = {
        "5_UAH", "10_UAH", "20_UAH", "50_UAH", 
        "100_UAH", "200_UAH", "500_UAH", "1000_UAH"
    };
    
    for (int i = 0; i < 8; i++) {
        if (label.equals(valid_labels[i])) {
            return true;
        }
    }
    
    return false;
}

// Запуск інференції з обробкою labels та логуванням
String runInference(camera_fb_t* fb) {
    if (!fb) {
        Serial.println("[INFERENCE] ERROR: Frame buffer is NULL");
        return "Frame NULL";
    }
    
    if (fb->buf == NULL || fb->len == 0) {
        Serial.printf("[INFERENCE] ERROR: Invalid frame - buf=%p len=%u\n", fb->buf, fb->len);
        return "Frame Invalid";
    }
    
    if (!ei_camera_capture(fb)) {
        Serial.println("[INFERENCE] ERROR: Failed to capture frame to grayscale buffer");
        return "Capture Failed";
    }

    // Перевірка попередньої за буфером
    if (gray_buffer == NULL) {
        Serial.println("[INFERENCE] ERROR: Grayscale buffer not allocated!");
        return "Buffer Error";
    }

    // Підготовка сигналу для класифікатора    
    ei::signal_t signal;
    signal.total_length = EI_CAMERA_RAW_FRAME_SIZE;
    signal.get_data = &ei_camera_get_data;

    ei_impulse_result_t result = { 0 };
    
    uint32_t start_time = millis();
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
    uint32_t inference_time = millis() - start_time;

    if (res != EI_IMPULSE_OK) {
        Serial.print("[INFERENCE] ERROR: Classifier failed with code: ");
        Serial.println(res);
        return "Classifier Error";
    }
    
    Serial.printf("[INFERENCE] Completed in %lu ms\n", inference_time);

    float best_val = 0.0f;
    String best_label = "";

    // Модель використовує FOMO (об'єктна детекція)
    if (result.bounding_boxes_count > 0) {
        Serial.printf("[INFERENCE] Found %u bounding boxes\n", result.bounding_boxes_count);
        
        for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
            if (result.bounding_boxes[i].label == NULL) {
                Serial.printf("[DETECTION] Box %u: NULL label, confidence: %.2f\n",
                             i, result.bounding_boxes[i].value);
                continue;
            }
            
            Serial.printf("[DETECTION] Box %u: %s (%.2f)\n",
                         i, 
                         result.bounding_boxes[i].label,
                         result.bounding_boxes[i].value);
            
            if (result.bounding_boxes[i].value > best_val) {
                best_val = result.bounding_boxes[i].value;
                best_label = String(result.bounding_boxes[i].label);
            }
        }
    } else {
        Serial.println("[INFERENCE] No objects detected (empty bounding boxes)");
    }

    // Поріг довіри 50%
    if (best_val > 0.50f) {
        // Перевіряємо чи це валідна UAH номіналу
        if (isValidUAHLabel(best_label)) {
            String result_str = best_label + " " + String((int)(best_val * 100)) + "%";
            Serial.print("[RECOGNIZED] ");
            Serial.println(result_str);
            return result_str;
        } else {
            Serial.print("[UNRECOGNIZED_LABEL] ");
            Serial.println(best_label);
            return "Invalid: " + best_label;
        }
    }

    Serial.printf("[INFERENCE] Confidence %.2f below threshold (50%%)\n", best_val);
    return "Scanning...";
}

void ei_camera_deinit() {
    if (gray_buffer != NULL) {
        free(gray_buffer);
        gray_buffer = NULL;
    }
}

#endif