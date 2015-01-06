

CFLAGS += -I../inc 

SRCS = sgn_list.c sgn_queue.c sgn_timer.c sgn_string.c
OBJS = $(SRCS:.c=.o)

TARGET = libsgnlib.a

all: $(OBJS)
	rm -f $(TARGET)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJS)
#	rm $(TARGET)
	touch $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
