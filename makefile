cxx = g++

SOURCE = main.cpp \
	icmp_checksum.c icmp_checksum.h \
	sample_tunnel.c  sample_tunnel.h \
	cRouter.cpp cRouter.h \
	network.cpp network.h \
	RouterPrim.cpp RouterSecond.cpp Router.h \
	./timers
	
SUPPORT = *.txt makefile 

SUB_DIR = ./timers

proja:ProjectA_main.o cRouter.o networkLib.o sample_tunnel.o RouterPrim.o RouterSecond.o checkSum.o $(SUB_DIR)/timers.o $(SUB_DIR)/tools.o
	g++ -Wall ProjectA_main.o cRouter.o networkLib.o sample_tunnel.o RouterPrim.o RouterSecond.o checkSum.o $(SUB_DIR)/timers.o $(SUB_DIR)/tools.o -o projc
ProjectA_main.o:main.cpp
	g++ -Wall -c main.cpp -o ProjectA_main.o
cRouter.o:cRouter.cpp
	g++ -Wall -c cRouter.cpp -o cRouter.o
networkLib.o:network.cpp
	g++ -Wall  -c network.cpp -o networkLib.o
tunnel.o:sample_tunnel.c
	g++ -Wall  -c sample_tunnel.c -o tunnel.o
RouterPrim.o: RouterPrim.cpp
	g++ -Wall  -c RouterPrim.cpp -o RouterPrim.o
RouterSecond.o: RouterSecond.cpp
	g++ -Wall -c RouterSecond.cpp -o RouterSecond.o
checkSum.o: icmp_checksum.c
	g++ -Wall  -c icmp_checksum.c -o checkSum.o

	
	
clean:
	rm -f projc *.o
tar:
	tar czvf projc.tar.gz $(SOURCE) $(SUPPORT)
