#pragma once

#include <algorithm>
#include <cmath>
#include <iterator>
#include <type_traits>
#include <utility>

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct Rect;

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct Vec2
{
	T X, Y;

	constexpr Vec2(T x, T y):
		X(x),
		Y(y)
	{
	}

	template<typename S, typename = std::enable_if_t<std::is_constructible_v<T, S>>>
	constexpr explicit Vec2(Vec2<S> other):
		X(other.X),
		Y(other.Y)
	{
	}

	constexpr bool operator==(Vec2<T> other) const
	{
		return X == other.X && Y == other.Y;
	}

	constexpr bool operator!=(Vec2<T> other) const
	{
		return X != other.X || Y != other.Y;
	}

	template<typename S>
	constexpr Vec2<decltype(std::declval<T>() + std::declval<S>())> operator+(Vec2<S> other) const
	{
		return Vec2<decltype(std::declval<T>() + std::declval<S>())>(X + other.X, Y + other.Y);
	}

	constexpr Vec2<T> operator-() const
	{
		return Vec2<T>(-X, -Y);
	}

	template<typename S>
	constexpr Vec2<decltype(std::declval<T>() - std::declval<S>())> operator-(Vec2<S> other) const
	{
		return Vec2<decltype(std::declval<T>() - std::declval<S>())>(X - other.X, Y - other.Y);
	}

	template<typename S, typename = std::enable_if_t<std::is_arithmetic_v<S>>>
	constexpr Vec2<decltype(std::declval<T>() * std::declval<S>())> operator*(S other) const
	{
		return Vec2<decltype(std::declval<T>() * std::declval<S>())>(X * other, Y * other);
	}

	template<typename S, typename = std::enable_if_t<std::is_arithmetic_v<S>>>
	constexpr Vec2<decltype(std::declval<T>() / std::declval<S>())> operator/(S other) const
	{
		return Vec2<decltype(std::declval<T>() / std::declval<S>())>(X / other, Y / other);
	}

	template<typename S>
	constexpr Vec2<T> &operator+=(Vec2<S> other)
	{
		return *this = *this + other;
	}

	template<typename S>
	constexpr Vec2<T> &operator-=(Vec2<S> other)
	{
		return *this = *this - other;
	}

	template<typename S>
	constexpr Vec2<T> &operator*=(Vec2<S> other)
	{
		return *this = *this * other;
	}

	template<typename S>
	constexpr Vec2<T> &operator/=(Vec2<S> other)
	{
		return *this = *this / other;
	}

	// Round towards -infinity
	template<typename S = T, typename = std::enable_if_t<std::is_floating_point_v<S>>>
	Vec2<T> Floor() const
	{
		return Vec2<T>(std::floor(X), std::floor(Y));
	}

	// Round towards nearest integer, halfpoints towards -infinity
	template<typename S = T, typename = std::enable_if_t<std::is_floating_point_v<S>>>
	Vec2<T> Round() const
	{
		return (*this + Vec2<T>(0.5, 0.5)).Floor();
	}

	Vec2<T> Clamp(Rect<T> rect) const
	{
		return Vec2<T>(
			std::clamp(X, rect.pos.X, rect.pos.X + rect.size.X - T(1)),
			std::clamp(Y, rect.pos.Y, rect.pos.Y + rect.size.Y - T(1))
		);
	}

	Vec2<T> Min(Vec2<T> other) const
	{
		return Vec2<T>(
			std::min(X, other.X),
			std::min(Y, other.Y)
		);
	}

	// Return a rectangle starting at origin, whose dimensions match this vector
	template<typename S = T, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr inline Rect<T> OriginRect() const
	{
		return RectSized(Vec2<T>(0, 0), *this);
	}

	static Vec2<T> const Zero;
};

template<typename T, typename V>
Vec2<T> const Vec2<T, V>::Zero = Vec2<T>(0, 0);

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct Mat2
{
	// ⎛A B⎞
	// ⎝C D⎠, acting on column vectors
	T A, B, C, D;

	constexpr Mat2(T a, T b, T c, T d):
		A(a),
		B(b),
		C(c),
		D(d)
	{
	}

	constexpr bool operator==(Mat2<T> other) const
	{
		return A == other.A && B == other.B && C == other.C && D == other.D;
	}

	constexpr bool operator!=(Mat2<T> other) const
	{
		return A != other.A || B != other.B || C != other.C || D != other.D;
	}

	template<typename S>
	constexpr Vec2<decltype(std::declval<T>() * std::declval<S>())> operator*(Vec2<S> vec) const
	{
		return Vec2<decltype(std::declval<T>() * std::declval<S>())>(A * vec.X + B * vec.Y, C * vec.X + D * vec.Y);
	}

	template<typename S>
	constexpr Mat2<decltype(std::declval<T>() * std::declval<S>())> operator*(Mat2<S> mat) const
	{
		return Mat2<decltype(std::declval<T>() * std::declval<S>())>(
			A * mat.A + B * mat.C, A * mat.B + B * mat.D,
			C * mat.A + D * mat.C, C * mat.B + D * mat.D
		);
	}

	static Mat2<T> const Identity, MirrorX, MirrorY, CCW;
};

