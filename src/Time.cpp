#include "Time.hpp"

int change_to_minute(const string &s) {
    int T = 0, t = 0;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] == ':') {
            T = t * 60;
            t = 0;
        } else
            t = t * 10 + s[i] - '0';
    }
    return T + t;
}
string change_to_hour(int T) {
    int hour = T / 60, minute = T % 60;
    string Time = std::to_string(hour) + ":" + std::to_string(minute);
    return Time;
}

int change_to_date(const string &s){
    assert(s.length()>=5&&s[2]=='-'&&s[0]=='0');
    int x=(s[1]-'0')*100+(s[3]-'0')*10,(s[4]-'0')*10;

}