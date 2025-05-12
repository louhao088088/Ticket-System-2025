#include "src/BPT.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

int main() {
    std::fstream file("database.bin", std::ios::in | std::ios::out | std::ios::binary);
    BPlusTree tree(file);

    int n;
    std::cin >> n;
    while (n--) {
        std::string cmd(64, '\0'), index(64, '\0');
        int value;
        std::cin >> cmd;
        if (cmd == "insert") {
            std::cin >> index >> value;

            char key[64];
            memset(key, 0, 64);
            strncpy(key, index.c_str(), 64);

            tree.insert(Hash(key), value);
        } else if (cmd == "delete") {
            std::cin >> index >> value;

            char key[64];
            memset(key, 0, 64);
            strncpy(key, index.c_str(), 64);

            tree.remove(Hash(key), value);
        } else if (cmd == "find") {
            std::cin >> index;
            char key[64];
            memset(key, 0, 64);
            strncpy(key, index.c_str(), 64);

            vector<int> values = tree.find(Hash(key));
            if (values.empty()) {
                std::cout << "null\n";
            } else {
                for (size_t i = 0; i < values.size(); ++i) {
                    if (i > 0)
                        std::cout << ' ';
                    std::cout << values[i];
                }
                std::cout << '\n';
            }
        }
    }

    return 0;
}