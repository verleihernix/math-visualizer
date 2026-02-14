#include "console/Console.hpp"
#include "parser/ExpressionParser.hpp"
#include <iomanip>
#include <thread>

int main()
{
	console::open("Test");

	std::string expression = console::input("Enter a function (e.g. sin(x) * x^2)", true);

	try
	{
		auto func = parser::parseExpression(expression);

		console::print(console::Color::Yellow, true, "\nTable of values:");
		console::print(console::Color::White, true, "------------------------------------------------------------------");

		console::print(console::Color::Cyan, true,
			std::setw(10), "x",
			std::setw(10), "f(x)"
		);

		console::print(console::Color::White, true, "------------------------------------------------------------------");

		for (float x = -5.f; x <= 5.f; x += 1.f)
		{
			float y = func(x);

			console::print(
				console::Color::White,
				true,
				std::setw(10), x,
				std::setw(15), y
			);
		}
	}
	catch (const std::exception& e)
	{
		console::print(console::Color::Red, true, "\n Error while parsing:");
		console::print(console::Color::Red, true, e.what());
	}

	std::this_thread::sleep_for(std::chrono::seconds(10));

	return 0;
}