#pragma GCC Optimize("Ofast")
#include <bits/stdc++.h>
using namespace std;
const int maxn = 5e5 + 5, mod = 998244353;
#define int long long
#define pb push_back
#define pi pair<int, int>
#define fi first
#define se second
#define mp make_pair
inline int read() {
    char ch = getchar();
    int x = 0;
    bool f = 0;
    for (; !isdigit(ch); ch = getchar())
        if (ch == '-') {
            f = 1;
        }
    for (; isdigit(ch); ch = getchar())
        x = (x << 1) + (x << 3) + (ch ^ 48);
    if (f == 1)
        x = -x;
    return x;
}
int n, m, a[maxn], x, y;
vector<int> tmp;
set<pi> s;
char ch[maxn];
signed main() {
    n = read();
    for (int i = 1; i <= n; i++) {
        scanf("%s", ch + 1);
        if (ch[1] == 'i') {
            x = read(), y = read();
            s.insert(mp(x, y));
        } else if (ch[1] == 'd') {
            x = read(), y = read();
            s.erase(mp(x, y));
        } else {
            x = read();
            tmp.clear();
            auto it = s.lower_bound(mp(x, -100000000));
            for (; it != s.end(); it++) {
                if (it->fi == x)
                    tmp.pb(it->se);
                else
                    break;
            }
            if (!tmp.size()) {
                puts("null");
            } else {
                for (int i = 0; i < tmp.size(); i++)
                    printf("%lld ", tmp[i]);
                puts("");
            }
        }
    }
}