# Use an appropriate base image
FROM ubuntu:22.04

# Set environment variables for non-interactive installation
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    wget \
    g++ \
    pkg-config \
    libpoppler-cpp-dev \
    libleptonica-dev \
    libpthread-stubs0-dev \
    libfontconfig1-dev \
    libfreetype-dev \
    libxml2-dev \
    libssl-dev \
    libjpeg-dev \
    libpng-dev \
    libtiff-dev \
    libidn11-dev \
    ca-certificates \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Determine architecture and set CMake version
ARG CMAKE_VERSION="3.29.4"

# Download and install CMake based on architecture
RUN ARCH=$(dpkg --print-architecture) && \
    if [ "$ARCH" = "amd64" ]; then \
        CMAKE_FILE="cmake-${CMAKE_VERSION}-linux-x86_64.sh"; \
    elif [ "$ARCH" = "arm64" ]; then \
        CMAKE_FILE="cmake-${CMAKE_VERSION}-linux-aarch64.sh"; \
    else \
        echo "Unsupported architecture: $ARCH"; exit 1; \
    fi && \
    cd /tmp && \
    wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/${CMAKE_FILE} && \
    chmod +x ${CMAKE_FILE} && \
    ./${CMAKE_FILE} --skip-license --prefix=/usr/local && \
    rm ${CMAKE_FILE}

# Set up the working directory
WORKDIR /app

# Copy over files
COPY form.cpp /app/form.cpp
COPY form.h /app/form.h
COPY Question.cpp /app/Question.cpp
COPY Question.h /app/Question.h
COPY main.cpp /app/main.cpp
COPY dockerCMakeLists.txt /app/CMakeLists.txt

# Create build directory and copy PDF files
RUN mkdir -p build
COPY nico.pdf /app/build/nico.pdf
COPY victor.pdf /app/build/victor.pdf


# Build the project
WORKDIR /app/build
RUN cmake .. && make


# ---------------------------------------------------------------------------
# Flask Project
WORKDIR /app

RUN mkdir -p output

RUN apt-get update && apt-get install -y --no-install-recommends --assume-yes \
    python3 \
    python3-pip \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# cooy over the requirements file
COPY requirements.txt /app/
RUN pip3 install -r requirements.txt

# Copy over the Python files
COPY webApp.py /app/
COPY templates /app/templates
COPY static /app/static

EXPOSE 3030

CMD ["python3", "webApp.py"]