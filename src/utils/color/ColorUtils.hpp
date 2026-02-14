#pragma once
#include <SFML/Graphics.hpp>
#include <string>

namespace colors
{
	sf::Color colorFromString(const std::string& name);
}