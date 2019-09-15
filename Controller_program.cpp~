////////////////////////////////////////////////
//
//
//      CS 6378 - PROJECT 3
//         VOTING ALGORITHM
//          Controller
//         SUBMITTED BY
//    Ayushi Chourasi(akc170630)
//  Jayaramaraja Balaramaraja(jxb162030)
//
/////////////////////////////////////////////////
#include "connection.h"

using namespace std;

#define TRUE             1
#define FALSE            0
typedef int64_t msec_t;
//Global mutex for receiving threads
pthread_mutex_t list_lock;
pthread_mutex_t server_list_lock;
pthread_mutex_t struct_lock;
pthread_mutex_t Id_thread_lock;
pthread_mutex_t expected_grant_lock;
pthread_mutex_t message_received_count_lock;
pthread_mutex_t message_sent_count_lock;
//Global variable for thread control
int Id_thread=0;
int connect_complete=0; 
int expected_grant=4;
int start_transaction=0;
int unit_delay=100000;//100 miliseconds
int message_sent_count=0;
int message_received_count=0;
int deadlock_count=-1;
bool exit_number=0;
pthread_t start_Quorum[30];
//Grab the IP Address of the current machine
string getIPAddress()
{
    	string ipAddress="Unable to get IP Address";
    	struct ifaddrs *interfaces = NULL;
    	struct ifaddrs *temp_addr = NULL;
    	int success = 0;
    
    	success = getifaddrs(&interfaces);
    	if (success == 0) 
	{
        
        	temp_addr = interfaces;
        	while(temp_addr != NULL) 
		{
            		if(temp_addr->ifa_addr->sa_family == AF_INET) 
			{
                
                		if(strcmp(temp_addr->ifa_name, "en0"))
				{
                    			ipAddress=inet_ntoa(((struct sockaddr_in*)temp_addr->ifa_addr)->sin_addr);
                		}
            		}
            	temp_addr = temp_addr->ifa_next;
        	}
    	}
   
    	freeifaddrs(interfaces);
    	return ipAddress;
}

