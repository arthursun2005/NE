CXX = g++ -std=gnu++14 -Ofast

NE: main.o population.o
	$(CXX) main.o population.o -o NE

main.o: main.cpp population.h genome.h ne.h common.h
	$(CXX) -c main.cpp -o main.o

population.o: population.cpp population.h genome.h ne.h common.h
	$(CXX) -c population.cpp -o population.o

clean:
	rm -f population.o main.o NE
