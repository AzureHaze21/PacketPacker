#include <vector>
#include <string>

using byte = unsigned char;

uint32_t GetSize(std::string& str)
{
	return str.size() + 4;
}

uint32_t GetSize(const char *pStr)
{
	return strlen(pStr) + 4;
}

template<typename T>
uint32_t GetSize(std::vector<T> bytes)
{
	return bytes.size() * sizeof(T) + 4;
}

template<typename T>
constexpr uint32_t GetSize(T& val)
{
	return sizeof(T);
}

template<typename T, typename... Args>
uint32_t GetSize(T first, Args&&... args)
{
	return (GetSize(first) + GetSize(args...));
}

enum class Opcode : byte
{
	NOP = 0x00,
	HELLO = 0x01,
	TEST = 0x02,
	AUTH = 0x03,
	BYE = 0x04,
	EXEC = 0x05,
	PING = 0x06,
	PONG = 0x07,
	SHOW = 0x08,
	DOWNLOAD = 0x09,
	SHELL = 0x0A,
	CAPTURE = 0x0B,
	KILL = 0x0C,
	XFER = 0x0D,
	LOGS = 0x0E,
	INFO = 0x0F
};

#pragma pack(push, 1)
struct PacketHeader
{
	uint32_t sync = 0x636e7973;
	Opcode opcode{ Opcode::NOP };
	uint32_t size{ sizeof(PacketHeader) };
};
#pragma pack(pop)

template<Opcode op>
class Packet
{
private:
	PacketHeader m_header;
	byte *m_data = nullptr;

public:
	Packet() 
	{ 
		m_header.opcode = op;
		m_data = new byte[sizeof(PacketHeader)]{ 0 };
		std::memcpy(&m_data[0], &m_header, sizeof(PacketHeader));
	}

	template<typename T, typename... Args>
	Packet(T first, Args&&... args)
	{	
		uint32_t totalSize = GetSize(first, args...) + sizeof(PacketHeader);
		m_data = new byte[totalSize]{ 0 };
		m_header.opcode = op;
		m_header.size = totalSize;
		std::memcpy(&m_data[0], &m_header, sizeof(PacketHeader));
		m_bufpos += sizeof(PacketHeader);
		Pack(first, args...);
	}

	~Packet() { if (m_data) { delete[] m_data; } }

	const byte* ToBytes() const { return m_data; }
	const uint32_t& Size() const { return m_header.size; }

private:
	uint32_t m_bufpos{ 0 };

	template<typename T>
	void Pack(T& val)
	{
		auto size = GetSize(val);
		std::memcpy(&m_data[m_bufpos], &val, size);
		m_bufpos += size;
	}

	template<typename T>
	void Pack(std::vector<T>& values)
	{
		auto size = GetSize(values);
		uint32_t len = values.size();
		std::memcpy(&m_data[m_bufpos], &len, sizeof(uint32_t));
		std::memcpy(&m_data[m_bufpos + sizeof(uint32_t)], &values[0], len);
		m_bufpos += size;
	}

	void Pack(std::string& str)
	{
		auto size = GetSize(str);
		uint32_t len = str.size();
		std::memcpy(&m_data[m_bufpos], &len, sizeof(uint32_t));
		std::memcpy(&m_data[m_bufpos + sizeof(uint32_t)], &str[0], len);
		m_bufpos += size;
	}

	void Pack(const char *str)
	{
		auto size = GetSize(str);
		auto len = size - sizeof(uint32_t);
		std::memcpy(&m_data[m_bufpos], &len, sizeof(uint32_t));
		std::memcpy(&m_data[m_bufpos + sizeof(uint32_t)], &str[0], len);
		m_bufpos += size;
	}

	template<typename T, typename... Args>
	void Pack(T first, Args&&... args)
	{
		Pack(first);
		Pack(args...);
	}
};

/*
int main()
{
	Packet p1(
		"string1", 
		std::string("string_2"),
		324,
		45.3,
		std::vector<int>{ 1, 2, 3, 4, 5, 6 });

	return 0;
}
*/
