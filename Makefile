TARGET	:= slowmotion
OBJS	:= flowVectorFrame.o flowVectorService.o slowMotionService.o videoProcessor.o main.o

CC 		:= g++
CFLAGS 	:= -Wall -std=c++17 -Wno-nullability-completeness
LDFLAGS	:= -lzmq -lboost_serialization

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)
