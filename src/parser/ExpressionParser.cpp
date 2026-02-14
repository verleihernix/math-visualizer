#include "ExpressionParser.hpp"
#include <cctype>
#include <cmath>
#include <stdexcept>

namespace parser
{
	class ExpressionParser
	{
	public:
		ExpressionParser(const std::string& str)
			: input(str), pos(0), xValue(0.f) {}

		float evaluate(float x)
		{
			xValue = x;
			pos = 0;
			return parseExpression();
		}
	private:
		std::string input;
		size_t pos;
		float xValue;

		// helpers

		void skipWhitespace()
		{
			while (pos < input.size() && std::isspace(input[pos]))
				pos++;
		}

		bool match(char c)
		{
			skipWhitespace();
			if (pos < input.size() && input[pos] == c)
			{
				pos++;
				return true;
			}

			return false;
		}

		float parseExpression()
		{
			float value = parseTerm();

			while (true)
			{
				skipWhitespace();
				if (match('+')) value += parseTerm();
				else if (match('-')) value -= parseTerm();
				else break;
			}

			return value;
		}

		float parseTerm()
		{
			float value = parseFactor();

			while (true)
			{
				skipWhitespace();
				if (match('*')) value *= parseFactor();
				else if (match('/')) value /= parseFactor();
				else break;
			}

			return value;
		}

		float parseFactor()
		{
			float value = parseUnary();

			skipWhitespace();
			if (match('^'))
			{
				value = std::pow(value, parseFactor());
			}

			return value;
		}

		float parseUnary()
		{
			skipWhitespace();
			if (match('-')) return -parseUnary();
			if (match('+')) return parseUnary();
			return parsePrimary();
		}

		float parsePrimary()
		{
			skipWhitespace();

			// number
			if (std::isdigit(input[pos]) || input[pos] == '.')
			{
				return parseNumber();
			}

			// variable x
			if (match('x'))
			{
				return xValue;
			}

			// function
			if (std::isalpha(input[pos]))
			{
				std::string name = parseIdentifier();
				if (!match('('))
					throw std::runtime_error("Expected '(' after function");

				float arg = parseExpression();

				if (!match(')'))
					throw std::runtime_error("Missing ')'");

				return applyFunction(name, arg);
			}

			if (match('('))
			{
				float value = parseExpression();
				if (!match(')'))
					throw std::runtime_error("Missing ')");
				return value;
			}

			throw std::runtime_error("Unexpected token");
		}

		float parseNumber()
		{
			skipWhitespace();
			size_t start = pos;

			while (pos < input.size() && (std::isdigit(input[pos]) || input[pos] == '.'))
			{
				pos++;
			}

			return std::stof(input.substr(start, pos - start));
		}

		std::string parseIdentifier()
		{
			size_t start = pos;
			while (pos < input.size() && std::isalpha(input[pos]))
			{
				pos++;
			}

			return input.substr(start, pos - start);
		}

		float applyFunction(const std::string& name, float value)
		{
			if (name == "sin") return std::sin(value);
			if (name == "cos") return std::cos(value);
			if (name == "tan") return std::tan(value);
			if (name == "log") return std::log(value);
			if (name == "exp") return std::exp(value);
			if (name == "sqrt") return std::sqrt(value);

			throw std::runtime_error("Unknown function: " + name);
		}
	};

	std::function<float(float)> parseExpression(const std::string& expression)
	{
		return [expression](float x) {
			ExpressionParser parser(expression);
			return parser.evaluate(x);
		};
	}
}