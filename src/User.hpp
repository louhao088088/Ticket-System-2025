#pragma once
#include "BPT.hpp"

#include <bits/stdc++.h>
using namespace std;

// 用户账户结构
struct User {
    char username[21] = {0};
    char password[31] = {0};
    char name[31] = {0};
    char mailAddr[31] = {0};
    int privilege = 0;
    int next = -1;

    User() = default;
    User(const string &id, const string &pwd, const string &Name, const string &mail, int priv)
        : privilege(priv) {
        strncpy(username, id.c_str(), 20);
        strncpy(password, pwd.c_str(), 30);
        strncpy(name, Name.c_str(), 30);
        strncpy(mailAddr, mailAddr.c_str(), 30);
    }
};

class UserSystem {
    friend class TicketSystem;

  private:
    int total = 0;
    BPlusTree Userbase;
    map<long long, bool> LoginStack;

  public:
    UserSystem(std::fstream &file) : Userbase(file) {}
    
    void add_user(string &cur_username, string &username, string &password, string &name,
                  string &mailAddr, int &privilege);

    void login(string &username, string &password);

    void logout(string &username);

    void query_profile(string &cur_username, string &username);

    void modify_profile(string &cur_username, string &username, string &password, string &name,
                        string &mailAddr, int &privilege);

    void clean() {
        std::remove("UserBase.bin");
        LoginStack.clear();
    }
}
