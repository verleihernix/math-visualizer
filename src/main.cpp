#include "console/Console.hpp"
#include "parser/ExpressionParser.hpp"
#include "utils/math/MathUtil.hpp"
#include "utils/color/ColorUtils.hpp"
#include <SFML/Graphics.hpp>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <cstdlib>

struct FunctionEntry
{
	std::function<float(float)> func;
	sf::Color color;
	sf::VertexArray cached_graph{ sf::PrimitiveType::LineStrip };
};

std::vector<FunctionEntry> functions;
std::mutex functions_mutex;

math::Viewport viewport{ 1200.f,800.f,50.f,0.f,0.f };
std::atomic<bool> running = true;
std::atomic<bool> viewport_dirty = true;

void renderLoop()
{
	sf::RenderWindow window(
		sf::VideoMode({ (unsigned)viewport.height, (unsigned)viewport.width }),
		"Visualizer"
	);

	window.setVerticalSyncEnabled(true);

	sf::Vector2i last_mouse_pos;
	bool dragging = false;

	while (window.isOpen() && running)
	{
		while (auto event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				running = false;
				window.close();
			}

			// zoom via scroll
			if (const auto* scroll = event->getIf<sf::Event::MouseWheelScrolled>())
			{
				if (scroll->delta > 0)
					viewport.scale *= 1.1f;
				else
					viewport.scale /= 1.1f;

				viewport_dirty = true;
			}

			// Start Drag
			if (const auto* pressed = event->getIf<sf::Event::MouseButtonPressed>())
			{
				if (pressed->button == sf::Mouse::Button::Left)
				{
					dragging = true;
					last_mouse_pos = sf::Mouse::getPosition(window);
				}
			}

			// stop Drag
			if (const auto* released = event->getIf<sf::Event::MouseButtonReleased>())
			{
				if (released->button == sf::Mouse::Button::Left)
					dragging = false;
			}

			// drag viewport with mouse
			if (dragging)
			{
				sf::Vector2i current_pos = sf::Mouse::getPosition(window);
				sf::Vector2i delta = current_pos - last_mouse_pos;

				viewport.offsetX += -delta.x / viewport.scale;
				viewport.offsetY += delta.y / viewport.scale;

				last_mouse_pos = current_pos;

				viewport_dirty = true;
			}
		}

		window.clear(sf::Color::Black);

		// draw axis
		{
			sf::Vertex x_axis[] =
			{
				{ math::worldToScreen({ -1000.f, 0.f }, viewport), sf::Color::White },
				{ math::worldToScreen({  1000.f, 0.f }, viewport), sf::Color::White }
			};

			sf::Vertex y_axis[] =
			{
				{ math::worldToScreen({ 0.f, -1000.f }, viewport), sf::Color::White },
				{ math::worldToScreen({ 0.f,  1000.f }, viewport), sf::Color::White }
			};

			window.draw(x_axis, 2, sf::PrimitiveType::Lines);
			window.draw(y_axis, 2, sf::PrimitiveType::Lines);
		}

		// graphs
		{
			std::lock_guard<std::mutex> lock(functions_mutex);

			if (viewport_dirty)
			{
				float step = 1.f / viewport.scale; // zoom based sampling
				if (step > 0.5f) step = 0.5f;
				if (step < 0.0005f) step = 0.0005f;

				for (auto& func : functions)
				{
					func.cached_graph = math::sampleFunction(func.func, viewport, step);

					for (std::size_t itr = 0; itr < func.cached_graph.getVertexCount(); ++itr)
						func.cached_graph[itr].color = func.color;
				}

				viewport_dirty = false;
			}

			for (auto& func : functions)
				window.draw(func.cached_graph);
		}

		window.display();
	}
}

int main()
{
	SetConsoleTitle("Math Visualizer");
	console::open("Math Visualizer v1.1");

	std::thread renderThread(renderLoop);

	while (running)
	{
		std::string cmd = console::input();

		if (cmd == "exit")
		{
			running = false;
			break;
		}

		if (cmd == "help")
		{
			console::print(console::Color::Cyan, true, "\nAvailable Commands:");
			
			console::print(console::Color::White, true, "plot <expression>");
				console::print(console::Color::Default, true, "  Draws a function");
			console::print(console::Color::White, true, "clear");
				console::print(console::Color::Default, true, "  Removes all functions");
			console::print(console::Color::White, true, "help");
				console::print(console::Color::Default, true, "  Shows this menu");
			continue;
		}

		if (cmd == "clear")
		{
			std::lock_guard<std::mutex> lock(functions_mutex);
			functions.clear();
			viewport_dirty = true;
			console::print(console::Color::Yellow, true, "Removed all functions");
			continue;
		}

		if (cmd.rfind("plot", 0) == 0)
		{
			std::string expr = cmd.substr(5);

			try
			{
				auto func = parser::parseExpression(expr);
				std::lock_guard<std::mutex> lock(functions_mutex);

				functions.push_back({
					func,
					sf::Color(
						std::rand() % 255,
						std::rand() % 255,
						std::rand() % 255
					)
				});

				viewport_dirty = true;
				console::print(console::Color::Green, true, "Function added");
			}
			catch (const std::exception& e)
			{
				console::print(console::Color::Red, true, "Error: ", e.what());
			}

			continue;
		}

		console::print(console::Color::Red, true, "Unknown command. Type 'help'");
	}

	renderThread.join();
	return 0;
}