#include "Time.hpp"

int change_minute_to_num(const string &s) {
    assert(s.length() >= 5 && s[2] == ':');
    int T = 0, t = 0;
    T = (s[0] - '0') * 10 + s[1] - '0';
    t = (s[3] - '0') * 10 + s[4] - '0';
    return T * 60 + t;
}
string change_num_to_minute(int T) {
    int hour = T / 60, minute = T % 60;

    string Time = "0";
    if (hour < 10)
        Time += std::to_string(hour) + ":" + std::to_string(minute);
    else
        Time = std::to_string(hour) + ":" + std::to_string(minute);
    return Time;
}

int change_date_to_num(const string &s) {
    assert(s.length() >= 5 && s[2] == '-' && s[0] == '0');
    int sum = 0;
    if (s[1] == '7')
        sum = 30;
    else if (s[1] == '8')
        sum = 61;
    sum += (s[3] - '0') * 10 + s[4] - '0' - 1;
    return sum;
}

string change_num_to_date(int x) {
    string date;
    int m = 6, d = 1;
    if (x >= 61)
        x -= 61, m = 8;
    else if (x >= 30)
        x -= 30, m = 7;
    d += x;
    data = "0" + char('0' + m) + "-";
    if (d < 10)
        date += "0";
    else
        date += char('0' + d / 10);
    date += char('0' + d % 10);
    return date;
}