source ../esp-idf/export.sh
idf.py build
if [ $? -eq 0 ] ; then
  ./secure_reflash.sh
  if [ $? -eq 0 ] ; then
    python ../reset_esp32_and_listen.py
  fi
fi
