#pragma once

#include "../../details/base_types.h"
#include "../../../rect.h"

namespace utils::math::geometry::shape
	{
	namespace generic
		{
		template <storage::type storage_type>
		using aabb = ::utils::math::rect<utils::storage::storage_type_for<float, storage_type>>;
		}
	namespace concepts
		{
		template <typename T>
		concept aabb = std::same_as<T, shape::generic::aabb<T::storage_type>>;
		}

	namespace owner         { using aabb = shape::generic::aabb<storage::type::create::owner         ()>; }
	namespace observer      { using aabb = shape::generic::aabb<storage::type::create::observer      ()>; }
	namespace const_observer{ using aabb = shape::generic::aabb<storage::type::create::const_observer()>; }
	
	template <concepts::aabb T, storage::type desired_storage_type>
	struct cast_storage_type<T, desired_storage_type>
		{
		using type = generic::aabb<desired_storage_type>;
		};
	}