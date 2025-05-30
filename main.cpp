#pragma once
#include "BPT.hpp"
#include "User.hpp"
int n;
string op, Time, keys;

void readUser(vector<string> words, string &cur_username, string &username, string &password,
              string &name, string &mailAddr, int &privilege) {
    for (int i = 2; i < words.size(); i += 2) {
        keys = words[i];
        if (keys[1] == 'c')
            cur_username = words[i + 1];
        if (keys[1] == 'u')
            username = words[i + 1];
        if (keys[1] == 'p')
            password = words[i + 1];
        if (keys[1] == 'm')
            mailAddr = words[i + 1];
        if (keys[1] == 'n')
            name = words[i + 1];
        if (keys[1] == 'g') {
            if (words[i + 1].length() == 2)
                privilege = 10;
            else
                privilege = words[i + 1][0] - '0';
        }
    }
}

signed mian() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    std::fstream file1("user.bin", std::ios::in | std::ios::out | std::ios::binary);
    std::fstream file2("train.bin", std::ios::in | std::ios::out | std::ios::binary);
    TicketSystem sys(file1, file2);
    while (true) {
        std::getline(std::cin, line);

        std::istringstream iss(line);
        std::string word;
        std::vector<std::string> words;

        while (iss >> word) {
            words.push_back(word);
        }

        Time = words[0];
        op = words[1];

        if (op == "exit") {
            cout << "bye\n";
            exit(0);
        }
        if (op == "clean") {
            sys.clean();
        }
        if (op == "add_user") {
            string cur_username = {0}, username = {0}, password = {0}, name = {0}, mailAddr = {0};
            int privilege = 0;
            readUser(words, cur_usernamem, username, password, name, mailAddr, privilege);
            sys.add_user(cur_usernamem, username, password, name, mailAddr, privilege);
        }
        if (op == "login") {
            string cur_username = {0}, username = {0}, password = {0}, name = {0}, mailAddr = {0};
            int privilege = 0;
            readUser(words, cur_usernamem, username, password, name, mailAddr, privilege);
            sys.login(username, password);
        }
        if (op == "logout") {
            string cur_username = {0}, username = {0}, password = {0}, name = {0}, mailAddr = {0};
            int privilege = 0;
            readUser(words, cur_usernamem, username, password, name, mailAddr, privilege);
            sys.logout(username);
        }
        if (op == "query_profile") {
            string cur_username = {0}, username = {0}, password = {0}, name = {0}, mailAddr = {0};
            int privilege = 0;
            readUser(words, cur_usernamem, username, password, name, mailAddr, privilege);
            sys.query_profile(cur_usernamem, username);
        }
        if (op == "modify_profile") {
            string cur_username = {0}, username = {0}, password = {0}, name = {0}, mailAddr = {0};
            int privilege = 0;
            readUser(words, cur_usernamem, username, password, name, mailAddr, privilege);
            sys.modify_profile(cur_usernamem, username, password, name, mailAddr, privilege);
        }
    }

    return 0;
}