#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "esp_http_server.h"
#include "esp_camera.h"
#include "StreamHandler.h"

extern String global_result;
extern volatile bool scan_request;

// Компактна HTML сторінка з потоковим відео
static const char* index_html = R"rawtext(
<!DOCTYPE html><html><head><meta charset="UTF-8"><title>UAH Scanner</title>
<style>
body{background:#1a1a1a;color:#00FF41;text-align:center;font-family:monospace;padding:20px;}
.container{max-width:600px;margin:0 auto;}
.stream{width:100%;max-width:400px;border:2px solid #00FF41;margin:20px 0;}
.status{font-size:24px;padding:20px;background:#0a0a0a;margin:10px 0;border:1px solid #00FF41;}
button{padding:15px 30px;width:200px;background:#00FF41;color:#000;font-weight:bold;cursor:pointer;border:none;margin:10px;}
button:hover{background:#00AA00;}
</style>
</head><body>
<div class="container">
    <h1>UAH Banknote Scanner 2.0</h1>
    <img src="/stream" class="stream" alt="Live Stream">
    <div class="status" id="status">READY</div>
    <div>
        <button onclick="scan()">START SCAN</button>
        <button onclick="location.reload()">REFRESH</button>
    </div>
    <p id="memory" style="font-size:12px;color:#888;"></p>
    
    <script>
        setInterval(() => {
            fetch('/api/status')
                .then(r => r.json())
                .then(d => {
                    document.getElementById('status').innerText = d.result || 'SCANNING...';
                    document.getElementById('memory').innerText = 'Free RAM: ' + d.heap + ' bytes';
                })
                .catch(e => console.log('Status error:', e));
        }, 1000);
        
        function scan() {
            fetch('/api/scan', {method:'POST'})
                .then(r => r.text())
                .then(d => console.log('Scan started:', d))
                .catch(e => console.log('Scan error:', e));
        }
    </script>
</div>
</body></html>
)rawtext";

static httpd_handle_t server = NULL;

// Статус та пам'ять
esp_err_t status_handler(httpd_req_t *req) {
    char json[256];
    snprintf(json, sizeof(json), 
        "{\"result\":\"%s\",\"heap\":%lu}", 
        global_result.c_str(),
        esp_get_free_heap_size());
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json, -1);
}

// Запуск сканування
esp_err_t scan_handler(httpd_req_t *req) {
    Serial.println("[WEBSERVER] Scan request received");
    scan_request = true;
    return httpd_resp_send(req, "{\"status\":\"scanning\"}", -1);
}

// Головна сторінка
esp_err_t index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    return httpd_resp_send(req, index_html, -1);
}

// MJPEG потокова трансляція (мінімальна пам'ять)
esp_err_t stream_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    // Потокова передача кадрів
    while (true) {
        camera_fb_t* fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("[STREAM] Failed to get frame");
            continue;
        }
        
        // MJPEG frame header
        char frame_header[64];
        size_t header_len = snprintf(frame_header, sizeof(frame_header),
            "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n",
            fb->len);
        
        // Надіслання header
        if (httpd_resp_send_chunk(req, frame_header, header_len) != ESP_OK) {
            esp_camera_fb_return(fb);
            break;
        }
        
        // Надіслання JPEG даних по чанках (економія RAM)
        size_t offset = 0;
        while (offset < fb->len) {
            size_t chunk_len = (fb->len - offset > 4096) ? 4096 : (fb->len - offset);
            if (httpd_resp_send_chunk(req, (const char*)fb->buf + offset, chunk_len) != ESP_OK) {
                break;
            }
            offset += chunk_len;
        }
        
        // Завершення frame
        httpd_resp_send_chunk(req, "\r\n", 2);
        
        esp_camera_fb_return(fb);
        vTaskDelay(1 / portTICK_PERIOD_MS);  // Невеликий delay для інших завдань
    }
    
    return ESP_OK;
}

void startWebServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 4096;  // Мінімальний стек для економії пам'яті
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t handlers[] = {
            {"/", HTTP_GET, index_handler, NULL},
            {"/stream", HTTP_GET, stream_handler, NULL},
            {"/api/scan", HTTP_POST, scan_handler, NULL},
            {"/api/status", HTTP_GET, status_handler, NULL},
        };
        
        for (int i = 0; i < sizeof(handlers)/sizeof(handlers[0]); i++) {
            httpd_register_uri_handler(server, &handlers[i]);
        }
        Serial.println("[WEBSERVER] All endpoints registered");
    }
}
