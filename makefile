cc = g++

proja:ProjectA_main.o cRouter.o
	g++ ProjectA_main.o cRouter.o -o proja
ProjectA_main.o:main.cpp
	g++ -c main.cpp -o ProjectA_main.o
cRouter.o:projectA.cpp
	g++ -c projectA.cpp -o cRouter.o
clean:
	rm -f proja *.o
