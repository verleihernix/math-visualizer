#include "ColorUtils.hpp"

namespace colors
{
	sf::Color colorFromString(const std::string& name)
	{
        if (name == "red")     return sf::Color::Red;
        if (name == "green")   return sf::Color::Green;
        if (name == "blue")    return sf::Color::Blue;
        if (name == "yellow")  return sf::Color::Yellow;
        if (name == "cyan")    return sf::Color::Cyan;
        if (name == "magenta") return sf::Color::Magenta;
        if (name == "white")   return sf::Color::White;
        if (name == "black")   return sf::Color::Black;

        return sf::Color::White;
	}
}