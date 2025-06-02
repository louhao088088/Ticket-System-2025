#include "Train.h"

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
    TrainBase.insert(key, total);
    TrainData.write(new_train, total);
    for (int i = 0; i < stationNum; i++) {
        long long stationKey = Hash(new_train.station[i]);
        stationBase.insert(stationKey, total);
        for (int j = i + 1; j < stationNum; j++) {
            long long Key1 = Hash(new_train.station[i] + new_train.station[j]);
            sta_to_staBase.insert(Key1, total);
        }
    }
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
    int T = train.Start_time, D = Date;
    for (int i = 0; i < trian.stationNum; i++) {
        cout << station[i] << " ";
        if (i == 0)
            cout << "xx-xx xx:xx";
        else
            cout << change_num_to_date(D) << " " << change_num_to_minute(T);
        cout << " -> ";
        T += stopoverTimes[i];
        if (T >= 1440)
            T -= 1440, D++;
        if (i == trian.stationNum - 1)
            cout << "xx-xx xx:xx";
        else
            cout << change_num_to_date(D) << " " << change_num_to_minute(T);
        cout << " " << price[i] << " " << ;
        if (i == trian.stationNum - 1)
            cout << "x\n";
        else
            cout << seat[Date][i] << "\n";
        T += travelTimes[i];
        if (T >= 1440)
            T -= 1440, D++;
    }
}
void TrainSystem::query_ticket(const string &Start, const string &End, int Date, int op) {
    int Key = Hash(Start + End);
    vector<int> train1;
    train1 = sta_to_staBase.query(Key);
    int num = train1.size();
    int tot = 0;
    vector<int> id, Price, Time, Leave, Seat;
    vector<string> TrainId;
    for (int i = 0; i < num; i++) {
        Train train;
        TrainBase(train, train1[i]);
        int T = train.Start_time, startDay = Date, P = 0, passTime = 0,
            seat = train.seatNum;

        for (int j = 0; j < train.stationNum; j++) {
            T += stopoverTimes[j];
            passTime += stopoverTimes[j];

            if (train.stations[j] == Start) {
                while (T >= 1440)
                    T -= 1440, startDay--;
                if (startDay >= saleDateStart && startDay <= saleDateEnd) {
                    passTime = 0;
                    P = -price[j];
                    seat = train.seat[startDay][j];
                    id.push_back(tot++);
                    TrainID.push_back(train.trainID);
                    Leave.push_back(T);
                } else
                    break;
            }
            if (train.stations[j] == End) {
                P += price[j];
                Time.push_back(passTime);
                Price.push_back(P);
                Seat.push_back(seat);
                break;
            }
            T += travelTimes[j];
            passTime += stopoverTimes[j];
            seat = min(seat, train.seat[startDay][j]);
        }
    }
    cout << tot << "\n";
    if (op == 0)
        std::sort(id.begin(), id.end(), [](const int &a, const int &b) {
            if (Time[a] == Time[b])
                return TrainID[a] < TrainID[b];
            return Time[a] < Time[b];
        });
    else if (op == 1)
        std::sort(id.begin(), id.end(), [](const int &a, const int &b) {
            if (Price[a] == Price[b])
                return TrainID[a] < TrainID[b];
            return Price[a] < Price[b];
        });
    for (int i = 0; i < tot; i++) {
        int ID = id[i];
        cout << TrainID[ID] << " " << Start << " " << change_num_to_date(Date) << " ";
        cout << change_num_to_minute(Leave[ID]) << " -> " << End;
        int T = Leave[ID] + Time[ID], D = Date;
        while (T >= 1440)
            T -= 1440, D++;
        cout << change_num_to_date(Date) << " " << change_num_to_minute(T) << " "
             << Price[ID] << " " << Seat[ID] << "\n";
    }
}
void TrainSystem::query_transfer(const string &Start, const string &End, int Date,
                                 int op) {
    int Key = Hash(Start);
    vector<int> train1;
    train1 = stationBase.query(Key);
    int num = train1.size();
    int tot = 0;
    int Price = -1, Time = -1, LeaveMinute1 = -1, ArriveMinute1 = -1, LeaveMinute2 = -1,
        ArriveMinute2 = -1, Seat1 = -1, Seat2 = -1, ArriveDay = -1, LeaveDay = Date,
        Price1, Price2;
    ;
    string Train1Id = "", Train2ID = "", midStation = "";
    for (int i = 0; i < num; i++) {
        Train Train1;
        TrainBase(Train1, train1[i]);
        int T = Train1.Start_time, startDay = Date, P = 0, F = 0, passTime = 0,
            seat = Train1.seatNum, leave, D = Date;

        for (int j = 0; j < Train1.stationNum; j++) {
            T += stopoverTimes[j];

            if (Train1.stations[j] == Start) {

                while (T >= 1440)
                    T -= 1440, startDay--;
                if (startDay >= saleDateStart && startDay <= saleDateEnd) {
                    F = 1;
                    P = -Train1.price[j];
                    seat = Train1.seat[startDay][j];
                    leave = T;
                } else
                    break;
            }
            if (F) {
                P += Train1.price[i];
                while (T >= 1440)
                    T -= 1440, D++;
                long long KEY2 = Hash(stations[j] + End);
                vector<int> train2 = sta_to_staBase.query(Key);
                int num2 = train2.size();
                for (int k = 0; k < num2; k++) {
                    Train Train2;
                    TrainBase(Train2, train2[k]);
                    if (Train2.trainID == Train1.ID)
                        continue;
                    int T2 = Train2.Start_time, startDay2 = D, seat2, P2 = P, D2 = D;
                    int leave2, leaveD2;
                    for (int l = 0; l < Train2.stationNum; l++) {
                        T2 += stopoverTimes[l];

                        if (Train2.stations[l] == Train1.station[j]) {

                            while (T2 >= 1440)
                                T2 -= 1440, startDay2--;
                            if (T2 < T)
                                startDay++, D2++;
                            if (startDay < saleDateStart)
                                D2 += saleDateStart - startDay, startDay = saleDateStart;

                            leaveD2 = D2, leave2 = T2;
                            if (startDay >= saleDateStart && startDay <= saleDateEnd) {
                                P2 -= Train2.price[l];
                                seat2 = Train2.seat[startDay][l];

                            } else
                                break;
                        }
                        if (Train2.stations[l] == End) {
                            while (T2 >= 1440)
                                T2 -= 1440, D2++;
                            P2 += Train2.price[l];
                            int totalT = (D2 - Date) * 1440 + T2 - leave;
                            int G = 0;
                            if (Price == -1)
                                G = 1;
                            else if (op == 0 && Time > totalT)
                                G = 1;
                            else if (op == 1 && P2 < Price)
                                G = 1;
                            else if (Time == totalT && P2 < Price)
                                G = 1;
                            else if (Time < totalT && P2 == Price)
                                G = 1;
                            if (Time == totalT && P2 == Price &&
                                Train1.trainID < TrainID1)
                                G = 1;
                            else if (Time == totalT && P2 == Price &&
                                     Train1.trainID == TrainID1 &&
                                     Train2.trainID < TrainID2)
                                G = 1;
                            if (G == 1) {
                                Time = totalT, Price = P2;
                                Price1 = P, Price2 = P2 - P;
                                TrainID1 = Train1.trainID;
                                TrainID2 = Train2.TrainID;
                                LeaveMinute1 = leave, ArriveMinute2 = T2;
                                LeaveMinute2 = leave2, ArriveMinute1 = T;
                                LeaveDate1 = Date, ArriveDate2 = D2;
                                LeaveMinute2 = leaveD2, ArriveDate1 = D;
                                Seat1 = seat, Seat2 = seat2;
                                ArriveDay2 = D2;
                                midStation = Train1.station[j];
                            }
                            break;
                        }
                        T2 += travelTimes[l];
                        seat2 = min(seat2, Train2.seat[startDay][l]);
                    }
                }
                P -= Train1.price[i];
            }
            T += travelTimes[j];
            seat = min(seat, train.seat[startDay][j]);
        }
    }
    if (Time == -1) {
        cout << "0\n";
        return;
    }
    cout << TrainID1 << " " << Start << " " << change_num_to_Date(leaveDate1) << " "
         << change_num_to_minute(leaveMinute1) << " -> " << midStation << " "
         << change_num_to_Date(arriveDate1) << " " << change_num_to_minute(arriveMinute1)
         << Price1 << " " << Seat1 << "\n";
    cout << TrainID2 << " " << midStation << " " << change_num_to_Date(leaveDate2) << " "
         << change_num_to_minute(leaveMinute2) << " -> " << End << " "
         << change_num_to_Date(arriveDate2) << " " << change_num_to_minute(arriveMinute2)
         << Price2 << " " << Seat2 << "\n";
}
