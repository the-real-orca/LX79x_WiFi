FROM python:3.12-bookworm

ARG ARDUINO_CLI_VERSION=0.35.3
ARG ESP_IDF_VERSION=v5.2
ARG NODE_MAJOR=20

ENV DEBIAN_FRONTEND=noninteractive
ENV IDF_PATH=/opt/esp-idf
ENV PLATFORMIO_CORE_DIR=/opt/platformio
ENV PATH="/root/.local/bin:/usr/local/bin:${PATH}"

# Install additional system packages not already covered by the base image
RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    ninja-build \
    pkg-config \
    dfu-util \
    libusb-1.0-0 \
    && rm -rf /var/lib/apt/lists/*

# Install Arduino CLI, configure board sources, and install cores
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh \
    | BINDIR=/usr/local/bin sh -s -- ${ARDUINO_CLI_VERSION} \
    && arduino-cli config init \
    && arduino-cli config add board_manager.additional_urls \
        https://espressif.github.io/arduino-esp32/package_esp32_index.json \
    && arduino-cli core update-index \
    && arduino-cli core install arduino:avr \
    && arduino-cli core install esp32:esp32

# Install Python tooling and initialize PlatformIO storage
RUN pip install --no-cache-dir --upgrade pip \
    && pip install --no-cache-dir platformio pyserial \
    && mkdir -p ${PLATFORMIO_CORE_DIR}

# Install ESP-IDF and load its environment automatically in interactive shells
RUN git clone --recursive https://github.com/espressif/esp-idf.git ${IDF_PATH} \
    && cd ${IDF_PATH} \
    && git checkout ${ESP_IDF_VERSION} \
    && ./install.sh \
    && echo "source ${IDF_PATH}/export.sh" >> /root/.bashrc

# Install pytest for testing
RUN pip install --no-cache-dir pytest

# Install Node.js 20 for Gemini CLI
RUN mkdir -p /etc/apt/keyrings \
    && curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key \
    | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg \
    && echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_${NODE_MAJOR}.x nodistro main" \
    > /etc/apt/sources.list.d/nodesource.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends nodejs \
    && rm -rf /var/lib/apt/lists/*

# Install Gemini CLI
RUN echo "build Gemini CLI v0.35.3"
RUN npm install -g @google/gemini-cli

WORKDIR /workspace
CMD ["/bin/bash"]