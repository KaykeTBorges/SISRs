CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O3 -march=native -flto
CPPFLAGS = -Iinclude

TARGET = leitor
SOURCES = $(wildcard src/*.cpp)
HEADERS = $(wildcard include/*.h)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean