TARGET		= libdxl.a
OBJS		= serial.o dynamixel.o instruction_packet.o status_packet.o
SRCS		= $(OBJS:.o=.cpp)
INCLUDEDIRS	+= -I../include
LIBDIRS		+=
CFLAGS		= $(INCLUDEDIRS) -W -Wall -O2 - std=c++0x

CXX			= g++
AR			= ar

$(TARGET): $(OBJS)
	$(AR) rs $@ $^
	cp $(TARGET) ../lib
	rm $(TARGET)

.c.o:
	$(CXX) -c $< $(CFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)
	@echo "file deleted."

dep:
	gccmakedep $(SRCS)
