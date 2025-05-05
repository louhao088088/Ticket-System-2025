#include "vector.hpp"

#include <iostream>

void TestClear() {
    std::cout << "Testing clear function..." << std::endl;
    
    sjtu::vector<int> v1;
    for (int i = 1; i <= 10; ++i) {
        v1.push_back(i);
    }
    std::cout << "Before clear - size: " << v1.size() << ", empty: " << v1.empty() << std::endl;
    v1.clear();
    std::cout << "After clear - size: " << v1.size() << ", empty: " << v1.empty() << std::endl;
    
    for (int i = 20; i <= 25; ++i) {
        v1.push_back(i);
    }
    std::cout << "After inserting new elements:" << std::endl;
    for (sjtu::vector<int>::iterator it = v1.begin(); it != v1.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    v1.clear();
    v1.clear();
    std::cout << "After multiple clears - size: " << v1.size() << ", empty: " << v1.empty() << std::endl;
    
    sjtu::vector<int> v2;
    for (int i = 0; i < 10000; ++i) {
        v2.push_back(i);
    }
    std::cout << "Before clearing large vector - size: " << v2.size() << std::endl;
    v2.clear();
    std::cout << "After clearing large vector - size: " << v2.size() << std::endl;
    
    v2.push_back(42);
    sjtu::vector<int>::iterator it = v2.begin();
    std::cout << "First element after clear and push_back: " << *it << std::endl;
}

int main() {
    TestClear();
    return 0;
} 