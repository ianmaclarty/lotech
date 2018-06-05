LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := liblt-prebuilt
LOCAL_SRC_FILES := ../../../android/liblt.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libpng-prebuilt
LOCAL_SRC_FILES := ../../../android/libpng.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libz-prebuilt
LOCAL_SRC_FILES := ../../../android/libz.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := liblua-prebuilt
LOCAL_SRC_FILES := ../../../android/liblua.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libbox2d-prebuilt
LOCAL_SRC_FILES := ../../../android/libbox2d.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libvorbis-prebuilt
LOCAL_SRC_FILES := ../../../android/libvorbis.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libft2-prebuilt
LOCAL_SRC_FILES := ../../../android/libft2.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libopenal-prebuilt
LOCAL_SRC_FILES := ../../../android/libopenal.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := ltandroid
LOCAL_SRC_FILES := ltandroid.cpp
#LOCAL_STATIC_LIBRARIES := android_native_app_glue liblt-prebuilt libpng-prebuilt libz-prebuilt liblua-prebuilt libopenal-prebuilt libbox2d-prebuilt
LOCAL_STATIC_LIBRARIES := android_native_app_glue liblt-prebuilt libpng-prebuilt libz-prebuilt liblua-prebuilt libbox2d-prebuilt libvorbis-prebuilt libft2-prebuilt libopenal-prebuilt
LOCAL_C_INCLUDES := ../../../android/include
LOCAL_CFLAGS := -DLTDEVMODE -DLTANDROID -DLTDEPTHBUF
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv1_CM -lOpenSLES

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
