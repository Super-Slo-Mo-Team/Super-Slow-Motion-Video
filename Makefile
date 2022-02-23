TARGET	:= slowmotion
OBJS	:= flowVectorFrame.o flowVectorService.o slowMotionService.o main.o

CC 		:= g++
CFLAGS 	:= -Wall -std=c++14 -Wno-nullability-completeness
LDFLAGS	:= -L/usr/local/include -lzmq

all: $(TARGET)

# DEPS	:= $(patsubt %.o, %.d, $(OBJS))
# -include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)

# flowVectorFrame.o: flowVectorFrame.cpp flowVectorFrame.hpp
# 	$(CC) $(CFLAGS) -c flowVectorFrame.cpp

# flowVectorService.o: flowVectorService.cpp flowVectorService.hpp
# 	$(CC) $(CFLAGS) -c flowVectorService.cpp

# slowMotionService.o: slowMotionService.cpp slowMotionService.hpp
# 	$(CC) $(CFLAGS) -c slowMotionService.cpp

# main.o: main.cpp config.hpp
# 	$(CC) $(CFLAGS) -c main.cpp
