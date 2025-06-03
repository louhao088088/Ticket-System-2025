#include "User.h"

void UserSystem::add_user(string &cur_username, string &username, string &password,
                          string &name, string &mailAddr, int privilege) {
    User new_User(username, password, name, mailAddr, privilege);
    long long UserKey = Hash(username), Cur_UserKey = Hash(cur_username);
    if (UserBase.empty()) {
        new_User.privilege = 10;
        UserBase.insert(UserKey, total);
        UserData.write(new_User, total);
        total++;
        cout << "0\n";
    } else {
        vector<int> Data = UserBase.find(UserKey);
        vector<int> Cur_Data = UserBase.find(Cur_UserKey);
        if (Data.size()) {
            cout << "-1\n";
            return;
        }
        if (!LoginStack[Cur_UserKey]) {
            cout << "-1\n";
            return;
        }
        assert(Cur_Data.size() == 1);
        User Cur;
        UserData.read(Cur, Cur_Data[0]);
        if (Cur.privilege <= privilege) {
            cout << "-1\n";
            return;
        }
        UserBase.insert(UserKey, total);
        UserData.write(new_User, total);
        total++;
        cout << "0\n";
    }
}

void UserSystem::login(string &username, string &password) {
    long long UserKey = Hash(username);
    if (LoginStack[UserKey]) {
        cout << "-1\n";
        return;
    }

    vector<int> Data = UserBase.find(UserKey);
    if (!Data.size()) {
        cout << "-1\n";
        return;
    }

    User Cur;
    UserData.read(Cur, Data[0]);
    if (Cur.password != password) {
        cout << "-1\n";
        return;
    }

    LoginStack[UserKey] = 1;
    cout << "0\n";
}

void UserSystem::logout(string &username) {
    long long UserKey = Hash(username);

    if (LoginStack[UserKey] == 0) {
        cout << "-1\n";
        return;
    }
    LoginStack[UserKey] = 0;
    cout << "0\n";
}

void UserSystem::query_profile(string &cur_username, string &username) {
    long long UserKey = Hash(username), Cur_UserKey = Hash(cur_username);
    if (LoginStack[Cur_UserKey] == 0) {
        cout << "-1\n";
        return;
    }
    vector<int> Data = UserBase.find(UserKey);
    vector<int> Cur_Data = UserBase.find(Cur_UserKey);
    if (!Data.size()) {
        cout << "-1\n";
        return;
    }
    assert(Cur_Data.size() == 1);
    User user1, user2;
    UserData.read(user2, Data[0]);
    UserData.read(user1, Cur_Data[0]);
    if (user1.privilege <= user2.privilege && UserKey != Cur_UserKey) {
        cout << "-1\n";
        return;
    }
    cout << user2.username << " " << user2.name << " " << user2.mailAddr << " "
         << user2.privilege << "\n";
}

void UserSystem::modify_profile(string &cur_username, string &username, string &password,
                                string &name, string &mailAddr, int privilege) {
    long long UserKey = Hash(username), Cur_UserKey = Hash(cur_username);
    if (LoginStack[Cur_UserKey] == 0) {
        cout << "-1\n";
        return;
    }

    vector<int> Data = UserBase.find(UserKey);
    vector<int> Cur_Data = UserBase.find(Cur_UserKey);
    if (!Data.size()) {
        cout << "-1\n";
        return;
    }
    assert(Cur_Data.size() == 1);
    User user1, user2;
    UserData.read(user2, Data[0]);
    UserData.read(user1, Cur_Data[0]);
    if (user1.privilege <= user2.privilege && UserKey != Cur_UserKey) {
        cout << "-1\n";
        return;
    }
    if (user1.privilege <= privilege) {
        cout << "-1\n";
        return;
    }
    if (!password.empty())
        strncpy(user2.password, password.c_str(), 31);

    if (!name.empty())
        strncpy(user2.name, name.c_str(), 31);

    if (!mailAddr.empty())
        strncpy(user2.mailAddr, mailAddr.c_str(), 31);

    if (privilege != -1)
        user2.privilege = privilege;

    cout << user2.username << " " << user2.name << " " << user2.mailAddr << " "
         << user2.privilege << "\n";
    UserData.write(user2, Data[0]);
}