template<typename T, typename V>
Mat2<T> const Mat2<T, V>::Identity = Mat2<T>(1, 0, 0, 1);
template<typename T, typename V>
Mat2<T> const Mat2<T, V>::MirrorX = Mat2<T>(-1, 0, 0, 1);
template<typename T, typename V>
Mat2<T> const Mat2<T, V>::MirrorY = Mat2<T>(1, 0, 0, -1);
template<typename T, typename V>
Mat2<T> const Mat2<T, V>::CCW = Mat2<T>(0, 1, -1, 0); // reminder: the Y axis points down

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr static inline Rect<T> RectSized(Vec2<T>, Vec2<T>);

enum IterationDirection
{
	TOP_TO_BOTTOM,
	BOTTOM_TO_TOP,
	LEFT_TO_RIGHT,
	RIGHT_TO_LEFT,
};

template<typename T, typename>
struct Rect
{
	// Inclusive
	Vec2<T> pos, size;

	constexpr Rect(Vec2<T> newPos, Vec2<T> newSize):
		pos(newPos),
		size(newSize)
	{
	}

private:
	struct end_sentinel
	{};

	template<IterationDirection D1, IterationDirection D2>
	struct range_row_major
	{
		static_assert(D1 == TOP_TO_BOTTOM || D1 == BOTTOM_TO_TOP);
		static_assert(D2 == LEFT_TO_RIGHT || D2 == RIGHT_TO_LEFT);
		T left, top, right, bottom;

		struct iterator
		{
			T x, y;
			T const first_x, last_x, end_y;

			iterator &operator++()
			{
				if (x == last_x)
				{
					x = first_x;
					if constexpr (D1 == TOP_TO_BOTTOM)
						y++;
					else
						y--;
				}
				else
				{
					if constexpr (D2 == LEFT_TO_RIGHT)
						x++;
					else
						x--;
				}
				return *this;
			}

			Vec2<T> operator*() const
			{
				return Vec2<T>(x, y);
			}

			bool operator!=(end_sentinel) const
			{
				if constexpr (D1 == TOP_TO_BOTTOM)
					return y < end_y;
				else
					return y > end_y;
			}

			using difference_type = void;
			using value_type = Vec2<T>;
			using pointer = void;
			using reference = void;
			using iterator_category = std::forward_iterator_tag;
		};

		iterator begin() const
		{
			T first_x = D2 == LEFT_TO_RIGHT ? left : right;
			T last_x = D2 == LEFT_TO_RIGHT ? right : left;
			T first_y = D1 == TOP_TO_BOTTOM ? top : bottom;
			T end_y = D1 == TOP_TO_BOTTOM ? bottom + 1 : top - 1;
			return iterator{first_x, right >= left ? first_y : end_y, first_x, last_x, end_y};
		}

		end_sentinel end() const
		{
			return end_sentinel();
		}
	};

	template<IterationDirection D1, IterationDirection D2>
	struct range_column_major
	{
		static_assert(D1 == LEFT_TO_RIGHT || D1 == RIGHT_TO_LEFT);
		static_assert(D2 == TOP_TO_BOTTOM || D2 == BOTTOM_TO_TOP);
		T left, top, right, bottom;

		struct iterator
		{
			T x, y;
			T const first_y, last_y, end_x;

			iterator &operator++()
			{
				if (y == last_y)
				{
					y = first_y;
					if constexpr (D1 == LEFT_TO_RIGHT)
						x++;
					else
						x--;
				}
				else
				{
					if constexpr (D2 == TOP_TO_BOTTOM)
						y++;
					else
						y--;
				}
				return *this;
			}

			Vec2<T> operator*() const
			{
				return Vec2<T>(x, y);
			}

			bool operator!=(end_sentinel) const
			{
				if constexpr (D1 == LEFT_TO_RIGHT)
					return x < end_x;
				else
					return x > end_x;
			}

