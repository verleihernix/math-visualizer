#include "render/console/Console.hpp"
#include <string>
#include <thread>

int main()
{
	console::open("Math Visualizer");
	console::print(console::Color::Default, true, "Enter a function below to visualize it!");
	console::input();
}