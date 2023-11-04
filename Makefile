CXX=gcc
CFLAGS=-g -Wall -O3
LFALGS=-lpthread
SRC=sched_demo_312551132.cpp
OBJ=${SRC:.cpp=.o}
EXE=sched_demo_312551132

all: $(EXE)

$(EXE): $(OBJ)
	$(CXX) $^ -o $@ $(CFLAGS) $(LFALGS)

%.o: %.cpp
	$(CXX) -c $^ -o $@ $(CFLAGS) $(LFALGS)

.PHONY: clean

clean:
	rm -rf $(EXE) *.o
