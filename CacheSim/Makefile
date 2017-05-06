CXX=g++
CPPFLAGS=-std=c++11 -lpthread
OBJS=Bus.o Cache.o MSI.o Protocol.o main.o
# This line complies the code and creates the binary

%o: %.cpp
	$(CXX) $(CPPFLAGS) -c $<

run_cacheSim: $(OBJS)
	$(CXX) $(CPPFLAGS) $(OBJS) -o $@

clean:
	rm -f *.o run_cacheSim
