//g++ setWires.cpp -o setWires
#include <iostream>
#include <bitset>

using namespace std;

int main()
{
    uint8_t datah = 0xFF;
    uint8_t datal = 0xFF;
    bool W[] = {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0};
    for (int i=15;i>=0;i--) printf("%d", W[i]);
    printf("\n");
    for (int i=0;i<8;i++)  datal = datal & ~(W[i] << i); // Pull down
    for (int i=8;i<16;i++) datah = datah & ~(W[i] << i-8); // Pull down
    std::cout << "a = " << std::bitset<8>(datah)  << std::endl;
    std::cout << "b = " << std::bitset<8>(datal)  << std::endl;
    return 0;
}