echo "** idf.py build is not run automatically, ensure files are up to date **"

SERIAL_PORT=/dev/ttyUSB0

esptool.py -p $SERIAL_PORT -b 460800 --before default_reset --after no_reset --chip esp32 \
	 write_flash --force --flash_mode dio --flash_size keep --flash_freq 40m \
	 0x1000 ./bootloader-encrypted.bin \
	 0x10000 ./partition-table-encrypted.bin \
	 0x20000 ./nsec-badge-controller-screen-encrypted.bin
