CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

TARGET = compiler


SOURCES = main.cpp \
          Tokenizer.cpp \
          Parser.cpp \
          ir.cpp \
          generator.cpp \
          CodeGen.cpp

HEADERS = main.h \
          Tokenizer.h \
          Parser.h \
          generator.h \
          CodeGen.h

TEST_SOURCES = tests/test_parser.cpp
TEST_TARGET = test_parser

OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET) $(TEST_TARGET) *.ir

test: $(TARGET)
	./$(TARGET) test_global.bcc --print-ir

unit-tests: $(TEST_SOURCES) $(filter-out main.o, $(OBJECTS))
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_SOURCES) $(filter-out main.cpp, $(SOURCES))
	./$(TEST_TARGET)

.PHONY: all clean test unit-tests
