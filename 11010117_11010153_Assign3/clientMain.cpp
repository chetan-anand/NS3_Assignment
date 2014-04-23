
#include <string>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/types.h>
#include <sstream>
#include <errno.h>
#include <sys/stat.h>

using namespace std;

namespace sys
{
	inline bool isRegularFile(string path)
	{
		struct stat s;
		if (stat(path.c_str(), &s)!=0) return false;
		return (bool) (s.st_mode & S_IFREG);
	}

	inline string pwd()
	{
		char cwd[1024];
		errno = 0;
		getcwd(cwd, sizeof(cwd));
		if(errno)	return string(strerror(errno));
		return string(cwd);	
	}
	
	inline bool cd(string dir)
	{
		return chdir(dir.c_str()) != -1;
	}
	
	inline bool setRootDir(string dir)
	{
		errno = 0;
		chroot(dir.c_str());
		if(errno)	false;
		return true;
	}
	
	inline string ls(string arg)
	{
		string cmd = "ls";
		if(arg != "")	cmd += " " + arg;
		
		cmd += " 2>&1\n";
		
		errno = 0;
		
		FILE* file = popen(cmd.c_str(), "r");
		
		if(errno)	return string(strerror(errno)); 
		
		char buffer[1024];
		stringstream fileList;
		int n;
		while((n=fread(buffer, 1, 1024, file))>0)
		{
			for (int i=0; i<n; i++)
			{
				if (buffer[i]=='\n') fileList<<'\r';
				fileList<<buffer[i];
			}
		}
		pclose(file);
		
		return fileList.str();
	}
	
	inline string syst()
	{
		return string("UNIX");
	}
	
}

/////////////////////////////////////includes/tcpSocket.h/////////////////////

#include<stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

class tcpSocket
{
private:
	int m_sd;	//socket file descriptor
	string m_dest_addr;
	unsigned short m_dest_port, m_src_port;
	bool m_passive;
	
	const static int RECV_STR_BUF_SIZE = 1024;
	
public:
	tcpSocket();
	tcpSocket(int sd);
	
	unsigned short getSrcPort();
	string getSrcHostname();
	
	unsigned short getDestPort();
	string getDestHostname();
	
	bool connect(string hostname, unsigned short port);
	bool bind();
	bool bind(unsigned short port);
	bool listen();
	tcpSocket accept();
	
	int sendString(string data);
	int sendData(char* buffer, int size);
	string recvString(int max_bytes);
	string recvString();
	
	int recvData(char* buffer, int size);
	
	void close();
};

////////////////////////////////////////////////////////////////////////////////

tcpSocket::tcpSocket():
	m_sd(0)
{}

tcpSocket::tcpSocket(int sd):
	m_sd(sd)
{}

unsigned short tcpSocket::getSrcPort()
{
	struct sockaddr_in localAddress;
	socklen_t addressLength = sizeof(localAddress);
	getsockname(m_sd, (struct sockaddr*)&localAddress, &addressLength);
	return ntohs(localAddress.sin_port);
}

string tcpSocket::getSrcHostname()
{
	struct sockaddr_in localAddress;
	socklen_t addressLength = sizeof(localAddress);
	getsockname(m_sd, (struct sockaddr*)&localAddress, &addressLength);
	return string(inet_ntoa( localAddress.sin_addr));
	//return string("127.0.0.1");
}

unsigned short tcpSocket::getDestPort()
{
	struct sockaddr_in localAddress;
	socklen_t addressLength = sizeof(localAddress);
	getpeername(m_sd, (struct sockaddr*)&localAddress, &addressLength);
	return ntohs(localAddress.sin_port);
}

string tcpSocket::getDestHostname()
{
	struct sockaddr_in localAddress;
	socklen_t addressLength = sizeof(localAddress);
	getpeername(m_sd, (struct sockaddr*)&localAddress, &addressLength);
	return string(inet_ntoa( localAddress.sin_addr));
	//return string("127.0.0.1");
}