msec_t time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (msec_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
///Creating a class to deal with lower socket 
class Socket_connection
{
public:
	int socket_fd, new_socket, valread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	int iMode=0;
	Socket_connection()
	{
		// Creating socket file descriptor 
		if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
		{ 
			perror("socket failed"); 
			exit(EXIT_FAILURE); 
		} 
		bzero((char *) &address, sizeof(address));
		ioctl(socket_fd, FIONBIO, &iMode); 
		address.sin_port = htons( PORT );
		address.sin_family = AF_INET;
		 
	}
	Socket_connection(const Socket_connection &sock)  //copy constructor
	{
	socket_fd=sock.socket_fd;
	new_socket=sock.new_socket; 
	valread=sock.valread;
	address=sock.address; 
	opt=1;
	addrlen =sock.addrlen;
	iMode=0;

	}
	~Socket_connection()  //destructor
	{
	
	}
	int connect_socket(char* IPname)
	{	
		
		
		if(inet_pton(AF_INET, IPname, &address.sin_addr)<=0)  
    		{ 
        		cout<<"Invalid address/ Address not supported "<<endl; 
        		return 0; 
    		}		
		
		if(connect(socket_fd, (struct sockaddr *)&address, sizeof(address))<0) 
		{
			cout<<"Connection Failed "<<endl; 
        		return 0;
		}
		else
		{
			return 1;
		}
		
		
		
    		
	}
	int listen_socket()
	{	
		
		// Forcefully attaching socket to the port 2408 
		if (setsockopt(socket_fd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR),&opt, sizeof(opt))) 
		{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
		} 
		address.sin_addr.s_addr = INADDR_ANY; 
		
		// Forcefully attaching socket to the port 8080 
		if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address))<0) 
		{ 
		perror("bind failed"); 
		return 0; 
		} 
		
		if (listen(socket_fd, 32) < 0) 
		{ 
		perror("listen failed"); 
		return 0; 
		} 
		
	}
	int return_accept_response()
	{
		
		if ((new_socket = accept(socket_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0) 
		{ 
		perror("accept failed"); 
		return 0; 
		} 
		else
		{
			return 1;
		}
		
	}
	int return_accept_socket()
	{
		return new_socket;
	}
	
			
};

//Creating a wrapper for the socket with sender and receiver information
class SocketWrapper
{
public:
	string sender;
	string receiver;
	Socket_connection socket_id;
	int connect_num;
	SocketWrapper()
	{
		sender="";
		receiver="";

	}
	SocketWrapper(const SocketWrapper &wrap)//copy constructor
	{
		sender=wrap.sender;
		receiver=wrap.receiver;
		socket_id=wrap.socket_id;
		connect_num=wrap.connect_num;

	}
	~SocketWrapper()//destructor
	{
	}
	
};

//find the server number
int findServerNum(string IP)
{
	char* serverIPList[8]={A,B,C,D,E,F,G,H};	
	int N=8; //Total number of Clients
	int server_num=0;
	for(int i=0;i<N;i++)// find the client number
	{
		if(IP==string(serverIPList[i]))
		{
			server_num=i;
			break;	
		}
	}
	return server_num;
}

//Main function to trigger connection to other clients and Servers in thread
int makeConnection(list <SocketWrapper> *SocketConnectionListServer)
{	
	cout<<"Main Client Thread created"<<endl;
	int value;
	char* serverIPList[8]={A,B,C,D,E,F,G,H};
	int valread;
	int connection_start=0,status,client_num,flag=0,rc;
	char *setup_msg = "received";
	cout<<"I am an server controller and My Ip address is::"<<getIPAddress()<<endl;
	cout<<"Type 1 to start connection:"<<endl;	
	cin>>value;
	while(1)
	{
		if(value==1)
		{
			for(int i=0;i<8;i++)
			{
				Socket_connection s1; //created for the sender clients
				SocketWrapper w1;
				w1.sender=getIPAddress();
				w1.receiver=serverIPList[i];
				w1.connect_num=findServerNum(string(serverIPList[i]));
				//connecting other servers
				int stat=s1.connect_socket(serverIPList[i]);
				w1.socket_id=s1;
				if (stat==1)
				{	

					char buf[1024]={0};
					valread = read(s1.socket_fd  , buf, 1024); 
					if(valread && (strcmp(buf, "received") == 0))
					{
						pthread_mutex_lock(&list_lock);
						SocketConnectionListServer->push_back(w1); //need mutex here
						pthread_mutex_unlock(&list_lock);
						cout<<"Sender - connected "<< w1.sender <<" to "<< w1.receiver <<endl;
					}
				}
				else
				{
					cout<<"error in sending the client connect.."<<endl;

				}
			}
			break;
		}
		else
		{
			cout<<"Please Enter 1 to start connection"<<endl;
		}
	}
		
	cout<<"************Connection completed*************"<<endl;	
	return 1;	

	
}

int sendMessage(list <SocketWrapper> *SocketConnectionListServer)
{
	string Message="";
	char *write_Request="write";
	while(1)
	{
		cout<<"Type the Message to broadcast to Server:"<<endl;
		cin>>Message;
		if(strcmp(Message.c_str(),"phase")==0)
		{
			list <SocketWrapper> :: iterator itt;
			for(itt = SocketConnectionListServer->begin(); itt != SocketConnectionListServer->end(); ++itt) 
			{
				char phase_msg[100]={0};
				snprintf(phase_msg, sizeof(phase_msg), "%s", Message.c_str());
				send(((*itt).socket_id).socket_fd, &phase_msg , strlen(phase_msg) , 0 );
			}
		}
		else if(strcmp(Message.c_str(),"write")==0)
		{
			int server_num;
			string content;
			cout<<"Enter the Server number to send write:"<<endl;
			// send to the specific server
			cin>>server_num;
			list <SocketWrapper> :: iterator itt;
			for(itt = SocketConnectionListServer->begin(); itt != SocketConnectionListServer->end(); ++itt) 
			{
				if((*itt).connect_num==int(server_num))
				{
					char lock_Request_msg[1024]={0};
					snprintf(lock_Request_msg, sizeof(lock_Request_msg), "write");
					send(((*itt).socket_id).socket_fd, &lock_Request_msg , strlen(lock_Request_msg) , 0 );
				}
			}
			
		}
		else if(strcmp(Message.c_str(),"end")==0)
		{
			list <SocketWrapper> :: iterator itt;
			for(itt = SocketConnectionListServer->begin(); itt != SocketConnectionListServer->end(); ++itt) 
			{
				
				char lock_Request_msg[1024]={0};
				snprintf(lock_Request_msg, sizeof(lock_Request_msg), "end");
				send(((*itt).socket_id).socket_fd, &lock_Request_msg , strlen(lock_Request_msg) , 0 );
				
			}
			cout<<"Transactions Ended"<<endl;
			cout<<"SYSTEM EXIT!"<<endl;
			sleep(2);
			exit(0);
			break; // need to send termination message.
		}
	}

}
///MAIN FUNCTION
int main()
{	
	
	list <SocketWrapper> SocketConnectionListServer;
	int status=makeConnection(&SocketConnectionListServer);
	if (status==0)
	{
		cout<<"problem with the connection"<<endl;
		return 0;
	}
	int stat=sendMessage(&SocketConnectionListServer);
	if (stat==0)
	{
		cout<<"problem with the sending connection"<<endl;
		return 0;
	}
	
	return 0;
}
