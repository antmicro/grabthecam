#!/bin/bash

# Copyright (c) 2022-2023 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

set -e

echo "Installing dependencies..."
export DEBIAN_FRONTEND=noninteractive

apt-get update >/dev/null 2>&1
apt-get install -qqy \
    clang \
    clang-format \
    clang-tidy \
    cmake \
    git \
    libopencv-dev \
    libv4l-dev \
    make \
    rapidjson-dev \
    libglfw3-dev \
    libglew-dev \
    libgl1-mesa-dev \
    >/dev/null 2>&1

echo "Dependencies installed successfully"
