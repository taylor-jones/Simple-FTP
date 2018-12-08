/**
 * Program Name: FTP Server
 * File Name: Util.cpp
 * Author: Taylor Jones
 * Last Modified: 11/20/18
 * Description: Util.cpp contains utility function
 *  implementations for use with the FTP Server program.
 */


#include <arpa/inet.h>
#include <dirent.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include "Util.hpp"


using std::string;
using std::ifstream;
using std::istringstream;
using std::stringstream;
using std::cout;
using std::endl;


/**
 * Clears the current line in the terminal.
 */
void clearConsoleLine() {
    cout << "\33[2K\r";
}


/**
 * Checks that a string has any non-whitespace value.
 * @param content - the string to inspect
 * @return - bool - true if the string has any real value, false if not.
 * @note: This function implementation is adopted from a response to the accepted solution found at:
 * https://stackoverflow.com/questions/6444842/efficient-way-to-check-if-stdstring-has-only-spaces
 */
bool hasAnyValue(const string &content) {
    return (content.find_first_not_of(" \t\n\v\f\r") != string::npos);
}


/**
 * @brief Inspects an array of characters to determine if it represents an integer.
 * @param content - the array of characters to inspect.
 * @return bool - true if the content represents an integer, false if not.
 * @note - this solution was inspired by the following stackOverflow post:
 * https://stackoverflow.com/questions/19372145/checking-if-argvi-is-a-valid-integer-passing-arguments-in-main
 */
bool isInt(char* content) {
    while (content != 0) {
        if (!isdigit(*content++)) {
            return false;
        }
    }
    
    return true;
}


/**
 * Overloaded version of isInt that allows for passing a string to determine
 * if it represents an integer.
 * @param content - the string to inspect.
 * @return bool - true if the string represents and integer, false if not.
 * @note: This function implementation is modified from the proposed solution provided
 *  by user "Jahid" for the following slackOverflow question:
 *  https://stackoverflow.com/questions/2844817/how-do-i-check-if-a-c-string-is-an-int
 */
bool isInt(string content) {
    if (!hasAnyValue(content)) {
        return false;
    }
    
    char* nonInt;
    strtol(content.c_str(), &nonInt, 10);
    return (*nonInt == 0);
}


/**
 * Overloaded version of isInt that allows for passing an integer
 *  by reference, which will hold the converted integer version of the
 *  content string, if valid, or 0 if not.
 * @param content - the string to inspect.
 * @param value - an integer reference to hold the converted value.
 * @return bool - true if the string represents and integer, false if not.
 */
bool isInt(string content, int &value) {
    if (!hasAnyValue(content)) {
        return false;
    }
    
    char* nonInt;
    value = strtol(content.c_str(), &nonInt, 10);
    return (*nonInt == 0);
}



/**
 * @brief reads in input stream to determine if its contents represent an integer value
 * @param input - a reference to an input stream
 * @param value - a reference to an integer that will store the resulting integer value.
 * @return bool (true if the input stream value is an integer, false if not)
 * @note The design of this function was inspired by a similar example found at:
 *  https://stackoverflow.com/questions/3403632/check-if-user-input-a-float-to-a-int-var
 */
bool isInputInt(std::istream& input, int& value) {
    std::string line;
    
    if (getline(input, line) && !line.empty()) {
        char* nonInt;
        value = strtol(line.c_str(), &nonInt, 10);
        return (*nonInt == 0);
    }
    
    return false;
}



/**
 * @brief Determines if an input value is an integer that fits within a specified number range.
 * @param input - a reference to an input stream
 * @param value - a reference to an integer that will store the resulting integer value.
 * @param min - the minimum value allowed in order to be valid
 * @param max - the maximum value allowed in order to be valid
 * @return bool (true if the input stream value is an integer, false if not)
 */
bool isValidInt(std::istream& input, int& value, int min, int max) {
    return (isInputInt(input, value)) && (value >= min) && (value <= max);
}



