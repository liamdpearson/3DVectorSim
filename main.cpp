#include "main.hpp"

// g++ -c main.cpp ui.cpp -I"C:/SFML-2.5.1/include" -DSFML_STATIC
// g++ main.o ui.o -o main -L"C:/SFML-2.5.1/lib" -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lfreetype -lwinmm -lgdi32 -mwindows

std::string vecToString(float x, float y, float z) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "(" << x << ", " << y << ", " << z << ")";
    return oss.str();
}


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


void update_buttons(std::vector<Vector>& vectors, std::vector<Button>& del_buttons, const sf::Font& font) {
    del_buttons = {};

    for (int i = 0; i < vectors.size(); i++) {
        del_buttons.push_back(Button(
        sf::Vector2f(10, 10 + 50*i), sf::Vector2f(30, 30), "X", font,
        [&vectors, i]() { vectors.erase(vectors.begin() + i); }));
    }
}

std::tuple<float, float, float> calc_dist(const Vector3 vec, Vector3 cam_vec, float cam_len_sqrd, Vector3 y_vec, Vector3 x_vec) {
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

    return {x_dist, y_dist, z_dist};
}

std::vector<std::string> split_by_spaces(std::string s) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string token;

    while (ss >> token)
    {
        tokens.push_back(token);
    }

    return tokens;
}

void handle_cmd_input(std::vector<Vector>& vectors, const std::string cmd, std::string& e_msg) {
    std::vector<std::string> tokens = split_by_spaces(cmd);

    if (tokens[0] == "new") {
        if (tokens[1] == "vec" && tokens.size() == 5) {
            try {
                float x = std::stof(tokens[2]);
                float y = std::stof(tokens[3]);
                float z = std::stof(tokens[4]);
                vectors.push_back(Vector{Vector3{x, y, z}, colors[vectors.size() % 6]});
                e_msg = "";
            } catch (std::invalid_argument e) {
                e_msg = "Error: invalid vector value input";
                return;
            }
        }
    }
    else if (tokens[0] == "add" || tokens[0] == "sub") {
        if (tokens.size() == 3) {
            try {
                int a = std::stoi(tokens[1]);
                int b = std::stoi(tokens[2]);

                if (a < 1 || b < 1 || a > vectors.size() || b > vectors.size()) {
                    e_msg = "Error: index out of range";
                    return;
                }
                Vector3 vec = Vector3{};

                if (tokens[0] == "add") {
                    vec.x = vectors[a-1].vec.x + vectors[b-1].vec.x;
                    vec.y = vectors[a-1].vec.y + vectors[b-1].vec.y;
                    vec.z = vectors[a-1].vec.z + vectors[b-1].vec.z;
                }
                if (tokens[0] == "sub") {
                    vec.x = vectors[a-1].vec.x - vectors[b-1].vec.x;
                    vec.y = vectors[a-1].vec.y - vectors[b-1].vec.y;
                    vec.z = vectors[a-1].vec.z - vectors[b-1].vec.z;
                }

                vectors.push_back(Vector{vec, colors[vectors.size() % 6]});
            } catch (std::invalid_argument e) {
                e_msg = "Error: invalid index input";
                return;
            }
        }   
    }
    else if (tokens[0] == "proj") {
        if (tokens.size() == 3) {
            try {
                int a = std::stoi(tokens[1]);
                int b = std::stoi(tokens[2]);

                if (a < 1 || b < 1 || a > vectors.size() || b > vectors.size()) {
                    e_msg = "Error: index out of range";
                    return;
                }

                Vector3 vec = proj(vectors[a-1].vec, vectors[b-1].vec);
                vectors.push_back(Vector{vec, colors[vectors.size() % 6]});
            } catch (std::invalid_argument e) {
                e_msg = "Error: invalid index input";
                return;
            }
        }
    }
}

