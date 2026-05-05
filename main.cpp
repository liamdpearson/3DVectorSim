#include "main.hpp"

// g++ -c main.cpp ui.cpp -I"C:/SFML-2.5.1/include" -DSFML_STATIC
// g++ main.o ui.o -o main -L"C:/SFML-2.5.1/lib" -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lfreetype -lwinmm -lgdi32 -mwindows

std::string float_to_frac(float val) {
    if (std::abs(val) < 1e-6f) return "0";

    bool negative = val < 0;
    double x = std::abs((double)val);

    const double eps = 1e-6;
    const long long max_denom = 100000;

    long long h0 = 0, h1 = 1;
    long long k0 = 1, k1 = 0;
    double b = x;

    for (int i = 0; i < 64; i++) {
        long long a = (long long)std::floor(b);
        long long h2 = a * h1 + h0;
        long long k2 = a * k1 + k0;

        if (k2 > max_denom) break;

        h0 = h1; h1 = h2;
        k0 = k1; k1 = k2;

        if (std::abs(x - (double)h1 / (double)k1) < eps) break;

        double frac = b - (double)a;
        if (frac < 1e-12) break;
        b = 1.0 / frac;
    }

    std::string sign = negative ? "-" : "";
    if (k1 == 1) return sign + std::to_string(h1);
    return sign + std::to_string(h1) + "/" + std::to_string(k1);
}

std::string nvec_to_string(float x, float y, float z) {
    std::string s1 = "x", s2 = "y";
    if (y>0) s1 = "x+";
    if (z>0) s2 = "y+";
    return float_to_frac(x) + s1 + float_to_frac(y) + s2 + float_to_frac(z) + "z = 0";
}

float dot_prod(const Vector3& u, const Vector3& v) {
    return (u.x * v.x) + (u.y * v.y) + (u.z * v.z);
}

// returns projection of u onto v
Vector3 proj_vec(const Vector3& u, const Vector3& v) {
    float len_v_sqrd = dot_prod(v, v);
        if (len_v_sqrd == 0) return Vector3{0, 0, 0};

    float dot = dot_prod(u, v);
    float scalar = dot/len_v_sqrd;

    return Vector3{v.x * scalar, v.y * scalar, v.z * scalar};
}

Vector3 proj_plane(const Vector3& u, const Plane& p) {
    Vector3 proj_onto_n = proj_vec(u, p.n);
    return Vector3{u.x - proj_onto_n.x, u.y - proj_onto_n.y, u.z - proj_onto_n.z};
}

Vector3 scale(const Vector3& v, float scalar) {
    return Vector3{v.x * scalar, v.y * scalar, v.z * scalar};
}


void update_buttons(std::vector<Vector>& vectors, std::vector<Plane>& planes, std::vector<Button>& del_buttons, const sf::Font& font) {
    del_buttons = {};

    for (int i = 0; i < vectors.size(); i++) {
        del_buttons.push_back(Button(
        sf::Vector2f(10, 60 + 50*i), sf::Vector2f(30, 30), "X", font,
        [&vectors, i]() { vectors.erase(vectors.begin() + i); }));
    }

    for (int i = vectors.size(); i < vectors.size() + planes.size(); i++) {
        del_buttons.push_back(Button(
        sf::Vector2f(10, 110 + 50*i), sf::Vector2f(30, 30), "X", font,
        [&planes, i]() { planes.erase(planes.begin() + i); }));
    }
}

