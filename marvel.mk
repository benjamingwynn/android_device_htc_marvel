#
# Copyright (C) 2011 The CyanogenMod Project
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

## (1) First, the most specific values, i.e. the aspects that are specific to GSM

$(call inherit-product, build/target/product/full_base_telephony.mk)
$(call inherit-product, build/target/product/languages_full.mk)

$(call inherit-product, device/htc/marvel/marvel-vendor.mk)
$(call inherit-product, device/common/gps/gps_eu_supl.mk)

DEVICE_PACKAGE_OVERLAYS += device/htc/marvel/overlay

# dalvik heap config for devices with 512MB memory
$(call inherit-product, frameworks/native/build/phone-hdpi-512-dalvik-heap.mk)

PRODUCT_COPY_FILES += \
    device/htc/marvel/init.marvel.rc:root/init.marvel.rc \
    device/htc/marvel/ueventd.marvel.rc:root/ueventd.marvel.rc

# Default network type.
# 0 => WCDMA preferred.
PRODUCT_PROPERTY_OVERRIDES += \
    ro.telephony.default_network=0

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml

# USB
PRODUCT_COPY_FILES += \
    device/htc/marvel/init.msm7x27.usb.rc:root/init.msm7x27.usb.rc

# Media
PRODUCT_COPY_FILES += \
    device/htc/marvel/libaudio/audio_policy.conf:system/etc/audio_policy.conf \
    device/htc/marvel/media_codecs.xml:system/etc/media_codecs.xml \
    device/htc/marvel/media_profiles.xml:system/etc/media_profiles.xml

PRODUCT_PROPERTY_OVERRIDES += \
      media.a1026.nsForVoiceRec            = 0 \
      media.a1026.enableA1026              = 1

# Firmware
PRODUCT_COPY_FILES += \
    device/htc/marvel/firmware/bcm4329.hcd:system/etc/firmware/bcm4329.hcd \
    device/htc/marvel/firmware/fw_bcm4329.bin:system/etc/firmware/fw_bcm4329.bin \
    device/htc/marvel/firmware/fw_bcm4329_apsta.bin:system/etc/firmware/fw_bcm4329_apsta.bin

# Audio
PRODUCT_PACKAGES += \
    audio.a2dp.default \
    audio_policy.msm7x27 \
    audio.primary.msm7x27 \
    libtinyalsa \
    libaudioutils

### Audio policy
PRODUCT_COPY_FILES += \
    device/htc/marvel-common/libaudio/audio_policy.conf:system/etc/audio_policy.conf

# Video
PRODUCT_PACKAGES += \
    copybit.msm7x27 \
    gralloc.msm7x27 \
    hwcomposer.msm7x27 \
    libgenlock \
    libmemalloc \
    liboverlay \
    libqdutils \
    libtilerenderer

# Camera
PRODUCT_PACKAGES += \
    camera.msm7x27

# QCOM OMX
PRODUCT_PACKAGES += \
    libstagefrighthw \
    libOmxCore \
    libmm-omxcore

### Mount vold
PRODUCT_COPY_FILES += \
    device/htc/marvel-common/vold.fstab:system/etc/vold.fstab

# Lights and sensors
PRODUCT_PACKAGES += \
    sensors.msm7x27 \
    lights.marvel \
    lights.msm7x27

# Misc
PRODUCT_PACKAGES += \
    com.android.future.usb.accessory \
    Superuser

### Add system daemons
PRODUCT_PACKAGES += \
    rild

# Filesystem management tools
PRODUCT_PACKAGES += \
    make_ext4fs \
    setup_fs

# GPS
PRODUCT_PROPERTY_OVERRIDES += \
    ro.ril.def.agps.mode=2

PRODUCT_PACKAGES += \
    gps.marvel

# RIL
PRODUCT_PROPERTY_OVERRIDES += \
    rild.libpath=/system/lib/libhtc_ril.so \
    ro.ril.enable.dtm=1 \
    ro.ril.hsdpa.category=8 \
    ro.ril.hsupa.category=5 \
    ro.ril.disable.fd.plmn.prefix=23402,23410,23411 \
    ro.ril.def.agps.mode=2 \
    ro.ril.hsxpa=2 \
    ro.ril.gprsclass=12 \
    ro.telephony.ril.v3=signalstrength,skipbrokendatacall

# Interfaces
PRODUCT_PROPERTY_OVERRIDES += \
    mobiledata.interfaces=rmnet0,rmnet1,rmnet2,gprs,ppp0 \
    wifi.interface = wlan0 \
    wifi.supplicant_scan_interval=60

# Graphics
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=160 \
    ro.opengles.version=131072 \
    debug.sf.hw=1 \
    debug.enabletr=false \
    debug.qctwa.statusbar=1 \
    debug.qctwa.preservebuf=1 \
    debug.hwui.render_dirty_regions=false \
    com.qc.hardware=true \
    com.qc.hdmi_out=false

# Bluetooth
RODUCT_PROPERTY_OVERRIDES += \
   net.bt.name=Android

# Camera
PRODUCT_PROPERTY_OVERRIDES += \
    debug.camcorder.disablemeta=1 \
    ro.htc.camerahack=msm7k

# Keylayouts
PRODUCT_COPY_FILES += \
    device/htc/marvel/key/marvel-keypad.kl:system/usr/keylayout/marvel-keypad.kl \
    device/htc/marvel/init.marvel.rc:root/init.marvel.rc \
    device/htc/marvel/ueventd.marvel.rc:root/ueventd.marvel.rc \
    device/htc/marvel/key/marvel-keypad.kcm.bin:system/usr/keychars/marvel-keypad.kcm.bin \
    device/htc/marvel/key/h2w_headset.kl:system/usr/keylayout/h2w_headset.kl \
    device/htc/marvel/key/cy8c-touchscreen.kl:system/usr/keylayout/cy8c-touchscreen.kl

# For emmc phone storage
PRODUCT_PROPERTY_OVERRIDES += \
    ro.phone_storage=0

### Touchscreen
PRODUCT_COPY_FILES += \
    device/htc/marvel-common/idc/atmel-touchscreen.idc:system/usr/idc/atmel-touchscreen.idc \
    device/htc/marvel-common/idc/cy8c-touchscreen.idc:system/usr/idc/cy8c-touchscreen.idc

# Misc
PRODUCT_PROPERTY_OVERRIDES += \
    settings.display.autobacklight=1 \
    settings.display.brightness=143 \
    persist.service.mount.playsnd=0 \
    ro.com.google.locationfeatures=1 \
    ro.setupwizard.mode=OPTIONAL \
    ro.setupwizard.enable_bypass=1 \
    ro.media.dec.aud.wma.enabled=1 \
    ro.media.dec.vid.wmv.enabled=1 \
    dalvik.vm.dexopt-flags=m=y \
    ro.config.sync=yes \
    persist.sys.usb.config=mass_storage,adb \
    dalvik.vm.dexopt-data-only=1

# We have enough storage space to hold precise GC data
#PRODUCT_TAGS += dalvik.gc.type-precise

### Artwork
PRODUCT_LOCALES += mdpi

### Add additional packages
PRODUCT_PACKAGES += \
    Torch

PRODUCT_AAPT_CONFIG := normal mdpi
PRODUCT_AAPT_PREF_CONFIG := mdpi
