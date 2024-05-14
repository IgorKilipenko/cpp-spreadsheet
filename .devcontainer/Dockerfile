FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-22.04

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="3.22.2"
ARG REINSTALL_CLANG_VERSION_FROM_SOURCE="17"

# Optionally install the cmake for vcpkg
COPY ./reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# [Optional] Uncomment this section to install additional vcpkg ports.
# RUN su vscode -c "${VCPKG_ROOT}/vcpkg install <your-port-name-here>"

# [Optional] Uncomment this section to install additional packages.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
        # Threading Building Blocks - for parallelism
        libtbb-dev \
        libtbb2-dev \
        software-properties-common \
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

# Reinstall clang
COPY COPY ./reinstall-clang.sh /tmp/

RUN if [ "${REINSTALL_CLANG_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/clang_install.sh && /tmp/clang_install.sh ${REINSTALL_CLANG_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/clang_install.sh