bool tcpSocket::connect(string hostname, unsigned short port)
{
	stringstream strport;
	strport<<port;

	struct addrinfo hints, *res;

	// first, load up address structs with getaddrinfo():

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(hostname.c_str(), strport.str().c_str(), &hints, &res);

	// make a socket:

	m_sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	// connect!

	return ::connect(m_sd, res->ai_addr, res->ai_addrlen) != -1;
}

bool tcpSocket::bind(unsigned short port)
{
	stringstream strport;
	strport<<port;
	struct addrinfo hints, *res;

	// first, load up address structs with getaddrinfo():

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	getaddrinfo(NULL, strport.str().c_str(), &hints, &res);

	// make a socket:

	m_sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	// bind it to the port we passed in to getaddrinfo():

	bool success = ::bind(m_sd, res->ai_addr, res->ai_addrlen) != -1;
	return success;
	
}

bool tcpSocket::bind()
{
	return tcpSocket::bind(0);
}

bool tcpSocket::listen()
{
	return ::listen(m_sd, 10) !=-1;
}

tcpSocket tcpSocket::accept()
{
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	return tcpSocket(::accept(m_sd, (struct sockaddr *)&their_addr, &addr_size));
}

int tcpSocket::sendString(string data)
{
	//if socket is uninitialized
	if (m_sd <= 0)
	{
		return -1;
	}
	
	return send(m_sd, data.c_str(), data.length(), 0);
	
}

int tcpSocket::sendData(char* buffer, int size)
{
	//if socket is uninitialized
	if (m_sd <= 0)
	{
		return -1;
	}
	
	return send(m_sd, buffer, size, 0);
}

string tcpSocket::recvString(int max_bytes)
{
	//if socket is uninitialized
	if (m_sd <= 0)
	{
		return "";
	}
	
	char* buffer = new char[max_bytes];
	
	int bytes_recveived = recv(m_sd, buffer, max_bytes-1, 0);
	buffer[bytes_recveived] = '\0';
	
	return string(buffer);
}

string tcpSocket::recvString()
{
	return recvString(RECV_STR_BUF_SIZE);
}

int tcpSocket::recvData(char* buffer, int size)
{
	//if socket is uninitialized
	if (m_sd <= 0)
	{
		return -1;
	}
	
	return recv(m_sd, buffer, size, 0);
}

void tcpSocket::close()
{
	::close(m_sd);
}


///////////////////////////////includes/ftpRequest.h/////////////////////////////////


#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>

using namespace std;

class ftpRequest
{
private:
	string m_cmd;
	string m_arg;

public:

	ftpRequest();
	ftpRequest(string cmd);
	ftpRequest(string cmd, string arg);
	static ftpRequest parseFtpRequest(string s);
	string toString();
	string getCmd();
	string getArg();
	void setCmd(string cmd);
	void setArg(string arg);
	/// input: xx.yy.zz.ww:1234
	/// output: xx,yy,zz,ww,high_byte(1234),low_byte(1234)
	static string splitPortArg(string portArg);

	
	/// input: xx,yy,zz,ww,high_byte(1234),low_byte(1234)
	/// output: xx.yy.zz.ww:1234
	static string combinePortArg(string portArg);
	
};

//#endif


///////////////////////////////////////////////////////////////////////////////
//#include "../includes/ftpRequest.h"


ftpRequest::ftpRequest() {}

ftpRequest::ftpRequest(string cmd)
{
	m_cmd = cmd;
	m_arg = string("");
}

ftpRequest::ftpRequest(string cmd, string arg)
{
	m_cmd = cmd;
	m_arg =  arg;
}

/// input: string containing request from socket
/// output: ftpRequest object
ftpRequest ftpRequest::parseFtpRequest(string s)
{
	int i = 0;
	string cmd,arg;
	cmd = "";
	while(s[i] != ' ' && s[i] != '\r')
	{
		cmd += s[i];
		i++;
	}
	arg = "";
	if(s[i] == '\r')	return ftpRequest(cmd,arg);
	for(i += 1; s[i] != '\r'; i++)
	{
		arg += s[i];
	}
	if(cmd == "PORT")
	{
		arg = combinePortArg(arg);
	}
	return ftpRequest(cmd,arg);
}

