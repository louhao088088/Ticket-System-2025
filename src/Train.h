#pragma once
#include "BPT.hpp"
#include "MemoryRiver.hpp"
#include "Time.h"

// 火车信息结构
struct Train {
    char trainID[21] = {};
    int stationNum;
    int seatNum;
    int seat[101][101];
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
        strncpy(trainID, id.c_str(), 21);
        string token;
        std::istringstream STATION(station);
        for (int i = 0; i < num; i++) {

            std::getline(STATION, token, '|');
            std::strncpy(stations[i], token.c_str(), 31);
        }
        std::istringstream PRICE(Price);
        for (int i = 0; i < num - 1; i++) {
            std::getline(PRICE, token, '|');
            price[i + 1] = change_string_to_int(token) + price[i];
        }
        std::istringstream TRAVELTIME(travelTime);
        for (int i = 0; i < num - 1; i++) {
            std::getline(TRAVELTIME, token, '|');
            travelTimes[i] = change_string_to_int(token);
        }
        std::istringstream STOPOVERTIME(stopoverTime);
        for (int i = 1; i < num - 1; i++) {
            std::getline(STOPOVERTIME, token, '|');
            stopoverTimes[i] = change_string_to_int(token);
        }

        std::istringstream SALEDATE(saleDate);
        for (int i = 0; i < 2; i++) {
            std::getline(SALEDATE, token, '|');
            if (i == 0)
                saleDateStart = change_date_to_num(token);
            else
                saleDateEnd = change_date_to_num(token);
        }
        assert(saleDateStart >= 0 && saleDateEnd <= 100);
        for (int i = saleDateStart; i <= saleDateEnd; i++)
            for (int j = 0; j < stationNum - 1; j++)
                seat[i][j] = seatNum;
        Type = type[0];
    }
}

;

class TrainSystem {
    friend class TicketSystem;

  private:
    int total = 0;
    BPlusTree TrainBase, stationBase, sta_to_staBase;
    MemoryRiver<Train> TrainData;

  public:
    TrainSystem()
        : TrainBase("TrainBase.bin"), stationBase("stationBase.bin"),
          sta_to_staBase("sta_to_staBase.bin") {
        TrainData.initialise("TrainData.bin");
        TrainData.get_info(total, 1);
    }

    ~TrainSystem() {
        TrainData.write_info(total, 1);
        TrainBase.flush();
        stationBase.flush();
        sta_to_staBase.flush();
    }

    void add_train(const string &trainID, int stationNum, int seatNum,
                   const string &stations, string &price, int startTime,
                   string &travelTimes, string &stopoverTimes, string &saleDate,
                   string &Type);

    void delete_train(const string &trainID);

    void release_train(const string &trainID);

    void query_train(const string &trainID, int Date);

    void query_ticket(const string &Start, const string &End, int Date, int flag);

    void query_transfer(const string &Start, const string &End, int Date, int flag);

    void clean() {
        std::remove("TrainBase.bin");
        std::remove("TrainData.bin");
        std::remove("stationBase.bin");
        std::remove("sta_to_staBase.bin");
    }
};
