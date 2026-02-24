# Live Streaming - Implementation & Testing Guide

## 🎥 Stream Handler Implementation

### Architecture Overview

```
Camera (96x96 JPEG)
        ↓
Camera Frame Buffer (PSRAM)
        ↓
StreamHandler (Chunked transmission)
        ↓
WiFi AP (2.4 GHz)
        ↓
Client Browser (MJPEG decoder)
```

---

## 💾 Memory Optimization Strategy

### Before Optimization (Traditional)
```
Buffer entire frame in RAM: 65KB → System freeze/crash
```

### After Optimization (Chunked)
```
4KB chunks processed: 9 × 4KB passes → Works smoothly
```

### Current Memory Profile

| Component | Size | Location |
|-----------|------|----------|
| Camera frame buffer | 9 KB | PSRAM |
| Grayscale buffer | 9 KB | PSRAM |
| HTTP server stack | 4 KB | IRAM |
| Stream buffer (chunk) | 4 KB | DRAM |
| Classifier buffer | ~1 KB | IRAM |
| **Total minimal** | **~27 KB** | Mixed |

**ESP32 Total RAM**: 520 KB (IRAM) + 4 MB (PSRAM)
**Free for operations**: ~400 KB after allocation

---

## 🌐 Streaming Endpoints

### Endpoint 1: `/stream` - Direct MJPEG

**Purpose**: Live video feed (optimized for minimal memory)

**Protocol**: 
- Multipart stream
- Boundary: `--frame`
- Format: JPEG images

**Usage**:
```html
<img src="http://192.168.4.1/stream" />
```

**Technical Details**:
```
POST /stream HTTP/1.1
Accept: image/jpeg

Response:
--frame\r\n
Content-Type: image/jpeg\r\n
Content-Length: 8234\r\n
\r\n
[JPEG DATA - 8234 bytes]\r\n
--frame\r\n
...
```

### Endpoint 2: `/` - HTML UI with Stream

**Purpose**: Web dashboard with video + controls

**Features**:
- Embedded MJPEG stream
- Real-time status updates
- Manual scan trigger
- Memory monitoring

**Live Updates**:
```javascript
setInterval(() => {
    fetch('/api/status')
        .then(r => r.json())
        .then(d => {
            document.getElementById('status').innerText = d.result;
            document.getElementById('memory').innerText = 
                'Free RAM: ' + d.heap + ' bytes';
        });
}, 1000);
```

### Endpoint 3: `/api/status` - JSON Status

**Response**:
```json
{
    "result": "50_UAH 85%",
    "heap": 185432
}
```

---

## 🚀 Streaming Performance

### Throughput Analysis

```
Frame size: ~9 KB (96x96 JPEG)
Chunk size: 4 KB
Chunks per frame: 3 × 4 KB

WiFi speed: ~20 Mbps (AP mode, 2.4GHz)
Upload speed: 9KB × 10 FPS = 900 KB/s

Result: ✅ No bottleneck
```

### Frame Rate Calculation

```
Camera capture: ~33ms (30 FPS native)
JPEG encoding: Done by camera
Transmission chunk: 4KB @ 20Mbps ≈ 1.6ms

Total per frame: 
- Send 3 chunks: 3 × 1.6ms = 4.8ms
- Next frame ready: 33ms (non-blocking)

Expected FPS: ~10-15 FPS (MJPEG)
```

---

## 🔧 Configuration

### Camera Settings (`CameraHandler.h`)

```cpp
config.pixel_format = PIXFORMAT_GRAYSCALE;  // 8-bit format
config.frame_size = FRAMESIZE_96X96;        // Minimal size
config.fb_count = 1;                         // Single buffer
config.fb_location = CAMERA_FB_IN_PSRAM;    // Use PSRAM
```

### Stream Settings (`StreamHandler.h`)

```cpp
static const uint16_t STREAM_CHUNK_SIZE = 4096;  // 4KB chunks
static const uint16_t MAX_FRAME_SIZE = 65536;    // 64KB max
```

### HTTP Server Settings (`WebServerHandler.h`)

```cpp
config.stack_size = 4096;  // Minimal stack for memory
config.max_uri_handlers = 4;  // Number of endpoints
```

---

## 🧪 Testing Streaming

### Test 1: Local Browser

```bash
1. Power on ESP32
2. Connect WiFi: "UAH_Scanner" (password: 12345678)
3. Open: http://192.168.4.1
4. Should see:
   ✅ Live video stream
   ✅ Status indicator
   ✅ SCAN button
   ✅ Memory usage
```

### Test 2: Direct Stream

```bash
# Linux/Mac:
curl http://192.168.4.1/stream > stream.mjpeg
ffplay stream.mjpeg

# Windows PowerShell:
Invoke-WebRequest -Uri "http://192.168.4.1/stream" -OutFile "stream.mjpeg"
# Then open with VLC/Media Player
```

