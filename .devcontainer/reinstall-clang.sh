#!/usr/bin/env bash

set -e

CLANG_VERSION=${1:-"none"}

if [ "${CLANG_VERSION}" = "none" ]; then
    echo "No Clang version specified, skipping Clang reinstallation"
    exit 0
fi

set +e

# Remove installed Clang
apt-get -y purge --auto-remove clang

# Install LLVM
wget https://apt.llvm.org/llvm.sh -P /tmp
chmod +x /tmp/llvm.sh
/tmp/llvm.sh ${CLANG_VERSION} all
rm -f /tmp/llvm.sh

update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-${CLANG_VERSION} 100
update-alternatives --install /usr/bin/clang clang /usr/bin/clang-${CLANG_VERSION} 100
update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-${CLANG_VERSION} 100