string ftpRequest::toString()
{
	if(m_arg == "")
	{
		return m_cmd + "\r\n";
	}
	else if(m_cmd == "PORT")
	{
		return m_cmd + " " + splitPortArg(m_arg) + "\r\n";
	}
	else
	{
		return m_cmd + " " + m_arg + "\r\n";
	}
}

/// input: xx.yy.zz.ww:1234
/// output: xx,yy,zz,ww,high_byte(1234),low_byte(1234)
string ftpRequest::splitPortArg(string portArg)
{
	int port;
	stringstream convert;
	for(int i=0;i<portArg.length();i++)
	{
		if(portArg[i]=='.')	portArg[i] = ',';
		if(portArg[i]==':')
		{
			portArg[i] = ',';
			port = atoi(portArg.substr(i+1,portArg.length()-1-i).c_str());
			portArg = portArg.substr(0,i+1);
			convert << port/256 << "," << port%256;
			portArg += convert.str();
			break;
		}
	}
	return portArg;
}

/// input: xx,yy,zz,ww,high_byte(1234),low_byte(1234)
/// output: xx.yy.zz.ww:1234
string ftpRequest::combinePortArg(string portArg)
{
	int cnt = 0,port = 0, portTemp=0;
	stringstream convert;
	for(int i=0;i<portArg.length();i++)
	{
		if(portArg[i]==',')
		{
			cnt++;
			if(cnt < 4)
			{
				portArg[i] = '.';
			}
			else if(cnt == 4)
			{
				portArg[i] = ':';
				for(int j=i+1;j<portArg.length();j++)
				{
					if(portArg[j]==',')
					{
						port = port*256 + portTemp;
						portTemp=0;
					}
					else if (isdigit(portArg[j]))
					{
						portTemp = portTemp*10 + (portArg[j]-'0');
					}
					else
					{
						port = port*256 + portTemp;
						portTemp=0;
						break;
					}
				}
				port = port*256 + portTemp;
				portTemp=0;
				
				portArg = portArg.substr(0,i+1);
				convert << port;
				portArg += convert.str();
				break;
			}
		}
	}
	return portArg;
}

string ftpRequest::getCmd()
{
	return m_cmd; 
}

string ftpRequest::getArg()
{
	return m_arg; 
}

void ftpRequest::setCmd(string cmd)
{
	m_cmd = cmd; 
}

void ftpRequest::setArg(string arg)
{
	m_arg = arg; 
}



//////////////////////////////////////includes/ftpResponse.h//////////////////////////

//#ifndef H_FTPRESPONSE
//#define H_FTPRESPONSE

#include <string>
#include <iostream>
#include <cstdlib>

using namespace std;

class ftpResponse
{

private:
	int m_code;
	string m_msg;

public:

	ftpResponse();
	ftpResponse(int code, string msg);

	int getCode();
	string getMessage();
	
	void setCode(int code);
	void setMessage(string msg);

	/// input: string containing response from socket
	/// output: ftpResponse object
	static ftpResponse parseFtpResponse(string s);
	
	string toString();

};

//#endif


//////////////////////////////////////////////////////////////////////////////////
//#include "../includes/ftpResponse.h"
#include <sstream>

ftpResponse::ftpResponse() {}

ftpResponse::ftpResponse(int code, string msg): 
	m_code(code), 
	m_msg(msg) 
{}

int ftpResponse::getCode()
{
	return m_code;
}

string ftpResponse::getMessage()
{
	return m_msg;
}

void ftpResponse::setCode(int code)
{
	m_code = code;
}

void ftpResponse::setMessage(string msg)
{
	m_msg = msg;
}

