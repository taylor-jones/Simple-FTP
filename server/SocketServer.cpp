/**
 * Program Name: FTP Server
 * File Name: SocketServer.cpp
 * Author: Taylor Jones
 * Last Modified: 11/20/18
 * Description: SocketServer.cpp is the class implementation
 *  file for the SocketServer class. This file contains definitions
 *  for the member functions of the SocketServer class.
 * 
 *  This class is responsible for implementing the FTP server's response
 *  to a request from an FTP client. 
 */


#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <exception>

#include "Util.hpp"
#include "ParsedRequest.hpp"
#include "SocketServer.hpp"

using std::cout;
using std::endl;
using std::ifstream;
using std::exception;


/**
 * Constructor that uses the argued port to create a socket connection
 * which is used to wait for FTP clients.
 */
SocketServer::SocketServer(int port) {
    this->controlPort = port;
    this->controlSock = getSocket(port);
}



/**
 * Sets the FTP server in a state of waiting for connection requests from FTP clients.
 * Once a request is received, the server will trigger processing the request.
 */
void SocketServer::start() {
    this->isRunning = true;
    string clientHost;
    
    // Listen on the specified port for FTP clients.
    while (true) {
        int clientSock;
        struct sockaddr_in client;
        socklen_t sizeOfClient = sizeof(client);
        
        // Accept & validate the client connection
        if ((clientSock = accept(this->controlSock, (struct sockaddr*) &client, &sizeOfClient)) < 0) {
            perror("Error accepting client connection: accept()");
            exit(1);
        }
        
        // Receive client request
        clientHost = inet_ntoa(client.sin_addr);
        cout << "\nConnection from " << clientHost << "." << endl;
        receiveClientRequest(clientSock, clientHost);
    }
}



/**
 * Stops the FTP server, attempting to close any open sockets
 * and then exiting the program.
 */
void SocketServer::disconnect() {
    if (this->isRunning) {
        this->isRunning = false;

        try {
          close(this->dataSock);
        } catch (exception& e) {
          // do nothing
        }

        try {
          close(this->controlSock);
        } catch (exception& e) {
          // do nothing
        }   
    }

    clearConsoleLine();
    cout << "\nFTP Server stopped.\n" << endl;
    exit(1);
}



/**
 * Creates and returns a socket file descriptor. If, at any point, an error
 *  occurs, an error message is printed to the terminal & the program exits.
 * @param port - the port number to bind the socket to.
 * @return int - a valid socket connection.
 */
int SocketServer::getSocket(int port) {
    // setup the server
    struct sockaddr_in server;
    int reuse = 1;
    
    memset((char*)&server, '\0', sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
    
    // setup the socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed: socket()");
        exit(1);
    }
    
    // Allow port reuse
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        perror("Setting socket reuse option failed: setsockopt()");
        exit(1);
    }
    
    // Bind the socket
    if (bind(sock, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("Socket binding failed: bind()");
        exit(1);
    }
    
    // Start listening on the bound socket.
    if (listen(sock, 10) < 0) {
        perror("Socket listening failed: listen()");
        exit(1);
    }
    
    cout << "Server open on port " << port << endl;
    return sock;
}



/**
 * Builds and returns a data socket connection to the client.
 * @param host - the host location of the client.
 * @param port - the port number to bind the data socket to.
 * @return int - a valid socket connection.
 */
int SocketServer::getDataSocket(string host, int port) {
    // Specify the FTP client connection.
    struct sockaddr_in clientAddress;
    struct hostent *he;
    
    memset(&clientAddress, '\0', sizeof(clientAddress));
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(port);
    he = gethostbyname(host.c_str());
    
    if (he == nullptr) {
        perror("No such host: gethostbyname()");
        exit(1);
    }
    
    memcpy((char*) &clientAddress.sin_addr.s_addr, (char*) he->h_addr, he->h_length);
    
    // Create a data socket
    int dSock = socket(AF_INET, SOCK_STREAM, 0);
    if (dSock == -1) {
        perror("Data socket creation failed: socket()");
        exit(1);
    }
    
    // Connect to the FTP client using the data socket.
    if (connect(dSock, (struct sockaddr *) &clientAddress, sizeof(clientAddress)) < 0) {
        perror("Data socket connection failed: connect()");
        exit(1);
    }
    
    return dSock;
}



/**
 * Converts the string message to a c string and then
 * continuously loops until the entire message has been sent
 * using the socket connection.
 * @param message - the message to send.
 */
void SocketServer::sendMessage(int sock, string const &message) {
    string sanitized = message + "\n";  // append a newline to complete the message.
    char* buffer = (char*)sanitized.c_str();
    const size_t BUFFER_SIZE = strlen(buffer);
    size_t sent = 0;
    
    while(sent < BUFFER_SIZE) {
        sent += write(sock, buffer, BUFFER_SIZE);
    }
}



/**
 * Receives the client request and passes it on for processing.
 * @param sock - socket connection with the client from
 *  where the request can be read.
 */
string SocketServer::receiveMessage(int sock) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', BUFFER_SIZE);
    
    if (recv(sock, buffer, sizeof(buffer), 0) < 0) {
        perror("Failed to receive client message.\n");
    }
    
    string message = buffer;
    return message;
}



/**
 * Sends a listing of the current files in the directory
 * using a given socket connection.
 */