int main() {
    
    sf::RenderWindow window(sf::VideoMode({SCREEN_WIDTH, SCREEN_HEIGHT}), "3DVectorSim");

    sf::Font font;
    font.loadFromFile("Arial.ttf");
    std::string cur_command = "";
    std::string e_msg = "";

    float cam_yaw = 45.f;
    float cam_pitch = 45.f;
    float cam_dist = 8.f;
    Vector3 up_vec = Vector3{0,0,1};

    std::vector<BasisVector> basis_vectors;
    for (int i = 1; i <= 4; i++) {
        std::string pos = std::to_string(i);
        std::string neg = "-" + pos;
        float f = (float)i;
        basis_vectors.push_back(BasisVector{Vector3{ f, 0, 0}, pos});
        basis_vectors.push_back(BasisVector{Vector3{-f, 0, 0}, neg});
        basis_vectors.push_back(BasisVector{Vector3{0,  f, 0}, pos});
        basis_vectors.push_back(BasisVector{Vector3{0, -f, 0}, neg});
        basis_vectors.push_back(BasisVector{Vector3{0, 0,  f}, pos});
        basis_vectors.push_back(BasisVector{Vector3{0, 0, -f}, neg});
    }

    basis_vectors.push_back(BasisVector{Vector3{5, 0, 0}, "5\tX"});
    basis_vectors.push_back(BasisVector{Vector3{0, 5, 0}, "5\tY"});
    basis_vectors.push_back(BasisVector{Vector3{0, 0, 5}, "5\tZ"});
    basis_vectors.push_back(BasisVector{Vector3{-5, 0, 0}, "-5\tX"});
    basis_vectors.push_back(BasisVector{Vector3{0, -5, 0}, "-5\tY"});
    basis_vectors.push_back(BasisVector{Vector3{0, 0, -5}, "-5\tZ"});

    std::vector<Vector> vectors = {
        Vector{Vector3{-2, 3, -4}, sf::Color::Red},
        Vector{Vector3{1, 1, 1}, sf::Color::Green}
    };

    std::vector<Button> del_buttons = {};
    update_buttons(vectors, del_buttons, font);
    

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

            else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\r') {
                    handle_cmd_input(vectors, cur_command, e_msg);
                    update_buttons(vectors, del_buttons, font);
                    cur_command = "";
                } 
                else if (event.text.unicode == '\b') {
                    if (!cur_command.empty()) cur_command.pop_back();
                } 
                else if (event.text.unicode < 128) {
                    cur_command += static_cast<char>(event.text.unicode);
                }
            }

            for (Button& button: del_buttons) {
                if (button.handleEvent(event)) {
                    update_buttons(vectors, del_buttons, font);
                }
            }   
        }

        sf::Vector2i cur_mouse_pos = sf::Mouse::getPosition(window);
        sf::Vector2i delta = cur_mouse_pos - prev_mouse_pos;
        prev_mouse_pos = cur_mouse_pos;

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
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

        for (BasisVector& bv : basis_vectors) {

            Vector3 vec = bv.vec;

            std::tuple<float, float, float> tup = calc_dist(vec, cam_vec, cam_len_sqrd, y_vec, x_vec);
            float x_dist = std::get<0>(tup), y_dist = std::get<1>(tup), z_dist = std::get<2>(tup);

            sf::Vector2f tip(3*SCREEN_HEIGHT/5 + (x_dist)/(z_dist)*500, 2*SCREEN_HEIGHT/5 - (y_dist)/(z_dist)*500);

            sf::VertexArray line(sf::Lines, 2);
            line[0].position = sf::Vector2f(3*SCREEN_HEIGHT/5, 2*SCREEN_HEIGHT/5);
            line[1].position = tip;

            window.draw(line);

            sf::Text label(bv.text, font, 14);
            label.setPosition(tip);
            window.draw(label);
        }

        for (int i = 0; i < vectors.size(); i++) {
            Vector vector = vectors[i];
            Vector3 vec = vector.vec;

            std::tuple<float, float, float> tup = calc_dist(vec, cam_vec, cam_len_sqrd, y_vec, x_vec);
            float x_dist = std::get<0>(tup), y_dist = std::get<1>(tup), z_dist = std::get<2>(tup);

            sf::Vector2f tip(3*SCREEN_HEIGHT/5 + (x_dist)/(z_dist)*500, 2*SCREEN_HEIGHT/5 - (y_dist)/(z_dist)*500);
            
            sf::VertexArray line(sf::Lines, 2);
            line[0].position = sf::Vector2f(3*SCREEN_HEIGHT/5, 2*SCREEN_HEIGHT/5);
            line[1].position = tip;

            line[0].color = vector.color;
            line[1].color = vector.color;

            window.draw(line);

            sf::Text label(std::to_string(i+1) + ". " + vector.text, font, 14);
            label.setFillColor(vector.color);
            label.setPosition(tip);
            window.draw(label);
            label.setPosition(sf::Vector2f(50, 15+50*i));
            window.draw(label);
        }   

        for (Button& button: del_buttons) {
                button.draw(window);
        }

        sf::Text cmd("Command: [" + cur_command + ']', font, 20);
        cmd.setPosition(sf::Vector2f(30, SCREEN_HEIGHT-60));
        window.draw(cmd);

        sf::Text error(e_msg, font, 20);
        error.setFillColor(sf::Color::Red);
        error.setPosition(sf::Vector2f(30, SCREEN_HEIGHT-120));
        window.draw(error);

        window.display();
        // stop drawing
    }

    return 0;
}