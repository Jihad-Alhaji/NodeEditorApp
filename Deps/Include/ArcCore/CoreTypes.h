#pragma once
#include"GLM/glm.hpp"
using Point2D_t = typename glm::vec2;
using Size2D_t = typename glm::vec2;

struct Rect 
{
	Rect() = default;
	Rect(const Point2D_t& origin, const Size2D_t& size);

	Point2D_t Origin;
	Size2D_t Size;
};

bool operator==(const Rect& a, const Rect& b);
bool Intersects(const Rect& a, const Rect& b);
bool Intersects(const Rect& a, const Point2D_t& point);