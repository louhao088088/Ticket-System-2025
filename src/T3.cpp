#include <bits/stdc++.h>
using namespace std;
struct node {
    long long w, lazy, lazi, l, r;
    bool f;
} zry[4000005];
int n, m, i, a, b, c, d;
inline void build(int l, int r, int x) {
    zry[x].l = l, zry[x].r = r, zry[x].w = 0, zry[x].lazy = 0, zry[x].lazi = 0, zry[x].f = false;
    if (l == r)
        return;
    int mid = (l + r) >> 1;
    build(l, mid, x << 1);
    build(mid + 1, r, x << 1 | 1);
}
inline void cl(int x) {
    if (zry[x].f) {
        long long ll = zry[x * 2].l, rr = zry[x * 2].r;
        zry[x * 2].w = zry[x * 2].w + zry[x].lazy * (rr - ll + 1) +
                       zry[x].lazi * (rr - ll + 1) * (rr - ll + 2) / 2;
        zry[x * 2].lazy = zry[x * 2].lazy + zry[x].lazy;
        zry[x * 2].lazi = zry[x * 2].lazi + zry[x].lazi;
        zry[x * 2].f = true;
        ll = zry[x * 2 + 1].l, rr = zry[x * 2 + 1].r;
        zry[x * 2 + 1].w = zry[x * 2 + 1].w +
                           (zry[x].lazy + (ll - zry[x * 2].l) * zry[x].lazi) * (rr - ll + 1) +
                           zry[x].lazi * (rr - ll + 1) * (rr - ll + 2) / 2;
        zry[x * 2 + 1].lazy = zry[x * 2 + 1].lazy + zry[x].lazy + zry[x].lazi * (ll - zry[x * 2].l);
        zry[x * 2 + 1].lazi = zry[x * 2 + 1].lazi + zry[x].lazi;
        zry[x * 2 + 1].f = true;
        zry[x].lazy = 0;
        zry[x].lazi = 0;
        zry[x].f = false;
    }
}
inline void jb(int l, int r, int x, long long y) {
    if (zry[x].l > r || zry[x].r < l)
        return;
    if (zry[x].l >= l && zry[x].r <= r) {
        long long ll = zry[x].l, rr = zry[x].r;
        zry[x].w = zry[x].w + (ll - l) * y * (rr - ll + 1) + (rr - ll + 1) * (rr - ll + 2) * y / 2;
        zry[x].lazy = zry[x].lazy + (ll - l) * y;
        zry[x].lazi = zry[x].lazi + y;
        zry[x].f = true;
        return;
    }
    cl(x);
    jb(l, r, x << 1, y);
    jb(l, r, x << 1 | 1, y);
    zry[x].w = zry[x << 1].w + zry[x << 1 | 1].w;
}
inline long long js(int l, int r, int x) {
    if (zry[x].l > r || zry[x].r < l)
        return 0;
    if (zry[x].l >= l && zry[x].r <= r)
        return zry[x].w;
    cl(x);
    return js(l, r, x << 1) + js(l, r, x << 1 | 1);
}
int main() {
    scanf("%d%d", &n, &m);
    build(1, n, 1);
    for (i = 1; i <= m; i++) {
        scanf("%d", &a);
        if (a == 1) {
            scanf("%d%d%d", &b, &c, &d);
            jb(c, d, 1, b);
        } else {
            scanf("%d%d", &b, &c);
            printf("%lld\n", js(b, c, 1));
        }
    }
    return 0;
}