/// input: string containing response from socket
/// output: ftpResponse object
ftpResponse ftpResponse::parseFtpResponse(string s)
{
	int code;
	string msg;
	
	for (int i=0; i<s.length(); i++)
	{
		//split at space
		if (s[i]==' ')
		{
			code = atoi(s.substr(0, i).c_str());
			int j;
			//end at "\r\n"
			for (j=i+2; j<s.length() && !(s[j-1]=='\r' && s[j]=='\n'); j++);
			msg = s.substr(i+1, j-1 - (i+1));
			break;
		}
	}
	
	return ftpResponse(code, msg);
}

string ftpResponse::toString()
{
	stringstream s;
	s<<m_code<<" "<<m_msg<<"\r\n";
	return s.str();
}



//////////////////////////////////////////ftpclient.h////////////////////


//#ifndef H_FTPCLIENT
//#define H_FTPCLIENT

//#include "ftpResponse.h"
//#include "ftpRequest.h"
//#include "tcpSocket.h"
//#include "sys.h"
#include <ostream>
#include <iostream>

#define FILE_BLOCK_SIZE 1024

using namespace std;

class ftpClient
{
private:
	unsigned short m_data_port;
	unsigned short m_server_port;
	string m_server_hostname;
	tcpSocket m_control_socket;
	tcpSocket m_data_socket;
	ostream& m_log;
public:
	ftpClient(string server_hostname, unsigned short server_port, ostream& log);
	void setDataPort(unsigned short data_port);
	void setServerPort(unsigned short server_port);
	void setServerName(string server_hostname);
	unsigned short getDataPort();
	unsigned short getServerPort();
	string getServerName();
	tcpSocket getDataSocket();
	tcpSocket getControlSocket();
	ostream& getLog();
	bool setupDataPort();
	bool sendRequest(ftpRequest request);
	ftpResponse recvResponse();
	bool connect();
	void sendUsername(string username);
	bool sendPassword(string password);
	void pwd();
	void cd(string pathname);
	void ls(string dir);
	bool get(string filename, ostream& f);
	bool put(string filename, istream& f);
	void quit();
};

//#endif


/////////////////////////////////////////////////////////////////////////////////

//#include "../includes/ftpClient.h"

ftpClient::ftpClient(string server_hostname, unsigned short server_port, ostream& log):
	m_log(log)
{
	m_server_hostname = server_hostname;
	m_server_port = server_port;
}

void ftpClient::setServerPort(unsigned short server_port)
{
	m_server_port = server_port;
}

void ftpClient::setDataPort(unsigned short data_port)
{
	m_data_port = data_port;
}

void ftpClient::setServerName(string server_hostname)
{
	m_server_hostname = server_hostname;
}

unsigned short ftpClient::getDataPort()
{
	return m_data_port;
}

unsigned short ftpClient::getServerPort()
{
	return m_server_port;
}

string ftpClient::getServerName()
{
	return m_server_hostname;
}

tcpSocket ftpClient::getDataSocket()
{
	return m_data_socket;
}

tcpSocket ftpClient::getControlSocket()
{
	return m_control_socket;
}

ostream& ftpClient::getLog()
{
	return m_log;
}

bool ftpClient::setupDataPort()
{
	if(m_data_socket.bind() && m_data_socket.listen())
	{
		m_data_port = m_data_socket.getSrcPort();
		return true;
	}
	return false;
};

bool ftpClient::sendRequest(ftpRequest request)
{
	return (m_control_socket.sendString(request.toString())>0);
}

ftpResponse ftpClient::recvResponse()
{
	return ftpResponse::parseFtpResponse(m_control_socket.recvString());
}

bool ftpClient::connect()
{
	if(!m_control_socket.connect(m_server_hostname,m_server_port))
	{
		ftpResponse response = recvResponse();
		cout << response.getMessage() << endl;
		return false;
	}
	else
	{
		ftpResponse response = recvResponse();
		cout << response.getMessage() << endl;
		return true;
	}
}


