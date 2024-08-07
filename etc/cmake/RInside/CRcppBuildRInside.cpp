#include <R_ext/Print.h>

extern "C" {
void CRcppBuildRInside() {
    Rprintf("CRcpp build of RInside for ");
#if defined(_WIN32) || defined(_MSC_VER)
    Rprintf("Windows");
    
#ifdef _MSC_VER
    Rprintf(" (MSVC)");
#else
    Rprintf(" (GCC)");
#endif
    
#else
    Rprintf("Unix");
#endif
    Rprintf("\n");
}
}