### Test 3: Frame Rate Check

```python
import requests
import time

url = "http://192.168.4.1/stream"
frames = 0
start = time.time()

with requests.get(url, stream=True) as r:
    for chunk in r.iter_content(chunk_size=4096):
        frames += 1
        if time.time() - start >= 1:
            print(f"FPS: ~{frames}")
            frames = 0
            start = time.time()
```

### Test 4: Memory Stress

```
1. Send continuous scan requests
2. Monitor serial output: [OK] Free heap: XXXXX bytes
3. Check for memory leaks (heap should stabilize)
4. Stream running for 5+ minutes should not crash
```

---

## 🐛 Troubleshooting

### Issue 1: Stream freezes after 1-2 minutes

**Cause**: Memory leak in streaming loop

**Solution**:
```cpp
// Ensure proper cleanup
if (httpd_resp_send_chunk(...) != ESP_OK) {
    esp_camera_fb_return(fb);
    break;  // Exit loop properly
}
```

### Issue 2: No video in browser

**Cause 1**: WiFi not connected
```
Solution: Check WiFi SSID "UAH_Scanner"
```

**Cause 2**: HTTP server not started
```
log:[OK] WebServer started
```

**Cause 3**: Camera interface incompatible
```
Check: [OK] Camera initialized
```

### Issue 3: Slow/choppy video

**Cause**: Network congestion or large frame size

**Solution**:
```cpp
// Reduce frame size further
config.frame_size = FRAMESIZE_QQVGA;  // 160x120 instead of 96x96

// Or reduce quality
config.jpeg_quality = 10;  // 0-63 (lower = smaller file)
```

### Issue 4: High memory usage

**Cause**: Multiple chunks kept in buffer

**Solution**:
```cpp
// Add delay between frames
vTaskDelay(10 / portTICK_PERIOD_MS);  // Small delay

// Or reduce chunk processing
if (offset % (STREAM_CHUNK_SIZE * 2) == 0) {
    vTaskDelay(1 / portTICK_PERIOD_MS);  // Yield CPU
}
```

---

## 📊 Network Topology

### WiFi AP Mode (Current)

```
ESP32 (192.168.4.1)
 ↕ WiFi AP: "UAH_Scanner"
[Client Devices]
- Phone browser (192.168.4.x)
- Laptop (192.168.4.x)
- Tablet (192.168.4.x)
```

### Station Mode (Future Option)

```
WiFi Router (192.168.1.1)
 ↕
ESP32 Station (192.168.1.x)
 ↕
External Server (via HTTPS)
```

---

## 🔐 Security Considerations

### Current: AP Mode (No Security)

```
⚠️ WARNING: Open network, no encryption
```

### Recommended Improvements

```cpp
// Add WPA2
WiFi.softAP("UAH_Scanner", "secure_password_here", 1, false, 4);

// Add CORS headers
httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

// Add authentication
if (!checkAuthToken(req)) {
    return httpd_resp_send_error(req, HTTPD_403_FORBIDDEN, "Forbidden");
}
```

---

## 🚀 Performance Optimization Tips

1. **Reduce JPEG Quality**:
   ```cpp
   config.jpeg_quality = 15;  // 0-63
   ```

2. **Use SmallFrameSize**:
   ```cpp
   config.frame_size = FRAMESIZE_96X96;  // Already optimal
   ```

3. **Disable Compression**:
   ```cpp
   // Skip JPEG, send raw grayscale (smaller)
   client->write(fb->buf, fb->len);
   ```

4. **Use UDP instead of TCP** (custom implementation):
   ```cpp
   // For extremely low-latency (lossy)
   // Not recommended - stick with TCP/MJPEG
   ```

---

## 📈 Monitoring

### Serial Output Example

```
[WEBSERVER] All endpoints registered
[STREAM] Frame transmitted: 9234 bytes
[STREAM] Free heap: 182450 bytes
[STREAM] Frame transmitted: 9214 bytes
[STREAM] Free heap: 182401 bytes
```

### Health Check

```bash
# Check if stream is running
curl -I http://192.168.4.1/

# Should return HTTP 200 OK
# Header: content-type: multipart/x-mixed-replace
```

---

## ✅ Streaming Verification Checklist

- [x] Chunked transmission implemented (4KB buffers)
- [x] MJPEG format supported
- [x] `/stream` endpoint active
- [x] `/api/status` provides JSON response
- [x] HTML UI displays live video
- [x] Memory optimized for multiple frames
- [x] Error handling for dropped connections
- [x] Multiple concurrent connections supported
- [x] Memory leaks prevented

---

**Last Updated**: 2026-02-24
**Streaming Type**: MJPEG
**Memory Mode**: Minimal (Chunked)
**WiFi Mode**: AP (Access Point)
**Expected FPS**: 10-15
