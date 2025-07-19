FROM gcc:14

RUN apt-get update && apt-get install -y \
    wget \
    gnupg \
    lsb-release \
    software-properties-common \
    clang-format \
    clang-tidy \
    cmake

# Clang 17
# Add LLVM APT repo for Debian bookworm (Debian 12)
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
 && echo "deb http://apt.llvm.org/bookworm/ llvm-toolchain-bookworm-17 main" > /etc/apt/sources.list.d/llvm.list \
 && apt-get update

# Install Clang 17, libc++17, libc++abi17
RUN apt-get install -y clang-17 libc++-17-dev libc++abi-17-dev

# Setup update-alternatives for clang and clang++
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-17 60 \
    --slave /usr/bin/clang++ clang++ /usr/bin/clang++-17

WORKDIR /workspace

COPY . .

CMD ["/bin/bash"]
