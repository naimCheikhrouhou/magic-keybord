#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <cstdlib>

// Button class
class Button {
public:
    sf::RectangleShape shape;
    sf::Text text;

    Button(float x, float y, float width, float height, const sf::String& label, sf::Font& font) {
        shape.setSize(sf::Vector2f(width, height));
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color(50, 50, 100));

        text.setFont(font);
        text.setString(label);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);

        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.width / 2, textRect.height / 2);
        text.setPosition(x + width / 2, y + height / 2 - 5);
    }

    bool isHovered(const sf::RenderWindow& window) {
        auto mousePos = sf::Mouse::getPosition(window);
        return shape.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Magic Tiles - Menu");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Couldn't load font!" << std::endl;
        return 1;
    }

    sf::Text title("MAGIC TILES", font, 48);
    title.setFillColor(sf::Color::Cyan);
    title.setPosition(250, 100);

    Button startButton(300, 250, 200, 60, "Start Game", font);
    Button exitButton(300, 350, 200, 60, "Exit", font);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (startButton.isHovered(window)) {
                    // Replace with a system call to your game or function call
                    std::cout << "Starting Game...\n";
                    system("./game.exe"); // change to your game executable name
                }
                else if (exitButton.isHovered(window)) {
                    window.close();
                }
            }
        }

        // Hover effects
        startButton.shape.setFillColor(startButton.isHovered(window) ? sf::Color::Blue : sf::Color(50, 50, 100));
        exitButton.shape.setFillColor(exitButton.isHovered(window) ? sf::Color::Red : sf::Color(50, 50, 100));

        window.clear(sf::Color(30, 30, 40));
        window.draw(title);
        window.draw(startButton.shape);
        window.draw(startButton.text);
        window.draw(exitButton.shape);
        window.draw(exitButton.text);
        window.display();
    }

    return 0;
}
