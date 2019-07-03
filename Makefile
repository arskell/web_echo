all:
	g++ -Wall main.cc -lwsock32 -lws2_32 -lboost_system-mgw81-mt-d-x64-1_68 -o echo.exe
