#include "src/BPT.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

int main() {
    fstream file("database.bin", ios::in | ios::out | ios::binary);
    BPlusTree tree(file);

    int n;
    cin >> n;
    // cout << n << endl;
    while (n--) {
        string cmd(64, '\0'), index(64, '\0');
        int value;
        std::cin >> cmd;
       // cout << n << endl;
        //cout << cmd << endl;

        if (cmd == "insert") {
            std::cin >> index >> value;

            char key[64];
            memset(key, 0, 64);
            strncpy(key, index.c_str(), 64);
           // cout << Hash(key) << " " << value << endl;
            tree.insert(Hash(key), value);
        } else if (cmd == "delete") {
            std::cin >> index >> value;

            char key[64];
            memset(key, 0, 64);
            strncpy(key, index.c_str(), 64);
           // cout << Hash(key) << " " << value << endl;
            tree.remove(Hash(key), value);
        } else if (cmd == "find") {
            std::cin >> index;
            char key[64];
            memset(key, 0, 64);
            strncpy(key, index.c_str(), 64);
          //  cout << Hash(key) << endl;
            vector<int> values = tree.find(Hash(key));
            if (values.empty()) {
                cout << "null\n";
            } else {
                for (size_t i = 0; i < values.size(); ++i) {
                    if (i > 0)
                        std::cout << ' ';
                    std::cout << values[i];
                }
                std::cout << '\n';
            }
        }
       // if (n <= 100)
        //    tree.print_bptree_structure(file);
    }

    return 0;
}