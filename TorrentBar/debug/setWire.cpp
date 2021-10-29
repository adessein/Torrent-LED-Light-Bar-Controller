  //g++ setWire.cpp -o setWire
#include <iostream>
#include <bitset>

using namespace std;

uint8_t datah, datal;

void setWire(int w, bool state)
{
  // Modifies the bytes to be sent to the shift registers
  // so that the wire *w* takes state *state*
  // state=true : Apply +12V
  // state=false : Apply 0V
  if(state==false) // I want 0V
  {
    if(w<8)
    {
      datal = datal | (1 << w); // Pull up
    }
    else
    {
      datah = datah | (1 << w-8); // Pull up
    }
  }
  else // I want +12V
  {
    if(w<8)
    {
      datal = datal & ~(1 << w); // Pull down
    }
    else
    {
      datah = datah & ~(1 << w-8); // Pull down
    }
  }
}

int main()
{
    datah = 0xFF;
    datal = 0xFF;
    std::cout << "datah = " << std::bitset<8>(datah)  << std::endl;
    std::cout << "datal = " << std::bitset<8>(datal)  << std::endl;
    setWire(5, true);
    std::cout << "datah = " << std::bitset<8>(datah)  << std::endl;
    std::cout << "datal = " << std::bitset<8>(datal)  << std::endl;
    return 0;
  }