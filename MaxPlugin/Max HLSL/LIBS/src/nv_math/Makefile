ifeq ("$(shell uname)", "Linux") 
  OS="Linux"
endif

ifeq ("$(shell uname)", "Darwin")
  OS="MacOSX"
endif

ifndef OS
  $(error Unsupported operating system)
endif

MAIN=../../lib/libnvmath.a
CC=g++
CFLAGS= -Wall -g
INCLUDES= -I../../inc
SRCS = nv_algebra.cpp

ifeq ($(OS), "Linux")
  CFLAGS += -DUNIX
endif

ifeq ($(OS), "MacOSX")
  CFLAGS += -DMACOS
endif

OBJS = $(SRCS:.cpp=.o)

$(MAIN):  $(OBJS)
	$(AR) -rs $(MAIN) $(OBJS)
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o *~ $(MAIN)
