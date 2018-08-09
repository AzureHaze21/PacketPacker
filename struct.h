#include <vector>
#include <string>
#include <tuple>
#include <cstring>

std::size_t GetSize(std::string& str)
{
	return str.size() + sizeof(uint16_t);
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
char *pack(Ts&&... ts)
{
	auto size = GetSize(ts...);
	char *data = new char[size];
	std::size_t i = 0;
	auto l = { (packArg(&data, i, ts), 0)... };

	return data;
}

/*
**
**	Unpack
**
*/

template<typename T>
inline T readValue(const char **data)
{
	static_assert(
		!std::is_pointer<T>::value && !std::is_class<T>::value,
		__FUNCTION__": parameter is not a basic type or a string");

	T ret = *(T*)(*data);
	*data += sizeof(T);

	return ret;
}

template<>
inline std::string readValue<std::string>(const char **data)
{
	auto size = *(uint16_t*)(*data);
	std::string ret(*data + sizeof(uint16_t), size);
	*data += size + sizeof(uint16_t);

	return ret;
}

template<class Tuple, std::size_t N, std::size_t I>
struct TupleMaker
{
	template<typename T, typename... Ts>
	static void write(Tuple& t, const char **data)
	{
		std::get<I - N>(t) = readValue<T>(data);
		TupleMaker<Tuple, N - 1, I>::write<Ts...>(t, data);
	}
};

template<class Tuple, std::size_t I>
struct TupleMaker<Tuple, 1, I>
{
	template<typename T>
	static void write(Tuple& t, const char **data)
	{
		std::get<I - 1>(t) = readValue<T>(data);
	}
};

template<class... Args>
auto unpack(const char *data)
{
	std::tuple<Args...> t;
	TupleMaker<decltype(t), sizeof...(Args), sizeof...(Args)>::write<Args...>(t, &data);

	return t;
}
