CPPFLAGS+=-Wall -Wextra -Wpedantic
CPPFLAGS+=-Wwrite-strings -Wstack-usage=1024 -Wfloat-equal
CPPFLAGS+=-Waggregate-return -Winline

LDLIBS+=-lm

ASFLAGS += -W
CFLAGS += -O1 -masm=intel -fno-asynchronous-unwind-tables -std=c11

BIN=fdr
OBJS=fdr.o fibonacci.o

.PHONY: clean debug profile

$(BIN): $(OBJS)

debug: CFLAG+=-g
debug: $(BIN)

profile: CFLAGS+=-pg
profile: LDFLAGS+=-pg
profile: $(BIN)

clean:
	$(RM) $(OBJS) ($BIN)
