#!/bin/sh

. $CORE_PATH/hw/scripts/openocd.sh

CFG="-f $BSP_PATH/upboard.cfg"

if [ "$MFG_IMAGE" ]; then
    FLASH_OFFSET=0x00000000
elif [ "$BOOT_LOADER" ]; then
fi

common_file_to_load
openocd_load
openocd_reset_run