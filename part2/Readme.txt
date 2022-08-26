IMPORTANT: 
all lines starts with '$' means the Linux instructions.
------------------------------------------------------------------------------
1.File discription
'Packet.h': Contains the defination of DATA, ACK, REJECT packets and related build, extract functions.
'Server.c': Contains server operations.
'Client.c': Contains client operations.
'Screenshots': Contains screenshots of running program.
------------------------------------------------------------------------------
2.Compile instruction
IMPORTANT: The program must run under LINUX environment, use Cygwin, Virtual Machine or other Linux simulation method to run it 
on Windows or other OSs.

2.1 Go to file address: 
$ cd "E:/example/file/address/Programming assignment-Mingjia Xue/Assignment1"

2.2 Compile
Compile 'Server.c' and 'Client.c' on seperate terminals
Complie Server.c: 
$ gcc Server.c -o ./s.out
Compile Client.c:
$ gcc Client.c -o ./c.out
------------------------------------------------------------------------------
3.Run
Run server:
$ ./s.out

Run client: 
$ ./c.out
------------------------------------------------------------------------------
4.Operation instruction while running

IMPORTANT: Server must run before client.

4.1 Server
When server starts running, do nothing as it will aotumaticlly receive packets from client.
When a transmission round is over(server received 5 packets from client), input a number on the screen based on instructions to continue trasmission or terminate server.

4.2 Client
When client starts running, do nothing as it will aotumaticlly send 1 correct packet and 3 incorrect packets to server.
When a transmission round is over(4 packets has sent to server), input a number on the screen based on instructions to continue trasmission or terminate client.



