BFB = ../braille-framebuffer
SRCS = data.c io.c text.c
OBJS = data.o io.o text.o
DEPENDS = Makefile.deps

# CC = clang
CFLAGS = -O0 -g -std=c11 -I ../braille-framebuffer
LDFLAGS = -lm

plot: plot.o $(OBJS) $(BFB)/bfb.o $(BFB)/unicode.o

generate: generate.o

clean:
	rm -f *.o $(DEPENDS)

depend:
	$(CC) -MM -MF - $(CFLAGS) $(SRCS) > $(DEPENDS)

-include $(DEPENDS)
