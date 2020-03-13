
ifeq ($(V),)
Q=@
else
Q=
endif

CFLAGS += -D__ENABLE_HARDCODE_LOGIN___ -D__ENABLE_LOGIN__

ifeq ($(shell uname),Darwin)
OS=MAC
else
OS=LINUX
endif

ifeq ($(OS),MAC)
CC      := $(Q)clang
OBJDUMP := $(Q)symbols
else
#CROSS   = arm-none-eabi-
CC      := $(Q)$(CROSS)$(CC)
OBJDUMP := $(Q)$(CROSS)objdump -x
endif

.PHONY: sizing

all: ut_cli sizing

clean:
	$(Q)rm -f *.o cli

CFLAGS += -g -Os -Wall

%.o: %.c io.h
	$(CC) -c -o $@ $< $(CFLAGS)

ut_cli: io.o knock.o ut_cli.o
	$(CC) -o $@ $^ $(CFLAGS)

sizing:
ifeq ($(OS),MAC)
	$(OBJDUMP) cli|grep FUNC|grep cli_|awk '{printf("%5d %s\n", $$3, $$4);}'|sed s/\)//
	$(OBJDUMP) cli|grep FUNC|grep cli_|awk '{printf(" + %d", $$3);}'|sed s/\)//g|xargs echo 0 | bc
else
	$(OBJDUMP) cli|grep text|grep cli_|awk '{s = ("0x" $$5) + 0; printf("%5d %s\n", s, $$6);}'
	$(OBJDUMP) cli|grep text|grep cli_|awk '{s = ("0x" $$5) + 0; printf(" + %d\n", s);}'| xargs echo 0 | bc
endif

