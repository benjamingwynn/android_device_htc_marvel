# Copyright (C) 2010 The Android Open Source Project
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

# Goo.im GooManager
PRODUCT_COPY_FILES += \
    device/htc/marvel/prebuilt/propietary/app/GooManager.apk:/system/app/GooManager.apk

# Prebuilt libraries that are needed to build open-source libraries
PRODUCT_COPY_FILES += \
    device/htc/marvel/prebuilt/proprietary/libcamera.so:obj/lib/libcamera.so

# Audio and WPDB
PRODUCT_COPY_FILES += \
    device/htc/marvel/prebuilt/proprietary/etc/AudioFilter.csv:/system/etc/AudioFilter.csv \
    device/htc/marvel/prebuilt/proprietary/etc/AudioPara4.csv:/system/etc/AudioPara4.csv \
    device/htc/marvel/prebuilt/proprietary/etc/AudioPara4_WB.csv:/system/etc/AudioPara4_WB.csv \
    device/htc/marvel/prebuilt/proprietary/etc/AudioPreProcess.csv:/system/etc/AudioPreProcess.csv \
    device/htc/marvel/prebuilt/proprietary/etc/WPDB.zip:/system/etc/WPDB.zip

# G-Sensors and Compass
PRODUCT_COPY_FILES += \
    device/htc/marvel/prebuilt/proprietary/bin/akmd:/system/bin/akmd

# RIL
PRODUCT_COPY_FILES += \
    device/htc/marvel/prebuilt/proprietary/lib/libaudioeq.so:/system/lib/libaudioeq.so \
    device/htc/marvel/prebuilt/proprietary/lib/libhtc_acoustic.so:/system/lib/libhtc_acoustic.so \
    device/htc/marvel/prebuilt/proprietary/lib/libhtc_ril.so:/system/lib/libhtc_ril.so

# OMX
PRODUCT_COPY_FILES += \
    device/htc/marvel/prebuilt/proprietary/lib/libOmxH264Dec.so:/system/lib/libOmxH264Dec.so \
    device/htc/marvel/prebuilt/proprietary/lib/libOmxMpeg4Dec.so:/system/lib/libOmxMpeg4Dec.so \
    device/htc/marvel/prebuilt/proprietary/lib/libOmxVidEnc.so:/system/lib/libOmxVidEnc.so \
    device/htc/marvel/prebuilt/proprietary/lib/libmm-adspsvc.so:/system/lib/libmm-adspsvc.so

PRODUCT_COPY_FILES += \
    device/htc/marvel/prebuilt/proprietary/libcamera.so:/system/lib/libcamera.so \
    device/htc/marvel/prebuilt/proprietary/liboemcamera.so:/system/lib/liboemcamera.so

# Adreno200 firmware blobs
PRODUCT_COPY_FILES += \
    device/htc/marvel/prebuilt/proprietary/etc/firmware/a225_pfp.fw:system/etc/firmware/a225_pfp.fw \
    device/htc/marvel/prebuilt/proprietary/etc/firmware/a225_pm4.fw:system/etc/firmware/a225_pm4.fw \
    device/htc/marvel/prebuilt/proprietary/etc/firmware/a225p5_pm4.fw:system/etc/firmware/a225p5_pm4.fw \
    device/htc/marvel/prebuilt/proprietary/etc/firmware/a300_pfp.fw:system/etc/firmware/a300_pfp.fw \
    device/htc/marvel/prebuilt/proprietary/etc/firmware/a300_pm4.fw:system/etc/firmware/a300_pm4.fw \
    device/htc/marvel/prebuilt/proprietary/etc/firmware/leia_pfp_470.fw:system/etc/firmware/leia_pfp_470.fw \
    device/htc/marvel/prebuilt/proprietary/etc/firmware/leia_pm4_470.fw:system/etc/firmware/leia_pm4_470.fw \
    device/htc/marvel/prebuilt/proprietary/etc/firmware/yamato_pfp.fw:system/etc/firmware/yamato_pfp.fw \
    device/htc/marvel/prebuilt/proprietary/etc/firmware/yamato_pm4.fw:system/etc/firmware/yamato_pm4.fw

# Adreno200 graphic blobs for ARMv6
PRODUCT_COPY_FILES += \
    device/htc/marvel/prebuilt/proprietary/lib/libC2D2.so:/system/lib/libC2D2.so \
    device/htc/marvel/prebuilt/proprietary/lib/libOpenVG.so:/system/lib/libOpenVG.so \
    device/htc/marvel/prebuilt/proprietary/lib/libgsl.so:/system/lib/libgsl.so \
    device/htc/marvel/prebuilt/proprietary/lib/libsc-a2xx.so:/system/lib/libsc-a2xx.so \
    device/htc/marvel/prebuilt/proprietary/lib/egl/eglsubAndroid.so:/system/lib/egl/eglsubAndroid.so \
    device/htc/marvel/prebuilt/proprietary/lib/egl/libEGL_adreno200.so:/system/lib/egl/libEGL_adreno200.so \
    device/htc/marvel/prebuilt/proprietary/lib/egl/libGLES_android.so:/system/lib/egl/libGLES_android.so \
    device/htc/marvel/prebuilt/proprietary/lib/egl/libGLESv1_CM_adreno200.so:/system/lib/egl/libGLESv1_CM_adreno200.so \
    device/htc/marvel/prebuilt/proprietary/lib/egl/libGLESv2_adreno200.so:/system/lib/egl/libGLESv2_adreno200.so \
    device/htc/marvel/prebuilt/proprietary/lib/egl/libq3dtools_adreno200.so:/system/lib/egl/libq3dtools_adreno200.so \
