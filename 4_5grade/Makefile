all: UDPClientRead UDPClientWrite UDPServer
UDPClientRead: UDPClientReader.c
	gcc UDPClientReader.c  -o UDPClientRead
UDPClientWrite: UDPClientWriter.c 
	gcc UDPClientWriter.c  -o UDPClientWrite
UDPServer: UDPServer.c 
	gcc UDPServer.c -o UDPServer

delete: 
	rm UDPServer UDPClientWrite UDPClientRead