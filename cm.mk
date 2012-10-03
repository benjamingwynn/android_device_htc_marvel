PRODUCT_RELEASE_NAME := CM

# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/gsm.mk)
$(call inherit-product, vendor/cm/config/common_full_phone.mk)

# Inherit device configuration
$(call inherit-product, device/htc/marvel/device_marvel.mk)

## Device identifier. This must come after all inclusions
PRODUCT_DEVICE := marvel
PRODUCT_NAME := cm_marvel
PRODUCT_BRAND := htc_europe
PRODUCT_MODEL := Wildfire S
PRODUCT_MANUFACTURER := HTC

PRODUCT_VERSION_DEVICE_SPECIFIC := -Build-$(shell date -u +%Y%m%d)

CM_BUILDTYPE := UNOFFICIAL

BUILD_ID := benjamingwynn$(shell date -u +%Y%m%d)

# Set build fingerprint / ID / Product Name ect.
PRODUCT_BUILD_PROP_OVERRIDES += PRODUCT_NAME=htc_marvel BUILD_ID=$(BUILD_ID) BUILD_DISPLAY_ID=$(BUILD_ID) BUILD_FINGERPRINT=cyanogenmod/htc_marvel/marvel:4.1.1/$(BUILD_ID)/0.1:user/release-keys PRIVATE_BUILD_DESC="benjamingwynncm10"
