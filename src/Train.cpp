#include "Train.h"

#include "Time.h"

void TrainSystem::add_train(const string &trainID, int stationNum, int seatNum,
                            const string &stations, string &price, int startTime,
                            string &travelTimes, string &stopoverTimes, string &saleDate,
                            string &Type) {
    long long key = Hash(trainID);
    vector<int> train1 = TrainBase.find(key);
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
        long long stationKey = Hash(new_train.stations[i]);
        stationBase.insert(stationKey, total);
        for (int j = i + 1; j < stationNum; j++) {

            long long Key1 =
                Hash((string) new_train.stations[i] + (string) new_train.stations[j]);
            sta_to_staBase.insert(Key1, total);
        }
    }
    total++;

    cout << "0\n";
    return;
}

void TrainSystem::delete_train(const string &trainID) {
    long long key = Hash(trainID);
    vector<int> train1 = TrainBase.find(key);
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
    vector<int> train1 = TrainBase.find(key);
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
    train.release_status = 1;
    TrainData.write(train, train1[0]);
    cout << "0\n";
    return;
}

void TrainSystem::query_train(const string &trainID, int Date) {
    long long key = Hash(trainID);
    vector<int> train1 = TrainBase.find(key);
    assert(train1.size() <= 1);
    if (train1.size() == 0) {
        cout << "-1\n";
        return;
    }
    Train train;
    TrainData.read(train, train1[0]);

    // cout << Date << " " << train.saleDateStart << " " << train.saleDateEnd << "\n";

    if (train.saleDateStart > Date || train.saleDateEnd < Date) {
        cout << "-1\n";
        return;
    }
    assert(Date >= 0 && Date <= 100);
    cout << train.trainID << " " << train.Type << "\n";
    int T = train.startTime, D = Date;

    for (int i = 0; i < train.stationNum; i++) {
        cout << train.stations[i] << " ";
        if (i == 0)
            cout << "xx-xx xx:xx";
        else
            cout << change_num_to_date(D) << " " << change_num_to_minute(T);
        cout << " -> ";
        T += train.stopoverTimes[i];
        if (T >= 1440)
            T -= 1440, D++;
        if (i == train.stationNum - 1)
            cout << "xx-xx xx:xx";
        else
            cout << change_num_to_date(D) << " " << change_num_to_minute(T);
        cout << " " << train.price[i] << " ";
        if (i == train.stationNum - 1)
            cout << "x\n";
        else {
            assert(Date >= 0 && Date <= 100);
            cout << train.seat[Date][i] << "\n";
        }

        T += train.travelTimes[i];
        if (T >= 1440)
            T -= 1440, D++;
    }
}

struct MyCompare {
    vector<int> &A;
    vector<string> &trainID;
    MyCompare(vector<int> &T, vector<string> &TID) : A(T), trainID(TID) {}
    bool operator()(int a, int b) const {
        if (A[a] == A[b])
            return trainID[a] > trainID[b];
        return A[a] > A[b];
    }
};

