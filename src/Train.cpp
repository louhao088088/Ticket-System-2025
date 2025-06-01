#include "Train.hpp"

void TrainSystem::add_train(const string &trainID, int stationNum, int seatNum,
                            const string &stations, string &price, int startTime,
                            string &travelTimes, string &stopoverTimes, string &saleDate,
                            string &Type) {
    long long key = Hash(trainID);
    vector<int> train1 = trainBase.find(key);
    assert(train1.size() <= 1);
    if (train1.size() == 1) {
        cout << "-1\n";
        return;
    }
    Train new_train(trainID, stationNum, seatNum, stations, price, startTime, travelTimes,
                    stopoverTimes, saleDate, Type);
    Trainbase.insert(key, total);
    TrainBase.write(new_train, total);
    total++;
    // for (int i = new_train.saleDateStart; i <= saleDateEnd; i++) {
    // TimeBase.insert(i, total);
    //}
    cout << "0\n";
    return;
}

void TrainSystem::delete_train(const string &trainID) {
    long long key = Hash(trainID);
    vector<int> train1 = trainBase.find(key);
    assert(train1.size() <= 1);
    if (train1.size() == 0) {
        cout << "-1\n";
        return;
    }
    Train train;
    TrainData.read(train, train1[0]);
    if (train.release_status == 1) {
        cout << "-1\n";
        return;
    }
    TrainBase.remove(key, train1[0]);
    cout << "0\n";
    return;
}

void TrainSystem::release_train(const string &trainID) {
    long long key = Hash(trainID);
    vector<int> train1 = trainBase.find(key);
    assert(train1.size() <= 1);
    if (train1.size() == 0) {
        cout << "-1\n";
        return;
    }
    Train train;
    TrainData.read(train, train1[0]);
    if (train.release_status == 0) {
        cout << "-1\n";
        return;
    }
    train.release_status = 1;
    TrainData.write(train, train1[0]);
    cout << "0\n";
    return;
}

void TrainSystem::query_train(const string &trainID, int Date) {
    long long key = Hash(trainID);
    vector<int> train1 = trainBase.find(key);
    assert(train1.size() <= 1);
    if (train1.size() == 0) {
        cout << "-1\n";
        return;
    }
    Train train;
    TrainData.read(train, train1[0]);
    cout << train.trainID << " " << train.Type << "\n";
    int T = train.Start_time, D = dat;
    for (int i = 0; i < trian.stationNum; i++) {
    }
}