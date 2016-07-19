CC = g++

SRC = main.cpp graph.cpp maxflow.cpp

EXEC = heuristic5

CFLAGS = `pkg-config --cflags opencv` -g
LFLAGS = `pkg-config --libs opencv` -lpthread
 
OBJ = $(addsuffix .o, $(basename $(SRC)))

$(EXEC): $(OBJ)
	$(CC) $(LFLAGS) $(OBJ) -o $(EXEC)

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXEC) *.o 