void TrainSystem::query_ticket(const string &Start, const string &End, int Date,
                               int flag) {
    long long Key = Hash(Start + End);
    vector<int> train1;
    train1 = sta_to_staBase.find(Key);
    int num = train1.size();
    int tot = 0, F = 0;
    vector<int> id, Price, Time, Leave, Seat;
    vector<string> TrainId;
    cerr << num << "\n";
    for (int i = 0; i < num; i++) {
        Train train;
        TrainData.read(train, train1[i]);
        if (train.release_status == 0)
            continue;
        int T = train.startTime, startDay = Date, P = 0, passTime = 0,
            seat = train.seatNum;
        // cout << change_num_to_minute(T) << "\n";
        for (int j = 0; j < train.stationNum; j++) {
            if (train.stations[j] == End) {
                P += train.price[j];
                Time.push_back(passTime);
                Price.push_back(P);
                Seat.push_back(seat);
                break;
            }
            T += train.stopoverTimes[j];
            passTime += train.stopoverTimes[j];
            // cout << change_num_to_minute(T) << " " << passTime << "\n";
            if (train.stations[j] == Start) {
                while (T >= 1440)
                    T -= 1440, startDay--;
                if (startDay >= train.saleDateStart && startDay <= train.saleDateEnd) {
                    passTime = 0;
                    F = 1;
                    P = -train.price[j];
                    assert(startDay >= 0 && startDay <= 100);
                    seat = train.seat[startDay][j];

                    id.push_back(tot++);
                    TrainId.push_back(train.trainID);
                    Leave.push_back(T);
                } else
                    break;
            }

            T += train.travelTimes[j];
            passTime += train.travelTimes[j];
            if (F) {
                assert(startDay >= 0 && startDay <= 100);
                seat = std::min(seat, train.seat[startDay][j]);
            }
        }
        // cout << change_num_to_date(startDay) << " \n";
    }
    cerr << "A\n"
         << " " << tot << " " << flag << "\n";
    for (int i = 0; i < id.size(); i++)
        cerr << id[i] << "\n";

    cout << tot << "\n";

    if (flag == 0) {
        MyCompare cmp(Time, TrainId);
        priority_queue<int, MyCompare> pq(cmp);
        for (int i = 0; i < tot; i++)
            pq.push(i);
        for (int i = 0; i < tot; i++) {
            id[i] = pq.top();
            pq.pop();
        }
    } else if (flag == 1) {
        MyCompare cmp(Price, TrainId);
        priority_queue<int, MyCompare> pq(cmp);
        for (int i = 0; i < tot; i++)
            pq.push(i);
        for (int i = 0; i < tot; i++) {
            id[i] = pq.top();
            pq.pop();
        }
    }

    for (int i = 0; i < tot; i++) {
        int ID = id[i];
        cout << TrainId[ID] << " " << Start << " " << change_num_to_date(Date) << " ";
        cout << change_num_to_minute(Leave[ID]) << " -> " << End << " ";
        int T = Leave[ID] + Time[ID], D = Date;
        while (T >= 1440)
            T -= 1440, D++;
        cout << change_num_to_date(D) << " " << change_num_to_minute(T) << " "
             << Price[ID] << " " << Seat[ID] << "\n";
    }
}
void TrainSystem::query_transfer(const string &Start, const string &End, int Date,
                                 int flag) {
    long long Key = Hash(Start);
    vector<int> train1;
    train1 = stationBase.find(Key);
    int num = train1.size();

    int Price = -1, Time = -1, LeaveMinute1 = -1, ArriveMinute1 = -1, LeaveMinute2 = -1,
        ArriveMinute2 = -1, Seat1 = -1, Seat2 = -1, ArriveDate1 = -1, LeaveDate1 = Date,
        ArriveDate2 = -1, LeaveDate2 = -1, Price1, Price2;
    ;
    string TrainID1 = "", TrainID2 = "", midStation = "";
    for (int i = 0; i < num; i++) {
        Train Train1;
        TrainData.read(Train1, train1[i]);
        if (Train1.release_status == 0)
            continue;
        int T = Train1.startTime, startDay = Date, P = 0, F = 0, seat = Train1.seatNum,
            leave, D = Date;

        for (int j = 0; j < Train1.stationNum; j++) {

            if (F) {
                P += Train1.price[j];
                while (T >= 1440)
                    T -= 1440, D++;
                long long KEY2 = Hash(Train1.stations[j] + End);
                vector<int> train2 = sta_to_staBase.find(KEY2);
                int num2 = train2.size();
                for (int k = 0; k < num2; k++) {
                    Train Train2;

                    TrainData.read(Train2, train2[k]);
                    if (Train2.release_status == 0)
                        continue;
                    if ((string) Train2.trainID == (string) Train1.trainID)
                        continue;
                    int T2 = Train2.startTime, startDay2 = D, seat2, P2 = P, D2 = D;
                    int leave2, leaveD2, F2 = 0;
                    for (int l = 0; l < Train2.stationNum; l++) {

                        if (Train2.stations[l] == End) {
                            while (T2 >= 1440)
                                T2 -= 1440, D2++;
                            P2 += Train2.price[l];
                            int totalT = (D2 - Date) * 1440 + T2 - leave;
                            int G = 0;
                            if (Price == -1)
                                G = 1;
                            else if (flag == 0 && Time > totalT)
                                G = 1;
                            else if (flag == 1 && P2 < Price)
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
                                TrainID2 = Train2.trainID;
                                LeaveMinute1 = leave, ArriveMinute2 = T2;
                                LeaveMinute2 = leave2, ArriveMinute1 = T;
                                LeaveDate1 = Date, ArriveDate2 = D2;
                                LeaveDate2 = leaveD2, ArriveDate1 = D;
                                Seat1 = seat, Seat2 = seat2;
                                ArriveDate2 = D2;
                                midStation = Train1.stations[j];
                            }
                            break;
                        }

                        T2 += Train2.stopoverTimes[l];

                        if ((string) Train2.stations[l] == (string) Train1.stations[j]) {

                            while (T2 >= 1440)
                                T2 -= 1440, startDay2--;
                            if (T2 < T)
                                startDay++, D2++;
                            if (startDay < Train2.saleDateStart)
                                D2 += Train2.saleDateStart - startDay,
                                    startDay = Train2.saleDateStart;

                            leaveD2 = D2, leave2 = T2;
                            if (startDay >= Train2.saleDateStart &&
                                startDay <= Train2.saleDateEnd) {
                                P2 -= Train2.price[l];
                                assert(startDay >= 0 && startDay <= 100);
                                seat2 = Train2.seat[startDay][l];
                                F2 = 0;
                            } else
                                break;
                        }

                        T2 += Train2.travelTimes[l];
                        if (F2) {
                            assert(startDay >= 0 && startDay <= 100);
                            seat2 = std::min(seat2, Train2.seat[startDay][l]);
                        }
                    }
                }
                P -= Train1.price[j];
            }

            T += Train1.stopoverTimes[j];

            if (Train1.stations[j] == Start) {

                while (T >= 1440)
                    T -= 1440, startDay--;
                if (startDay >= Train1.saleDateStart && startDay <= Train1.saleDateEnd) {
                    F = 1;
                    P = -Train1.price[j];
                    assert(startDay >= 0 && startDay <= 100);
                    seat = Train1.seat[startDay][j];
                    leave = T;
                } else
                    break;
            }

            T += Train1.travelTimes[j];
            if (F) {
                assert(startDay >= 0 && startDay <= 100);
                seat = std::min(seat, Train1.seat[startDay][j]);
            }
        }
    }
    if (Time == -1) {
        cout << "0\n";
        return;
    }
    cout << TrainID1 << " " << Start << " " << change_num_to_date(LeaveDate1) << " "
         << change_num_to_minute(LeaveMinute1) << " -> " << midStation << " "
         << change_num_to_date(ArriveDate1) << " " << change_num_to_minute(ArriveMinute1)
         << " " << Price1 << " " << Seat1 << "\n";
    cout << TrainID2 << " " << midStation << " " << change_num_to_date(LeaveDate2) << " "
         << change_num_to_minute(LeaveMinute2) << " -> " << End << " "
         << change_num_to_date(ArriveDate2) << " " << change_num_to_minute(ArriveMinute2)
         << " " << Price2 << " " << Seat2 << "\n";
}