void ftpClient::sendUsername(string username)
{
	sendRequest(ftpRequest(string("USER"), username));
	ftpResponse response = recvResponse();
	cout << response.getMessage() << endl;
}

bool ftpClient::sendPassword(string password)
{
	sendRequest(ftpRequest(string("PASS"), password));
	ftpResponse response = recvResponse();	
	cout << response.getMessage() << endl;
	
	if(response.getCode() == 230)	return true;
	else return false;
}

void ftpClient::pwd()
{
	sendRequest(ftpRequest(string("PWD")));
	ftpResponse response = recvResponse();
	cout << response.getMessage() << endl;
}

void ftpClient::cd(string pathname)
{
	sendRequest(ftpRequest(string("CWD"), pathname));
	ftpResponse response = recvResponse();
	cout << response.getMessage() << endl;
}

void ftpClient::ls(string dir)
{
	stringstream clientInfo;
	
	clientInfo << m_control_socket.getSrcHostname() << ":" << m_data_port;
	
	sendRequest(ftpRequest(string("PORT"),clientInfo.str()));
	tcpSocket cur_data_socket = m_data_socket.accept();
	ftpResponse response = recvResponse();
	cout << response.getMessage() << endl;
	
	if(response.getCode() != 200)	return;
	
	sendRequest(ftpRequest(string("LIST ") + dir));
	response = recvResponse();
	cout << response.getMessage() << endl;
	
	string s;
	
	while((s = cur_data_socket.recvString()).length() > 0)
	{
		//cout << s << endl;
		printf("%s", s.c_str());
	}
	cur_data_socket.close();
	
	response = recvResponse();
	cout << response.getMessage() << endl;
}

bool ftpClient::get(string filename, ostream& f)
{
	
	sendRequest(ftpRequest(string("TYPE"), string("I")));
	ftpResponse response = recvResponse();
	cout << response.getMessage() << endl;
	
	stringstream clientInfo;
	
	clientInfo << m_control_socket.getSrcHostname() << ":" << m_data_port;
	
	sendRequest(ftpRequest(string("PORT"), clientInfo.str()));
	tcpSocket cur_data_socket = m_data_socket.accept();
	response = recvResponse();
	cout << response.getMessage() << endl;
	
	if(response.getCode() != 200)	return false;
	
	sendRequest(ftpRequest(string("RETR"), filename));
	response = recvResponse();
	cout << response.getMessage() << endl;
	
	if(response.getCode() != 150)	return false;
	
	string s;
	
	char buffer[FILE_BLOCK_SIZE];
	int bytes_received;
	
	while((bytes_received = cur_data_socket.recvData(buffer,FILE_BLOCK_SIZE)) > 0)
	{
		f.write(buffer, bytes_received);
	}
	
	//cur_data_socket.close();
	
	response = recvResponse();
	cout << response.getMessage() << endl;
	
	return true;
}

bool ftpClient::put(string filename, istream& f)
{
	
	if (!sys::isRegularFile(filename))
	{
		cout << "File not present." << endl;
		return false;
	}
	
	sendRequest(ftpRequest(string("TYPE"), string("I")));
	ftpResponse response = recvResponse();
	cout << response.getMessage() << endl;
	
	stringstream clientInfo;
	
	clientInfo << m_control_socket.getSrcHostname() << ":" << m_data_port;
	
	sendRequest(ftpRequest(string("PORT"), clientInfo.str()));
	tcpSocket cur_data_socket = m_data_socket.accept();
	response = recvResponse();
	cout << response.getMessage() << endl;
	
	if(response.getCode() != 200)	return false;
	
	sendRequest(ftpRequest(string("STOR"), filename));
	response = recvResponse();
	cout << response.getMessage() << endl;
	
	if(response.getCode() != 150)	return false;
	
	string s;
	
	char buffer[FILE_BLOCK_SIZE];
	
	while(!f.eof())
	{
		f.read(buffer, FILE_BLOCK_SIZE);
		cur_data_socket.sendData(buffer, f.gcount());
	}
	
	cur_data_socket.close();
	
	response = recvResponse();
	cout << response.getMessage() << endl;
	
	return true;
}

