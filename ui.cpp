#include "ui.hpp"

Button::Button(sf::Vector2f pos, sf::Vector2f size, const std::string& label,
               const sf::Font& font, std::function<void()> onClick)
    : onClickFn(onClick)
{
    shape.setPosition(pos);
    shape.setSize(size);
    shape.setFillColor(sf::Color(50, 50, 50));
    shape.setOutlineColor(sf::Color::White);
    shape.setOutlineThickness(1.f);

    text = sf::Text(label, font, 14);
    sf::FloatRect b = text.getLocalBounds();
    text.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    text.setPosition(pos.x + size.x / 2.f, pos.y + size.y / 2.f);
}

void Button::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonReleased &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mouse(event.mouseButton.x, event.mouseButton.y);
        if (shape.getGlobalBounds().contains(mouse))
            onClickFn();
    }
}

void Button::draw(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(text);
}

bool Button::contains(sf::Vector2f point) const {
    return shape.getGlobalBounds().contains(point);
}
