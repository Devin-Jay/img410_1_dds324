IDIR = include
SDIR = src
ODIR = build

CXX = g++
CFLAGS = -I$(IDIR)/demo

# Header dependencies (inside include/demo/)
_DEPS = demo/utils.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

# Object files (go into build/)
_OBJ = main.o utils.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

# Compile source files into build/*.o
$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	mkdir -p $(ODIR)
	$(CXX) -c -o $@ $< $(CFLAGS)

# Link executable (placed in build/)
blur: $(OBJ)
	$(CXX) -o $(ODIR)/$@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(ODIR)/blur *~ core $(IDIR)/*~ output.ppm