			using difference_type = void;
			using value_type = Vec2<T>;
			using pointer = void;
			using reference = void;
			using iterator_category = std::forward_iterator_tag;
		};

		iterator begin() const
		{
			T first_y = D2 == TOP_TO_BOTTOM ? top : bottom;
			T last_y = D2 == TOP_TO_BOTTOM ? bottom : top;
			T first_x = D1 == LEFT_TO_RIGHT ? left : right;
			T end_x = D1 == LEFT_TO_RIGHT ? right + 1 : left - 1;
			return iterator{bottom >= top ? first_x : end_x, first_y, first_y, last_y, end_x};
		}

		end_sentinel end() const
		{
			return end_sentinel();
		}
	};

public:
	constexpr bool operator==(Rect other) const
	{
		return pos == other.pos && size == other.size;
	}

	constexpr bool operator!=(Rect other) const
	{
		return pos != other.pos || size != other.size;
	}

	constexpr explicit operator bool() const
	{
		return size.X > 0 && size.Y > 0;
	}

	// Return the intersection of two rectangles (possibly empty)
	Rect<T> operator&(Rect<T> other) const
	{
		auto tl  = Vec2<T>(std::max(pos.X         , other.pos.X               ), std::max(pos.Y         , other.pos.Y               ));
		auto br1 = Vec2<T>(std::min(pos.X + size.X, other.pos.X + other.size.X), std::min(pos.Y + size.Y, other.pos.Y + other.size.Y));
		auto size = br1 - tl;
		return Rect<T>(tl, Vec2<T>(std::max(size.X, T(0)), std::max(size.Y, T(0))));
	}

	inline Rect<T> &operator&=(Rect<T> other)
	{
		return *this = *this & other;
	}

	constexpr bool Contains(Vec2<T> point) const
	{
		return point.X >= pos.X && point.X < pos.X + size.X && point.Y >= pos.Y && point.Y < pos.Y + size.Y;
	}

	template<typename S = T, typename = std::enable_if_t<std::is_integral_v<S>>>
	inline Vec2<T> TopLeft() const
	{
		return pos;
	}

	template<typename S = T, typename = std::enable_if_t<std::is_integral_v<S>>>
	inline Vec2<T> BottomRight() const
	{
		return pos + size - Vec2<T>(1, 1);
	}

	template<typename S>
	Rect<decltype(std::declval<T>() + std::declval<S>())> Inset(S delta) const
	{
		return Rect<decltype(std::declval<T>() + std::declval<S>())>(pos + Vec2(delta, delta), size - Vec2(delta, delta) * S(2));
	}

	template<IterationDirection D1, IterationDirection D2, typename S = T, typename = std::enable_if_t<std::is_integral_v<T>>>
	constexpr auto Range() const
	{
		static_assert(
			((D1 == TOP_TO_BOTTOM || D1 == BOTTOM_TO_TOP) && (D2 == LEFT_TO_RIGHT || D2 == RIGHT_TO_LEFT)) ||
			((D1 == LEFT_TO_RIGHT || D1 == RIGHT_TO_LEFT) && (D2 == TOP_TO_BOTTOM || D2 == BOTTOM_TO_TOP)),
			"Must include exactly 1 of TOP_TO_BOTTOM/BOTTOM_TO_TOP and exactly 1 of LEFT_TO_RIGHT/RIGHT_TO_LEFT"
		);
		if constexpr (D1 == TOP_TO_BOTTOM || D1 == BOTTOM_TO_TOP)
		{
			return range_row_major<D1, D2>{pos.X, pos.Y, pos.X + size.X - T(1), pos.Y + size.Y - T(1)};
		}
		else
			return range_column_major<D1, D2>{pos.X, pos.Y, pos.X + size.X - T(1), pos.Y + size.Y - T(1)};
	}

	// Use when the order isn't important
	constexpr typename range_row_major<TOP_TO_BOTTOM, LEFT_TO_RIGHT>::iterator begin() const
	{
		return Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>().begin();
	}

	constexpr end_sentinel end() const
	{
		return end_sentinel();
	}
};

template<typename T, typename>
constexpr inline Rect<T> RectSized(Vec2<T> pos, Vec2<T> size)
{
	return Rect<T>(pos, size);
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr inline Rect<T> RectBetween(Vec2<T> topLeft, Vec2<T> bottomRight)
{
	return RectSized(topLeft, bottomRight - topLeft + Vec2<T>(1, 1));
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr inline Rect<T> RectAt(Vec2<T> pos)
{
	return RectSized(pos, Vec2<T>(1, 1));
}
