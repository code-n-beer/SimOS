SRC = $(wildcard src/*.cpp)

OBJDIR = build
OBJ = $(patsubst src/%.cpp, $(OBJDIR)/%.o, $(SRC))

TARGET = $(OBJDIR)/simotest

CXXFLAGS += -Wall -Wextra -O3 -fconcepts -std=gnu++17 -I./include

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)

build/%.o: src/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(TARGET) $(OBJDIR)

run: $(TARGET)
	$(TARGET)

$(OBJDIR):
	@mkdir -p build/