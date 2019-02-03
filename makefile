cc = g++

proja:ProjectA_main.o cRouter.o networkLib.o sample_tunnel.o
	g++ ProjectA_main.o cRouter.o networkLib.o sample_tunnel.o -o proja
ProjectA_main.o:main.cpp
	g++ -c main.cpp -o ProjectA_main.o
cRouter.o:cRouter.cpp
	g++ -c cRouter.cpp -o cRouter.o
networkLib.o:network.cpp
	g++ -c network.cpp -o networkLib.o
tunnel.o:sample_tunnel.c
	g++ -c sample_tunnel.c -o tunnel.o
clean:
	rm -f proja *.o
