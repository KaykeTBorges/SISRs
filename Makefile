CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -g

TARGET = leitor
SOURCES = main.cpp data.cpp solution.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean