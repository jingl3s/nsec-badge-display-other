echo "** idf.py build is not run automatically, ensure files are up to date **"

SERIAL_PORT=/dev/ttyUSB0

espsecure.py encrypt_flash_data --keyfile flash_encryption_key.bin --address 0x1000 --output build/bootloader/bootloader-encrypted.bin build/bootloader/bootloader.bin
espsecure.py encrypt_flash_data --keyfile flash_encryption_key.bin --address 0x10000 --output build/partition_table/partition-table-encrypted.bin build/partition_table/partition-table.bin
espsecure.py encrypt_flash_data --keyfile flash_encryption_key.bin --address 0x20000 --output build/nsec-badge-controller-screen-encrypted.bin build/nsec-badge-controller-screen.bin

python ../esp-idf/components/esptool_py/esptool/esptool.py -p $SERIAL_PORT -b 460800 --before default_reset --after no_reset --chip esp32 \
	 write_flash --force --flash_mode dio --flash_size keep --flash_freq 40m 0x1000 build/bootloader/bootloader-encrypted.bin 0x10000 build/partition_table/partition-table-encrypted.bin 0x20000 build/nsec-badge-controller-screen-encrypted.bin
