////////////////////////////////////////////////
//
//
//      CS 6378 - PROJECT 3
//       VOTING ALGORITHM
//         SUBMITTED BY
//   Jayaramaraja Balaramaraja(jxb162030)
//     Ayushi Chourasi(akc170630)
//
/////////////////////////////////////////////////


The project focusses on implement the Jajodia-Mutchler voting algorithm. Considering there be eight servers, A, B, C, D, E, F, G, and H. There is
only one data object X, replicated across the eight servers, that is subject to writes. Initially, the version number (VN),
replicas update (RU) and distinguished site (DS) values for X at all servers are 1, 8, and A, respectively. The rule for
selection of distinguished site favors the server that is alphabetically smallest among the candidates.

The graph is represented as Configure_Connection.txt file with ":" denoting the partition for the Servers

Files Included
--------------
->Controller_program.cpp
->server_program.cpp

Process to compile
------------------
controller:
-------

->Navigate to the program location.
-> type "g++ Controller_program.cpp -o client.out -lpthread -std=c++11"
->./controller.out
->(run the controller.out in machine dc01)

server:
-----
->Navigate to the program location.
-> type "g++ server_program.cpp -o server.out -lpthread -std=c++11"
->./server.out
->(run the server.out in different client machines given in the header as SERVER:A-H)


[Note:If running in different environment,change the directory path in the server]


MAKE CONNECTION:
----------------
->The Controller waits for the user input "1" to start the connection with all the Servers
->It acts as the user input to send "Phase" meassage and "Write" messages.
-> "Phase" Message to the Server makes it to read the configuration file and setup the connection accordingly.
-> All Socket connections are properly closed in case of disconnect. 
-> "Write" Message sends the write request to specific server - Server then initiates alogrithm to calculate VN,RU and DS
-> At the End of each write mesasge - Status are displayed on the terminal.

VOTING ALGORITHM:
--------------------------------------------
Each server runs on a different machine. Initially, all the servers have reliable socket connections with each other
forming one connected component  Subsequently, partitions and mergers happen as described in config file. 
In order to emulate partitioning and mergers, all socket connections between servers in the same partition are maintained/created,
while all socket connections between servers in different partition(s) are severed.
1. Two writes in each of the network components are done and results are documented in the report.
2. After each write attempt, successful or unsuccessful, output the VN, RU, and DS values for each server are displayed in the console.

