#pragma once

#include "../return_types.h"
#include "../../shape/bezier.h"

namespace utils::math::geometry::sdf::details::bezier::_3pt
	{
	template<storage::type storage_type, geometry::ends::optional_ab optional_ends>
	struct sdf_proxy
		{
		using shape_t = geometry::shape::generic::bezier<storage_type, 3, optional_ends>;
		sdf_proxy(const shape_t& shape, const vec2f& point) : shape{shape}, point{point} {};
		const shape_t& shape;
		const vec2f point;

		template <ends::ab ends>
		utils_gpu_available constexpr float closest_t()const noexcept
			{
			//https://www.shadertoy.com/view/NdfSDl
			const float t_min{ends.is_a_finite() ? 0.f : -utils::math::constants::finf};
			const float t_max{ends.is_b_finite() ? 1.f :  utils::math::constants::finf};

			const auto dot2{[](utils::math::vec2f v) -> float
				{
				return utils::math::vec2f::dot(v, v); 
				}};

			utils::math::vec2f c1 = point - shape.vertices[0];
			utils::math::vec2f c2 = (shape.vertices[1] * 2.f) - shape.vertices[2] - shape.vertices[0];
			utils::math::vec2f c3 =  shape.vertices[0]        - shape.vertices[1];

			// Cubic coefficients ---> t3*t^3 + t2*t^2 + t1*t + t0*t^0
			float t3 = utils::math::vec2f::dot(c2, c2);
			float t2 = utils::math::vec2f::dot(c3, c2) * 3.f;
			float t1 = utils::math::vec2f::dot(c1, c2) + 2.f * utils::math::vec2f::dot(c3, c3);
			float t0 = utils::math::vec2f::dot(c1, c3);

			// Reduce by dividing by leading coefficient
			// This simplifies out a lot of things
			t2 /= t3, t1 /= t3, t0 /= t3;

			// Depressed cubic coefficients (point and q) and precomputation
			float t22 = t2 * t2;
			utils::math::vec2f pq{t1 - t22 / 3.f, t22 * t2 / 13.5f - t2 * t1 / 3.f + t0};
			float ppp = pq.x() * pq.x() * pq.x(), qq = pq.y() * pq.y();

			float p2 = std::abs(pq.x());
			float r1 = 1.5f / pq.x() * pq.y();

			// Solutions and details gathered from here: https://en.wikipedia.org/wiki/Cubic_equation
			if (qq * 0.25f + ppp / 27.f > 0.f) 
				{ // One real root, use hyperbolic trig
				float r2 = r1 * std::sqrt(3.f / p2), root;
				if (pq.x() < 0.f) root = utils::math::sign(pq.y()) * std::cosh(std::acosh(r2 * -utils::math::sign(pq.y())) / 3.f);
				else root = std::sinh(std::asinh(r2) / 3.f);
				root = -2.f * std::sqrt(p2 / 3.f) * root - t2 / 3.f;
				root = utils::math::clamp(root, t_min, t_max);
				//return utils::math::vec2f(length(point - posBezier(shape.vertices[0], shape.vertices[1], shape.vertices[2], root)), root);
				return root;
				}
			else 
				{ // Three real roots (only need to use two), use "normal" trig
				float ac = std::acos(r1 * std::sqrt(-3.f / pq.x())) / 3.f; // 4pi/3 goes here --v
				//utils::math::vec2f roots = 2.f * std::sqrt(-pq.x() / 3.f) * std::cos(utils::math::vec2f(ac, ac - 4.18879020479f)) - t2 / 3.f;
				utils::math::vec2f roots = (utils::math::vec2f{float{std::cos(ac)}, float{std::cos(ac - 4.18879020479f)}} * 2.f * float{std::sqrt(-pq.x() / 3.f)}) - t2 / 3.f;
				roots = utils::math::clamp(roots, t_min, t_max);
				float d1 = dot2(point - shape.at(roots.x()).point());
				float d2 = dot2(point - shape.at(roots.y()).point());
				//return d1 < d2 ? utils::math::vec2f(sqrt(d1), roots.x()) : utils::math::vec2f(sqrt(d2), roots.t);
				return d1 < d2 ? roots.x() : roots.y();
				}
			}

		};
	}