
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
    string userfile1_name = "Userbase.bin", userfile2_name = "Userdata.bin";
    string trainfile1_name = "Trainbase.bin", trainfile2_name = "Traindata.bin";
    UserSystem UserSys(userfile1_name, userfile12_name);
    TrainSystem TrainSys(trainfile1_name, trainfile2_name);

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
        if (op == "add_train") {
            string trainID = "", stationNum = "", seatNum = "", stations = "",
                   prices = "", startTime = "", travelTimes = "", stopoverTimes = "",
                   saleDate = "", type = "";
            for (int i = 2; i < int(words.size()); i += 2) {
                keys = words[i];
                if (keys[1] == 'i')
                    trainID = words[i + 1];
                if (keys[1] == 'n')
                    stationNum = words[i + 1];
                if (keys[1] == 'm')
                    seatNum = words[i + 1];
                if (keys[1] == 's')
                    stations = words[i + 1];
                if (keys[1] == 'p')
                    prices = words[i + 1];
                if (keys[1] == 'x')
                    startTime = words[i + 1];
                if (keys[1] == 't')
                    travelTimes = words[i + 1];
                if (keys[1] == 'o')
                    stopoverTimes = words[i + 1];
                if (keys[1] == 'd')
                    saleDate = words[i + 1];
                if (keys[1] == 'y')
                    type = words[i + 1];
            }
            TrainSys.add_train(trainID, change_to_int(stationNum), change_to_int(seatNum),
                               stations, prices, change_to_minute(startTime), travelTimes,
                               stopoverTimes, saleDate, type);
        }
    }

    return 0;
}