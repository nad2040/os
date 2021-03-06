#include <iostream>
#include <bitset>

bool allDiff(int a, int b, int c, int d, int e, int f, int g, int h) {
    uint16_t mask = 0;
    
    mask |= (1<<a);
    if (mask & (1<<b)) return false;
    mask |= (1<<b);
    if (mask & (1<<c)) return false;
    mask |= (1<<c);
    if (mask & (1<<d)) return false;
    mask |= (1<<d);
    if (mask & (1<<e)) return false;
    mask |= (1<<e);
    if (mask & (1<<f)) return false;
    mask |= (1<<f);
    if (mask & (1<<g)) return false;
    mask |= (1<<g);
    if (mask & (1<<h)) return false;

    return mask;
}

int add(int a, int b) {
    int sum;
    while (b != 0) {
        sum = a^b;
        b = ((a&b) << 1);
        a = sum;
    }
    return a;
}

void LFSR() {
    u_int32_t lfsr[4] = {0x00000001,0,0,0x80000000}; //4*32 = 128 bit reg
    for (;;) {
        printf("%x", lfsr[0] & 1);
        u_int32_t bit = (lfsr[0] ^ (lfsr[0] >> 1) ^ (lfsr[0] >> 2) ^ (lfsr[0] >> 7)) & 1;
        lfsr[0] = (lfsr[0] >> 1) | (lfsr[1] << 31);
        lfsr[1] = (lfsr[1] >> 1) | (lfsr[2] << 31);
        lfsr[2] = (lfsr[2] >> 1) | (lfsr[3] << 31);
        lfsr[3] = (lfsr[3] >> 1) | (bit << 31);
    }
}

int main() {
    /* for (int s=1; s<10; s++)
    for (int e=0; e<10; e++)
    for (int n=0; n<10; n++)
    for (int d=0; d<10; d++)
    for (int m=1; m<10; m++)
    for (int o=0; o<10; o++)
    for (int r=0; r<10; r++)
    for (int y=0; y<10; y++) {
        int send = 1000*s + 100*e + 10*n + d;
        int more = 1000*m + 100*o + 10*r + e;
        int money = 10000*m + 1000*o + 100*n + 10*e + y;
        
        if (allDiff(s,e,n,d,m,o,r,y) && send + more == money)
            std::cout << send << " + " << more << " = " << money << std::endl;
    } */

    // std::cout << add(0,134) << '\n';

    LFSR();

}
