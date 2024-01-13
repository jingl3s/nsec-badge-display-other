
if [ ! -e lvgl ] ; then
  git clone -b release/v7 https://github.com/lvgl/lvgl.git
fi

if [ ! -e lv_drivers ] ; then
  git clone -b release/v7 https://github.com/lvgl/lv_drivers.git
fi

if [ ! -e lv_nsec_badge/debug.c ] ; then
  ln $PWD/../esp32/components/display/screens/debug.cpp lv_nsec_badge/debug.c -s
fi
if [ ! -e lv_nsec_badge/lv_utils.c ] ; then
  ln $PWD/../esp32/components/display/lv_utils.cpp lv_nsec_badge/lv_utils.c -s
fi
if [ ! -e lv_nsec_badge/score_teams.c ] ; then
  ln $PWD/../esp32/components/display/screens/score_teams.c lv_nsec_badge/score_teams.c -s
fi
if [ ! -e lv_nsec_badge/score_teams.h ] ; then
  ln $PWD/../esp32/components/display/screens/score_teams.h lv_nsec_badge/score_teams.h -s
fi
make
./demo