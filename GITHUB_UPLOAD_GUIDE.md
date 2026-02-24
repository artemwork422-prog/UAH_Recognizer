# GitHub Upload Instructions

## 🔧 Setup for UAH_Recognizer Repository

### Option 1: Using Personal Access Token (RECOMMENDED)

#### Step 1: Create GitHub Personal Access Token
1. Go to: https://github.com/settings/tokens/new
2. Name: `UAH_Scanner_Token`
3. Select scopes:
   - ✅ `repo` (full control of private repositories)
   - ✅ `workflow` (update GitHub Action workflows)
4. Copy the token (you won't see it again!)

#### Step 2: Configure Git with Token
```powershell
# Set up Git to use token for authentication
git config --global credential.helper wincred

# Or use HTTPS URL with embedded token (NOT RECOMMENDED for security)
git remote add origin https://[TOKEN]@github.com/[USERNAME]/UAH_Recognizer.git
```

#### Step 3: Push to GitHub
```powershell
# Add remote repository
git remote add origin https://github.com/[USERNAME]/UAH_Recognizer.git

# Push to GitHub
git branch -M main
git push -u origin main
```

---

### Option 2: Using SSH Keys

#### Step 1: Generate SSH Key
```powershell
ssh-keygen -t ed25519 -C "your_email@example.com"
# Or for older systems:
ssh-keygen -t rsa -b 4096 -C "your_email@example.com"
```

#### Step 2: Add to GitHub
1. Copy SSH key: `type $env:USERPROFILE\.ssh\id_ed25519.pub`
2. Go to: https://github.com/settings/keys
3. Click "New SSH key"
4. Paste the entire public key content

#### Step 3: Push to GitHub
```powershell
git remote add origin git@github.com:[USERNAME]/UAH_Recognizer.git
git branch -M main
git push -u origin main
```

---

### Option 3: Using GitHub CLI

#### Step 1: Install GitHub CLI
```powershell
winget install GitHub.cli
```

#### Step 2: Authenticate
```powershell
gh auth login
# Select: GitHub.com
# Protocol: HTTPS (or SSH)
# Authenticate with browser
```

#### Step 3: Create Repository
```powershell
gh repo create UAH_Recognizer --source=. --remote=origin --push
```

---

## 📋 What to Provide

To complete the GitHub upload, I need:

1. **GitHub Username**: Your GitHub account username
2. **Authentication Method**:
   - Option A: Personal Access Token (paste it safely)
   - Option B: Confirm SSH keys are set up
   - Option C: Confirm GitHub CLI is installed

---

## Repository Structure

The following will be uploaded:
```
UAH_Recognizer/
├── src/
│   ├── main.cpp                 (Main program)
│   ├── InferenceHandler.h       (AI recognition with label validation)
│   ├── CameraHandler.h          (ESP32 camera configuration)
│   ├── WebServerHandler.h       (Web UI + MJPEG stream)
│   ├── StreamHandler.h          (Minimal memory streaming)
│   └── LabelConfig.h            (Label configuration)
├── lib/
│   └── Robotics_Practice_inferencing/  (Edge Impulse library)
├── platformio.ini               (Build configuration)
├── LABEL_RECOGNITION_REPORT.md  (Verification report)
└── README.md                    (Project documentation)
```

---

## After Upload

Once repository is on GitHub:

1. **Share repository link with team**
2. **Set up GitHub Actions** (optional) for CI/CD
3. **Issues/Discussions** for collaboration
4. **Releases** for stable versions

---

**Status**: Ready for upload ✅
**Repository Name**: UAH_Recognizer
**Branch**: main
**Commits**: 2 (Initial + Enhancements)

Last generated: 2026-02-24
