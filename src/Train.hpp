#pragma once
#include "BPT.hpp"
#include "MemoryRiver.hpp"

// 用户账户结构
struct Train {
    char trainID[21] = {};
    int stationNum;
    int seatNum;
    int seat[101];
    char stations[101][31] = {};

    int price[101] = {};
    int startTime;
    int travelTimes[101] = {};
    int stopoverTimes[101] = {};
    int saleDateStart, saleDateEnd;
    char Type;
    int release_status = 0;

    Train() = default;
    Train(const string &id, int num, int seatnum, const string &station, string &Price,
          int start, string &travelTime, string &stopoverTime, string &saleDate,
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
                saleDateStart = change_num_to_data(token);
            else
                saleDateEnd = change_num_to_data(token);
        }
        for (int i = saleDateStart; i <= saleDateEnd; i++)
            seat[i] = seatNum;
        Type = type[0];
    }
}

;

class TrainSystem {
    friend class TicketSystem;
    friend class UserSystem;

  private:
    int total = 0;
    BPlusTree TrainBase, ReleaseBase, TimeBase, stationBase;
    MemoryRiver<User> TrainData;
    // map<long long, bool> RealseStack;

  public:
    TrainSystem(std::string &file1, std::string &file2) : TrainBase(file1) {

        TrainData.initialise(file2);
        TrainData.get_info(total, 1);
    }

    ~TrainSystem() { TrainData.write_info(total, 1); }

    void add_train(const string &trainID, int stationNum, int seatNum,
                   const string &stations, string &price, int startTime,
                   string &travelTimes, string &stopoverTimes, string &saleDate,
                   string &Type);

    void delete_train(const string &trainID);

    void release_train(const string &trainID);

    void query_train(const string &trainID, int Date);

    void clean() {
        std::remove("TrainBase.bin");
        std::remove("TrainData.bin");
    }
};
