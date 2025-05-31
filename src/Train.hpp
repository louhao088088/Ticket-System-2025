#pragma once
#include "BPT.hpp"
#include "MemoryRiver.hpp"

// 用户账户结构
struct Train {
    char trainID[21] = {};
    int stationNum;
    int seatNum;
    char stations[101][31] = {};

    int price[101] = {};
    int startTime;
    int travelTimes[101] = {};
    int stopoverTimes[101] = {};
    int saleDateStart, saleDateEnd;
    char Type;
    int status = 0;

    User() = default;
    User(const string &id, int &num, int &seatnum, const string &station, string &Price,
         int &start, string &travelTime, string &stopoverTime, string &saleDate,
         string &type)
        : stationNum(num), seatNum(seatnum), startTime(start) {
        strncpy(train_id, id.c_str(), 21);
        string token;

        for (int i = 0; i < num; i++) {
            std::getline(station, token, '|');
            std::strncpy(result[i], token.c_str(), 31);
        }
        for (int i = 0; i < num - 1; i++) {
            std::getline(Price, token, '|');
            price[i] = change_to_int(token);
        }
        for (int i = 0; i < num - 1; i++) {
            std::getline(travelTime, token, '|');
            travalTimes[i] = change_to_int(token);
        }
        for (int i = 0; i < num - 1; i++) {
            std::getline(stopoverTime, token, '|');
            stopoverTimes[i] = change_to_int(token);
        }
        for (int i = 0; i < 2; i++) {
            std::getline(stopoverTime, token, '|');
            if (i == 0)
                saleDateStart = change_to_data(token);
            else
                saleDateEnd = change_to_data(token);
        }
        Type = type[0];
    }
}

;

class TrainSystem {
    friend class TicketSystem;
    friend class UserSystem;

  private:
    int total = 0;
    BPlusTree TrainBase;
    MemoryRiver<User> TrainData;
    // map<long long, bool> RealseStack;

  public:
    TrainSystem(std::string &file1, std::string &file2) : TrainBase(file1) {

        TrainData.initialise(file2);
        TrainData.get_info(total, 1);
    }

    ~TrainSystem() { UserData.write_info(total, 1); }

    void add_train(const string &id, int &num, int &seatnum, const string &station,
                   string &Price, int &start, string &travelTime, string &stopoverTime,
                   string &saleDate, string &type);

    void login(string &username, string &password);

    void logout(string &username);

    void query_profile(string &cur_username, string &username);

    void modify_profile(string &cur_username, string &username, string &password,
                        string &name, string &mailAddr, int &privilege);

    void clean() {
        std::remove("TrainBase.bin");
        std::remove("TrainData.bin");
        LoginStack.clear();
    }
};
