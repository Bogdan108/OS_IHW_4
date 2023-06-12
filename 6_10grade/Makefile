all: UDPClientRead UDPClientWrite UDPServer UDPClientVisit
UDPClientRead: UDPClientReader.c
	gcc UDPClientReader.c  -o UDPClientRead
UDPClientWrite: UDPClientWriter.c 
	gcc UDPClientWriter.c  -o UDPClientWrite
UDPClientVisit: UDPClientVisitor.c
	gcc UDPClientVisitor.c  -o UDPClientVisit
UDPServer: UDPServer.c 
	gcc UDPServer.c -o UDPServer
	
delete: 
	rm UDPServer UDPClientWrite UDPClientRead UDPClientVisit