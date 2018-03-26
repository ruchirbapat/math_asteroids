// Headers
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <list>
#include <unordered_map>
#include <cctype>
#include <stdexcept>
#include <sstream>
#include <thread>
#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>

using std::vector;
using std::cout;
using std::printf;

#define array_size(a) (sizeof(a)/sizeof(a[0]))

#define lerp(a, b, t) ((a * (1-t)) + (b*t))

// Helper function
template<typename T>
inline auto rand_between (T min, T max) { return std::rand() % (max + 1) + min; };

#define DEG2RAD (M_PI/180)
#define RAD2DEG (180/M_PI)

// Game constants
const int WIN_WIDTH = 600;
const int WIN_HEIGHT = 600;
bool onMenu = false;

// Forward declare MovingEntity class for the renderables
class MovingEntity;

//Update this shit to use C++ smart pointers, the fastest one preferably
static std::list<MovingEntity*> renderables; 

static sf::Font font_face;
class MovingEntity : public sf::Transformable, public sf::Drawable {
public:
	explicit MovingEntity(sf::Vector2f initial_vel, sf::Vector2f initial_max_vel, const float accel, MovingEntity* child, std::string _name) :
		velocity(initial_vel), max_velocity(initial_max_vel), acceleration(accel),
		/*text(sf::Text()),*/ name(_name) {

		::renderables.push_back(child);
		/*
		text.setFont(font_face);
		text.setCharacterSize(16);
		text.setFillColor(sf::Color::White);
		text.setStyle(sf::Text::Regular);
		*/
	
	};
	sf::ConvexShape mesh;
	sf::Vector2f velocity;
	const sf::Vector2f max_velocity;
//	const sf::Vector2f textOffset;
	const float acceleration;
//	sf::Text text;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		std::cout << "MovingEntity::draw() called on " << name << std::endl;
		states.transform *= getTransform();
		target.draw(mesh, states);
	}

	virtual void Tick(float deltaTime) {
		// Velocity clamping
		std::cout << "MovingEntity::Tick() called on " << name << std::endl;
		if(velocity.x > max_velocity.x) {
			velocity.x = max_velocity.x;
		}
		if(velocity.y > max_velocity.y) {
			velocity.y = max_velocity.y; 
		}
		
		// Move object
		move(velocity * deltaTime);

		// Bounds checking
		if(getPosition().y <= 0) {
			setPosition(getPosition().x, WIN_HEIGHT);
		} else if (getPosition().y >= WIN_HEIGHT) {
			setPosition(getPosition().x, 0);
		}
		if(getPosition().x <= 0) {
			setPosition(WIN_WIDTH, getPosition().y);
		} else if (getPosition().x >= WIN_WIDTH) {
			setPosition(0, getPosition().y);
		}
	}
 
	std::string name;
};

#if 0
class MovingEntitywText : public MovingEntity {
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		target.draw(my_text);
	}
private:
	sf::Text my_text;
};
#endif 

class Asteroid;
static std::unordered_map<int, Asteroid*> asteroid_lut;

//const static std::string operands = "+-*";
class Asteroid : public MovingEntity {
public:
	Asteroid() : 
		MovingEntity(sf::Vector2f(0, 0), sf::Vector2f(50, 50), 0.0f, this, "Asteroid"), // Base constructor first
		answer(0), num_points( rand_between<short>(5, max_points) ), text(sf::Text())
	{
		// Muh procedural asteroid generation
		// TODO: use line rendering rather than a small thickness
		//num_points = (std::rand() % max_points) + 5;
		float perc_diff = 5/100;
		unsigned short gen_radius = (std::rand() % 7) + 2;
		float min_rad = gen_radius * (1 + perc_diff), max_rad = gen_radius * (1 - perc_diff);;
		mesh.setPointCount(num_points);
		for(unsigned short i = 0, cur_angle = 0; i < num_points; i++, cur_angle += (360/num_points)) {
//			unsigned short rand_radius = (std::rand() % 10) + 2;
			unsigned short rand_radius = static_cast<unsigned short>((std::rand() % (short)max_rad) + (short)min_rad);
			mesh.setPoint(i, sf::Vector2f(rand_radius * std::sin(cur_angle), rand_radius * std::cos(cur_angle)));
		}
		mesh.setFillColor(sf::Color(0, 0, 0, 0));
		mesh.setOutlineColor(sf::Color::White);
		mesh.setOutlineThickness(0.1f);

		// Numerals, change to it to have a private member of min and max values
		short numerals[2] = { static_cast<short>((rand_between(1, 15))), static_cast<short>(rand_between(1, 15)) }; 
		auto randop = rand_between<short>(0, operands.length() - 1); 
		unsigned char operandType( operands.at(randop) );
		answer = [=]() -> int {
			switch(operandType) {
				case '+':
					return numerals[0] + numerals[1];
					break;
				case '-':
					return numerals[0] - numerals[1];
					break;
				case '*':
					return numerals[0] * numerals[1];
					break;
				default:
					throw std::runtime_error("Unknown operand chosen!");
			}
		}();

		asteroid_lut[answer] = this;
		std::stringstream ss;
		ss << numerals[0] << ' ' << operandType << ' ' << numerals[1];
		text.setString(ss.str());
		std::cout << "text: " << text.getString().toAnsiString() << std::endl;
	}

