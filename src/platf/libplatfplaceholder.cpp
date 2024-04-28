#include "libdisplaydevice/libplatfplaceholder.h"

#ifdef DD_WIN
  #include "libdisplaydevice/windows/winapilayerinterface.h"
#else
static_assert(false, "LOL");
#endif

int
plaftplaceholder() {
  return 0;
}
