FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-22.04

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="3.22.2"
ARG REINSTALL_CLANG_VERSION_FROM_SOURCE="17"

# Optionally install the cmake for vcpkg
COPY ./reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# Install apt packges
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
        openjdk-17-jdk \
        libtbb-dev \
        libtbb2-dev \
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

# Reinstall clang
COPY COPY ./reinstall-clang.sh /tmp/

RUN if [ "${REINSTALL_CLANG_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/clang_install.sh && /tmp/clang_install.sh ${REINSTALL_CLANG_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/clang_install.sh

ARG LIBS_DIR=/usr/local/lib
ARG ANTLR_SRC=${LIBS_DIR}/src
ARG ANTLR_RUNTIME=${ANTLR_SRC}/antlr4/runtime/Cpp

# Add ANTLR4 JAR file
ADD https://www.antlr.org/download/antlr-4.13.1-complete.jar /tmp/antlr-4.13.1-complete.jar
ADD https://www.antlr.org/download/antlr-4.13.1-complete.jar /tmp/antlr-4.13.1-complete.jar

# Install ANTLR4
ARG ANTLR_SRC=/usr/local/src
ARG ANTLR_RUNTIME=${ANTLR_SRC}/antlr4/runtime/Cpp
RUN cp /tmp/antlr-4.13.1-complete.jar ${LIBS_DIR}/antlr-4.13.1-complete.jar && \
    rm /tmp/antlr-4.13.1-complete.jar

# Download and build C++ ANTLR4 runtime
RUN git clone https://github.com/antlr/antlr4.git ${ANTLR_SRC}/antlr4
RUN mkdir ${ANTLR_RUNTIME}/build && \
    cd ${ANTLR_RUNTIME}/build && \
    cmake .. && \
    make && \
    make install

# Set environment variables
ENV CLASSPATH=".:${LIBS_DIR}/antlr-4.13.1-complete.jar:$CLASSPATH"
ENV PATH="${ANTLR_RUNTIME}/build/bin:$PATH"