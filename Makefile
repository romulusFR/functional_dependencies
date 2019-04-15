CXX=g++
CXXFLAGS=-c -Wall -Wextra -Weffc++ -Winline -std=c++14 -pedantic 
LDLIBS= -lboost_program_options
LDFLAGS= -std=c++14 

SRCDIR=./src/

SOURCES= $(SRCDIR)FunDep.cpp $(SRCDIR)Main.cpp
OBJECTS=$(SOURCES:.cpp=.o)

EXECUTABLE=fundep

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(SRCDIR)FunDep.o: $(SRCDIR)FunDep.hpp

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(SRCDIR)*.o $(SRCDIR)*~ *~
