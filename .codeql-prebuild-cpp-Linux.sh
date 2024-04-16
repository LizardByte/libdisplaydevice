# install dependencies for C++ analysis
set -e

gcc_version=13

sudo apt-get update -y

# allow newer gcc
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y

sudo apt-get install -y \
  build-essential \
  gcc-${gcc_version} \
  g++-${gcc_version} \

# clean apt cache
sudo apt-get clean
sudo rm -rf /var/lib/apt/lists/*

# build
mkdir -p build
cd build || exit 1
cmake \
  -DCMAKE_C_COMPILER="$(which gcc-${gcc_version})" \
  -DCMAKE_CXX_COMPILER="$(which g++-${gcc_version})" \
  -G "Unix Makefiles" ..
make -j"$(nproc)"

# skip autobuild
echo "skip_autobuild=true" >> "$GITHUB_OUTPUT"
