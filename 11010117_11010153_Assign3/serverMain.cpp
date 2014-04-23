

///////////////////////////////////../includes/auth.h//////////////////////////////

//#ifndef H_AUTH
////#define H_AUTH

#include <vector>
#include <fstream>
#include <string>

using namespace std;

class auth
{
	struct userInfo
	{
		string username;
		string password;
		string directory;
	};
	
	vector<userInfo> users;
	int cur_user_index;
	
public:
	auth();
	bool login(string name, string passwd);
	userInfo getCurUser();
	bool isLoggedin();
};

//#endif




///////////////////////////////////////////////////////////////////////////////
//#include "../includes/auth.h"

auth::auth()
{
	cur_user_index = -1;
	
	ifstream fin("auth");
	userInfo user;
	while(fin >>  user.username >> user.password >> user.directory)
	{
		users.push_back(user);
	}
}

bool auth::login(string name, string passwd)
{
	if(name == "anonymous")
	{
		cur_user_index = 0;
		return true;
	}
	for(vector<userInfo>::iterator it = users.begin(); it != users.end(); it++)
	{
		if(it->username == name && it->password == passwd)
		{
			cur_user_index = it - users.begin();
			return true;
		}
		cur_user_index = it - users.begin();
			return true;
	}
	cur_user_index = -1;
	return false;
}

auth::userInfo auth::getCurUser()
{
	if(cur_user_index == -1)	throw string("Invalid user!");
	else	return users[cur_user_index];
}

bool auth::isLoggedin()
{
	return cur_user_index != -1;
}




/////////////////////////////////////includes/tcpSocket.h/////////////////////

//#ifndef H_TCPSOCKET
//#define H_TCPSOCKET

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

//#endif



////////////////////////////////////////////////////////////////////////////////
//#include "../includes/tcpSocket.h"

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


///////////////////////////////includes/ftpRequest.h/////////////////////////////////

//#ifndef H_FTPREQUEST
//#define H_FTPREQUEST

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



////////////////////////////includes/ftpServer.h////////////////////////////////

//#ifndef H_FTPSERVER
//#define H_FTPSERVER

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
/*#include "ftpRequest.h"
#include "ftpResponse.h"
#include "tcpSocket.h"
#include "auth.h"*/

using namespace std;

class ftpServer
{
private:
	unsigned short m_port;
	tcpSocket m_control_sock;
	tcpSocket m_data_sock;
	auth Auth;
public:
	ftpServer(int port);
	
	bool start();
	void serveClient(tcpSocket);
	bool processRequest(ftpRequest&, tcpSocket&);
	
};

//#endif



///////////////////////includes/sys.h///////////////////////////////////////////////



//#ifndef H_SYS
//#define H_SYS

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

//#endif




/////////////////////////////////////////////////////////////////////////////////////
/*#include "../includes/ftpServer.h"
#include "../includes/sys.h"*/
#include <cstring>
/*

How the server works:

Main process listens on port 21 for incoming connections.
When a connection is received, a new process is forked to handle that client,
and this process exits when it has finished serving the client.

Sockets:
m_control_sock: This is the socket used to listen for incoming connections
				Its value is same for main process and forked processes.

m_data_sock: Set up when a client sends a PORT request.
			 Each forked process will have its own data socket (values will be different).

client_control_sock: This is passed to the function serveClient. It refers to
					 the connection between port 21 of server and control port of a specific client.

*/

ftpServer::ftpServer(int port):
	m_port(port)
{}

bool ftpServer::start()
{
	if (!(m_control_sock.bind(m_port) && m_control_sock.listen())) return false;
	
	cout<<"Server running ..."<<endl;
	
	while (true)
	{
		tcpSocket client_sock = m_control_sock.accept();
		
		cout << "Got connection from " << client_sock.getDestHostname() << ":" << client_sock.getDestPort() << endl;
		
		if (!fork())
		{
			//child
			serveClient(client_sock);
			exit(0);
		}
	}
}

void ftpServer::serveClient(tcpSocket client_control_sock)
{
	string sreq;
	ftpRequest req;
	
	//connection ack
	client_control_sock.sendString( ftpResponse(220, "Welcome to the Server").toString() );
	
	while ((sreq = client_control_sock.recvString()).length() > 0)
	{
		ftpRequest req = ftpRequest::parseFtpRequest(sreq);
		if (!processRequest( req, client_control_sock)) break;
	}
	
}

