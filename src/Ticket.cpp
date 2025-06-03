#include "Ticket.h"

#include "Time.h"

void TicketSystem::buy_ticket(UserSystem &UserSys, TrainSystem &TrainSys,
                              const string &username, const string &trainID, int Date,
                              const string &Start, const string &End, int ticketNum,
                              int flag) {
    long long UserKey = Hash(username);
    vector<int> user1 = UserSys.UserBase.find(UserKey);
    if (user1.size() == 0) {
        cout << "-1\n";
        return;
    }
    if (!UserSys.LoginStack[UserKey]) {
        cout << "-1\n";
        return;
    }
    long long TrainKey = Hash(trainID);
    vector<int> train1 = TrainSys.TrainBase.find(TrainKey);
    // cerr << "A";
    if (train1.size() == 0) {
        cout << "-1\n";
        return;
    }
    Train train;
    TrainSys.TrainData.read(train, train1[0]);
    if (train.release_status == 0) {
        cout << "-1\n";
        return;
    }
    int T = train.startTime, startDay = Date, P = 0, seat = train.seatNum, D = Date;
    int l = 0, r = 0, leave, F = 0;
    for (int i = 0; i < train.stationNum; i++) {

        if (train.stations[i] == End) {
            while (T >= 1440)
                T -= 1440, D++;
            F++;
            r = i;
            P += train.price[i];
            break;
        }

        T += train.stopoverTimes[i];

        if (train.stations[i] == Start) {
            l = i;
            while (T >= 1440)
                T -= 1440, startDay--;
            leave = T;

            if (startDay >= train.saleDateStart && startDay <= train.saleDateEnd) {
                P = -train.price[i];
                F = 1;
                assert(startDay >= 0 && startDay <= 100);
                seat = train.seat[startDay][i];
            } else {
                cout << "-1\n";
                return;
            }
        }

        T += train.travelTimes[i];
        if (F) {
            assert(startDay >= 0 && startDay <= 100);
            seat = std::min(seat, train.seat[startDay][i]);
        }
    }
    if ((seat < ticketNum && !flag) || F < 2) {
        cout << "-1\n";
        return;
    }
    // cerr << P << "\n";
    Ticket new_ticket(username, trainID, Start, End, Date, leave, D, T, P, ticketNum,
                      seat >= ticketNum ? 1 : 2, startDay);
    TicketBase.insert(UserKey, total);
    TicketData.write(new_ticket, total);
    if (seat >= ticketNum) {

        for (int i = l; i < r; i++) {
            assert(startDay >= 0 && startDay <= 100);
            train.seat[startDay][i] -= ticketNum;
        }
        TrainSys.TrainData.write(train, train1[0]);
        cout << 1ll * P * ticketNum << "\n";
    }

    else {
        QueueBase.insert(TrainKey, total);
        cout << "queue\n";
    }
    total++;
}

void TicketSystem::query_order(UserSystem &UserSys, TrainSystem &TrainSys,
                               const string &username) {
    long long UserKey = Hash(username);
    vector<int> user1 = UserSys.UserBase.find(UserKey);
    if (user1.size() == 0) {
        cout << "-1\n";
        return;
    }
    if (!UserSys.LoginStack[UserKey]) {
        cout << "-1\n";
        return;
    }
    vector<int> ticket1 = TicketBase.find(UserKey);
    // cerr<<""
    cout << ticket1.size() << "\n";
    for (int i = ticket1.size() - 1; i >= 0; i--) {
        Ticket ticket;
        TicketData.read(ticket, ticket1[i]);
        assert(ticket.status >= 1 && ticket.status <= 3);
        if (ticket.status == 1)
            cout << "[success] ";
        else if (ticket.status == 2)
            cout << "[pending] ";
        else if (ticket.status == 3)
            cout << "[refunded] ";
        cout << ticket.trainID << " " << ticket.Start << " "
             << change_num_to_date(ticket.leaveDay) << " "
             << change_num_to_minute(ticket.leaveMinute) << " -> " << ticket.End << " "
             << change_num_to_date(ticket.arriveDay) << " "
             << change_num_to_minute(ticket.arriveMinute) << " " << ticket.price << " "
             << ticket.num << "\n";
    }
}
void TicketSystem::refund_ticket(UserSystem &UserSys, TrainSystem &TrainSys,
                                 const string &username, int num) {
    long long UserKey = Hash(username);
    vector<int> user1 = UserSys.UserBase.find(UserKey);
    if (user1.size() == 0) {
        cout << "-1\n";
        return;
    }
    if (!UserSys.LoginStack[UserKey]) {
        cout << "-1\n";
        return;
    }
    vector<int> ticket1 = TicketBase.find(UserKey);
    if (int(ticket1.size()) < num) {
        cout << "-1\n";
        return;
    }

    Ticket ticket;
    TicketData.read(ticket, ticket1[ticket1.size() - num]);
    // cerr << ticket.status << "\n";
    if (ticket.status == 3) {
        cout << "-1\n";
        return;
    }

    long long TrainKey = Hash(ticket.trainID);
    if (ticket.status == 2) {
        ticket.status = 3;
        QueueBase.remove(TrainKey, ticket1[ticket1.size() - num]);
        TicketData.write(ticket, ticket1[ticket1.size() - num]);
        cout << "0\n";
        return;
    }
    vector<int> train1 = TrainSys.TrainBase.find(TrainKey);
    if (train1.size() == 0) {
        cout << "-1\n";
        return;
    }
    Train train;
    TrainSys.TrainData.read(train, train1[0]);
    int F = 0;
    for (int i = 0; i < train.stationNum; i++) {

        if ((string) train.stations[i] == (string) ticket.Start) {
            F = 1;
        }
        if ((string) train.stations[i] == (string) ticket.End) {
            break;
        }
        if (F) {
            assert(ticket.startDay >= 0 && ticket.startDay <= 100);
            train.seat[ticket.startDay][i] += ticket.num;
        }
    }
    vector<int> queue1 = QueueBase.find(TrainKey);
    int N = queue1.size();
    for (int i = 0; i < N; i++) {
        Ticket ticket;

        TicketData.read(ticket, queue1[i]);

        assert(ticket.startDay >= 0 && ticket.startDay <= 100);
        int seat = 0, l, r;
        for (int j = 0; j < train.stationNum; j++) {
            if ((string) train.stations[j] == (string) ticket.Start) {
                seat = train.seat[ticket.startDay][j];
                l = j;
            } else if ((string) train.stations[j] == (string) ticket.End) {
                r = j;
                if (seat >= ticket.num) {
                    for (int k = l; k < r; k++) {
                        train.seat[ticket.startDay][k] -= ticket.num;
                    }
                    ticket.status = 1;
                    TicketData.write(ticket, queue1[i]);
                    QueueBase.remove(TrainKey, queue1[i]);
                }
                break;
            }
            assert(ticket.startDay >= 0 && ticket.startDay <= 100);
            seat = std::min(seat, train.seat[ticket.startDay][j]);
        }
    }
    ticket.status = 3;
    TicketData.write(ticket, ticket1[ticket1.size() - num]);
    TrainSys.TrainData.write(train, train1[0]);
    cout << "0\n";
}
