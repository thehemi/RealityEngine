MAIN=extgen

CC=g++

CFLAGS= -DUNIX -Wall -g

INCLUDES= -I..

SRCS =  extgen.cpp

OBJS = $(SRCS:.cpp=.o)

$(MAIN):  $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(MAIN) $(LFLAGS)
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o *~ $(MAIN)
