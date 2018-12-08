/**
 * Program Name: FTP Server
 * File Name: main.cpp
 * Author: Taylor Jones
 * Last Modified: 11/20/18
 * Description: This program implements a server program
 *  for a simple client-server FTP application.
 *
 * Sources: The following sources were especially helpful in my implementation of the FTP server:
 * - CS344 Operating Systems - Block 4 - Network Clients/Network Servers
 *   https://www.youtube.com/watch?v=1EUJboWF-tc&t=0s&index=3&list=PL-N9CTl1CH83XHo09FC6pRaZEjaQHkSQh
 *
 * - Edonix Learning Solutions - Socket Programming Tutorial
 *   https://www.youtube.com/watch?v=LtXEMwSG5-8
 *
 * - Beej's Guide to Network Programming
 *   https://beej.us/guide/bgnet/html/multi/index.html
 *
 * - GeeksForGeeks - Socket Programming in C/C++
 *   https://www.geeksforgeeks.org/socket-programming-cc/
 * 
 * - My own implementation of a Chat Client from CS 372 - Project 1.
 * 
 * - Some of the function implementations also contain notes about references
 *   that were particularly useful for a specific function.
 */


#include <iostream>
#include <functional>
#include <signal.h>

#include "Util.hpp"
#include "SocketServer.hpp"


using std::to_string;
using std::string;
using std::cout;
using std::cin;
using std::endl;


/**
 * This function allows for binding c++ class members as
 * c signal handlers. This means we can pass a SIGINT into
 * a SocketServer member function, which will then trigger
 * a disconnect on the FTP server.
 *
 * @note The idea for this concept was found here:
 * http://matetelki.com/blog/?p=636
 */
std::function<void(int)> signalWrapper;
void signalHandler(int sig) {
    signalWrapper(sig);
}



/**
 * Ensures a valid port is provided by the user. It first checks for a valid port
 *  argument. If provided, the argued port is returned. Otherwise, it continues to
 *  prompt the user for a valid port # until one is provided.
 * @param count - the # of arguments provided to the "main" function.
 * @param args - the arguments provided to the "main" function
 * @return int - a valid port #, once one is determined.
 */
int getValidPort(int count, char* args[]) {
    const int MIN_VALID_PORT = 1024;
    const int MAX_VALID_PORT = 65535;
    const string prompt = "Enter a valid port number [" +
    to_string(MIN_VALID_PORT) + " - " + to_string(MAX_VALID_PORT) + "]: ";
    
    // set a default invalid port # to check against.
    int port = -1;
    
    // check if a port argument was provided.
    if (count == 2) {
        port = atoi(args[1]);
        
        // if a valid port # was argued, use that.
        if (port >= MIN_VALID_PORT && port <= MAX_VALID_PORT) {
            return port;
        }
    }
    
    // otherwise, continuously prompt for a valid port # until one is provided.
    do {
        cout << prompt;
    } while (!isValidInt(cin, port, MIN_VALID_PORT, MAX_VALID_PORT));
    
    // once a valid port # is provided, return the valid port.
    return port;
}



int main(int argc, char* argv[]) {    
    // make sure the user has provided a valid port
    int port = getValidPort(argc, argv);

    // use the port to create the FTP server.
    SocketServer socketServer(port);
    
    /* Watch for SIGINT. If one occurs, call the signalWrapper
    to pass the signal into the SockerServer object, which will 
    trigger the stop() method of the SocketServer object. */
    signalWrapper = std::bind(&SocketServer::disconnect, &socketServer);
    struct sigaction sh;
    sh.sa_handler = signalHandler;
    sigemptyset(&sh.sa_mask);
    sh.sa_flags = 0;
    sigaction(SIGINT, &sh, NULL);
    
    
    // start the socket server
    socketServer.start();
    return 0;
}
