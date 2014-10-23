LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=	\
	actions.c \
	parser.c \
	autotest.c

LOCAL_MODULE:= autotest

CFLAGS += -g

include $(BUILD_EXECUTABLE)