void ftpClient::quit()
{
	sendRequest(ftpRequest(string("QUIT")));
	ftpResponse response = recvResponse();
	cout << response.getMessage() << endl;
}



////////////////////////////////////////////////////sys.h///////////////////


//#endif

/////////////////////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <cstring>
#include <cctype>
/*#include "../includes/sys.h"
#include "../includes/ftpClient.h"
#include "../includes/ftpResponse.h"
#include "../includes/ftpRequest.h"
#include "../includes/tcpSocket.h"*/


using namespace std;

int main(int argc, char* argv[])
{
	string hostname;
	unsigned short port = 21;
	
	if(argc<2)
	{
		printf("Specify hostname\n");
		return 0;
	}
	
	hostname = string(argv[1]);
	
	if(argc>2)
	{
		if(strlen(argv[2]) > 5)
		{
			printf("Invalid Port\n");
			return 0; 
		}
		port = 0;
		for(int i=0; i<strlen(argv[2]); i++)
		{
			if(!isdigit(argv[2][i]))
			{
				printf("Invalid Port\n");
				return 0;
			}
			port = port*10 + (argv[2][i] - '0');
		}
	}
	
	ftpClient client(hostname,port,cout);
	
	if(!client.connect())	printf("----------------Could not connect to server--------------------\n");
	else
	{	
		char username[100]="key";
		char *password="123";

	
		do
		{
			//printf("Enter Username: ");
			//fgets(username, 100, stdin);
			username[strlen(username)-1] = '\0';
			client.sendUsername(username);
			//password = getpass("Enter Password: ");
		}while(!client.sendPassword(password));
		
		if(!client.setupDataPort())
		{
			printf("Unable to setup data port\n");
			return 0;
		}
		
		string cmd = "";
		char buffer[100];
		
		while(true)
		{
			printf("ftp> ");
			*buffer=0;
			fgets(buffer, 100, stdin);
			cmd = string(strtok(buffer," \t\r\n"));
			
			if(cmd == "pwd")
			{
				client.pwd();
			}
			else if(cmd == "ls")
			{
				char* dir = strtok(NULL,"\n");
				if(dir)	client.ls(dir);
				else client.ls("");
			}
			else if(cmd == "cd")
			{
				char* dir = strtok(NULL,"\n");
				if(dir)	client.cd(dir);
			}
			else if(cmd == "get")
			{
				char* filename = strtok(NULL,"\n");
			
				filebuf fb;
	  			fb.open (filename, ios::out);
	  			ostream os(&fb);
			
				//ofstream f(filename);
				if (fb.is_open())	client.get(filename, os);
				else printf("Unable to create file\n");
				
				fb.close();
			}
			else if(cmd == "put")
			{
				char* filename = strtok(NULL,"\n");
			
				filebuf fb;
	  			fb.open (filename, ios::in);
	  			
	  			istream is(&fb);
	  			
				//ifstream f(filename);
				if (fb.is_open())	client.put(filename, is);
				else 	printf("Unable to open file\n");
				
				fb.close();
			}
			else if(cmd == "quit")
			{
				client.quit();
				break;
			}
			else if(cmd == "!pwd")
			{
				cout << sys::pwd() << endl;
			}
			else if(cmd == "!ls")
			{
				char* dir = strtok(NULL,"\n");
				if(dir)	cout << sys::ls(dir) << endl;
				else cout << sys::ls("") << endl;
			}
			else if(cmd == "!cd")
			{
				char* dir = strtok(NULL,"\n");
				if(dir)
				{
					if(sys::cd(dir))	cout << "-----------------Directory successfully changed-----------------------------" << endl;
					else	cout << "Failed to change directory." << endl;
				}
			}
			else
			{
				cout<<"Invalid command operation"<<endl;
			}		
		}
	}	
	return 0;
}
