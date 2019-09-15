////////////////////////////////////////////////
//
//
//      CS 6378 - PROJECT 3
//        VOTING ALGORITHM
//            SERVER
//         SUBMITTED BY
//   Jayaramaraja Balaramaraja(jxb162030)
//     Ayushi Chourasi(akc170630)
//
/////////////////////////////////////////////////
#include "connection.h"
using namespace std;
#define TRUE             1
#define FALSE            0
//global mutuex
pthread_mutex_t list_lock;
pthread_mutex_t resource_lock;
pthread_mutex_t LOCKED_lock;
pthread_mutex_t message_received_count_lock;
pthread_mutex_t message_sent_count_lock;
pthread_mutex_t exit_number_lock;
//global variables
bool LOCKED=FALSE;
int message_sent_count=0;
int message_received_count=0;
int deadlock_count=0;
int exit_number=5;
priority_queue <string, vector<string> ,greater<string> > QUEUE;
bool terminate_socket=TRUE;
bool close_flag=TRUE;
int VN=1;
int RU=8;
char DS='A';
bool LOCK_REQUEST=FALSE;
int M=0;
int waiting_reply=0;
bool Update_Success=FALSE;
bool Update_Flag=TRUE;
//struct to store the VN,RU and DS from answering servers
struct component
{
int RU;
int VN;
char DS;
};
list <component> set_I;

//Function
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
		address.sin_port = htons(PORT);
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
		cout<<"waiting to connect here"<<endl;
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

