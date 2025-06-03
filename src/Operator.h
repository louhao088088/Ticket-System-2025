#pragma once
#include "BPT.hpp"

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

using std::cin;
using std::cout;
using std::ios;
using std::string;

void readUser(vector<string> words, string &cur_username, string &username,
              string &password, string &name, string &mailAddr, int &privilege);