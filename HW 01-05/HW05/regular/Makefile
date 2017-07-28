uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

-include modules.mk

HW=hw05
TARGET=main

CFLAGS+=-std=c99 -g -pedantic -Wall -Werror
LDFLAGS+=-lm
 
SRC:=$(TARGET).c $(SRC)
SRC+=$(wildcard *.c)
 
OBJS:=$(patsubst %.c,%.o,$(call uniq,$(SRC)))
 
bin: $(TARGET)

$(OBJS): %.o: %.c
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@
#vytvarim object files z c filers 
# < vezme jeden vstup 
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@ 
#get the target file from all the files
zip:
	zip ${HW}-brute.zip $(TARGET).c

clean:
	$(RM) $(OBJS) $(TARGET)
	$(RM) -f ${HW}-brute.zip

.PHONY: clean zip
#CC compilator
#CFLAGS - compiler flags
