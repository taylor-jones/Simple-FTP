/**
 * Program Name: FTP Server
 * File Name: SocketServer.hpp
 * Author: Taylor Jones
 * Last Modified: 11/20/18
 * Description: SocketServer.cpp is the class specification
 *  file for the SocketServer class. This file contains declarations
 *  for the member functions of the SocketServer class.
 */


#ifndef SocketServer_hpp
#define SocketServer_hpp

#include <string>
#include "ParsedRequest.hpp"

using std::string;


class SocketServer {
 // member variables
  private:
    const string DONE_MSG = "\\done";
    const string GOOD_MSG = "\\good";
    const string BAD_MSG = "\\bad";
    const string CANCEL_MSG = "\\cancel";
    const string QUIT_MSG = "\\quit";
    
    const string LIST_CMD = "-l";
    const string LIST_ALL_CMD = "-la";
    const string LIST_WITH_SIZE_CMD = "-ll";
    const string LIST_RECURSIVE_CMD = "-lr";
    const string GET_CMD = "-g";
    
    int controlPort;
    int controlSock;
    int dataSock;
    
    string clientHost;
    bool isRunning;
    
    
 // member functions
  private:
    int getSocket(int port);
    int getDataSocket(string host, int port);
    
    string receiveMessage(int sock);
    void receiveClientRequest(int clientSock, string clientHost);
    
    void sendMessage(int sock, string const &message);
    void sendDirectoryList(int sock, string clientHost, int dataPort, bool showHidden = false, bool showSize = false, bool showRecursive = false);
    void sendRequestedFile(int clientSock, int dataSock, string clientHost, ParsedRequest &parsedRequest);
    
    void processClientRequest(string request, int clientSock, string clientHost);
    void processDataResponse(ParsedRequest &parsedRequest, string clientHost, int clientSock);
    
  public:
    explicit SocketServer(int port);
    void start();
    void disconnect();
};


#endif /* SocketServer_hpp */
