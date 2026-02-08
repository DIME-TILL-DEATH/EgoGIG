PRJ_NAME=sd_multi_player

SDK_DIR=$(SRC_DIR)/../../SDK_embedded

PLL_CONFIG=0

FIRMWARE_VER=1.04.07

# set relocation offset for application code in flash aria
# for support bootloader SunSet
#FLASH_TEXT_SECTION_OFFSET=0x40000
#FLASH_ORIGIN=0x08040000


CPU_EXT_FLAGS=
EXT_DEFS=

EEPROM_16K_FLASH_SECTOR_A=use


# set a RAM vector table define
RAM_VEC_TABLE=0

USBD_DEFS= -DUSE_USB_FS \
           -DUSBD_VID=$(USBD_VID) \
           -DUSBD_PID=$(USBD_PID) \
           -DUSBD_MANUFACTURER_STRING=$(MANUFACTURER_STRING) \
           -DUSBD_PRODUCT_STRING=$(PROJECT_DESCRIPTION)

# add project specific hardware defines
HARDWARE_DEFS+= -DRAM_VEC_TABLE=$(RAM_VEC_TABLE) $(USBD_DEFS)

# set a FreeRTOS defines
TICK_RATE_HZ_DEFAULT=1000UL
MAX_PRIORITIES=2UL
MINIMAL_STACK_SIZE=64UL
#CCM_SRAM_POOL_SIZE="(64*1024UL)"
INTERNAL_SRAM_POOL_SIZE="(78*1024UL)"
MAX_TASK_NAME_LEN=16UL

USE_FREERTOS=1
#USE_REENTRANT=1
FREERTOS_DEFS= -DTICK_RATE_HZ_DEFAULT=$(TICK_RATE_HZ_DEFAULT)  \
               -DMAX_PRIORITIES=$(MAX_PRIORITIES)              \
               -DMINIMAL_STACK_SIZE=$(MINIMAL_STACK_SIZE)      \
               -DMAX_TASK_NAME_LEN=$(MAX_TASK_NAME_LEN)

# set tlsf max block size 17-128K 18-256K 19-512K 20-1M ....  
TLSF_FL_INDEX_MAX=17
TLSF_DEFS=-DTLSF_USE_LOCKS -D__USE_FREERTOS__ -DTLSF_FL_INDEX_MAX=$(TLSF_FL_INDEX_MAX)
APP_DEFS=-DFIRMWARE_VER="\"$(FIRMWARE_VER)"\"

#OPT_LTO=8
COMPILE_EXT_FLAGS=-Wno-errors -Wno-incompatible-pointer-types -Wno-implicit-function-declaration -Wno-odr -Wno-register -Wno-volatile -Wno-sign-compare -Wno-maybe-uninitialized -Wno-shift-negative-value  -Wno-builtin-declaration-mismatch -Wno-missing-field-initializers -Wno-sequence-point -Wno-unused-variable -Wno-char-subscripts

#-mlong-calls
OPT_EXT_FLAGS=-Wno-odr -fno-lto
#
#LINK_EXT_FLAGS=-Wl,-nostdlib


