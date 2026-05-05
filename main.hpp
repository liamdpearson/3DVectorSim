#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <string>
#include <iomanip>
#include "ui.hpp"

const int SCREEN_WIDTH = 1000, SCREEN_HEIGHT = 1000;
const std::vector<sf::Color> vector_colors = {
    sf::Color::Green,
    sf::Color::Yellow,
    sf::Color::Cyan,
    sf::Color::Red,
    sf::Color::Magenta
};

const std::vector<sf::Color> plane_colors = {
    sf::Color::Red,
    sf::Color::Blue,
    sf::Color::Magenta
};

std::string float_to_frac(float val);
std::string nvec_to_string(float x, float y, float z);

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
    void normalize() {
        float len = length();
        if (len > 0) {
            x /= len;
            y /= len;
            z /= len;
        }
    }
    void scale(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }
};

struct Vector {
    Vector3 vec;
    sf::Color color;
    std::string text = "(" + float_to_frac(vec.x) + ", " + float_to_frac(vec.y) + ", " + float_to_frac(vec.z) + ")";
};

struct BasisVector {
    Vector3 vec;
    std::string text;
};

Vector3 scale(const Vector3& v, float scalar);

float dot_prod(const Vector3& u, const Vector3& v);

Vector3 proj_vec(const Vector3& u, const Vector3& v);

struct Plane {
    Vector3 n;
    sf::Color color;
    std::string text;

    Vector3 p1;
    Vector3 p2;
    Vector3 p3;
    Vector3 p4;

    Plane(const Vector3& n, sf::Color c) : color(c), n(n) {
        Vector3 a;
        float ax = std::abs(n.x), ay = std::abs(n.y), az = std::abs(n.z);
        if (ax <= ay && ax <= az)      a = {1, 0, 0};
        else if (ay <= az)             a = {0, 1, 0};
        else                           a = {0, 0, 1};
    
        Vector3 v1 = Vector3{n.y * a.z - n.z * a.y,
                             n.z * a.x - n.x * a.z,
                             n.x * a.y - n.y * a.x};
        
        Vector3 v2 = Vector3{n.y * v1.z - n.z * v1.y,
                             n.z * v1.x - n.x * v1.z,
                             n.x * v1.y - n.y * v1.x};
        
        p1 = scale(v1, 5/v1.length());
        p2 = scale(v2, 5/v2.length());
        p3 = scale(v1, -5/v1.length());
        p4 = scale(v2, -5/v2.length());

        text = nvec_to_string(n.x, n.y, n.z);
    }

    Plane(const Vector3& u1, const Vector3& u2, sf::Color c) : color(c) {
        Vector3 v1 = u1;

        Vector3 v2 = u2 - proj_vec(u2, u1);

        n = Vector3{v1.y * v2.z - v1.z * v2.y,
                    v1.z * v2.x - v1.x * v2.z,
                    v1.x * v2.y - v1.y * v2.x};

        p1 = scale(v1, 5/v1.length());
        p2 = scale(v2, 5/v2.length());
        p3 = scale(v1, -5/v1.length());
        p4 = scale(v2, -5/v2.length());

        text = nvec_to_string(n.x, n.y, n.z);
    }

};

Vector3 proj_plane(const Vector3& u, const Plane& v);

void update_buttons(std::vector<Vector>& vectors, std::vector<Button>& del_buttons, const sf::Font& font);

std::tuple<float, float, float> calc_dist(const Vector3 vec, Vector3 cam_vec, float cam_len_sqrd, Vector3 y_vec, Vector3 x_vec);

void handle_cmd_input(std::vector<Vector>& vectors, std::vector<Plane>& planes, const std::string cmd, std::string& e_msg);