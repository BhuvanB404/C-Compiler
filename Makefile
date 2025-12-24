CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

SRC_DIR = src
INC_DIR = lib
TARGET = compiler

SOURCES = $(SRC_DIR)/main.cpp \
		  $(SRC_DIR)/Tokenizer.cpp \
		  $(SRC_DIR)/Parser.cpp \
		  $(SRC_DIR)/ir.cpp \
		  $(SRC_DIR)/generator.cpp \
		  $(SRC_DIR)/CodeGen.cpp

HEADERS = $(INC_DIR)/main.h \
		  $(INC_DIR)/Tokenizer.h \
		  $(INC_DIR)/Parser.h \
		  $(INC_DIR)/generator.h \
		  $(INC_DIR)/CodeGen.h

OBJECTS = $(SOURCES:.cpp=.o)

TEST_SOURCES = tests/test_parser.cpp
TEST_TARGET = test_parser

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -o $(TARGET) $(OBJECTS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET) $(TEST_TARGET) *.ir

test: $(TARGET)
	./$(TARGET) tests/test.b --print-ir

unit-tests: $(TEST_SOURCES) $(filter-out $(SRC_DIR)/main.o, $(OBJECTS))
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -o $(TEST_TARGET) $(TEST_SOURCES) $(filter-out $(SRC_DIR)/main.cpp, $(SOURCES))
	./$(TEST_TARGET)

.PHONY: all clean test unit-tests
