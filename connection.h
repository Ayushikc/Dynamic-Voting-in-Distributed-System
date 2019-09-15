//Header file

//8 servers
#define con  "10.176.69.32"             //ip addr dc01
#define A    "10.176.69.36"           	//ip addr dc05
#define B    "10.176.69.38"		//ip addr dc07
#define C    "10.176.69.46"		//ip addr dc15
#define D    "10.176.69.48"		//ip addr dc17
#define E    "10.176.69.56"		//ip addr dc25
#define F    "10.176.69.58"		//ip addr dc27
#define G    "10.176.69.66"		//ip addr dc35
#define H    "10.176.69.68"		//ip addr dc37
#define PORT	    2408

//////////////////////
//Required Header files
//////////////////////
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <ctime>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <fstream>
#include <stdlib.h>
#include <errno.h>
#include <list> 
#include <iterator>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <ifaddrs.h>
#include <sys/shm.h> 
#include <sys/stat.h>
#include <regex>
#include <ctime>
#include <queue>
#include <sys/time.h>
#include <limits>



