#
# Makefile
#
CC ?= gcc
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}

WARNINGS ?= -Wall \
						-Wshadow -Wmaybe-uninitialized -Wmissing-prototypes -Wno-discarded-qualifiers \
						-Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized \
						-Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default  \
					  -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated  \
						-Wempty-body -Wshift-negative-value -Wstack-usage=2048 \
            -Wtype-limits -Wsizeof-pointer-memaccess -Wpointer-arith
            
CFLAGS ?= -O0 -g -I$(LVGL_DIR)/ $(WARNINGS) -D LV_USE_USER_DATA_FREE=0 -D USE_DRM=0 -D SIMULATOR=1
LDFLAGS ?= -lSDL2 -lm -g
BIN = demo

ESP32_DISPLAY_DIR := $(LVGL_DIR)/../esp32/components/display

#Collect the files to compile
MAINSRC = ./main.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk
# include $(LVGL_DIR)/lv_examples/lv_examples.mk
include $(LVGL_DIR)/lv_nsec_badge/lv_nsec_badge.mk

CSRCS +=$(LVGL_DIR)/mouse_cursor_icon.c

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES

all: default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
    
default: $(AOBJS) $(COBJS) $(MAINOBJ)
	@$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)

clean:
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)

# Recreate all nsec badge symlinks before any compilation starts.
# Uses ln -sf to overwrite broken or stale symlinks from old worktrees.
.PHONY: symlinks
symlinks:
	@ln -sf $(abspath $(ESP32_DISPLAY_DIR)/screens/debug.cpp) lv_nsec_badge/debug.c
	@ln -sf $(abspath $(ESP32_DISPLAY_DIR)/lv_utils.cpp) lv_nsec_badge/lv_utils.c
	@ln -sf $(abspath $(ESP32_DISPLAY_DIR)/screens/score_teams.c) lv_nsec_badge/score_teams.c
	@ln -sf $(abspath $(ESP32_DISPLAY_DIR)/screens/score_teams.h) lv_nsec_badge/score_teams.h
	@ln -sf $(abspath $(ESP32_DISPLAY_DIR)/screens/image_viewer.cpp) lv_nsec_badge/image_viewer.c
	@ln -sf $(abspath $(ESP32_DISPLAY_DIR)/screens/image_viewer.h) lv_nsec_badge/image_viewer.h

# Order-only prerequisite: symlinks must exist before compiling any object file.
$(COBJS) $(MAINOBJ): | symlinks

