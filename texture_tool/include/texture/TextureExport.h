#pragma once

#if defined(_WIN32)
  #if defined(TEXTURE_BUILD_SHARED)
    #if defined(TEXTURE_EXPORTS)
      #define TEXTURE_API __declspec(dllexport)
    #else
      #define TEXTURE_API __declspec(dllimport)
    #endif
  #else
    #define TEXTURE_API
  #endif
#else
  #define TEXTURE_API
#endif
