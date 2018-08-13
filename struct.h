#include <vector>
#include <string>
#include <tuple>
#include <cstring>

/*
**
**	Packet
**
*/

class Packet
{
	char *m_data = nullptr;
	std::size_t m_size{ 0 };

public:
	Packet() = default;
	Packet(char *data, std::size_t size) : m_data(data), m_size(size) {}
	~Packet() { if (m_data) delete[] m_data; }

	const char *data() const { return m_data; }
	std::size_t size() const { return m_size; }
};

std::size_t GetSize(std::string& str)
{
	return str.size() + sizeof(uint16_t); //each string is prefixed with its size for unpacking
}

std::size_t GetSize(const char *pStr)
{
	return std::strlen(pStr) + sizeof(uint16_t);
}

template<typename T>
constexpr std::size_t GetSize(T& val)
{
	return sizeof(T);
}

template<typename T, typename... Ts>
std::size_t GetSize(T& first, Ts&... ts)
{
	return (GetSize(first) + GetSize(ts...));
}

/*
**
**	Pack
**
*/

template<typename T>
inline void packArg(char **data, std::size_t& i, T val)
{
	auto size = GetSize(val);
	std::memcpy(*data + i, &val, size);
	i += size;
}

template<>
inline void packArg(char **data, std::size_t& i, std::string& val)
{
	auto size = static_cast<uint16_t>(val.size());
	std::memcpy(*data + i, &size, sizeof(uint16_t));
	std::memcpy(*data + i + sizeof(uint16_t), val.c_str(), size);
	i += size + sizeof(uint16_t);
}

template<>
inline void packArg(char **data, std::size_t& i, const char *val)
{
	auto size = static_cast<uint16_t>(strlen(val));
	std::memcpy(*data + i, &size, sizeof(uint16_t));
	std::memcpy(*data + i + sizeof(uint16_t), val, size);
	i += size + sizeof(uint16_t);
}

template <typename... Ts>
auto pack(Ts&&... ts)
{
	auto size = GetSize(ts...);
	char *data = new char[size];

	if (data == nullptr)
		return Packet();

	std::size_t pos = 0;
	auto l = { (packArg(&data, pos, ts), 0)... };

	return Packet(data, size);
}

/*
**
**	Unpack
**
*/

template<typename T>
inline void readValue(const char **data, T& elem)
{
	static_assert(
		!std::is_pointer<T>::value && !std::is_class<T>::value,
		__FUNCTION__": parameter is not a basic type or a string");

	elem = *(T*)(*data);
	*data += sizeof(T);
}

template<>
inline void readValue<std::string>(const char **data, std::string& elem)
{
	auto size = *(uint16_t*)(*data);
	elem.assign(*data + sizeof(uint16_t), size);
	*data += size + sizeof(uint16_t);
}

template<class Tuple, std::size_t N, std::size_t I>
struct TupleMaker
{
	template<typename T, typename... Ts>
	static void write(Tuple& t, const char **data)
	{
		readValue<T>(data, std::get<I - N>(t));
		TupleMaker<Tuple, N - 1, I>::write<Ts...>(t, data);
	}
};

template<class Tuple, std::size_t I>
struct TupleMaker<Tuple, 1, I>
{
	template<typename T>
	static void write(Tuple& t, const char **data)
	{
		 readValue<T>(data, std::get<I - 1>(t));
	}
};

template<class... Args>
auto unpack(const char *data)
{
	std::tuple<Args...> t;
	TupleMaker<decltype(t), sizeof...(Args), sizeof...(Args)>::write<Args...>(t, &data);

	return t;
}
