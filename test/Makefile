DEPS = $(shell find ./ -name "*.h")
SRC = $(shell find ./ -name "*.cpp")
OBJ = $(SRC:%.cpp=%.o) 
OUTF = ../output

#gen rule
build: test

test: $(OBJ)
	g++ -o $(OUTF)/test $(COMMON) -I../src/  $(OBJ)  $(LINKLIB) -L../output/ -lnap -g
	-mv ./*.o $(OUTF)/

%.o: %.cpp $(DEPS)
	$(CXX) -c $<  -o $@ $(COMMON) -I../src/ 

clean:
	rm -f *.o
