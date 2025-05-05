#include <iostream>
#include <chrono>
#include "vector.hpp"

using std::cout;
using std::endl;
using std::cerr;
using namespace std::chrono;
using namespace sjtu;

// 用于检测扩容的阈值（微秒）
constexpr int EXPANSION_THRESHOLD_US = 100;

int main() {
    vector<int> vec;
    
    // 第一阶段：push_back 1e6 个元素
    for (int i = 0; i < 1000000; ++i) {
        vec.push_back(i);
    }
    
    // 记录当前大小
    size_t initial_size = vec.size();
    
    // 继续push直到检测到扩容（通过时间判断）
    size_t expansion_size = initial_size;
    auto last_time = high_resolution_clock::now();
    bool expansion_detected = false;
    
    while (!expansion_detected && vec.size() < 8000000) {  // 设置上限防止无限循环
        vec.push_back(vec.size());
        auto current_time = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(current_time - last_time).count();
        
        if (duration > EXPANSION_THRESHOLD_US) {
            expansion_detected = true;
            expansion_size = vec.size();
        }
        last_time = current_time;
    }
    
    cout << "Initial size: " << initial_size << endl;
    cerr << "Size at expansion: " << expansion_size << endl;
    
    // 在扩容点附近反复进行push和pop操作
    auto start = high_resolution_clock::now();
    for (int round = 0; round < 100000; ++round) {
        // 每轮先pop 16次
        for (int i = 0; i < 16; ++i) {
            vec.pop_back();
        }
        // 再push 16次
        for (int i = 0; i < 16; ++i) {
            vec.push_back(round);
        }
    }
    auto end = high_resolution_clock::now();
    
    cerr << "Time taken for 100000 rounds of 16 push-pops (ms): " 
         << duration_cast<milliseconds>(end - start).count() << endl;
    cerr << "Final size: " << vec.size() << endl;
    
    return 0;
}