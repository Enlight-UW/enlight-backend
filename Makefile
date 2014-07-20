all:
	g++ -static -std=c++11 -o enlight-backend.exe *.cpp

clean:
	rm -rf enlight-backend.exe