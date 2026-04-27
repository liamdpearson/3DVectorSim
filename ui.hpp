#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

class Button {
public:
    Button(sf::Vector2f pos, sf::Vector2f size, const std::string& label,
           const sf::Font& font, std::function<void()> onClick);

    bool handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window);
    bool contains(sf::Vector2f point) const;

private:
    sf::RectangleShape shape;
    sf::Text text;
    std::function<void()> onClickFn;
};
