# install dependencies for C++ analysis
set -e

# install dependencies
brew install \
  cmake \
  ninja \
  boost \
  nlohmann-json

# build
mkdir -p build
cd build || exit 1
cmake \
  -DBUILD_DOCS=OFF \
  -G Ninja ..
ninja

# skip autobuild
echo "skip_autobuild=true" >> "$GITHUB_OUTPUT"
