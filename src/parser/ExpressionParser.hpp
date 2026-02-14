#pragma once
#include <string>
#include <functional>

namespace parser
{
	std::function<float(float)> parseExpression(const std::string& expression);
}