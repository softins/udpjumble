CC = gcc
INCLUDES =
#LIBS = -lpthread
SRCS = udpjumble.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
PROG = udpjumble

CFLAGS = -Wall -Werror -g -O2 -MMD -MP $(INCLUDES) -D_GNU_SOURCE -D_REENTRANT -D_THREAD_SAFE
#CFLAGS = -Wall -g -O2 -MMD -MP $(INCLUDES) -D_GNU_SOURCE -D_REENTRANT -D_THREAD_SAFE -DUSE_MYSQL
#LFLAGS = -Wl,-Map,$(PROG).map -Wl,--cref

all: $(PROG)

-include $(DEPS)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) -o $(PROG) $(OBJS) $(LIBS)

clean:
	rm -f $(PROG) $(PROG).map $(OBJS) $(DEPS)
