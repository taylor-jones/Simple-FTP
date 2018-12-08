/*
 * Program Name: FTP Server
 * File Name: ParsedRequest.cpp
 * Author: Taylor Jones
 * Last Modified: 11/20/18
 * Description: The ParsedRequest class handles validating and parsing
 *  the FTP clients request data on the server side. It makes sure that
 *  the proper # of arguments exist and that each respective argument
 *  has the necessary information available. If any of the request 
 *  components are found to be invalid, an error message is stored
 *  and ultimately returned to the FTP client as the server's response.
 */


#include "ParsedRequest.hpp"
#include "Util.hpp"

using std::to_string;
using std::cout;
using std::endl;


/**
 * Class constructor that sets the default values for all of the member variables
 * and then triggers parsing the request.
 * @param request - the string representation of the command-line request.
 * @param commandPort - the port to establish a FTP control connection with the server.
 */
ParsedRequest::ParsedRequest(string &request, int commandPort) {
    // Apply all the default member variable values.
    this->command = "";
    this->filename = "";
    this->dataPort = -1;
    this->errorFlag = false;
    this->errorMessage = "";
    this->commandPort = commandPort;
    
    // parse the request
    this->parseRequest(request);
}



/**
 * Sets the error flag to true and specifies the error message.
 * @param message - the value to apply to the error message.
 * @return bool - always false.
 */
bool ParsedRequest::raiseErrorFlag(const string &message) {
    this->errorFlag = true;
    this->errorMessage = "Error: " + message;
    return false;
}



/**
 * Inspects the # of request components to determine if a valid amount
 * of arguments have been provided. If not, the error flag is raised,
 * and an applicable error message is set.
 * @return bool - true if a valid # of request components exist, false if not.
 */
bool ParsedRequest::componentCountIsValid() {
    const size_t componentCount = this->components.size();
    const size_t VALID_MIN = 2;
    const size_t VALID_MAX = 3;
    
    // There should be either 2 or 3 components.
    if (componentCount < VALID_MIN) {
        return this->raiseErrorFlag("Too few FTP request arguments were provided.");
    } else if (componentCount > VALID_MAX) {
        return this->raiseErrorFlag("Too many FTP request arguments were provided.");
    }
    
    return true;
}



/**
 * Checks if the first component represents a valid command
 * based on the number of total components and the allowable commands.
 * If found to be valid, the object's command is set to the value. If not,
 * an appropriate error message is set and the error flag is raised.
 * @return bool - true if valid, false if not.
 */
bool ParsedRequest::commandIsValid() {
    const string prospect = this->components[0];
    const size_t count = this->components.size();
    const string LIST_CMD = "-l";
    const string LIST_ALL_CMD = "-la";
    const string LIST_WITH_SIZE_CMD = "-ll";
    const string LIST_RECURSIVE_CMD = "-lr";
    const string GET_CMD = "-g";
    
    // check for a valid command/command-count match.
    if ((count == 2 && prospect == LIST_CMD) ||
        (count == 2 && prospect == LIST_ALL_CMD) ||
        (count == 2 && prospect == LIST_WITH_SIZE_CMD) ||
        (count == 2 && prospect == LIST_RECURSIVE_CMD) ||
        (count == 3 && prospect == GET_CMD)) {
        this->command = prospect;
        return true;
    }
    
    // check for an invalid command.
    if (prospect != LIST_CMD && 
        prospect != LIST_ALL_CMD && 
        prospect != LIST_WITH_SIZE_CMD &&
        prospect != LIST_RECURSIVE_CMD &&
        prospect != GET_CMD) {
        return this->raiseErrorFlag("An invalid command was provided. Please use \"" +
        LIST_CMD + "\", \"" + LIST_ALL_CMD + "\", \"" + LIST_WITH_SIZE_CMD + 
        "\", \"" + LIST_RECURSIVE_CMD + "\", or \"" + GET_CMD + "\".");
    }
    
    // check for a command/command-count mismatch
    if (count == 2 || count == 3) {
        return this->raiseErrorFlag("Command mismatch: " + to_string(count) +
        " arguments were provided with a command of " + prospect + ".");
    }
    
    // fall-through if # of commands is not valid. this should not happen.
    return this->raiseErrorFlag("Invalid component count: commandIsValid().");
}



/**
 * Checks if the filename argument is valid (if the -g command was argued. If not,
 * the function will return true, since no filename needs to be validated).
 * @return bool - true if valid, false if not.
 */
bool ParsedRequest::fileNameIsValid() {
    // only require validation if it was a -g command.
    if (this->command == "-g") {
        // the filename should be the second component
        const string fileName = this->components[1];
        
        // make sure the filename component has a value
        if (!hasAnyValue(fileName)) {
            return this->raiseErrorFlag("No file name was provided. Please provide one");
        }
        
        this->filename = fileName;
    }
    
    // if all inspections pass or there's not a -g command, return true.
    return true;
}



/**
 * Checks if a data port argument is valid, meaning it should:
 * - be a valid port number
 * - not be the same port number as the control connection.
 * If valid, this function with set the object's dataPort value.
 * If invalid, it will raise the appropriate error flag.
 * @return bool - true if the data port is valid, false if not.
 */
bool ParsedRequest::dataPortIsValid() {
    const int MIN_VALID_PORT = 1024;
    const int MAX_VALID_PORT = 65535;
    const size_t count = this->components.size();
    const string portComponent = this->components[count - 1];
    int dPort;
    
    // Make sure the argued data port is numeric.
    if (!isInt(portComponent, dPort)) {
        return this->raiseErrorFlag("Non-numeric data port argument. Please provide a numeric port in the range: " +
          to_string(MIN_VALID_PORT) + ".." + to_string(MAX_VALID_PORT));
    }
    
    // Make sure the argued data port is in range.
    if (dPort < MIN_VALID_PORT || dPort > MAX_VALID_PORT) {
        return this->raiseErrorFlag("Invalid data port argument. Please provide a numeric port in the range: " +
          to_string(MIN_VALID_PORT) + ".." + to_string(MAX_VALID_PORT));
    }
    
    // Make sure the argued data port is not the same as the command port
    if (dPort == this->commandPort) {
        return this->raiseErrorFlag("Invalid data port argument. The data port should not be the same as the command port.");
    }
    
    // If the data port passes inspection, set it and return true.
    this->dataPort = dPort;
    return true;
}



/**
 * Parses the elements of the request and determines if they are valid or not.
 * If found to be invalid, the error flag is raised and an error message is specified.
 * Otherwise, the respective member variables are set according to the request components.
 * @param request - the raw request string.
 * @return bool - true if the request was successfully parsed without error, false if not.
 */
bool ParsedRequest::parseRequest(string &request) {
    // Make sure the request has any real value
    if (!hasAnyValue(request)) {
        return this->raiseErrorFlag("The FTP request does not appear to have any valid arguments.");
    }
    
    // Split the request into a vector of strings.
    this->components = split(request);
    
    // validate each of the components, and return true only if all are valid.
    return (
            this->componentCountIsValid() &&
            this->commandIsValid() &&
            this->dataPortIsValid() &&
            this->fileNameIsValid()
        );
}



/**
 * Helper function to print the object data to the console.
 */
void ParsedRequest::printRequestData() {
    cout << "Err Flag:\t" << this->errorFlag << endl;
    cout << "Err Message:\t" << this->errorMessage << endl;
    cout << "Command:\t" << this->command << endl;
    cout << "FileName:\t" << this->filename << endl;
    cout << "Data Port:\t" << this->dataPort << endl;
}

