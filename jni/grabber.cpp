#include <dlfcn.h>
#include <stdio.h>
#include "grabber.h"

Grabber::Grabber(){
  printf("--Loading flinger native lib--\n");
  int i, len;
  char lib_name[64];

  len = ARR_LEN(compiled_sdks);
  for (i = 0; i < len; i++) {

    #if defined(_LP64_)
      #define DVNC_LIB_PATH "/system/lib64"
      sprintf(lib_name, DVNC_LIB_PATH "/libvncGraphicsClient_sdk%d.so", compiled_sdks[i]);
    #else
      #define DVNC_LIB_PATH "system/lib"
      sprintf(lib_name, DVNC_LIB_PATH "/libvncGraphicsClient_sdk%d.so", compiled_sdks[i]);
    #endif
    
    printf("Loading lib: %s\n", lib_name);

    void *flinger_lib = dlopen(lib_name, RTLD_NOW);

    if (flinger_lib == nullptr) {
      printf("Couldnt load flinger library %s! Error string: %s\n", lib_name, dlerror());
      continue;
    }

    base.init_flinger = (init_flinger)dlsym(flinger_lib, "init_flinger");
    if (base.init_flinger == nullptr) {
      printf("Couldn't load init_flinger! Error string: %s\n", dlerror());
      continue;
    }

    base.close_flinger = (close_flinger)dlsym(flinger_lib, "close_flinger");
    if (base.close_flinger == nullptr) {
      printf("Couldn't load close_flinger! Error string: %s\n", dlerror());
      continue;
    }

    base.lockBuffer_flinger = (lockBuffer_flinger)dlsym(flinger_lib, "lockGraphicsBuffer_flinger");
    if (base.lockBuffer_flinger == nullptr) {
      printf("Couldn't load lockGraphicsBuffer_flinger! Error string: %s\n", dlerror());
      continue;
    }

    base.unlockBuffer_flinger = (unlockBuffer_flinger)dlsym(flinger_lib, "unlockGraphicsBuffer_flinger");
    if (base.unlockBuffer_flinger == nullptr) {
      printf("Couldn't load unlockGraphicsBuffer_flinger! Error string: %s\n", dlerror());
      continue;
    }

    base.getFrameFormat_flinger = (getFrameBuffer_flinger)dlsym(flinger_lib, "getScreenformat_flinger");
    if (base.getFrameFormat_flinger == nullptr) {
      printf("Couldn't load getScreenformat_flinger! Error string: %s\n", dlerror());
      continue;
    }

    int ret = base.init_flinger();

    if (ret == -1) {
      printf("flinger method not supported by this device!\n");
      continue;
    }
    return;
  }
}

Grabber::~Grabber(){
	base.close_flinger();
}

FrameFormat& Grabber::getFrameFormat(){
	return base.getFrameFormat_flinger();
}

void* Grabber::lock(){
  return base.lockBuffer_flinger();
}

void Grabber::unlock(){
  return base.unlockBuffer_flinger();
}