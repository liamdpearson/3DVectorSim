#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <string>
#include "ui.hpp"

// g++ -c main.cpp ui.cpp -I"C:/SFML-2.5.1/include" -DSFML_STATIC
// g++ main.o ui.o -o main -L"C:/SFML-2.5.1/lib" -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lfreetype -lwinmm -lgdi32 -mwindows

const int SCREEN_WIDTH = 1000, SCREEN_HEIGHT = 1000;

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

struct ColoredVector {
    Vector3 vec;
    sf::Color color;
};

struct TextVector {
    Vector3 vec;
    std::string str;
};

float dot_prod(const Vector3& u, const Vector3& v) {
    return (u.x * v.x) + (u.y * v.y) + (u.z * v.z);
}

// returns projection of u onto v
Vector3 proj(const Vector3& u, const Vector3& v) {
    float len_v_sqrd = dot_prod(v, v);
        if (len_v_sqrd == 0) return Vector3{0, 0, 0};

    float dot = dot_prod(u, v);
    float scalar = dot/len_v_sqrd;

    return Vector3{v.x * scalar, v.y * scalar, v.z * scalar};
}

int main() {
    
    sf::RenderWindow window(sf::VideoMode({SCREEN_WIDTH, SCREEN_HEIGHT}), "3DVectorSim");

    sf::Font font;
    font.loadFromFile("Arial.ttf");

    float cam_yaw = 45.f;
    float cam_pitch = 45.f;
    float cam_dist = 8.f;
    Vector3 up_vec = Vector3{0,0,1};

    std::vector<TextVector> basis_vectors;
    for (int i = 1; i <= 4; i++) {
        std::string pos = std::to_string(i);
        std::string neg = "-" + pos;
        float f = (float)i;
        basis_vectors.push_back(TextVector{Vector3{ f, 0, 0}, pos});
        basis_vectors.push_back(TextVector{Vector3{-f, 0, 0}, neg});
        basis_vectors.push_back(TextVector{Vector3{0,  f, 0}, pos});
        basis_vectors.push_back(TextVector{Vector3{0, -f, 0}, neg});
        basis_vectors.push_back(TextVector{Vector3{0, 0,  f}, pos});
        basis_vectors.push_back(TextVector{Vector3{0, 0, -f}, neg});
    }

    basis_vectors.push_back(TextVector{Vector3{5, 0, 0}, "5\tX"});
    basis_vectors.push_back(TextVector{Vector3{0, 5, 0}, "5\tY"});
    basis_vectors.push_back(TextVector{Vector3{0, 0, 5}, "5\tZ"});
    basis_vectors.push_back(TextVector{Vector3{-5, 0, 0}, "-5\tX"});
    basis_vectors.push_back(TextVector{Vector3{0, -5, 0}, "-5\tY"});
    basis_vectors.push_back(TextVector{Vector3{0, 0, -5}, "-5\tZ"});

    std::vector<ColoredVector> vectors = {
        ColoredVector{Vector3{-2, 3, -4}, sf::Color::Blue}
    };

    Button addVecBtn(
        sf::Vector2f(10, 10), sf::Vector2f(140, 30), "Add (1,1,1)", font,
        [&vectors]() { vectors.push_back(ColoredVector{Vector3{1, 1, 1}, sf::Color::Green}); }
    );

    sf::Vector2i prev_mouse_pos = sf::Mouse::getPosition(window);

    while (window.isOpen())
    {
        sf::Event event;


        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            else if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    float delta = event.mouseWheelScroll.delta * 0.1;

                     cam_dist -= delta;

                     if (cam_dist < 0.1) cam_dist = 0.1;
                }
            }

            addVecBtn.handleEvent(event);
        }

        sf::Vector2i cur_mouse_pos = sf::Mouse::getPosition(window);
        sf::Vector2i delta = cur_mouse_pos - prev_mouse_pos;
        prev_mouse_pos = cur_mouse_pos;

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
            !addVecBtn.contains(sf::Vector2f(cur_mouse_pos))) {
            cam_yaw += delta.x;
            cam_pitch += delta.y;
            if (cam_pitch > 89.99) cam_pitch = 89.99;
            if (cam_pitch < -89.99) cam_pitch = -89.99;
        }
    
        float z = cam_dist * sin(cam_pitch*M_PI/180);

        float flat_dist = cam_dist * cos(cam_pitch*M_PI/180);
        float x = flat_dist * cos(cam_yaw*M_PI/180);
        float y = flat_dist * sin(cam_yaw*M_PI/180);
        

        Vector3 cam_vec = Vector3{x, y, z};
        float cam_len_sqrd = dot_prod(cam_vec, cam_vec);
        Vector3 y_vec = up_vec - proj(up_vec, cam_vec);
        Vector3 x_vec = Vector3{
            cam_vec.y * y_vec.z - cam_vec.z * y_vec.y,
            cam_vec.z * y_vec.x - cam_vec.x * y_vec.z,
            cam_vec.x * y_vec.y - cam_vec.y * y_vec.x
        };

        // start drawing
        window.clear();

        for (TextVector& tv : basis_vectors) {

            Vector3 vec = tv.vec;

            Vector3 proj_onto_cam = proj(vec, cam_vec);
            Vector3 perp_onto_cam = vec - proj_onto_cam;

            // scalar component of vec along cam_vec direction
            float along = dot_prod(vec, cam_vec) / cam_len_sqrd;

            float z_dist;
            if (along >= 1.0f) {
                z_dist = 0.001f;  // behind camera, push to near-zero
            } else {
                z_dist = (1.0f - along) * cam_vec.length();
            }

            float x_dist = dot_prod(perp_onto_cam, x_vec) / x_vec.length();
            float y_dist = dot_prod(perp_onto_cam, y_vec) / y_vec.length();

            sf::Vector2f tip(3*SCREEN_HEIGHT/5 + (x_dist)/(z_dist)*500, 2*SCREEN_HEIGHT/5 - (y_dist)/(z_dist)*500);

            sf::VertexArray line(sf::Lines, 2);
            line[0].position = sf::Vector2f(3*SCREEN_HEIGHT/5, 2*SCREEN_HEIGHT/5);
            line[1].position = tip;

            window.draw(line);

            sf::Text label(tv.str, font, 14);
            label.setPosition(tip);
            window.draw(label);
        }

        for (ColoredVector& vector : vectors) {

            Vector3 vec = vector.vec;

            Vector3 proj_onto_cam = proj(vec, cam_vec);
            Vector3 perp_onto_cam = vec - proj_onto_cam;

            // scalar component of vec along cam_vec direction
            float along = dot_prod(vec, cam_vec) / cam_len_sqrd;

            float z_dist;
            if (along >= 1.0f) {
                z_dist = 0.001f;  // behind camera, push to near-zero
            } else {
                z_dist = (1.0f - along) * cam_vec.length();
            }

            float x_dist = dot_prod(perp_onto_cam, x_vec) / x_vec.length();
            float y_dist = dot_prod(perp_onto_cam, y_vec) / y_vec.length();

            sf::Vector2f tip(3*SCREEN_HEIGHT/5 + (x_dist)/(z_dist)*500, 2*SCREEN_HEIGHT/5 - (y_dist)/(z_dist)*500);
            
            sf::VertexArray line(sf::Lines, 2);
            line[0].position = sf::Vector2f(3*SCREEN_HEIGHT/5, 2*SCREEN_HEIGHT/5);
            line[1].position = tip;

            line[0].color = vector.color;
            line[1].color = vector.color;

            window.draw(line);
        }   

        addVecBtn.draw(window);
        window.display();
        // stop drawing
    }

    return 0;
}