void SocketServer::sendDirectoryList(int sock, string clientHost, int dataPort, bool showHidden, bool showSize, bool showRecursive) {
    cout << "Sending directory contents to " << clientHost << ":" << dataPort << "." << endl;
    vector<string>items;

    // build a vector of the specified directory items
    if (!showRecursive) {
      items = getListItems(".", showHidden, showSize);
    } else {
      getListItemsRecursive(".", showHidden, showSize, items);
    }
    
    // send each of the resulting directory items.
    for (auto i : items) {
        sendMessage(sock, i);
    }
    
    // send a final message to indicate that the server is finished with the file list.
    sendMessage(sock, DONE_MSG);
}



/**
 * Sends the requested file (if it can be accessed).
 * Otherwise, an error message is sent.
 */
void SocketServer::sendRequestedFile(int clientSock, int dataSock, string clientHost, ParsedRequest &parsedRequest) {
    int dataPort = parsedRequest.dataPort;
    string filename = parsedRequest.filename;
    string line;
    
    // first, make sure the file exists. if not, send an error message.
    if (canAccessFile(filename)) {
        cout << "File \"" << filename << "\" ready to send to " << clientHost << ":" << dataPort << "." << endl;
        sendMessage(dataSock, this->GOOD_MSG);
        
        // wait for the client to be ready, and make sure the client doesn't cancel.
        if ((line = receiveMessage(clientSock)).find(this->CANCEL_MSG) == string::npos) {
            cout << "Sending \"" << filename << "\" to " << clientHost << ":" << dataPort << "." << endl;
            ifstream ifs(filename);
            if (ifs.good()) {
                while (ifs.peek() != EOF) {
                    getline(ifs, line);
                    sendMessage(dataSock, line);
                }
            }
            ifs.close(); // close the file once finished
            
        } else {  // indicate if the client cancelled receiving the file.
            cout << "Receiver cancelled the file transfer." << endl;
        }
        
        sendMessage(dataSock, this->DONE_MSG);  // send the DONE message to finish.
        
    } else {
        // the file couldn't be accessed, so send an error message.
        cout << "File \"" << filename << "\" not found. Sending error message to " << clientHost << ":" << dataPort << endl;
        sendMessage(dataSock, this->BAD_MSG);
        sendMessage(dataSock, "Response: Error - \"" + filename + "\" not found");
        sendMessage(dataSock, this->DONE_MSG);
    }
}



/**
 * Processes the data response after the client's request has been received
 * and validated without error.
 * @param parsedRequest - a ParsedRequest object containing all the client request information.
 */
void SocketServer::processDataResponse(ParsedRequest &parsedRequest, string clientHost, int clientSock) {
    // initiate a data connection with the client on the data port.
    receiveMessage(clientSock); // accept a message to know when the client is ready.
    int dataPort = parsedRequest.dataPort;
    int dataSock = getDataSocket(clientHost, dataPort);
    
    // use the parsedRequest information to determine what to send back to the client.
    if (parsedRequest.command == LIST_CMD) {
        sendDirectoryList(dataSock, clientHost, dataPort, false);
    } else if (parsedRequest.command == LIST_ALL_CMD) {
        sendDirectoryList(dataSock, clientHost, dataPort, true);
    } else if (parsedRequest.command == LIST_WITH_SIZE_CMD) {
        sendDirectoryList(dataSock, clientHost, dataPort, true, true);
    } else if (parsedRequest.command == LIST_RECURSIVE_CMD) {
        sendDirectoryList(dataSock, clientHost, dataPort, true, true, true);
    } else if (parsedRequest.command == GET_CMD) {
        sendRequestedFile(clientSock, dataSock, clientHost, parsedRequest);
    }
    
    // once the response has completed, close the data connection.
    close(dataSock);
    cout << "FTP data connection with " << clientHost << ":" << dataPort << " closed." << endl;
}



/**
 * Declares a ParsedRequest object which handles parsing all the request information,
 * and then either sends an error message to the client (if applicable) or continues
 * processing the client's request if the request is valid.
 * @param request - a string representing the requeste sent from the client.
 * @param clientSock - the socket file descriptor for the control connection to the client.
 */
void SocketServer::processClientRequest(string request, int clientSock, string clientHost) {
    ParsedRequest parsedRequest(request, portFromSocket(clientSock));
    string cmd = parsedRequest.command;
    
    // print a message indicating the information requested from the client.
    if (cmd == LIST_CMD || cmd == LIST_ALL_CMD || cmd == LIST_WITH_SIZE_CMD || cmd == LIST_RECURSIVE_CMD) {
        cout << "List directory requested on port " << parsedRequest.dataPort << "." << endl;
    } else if (cmd == GET_CMD) {
        cout << "File \"" << parsedRequest.filename << "\" requested on port "
        << parsedRequest.dataPort << "." << endl;
    }
    
    // If for an error flag, which indicates an invalid command.
    // If found, send the error message to the client.
    if (parsedRequest.errorFlag) {
        sendMessage(clientSock, parsedRequest.errorMessage);
    } else {
        // The client's request was valid. Begin processing the data response.
        sendMessage(clientSock, this->GOOD_MSG);
        processDataResponse(parsedRequest, clientHost, clientSock);
    }
}



/**
 * Receives the client request and passes it on for processing.
 * @param clientSock - socket connection with the client from
 *  where the request can be read.
 */
void SocketServer::receiveClientRequest(int clientSock, string clientHost) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', BUFFER_SIZE);
    
    if (recv(clientSock, buffer, sizeof(buffer), 0) < 0) {
        perror("Failed to receive client message.");
    }
    
    // Pass the request on to begin processing.
    processClientRequest(buffer, clientSock, clientHost);
}




