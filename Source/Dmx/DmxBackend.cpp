#include "DmxBackend.h"
#include "NullDmxBackend.h"

#if HITDMX_HAVE_FTDI_D2XX
 #include "EnttecProBackend.h"
#endif

namespace hitdmx
{

std::unique_ptr<DmxBackend> DmxBackend::createDefault()
{
   #if HITDMX_HAVE_FTDI_D2XX
    return std::make_unique<EnttecProBackend>();
   #else
    return std::make_unique<NullDmxBackend>();
   #endif
}

}
