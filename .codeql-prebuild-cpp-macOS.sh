# install dependencies for C++ analysis
set -e

gcc_version=13

# install dependencies
brew install \
  cmake \
  gcc@${gcc_version} \

# build
mkdir -p build
cd build || exit 1
cmake \
  -DCMAKE_C_COMPILER="/usr/local/bin/gcc-${gcc_version}" \
  -DCMAKE_CXX_COMPILER="/usr/local/bin/g++-${gcc_version}" \
  -G "Unix Makefiles" ..
make -j"$(sysctl -n hw.logicalcpu)"

# skip autobuild
echo "skip_autobuild=true" >> "$GITHUB_OUTPUT"
