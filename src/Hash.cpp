#include "Hash.hpp"

const int mod1 = 998244353, mod2 = 1019260817, base1 = 233, base2 = 279;
long long Hash(const char *data) {
    long long res1 = 1, res2 = 1;
    for (int i = 0; i < 64; i++) {
        res1 = res1 * base1 + data[i], res1 %= mod1;
        res2 = res2 * base2 + data[i], res2 %= mod2;
    }
    return res1 * mod2 + res2;
}

long long Hash(const string &data) {
    long long res1 = 1, res2 = 1, len = data.length();
    for (int i = 0; i < len; i++) {
        res1 = res1 * base1 + data[i], res1 %= mod1;
        res2 = res2 * base2 + data[i], res2 %= mod2;
    }
    return res1 * mod2 + res2;
}