/**
 * @brief Splits a string into a vector of string at each word.
 * @param input - the string to split
 * @return vector of strings
 */
vector<string> split(string &input) {
    vector<string>words;
    istringstream iss(input);
    
    string word;
    while (iss >> word) {
        words.push_back(word);
    }
    
    return words;
}



/**
 * Joins the items in a vector into a single string, delimited by a delimiter.
 * @param items - a vector of string items;
 * @delimiter - a delimiter to separate each item by.
 */
string join(vector<string> items, const string &delimiter) {
    stringstream ss;
    for (size_t i = 0; i < items.size(); i++) {
        if (i != 0) { ss << delimiter; }
        ss << items[i];
    }
    
    return ss.str();
}



/**
 * Removes any trailing carriage returns or newlines from a string & returns
 * a copy of the original string with the line-ending removed.
 * @param input - the string to removes endings from.
 * @return string - a copy with the line-ending(s) removed
 */
string removeLineEnding(string input) {
    while (!input.empty() &&
           (input[input.size() - 1] == '\r' || input[input.size() - 1] == '\n')) {
        input.erase(input.size() - 1);
    }
    
    return input;
}



/**
 * Retrieves the port associated with a socket connection.
 * @param sock - a socket file descriptor
 * @return int - the associated port number or 0 if not found.
 */
int portFromSocket(int sock) {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    
    if (getsockname(sock, (struct sockaddr*) &sin, &len) == 0) {
        return ntohs(sin.sin_port);
    }
    
    perror("Failed to retrieve port from socket: getsockname()");
    return 0;
}



/**
 * Creates a concatenated listing of files in a directory.
 * @param path - the relative path of the directory.
 * @return string - a list of the files in the directroy.
 * @note - This function implementation was inspired by a similar
 *  solution found at the following location:
 * https://www.tutorialspoint.com/How-can-I-get-the-list-of-files-in-a-directory-using-C-Cplusplus
 */
vector<string> getListItems(const string& path, bool includeHidden, bool includeSize) {
    vector<string> items;
    struct dirent *entry;
    string item;
    int offset;
    DIR* dir = opendir(path.c_str());
    
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (includeHidden || (entry->d_name[0] != '.')) {
              item = coloredListEntry(entry);
              
              if (includeSize) {
                offset = 40 - item.length();
                item = item + string(offset, ' ') + std::to_string(fileSize(entry));
              }

              items.push_back(item);
            }
        }

        closedir(dir);
    }
    
    return items;
}



/**
 * Recursively populates a vector reference with the directory items.
 * Note: this function implementation drew some inspiration from the following reference:
 * https://www.lemoda.net/c/recursive-directory/
 */
void getListItemsRecursive(const string& path, bool withHidden, bool withSize, vector<string> &items) {
    DIR * d = opendir (path.c_str());

    if (d != NULL) {
      while (1) {
        struct dirent *entry;
        const char *d_name;

        entry = readdir(d);
        if (!entry) break;

        d_name = entry->d_name;
        if (entry->d_type != DT_DIR) {
          items.push_back(path + "/" + coloredListEntry(entry));
        }

        if (entry->d_type & DT_DIR) {
          if (strcmp (d_name, "..") != 0 && strcmp (d_name, ".") != 0) {
            int path_length;
            char nest[PATH_MAX];
 
            path_length = snprintf (nest, PATH_MAX, "%s/%s", path.c_str(), d_name);
            items.push_back(path + "/" + coloredListEntry(entry));

            if (path_length >= PATH_MAX) break;
            getListItemsRecursive((char *)&nest, withHidden, withSize, items);
          }
        }
      }
    }
    
    closedir(d);
}



/**
 * Determines if the file at a given path can be accessed.
 * @oaram path - the relative filepath of the program directory.
 * @return bool - true if the file exists, false if not.
 */
bool canAccessFile(const string& path) {
    ifstream access(path.c_str());
    return access.good();
}