	virtual void Tick(float deltaTime) override {
		MovingEntity::Tick(deltaTime);
		std::cout << "Asteroid::Tick() called" << std::endl;
		text.setPosition(getPosition().x + (mesh.getGlobalBounds().width / 2), getPosition().y + (mesh.getGlobalBounds().height) + textOffset.y);
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
//		MovingEntity::draw(target, states);
		states.transform *= getTransform();
		target.draw(mesh, states);
		std::cout << "Asteroid::draw() called: " << text.getString().toAnsiString() << " on target: " << &target << std::endl;
		target.draw(text, states);
	}

	~Asteroid() {
		asteroid_lut[answer] = nullptr;
	}
private:
	int answer;
	static std::string operands;//  = "+-*";
	const unsigned short max_points = 6;
	unsigned short num_points;
	sf::Text text;
	const sf::Vector2f textOffset = sf::Vector2f(0, 20);
};
std::string Asteroid::operands = "+-*";

class Spaceship : public MovingEntity {
public:
	// Spaceship constructor
	Spaceship() : 
		MovingEntity(sf::Vector2f(0, 0), sf::Vector2f(75, 75), 3.0f, this, "Spaceship"), // Base constructor members
		rotation_factor(6.0f) { // This class's members
		mesh.setPointCount(4);
		mesh.setPoint(0, sf::Vector2f(1, 0));
		mesh.setPoint(1, sf::Vector2f(2, 2));
		mesh.setPoint(2, sf::Vector2f(1, 1.5)); 
		mesh.setPoint(3, sf::Vector2f(0, 2));
		mesh.setFillColor(sf::Color::Black);
		mesh.setOutlineColor(sf::Color::White);
		mesh.setOutlineThickness(0.1f);
		mesh.setPosition(0, 0);
	}
	~Spaceship() {};
	static void reset_position();
	const float rotation_factor;
};

enum class button_state {
	hover,
	clicked,
	normal
};

class Button : public sf::Drawable, public sf::Transformable {
public:

	// Default constructor
	Button(sf::Vector2f pos, sf::Vector2f scale, sf::Text show_text) : btn_text(show_text) {
		this->setPosition(pos);
		this->setScale(scale);
	}

	// Copy constructor
	Button(Button& btn) : btn_text(btn.btn_text) {
		this->setPosition(btn.getPosition());
		this->setScale(btn.getScale());
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
	}

private:
	sf::Text btn_text;
};

