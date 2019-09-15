CXX = g++ -std=gnu++14 -Ofast

NE: main.o ne.o
	$(CXX) main.o ne.o -o NE

main.o: main.cpp population.h genome.h ne.h common.h
	$(CXX) -c main.cpp -o main.o

ne.o: ne.cpp population.h genome.h ne.h common.h
	$(CXX) -c ne.cpp -o ne.o

clean:
	rm -f ne.o main.o NE
