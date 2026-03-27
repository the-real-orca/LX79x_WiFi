# LX790 ESP32 Docker Environment

This repository provides a comprehensive Docker environment for developing, building, and flashing the LX790 ESP32 project.

## Features
- **Python 3.12**: Modern Python environment.
- **PlatformIO**: Primary build system for this project.
- **Arduino CLI (v0.35.3)**: Alternative build system with `esp32:esp32` and `arduino:avr` cores.
- **ESP-IDF (v5.2)**: For low-level ESP32 development.
- **Node.js 20 & Gemini CLI**: Built-in AI-powered development assistance.
- **Build Tools**: Includes `cmake`, `ninja`, `dfu-util`, and `libusb-1.0-0`.

---

## 1. Setup

### Build the Docker Image
```bash
docker build -t lx790-dev .
```

### Start an Interactive Shell
```bash
docker run --rm -it -v ./:/workspace lx790-dev
```

---

## 2. Usage with PlatformIO (Recommended)

PlatformIO is the preferred way to manage dependencies and build this project.

### Build Project
```bash
docker run --rm -v ./:/workspace lx790-dev pio run
```

### Clean Build Files
```bash
docker run --rm -v ./:/workspace lx790-dev pio run -t clean
```

### Flash to Device
Ensure your ESP32 is connected and recognized (usually `/dev/ttyUSB0`).
```bash
docker run --rm -it --device=/dev/ttyUSB0 -v ./:/workspace lx790-dev pio run -t upload
```

#### Windows

Make sure that the USB is accessable to docker.
```PowerShell
usbipd list
usbipd bind --busid <BUSID>
usbipd attach --wsl --busid <BUSID>
```


---

## 3. Usage with Arduino CLI

If you prefer using the Arduino CLI directly for the `.ino` sketch:

### Compile Sketch
```bash
docker run --rm -v ./:/workspace lx790-dev \
    arduino-cli compile --fqbn esp32:esp32:esp32doit-devkit-v1 /workspace/src/LX790_ESP32.ino
```

### Upload Sketch
```bash
docker run --rm -it --device=/dev/ttyUSB0 -v ./:/workspace lx790-dev \
    arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32doit-devkit-v1 /workspace/src/LX790_ESP32.ino
```

---

## 4. Usage with Gemini CLI

The environment includes the Gemini CLI for AI assistance.

### Run Gemini CLI
```bash
docker run --rm -it -v ./:/workspace -e GEMINI_API_KEY=$GEMINI_API_KEY lx790-dev gemini-cli
```

---

## Troubleshooting & Tips

### Permission Denied on /dev/ttyUSB0
If you encounter permission issues when flashing, you might need to add your user to the `dialout` group or use `sudo` (not recommended).
Alternatively, temporarily change permissions:
```bash
sudo chmod 666 /dev/ttyUSB0
```

### Build Context
A `.dockerignore` file is used to speed up the image build process by excluding unnecessary directories like `.git`, `.pio`, `docs`, and `pic`.