// Linker entry-point
int main()
{
	//Seed random number generator
	std::srand(std::time(nullptr));

	// Init main window 
	sf::RenderWindow window;
 	window.create(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "Math Asteroids", sf::Style::Close,
		[]() -> sf::ContextSettings {
			sf::ContextSettings s;
			s.antialiasingLevel = 4;
			return s;
		}()
	);
	
	std::cout << "Window: " << &window << std::endl;

	// Enable Vsync
	window.setVerticalSyncEnabled(true);
		
	font_face.loadFromFile("NimbusRegular.otf");
	
	sf::Text menuTitle;
	menuTitle.setFont(font_face);
	menuTitle.setString("MATH ASTEROIDS");
	menuTitle.setCharacterSize(60);
	menuTitle.setFillColor(sf::Color::White);
	menuTitle.setStyle(sf::Text::Bold);
	menuTitle.setOrigin(menuTitle.getGlobalBounds().width / 2, menuTitle.getGlobalBounds().height / 2);
	menuTitle.setPosition(WIN_WIDTH / 2, (WIN_HEIGHT / 2) - menuTitle.getGlobalBounds().height);
	
#if 0
	sf::Text text;
	text.setFont(font_face);
	text.setString("Math Asteroids");
	text.setCharacterSize(36);
	text.setFillColor(sf::Color::White);
	text.setStyle(sf::Text::Bold);
#endif

	Spaceship ship;
	ship.setPosition(WIN_WIDTH/2, WIN_HEIGHT/2);
	ship.scale(10, 10);
	ship.setOrigin(0.5 * ship.mesh.getGlobalBounds().width / 2, 0.5 * ship.mesh.getGlobalBounds().height / 2);

	std::string typed_text = std::string();
	short text_sign = 1;

	for(int i = 0; i < 2; i++) {
		Asteroid* a = new Asteroid();
		auto w = static_cast<unsigned int>(a->mesh.getGlobalBounds().width);
		auto h = static_cast<unsigned int>(a->mesh.getGlobalBounds().height);
		auto x = std::rand() % (WIN_WIDTH - w) + w;
		auto y = std::rand() % (WIN_HEIGHT - h) + h;
		a->setPosition(x, y);
		a->scale(5, 5);
		a->velocity = sf::Vector2f(-1 * 50, -1 * 50);
	}

	// Clock has started!
	sf::Time lastTime = sf::microseconds(0);
	sf::Clock clock;
	
	// Main event loop
	while(window.isOpen()) {
		window.clear();
		sf::Time currentTime = clock.getElapsedTime(), deltaTimeMS = currentTime - lastTime; 
		lastTime = currentTime;

		// Capture events
		sf::Event e;
		float deltaTime = deltaTimeMS.asSeconds();
		if(!onMenu) {
			while(window.pollEvent(e)) {
				if(e.type == sf::Event::EventType::Closed) {
					window.close(); break;

				} if(e.type == sf::Event::EventType::TextEntered) {
					char c = static_cast<char>(e.text.unicode);
					if (isdigit(c)) {
						typed_text += c;
					} else if (c == '-') {
						text_sign *= -1;
					}
					std::cout << "Char entered: " << e.text.unicode << std::endl;
				} else if(e.type == sf::Event::EventType::KeyReleased) {
					if (e.key.code == sf::Keyboard::Return && !typed_text.empty()){
						std::cout << "Text: " << typed_text << std::endl;
						// handle the text entered
						int sum = text_sign * std::atoi(typed_text.c_str());
						
						// Reset text string and text sign (1 or -1)
						typed_text = std::string();
						text_sign = 1;
						
						// Destroy the appropriate asteroid
						if(asteroid_lut.find(sum) != asteroid_lut.end()) {
							auto asteroid = asteroid_lut[sum];
							auto angle = atan2((asteroid->getPosition().y + asteroid->velocity.y) - ship.getPosition().y, 
											   (asteroid->getPosition().x + asteroid->velocity.x)- ship.getPosition().x) * RAD2DEG;
						
							ship.setRotation(angle + 90);
							sf::VertexArray bullet_ray(sf::Lines, 2);
							bullet_ray[0].position = ship.mesh.getPoint(0);
							bullet_ray[1].position = asteroid->getPosition() + asteroid->velocity;
							bullet_ray[0].color = sf::Color::Red;
							bullet_ray[1].color = sf::Color::Red;
							
							for(float t = 0; t <= 1; t += 0.001) {
								
							}

							renderables.remove(asteroid_lut[sum]);
							delete asteroid_lut[sum];
						}
					
						/*
						for(float t = 0; t <= 1; t += 0.001) {
							ship.rota
							}
						*/

						// shoot at meme with sum on it
						std::cout << "Shot at " << sum << std::endl; // This current throws an error!
					}
				}
				// End of checking the event type
			} // End of event polling
		
			int direction = 0;
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
				direction = 1;
			} else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
				direction = -1;
			}
			ship.rotate(direction * ship.rotation_factor);
			
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
				float abs_rotation = ship.getRotation();
	//			sf::Vector2f heading = sf::Vector2f(static_cast<float>(std::cos(2 * M_PI * (abs_rotation / 360))), static_cast<float>(std::sin(2 * M_PI * (abs_rotation / 360))));
			
				// Create a normalized vector in the direction of travel
				float xN = static_cast<float>(sin(2 * M_PI * (abs_rotation / 360)));
				float yN = static_cast<float>(cos(2 * M_PI * (abs_rotation / 360)));

				// Add to velocity vector (using minus for y because of coordinate system)
				ship.velocity.x += xN * ship.acceleration;
				ship.velocity.y -= yN * ship.acceleration;
			}
			
			// Drag or wind or some shit
			const float dragFactor = 0.0001;

			// Use Stokes' law to apply drag to the ship
			ship.velocity.x = ship.velocity.x - ship.velocity.x * dragFactor;
			ship.velocity.y = ship.velocity.y - ship.velocity.y * dragFactor;
			
			// TODO: Fix the really obscure and rare case where the ship can stuck at any of the corners if hit perfectly
			for (auto i : renderables) {
#if 0
				// Velocity clamping
				if(i->velocity.x > i->max_velocity.x) {
					i->velocity.x = i->max_velocity.x;
				}
				if(i->velocity.y > i->max_velocity.y) {
					i->velocity.y = i->max_velocity.y; 
				}
				
				// Move object
				i->move(i->velocity * deltaTime);

				// Bounds checking
				if(i->getPosition().y <= 0) {
					i->setPosition(i->getPosition().x, WIN_HEIGHT);
				} else if (i->getPosition().y >= WIN_HEIGHT) {
					i->setPosition(i->getPosition().x, 0);
				}
				if(i->getPosition().x <= 0) {
					i->setPosition(WIN_WIDTH, i->getPosition().y);
				} else if (i->getPosition().x >= WIN_WIDTH) {
					i->setPosition(0, i->getPosition().y);
				}
				
				i->text.setPosition(i->getPosition().x + (i->mesh.getGlobalBounds().width / 2), i->getPosition().y + (i->mesh.getGlobalBounds().height) + i->textOffset.y);
				window.draw(i->text);
				window.draw(*i);
#endif		
				i->Tick(deltaTime);
				window.draw(*i);
				std::putc('\n', stdout);
			}
		} else { // On menu
			window.draw(menuTitle);
		}

		window.display();
	}

	std::cout << "\nQuitting successfully... thank you for playing Math Asteroids! :)" << std::endl;
	// Exit successfully
	return 0;
}
