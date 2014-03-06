TARGET		= DataCheck
OBJS		= DataCheck.o
SRCS		= $(OBJS:.o=.cpp)
INCLUDEDIRS	+= -I../../include
LIBDIRS		+= -L../../lib
CXXFLAGS	= $(INCLUDEDIRS) -W -Wall -O2 -std=c++0x
LDFLAGS		= $(LIBDIRS) -lm -ldxl

CXX			= g++

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
	
.c.o:
	$(CXX) -c $< $(CXXFLAGS)
	
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "file deleted."

dep:
	gccmakedep $(SRCS)

