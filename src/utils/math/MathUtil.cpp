#include "MathUtil.hpp"
#include <cmath>

namespace math
{
	sf::Vector2f worldToScreen(const sf::Vector2f& world, const Viewport& view)
	{
		float x = (world.x - view.offsetX) * view.scale + view.width / 2.f;
		float y = view.height / 2.f - (world.y - view.offsetY) * view.scale;

		return { x, y };
	}

	sf::Vector2f screenToWorld(const sf::Vector2f& screen, const Viewport& view)
	{
		float x = (screen.x - view.width / 2.f) / view.scale + view.offsetX;
		float y = (view.height / 2.f - screen.y) / view.scale + view.offsetY;

		return { x, y };
	}

	sf::VertexArray sampleFunction(
		const std::function<float(float)>& func,
		const Viewport& view,
		float step
	)
	{
		sf::VertexArray graph(sf::PrimitiveType::LineStrip);

		float worldLeft = screenToWorld({ 0.f, 0.f }, view).x;
		float worldRight = screenToWorld({ view.width, 0.f }, view).x;

		for (float x = worldLeft; x <= worldRight; x += step)
		{
			float y = func(x);

			if (std::isfinite(y))
			{
				sf::Vector2f screen = worldToScreen({ x, y }, view);

				sf::Vertex vertex;
				vertex.position = screen;
				vertex.color = sf::Color::White;

				graph.append(vertex);
			}
		}
		
		return graph;
	}
}