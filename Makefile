CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -laeron -laeron_client -laeron_archive

TARGET = calculator
SRC = calculator.cpp
HEADERS = calculator_functions.h calculator_queue.h

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean 