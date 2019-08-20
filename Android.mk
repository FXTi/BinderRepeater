
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := repeater
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := repeater.cpp
LOCAL_SHARED_LIBRARIES := libutils libcutils libbinder liblog
LOCAL_C_INCLUDES += frameworks/native/include system/core/include
include $(BUILD_EXECUTABLE)

