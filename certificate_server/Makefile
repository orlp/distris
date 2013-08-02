certificate_server.exe: ../deps/boost/libboost.a certificate_server.cpp
	g++ -O3 -std=c++11 -I../deps/boost/include certificate_server.cpp -L../deps/boost -lboost -lws2_32 -lmswsock -static-libgcc -static-libstdc++ -s -o certificate_server.exe

../deps/boost/libboost.a:
	$(MAKE) -C ../deps/boost/ libboost.a
