# Extension handling for Windows vs Linux/Mac
ifeq ($(OS),Windows_NT)
    EXT = .exe
else
    EXT =
endif

CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall

SRCS   = main.cpp board.cpp movegen.cpp eval.cpp search.cpp uci.cpp
OBJS   = $(SRCS:.cpp=.o)
TARGET = engine$(EXT)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
