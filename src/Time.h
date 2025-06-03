#pragma once
#include <algorithm>
#include <cassert>
#include <string>
using std::string;

int change_minute_to_num(const string &s);
string change_num_to_minute(int T);
int change_date_to_num(const string &s);
string change_num_to_date(int x);