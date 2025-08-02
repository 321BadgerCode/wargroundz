# Compiler and flags
CXX = g++
CXXFLAGS = -Wall

# Libraries
LIBS = -lglfw -lGL -lGLEW -lSOIL

# Source files
SOURCES = main.cpp

# Output executable
EXECUTABLE = warground

# Build rules
all: $(EXECUTABLE) run

# Compile the program
$(EXECUTABLE): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(EXECUTABLE) $(SOURCES) $(LIBS)

# Clean the build
clean:
	rm -f $(EXECUTABLE)

# Run the program
run: $(EXECUTABLE)
	./$(EXECUTABLE)