/**
 * Determines if a file represents a hidden file
 * @param entry - a dirent struct
 * @return bool - true if hidden, false if not.
 */
bool fileIsHidden(struct dirent *entry) {
  return entry->d_name[0] == '.';
}



/**
 * Determines the size of a file (in bytes).
 * @param entry - a dirent struct
 * @return the file size.
 * @note - the implementation of this function uses code from the following stackOverflow solutions:
 * https://stackoverflow.com/questions/8236/how-do-you-determine-the-size-of-a-file-in-c
 * https://stackoverflow.com/questions/5840148/how-can-i-get-a-files-size-in-c
 */
long fileSize(struct dirent *entry) {
  struct stat st;
  long rc = stat(entry->d_name, &st);
  return rc == 0 ? st.st_size : -1;
}


/**
 * @name inColor
 * @brief returns a string formatted to be displayed in a particular color & style in the terminal
 * @param content - the original string to format
 * @param foreGround - the foreground color to display the string in
 * @param backGround - the background color to display the string in
 * @param format - the format attribute to modify
 * @return string - the formatted (colored) string
 * @note The inspiration to create this function came from the following source:
 *  https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
 */
string inColor(const string content, Color foreGround, Color backGround, ColorFormat format) {
    string fgColor, bgColor, formatSet, formatReset;

    // get the foreground ansi color code
    switch (foreGround) {
        case DEFAULT_COLOR: fgColor = "39"; break;
        case BLACK: fgColor = "30"; break;
        case RED: fgColor = "31"; break;
        case GREEN: fgColor = "32"; break;
        case YELLOW: fgColor = "33"; break;
        case BLUE: fgColor = "34"; break;
        case MAGENTA: fgColor = "35"; break;
        case CYAN: fgColor = "36"; break;
        case WHITE: fgColor = "97"; break;
        case GREY: fgColor = "37"; break;
        case INVISIBLE: fgColor = ""; break;
    }

    // get the background ansi color code
    switch (backGround) {
        case DEFAULT_COLOR: bgColor = "49"; break;
        case BLACK: bgColor = "40"; break;
        case RED: bgColor = "41"; break;
        case GREEN: bgColor = "42"; break;
        case YELLOW: bgColor = "43"; break;
        case BLUE: bgColor = "44"; break;
        case MAGENTA: bgColor = "45"; break;
        case CYAN: bgColor = "46"; break;
        case WHITE: bgColor = "107"; break;
        case GREY: bgColor = "47"; break;
        case INVISIBLE: bgColor = ""; break;
    }

    // get the formatting style
    switch (format) {
        case DEFAULT_FORMAT: formatSet = "0"; formatReset = "0"; break;
        case BOLD: formatSet = "1"; formatReset = "21"; break;
        case DIM: formatSet = "2"; formatReset = "22"; break;
        case UNDERLINED: formatSet = "3"; formatReset = "24"; break;
        case BLINK: formatSet = "5"; formatReset = "25"; break;
        case REVERSE: formatSet = "7"; formatReset = "27"; break;
        case HIDDEN: formatSet = "8"; formatReset = "28"; break;
    }

    // wrap the content in the ansi color declaration and return the wrapped content
    return "\033[" + formatSet + ";" + fgColor + ";" + bgColor + "m" + content + "\033[0m";
}



/**
 * Accepts a dirent entry and uses the d_type to format the entry
 * as an ANSI-colored string.
 */
string coloredListEntry(struct dirent *entry) {
  string formatted = entry->d_name;

  switch (entry->d_type) {
      case DT_DIR: formatted = inColor(formatted, BLUE); break;
      case DT_LNK: formatted = inColor(formatted, RED); break;
      case DT_REG: formatted = inColor(formatted, WHITE); break;
  }

  if (fileIsHidden(entry)) {
    formatted = inColor(entry->d_name, MAGENTA);
  }

  return formatted;
}