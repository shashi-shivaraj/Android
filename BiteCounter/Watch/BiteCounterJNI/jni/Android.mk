# 
#  FILE NAME	: Android.mk
#
#  DESCRIPTION  : Android Make file
#
#
#  DATE	                NAME	             REASON
#  2nd July,2018		Shashi Shivaraju	 BiteCounter

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := BiteCounterAlgo
$(warning $(TARGET_ARCH_ABI))
$(warning $(LOCAL_PATH))
LOCAL_SRC_FILES := ./../../BiteCounterAlgo/libs/$(TARGET_ARCH_ABI)/libBiteCounterAlgo.so
LOCAL_EXPORT_C_INCLUDES := ./../../BiteCounterAlgo/jni/inc
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libBiteCounterJNI

LOCAL_CFLAGS +=  -D__ANDROID -DCOMPILER=ANDROID_GCC_COMPILER -DPLATFORM=ANDROID_ARMVI

DEBUG_BUILD							=	1
RELEASE_BUILD						=	0

ifeq ($(DEBUG_BUILD),1)
LOCAL_CFLAGS += -Wall -O1 -Wunused-variable -Wunused-function -DLOG_LEVEL=LOG_VERBOSE
CC_FLAGS += -Wall -O1
endif

ifeq ($(RELEASE_BUILD),1)
LOCAL_CFLAGS += -w -O3
CC_FLAGS +=  -w -O3
endif

LOCAL_LDFLAGS += -Wl,--export-dynamic -ldl

LOCAL_SRC_FILES := BiteCounterJNI.c

LOCAL_LDLIBS += -lm -llog
	
LOCAL_SHARED_LIBRARIES := BiteCounterAlgo
 
LOCAL_C_INCLUDES += $(JNI_H_INCLUDE)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../BiteCounterAlgo/jni/inc \
							
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
