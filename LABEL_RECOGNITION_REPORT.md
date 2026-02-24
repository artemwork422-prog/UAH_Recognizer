# Label Recognition Verification Report

## Status: ✅ IMPLEMENTED & READY

### Recognition System Overview

#### Labels Configuration
The system recognizes the following UAH banknote denominations:
- `5_UAH` - 5 гривні
- `10_UAH` - 10 гривні  
- `20_UAH` - 20 гривні
- `50_UAH` - 50 гривні
- `100_UAH` - 100 гривні
- `200_UAH` - 200 гривні
- `500_UAH` - 500 гривні
- `1000_UAH` - 1000 гривні

### Implementation Details

#### 1. Label Validation (`InferenceHandler.h`)
```cpp
bool isValidUAHLabel(const String& label)
```
- Перевіряє наявність `_UAH` суфіксу
- Валідує проти списку дозволених номіналів
- Повертає `true` для валідних labels

#### 2. Enhanced Inference Output
```cpp
String runInference(camera_fb_t* fb)
```
- Детальне логування всіх детекцій у серійний порт
- Format: `[DETECTION] Box N: LABEL - confidence: VALUE`
- Поріг довіри: **50%** для результату

#### 3. Serial Output Format
```
[INFERENCE] Capture Error
[DETECTION] Box 0: 50_UAH - confidence: 0.85
[RECOGNIZED] 50_UAH 85%
[RESULT] 50_UAH 85%
```

### Live Streaming Features

#### StreamHandler Implementation
- **Memory Optimization**: Chunked transmission (4KB buffers)
- **MJPEG Format**: Multi-part image stream
- **Endpoint**: `http://<ESP32_IP>/stream`
- **Bandwidth**: Minimal due to chunked approach

#### Web Interface
- `/` - Live video stream with real-time status
- `/stream` - MJPEG stream (for standalone viewing)
- `/api/scan` - Trigger scanning
- `/api/status` - Get current result + free RAM

### Memory Usage

#### Optimizations Applied
1. **Single Frame Buffer**: 1 x 9216 bytes (96x96 grayscale)
2. **Chunked Streaming**: 4KB buffers instead of full frame in RAM
3. **PSRAM Usage**: Camera frame buffer in external PSRAM
4. **Stack Size**: Minimal HTTP server stack (4096 bytes)

#### Expected Memory Profile
- Camera buffer: ~9 KB (PSRAM)
- Grayscale buffer: ~9 KB (PSRAM)  
- HTTP stack: ~4 KB
- Inference buffer: ~1 KB

**Total minimal RAM**: < 100 KB

### Verification Checklist

- [x] Labels properly configured in `LabelConfig.h`
- [x] `isValidUAHLabel()` validates against known denominations
- [x] Serial port output includes detailed debug information
- [x] Confidence threshold set at 50%
- [x] Invalid labels rejected with `[UNRECOGNIZED_LABEL]` message
- [x] Live stream implementation with minimal memory
- [x] MJPEG streaming endpoint implemented
- [x] Memory statistics displayed on web UI

### Testing Instructions

#### 1. Via Serial Monitor
```
WifiConnect to: 192.168.4.1
Press SCAN button on web interface
Observe serial output:
[FRAME] Processing frame...
[DETECTION] Box 0: 50_UAH - confidence: 0.82
[RECOGNIZED] 50_UAH 82%
[RESULT] 50_UAH 82%
```

#### 2. Via Web Browser
- Visit: `http://192.168.4.1` (ESP32 IP)
- Should see live video stream
- Click "START SCAN" button
- Result appears in status box with confidence %

#### 3. Via Direct Stream
- Visit: `http://192.168.4.1/stream`
- View MJPEG stream directly

### Troubleshooting

#### "Frame error" - Camera issue
- Check camera connection
- Verify PIXFORMAT_GRAYSCALE setting

#### "Confidence below threshold" - Recognition issue
- Object may not be trained in model
- Try better lighting
- Ensure full banknote is visible

#### "[UNRECOGNIZED_LABEL]" - Wrong denomination
- Label not in supported list
- May need model retraining
- Check Edge Impulse dataset

### Serial Port Debugging

Enable detailed logging via serial (115200 baud):
```
[OK] Camera initialized
[OK] Grayscale buffer allocated
[OK] Classifier initialized
[OK] Stream handler initialized
[OK] WiFi AP started - IP: 192.168.4.1
[OK] WebServer started
[READY] System ready for scanning!
```

---

**Last Updated**: 2026-02-24
**System**: ESP32 + Edge Impulse ML
**Streaming**: MJPEG with minimal memory
**Recognition Accuracy**: Model dependent (monitor via serial output)
