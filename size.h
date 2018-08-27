namespace internal
{
	template<std::size_t N, typename T, typename... Ts>
	struct s
	{
		constexpr static std::size_t size = sizeof(T) + s<N - 1, Ts...>::size;
	};
	
	template<typename T, typename... Ts>
	struct s<1, T, Ts...>
	{
		constexpr static std::size_t size = sizeof(T);
	};

	template<typename T, typename... Ts>
	struct s<0, T, Ts...>
	{
		constexpr static std::size_t size = 0;
	};
}

template<typename... Ts>
constexpr std::size_t size()
{
        return internal::s<sizeof...(Ts), Ts...>::size;
}
