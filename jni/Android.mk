# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

FFMPEG_SOURCE_PATH := $(LOCAL_PATH)/ffmpeg
FFMPEG_INSTALL_PATH := $(FFMPEG_SOURCE_PATH)
FFMPEG_LIB_PATH := $(FFMPEG_INSTALL_PATH)/$(TARGET_ARCH_ABI)/lib
FFMPEG_INCLUDE_PATH := $(FFMPEG_INSTALL_PATH)/$(TARGET_ARCH_ABI)/include

#ffmpeg_build := $(shell $(FFMPEG_SOURCE_PATH)/android_ffmpeg.sh -s $(FFMPEG_SOURCE_PATH) -u $(TARGET_ARCH_ABI) -p $(TARGET_PLATFORM) -i $(FFMPEG_INSTALL_PATH))
########################################
###############LIBRARIES################
########################################
include $(CLEAR_VARS)
LOCAL_LDLIBS :=
LOCAL_MODULE := avutil
LOCAL_SRC_FILES := $(FFMPEG_LIB_PATH)/libavutil.a
LOCAL_EXPORT_C_INCLUDES := $(FFMPEG_INCLUDE_PATH)
LOCAL_SHARED_LIBRARIES :=
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS := avutil
LOCAL_MODULE := swresample
LOCAL_SRC_FILES := $(FFMPEG_LIB_PATH)/libswresample.a
LOCAL_EXPORT_C_INCLUDES := $(FFMPEG_INCLUDE_PATH)
LOCAL_SHARED_LIBRARIES :=
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS := avutil
LOCAL_MODULE := swscale 
LOCAL_SRC_FILES := $(FFMPEG_LIB_PATH)/libswscale.a
LOCAL_EXPORT_C_INCLUDES := $(FFMPEG_INCLUDE_PATH)
LOCAL_SHARED_LIBRARIES :=
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS := avutil swresample
LOCAL_MODULE := avcodec 
LOCAL_SRC_FILES := $(FFMPEG_LIB_PATH)/libavcodec.a
LOCAL_EXPORT_C_INCLUDES := $(FFMPEG_INCLUDE_PATH)
LOCAL_SHARED_LIBRARIES :=
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS := avutil swresample avcodec
LOCAL_MODULE := avformat 
LOCAL_SRC_FILES := $(FFMPEG_LIB_PATH)/libavformat.a
LOCAL_EXPORT_C_INCLUDES := $(FFMPEG_INCLUDE_PATH)
LOCAL_SHARED_LIBRARIES :=
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS := avutil swresample avcodec avfilter
LOCAL_MODULE := avfilter 
LOCAL_SRC_FILES := $(FFMPEG_LIB_PATH)/libavfilter.a
LOCAL_EXPORT_C_INCLUDES := $(FFMPEG_INCLUDE_PATH)
LOCAL_SHARED_LIBRARIES :=
include $(PREBUILT_STATIC_LIBRARY)

#######################################
############LOCAL MODULE###############
#######################################

include $(CLEAR_VARS)

LOCAL_CPPFLAGS += -std=c++11

ifeq ( $(TARGET_ARCH_ABI), arm64-v8a )
	LOCAL_CPPFLAGS += -D_LP64_
else
	LOCAL_CPPFLAGS += -D_LP32_
endif

LOCAL_MODULE := vnc-ffmpeg

LOCAl_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(FFMPEG_INCLUDE_PATH)

LOCAL_STATIC_LIBRARIES := avfilter avformat avcodec swscale swresample avutil

LOCAL_LDLIBS := -lz

LOCAL_SRC_FILES := \
			frame.cpp \
			bufferQueue.cpp \
			packetPool.cpp \
			encoder.cpp \
			grabber.cpp \
			server.cpp \
			vncServer.cpp \
			Main.cpp

include $(BUILD_EXECUTABLE)
