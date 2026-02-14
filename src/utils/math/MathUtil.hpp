#pragma once
#include <SFML/Graphics.hpp>
#include <functional>

namespace math
{
	struct Viewport
	{
		float width;
		float height;

		float scale; // zoom (pixel per unit)
		float offsetX; // shift in world cords
		float offsetY;
	};

	sf::Vector2f worldToScreen(const sf::Vector2f& world, const Viewport& view);

	sf::Vector2f screenToWorld(const sf::Vector2f& world, const Viewport& view);

	sf::VertexArray sampleFunction(
		const std::function<float(float)>& func,
		const Viewport& view,
		float step = 0.01f
	);
}