///Function
//to read all the files present in the directory
string read_directory(char* name)
{
		
	char files[200];
	DIR* dirp = opendir(name);
	list<string> filesList;
	struct dirent * dp;
	
	while ((dp = readdir(dirp)) != NULL) 
	{
	
		if(strcmp(dp->d_name,".")==0 || strcmp(dp->d_name,"..")==0 || string(dp->d_name).back()=='~')
			continue;
		
		filesList.push_back(dp->d_name);
		
	}
	closedir(dirp);
	filesList.sort();
	list<string>::iterator it2;

	
	for (it2 = filesList.begin(); it2 != filesList.end(); it2++)
	{  	
		strcat(files,(*it2).c_str());
		strncat(files,",",sizeof(","));
	}
	return string(files);
}
//find the server number
int findServerNum(string IP)
{
	char* serverIPList[8]={A,B,C,D,E,F,G,H};	
	int N=8; //Total number of Servers
	int server_num=10;
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

//Reading the phase of connection and disconnection
// This function creates a bit mask to show connection to send as (1) and nodes to disconnect as (-1)
int readConfigure(int phase_num,int server_num,int makeConnect[8])
{
	char* serverIPList[8]={A,B,C,D,E,F,G,H};
	std::ifstream f("Configure_Connection.txt");
	string s;
	string server_letter="ABCDEFGH-";
	int flag=0;
	int end_file=0; 
	int count=0;
	list<char> fromConnect;
	for (int i = 1; i <= phase_num; i++)
	{	
		try
		{
        		std::getline(f, s);
		}
		catch(...)
		{
			return 1;
		}
	}
	cout<<s<<endl;
	for(int j=0;j<s.length();j++)
	{
		if(s[j]==':')
		{
			if(flag==1)
				break;
			else
			{
			 	fromConnect.clear();
			}
		}
		if(server_letter[server_num]==s[j])
		{
			flag=1;
			
			continue;
		}
		else if(flag!=1)
		{
			fromConnect.push_back(s[j]);
		}
		if(flag==1 && s[j]!=':')
		{
			size_t found0 = server_letter.find(s[j]);
			if(found0 != string::npos)
			{
				makeConnect[found0]=1;
			}
			
		}
		
		
	}
	list<char>::iterator it;
	int l;
	for (l=0,it=fromConnect.begin(); it!=fromConnect.end(); ++it,l++)
	{
		size_t found0 = server_letter.find((*it));
		if(found0 != string::npos)
		{
    			makeConnect[found0]=0;
		}
		
	}
	return end_file;
}


//function to append the string to the file
void writeFile(char* filename,string appendText)
{

  	std::ofstream outfile;

  	outfile.open(filename, std::ios_base::app);
  	outfile << appendText<<endl;;


}
//function to get the last line from the file
string getLastLine(char* filename)
{
	std::string lastline;
	std::ifstream fs;
	fs.open(filename, std::fstream::in);
	if(fs.is_open())
	{
		//Got to the last character before EOF
		fs.seekg(-1, std::ios_base::end);
		if(fs.peek() == '\n')
		{
			//Start searching for \n occurrences
			fs.seekg(-1, std::ios_base::cur);
			int i = fs.tellg();
			for(i;i > 0; i--)
			{
				if(fs.peek() == '\n')
				{
 					 //Found
  					fs.get();
  					break;
				}		
				//Move one character back
				fs.seekg(i, std::ios_base::beg);
			}
		}

		getline(fs, lastline);
		std::cout << lastline << std::endl;
	}
	else
	{
		std::cout << "Could not find end line character" << std::endl;
		lastline="no content";
	}	
	return lastline;
}
//read thread struct
struct read_app
{
		list <SocketWrapper> *SocketConnectionList; // gives the number of components connected
		int socket_id;
		int connect_num;
		
};
//Read thread that connects to specific servers socket
void *readThreadFunction(void *threadarg)
{
	struct read_app *data;
	data = (struct read_app *) threadarg;
	char *close_msg="close";
	cout<<"Read Thread connected to SERVER"<<(char)('A'+data->connect_num)<<" with socket_id:"<<data->socket_id<<endl;
	
	while(1)
	{
		char buf[1024]={0};
		int valread1;
		valread1 = read(data->socket_id  , buf, 1024); 
		cout<<string(buf)<<endl;
		string buffer(buf);
		
		size_t found0 = buffer.find("LOCK_REQUEST");
		size_t found1 = buffer.find("VOTE_REQUEST");
		size_t found2 = buffer.find("COMMIT");
		size_t found3 = buffer.find("ABORT");
		std::string delimiter = ":";
		if(valread1 && (strcmp(buf, "close") == 0))
		{
			break;
		}
		else if(valread1 && (found0 != string::npos))
		{
			
			if (!LOCK_REQUEST)
			{
				list <SocketWrapper> :: iterator it1;
				char fileStatus[100]={0};
				snprintf( fileStatus, sizeof(fileStatus), "VOTE_REQUEST:%d :%d :%c",VN,RU,DS);
				send(data->socket_id, fileStatus , strlen(fileStatus) , 0 );
				LOCK_REQUEST=TRUE;
			}
		}
		else if(valread1 && (found1 != string::npos))
		{
			if(LOCK_REQUEST)
			{
				int RU_final;
				char DS_final;
				waiting_reply--; //received a  reply
				
				size_t pos = 0;
				std::string token;
				pos = buffer.find(":");
				token = buffer.substr(0, buffer.find(":"));
				buffer.erase(0, pos + delimiter.length());

				pos = buffer.find(":");
				token = buffer.substr(0, buffer.find(":"));
				int temp_VN=stoi(token); //string might fail
				if(M<temp_VN)
					M=temp_VN;
				buffer.erase(0, pos + delimiter.length());
				
				pos = buffer.find(":");
				token = buffer.substr(0, buffer.find(":"));
				int temp_RU=stoi(token);
				buffer.erase(0, pos + delimiter.length());
				
				
				pos = buffer.find(":");
				token = buffer.substr(0, buffer.find(":"));
				
				char temp_DS=token[token.size()-1];
				component c;
				c.VN=temp_VN;
				c.RU=temp_RU;
				c.DS=temp_DS;
				set_I.push_back(c);
				
				cout<<"Status of SERVER:"<<(char)('A'+data->connect_num)<<" VN:"<<temp_VN<<" RU:"<<temp_RU<<" DS:"<<temp_DS<<endl;
				while(waiting_reply!=0)  // wait for all the reply of VN
				{
				}
				if(Update_Flag)
				{
					Update_Flag=FALSE;
					list <component> ::iterator m;
					for( m = set_I.begin(); m != set_I.end(); ++m) 
					{
						if((*m).VN==M)
						{
							RU_final=temp_RU;
						}
					}
					
					if(RU_final/2<(data->SocketConnectionList)->size())
					{
						Update_Success=TRUE;
						//temp_success=TRUE;	
					
					}
					else if(RU_final/2==(data->SocketConnectionList)->size())
					{
						int check_DS=DS-'A';
						list <SocketWrapper> :: iterator it1;
						for( it1 = (data->SocketConnectionList)->begin(); it1 != (data->SocketConnectionList)->end(); ++it1) 
						{
							if((*it1).connect_num==check_DS)
							{
							Update_Success=TRUE;
							break;
							}
						}					
					
					}
					else
					{
						Update_Success=FALSE;
					}
				
					if(Update_Success)
					{
					
										
						VN=M;
						VN++;
						RU=(data->SocketConnectionList)->size();
						int DS_low=8;
						list <SocketWrapper> :: iterator it1;
						for( it1 = (data->SocketConnectionList)->begin(); it1 != (data->SocketConnectionList)->end(); ++it1) 
						{
							if((*it1).connect_num<DS_low)
							{
								DS_low=(*it1).connect_num;
							}
						}
						if(findServerNum(getIPAddress())<DS_low)
						{
							DS_low=findServerNum(getIPAddress());
						}
						if((data->SocketConnectionList)->size()%2==0)
						{
							DS='A'+ DS_low;
						}
						for( it1 = (data->SocketConnectionList)->begin(); it1 != (data->SocketConnectionList)->end(); ++it1) 
						{
							if((*it1).connect_num!=10 || (*it1).connect_num!=findServerNum(getIPAddress()))
							{
								char fileStatus[100]={0};
								snprintf( fileStatus, sizeof(fileStatus), "COMMIT:%d :%d :%c",VN,RU,DS);
								if((*it1).receiver==getIPAddress())
								{
									 send((*it1).socket_id.return_accept_socket(), fileStatus , strlen(fileStatus) , 0 );
							
								}
								else
								{
									send((*it1).socket_id.socket_fd, fileStatus , strlen(fileStatus) , 0 );
								
								}
				
							}
						}
						cout<<"********************************************"<<endl;
						cout<<"Status::VN="<<VN<<", RU="<<RU<<", DS="<<DS<<endl;
						cout<<"********************************************"<<endl;
						cout<<"VN updated"<<endl;
					
					}
					else
					{
					
					
						list <SocketWrapper> :: iterator it1;
						for( it1 = (data->SocketConnectionList)->begin(); it1 != (data->SocketConnectionList)->end(); ++it1) 
						{
							if((*it1).connect_num!=10 || (*it1).connect_num!=findServerNum(getIPAddress()))
							{
								char fileStatus[100]={0};
								snprintf( fileStatus, sizeof(fileStatus), "ABORT");
								if((*it1).receiver==getIPAddress())
								{
									 send((*it1).socket_id.return_accept_socket(), fileStatus , strlen(fileStatus) , 0 );
							
								}
								else
								{
									send((*it1).socket_id.socket_fd, fileStatus , strlen(fileStatus) , 0 );
								
								}
				
							}
						}
						cout<<"********************************************"<<endl;
						cout<<"Status::VN="<<VN<<", RU="<<RU<<", DS="<<DS<<endl;
						cout<<"********************************************"<<endl;
				 		cout<<"write aborted"<<endl;
					
					}
				//Update for sucess
				set_I.clear();
				LOCK_REQUEST=FALSE;
				}
			}
		}
		else if(valread1 && (found2 != string::npos))
		{
			if(LOCK_REQUEST)
			{
				size_t pos = 0;
				std::string token;
				pos = buffer.find(":");
				token = buffer.substr(0, buffer.find(":"));
				buffer.erase(0, pos + delimiter.length());
		
				pos = buffer.find(":");
				token = buffer.substr(0, buffer.find(":"));
				VN=stoi(token); //string might fail
				buffer.erase(0, pos + delimiter.length());

				pos = buffer.find(":");
				token = buffer.substr(0, buffer.find(":"));
				RU=stoi(token);
				buffer.erase(0, pos + delimiter.length());

				pos = buffer.find(":");
				token = buffer.substr(0, buffer.find(":"));
				
				DS=token[token.size()-1];
				cout<<"********************************************"<<endl;
				cout<<"Status::VN="<<VN<<", RU="<<RU<<", DS="<<DS<<endl;
				cout<<"********************************************"<<endl;
				cout<<"Write Success!"<<endl;
				LOCK_REQUEST=FALSE;
				Update_Flag=TRUE;
			}
		}
		else if(valread1 && (found3 != string::npos))
		{
			if(LOCK_REQUEST)
			{
				cout<<"Write Failed!"<<endl;
				LOCK_REQUEST=FALSE;
				Update_Flag=TRUE;
			}
		}
		
	}
	cout<<"READ Thread exit - disconnected to SERVER:"<<(char)('A'+data->connect_num)<<endl;
}

//listen thread Struct
struct listenThreadStruct
{
	list <SocketWrapper> *SocketConnectionList;
};
//Declaring globally
// The thread cant resuse the namespace - as it creates pointer reference problem hence array of thread are created
// with thread index taking with specific server number
pthread_t readThreadApplication[8];
struct read_app R[8];


// This thread always listen in the port 2408 and updates the list
void *listenThread(void *threadarg)
{
	struct listenThreadStruct *data;
	data = (struct listenThreadStruct *) threadarg;
	cout<<"Listen Thread Started"<<endl;
	char *setup_msg = "received";
	
	while(terminate_socket)
	{
		Socket_connection s1;
		s1.listen_socket();
		while(1)
		{	
			int stat=s1.return_accept_response();
			if (stat==1) 
			{	
			
				send(s1.return_accept_socket(), setup_msg , strlen(setup_msg) , 0 ); 
				SocketWrapper w1;
				w1.sender=inet_ntoa(s1.address.sin_addr);
				w1.receiver=getIPAddress();
				w1.socket_id=s1;
				if(string(inet_ntoa(s1.address.sin_addr))==con)
				{
					w1.connect_num=10;
				}
				else
				{
					w1.connect_num=findServerNum(string(inet_ntoa(s1.address.sin_addr)));
				}
				
				(data->SocketConnectionList)->push_back(w1);
				
				cout<<"Receiver - connected from SERVER "<<findServerNum(inet_ntoa(s1.address.sin_addr)) <<" to SERVER "<<findServerNum(getIPAddress())<<endl;
				//read thread created after adding in list
				if(w1.connect_num!=10)
				{
					
					
					R[w1.connect_num].connect_num=findServerNum(string(inet_ntoa(s1.address.sin_addr)));//(*iterator1).connect_num;
					R[w1.connect_num].socket_id=s1.return_accept_socket();
					R[w1.connect_num].SocketConnectionList=data->SocketConnectionList;
					int rc = pthread_create(&readThreadApplication[w1.connect_num], NULL, readThreadFunction, (void *)&R[w1.connect_num]);
					if (rc)
					{
						cout<<"Problem with the creating exit thread.."<<endl;
						return 0;	
					}
				}					
			}
			else
			{	
				cout<<"couldnt connect to the socket-receiver side"<<endl;
		
			}
		
		
		}
		close_flag=TRUE;
	}


}



// Make connection.. Server waits to connect to all threads
int makeConnection(list <SocketWrapper> *SocketConnectionList)
{	
	cout<<"Main Client Thread created"<<endl;
	int phase=4;//number of stage... this has to be entered manually... it is the length of the configure file
	char* serverIPList[8]={A,B,C,D,E,F,G,H};
	int connection_start=0,status,client_num,flag=0;
	char *setup_msg = "received";
	char *close_msg ="close";
	int valread=0;
	int server_num;
	server_num=findServerNum(getIPAddress());
	cout<<"I am a Server no:"<<(char)('A'+server_num) <<" and My Ip address is::"<<getIPAddress()<<endl;
	
	//creating the generic listen thread that listens in the PORT 2408 and push the socket to the List.
	pthread_t listen_application;
	struct listenThreadStruct l;
	l.SocketConnectionList=SocketConnectionList;
	int rc = pthread_create(&listen_application, NULL, listenThread, (void *)&l);
	if (rc)
	{
		cout<<"Problem with the creating listen thread.."<<endl;
		return 0;	
	}


	int phase_num=0;
	
	while(1)
	{

		string Phase_Shift;
			
		char Controller_msg[1024]={0};
		list <SocketWrapper> :: iterator it1;
		
		for( it1 = SocketConnectionList->begin(); it1 != SocketConnectionList->end(); ++it1) 
		{
			if((*it1).connect_num==10)
			{
				valread = read((*it1).socket_id.return_accept_socket(), Controller_msg, 1024); 
				break;
			}
		}
		string Controller_buffer(Controller_msg);
		size_t found0 = Controller_buffer.find("write");
		std::string delimiter = ":";
		if(strcmp(Controller_msg,"phase")==0 && valread)
		{
			if(phase_num>5)// 5 is the number of lines in the configure text
			{	
				cout<<"Exiting beacuse configure file ended"<<endl;
				break;
			}
			int makeConnect[8]={-1,-1,-1,-1,-1,-1,-1,-1};
			phase_num=phase_num+1;
			int end_file=readConfigure(phase_num,server_num,makeConnect);
			if(end_file)
			{
				cout<<"Config File Ended"<<endl;
				break;
			}
			cout<<"*********************************"<<endl;
			cout<<"ITERATION PHASE::"<<phase_num<<endl;
			cout<<"*********************************"<<endl;
			for(int i=0;i<8;i++)
			{
				cout<<makeConnect[i];
			}
			cout<<endl;
			for(int i=0;i<8;i++)
			{
				if(makeConnect[i]==-1)
				{
					list <SocketWrapper> :: iterator it;	
					for(it = SocketConnectionList->begin(); it != SocketConnectionList->end(); ++it) 
					{
						if((*it).connect_num==i)
						{
							if((*it).receiver==getIPAddress())
							{
								send((*it).socket_id.return_accept_socket(), close_msg , strlen(close_msg) , 0 ); 
								shutdown((*it).socket_id.return_accept_socket(),3);
								close((*it).socket_id.return_accept_socket());
							
							}
							else
							{
								send((*it).socket_id.socket_fd, close_msg , strlen(close_msg) , 0);
								shutdown((*it).socket_id.socket_fd,3);
								close((*it).socket_id.socket_fd);
							}		
							cout<<"Removing the node SERVER:"<< (char)('A'+findServerNum((*it).sender))<<" connected to SERVER:"<<(char)('A'+findServerNum((*it).receiver))<<endl;
							it=SocketConnectionList->erase(it);
							break;
						}
			
					}
				}
			}
			cout<<endl;
			int skip;
			

			for(int i=0;i<8;i++)
			{
				skip=0;
				if(makeConnect[i]==1)
				{
					list <SocketWrapper> :: iterator itt;
					int l;
					for(l=0, itt = SocketConnectionList->begin(); itt != SocketConnectionList->end(); ++itt,l++) 
					{
						if((*itt).sender==string(serverIPList[i]) || (*itt).receiver==string(serverIPList[i]))
						{
							skip=1;
							break;
						}
					}
					if (skip==1)
					{
						continue;
					}
					
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
						int valread1;
						valread1 = read(s1.socket_fd  , buf, 1024); 
						
						if(valread1 && (strcmp(buf, "received") == 0))
						{
							
							SocketConnectionList->push_back(w1); //need mutex here
							cout<<"Sender - connected SERVER:"<< (char)('A'+findServerNum(w1.sender)) <<" to SERVER:"<< (char)('A'+findServerNum(w1.receiver)) <<endl;
						}
					}
					else
					{
						cout<<"error in sending the client connect.."<<endl;
		
					}
					
					R[w1.connect_num].connect_num=i;
					R[w1.connect_num].socket_id=s1.socket_fd;
					R[w1.connect_num].SocketConnectionList=SocketConnectionList;
					int rc = pthread_create(&readThreadApplication[w1.connect_num], NULL, readThreadFunction, (void *)&R[w1.connect_num]);
					if (rc)
					{
						cout<<"Problem with the creating exit thread.."<<endl;
						return 0;	
					}		
				
				}
				
			}
			
					
		}
		else if((found0 != string::npos) && valread)
		{
			cout<<"Initiate the Write Sequence"<<endl;
			if(SocketConnectionList->size()==1)
			{
				cout<<"********************************************"<<endl;
				cout<<"Status::VN="<<VN<<", RU="<<RU<<", DS="<<DS<<endl;
				cout<<"********************************************"<<endl;
				cout<<"Write Aborted"<<endl;
			}
			else
			{
				LOCK_REQUEST=TRUE;
				Update_Success=FALSE;
				Update_Flag=TRUE;
				waiting_reply=SocketConnectionList->size()-1;
				
				list <SocketWrapper> :: iterator iterator1;
				int s;
				for(s=0, iterator1 = SocketConnectionList->begin(); iterator1 != SocketConnectionList->end(); ++iterator1,s++) 
				{
					if((*iterator1).connect_num!=10)
					{
						char fileStatus[100]={0};
						snprintf( fileStatus, sizeof(fileStatus), "LOCK_REQUEST");
						if((*iterator1).sender==getIPAddress())
						{
							send((*iterator1).socket_id.socket_fd, fileStatus , strlen(fileStatus) , 0 );
							cout<<"LOCK_REQUEST sent to SERVER"<<findServerNum((*iterator1).receiver)<<endl;
						}
						else
						{
							send((*iterator1).socket_id.return_accept_socket(), fileStatus , strlen(fileStatus) , 0 );
							cout<<"LOCK_REQUEST sent to SERVER"<<findServerNum((*iterator1).sender)<<endl;
						}
					}
				}
			}
			
		}
		else if(strcmp(Controller_msg,"end")==0  && valread)
		{
			cout<<"End the Transactions"<<endl;
			cout<<"SYSTEM EXIT"<<endl;
			exit(0);
			break;
		}
		
		
	}	


	cout<<"Connection completed"<<endl;	
}



//Main Function
int main()
{	
	list <SocketWrapper> SocketConnectionList;
	int status=makeConnection(&SocketConnectionList);
	if(status==0)
	{
		cout<<"Problem with connection/disconnection setup.."<<endl;
		return 1;	
	}

	cout<<"Back to Main Connection"<<endl;
	while(1)
	{
	}
		
	return 0;
}
