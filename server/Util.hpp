/**
 * Program Name: FTP Server
 * File Name: Util.hpp
 * Author: Taylor Jones
 * Last Modified: 11/20/18
 * Description: Util.hpp contains utility function
 *  declarations for use with the FTP Server program.
 */

#ifndef UTIL_HPP
#define UTIL_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using std::istream;
using std::string;
using std::vector;



void clearConsoleLine();
bool hasAnyValue(const string &content);

bool isInt(char* content);
bool isInt(string content);
bool isInt(string content, int &value);

bool isInputInt(istream& input, int& value);
bool isValidInt(istream& input, int& value, int min, int max);

vector<string> split(string &input);
string join(vector<string> items, const string &delimiter);
string removeLineEnding(string input);

int portFromSocket(int sock);

vector<string> getListItems(const string& path, bool includeHidden = false, bool includeSize = false);
void getListItemsRecursive(const string& path, bool withHidden, bool withSize, vector<string> &items);

bool canAccessFile(const string& path);
bool fileIsHidden(struct dirent *entry);
long fileSize(struct dirent *entry);

enum Color { BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, GREY, DEFAULT_COLOR, INVISIBLE };
enum ColorFormat { DEFAULT_FORMAT, BOLD, DIM, UNDERLINED, BLINK, REVERSE, HIDDEN };
string inColor(string content = "", Color foreGround = DEFAULT_COLOR, Color backGround = DEFAULT_COLOR, ColorFormat format = DEFAULT_FORMAT);
string coloredListEntry(struct dirent *entry);

#endif //UTIL_HPP
