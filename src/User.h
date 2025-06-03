#pragma once
#include "BPT.hpp"
#include "MemoryRiver.hpp"

// 用户账户结构
struct User {
    char username[21] = {};
    char password[31] = {};
    char name[31] = {};
    char mailAddr[31] = {};
    int privilege = -1;

    User() = default;
    User(const string &id, const string &pwd, const string &Name, const string &mail,
         int priv)
        : privilege(priv) {
        strncpy(username, id.c_str(), 21);
        strncpy(password, pwd.c_str(), 31);
        strncpy(name, Name.c_str(), 31);
        strncpy(mailAddr, mail.c_str(), 31);
    }
};

class UserSystem {
    friend class TicketSystem;

  private:
    int total = 0;
    BPlusTree UserBase;
    MemoryRiver<User> UserData;
    map<long long, bool> LoginStack;

  public:
    UserSystem() : UserBase("UserBase.bin") {
        UserData.initialise("UserData.bin");
        UserData.get_info(total, 1);
    }

    ~UserSystem() { UserData.write_info(total, 1); }

    void add_user(string &cur_username, string &username, string &password, string &name,
                  string &mailAddr, int privilege);

    void login(string &username, string &password);

    void logout(string &username);

    void query_profile(string &cur_username, string &username);

    void modify_profile(string &cur_username, string &username, string &password,
                        string &name, string &mailAddr, int privilege);

    void clean() {
        std::remove("UserBase.bin");
        std::remove("UserData.bin");
        LoginStack.clear();
    }
};
