
#include "src/Operator.h"
#include "src/Ticket.h"
int n;
string op, Time, keys, line;
int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    UserSystem UserSys;
    TrainSystem TrainSys;
    TicketSystem TicketSys;

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
            TrainSys.clean();
            cout << "0\n";
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
        if (op == "delete_train") {
            string trainID = "";
            trainID = words[3];
            TrainSys.delete_train(trainID);
        }
        if (op == "release_train") {
            string trainID = "";
            trainID = words[3];
            TrainSys.release_train(trainID);
        }
        if (op == "query_train") {
            string trainID = "", Date = "";
            for (int i = 2; i < int(words.size()); i += 2) {
                keys = words[i];
                if (keys[1] == 'i')
                    trainID = words[i + 1];
                if (keys[1] == 'd')
                    Date = words[i + 1];
            }
            TrainSys.query_train(trainID, change_date_to_num(Date));
        }

        if (op == "query_ticket") {
            string Start = "", End = "", Date = "", op = "time";
            for (int i = 2; i < int(words.size()); i += 2) {
                keys = words[i];
                if (keys[1] == 's')
                    Start = words[i + 1];
                if (keys[1] == 't')
                    End = words[i + 1];
                if (keys[1] == 'd')
                    Date = words[i + 1];
                if (keys[1] == 'p')
                    op = words[i + 1];
            }
            TrainSys.query_ticket(Start, End, change_date_to_num(Date), op);
        }

        if (op == "query_transfer") {
            string Start = "", End = "", Date = "", type = "time";
            for (int i = 2; i < int(words.size()); i += 2) {
                keys = words[i];
                if (keys[1] == 's')
                    Start = words[i + 1];
                if (keys[1] == 't')
                    End = words[i + 1];
                if (keys[1] == 'd')
                    Date = words[i + 1];
                if (keys[1] == 'p')
                    type = words[i + 1];
            }
            int op = 0;
            if (type == 'price')
                op = 1;
            TrainSys.query_transfer(Start, End, change_date_to_num(Date), op);
        }

        if (op == "")
    }

    return 0;
}