// Processes request and returns false if no further requests from client are expected (QUIT command)
bool ftpServer::processRequest(ftpRequest& req, tcpSocket& client_control_sock)
{
	if (req.getCmd() == "USER")
	{
		client_control_sock.sendString(ftpResponse(331, "-----------------------------").toString());
		ftpRequest passwd = ftpRequest::parseFtpRequest(client_control_sock.recvString());
		
		if(Auth.login(req.getArg(),passwd.getArg()))
		{
			client_control_sock.sendString(ftpResponse(230, "##############################").toString());
			sys::cd(Auth.getCurUser().directory);
			//cout << sys::setRootDir(Auth.getCurUser().directory) << endl;
			//sys::cd("/");
		}
		else
		{
			client_control_sock.sendString(ftpResponse(530, "Try again ").toString());
		}
	}
	else if(!Auth.isLoggedin())
	{
		client_control_sock.sendString(ftpResponse(530, "Try again").toString());
	}
	else if (req.getCmd() == "PWD")
	{
		client_control_sock.sendString( ftpResponse(257, sys::pwd()).toString() );
	}
	else if (req.getCmd() == "CWD")
	{
		if (sys::cd(req.getArg()))
		{
			client_control_sock.sendString( ftpResponse(250, "==================Directory successfully changed.======================").toString() );
		}
		else
		{
			client_control_sock.sendString( ftpResponse(550, "==========================Failed to change directory.===================").toString() );
		}
	}
	else if (req.getCmd() == "LIST")
	{
		client_control_sock.sendString( ftpResponse(150, "---------------Directory List-----------------").toString() );
		string result = sys::ls(req.getArg());
		m_data_sock.sendString(result);
		m_data_sock.close();
		client_control_sock.sendString( ftpResponse(226, "...................................").toString() );
	}
	else if (req.getCmd() == "RETR")
	{
		if (!sys::isRegularFile(req.getArg()))
		{
			m_data_sock.close();
			client_control_sock.sendString( ftpResponse(550, "File not present.").toString() );
			
		}
		else
		{
			ifstream f(req.getArg().c_str());
			if (f.is_open())
			{
				client_control_sock.sendString( ftpResponse(150, "...................File coming....................").toString() );
				char buffer[1024];
			
				while (!f.eof())
				{
					f.read(buffer, 1024);
					m_data_sock.sendData(buffer, f.gcount());
				}
				f.close();
				m_data_sock.close();
				client_control_sock.sendString( ftpResponse(226, "File Transfer successful").toString() );
			}
			else
			{
				m_data_sock.close();
				client_control_sock.sendString( ftpResponse(550, "File not found.").toString() );
			}
		}
	}
	else if (req.getCmd() == "STOR")
	{
		ofstream f(req.getArg().c_str());
		if (f.is_open())
		{
			client_control_sock.sendString( ftpResponse(150, "Sending date............").toString() );
			char buffer[1024];
			int n;
			while ((n=m_data_sock.recvData(buffer, 1024))>0)
			{
				f.write(buffer, n);
			}
			f.close();
			m_data_sock.close();
			client_control_sock.sendString( ftpResponse(226, "Transfer successful.").toString() );
		}
		else
		{
			m_data_sock.close();
			client_control_sock.sendString( ftpResponse(550, "Failed to create file.").toString() );
		}
	}
	else if (req.getCmd() == "SYST")
	{
		client_control_sock.sendString(ftpResponse(215, sys::syst()).toString());
	}
	else if (req.getCmd() == "TYPE")
	{
		client_control_sock.sendString(ftpResponse(200, "Mode switched.").toString());
	}
	else if (req.getCmd() == "PORT")
	{
		bool success = true;
		string hostname;
		unsigned short port;
		string sarg = req.getArg();
		char* arg = new char[sarg.length()];
		strcpy(arg, sarg.c_str());

		char* tok = strtok(arg, ":");
		if (tok) hostname = string(tok); else success = false;

		tok = strtok(NULL, ":");
		if (tok) port = atoi(tok); else success = false;

		if (success && m_data_sock.connect(hostname, port))
		{
			client_control_sock.sendString( ftpResponse(200, "VALID PORT").toString() );
		}
		else
		{
			client_control_sock.sendString( ftpResponse(500, "WRONG PORT").toString() );
		}

	}
	else if (req.getCmd() == "QUIT")
	{
		client_control_sock.sendString( ftpResponse(221, "Disconnecting..............").toString() );
		return false;
	}
	
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
#include <cstring>
#include <cctype>
#include <cstdio>
//#include "../includes/ftpServer.h"
using namespace std;

int main(int argc, char* argv[])
{
	unsigned short port;
	if(argc < 2)
	{
		port = 21;
	}
	else
	{
		if(strlen(argv[1]) > 5)
		{
			printf("Invalid Port\n");
			return 0; 
		}
		port = 0;
		for(int i=0; i<strlen(argv[1]); i++)
		{
			if(!isdigit(argv[1][i]))
			{
				printf("Invalid Port\n");
				return 0;
			}
			port = port*10 + (argv[1][i] - '0');
		}
	}
	
	ftpServer serv = ftpServer(port);
	if (!serv.start()){
		cout<<"Server is not starting Try another port."<<endl;
	}
	
	return 0;
}

