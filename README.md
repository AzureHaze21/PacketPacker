# PacketPacker

## How to use

```
char *p = pack(0xABCDEF, 123.f, 99., 'c', "string");
auto tuple = unpack<int, float, double, char, std::string>(p);
delete[] p;
```
