# 
#  FILE NAME	: Android.mk
#
#  DESCRIPTION  : Android Make file
#
#  DATE	                NAME	             REASON
#  2nd July,2018		Shashi Shivaraju	 BiteCounter

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= libBiteCounterAlgo


#comment below line to disable logs
#LOCAL_CFLAGS    := -DCONFIG_ALL_LOGS
#enable below line to enable smoothing of sensor data
LOCAL_CFLAGS    += -DCONFIG_SMOOTH_DATA
#enable below line to enable file based analysis
#LOCAL_CFLAGS    += -DCONFIG_FILE_METHOD

LOCAL_CFLAGS	+= -D__ANDROID -DCOMPILER=ANDROID_GCC_COMPILER -DPLATFORM=ANDROID_ARMVI
LOCAL_LDFLAGS	+= --no-warn-shared-textrel

DEBUG_BUILD							=	1
RELEASE_BUILD						=	0

ifeq ($(DEBUG_BUILD),1)
LOCAL_CFLAGS += -Wall -O1 -Wunused-variable -Wunused-function
CC_FLAGS += -Wall -O1
endif

ifeq ($(RELEASE_BUILD),1)
LOCAL_CFLAGS += -w -O3
CC_FLAGS +=  -w -O3
endif

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES += src/BiteCount_Algo.c\
LOCAL_SRC_FILES += src/BiteCount_Sensor.c\
					

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc \
LOCAL_C_INCLUDES + = $(LOCAL_PATH)/../../frameworks/native/include \
				 
#SYS_LIBS := 
#ALL_LIBS_A := 

LOCAL_LDLIBS += -landroid

LOCAL_LDFLAGS    := -llog 
#-Wl,--start-group $(ALL_LIBS_A) $(SYS_LIBS) -Wl,--end-group

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
