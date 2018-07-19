#include <vector>
#include <string>

using byte = unsigned char;

std::size_t GetSize(std::string& str)
{
	return str.size() + 1;
}

std::size_t GetSize(const char *pStr)
{
	return strlen(pStr) + 1;
}

template<typename T>
std::size_t GetSize(std::vector<T> bytes)
{
	return bytes.size() * sizeof(T);
}

template<typename T>
constexpr std::size_t GetSize(T& val)
{
	return sizeof(T);
}

template<typename T, typename... Args>
std::size_t GetSize(T first, Args&&... args)
{
	return (GetSize(first) + GetSize(args...));
}

class Packet
{
private:
	std::vector<byte> m_data;

public:
	template<typename T, typename... Args>
	Packet(T first, Args&&... args)
	{
		std::size_t totalSize = GetSize(first, args...);
		m_data.reserve(totalSize);
		m_data.resize(totalSize, 0);
		Pack(first, args...);
	}

	~Packet() = default;

	const byte* Data() const { return m_data.data(); }

private:
	std::size_t m_pos{ 0 };

	template<typename T>
	void Pack(T& val)
	{
		auto size = GetSize(val);
		std::memcpy(&m_data[m_pos], &val, size);
		m_pos += size;
	}

	template<typename T>
	void Pack(std::vector<T> values)
	{
		auto size = GetSize(values);
		std::memcpy(&m_data[m_pos], &values[0], size);
		m_pos += size;
	}

	void Pack(std::string& val)
	{
		auto size = GetSize(val);
		std::memcpy(&m_data[m_pos], val.c_str(), size);
		m_pos += size;
	}

	void Pack(const char *val)
	{
		auto size = GetSize(val);
		std::memcpy(&m_data[m_pos], val, size);
		m_pos += size;
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
