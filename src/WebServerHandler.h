#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "esp_http_server.h"
#include "esp_camera.h"

extern String global_result;
extern volatile bool scan_request;

static const char* index_html = R"rawtext(
<!DOCTYPE html><html><head><meta charset="UTF-8"><title>UAH AI</title>
<style>body{background:#121212;color:#00FF41;text-align:center;font-family:monospace;}</style>
</head><body>
    <h2>UAH GRAYSCALE SCANNER</h2>
    <div id="res" style="font-size:30px;padding:50px;">READY</div>
    <button style="padding:20px;width:200px;" onclick="fetch('/scan', {method:'POST'})">SCAN</button>
    <script>
        setInterval(() => {
            fetch('/status').then(r=>r.json()).then(d=>document.getElementById('res').innerText=d.result);
        }, 1000);
    </script>
</body></html>
)rawtext";

static httpd_handle_t server = NULL;

esp_err_t status_handler(httpd_req_t *req) {
    char json[128];
    snprintf(json, sizeof(json), "{\"result\":\"%s\"}", global_result.c_str());
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json, -1);
}

esp_err_t scan_handler(httpd_req_t *req) {
    scan_request = true;
    return httpd_resp_send(req, "ok", -1);
}

esp_err_t index_handler(httpd_req_t *req) {
    return httpd_resp_send(req, index_html, -1);
}

void startWebServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t u1={"/", HTTP_GET, index_handler, NULL}, 
                    u3={"/scan", HTTP_POST, scan_handler, NULL}, 
                    u4={"/status", HTTP_GET, status_handler, NULL};
        httpd_register_uri_handler(server, &u1); 
        httpd_register_uri_handler(server, &u3); 
        httpd_register_uri_handler(server, &u4);
    }
}