std::tuple<float, float, float> calc_dist(const Vector3 vec, Vector3 cam_vec, float cam_len_sqrd, Vector3 y_vec, Vector3 x_vec) {
    Vector3 proj_onto_cam = proj_vec(vec, cam_vec);
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

void handle_cmd_input(std::vector<Vector>& vectors, std::vector<Plane>& planes, const std::string cmd, std::string& e_msg) {
    std::vector<std::string> tokens = split_by_spaces(cmd);

    if (tokens[0] == "new") {
        if (tokens[1] == "vec") {

            if (vectors.size() >= 6) {
                e_msg = "Error: too many vectors";
            }
            else if (tokens.size() == 5) {
                try {
                    float x = std::stof(tokens[2]);
                    float y = std::stof(tokens[3]);
                    float z = std::stof(tokens[4]);
                    vectors.push_back(Vector{Vector3{x, y, z}, vector_colors[vectors.size() % 5]});
                    e_msg = "";
                } catch (std::invalid_argument e) {
                    e_msg = "Error: invalid vector value input";
                }
            }
            else {
                e_msg = "Error: invalid amount of arguments";
            }
        }

        else if (tokens[1] == "plane") {

            if (planes.size() >= 3) {
                e_msg = "Error: too many planes";
            }
            else if (tokens.size() == 3 && tokens[2].length() == 2) {
                if (tokens[2][0] == 'v') {
                    if (std::isdigit(tokens[2][1])) {
                        int n = tokens[2][1] - '0';
                        if (n < 1 || n > vectors.size()) {
                            e_msg = "Error: index out of range";
                            return;
                        }
                        planes.push_back(Plane{vectors[n-1].vec, plane_colors[planes.size() % 3]});
                        e_msg = "";
                    } else {
                        e_msg = "Error: invalid input";
                    }
                }
                else {
                    e_msg = "Error: invalid input";
                }
            }
            else if (tokens.size() == 4) {
                if (tokens[2][0] == 'v' && tokens[2].length() == 2 && tokens[3][0] == 'v' && tokens[3].length() == 2) {
                    if (std::isdigit(tokens[2][1]) && std::isdigit(tokens[3][1])) {
                        int n1 = tokens[2][1] - '0';
                        int n2 = tokens[3][1] - '0';
                        if (n1 < 1 || n1 > vectors.size() || n2 < 1 || n2 > vectors.size()) {
                            e_msg = "Error: index out of range";
                            return;
                        }
                        planes.push_back(Plane{vectors[n1-1].vec, vectors[n2-1].vec, plane_colors[planes.size() % 3]});
                        e_msg = "";
                    } else {
                        e_msg = "Error: invalid input";
                    }
                }
                else {
                    e_msg = "Error: invalid input";
                }
            }
            else {
                e_msg = "Error: invalid amount of arguments";
            }
        }
        else {
            e_msg = "Error: unrecognized object";
        }
    }
    else if (tokens[0] == "add" || tokens[0] == "sub") {
        if (tokens.size() == 3) {
            int a, b;
            if (tokens[1][0] == 'v' && tokens[1].length() == 2 && tokens[2][0] == 'v' && tokens[2].length() == 2) {
                if (std::isdigit(tokens[1][1]) && std::isdigit(tokens[2][1])) {
                        a = tokens[1][1] - '0';
                        if (a < 1 || a > vectors.size()) {
                            e_msg = "Error: index out of range";
                            return;
                        }
                        b = tokens[2][1] - '0';
                        if (b < 1 || b > vectors.size()) {
                            e_msg = "Error: index out of range";
                            return;
                        }
                } else {
                    e_msg = "Error: invalid input";
                    return;
                }
            } else {
                e_msg = "Error: invalid input";
                return;
            }

            Vector3 vec = Vector3{};

            if (tokens[0] == "add") {
                vectors[a-1].vec.x += vectors[b-1].vec.x;
                vectors[a-1].vec.y += vectors[b-1].vec.y;
                vectors[a-1].vec.z += vectors[b-1].vec.z;
            }
            if (tokens[0] == "sub") {
                vectors[a-1].vec.x -= vectors[b-1].vec.x;
                vectors[a-1].vec.y -= vectors[b-1].vec.y;
                vectors[a-1].vec.z -= vectors[b-1].vec.z;
            }
            vectors[a-1].text = "(" + float_to_frac(vectors[a-1].vec.x) + ", " + float_to_frac(vectors[a-1].vec.y) + ", " + float_to_frac(vectors[a-1].vec.z) + ")";
            e_msg = "";
        }
        else {
            e_msg = "Error: invalid amount of arguments";
        } 
    }
    else if (tokens[0] == "proj") {
        if (tokens.size() == 3) {
            int a, b;
            if (tokens[1][0] == 'v' && tokens[1].length() == 2 && tokens[2].length() == 2) {
                if (std::isdigit(tokens[1][1]) && std::isdigit(tokens[2][1])) {
                        a = tokens[1][1] - '0';
                        b = tokens[2][1] - '0';
                        if (a < 1 || a > vectors.size()) {
                            e_msg = "Error: index out of range";
                            return;
                        }
                        if (tokens[2][0] == 'v') {
                            if (b < 1 || b > vectors.size()) {
                                e_msg = "Error: index out of range";
                                return;
                            }
                            Vector3 vec = proj_vec(vectors[a-1].vec, vectors[b-1].vec);
                            vectors.push_back(Vector{vec, vector_colors[vectors.size() % 5]});
                        }
                        else if (tokens[2][0] == 'p') {
                            if (b < 1 || b > planes.size()) {
                                e_msg = "Error: index out of range";
                                return;
                            }
                            Vector3 vec = proj_plane(vectors[a-1].vec, planes[b-1]);
                            vectors.push_back(Vector{vec, vector_colors[vectors.size() % 5]});
                            e_msg = "";
                        }
                        else {
                            e_msg = "Error: invalid input";
                        }
                } else {
                    e_msg = "Error: invalid input";
                }
            } else {
                e_msg = "Error: invalid input";
            }
        }
        else {
            e_msg = "Error: invalid amount of arguments";
        }
    }
    else if (tokens[0] == "normalize") {
        if (tokens.size() == 2) {
            int a;
            if (tokens[1][0] == 'v' && tokens[1].length() == 2) {
                if (std::isdigit(tokens[1][1])) {
                        a = tokens[1][1] - '0';
                        if (a < 1 || a > vectors.size()) {
                            e_msg = "Error: index out of range";
                            return;
                        }
                } else {
                    e_msg = "Error: invalid input";
                    return;
                }
            } else {
                e_msg = "Error: invalid input";
                return;
            }
            if (vectors[a-1].vec.length() == 0) {
                e_msg = "Error: cannot normalize zero vector";
                return;
            }
            if (vectors[a-1].vec.length() >= .999999f && vectors[a-1].vec.length() <= 1.000001f) {
                e_msg = "Error: already normalized";
                return;
            }

            float x = vectors[a-1].vec.x;
            float y = vectors[a-1].vec.y;
            float z = vectors[a-1].vec.z;

            std::string x_str = float_to_frac(x);
            std::string y_str = float_to_frac(y);
            std::string z_str = float_to_frac(z);

            std::string len_sqrd = float_to_frac(x*x + y*y + z*z);
            
            vectors[a-1].vec.normalize();
            vectors[a-1].text = "(" + x_str + "/sqrt(" + len_sqrd + "), " + y_str + "/sqrt(" + len_sqrd + "), " + z_str + "/sqrt(" + len_sqrd + "))";
            e_msg = "";
        }
        else {
            e_msg = "Error: invalid amount of arguments";
        }
    }
    else if (tokens[0] == "scale") {
        if (tokens.size() == 3) {
            int a;
            float scalar;
            if (tokens[1][0] == 'v' && tokens[1].length() == 2) {
                if (std::isdigit(tokens[1][1])) {
                        a = tokens[1][1] - '0';
                        if (a < 1 || a > vectors.size()) {
                            e_msg = "Error: index out of range";
                            return;
                        }
                } else {
                    e_msg = "Error: invalid input";
                    return;
                }
            } else {
                e_msg = "Error: invalid input";
                return;
            }
            if (std::isdigit(tokens[2][0])) {
                scalar = std::stof(tokens[2]);
            } else {
                e_msg = "Error: invalid scalar value";
                return;
            }
            vectors[a-1].vec.scale(scalar);
            vectors[a-1].text = "(" + float_to_frac(vectors[a-1].vec.x) + ", " + float_to_frac(vectors[a-1].vec.y) + ", " + float_to_frac(vectors[a-1].vec.z) + ")";
            e_msg = "";
        }
        else {
            e_msg = "Error: invalid amount of arguments";
        }
    }
    else if (tokens[0] == "clear" && tokens.size() == 1) {
        vectors = {};
        planes = {};
    }
    else if (tokens[0] == "help" && tokens.size() == 1) {
        e_msg = "Commands:\nnew vec x y z\nnew plane vN\nnew plane vN vM\nadd vN vM\nsub vN vM\nproj vN vM\nproj vN pN\nclear\nhelp";
    }
    else {
        e_msg = "Error: unrecognized command";
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

    std::vector<Vector> vectors = {};

    std::vector<Plane> planes {};

    std::vector<Button> del_buttons = {};
    update_buttons(vectors, planes, del_buttons, font);
    

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
                    if (!cur_command.empty()) {
                        handle_cmd_input(vectors, planes, cur_command, e_msg);
                        update_buttons(vectors, planes, del_buttons, font);
                        cur_command = "";
                    }
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
                    update_buttons(vectors, planes, del_buttons, font);
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
        Vector3 y_vec = up_vec - proj_vec(up_vec, cam_vec);
        Vector3 x_vec = Vector3{
            cam_vec.y * y_vec.z - cam_vec.z * y_vec.y,
            cam_vec.z * y_vec.x - cam_vec.x * y_vec.z,
            cam_vec.x * y_vec.y - cam_vec.y * y_vec.x
        };

        // start drawing
        window.clear();

        sf::Text plane_header("Planes:", font, 20);
        plane_header.setPosition(sf::Vector2f(15, 65+50*(vectors.size())));
        window.draw(plane_header);

        for (int i = 0; i < planes.size(); i++) {
            Plane plane = planes[i];

            std::tuple<float, float, float> p1 = calc_dist(plane.p1, cam_vec, cam_len_sqrd, y_vec, x_vec);
            float p1x = std::get<0>(p1), p1y = std::get<1>(p1), p1z = std::get<2>(p1);
            std::tuple<float, float, float> p2 = calc_dist(plane.p2, cam_vec, cam_len_sqrd, y_vec, x_vec);
            float p2x = std::get<0>(p2), p2y = std::get<1>(p2), p2z = std::get<2>(p2);
            std::tuple<float, float, float> p3 = calc_dist(plane.p3, cam_vec, cam_len_sqrd, y_vec, x_vec);
            float p3x = std::get<0>(p3), p3y = std::get<1>(p3), p3z = std::get<2>(p3);
            std::tuple<float, float, float> p4 = calc_dist(plane.p4, cam_vec, cam_len_sqrd, y_vec, x_vec);
            float p4x = std::get<0>(p4), p4y = std::get<1>(p4), p4z = std::get<2>(p4);

            sf::ConvexShape quad;
            quad.setPointCount(4);

            quad.setPoint(0, sf::Vector2f(3*SCREEN_HEIGHT/5 + (p1x)/(p1z)*500, SCREEN_HEIGHT/2 - (p1y)/(p1z)*500));
            quad.setPoint(1, sf::Vector2f(3*SCREEN_HEIGHT/5 + (p2x)/(p2z)*500, SCREEN_HEIGHT/2 - (p2y)/(p2z)*500));
            quad.setPoint(2, sf::Vector2f(3*SCREEN_HEIGHT/5 + (p3x)/(p3z)*500, SCREEN_HEIGHT/2 - (p3y)/(p3z)*500));
            quad.setPoint(3, sf::Vector2f(3*SCREEN_HEIGHT/5 + (p4x)/(p4z)*500, SCREEN_HEIGHT/2 - (p4y)/(p4z)*500));

            sf::Color fill = plane.color;
            fill.a = 99;
            quad.setFillColor(fill);

            window.draw(quad);

            sf::Text label(std::to_string(i+1) + ". " + plane.text, font, 14);
            label.setFillColor(plane.color);
            label.setPosition(sf::Vector2f(50, 115+50*(vectors.size() + i)));
            window.draw(label);
        }

        for (BasisVector& bv : basis_vectors) {

            Vector3 vec = bv.vec;

            std::tuple<float, float, float> tup = calc_dist(vec, cam_vec, cam_len_sqrd, y_vec, x_vec);
            float x_dist = std::get<0>(tup), y_dist = std::get<1>(tup), z_dist = std::get<2>(tup);

            sf::Vector2f tip(3*SCREEN_HEIGHT/5 + (x_dist)/(z_dist)*500, SCREEN_HEIGHT/2 - (y_dist)/(z_dist)*500);

            sf::VertexArray line(sf::Lines, 2);
            line[0].position = sf::Vector2f(3*SCREEN_HEIGHT/5, SCREEN_HEIGHT/2);
            line[1].position = tip;

            window.draw(line);

            sf::Text label(bv.text, font, 14);
            label.setPosition(tip);
            window.draw(label);
        }

        sf::Text vec_header("Vectors:", font, 20);
        vec_header.setPosition(sf::Vector2f(15, 15));
        window.draw(vec_header);

        for (int i = 0; i < vectors.size(); i++) {
            Vector vector = vectors[i];
            Vector3 vec = vector.vec;

            std::tuple<float, float, float> tup = calc_dist(vec, cam_vec, cam_len_sqrd, y_vec, x_vec);
            float x_dist = std::get<0>(tup), y_dist = std::get<1>(tup), z_dist = std::get<2>(tup);

            sf::Vector2f tip(3*SCREEN_HEIGHT/5 + (x_dist)/(z_dist)*500, SCREEN_HEIGHT/2 - (y_dist)/(z_dist)*500);
            
            sf::VertexArray line(sf::Lines, 2);
            line[0].position = sf::Vector2f(3*SCREEN_HEIGHT/5, SCREEN_HEIGHT/2);
            line[1].position = tip;

            line[0].color = vector.color;
            line[1].color = vector.color;

            window.draw(line);

            sf::Text label(std::to_string(i+1) + ". " + vector.text, font, 14);
            label.setFillColor(vector.color);
            label.setPosition(tip);
            window.draw(label);
            label.setPosition(sf::Vector2f(50, 65+50*i));
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