# ------------------- #
# makefile for #
# ------------------- #

LINUX := $(shell uname -s | grep -c Linux)

#

# ------------------- #
#       LINUX         #
#-------------------- #
ifneq ($(LINUX), 0)
EXT=pd_linux
LIBS += -lpthread
LDFLAGS =  -export_dynamic -shared -L/usr/local/lib
LD = ld
LIBS = -lpthread -lc -lm
endif

NAME=counter
CSYM=counter
PDPATH=/etc/pd-extended/PD
CC = gcc
CFLAGS = -O2 -Wall -funroll-loops -fomit-frame-pointer -fPIC -DPD \
	-W -Wshadow -Wstrict-prototypes \
	-Wno-unused -Wno-parentheses -Wno-switch

INCLUDE = -I. -I$(PDPATH)/src 


SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
#LIBS = -lpd -luvsrp -lpthreadGCE -lws2_32 -lm

$(NAME).$(EXT): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)
	strip --strip-unneeded $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $?

$(LIB): $(OBJ)
	$(AR) r $@ $?
	$(AR) s $@

clean:
	rm -f *.o *.*~

install: $(NAME).$(EXT)
	if ! test -d  ../lib; then mkdir  ../lib;fi
	cp $(NAME).$(EXT) ../lib
