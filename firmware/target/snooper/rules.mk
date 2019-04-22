LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

STM32_CHIP := stm32f072_xB

PLATFORM := stm32f0xx

GLOBAL_DEFINES += \
	ENABLE_UART1=1 \
	USE_USB_CLKSOURCE_CRSHSI48=1 \
	TARGET_HAS_DEBUG_LED=1

MODULE_SRCS += \
	$(LOCAL_DIR)/init.c \
	$(LOCAL_DIR)/usb.c

include make/module.mk
