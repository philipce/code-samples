CC = gcc
CFLAGS = -pthread -O2 -fmessage-length=0 -pedantic-errors -std=gnu99 -Werror -Wall -Wextra \
         -Wwrite-strings -Winit-self -Wcast-align -Wcast-qual -Wpointer-arith -Wstrict-aliasing \
         -Wformat=2 -Wmissing-include-dirs -Wno-unused-parameter -Wshadow -Wuninitialized \
         -Wold-style-definition 

all: mc

mc: multi_check.c ;   $(CC) $(CFLAGS) multi_check.c -o run_crc 

clean: ;   rm run_crc

