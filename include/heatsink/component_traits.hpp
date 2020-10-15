#pragma once

#include <type_traits>

namespace hs
{
	namespace detail
	{
		// Get the type of an array subscript (the return type of operator[])
		// Gives void if operator[] is not defined for the given type
		template<class T>
		struct extent_type_of;

		template<class T>
		using extent_type_of_t = typename extent_type_of<T>::type;

		// Check if extent_type_of can be called without returning void
		template<class T>
		using is_deepest_extent = std::is_void<extent_type_of_t<T>>;
	}

	// An analogue to std::remove_extent
	// Works with any class type that defines operator[]
	template<class T>
	using remove_component_extent = std::conditional
		<
		detail::is_deepest_extent<T>::value,
		T,
		detail::extent_type_of_t<T>
		>;
	template<class T>
	using remove_component_extent_t = typename remove_component_extent<T>::type;

	// An analogue to std::remove_all_extents (see remove_component_extent)
	template<class T, class E = void>
	struct remove_all_component_extents;
	template<class T>
	using remove_all_component_extents_t = typename remove_all_component_extents<T>::type;

	// An analogue to std::extent (see remove_component_extent)
	template<class T>
	using component_extent = std::conditional_t
		<
		detail::is_deepest_extent<T>::value,
		std::integral_constant<size_t, 0>,
		std::integral_constant<size_t, sizeof(T) / sizeof(detail::extent_type_of_t<T>)>
		>;

	// An analogue to std::rank (see remove_component_extent)
	template<class T, class E = void>
	struct component_rank;

	/* Similar to std::decay; convert a class type (consisting of a uniform type)
	* to its scalar array equivalent
	*/
	// For example, converts glm::mat4 to GLfloat[4][4]
	template<class T, class E = void>
	struct component_decay;
	template<class T>
	using component_decay_t = typename component_decay<T>::type;
}

namespace hs
{
	template<class T>
	struct detail::extent_type_of
	{
		template<class U>
		static decltype(std::declval<U>()[0]) get_type(int);
		template<class>
		static void get_type(...);

		using type = std::remove_reference_t<decltype(get_type<T>(0))>;
	};

	template<class T, class E>
	struct component_rank
		: std::integral_constant
		<
		size_t,
		1 + component_rank<detail::extent_type_of_t<T>>::value
		>
	{
	};

		template<class T>
		struct component_rank<T, std::enable_if_t<detail::is_deepest_extent<T>::value>>
			: std::integral_constant<size_t, 0> {};

		template<class T, class E>
		struct remove_all_component_extents
		{
			using type = remove_all_component_extents_t<detail::extent_type_of_t<T>>;
		};

		template<class T>
		struct remove_all_component_extents<T, std::enable_if_t<detail::is_deepest_extent<T>::value>>
		{
			using type = T;
		};

		template<class T, class E>
		struct component_decay
		{
			using type = detail::extent_type_of_t<T>[component_extent<T>::value];
		};

		template<class T>
		struct component_decay<T, std::enable_if_t<(component_rank<T>::value == 0)>>
		{
			using type = T;
		};

		template<class T>
		struct component_decay<T, std::enable_if_t<(component_rank<T>::value > 1)>>
		{
			using extent_type = component_decay_t<remove_component_extent_t<T>>;
			using type = extent_type[component_extent<extent_type>::value];
		};
}
