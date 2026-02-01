CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

# WasmEdge configuration
WASMEDGE_INCLUDE = $(HOME)/.wasmedge/include
WASMEDGE_LIB_DIR = $(HOME)/.wasmedge/lib
WASMEDGE_LIBS = -L$(WASMEDGE_LIB_DIR) -lwasmedge -Wl,-rpath,$(WASMEDGE_LIB_DIR)

CXXFLAGS = -Wall -Wextra -std=c++17 -g -I$(WASMEDGE_INCLUDE) -Iinclude
LDFLAGS = $(WASMEDGE_LIBS)

SRC_DIR = src
INC_DIR = include
TARGET = compiler

SOURCES = $(SRC_DIR)/main.cpp \
		  $(SRC_DIR)/Tokenizer.cpp \
		  $(SRC_DIR)/Parser.cpp \
		  $(SRC_DIR)/ir.cpp \
		  $(SRC_DIR)/generator.cpp \
		  $(SRC_DIR)/target.cpp \
		  $(SRC_DIR)/codegen/x86_64_generator.cpp \
		  $(SRC_DIR)/codegen/arm_generator.cpp \
		  $(SRC_DIR)/codegen/wasm_generator.cpp \
		  $(SRC_DIR)/codegen/wasmedge_generator.cpp

HEADERS = $(INC_DIR)/main.h \
		  $(INC_DIR)/ir.h \
		  $(INC_DIR)/Tokenizer.h \
		  $(INC_DIR)/Parser.h \
		  $(INC_DIR)/target.h \
		  $(SRC_DIR)/codegen/x86_64_generator.h \
		  $(SRC_DIR)/codegen/arm_generator.h \
		  $(SRC_DIR)/codegen/wasm_generator.h \
		  $(INC_DIR)/generator.h

OBJECTS = $(SOURCES:.cpp=.o)

TEST_SOURCES = tests/test_parser.cpp
TEST_TARGET = test_parser

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(SRC_DIR)/codegen/*.o $(TARGET) $(TEST_TARGET) *.ir *.o test_x86

unit-tests: $(TEST_SOURCES) $(filter-out $(SRC_DIR)/main.o, $(OBJECTS))
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_SOURCES) $(filter-out $(SRC_DIR)/main.cpp, $(SOURCES))
	./$(TEST_TARGET)

.PHONY: all clean test unit-tests
