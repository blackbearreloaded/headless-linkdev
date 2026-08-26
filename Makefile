PS5_HOST ?= ps5
PS5_PORT ?= 9021
TARGET := headless-linkdev.elf
TEST_TARGET := headless_linkdev_test
HOST_CC ?= cc
CLANG_FORMAT ?= clang-format

ifndef PS5_PAYLOAD_SDK
$(error PS5_PAYLOAD_SDK is undefined)
endif

include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk

CFLAGS := -Wall -Wextra -Werror -O2
LDLIBS := -lSceRegMgr -lSceUserService -lSceRemoteplay

.PHONY: all clean format format-check send test

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): main.c
	$(HOST_CC) -DSELF_TEST -Wall -Wextra -Werror -O2 -o $@ $<

format:
	$(CLANG_FORMAT) -i main.c

format-check:
	$(CLANG_FORMAT) --dry-run --Werror main.c

send: $(TARGET)
	socat -t 99999999 - TCP:$(PS5_HOST):$(PS5_PORT) < $(TARGET)
