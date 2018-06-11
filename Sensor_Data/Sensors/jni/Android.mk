# 
#  FILE NAME	: Android.mk
#
#  DESCRIPTION  : Android Make file
#
#
#  DATE	                NAME	             REASON
#  23rd May,2018		Shashi Shivaraju	 AOSP

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= libSensors

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

LOCAL_SRC_FILES += src/sensors.c\
					

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc \
LOCAL_C_INCLUDES + = $(LOCAL_PATH)/../../frameworks/native/include \
				 
#DEP_LIBS :=  

#LOCAL_SHARED_LIBRARIES := libandroid
#LOCAL_SHARED_LIBRARIES := libcutils   libui  libbinder \
#			   			  libdl   libandroid_runtime \
#			   			  libgui

#LOCAL_STATIC_LIBRARIES :=  $(DEP_LIBS)


#ALL_LIBS_A := $(addsuffix .a, $(ALL_LIBS))
#ALL_LIBS_A += $(addsuffix .so, $(ALL_SHARED_LIBS))

SYS_LIBS := 
ALL_LIBS_A := 

LOCAL_LDLIBS += -landroid

LOCAL_LDFLAGS    := -llog -Wl,--start-group $(ALL_LIBS_A) $(SYS_LIBS) -Wl,--end-group

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
