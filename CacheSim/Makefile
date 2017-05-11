CXX=g++
CPPFLAGS=-std=c++11 -g -pthread
OBJS=Memory.o Bus.o Cache.o MSI.o MOSI.o MESI.o MOESI.o CompSnooping.o Dragon.o Protocol.o main.o
# This line complies the code and creates the binary

%o: %.cpp
	$(CXX) -c $< $(CPPFLAGS)

run_cacheSim: $(OBJS)
	$(CXX) $(OBJS) -o $@ $(CPPFLAGS)

clean:
	rm -f *.o run_cacheSim
