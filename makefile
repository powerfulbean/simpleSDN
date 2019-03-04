cxx = g++

SOURCE = main.cpp \
	icmp_checksum.c icmp_checksum.h \
	sample_tunnel.c  sample_tunnel.h \
	cRouter.cpp cRouter.h \
	network.cpp network.h \
	RouterPrim.cpp RouterSecond.cpp Router.h
	
SUPPORT = README.stage1.txt README.stage2.txt makefile 

SUB_DIR = ./timers

proja:ProjectA_main.o cRouter.o networkLib.o sample_tunnel.o RouterPrim.o RouterSecond.o checkSum.o $(SUB_DIR)/timers.o
	g++ ProjectA_main.o cRouter.o networkLib.o sample_tunnel.o RouterPrim.o RouterSecond.o checkSum.o $(SUB_DIR)/timers.o -o proja
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
checkSum.o: icmp_checksum.c
	g++ -c icmp_checksum.c -o checkSum.o

	
	
clean:
	rm -f proja *.o
tar:
	tar czvf proja.tar.gz $(SOURCE) $(SUPPORT)
