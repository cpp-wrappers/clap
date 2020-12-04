#pragma once

#include <iterator>
#include <type_traits>
#include <concepts>

namespace clap {

template<class T, class CharT>
concept convertible_to_string_view = std::convertible_to<T, std::basic_string_view<CharT>>;

template<class T, class CharT>
static constexpr bool is_constructible_from_string_view = std::is_constructible_v<T, std::basic_string_view<CharT>>;

template<class T, class CharT>
static constexpr bool is_string_view_assignable_v = std::is_assignable_v<T, std::basic_string_view<CharT>& >;

template<class It, class CharT>
concept iterator_value_convertible_to_string_view =
	std::input_iterator<It> &&
	convertible_to_string_view<std::iter_value_t<It>, CharT>;
}