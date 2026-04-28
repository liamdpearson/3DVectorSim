#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include "ui.hpp"

const int SCREEN_WIDTH = 1000, SCREEN_HEIGHT = 1000;
const std::vector<sf::Color> colors = {
    sf::Color::Red, sf::Color::Green,
    sf::Color::Blue, sf::Color::Yellow,
    sf::Color::Magenta, sf::Color::Cyan
};

std::string vecToString(float x, float y, float z);

struct Vector3 {
    float x;
    float y;
    float z;
    
    Vector3 operator+(const Vector3& other) const {
        return Vector3{x + other.x, y + other.y, z + other.z};
    }
    Vector3 operator-(const Vector3& other) const {
        return Vector3{x - other.x, y - other.y, z - other.z};
    }
    bool operator==(const Vector3& other) const {
        return (x == other.x && y == other.y && z == other.z);
    }
    float length() const {
        return std::sqrt(x*x + y*y + z*z);
    }
};

struct Vector {
    Vector3 vec;
    sf::Color color;
    std::string text = vecToString(vec.x, vec.y, vec.z);
};

struct BasisVector {
    Vector3 vec;
    std::string text;
};

float dot_prod(const Vector3& u, const Vector3& v);

Vector3 proj(const Vector3& u, const Vector3& v);

void update_buttons(std::vector<Vector>& vectors, std::vector<Button>& del_buttons, const sf::Font& font);

std::tuple<float, float, float> calc_dist(const Vector3 vec, Vector3 cam_vec, float cam_len_sqrd, Vector3 y_vec, Vector3 x_vec);

void handle_cmd_input(std::vector<Vector>& vectors, const std::string cmd);