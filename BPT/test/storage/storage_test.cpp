#include <gtest/gtest.h>
#include <storage/bptStorage.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio> // Required for std::remove

// Helper function to execute commands from input file
void executeCommands(BPTStorage<int, int>& storage, const std::string& inputFile, const std::string& outputFile) {
    std::ifstream input(inputFile);
    std::ifstream expectedOutput(outputFile);
    std::string command;
    int key, value;

    std::ostringstream actualOutput;

    while (input >> command) {
        if (command == "insert") {
            input >> key >> value;
            storage.insert(key, value);
        } else if (command == "find") {
            input >> key;
            auto result = storage.find(key);
            for (const auto& val : result) {
                actualOutput << val << " ";
            }
            actualOutput << "\n";
        } else if (command == "delete") {
            input >> key >> value;
            storage.remove(key, value);
        }
    }

    std::ostringstream expected;
    expected << expectedOutput.rdbuf();

    ASSERT_EQ(actualOutput.str(), expected.str());
}

// Test case for 1.in and 1.out
TEST(StorageTest, TestCase1) {
    // Ensure clean state before test
    std::remove("test1.db_data");
    std::remove("test1.db_node");
    BPTStorage<int, int> storage("test1.db", INT_MAX);
    executeCommands(storage, "../../test/storage/simple/1.in", "../../test/storage/simple/1.out");
    // Clean up after test
    std::remove("test1.db_data");
    std::remove("test1.db_node");
}

TEST(StorageTest, TestCase2) {
    std::remove("test2.db_data");
    std::remove("test2.db_node");
    BPTStorage<int, int> storage("test2.db",INT_MAX);
    executeCommands(storage, "../../test/storage/simple/2.in", "../../test/storage/simple/2.out");
    std::remove("test2.db_data");
    std::remove("test2.db_node");
}

TEST(StorageTest, TestCase3) {
    std::remove("test3.db_data");
    std::remove("test3.db_node");
    BPTStorage<int, int> storage("test3.db",INT_MAX);
    executeCommands(storage, "../../test/storage/simple/3.in", "../../test/storage/simple/3.out");
    std::remove("test3.db_data");
    std::remove("test3.db_node");
}

TEST(StorageTest, TestCase4) {
    std::remove("test4.db_data");
    std::remove("test4.db_node");
    BPTStorage<int, int> storage("test4.db",INT_MAX);
    executeCommands(storage, "../../test/storage/simple/4.in", "../../test/storage/simple/4.out");
    std::remove("test4.db_data");
    std::remove("test4.db_node");
}

TEST(StorageTest, TestCase5) {
    std::remove("test5.db_data");
    std::remove("test5.db_node");
    BPTStorage<int, int> storage("test5.db",INT_MAX);
    executeCommands(storage, "../../test/storage/simple/5.in", "../../test/storage/simple/5.out");
    std::remove("test5.db_data");
    std::remove("test5.db_node");
}

TEST(StorageTest, TestCase6) {
    std::remove("test6.db_data");
    std::remove("test6.db_node");
    BPTStorage<int, int> storage("test6.db",INT_MAX);
    executeCommands(storage, "../../test/storage/simple/6.in", "../../test/storage/simple/6.out");
    std::remove("test6.db_data");
    std::remove("test6.db_node");
}
