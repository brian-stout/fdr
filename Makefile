CPPFLAGS+=-Wall -Wextra -Wpedantic
CPPFLAGS+=-Wwrite-strings -Wstack-usage=1024 -Wfloat-equal
CPPFLAGS+=-Waggregate-return -Winline

LDLIBS+=-lm -lssl -lcrypto

CFLAGS += -std=c11

BIN=fdr
OBJS=fdr.o fibonacci.o roman_numeral.o

.PHONY: clean debug profile

$(BIN): $(OBJS)

debug: CFLAG+=-g
debug: $(BIN)

profile: CFLAGS+=-pg
profile: LDFLAGS+=-pg
profile: $(BIN)

clean:
	$(RM) $(OBJS) $(BIN)
