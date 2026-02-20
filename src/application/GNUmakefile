include $(SRC_DIR)/rules.mk

LIB_FLAGS=	-I./ \
		    -I$(SRC_DIR)/include \
		    -I$(STM32_PERIFERIAL_DRV_SRC_DIR)  \
		    -I$(STM32_CRT_SRC_DIR)	\
		    -I$(STM32_CMSIS_COMMON_SRC_DIR) \
        	-I$(STM32_CMSIS_SRC_DIR)	\
        	-I$(KLIBC_SRC_DIR) \
        	-I$(TLSF_SRC_DIR) \
		    -I$(RTCOUNTER_SRC_DIR) \
		    -I$(CONSOLE_SRC_DIR) \
		    -I$(CONSOLE_HANDLERS_SRC_DIR) \
		    -I$(READLINE_SRC_DIR) \
		    -I$(FREERTOS_KERNEL_SRC_DIR)/include \
		    -I$(FREERTOS_PORTABLE_SRC_DIR) \
		    -I$(MMGR_SRC_DIR) \
            -I$(KLEN_INLINED_SRC_DIR) \
            -I$(LIBOPENCM3_SRC_DIR) \
            -I$(LED_SRC_DIR) \
            -I$(HASH_SRC_DIR) \
            -I$(FATFS_SRC_DIR) \
            -I$(SRC_DIR)/application/dsp\
            -I$(SRC_DIR)/application/gui\
            -I$(SRC_DIR)/application/midi\
            -I$(SRC_DIR)/application/metronome\
			$(APP_DEFS) \
			$(HARDWARE_DEFS) \
			$(FREERTOS_DEFS) \
			$(MEM_POOLS_DEFS)

CXXFLAGS += $(LIB_FLAGS)
CFLAGS += $(LIB_FLAGS)

# сог поднять проект, но без сборки libop[encm3 - пока используем старый биннарь
LIBS =-lopencm3

LD_EXT_FLAGS=


CUSTOM_RM_FILES=$(OUT_DIR)/image.*




