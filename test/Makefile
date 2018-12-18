SRC = $(wildcard src/*.cpp)

OBJDIR = build
OBJ = $(patsubst src/%.cpp, $(OBJDIR)/%.o, $(SRC))

TARGET = $(OBJDIR)/simotest

CXXFLAGS += -Wall -Wextra -O3 -fconcepts -std=gnu++17 -I../include # -fdump-lang-class
DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJDIR)/$*.Td
POSTCOMPILE = @mv -f $(OBJDIR)/$*.Td $(OBJDIR)/$*.d && touch $@

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)

build/%.o: src/%.cpp $(OBJDIR)/%.d | $(OBJDIR)
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c -o $@ $<
	$(POSTCOMPILE)

clean:
	rm -rf $(TARGET) $(OBJDIR)

run: $(TARGET)
	$(TARGET)

$(OBJDIR):
	@mkdir -p build/

$(OBJDIR)/%.d: ;
.PRECIOUS: $(OBJDIR)/%.d

include $(wildcard $(patsubst src/%,$(OBJDIR)/%.d,$(basename $(SRC))))