/**
 * Program Name: FTP Server
 * File Name: ParsedRequest.hpp
 * Author: Taylor Jones
 * Last Modified: 11/20/18
 * Description: ParsedRequest.cpp is the class specification
 *  file for the ParsedRequest class. This file contains declarations
 *  for the member functions of the ParsedRequest class.
 */


#ifndef ParsedRequest_hpp
#define ParsedRequest_hpp

#include <string>
#include <vector>

using std::string;
using std::vector;


class ParsedRequest {
  // Member Variables
  private:
    int commandPort;              // the port of the FTP command connection.
    vector<string> components;    // the request components
    
  public:
    string command;               // the command that the client sent, either -l, -la, or -g
    string filename;              // the name of the file requested (if -g command was sent)
    int dataPort;                 // the port which should be used for the FTP data transfer
    bool errorFlag;               // an indicator of an error while validating the request.
    string errorMessage;          // an message describing the error (if applicable)
    
    
  // Member Functions
  private:
    bool raiseErrorFlag(const string &message);
    bool componentCountIsValid();
    bool commandIsValid();
    bool fileNameIsValid();
    bool dataPortIsValid();
    bool parseRequest(string &request);
    
  public:
    ParsedRequest(string &request, int commandPort);
    void printRequestData();
};


#endif /* ParsedRequest_hpp */
