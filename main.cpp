// Headers
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>

// Helper function, consider changing to an inline or a C-style macro
template<typename T>
inline auto rand_between (T min, T max) { return std::rand() % (max + 1) + min; };

#define DEG2RAD (M_PI/180)
#define RAD2DEG (180/M_PI)

// Game constants
const int WIN_WIDTH = 600;
const int WIN_HEIGHT = 600;

// Forward declare MovingEntity class for the renderables
class MovingEntity;

//Update this shit to use C++ smart pointers, the fastest one preferably
static std::vector<MovingEntity*> renderables; 

class MovingEntity : public sf::Transformable, public sf::Drawable {
public:
	explicit MovingEntity(sf::Vector2f initial_vel, sf::Vector2f initial_max_vel, const float accel, MovingEntity* child, std::string _name) : velocity(initial_vel), max_velocity(initial_max_vel), acceleration(accel), name(_name) {
		::renderables.push_back(child);
	};
	sf::ConvexShape mesh;
	sf::Vector2f velocity;
	const sf::Vector2f max_velocity;
	const float acceleration;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		states.transform *= getTransform();
		target.draw(mesh, states);
	}
	std::string name;
};

//std::string operands = "+-*/";

#if 0
	sf::Font f;
	f.loadFromFile("NimbusRegular.otf");

	sf::Text text;
	text.setFont(f);
	text.setString("Math Asteroids");
	text.setCharacterSize(24);
	text.setFillColor(sf::Color::White);
	text.setStyle(sf::Text::Bold);
#endif

const static std::string operands = "+-*";
class Asteroid : public MovingEntity {
public:
	Asteroid() : 
		MovingEntity(sf::Vector2f(0, 0), sf::Vector2f(50, 50), 0.0f, this, "Asteroid"), // Base constructor first
		answer(0), num_points( rand_between<short>(5, max_points) )
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
		short numerals[2] = { static_cast<short>((rand_between(5, 30))), static_cast<short>(rand_between(5, 30)) }; 
		unsigned char operandType( operands.c_str()[ rand_between<short>(0, operands.length() - 1) ] );

		// Sum initialised in initialiser list above
		std::printf("Asteroid created with %d points and the generated sum is %d%c%d\n", num_points, numerals[0], operandType, numerals[1]);
		switch(operandType) {
			case '+':
				answer = numerals[0] + numerals[1];
				std::cout << "Adding" << std::endl;
			case '-':
				answer = numerals[0] - numerals[1];
				std::cout << "Subtracting" << std::endl;
			case '*':
				answer = numerals[0] * numerals[1];
				std::cout << "Multiplying" << std::endl;
			case '/':
				answer = numerals[0] / numerals[1]; // DivisionError?
		}
	}
	~Asteroid() {};
private:
	int answer;
	const unsigned short max_points = 6;
	unsigned short num_points;
//	static const sf::Font font_face = font_face.loadFromFile("NimbusRegular.otf");
};

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
//	sf::Vector2f velocity; // adds onto the inherited position
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

	// Enable Vsync
	window.setVerticalSyncEnabled(true);
		
	sf::Font f;
	f.loadFromFile("NimbusRegular.otf");

	sf::Text text;
	text.setFont(f);
	text.setString("Math Asteroids");
	text.setCharacterSize(24);
	text.setFillColor(sf::Color::White);
	text.setStyle(sf::Text::Bold);
	
	Spaceship ship;
	ship.setPosition(WIN_WIDTH/2, WIN_HEIGHT/2);
	ship.scale(10, 10);
	ship.setOrigin(0.5 * ship.mesh.getGlobalBounds().width / 2, 0.5 * ship.mesh.getGlobalBounds().height / 2);
	
	std::string typed_text = "";
	for(int i = 0; i < 5; i++) {
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
	
	// Add custom callbacks
	// Main event loop
	while(window.isOpen()) {
		window.clear();
		sf::Time currentTime = clock.getElapsedTime(), deltaTimeMS = currentTime - lastTime; 
		lastTime = currentTime;

		// Capture events
		sf::Event e;
		auto deltaTime = deltaTimeMS.asSeconds();
		while(window.pollEvent(e)) {
			if(e.type == sf::Event::EventType::Closed) {
				window.close(); break;

			} else if(e.type == sf::Event::EventType::TextEntered && isdigit(static_cast<char>(e.text.unicode))) {
				typed_text += static_cast<char>(e.text.unicode);
			} else if(e.type == sf::Event::EventType::KeyReleased) {
				if (e.key.code == sf::Keyboard::Return && (typed_text.length() - 1)){
					std::cout << "meme: " << typed_text << std::endl;
					// handle the text entered
					int sum = std::atoi(typed_text.c_str());
					typed_text = "";
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
		}// else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Return)) {
	//		// handle the text entered
//			int sum = std::atoi(iftext.c_str());
//			iftext = "";
//			// shoot at meme with sum on it
//			std::cout << "Shot at " << sum << std::endl; // This current throws an error!
//
//		}
		
		ship.rotate(direction * ship.rotation_factor);
		
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			float abs_rotation = ship.getRotation();
//			std::cout << "Rotation: " << abs_rotation << std::endl;
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
		
		window.draw(text);

		// Bounds checking to make sure the ship doesn't go off the screen
		// TODO: Fix the really obscure and rare case where the ship can stuck at any of the corners if hit perfectly
		for (auto i : renderables) {
			// Velocity clamping
			if(i->velocity.x > i->max_velocity.x) {
				i->velocity.x = i->max_velocity.x;
			}
			if(i->velocity.y > i->max_velocity.y) {
				i->velocity.y = i->max_velocity.y; 
			}
			
			// Move object
			i->move(i->velocity.x * deltaTime, i->velocity.y * deltaTime);

			// Bounds checking
			if(i->getPosition().y <= 0) {
				i->setPosition(WIN_WIDTH - i->getPosition().x, WIN_HEIGHT);
			} else if (i->getPosition().y >= WIN_HEIGHT) {
				i->setPosition(WIN_HEIGHT - i->getPosition().x, 0);
			}
			if(i->getPosition().x <= 0) {
				i->setPosition(WIN_WIDTH, WIN_HEIGHT - i->getPosition().y);
			} else if (i->getPosition().x >= WIN_WIDTH) {
				i->setPosition(0, WIN_HEIGHT - i->getPosition().y);
			}
			
			// Render
			window.draw(*i);
		}

		window.display();
	}
	
	std::cout << "\nQuitting successfully... thank you for playing Math Asteroids! :)" << std::endl;
	// Exit successfully
	return 0;
}
