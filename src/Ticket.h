#pragma once
#include "Train.h"
#include "User.h"

struct Ticket {
    char username[21] = {};
    char trainID[21] = {};
    int leaveDay, leaveMinute;
    int arriveDay, arriveMinute;

    char Start[31] = {};
    char End[31] = {};
    int price;
    int num;
    int status = 0; // 1:success,2:pending,3:refunded
    int startDay;

    Ticket() = default;
    Ticket(const string &name, const string &id, const string &start, const string &end,
           int leaveday, int leaveminute, int arriveday, int arriveminute, int price,
           int num, int status, int startDay)
        : leaveDay(leaveday), leaveMinute(leaveminute), arriveDay(arriveday),
          arriveMinute(arriveminute), price(price), num(num), status(status),
          startDay(startDay) {
        strncpy(username, name.c_str(), 21);
        strncpy(trainID, id.c_str(), 21);
        strncpy(Start, start.c_str(), 31);
        strncpy(End, end.c_str(), 31);
    }
};

class TicketSystem {

  private:
    int total = 0;
    BPlusTree TicketBase, QueueBase;
    MemoryRiver<Ticket> TicketData;

  public:
    TicketSystem() : TicketBase("TicketBase.bin"), QueueBase("QueueBase.bin") {
        TicketData.initialise("TicketData.bin");
        TicketData.get_info(total, 1);
    }

    ~TicketSystem() {
        TicketData.write_info(total, 1);
        TicketBase.flush();
        QueueBase.flush();
    }
    void buy_ticket(UserSystem &UserSys, TrainSystem &TrainSys, const string &username,
                    const string &trainID, int Date, const string &Start,
                    const string &End, int ticketNum, int flag);
    void query_order(UserSystem &UserSys, TrainSystem &TrainSys, const string &username);
    void refund_ticket(UserSystem &UserSys, TrainSystem &TrainSys, const string &username,
                       int num);

    void clean() {
        std::remove("TicketBase.bin");
        std::remove("TicketData.bin");
        std::remove("QueueBase.bin");
    }
};
