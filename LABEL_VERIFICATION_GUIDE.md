# Label Recognition - Verification & Testing Guide

## 🔍 Label Validation System

### Current Implementation Status
✅ **Active in: `InferenceHandler.h`**

### Supported Labels

```
5_UAH    → 5 гривні (синьо-фіолетова)
10_UAH   → 10 гривні (коричнева)
20_UAH   → 20 гривні (зелена)
50_UAH   → 50 гривні (червона) 
100_UAH  → 100 гривні (синя)
200_UAH  → 200 гривні (зеленувата)
500_UAH  → 500 гривні (фіолетова)
1000_UAH → 1000 гривні (коричнева)
```

---

## 🧪 Testing Labels

### Method 1: Serial Port Output

```
[FRAME] Processing frame...
[DETECTION] Box 0: 50_UAH - confidence: 0.85
[DETECTED] detected box with confidence score

[INFERENCE] Capture Error      ❌ (Camera issue)
[UNRECOGNIZED_LABEL] xyz_ABC   ❌ (Wrong label)
[RECOGNIZED] 50_UAH 85%        ✅ (Valid label + confidence)
```

### Method 2: Web Interface

1. Connect to WiFi: `UAH_Scanner`
2. Visit: `http://192.168.4.1`
3. Status should show: `READY` or `[LABEL] XX%`
4. Live video stream visible in browser
5. Click `START SCAN` to trigger recognition

### Method 3: Direct API

```bash
# Get current status (includes recognition result)
curl http://192.168.4.1/api/status

# Response example:
# {"result":"50_UAH 85%","heap":123456}
```

---

## 🐛 Debugging Label Issues

### Problem 1: Label shows "Scanning..."

**Cause**: Confidence below 50% threshold

**Solution**:
- Adjust lighting on banknote
- Show full face of banknote (not tilted)
- Move closer to camera
- Check camera focus

**Check confidence in serial**:
```
[DETECTION] Box 0: 50_UAH - confidence: 0.42   ← Below 50%
[SCANNING] Confidence below threshold
```

### Problem 2: "[UNRECOGNIZED_LABEL] abc_XYZ"

**Cause**: Model detected wrong label pattern

**Solution**:
- Unit label is not in training dataset
- Edge Impulse model needs retraining
- Check model version

**Expected valid patterns**:
```
✅ 5_UAH, 10_UAH, 20_UAH, 50_UAH
✅ 100_UAH, 200_UAH, 500_UAH, 1000_UAH

❌ 27_UAH (not standard)
❌ UAH_50 (wrong format)
❌ 50 (missing _UAH)
```

### Problem 3: "[INFERENCE] Capture Error"

**Cause**: Camera frame capture failed

**Solution**:
```cpp
// Check camera initialization:
if (fb == NULL) {
    Serial.println("Camera not initialized");
}

// Verify PSRAM available:
Serial.print("PSRAM free: ");
Serial.println(esp_get_free_psram_size());
```

### Problem 4: No labels detected (empty bounding_boxes)

**Cause**: Model not recognizing object

**Solution**:
- Ensure banknote is clearly visible
- Check lighting (not too bright/dark)
- Verify camera resolution matches training (96x96)
- Check model file is loaded correctly

---

## 📊 Label Validation Logic

### Current Validation Function

```cpp
bool isValidUAHLabel(const String& label) {
    // Check for _UAH suffix
    if (label.indexOf("_UAH") == -1) {
        return false;
    }
    
    // Whitelist check
    const char* valid_labels[] = {
        "5_UAH", "10_UAH", "20_UAH", "50_UAH",
        "100_UAH", "200_UAH", "500_UAH", "1000_UAH"
    };
    
    for (int i = 0; i < 8; i++) {
        if (label.equals(valid_labels[i])) {
            return true;
        }
    }
    
    return false;  // Invalid label rejected
}
```

### Confidence Threshold
```cpp
if (best_val > 0.50f) {  // 50% threshold
    return result_str;
} else {
    return "Scanning...";
}
```

---

## 📈 Performance Metrics

### Expected Results

| Scenario | Output | Status |
|----------|--------|--------|
| Clear banknote @ 50% confidence | "50_UAH 50%" | ✅ Valid |
| Clear banknote @ 99% confidence | "50_UAH 99%" | ✅ Valid |
| Blurry @ 40% confidence | "Scanning..." | ⏳ Retry |
| Unknown label | "[UNRECOGNIZED_LABEL]" | ❌ Invalid |
| No object detected | "Scanning..." | ⏳ Retry |

### Memory During Recognition

```
[OK] Free heap: 185432 bytes (initial)
[FRAME] Processing frame...
[RECOGNITION] In progress...
[OK] Free heap: 180240 bytes (during inference)
[RESULT] 50_UAH 85%
[OK] Free heap: 185432 bytes (restored)
```

---

## 🔧 Configuration Changes

### To Modify Threshold
```cpp
// InferenceHandler.h, line ~95
if (best_val > 0.50f) {  // Change this value
    return best_label + " " + String((int)(best_val * 100)) + "%";
}
```

### To Add New Label
```cpp
// In isValidUAHLabel()
const char* valid_labels[] = {
    "5_UAH", ..., "10000_UAH"  // Add new denomination
};

// And update NUM_LABELS
static const int NUM_LABELS = 9;  // From 8
```

### To Change Output Format
```cpp
// In runInference() function
Serial.print("[RECOGNIZED] ");  // Change prefix
Serial.println(result_str);
```

---

## 🧬 Training Dataset Requirements

For Edge Impulse model to recognize labels:

1. **Dataset must contain**:
   - Images of each UAH denomination
   - Various angles and lighting conditions
   - At least 20-50 images per class

2. **Label format in training MUST be**:
   - Exactly: `5_UAH`, `10_UAH`, `20_UAH`, etc.
   - NO spaces or variations

3. **Model output validation**:
   - Model should return class labels exactly as trained
   - Confidence scores 0.0-1.0 (normalized)

---

## ✅ Verification Checklist

- [ ] All supported labels hardcoded in code
- [ ] Label validation function checks whitelist
- [ ] Serial port outputs "[RECOGNIZED]" for valid labels
- [ ] Serial port outputs "[UNRECOGNIZED_LABEL]" for invalid labels
- [ ] Confidence threshold working (50%)
- [ ] Web UI shows recognition results
- [ ] Memory statistics logged
- [ ] No crashes on unknown labels

---

## 🚀 Next Steps for Production

1. **Expand label support** if needed
2. **Fine-tune confidence threshold** based on real testing
3. **Add logging** to database for analytics
4. **Implement error recovery** for failed frames
5. **Test with different banknote conditions**
6. **Add rate limiting** for API endpoints

---

**Last Updated**: 2026-02-24
**System**: ESP32 + Edge Impulse
**Framework**: Arduino / PlatformIO
**Model Type**: FOMO (Fast Object Detection for Microcontrollers)
