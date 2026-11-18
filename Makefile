CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -g
SRC      = src/Token.cpp src/Lexer.cpp src/AST.cpp src/Parser.cpp src/main.cpp
OBJ      = $(SRC:.cpp=.o)
TARGET   = citu

.PHONY: all clean test demo

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Automated tests: valid programs must be accepted (and match expected ASTs),
# invalid programs must be rejected.
test: $(TARGET)
	@sh tests/run_tests.sh

# Run the three sample programs and write out/<name>.tokens and out/<name>.ast
demo: $(TARGET)
	@mkdir -p out
	@for p in prog1 prog2 prog3; do \
	  echo "========== $$p =========="; \
	  ./$(TARGET) examples/$$p.citu -o out/$$p; echo; \
	done

clean:
	rm -f $(OBJ) $(TARGET)
	rm -rf out
