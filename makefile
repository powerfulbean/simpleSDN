cc = g++

proja:ProjectA_main.o cRouter.o networkLib.o sample_tunnel.o RouterPrim.o RouterSecond.o
	g++ ProjectA_main.o cRouter.o networkLib.o sample_tunnel.o RouterPrim.o RouterSecond.o -o proja
ProjectA_main.o:main.cpp
	g++ -c main.cpp -o ProjectA_main.o
cRouter.o:cRouter.cpp
	g++ -c cRouter.cpp -o cRouter.o
networkLib.o:network.cpp
	g++ -c network.cpp -o networkLib.o
tunnel.o:sample_tunnel.c
	g++ -c sample_tunnel.c -o tunnel.o
RouterPrim.o: RouterPrim.cpp
	g++ -c RouterPrim.cpp -o RouterPrim.o
RouterSecond.o: RouterSecond.cpp
	g++ -c RouterSecond.cpp -o RouterSecond.o
clean:
	rm -f proja *.o
