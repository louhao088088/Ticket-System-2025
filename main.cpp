
#include "src/User.hpp"
using std::cin;
using std::cout;
using std::ios;
using std::string;

int n;
string op, Time, keys, line;

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

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    string userfile1_name = "Userbase.bin", userfile12_name = "Userdata.bin";
    UserSystem UserSys(userfile1_name, userfile12_name);
    while (true) {
        std::getline(cin, line);

        std::istringstream iss(line);
        string word;
        vector<string> words;

        while (iss >> word) {
            words.push_back(word);
        }

        Time = words[0];
        cout << Time << " ";
        op = words[1];

        if (op == "exit") {
            cout << "bye\n";
            exit(0);
        }
        if (op == "clean") {
            UserSys.clean();
        }
        if (op == "add_user") {
            string cur_username = "", username = "", password = "", name = "",
                   mailAddr = "";
            int privilege = -1;
            readUser(words, cur_username, username, password, name, mailAddr, privilege);
            UserSys.add_user(cur_username, username, password, name, mailAddr, privilege);
        }
        if (op == "login") {
            string cur_username = "", username = "", password = "", name = "",
                   mailAddr = "";
            int privilege = -1;
            readUser(words, cur_username, username, password, name, mailAddr, privilege);
            UserSys.login(username, password);
        }
        if (op == "logout") {
            string cur_username = "", username = "", password = "", name = "",
                   mailAddr = "";
            int privilege = -1;
            readUser(words, cur_username, username, password, name, mailAddr, privilege);
            UserSys.logout(username);
        }
        if (op == "query_profile") {
            string cur_username = "", username = "", password = "", name = "",
                   mailAddr = "";
            int privilege = -1;
            readUser(words, cur_username, username, password, name, mailAddr, privilege);
            UserSys.query_profile(cur_username, username);
        }
        if (op == "modify_profile") {
            string cur_username = "", username = "", password = "", name = "",
                   mailAddr = "";
            int privilege = -1;
            readUser(words, cur_username, username, password, name, mailAddr, privilege);
            UserSys.modify_profile(cur_username, username, password, name, mailAddr,
                                   privilege);
        }
    }

    return 0;
}