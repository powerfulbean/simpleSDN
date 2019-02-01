cc = g++

proja:ProjectA_main.o cRouter.o networkLib.o
	g++ ProjectA_main.o cRouter.o networkLib.o -o proja
ProjectA_main.o:main.cpp
	g++ -c main.cpp -o ProjectA_main.o
cRouter.o:cRouter.cpp
	g++ -c cRouter.cpp -o cRouter.o
networkLib.o:network.cpp
	g++ -c network.cpp -o networkLib.o

clean:
	rm -f proja *.o
