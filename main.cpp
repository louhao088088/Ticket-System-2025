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
        // cout << cmd << endl;

        if (cmd == "insert") {
            std::cin >> index >> value;
            // cout << index << " " << value << endl;
            char key[KEY_SIZE];
            memset(key, 0, 64);
            strncpy(key, index.c_str(), 64);
            memcpy(key + 64, &value, sizeof(int));
            tree.insert(key);
        } else if (cmd == "delete") {
            std::cin >> index >> value;
            // cout << index << " " << value << endl;
            char key[KEY_SIZE];
            memset(key, 0, 64);
            strncpy(key, index.c_str(), 64);
            memcpy(key + 64, &value, sizeof(int));
            tree.remove(key);
        } else if (cmd == "find") {
            std::cin >> index;
            // cout << index << " " << endl;
            vector<int> values = tree.find(index.c_str());
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
        // if (n <= 500)
        // tree.print_bptree_structure(file);
    }

    return 0;
}