#!/usr/bin/env bash

set -e

CLANG_VERSION=${1:-"none"}

if [ "${CLANG_VERSION}" = "none" ]; then
    echo "No Clang version specified, skipping Clang reinstallation"
    exit 0
fi

# Remove installed Clang
echo "Removing existing Clang installation..."
apt-get -y purge --auto-remove clang && apt-get autoremove -y && apt-get clean -y
rm -rf /var/lib/apt/lists/*
echo "Clang removal completed."

# Install LLVM
echo "Downloading and installing LLVM ${CLANG_VERSION}..."
wget -O /tmp/llvm.sh https://apt.llvm.org/llvm.sh
chmod +x /tmp/llvm.sh
/tmp/llvm.sh ${CLANG_VERSION} all
rm -f /tmp/llvm.sh
echo "LLVM installation completed."

# Update alternatives for clang, clang++, and clangd
update_alternatives() {
    local tool=$1
    local version=$2
    if [ -f /usr/bin/${tool}-${version} ]; then
        update-alternatives --install /usr/bin/${tool} ${tool} /usr/bin/${tool}-${version} 100
        echo "${tool}-${version} set as default."
    else
        echo "${tool}-${version} not found, skipping."
    fi
}

echo "Updating alternatives for clang, clang++, and clangd..."
update_alternatives "clang" ${CLANG_VERSION}
update_alternatives "clang++" ${CLANG_VERSION}
update_alternatives "clangd" ${CLANG_VERSION}

echo "Clang ${CLANG_VERSION} installation and configuration completed."