CC = g++

SRC = image-seam-carving.cpp

EXEC = image-seam-carving

CFLAGS = `pkg-config --cflags opencv` -g
LFLAGS = `pkg-config --libs opencv` 
 
OBJ = $(addsuffix .o, $(basename $(SRC)))

$(EXEC): $(OBJ)
	$(CC) $(LFLAGS) $(OBJ) -o $(EXEC)

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXEC) *.o 
