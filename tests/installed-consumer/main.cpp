/* SPDX-License-Identifier: MIT */
/* Direct C++17 compile and link of the installed package (D-38, plan 04-23). The gate step
 * pkg-install builds it with ${CXX:-c++} -std=c++17 against <prefix>/include and
 * <prefix>/lib/libivs_core.a, outside CMake: a public header without its extern "C" block would
 * compile here but fail to link (mangled names). The consumer CMake project (LANGUAGES C) ignores
 * this file. */
#include <cstdio>
#include <cstring>

#include <ivs/ivs.h>

int main()
{
  const uint32_t lib = ivs_api_version();
  const char *ok = ivs_status_name(IVS_OK);
  const char *dup = ivs_status_name(IVS_E_DUPLICATE_LOAD);
  const bool good = lib == static_cast<uint32_t>(IVS_API_VERSION) && std::strcmp(ok, "IVS_OK") == 0 &&
                    std::strcmp(dup, "IVS_E_DUPLICATE_LOAD") == 0;
  std::printf("cxx-link ok=%s\n", good ? "yes" : "no");
  return good ? 0 : 1;
}
