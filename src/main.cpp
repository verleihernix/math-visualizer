#include "console/Console.hpp"
#include "parser/ExpressionParser.hpp"
#include "utils/math/MathUtil.hpp"
#include "utils/color/ColorUtils.hpp"

#include <SFML/Graphics.hpp>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>

struct FunctionEntry {
    std::function<float(float)> func;
    sf::Color color;
};

std::vector<FunctionEntry> functions;
std::mutex functions_mutex;

math::Viewport viewport{ 1200.f, 800.f, 50.f, 0.f, 0.f };
std::atomic<bool> running = true;

void renderLoop() {
    sf::RenderWindow window(
        sf::VideoMode({ (unsigned)viewport.height, (unsigned)viewport.width }),
        "Visualizer"
    );

    window.setFramerateLimit(60);

    sf::Vector2i last_mouse_pos;
    bool dragging = false;

    while (window.isOpen() && running) {
        while (auto event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>()) {
                running = false;
                window.close();
            }

            // Zoom
            if (const auto* scroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (scroll->delta > 0)
                    viewport.scale *= 1.1f;
                else
                    viewport.scale /= 1.1f;
            }

            // Start dragging
            if (const auto* mouse_pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouse_pressed->button == sf::Mouse::Button::Left) {
                    dragging = true;
                    last_mouse_pos = sf::Mouse::getPosition(window);
                }
            }

            // Stop dragging
            if (const auto* mouse_released = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouse_released->button == sf::Mouse::Button::Left)
                    dragging = false;
            }

            // Move viewport while dragging
            if (dragging) {
                sf::Vector2i current_pos = sf::Mouse::getPosition(window);
                sf::Vector2i delta = current_pos - last_mouse_pos;

                viewport.offsetX += -delta.x / viewport.scale;
                viewport.offsetY += delta.y / viewport.scale;

                last_mouse_pos = current_pos;
            }
        }

        window.clear(sf::Color::Black);

        // Draw axes
        {
            sf::Vertex x_vertex_1;
            x_vertex_1.position = math::worldToScreen({ -1000.f, 0.f }, viewport);
            x_vertex_1.color = sf::Color::White;

            sf::Vertex x_vertex_2;
            x_vertex_2.position = math::worldToScreen({ 1000.f, 0.f }, viewport);
            x_vertex_2.color = sf::Color::White;

            sf::Vertex x_axis[] = { x_vertex_1, x_vertex_2 };

            sf::Vertex y_vertex_1;
            y_vertex_1.position = math::worldToScreen({ 0.f, -1000.f }, viewport);
            y_vertex_1.color = sf::Color::White;

            sf::Vertex y_vertex_2;
            y_vertex_2.position = math::worldToScreen({ 0.f, 1000.f }, viewport);
            y_vertex_2.color = sf::Color::White;

            sf::Vertex y_axis[] = { y_vertex_1, y_vertex_2 };

            window.draw(x_axis, 2, sf::PrimitiveType::Lines);
            window.draw(y_axis, 2, sf::PrimitiveType::Lines);
        }

        // Draw functions
        {
            std::lock_guard<std::mutex> lock(functions_mutex);

            for (auto& f : functions) {
                auto graph = math::sampleFunction(f.func, viewport, 0.01f);

                for (std::size_t itr = 0; itr < graph.getVertexCount(); ++itr)
                    graph[itr].color = f.color;

                window.draw(graph);
            }
        }

        window.display();
    }
}

int main() {
    SetConsoleTitle("Math Visualizer");
    console::open("Math Visualizer v1");

    std::thread renderThread(renderLoop);

    while (running) {
        std::string cmd = console::input();

        if (cmd == "exit") {
            running = false;
            break;
        }

        if (cmd == "help") {
            console::print(console::Color::Cyan, true, "\nAvailable Commands:");
            console::print(console::Color::White, true,
                " plot <expression> - Plot a mathematical function (e.g., plot sin(x))");
            console::print(console::Color::White, true,
                " clear - Remove all plotted functions");
            console::print(console::Color::White, true,
                " zoom <factor> - Zoom in/out (e.g., zoom 1.5 or zoom 0.5)");
            console::print(console::Color::White, true,
                " pan <dx> <dy> - Move viewport (e.g., pan 10 5)");
            console::print(console::Color::White, true,
                " help - Show this help message");
            console::print(console::Color::White, true,
                " exit - Close the application\n");
            continue;
        }

        if (cmd == "clear") {
            std::lock_guard<std::mutex> lock(functions_mutex);
            functions.clear();
            console::print(console::Color::Yellow, true, "Removed all functions");
            continue;
        }

        if (cmd.rfind("zoom", 0) == 0) {
            float factor = std::stof(cmd.substr(5));
            viewport.scale *= factor;
            console::print(console::Color::Cyan, true, "Changed zoom");
            continue;
        }

        if (cmd.rfind("pan", 0) == 0) {
            float dx = 0.f, dy = 0.f;
            sscanf_s(cmd.c_str(), "pan %f %f", &dx, &dy);

            viewport.offsetX += dx;
            viewport.offsetY += dy;

            console::print(console::Color::Cyan, true, "Viewport moved");
            continue;
        }

        if (cmd.rfind("plot", 0) == 0) {
            std::string expr = cmd.substr(5);

            try {
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

                console::print(console::Color::Green, true, "Function added");
            }
            catch (const std::exception& e) {
                console::print(console::Color::Red, true, "Error: ", e.what());
            }

            continue;
        }

        console::print(console::Color::Red, true,
            "Unknown command. Type 'help' for available commands");
    }

    renderThread.join();
    return 0;
}
