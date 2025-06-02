#include "Operator.h"

int n;
string op, Time, keys;

void readUser(vector<string> words, string &cur_username, string &username,
              string &password, string &name, string &mailAddr, int &privilege) {
    for (int i = 2; i < int(words.size()); i += 2) {
        keys = words[i];
        if (keys[1] == 'c')
            cur_username = words[i + 1];
        if (keys[1] == 'u')
            username = words[i + 1];
        if (keys[1] == 'p')
            password = words[i + 1];
        if (keys[1] == 'm')
            mailAddr = words[i + 1];
        if (keys[1] == 'n')
            name = words[i + 1];
        if (keys[1] == 'g') {
            if (words[i + 1].length() == 2)
                privilege = 10;
            else
                privilege = words[i + 1][0] - '0';
        }
    }
}
