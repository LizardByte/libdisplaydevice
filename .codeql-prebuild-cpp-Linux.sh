# install dependencies for C++ analysis
set -e

gcc_version=11

sudo apt-get update -y

# allow newer gcc
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y

sudo apt-get install -y \
  build-essential \
  cmake \
  ninja-build

# clean apt cache
sudo apt-get clean
sudo rm -rf /var/lib/apt/lists/*

# build
mkdir -p build
cd build || exit 1
cmake -G Ninja ..
ninja

# skip autobuild
echo "skip_autobuild=true" >> "$GITHUB_OUTPUT"
