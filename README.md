# PacketPacker

Example
-------

```cpp
#include "struct.h"

int main()
{
  char *p = pack(0xABCDEF, 123.f, 99., 'c', "string");
  auto tuple = unpack<int, float, double, char, std::string>(p);
  delete[] p;
  
  return 0;
}
```
