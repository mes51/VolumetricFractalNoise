#include "AE_Effect.h"
#include "AEConfig.h"

#include <windows.h>
#include <stdlib.h>
#include <stdint.h>

#if (_MSC_VER >= 1400)
    #define THREAD_LOCAL __declspec(thread)
#endif

#include <string>
#include <memory>
#include <set>
#include <atomic>