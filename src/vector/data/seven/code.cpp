
/**
 * Author: 于恩帝
 * Description: This is a more specific test for some details of the vector class.
 */
#include <vector>
#include <cstdio>

#include "vector.hpp"

void test_construct() {  // vector(const vector &other)
	sjtu::vector<std::vector<int>> a;
	a.push_back(std::vector<int>{0, 1, 2});
	sjtu::vector<std::vector<int>> b = a;
	a[0].push_back(3);
	b[0].pop_back();
	for (auto x : a[0])
		printf("%d ", x);
	puts("");
	for (auto x : b[0])
		printf("%d ", x);
	puts("");
}

void test_assign() {  // vector& operator=(const vector &other)
	sjtu::vector<std::vector<int>> a;
	a.push_back(std::vector<int>{0, 1, 2});
	sjtu::vector<std::vector<int>> b;
	b.push_back(std::vector<int>{3, 4});
	b = a;
	a[0].push_back(3);
	b[0].pop_back();
	for (auto x : a[0])
		printf("%d ", x);
	puts("");
	for (auto x : b[0])
		printf("%d ", x);
	puts("");
}

void test_resize() {
	sjtu::vector<std::vector<int>> a;
	for (int i = 0; i < 10; ++i)
		a.push_back(std::vector<int>{1, 2});
	sjtu::vector<std::vector<int>> b = a;
	sjtu::vector<std::vector<int>> c;
	c = a;
	for (int i = 0; i < 10; ++i) {
		b[i].push_back(0);
		c[i].push_back(1);
	}
	for (int i = 0; i < 10; ++i) {
		for (auto x : a[i])
			printf("%d ", x);
		puts("");
		for (auto x : b[i])
			printf("%d ", x);
		puts("");
		for (auto x : c[i])
			printf("%d ", x);
		puts("");
	}
}

void test_clear() {
	sjtu::vector<std::vector<int>> a;
	for (int i = 0; i < 10; ++i)
		a.push_back(std::vector<int>{1, 2});
	sjtu::vector<std::vector<int>> b = a;
	sjtu::vector<std::vector<int>> c;
	c = a;
	for (int i = 0; i < 10; ++i) {
		b[i].push_back(0);
		c[i].push_back(1);
	}
	a.clear();
	b.clear();
	c.clear();
}

void test_insert() {
	sjtu::vector<std::vector<int>> a;
	for (int i = 0; i < 10; ++i)
		a.push_back(std::vector<int>{1, 2});
	sjtu::vector<std::vector<int>> b = a;
	sjtu::vector<std::vector<int>> c;
	c = a;
	for (int i = 0; i < 10; ++i) {
		b[i].push_back(0);
		c[i].push_back(1);
	}
	for (int i = 0; i < 10; ++i) {
		a.insert(a.begin() + 1, std::vector<int>{1, 2});
		b.insert(b.begin() + 1, a[1]);
		c.insert(c.begin() + 1, b[1]);
		c[1].push_back(3);
		b[1].push_back(4);
	}
	for (int i = 0; i < 20; ++i) {
		for (auto x : a[i])
			printf("%d ", x);
		puts("");
		for (auto x : b[i])
			printf("%d ", x);
		puts("");
		for (auto x : c[i])
			printf("%d ", x);
		puts("");
	}
}

void test_iterator() {
	// Modified by 庄裕旻: remove some UB, only test cross container comparison in fixed condtiion
	sjtu::vector<int> a;
	sjtu::vector<int> b;
	a.push_back(1);
	b.push_back(1);
	puts(a.begin() == b.begin() ? "YES" : "NO");
	a.pop_back();
	b.pop_back();
}

int main() {
	test_construct();
	test_assign();
	test_resize();
	test_clear();
	test_insert();
	test_iterator();
	return 0;
}
