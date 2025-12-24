CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

TARGET = compiler


SOURCES = main.cpp \
          Tokenizer.cpp \
SRC_DIR = src
SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/Tokenizer.cpp \
          $(SRC_DIR)/Parser.cpp \
          $(SRC_DIR)/ir.cpp \
          $(SRC_DIR)/generator.cpp \
          $(SRC_DIR)/CodeGen.cpp
          Tokenizer.h \
          Parser.h \
          generator.h \
          CodeGen.h

TEST_SOURCES = tests/test_parser.cpp
TEST_TARGET = test_parser

OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

all: compiler
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)
compiler: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o compiler $(OBJECTS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	rm -f *.o $(TARGET) $(TEST_TARGET) *.ir

	rm -f $(SRC_DIR)/*.o compiler test_parser *.ir
	./$(TARGET) test_global.bcc --print-ir

	./compiler tests/test.b --print-ir
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_SOURCES) $(filter-out main.cpp, $(SOURCES))
	./$(TEST_TARGET)
	$(CXX) $(CXXFLAGS) -o test_parser $(TEST_SOURCES) $(filter-out $(SRC_DIR)/main.cpp, $(SOURCES))
	./test_parser
