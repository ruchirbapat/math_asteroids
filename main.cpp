// Headers
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <cctype>
#include <stdexcept>
#include <sstream>
#include <tuple>
#include <functional>
#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>

using std::vector;
using std::cout;
using std::printf;

#define array_size(a) (sizeof(a)/sizeof(a[0]))
#define lerp(a, b, t) ((a * (1-t)) + (b*t))
#define print(ss) std::cout << ss << std::endl

namespace cstar {

	namespace GameManager {
		static short sum_high = 1;
		static short sum_low = 15;
		static unsigned int asteroid_count = 0;
		static const unsigned int target_asteroid_count = 20;
		static float asteroid_delay = 0;
		static const float target_asteroid_delay = 5; // This is in seconds
	}

	// Helper function
	inline short rand_between(short min, short max) { return static_cast<short>(std::rand() % (max + 1) + min); };

	inline auto perp_distance(sf::Vector2f line_start, sf::Vector2f line_end, sf::Vector2f point) {
		double normalLength = hypot(line_end.x - line_start.x, line_end.y - line_start.y);
		double distance = (double)((point.x - line_start.x) * (line_end.y - line_start.y) - (point.y - line_start.y) * (line_end.x - line_start.x)) / normalLength;
		return std::abs(distance);
	}

#define DEG2RAD (M_PI/180)
#define RAD2DEG (180/M_PI)
#define b2v2(vec2) (b2Vec2(vec2.x, vec2.y))

	// Game constants
	const int WIN_WIDTH = 600;
	const int WIN_HEIGHT = 600;
	bool onMenu = true;

	// Forward declare MovingEntity class for the renderables
	class MovingEntity;

	//Update this shit to use C++ smart pointers, the fastest one preferably
	static std::vector<MovingEntity*> renderables;

	//static float worldScale = 1;
	//static b2World world(b2Vec2(0, 0));

	static sf::Font font_face;
	class MovingEntity : public sf::Transformable, public sf::Drawable {
		public:
			explicit MovingEntity(sf::Vector2f initial_vel, sf::Vector2f initial_max_vel, const float accel, MovingEntity* child, std::string _name) :
				meshPtr(nullptr), velocity(initial_vel), max_velocity(initial_max_vel), acceleration(accel), name(_name) {

					renderables.push_back(child);
				};
			//sf::ConvexShape mesh;
			sf::Shape* meshPtr;
			sf::Vector2f velocity;
			const sf::Vector2f max_velocity;
			const float acceleration;
			//b2Body* body;

#if 0
			void MakeBody() {
				b2BodyDef bodyDef;
				bodyDef.type = b2BodyType::b2_dynamicBody;
				bodyDef.position.Set(getPosition().x / worldScale, getPosition().y / worldScale);
				body = world.CreateBody(&bodyDef);

				b2PolygonShape polygonShape;
				polygonShape.SetAsBox(mesh.getLocalBounds().width / 2 / worldScale, mesh.getLocalBounds().height / 2 / worldScale);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &polygonShape;
				fixtureDef.density = 1;
				fixtureDef.friction = 0.3f;
				body->CreateFixture(&fixtureDef);
			}
#endif

			virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
				states.transform *= getTransform();
				target.draw(*meshPtr, states);
			}

			virtual void Tick(float deltaTime) {
				// Velocity clamping
				if (velocity.x > max_velocity.x) {
					velocity.x = max_velocity.x;
				}
				if (velocity.y > max_velocity.y) {
					velocity.y = max_velocity.y;
				}

				// Move object
				move(velocity * deltaTime);
#if 0
				b2Vec2 cur_pos = b2v2(getPosition());
				cur_pos += b2v2(velocity);
				cur_pos *= deltaTime;
				body->SetTransform(cur_pos, getRotation());

				setPosition(body->GetPosition().x * worldScale, body->GetPosition().y * worldScale);
#endif

				// Bounds checking
				if (getPosition().y <= 0) {
					setPosition(getPosition().x, WIN_HEIGHT);
				}
				else if (getPosition().y >= WIN_HEIGHT) {
					setPosition(getPosition().x, 0);
				}
				if (getPosition().x <= 0) {
					setPosition(WIN_WIDTH, getPosition().y);
				}
				else if (getPosition().x >= WIN_WIDTH) {
					setPosition(0, getPosition().y);
				}
			}

			virtual ~MovingEntity() {
				std::cout << "Called destructor on " << name << std::endl;
				delete meshPtr;

				std::find(renderables.begin(), renderables.end(), this) - renderables.begin();

				std::vector<MovingEntity*>::iterator iterator_index = std::find(renderables.begin(),
						renderables.end(),
						this);

				renderables.erase(iterator_index);
			}

			std::string name;
	};

	class Asteroid;
	static std::unordered_map<int, Asteroid*> asteroid_lut;

	//const static std::string operands = "+-*";
	class Asteroid : public MovingEntity {
		public:
			Asteroid() :
				MovingEntity(sf::Vector2f(0, 0), sf::Vector2f(50, 50), 0.0f, this, "Asteroid"), // Base constructor first
				answer(0), num_points(rand_between(5, max_points)), text(sf::Text())
		{
			sf::ConvexShape* mesh = new sf::ConvexShape();
			// Muh procedural asteroid generation
			// TODO: use line rendering rather than a small thickness
			//num_points = (std::rand() % max_points) + 5;
			float perc_diff = 5 / 100;
			unsigned short gen_radius = (std::rand() % 7) + 2;
			float min_rad = gen_radius * (1 + perc_diff), max_rad = gen_radius * (1 - perc_diff);;
			mesh->setPointCount(num_points);
			for (unsigned short i = 0, cur_angle = 0; i < num_points; i++, cur_angle += (360 / num_points)) {
				//			unsigned short rand_radius = (std::rand() % 10) + 2;
				unsigned short rand_radius = static_cast<unsigned short>((std::rand() % (short)max_rad) + (short)min_rad);
				mesh->setPoint(i, sf::Vector2f(rand_radius * std::sin(cur_angle), rand_radius * std::cos(cur_angle)));
			}
			mesh->setFillColor(sf::Color(0, 0, 0, 0));
			mesh->setOutlineColor(sf::Color::White);
			mesh->setOutlineThickness(0.1f);
			
			print("Generating between " << GameManager::sum_low << " and " << GameManager::sum_high);
			// Numerals, change to it to have a private member of min and max values
			short numerals[2] = { rand_between(GameManager::sum_low, GameManager::sum_high), 
								  rand_between(GameManager::sum_low, GameManager::sum_high) };
			short randop = rand_between(0, operands.length() - 1);
			unsigned char operandType(operands.at(randop));
			answer = [=]() -> int {
				switch (operandType) {
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

			meshPtr = mesh;

			asteroid_lut[answer] = this;
			std::stringstream ss;
			ss << numerals[0] << ' ' << operandType << ' ' << numerals[1];
			text.setFont(font_face);
			text.setString(ss.str());
			text.setCharacterSize(20);
			text.setFillColor(sf::Color::White);
			text.setStyle(sf::Text::Regular);
			
			GameManager::asteroid_count++;
		}

			virtual void Tick(float deltaTime) override {
				MovingEntity::Tick(deltaTime);
				text.setPosition(getPosition().x + (meshPtr->getGlobalBounds().width / 2), getPosition().y + (meshPtr->getGlobalBounds().height) + textOffset.y);
			}

			virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
				MovingEntity::draw(target, states);
				target.draw(text, states);
			}

			~Asteroid() {
				asteroid_lut[answer] = nullptr;
				cstar::GameManager::asteroid_count--;
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
				rotation_factor(6.0f), health_text(sf::Text()) { // This class's members

					sf::CircleShape* mesh = new sf::CircleShape();

					mesh->setRadius(5);

#if 0
					mesh.setPointCount(4);
					mesh.setPoint(0, sf::Vector2f(1, 0));
					mesh.setPoint(1, sf::Vector2f(2, 2));
					mesh.setPoint(2, sf::Vector2f(1, 1.5));
					mesh.setPoint(3, sf::Vector2f(0, 2));
#endif

					mesh->setFillColor(sf::Color::Black);
					mesh->setOutlineColor(sf::Color::White);
					mesh->setOutlineThickness(0.1f);
					mesh->setPosition(0, 0);
					mesh->setOrigin(mesh->getRadius(), mesh->getRadius());

					radius = mesh->getRadius();

					meshPtr = mesh;

					ss.clear();
					ss << health << ' ' << "HP";
					health_text.setFont(font_face);
					health_text.setString(ss.str());
					health_text.setCharacterSize(20);
					health_text.setFillColor(sf::Color::White);
					health_text.setStyle(sf::Text::Regular);

					//MakeBody();
				}

			virtual void Tick(float deltaTime) override {
				MovingEntity::Tick(deltaTime);
				//setOrigin((getPosition().x + meshPtr->getGlobalBounds().width) / 2, (getPosition().y + meshPtr->getGlobalBounds().height) / 2);
				if (health != prev_health) {
					ss.str("");
					ss.clear();
					ss << health << ' ' << "HP";
					health_text.setString(ss.str());
				}

				health_text.setPosition(getPosition().x + (meshPtr->getGlobalBounds().width / 2), getPosition().y + (meshPtr->getGlobalBounds().height) + textOffset.y);
				this->UpdateSpriteProperties();
			}

			virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {

				// Drawing of mesh turned off
				MovingEntity::draw(target, states);

				// Draw sprite and HP
				target.draw(sprite, states);
				target.draw(health_text, states);
			}

			inline void TakeDamage(unsigned short damage) {
				health -= damage;
				prev_health = damage;
			}

			void SetSprite(sf::Sprite& _sprite) {
				meshPtr->setTexture(_sprite.getTexture());
				sprite = _sprite;
				sprite.setOrigin((_sprite.getPosition().x + _sprite.getGlobalBounds().width) / 2, (_sprite.getPosition().y + sprite.getGlobalBounds().height) / 2);
			}

			inline void SetSpriteOrigin(const sf::Vector2f origin) {
				sprite.setOrigin(origin);
			}

			inline void UpdateSpriteProperties() {
				sprite.setScale((radius / getScale().x) * 0.75, (radius / getScale().y) * 0.75);
				sprite.setRotation(getRotation());
				sprite.setPosition(getPosition());
			}

			inline short GetRadius() {
				return radius;
			}

			inline auto SetRadius(short rad) {
				radius = rad;
				return radius;
			}

			~Spaceship() {};
			static void reset_position();
			const float rotation_factor;
		private:
			short radius;
			int health = 100;
			int prev_health = 100;
			std::stringstream ss;
			sf::Text health_text;
			const sf::Vector2f textOffset = sf::Vector2f(20, 20);
			sf::Sprite sprite;
	};

	struct button_style {
		sf::Color fill_colour;
		sf::Color text_colour;
	};

	// Forward declarations
	class Button;
	enum class button_state;

	static std::unordered_map<button_state,button_style> button_colours;
	static std::vector<Button*> gui_buttons;

	enum class button_state {
		hovered,
		clicked,
		neither
	};

	class Button : public sf::Transformable, public sf::Drawable {
		public:
			Button(sf::Vector2f initial_pos, sf::Vector2f initial_scale, std::string button_string) : text(sf::Text()), current_state(cstar::button_state::neither), state_style(), 
			on_click_function(nullptr)
		{
			
			set_state(button_state::neither);
			update_style();
			
			mesh = sf::RectangleShape(initial_scale);
			mesh.setFillColor(state_style.fill_colour);
			mesh.setOutlineColor(sf::Color::White);
			mesh.setOutlineThickness(1);
			mesh.setOrigin(mesh.getGlobalBounds().width / 2, mesh.getGlobalBounds().height / 2);
			mesh.setPosition(initial_pos);

			text.setFont(font_face);
			text.setString(sf::String(button_string));
			text.setCharacterSize(20);
			text.setFillColor(state_style.text_colour);
			text.setStyle(sf::Text::Regular);
			text.setOrigin(text.getGlobalBounds().width / 2, text.getGlobalBounds().height / 2);
			text.setPosition(mesh.getPosition());

			gui_buttons.push_back(this);
		};

			inline sf::Text GetText() {
				return text;
			}

			inline void SetOnClick(void (*func)(void)) {
				on_click_function = func;
			}

			inline void set_state(button_state new_state) {
				current_state = new_state;
			}

			inline void update_style() {
				state_style = cstar::button_colours[current_state];
			}

			bool window_intersects(const sf::RenderWindow* win) {

				sf::FloatRect mouse_box;
				mouse_box.width = 1;
				mouse_box.height = 1;
				mouse_box.left = sf::Mouse::getPosition(*win).x;
				mouse_box.top = sf::Mouse::getPosition(*win).y;
				
				return mouse_box.intersects(mesh.getGlobalBounds());
			}

			void Tick(const sf::RenderWindow* win, bool clicked) {
				// Check for intersections
				set_state(window_intersects(win) ? (clicked ? button_state::clicked : button_state::hovered) : button_state::neither);
				if(clicked) {
					this->on_click_function();
					print("Clicked on " << text.getString().toAnsiString());
				}
				update_style();
				mesh.setFillColor(state_style.fill_colour);
				text.setFillColor(state_style.text_colour);
	
			}
			virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
				target.draw(mesh, states);
				target.draw(text, states);
			}

		private:
			sf::Text text;
			sf::RectangleShape mesh;
			button_state current_state;
			button_style state_style;
			void (*on_click_function)(void);
	};

}

using namespace cstar;

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
			s.antialiasingLevel = 2;
			return s;
			}());

	const sf::RenderWindow* win_ptr = &window;

	// Enable Vsync
	window.setVerticalSyncEnabled(true);

	font_face.loadFromFile("NimbusRegular.otf");

	// Initialise all GUI stuff on the main menu
	sf::Text menuTitle;
	menuTitle.setFont(font_face);
	menuTitle.setString("MATH ASTEROIDS");
	menuTitle.setCharacterSize(60);
	menuTitle.setFillColor(sf::Color::White);
	menuTitle.setStyle(sf::Text::Bold);
	menuTitle.setOrigin(menuTitle.getGlobalBounds().width / 2, menuTitle.getGlobalBounds().height / 2);
	menuTitle.setPosition(WIN_WIDTH / 2, menuTitle.getGlobalBounds().height / 2);

	// Buttons and their callbacks
	auto easy_mode_callback = []() -> void { 
		cstar::GameManager::sum_low = 1; 
		cstar::GameManager::sum_high = 12; 
		onMenu = false;
		print("Easy mode selected.");
	};
	Button* easy_button = new Button(sf::Vector2f(WIN_WIDTH / 2, (WIN_HEIGHT / 2) - 50), sf::Vector2f(100, 50), "EASY");
	easy_button->SetOnClick(easy_mode_callback);

	auto hard_mode_callback = []() -> void { 
		cstar::GameManager::sum_low = 15; 
		cstar::GameManager::sum_high = 30; 
		onMenu = false;
		print("Hard mode selected");
	};
	Button* hard_button = new Button(sf::Vector2f(WIN_WIDTH / 2, (WIN_HEIGHT / 2) + 50), sf::Vector2f(100, 50), "HARD");
	hard_button->SetOnClick(hard_mode_callback);

	// Load player assets
	sf::Texture ship_texture;
	ship_texture.loadFromFile("ufo.png");
	ship_texture.setSmooth(true);
	sf::Sprite ship_sprite;
	ship_sprite.setTexture(ship_texture);

	Spaceship ship;
	ship.SetSprite(ship_sprite);
	ship.setPosition(0, 0);
	//	ship_sprite.setPosition(ship.getPosition());
	ship.setOrigin(ship.getPosition());
	//ship.meshPtr->setPosition(WIN_WIDTH / 2, WIN_HEIGHT / 2);
	ship.scale(10, 10);

	std::string typed_text = std::string();
	short text_sign = 1;

	cstar::button_colours[cstar::button_state::neither] = cstar::button_style{ sf::Color::Black, sf::Color::White };
	cstar::button_colours[cstar::button_state::hovered] = cstar::button_style{ sf::Color::White, sf::Color::Black };
	cstar::button_colours[cstar::button_state::clicked] = cstar::button_style{ sf::Color::Red, sf::Color::White };

#if 0
	for (int i = 0; i < 0; i++) {
		Asteroid* a = new Asteroid();
		auto w = static_cast<unsigned int>(a->meshPtr->getGlobalBounds().width);
		auto h = static_cast<unsigned int>(a->meshPtr->getGlobalBounds().height);
		auto x = std::rand() % (WIN_WIDTH - w) + w;
		auto y = std::rand() % (WIN_HEIGHT - h) + h;
		a->setPosition(x, y);
		a->scale(5, 5);
		a->velocity = sf::Vector2f(-1 * 50, -1 * 50);
	}
#endif 

	// Clock has started!
	sf::Time lastTime = sf::microseconds(0);
	sf::Clock clock;

	// Main event loop
	while (window.isOpen()) {

		window.clear();
		sf::Time currentTime = clock.getElapsedTime(), deltaTimeMS = currentTime - lastTime;
		lastTime = currentTime;

		// Capture events
		sf::Event e;
		float deltaTime = deltaTimeMS.asSeconds();
		if (!onMenu) {
hansali:
			if(cstar::GameManager::asteroid_count < cstar::GameManager::target_asteroid_count) {
				if(cstar::GameManager::asteroid_delay < cstar::GameManager::target_asteroid_delay) {
					cstar::GameManager::asteroid_delay += deltaTime;
				} else  {

					Asteroid* a = new Asteroid();
					auto w = static_cast<unsigned int>(a->meshPtr->getGlobalBounds().width);
					auto h = static_cast<unsigned int>(a->meshPtr->getGlobalBounds().height);
					unsigned int x;
					unsigned int y;
					do {
						x = std::rand() % (WIN_WIDTH - w) + w;
						y = std::rand() % (WIN_HEIGHT - h) + h;
					} while((x == ship.getPosition().x) || (y == ship.getPosition().y));
					a->setPosition(x, y);
					a->scale(5, 5);
					a->velocity = sf::Vector2f(rand_between(-50, 50), rand_between(-50, 50));
					cstar::GameManager::asteroid_delay = 0;
				}
			}

			while (window.pollEvent(e)) {
				if (e.type == sf::Event::EventType::Closed) {
					window.close(); break;

				}

				else if (e.type == sf::Event::EventType::TextEntered) {
					char c = static_cast<char>(e.text.unicode);
					if (isdigit(c)) {
						typed_text += c;
					}
					else if (c == '-') {
						text_sign *= -1;
					}
					std::cout << "Char entered: " << (char)e.text.unicode << std::endl;
				}
				else if (e.type == sf::Event::EventType::KeyReleased) {
					if (e.key.code == sf::Keyboard::Return && !typed_text.empty()) {
						std::cout << "Text: " << typed_text << std::endl;
						// handle the text entered
						int sum = text_sign * std::atoi(typed_text.c_str());

						// Reset text string and text sign (1 or -1)
						typed_text = std::string();
						text_sign = 1;

						// Destroy the appropriate asteroid
						if (asteroid_lut.find(sum) != asteroid_lut.end()) {
							auto asteroid = asteroid_lut[sum];
							auto angle = atan2((asteroid->getPosition().y + asteroid->velocity.y) - ship.getPosition().y,
									(asteroid->getPosition().x + asteroid->velocity.x) - ship.getPosition().x) * RAD2DEG;

							ship.setRotation(angle + 90);

							delete asteroid_lut[sum];
						}

						// shoot at meme with sum on it
						std::cout << "Shot at " << sum << std::endl; // This current throws an error!
					}
				}
				// End of checking the event type
			} // End of event polling

			int direction = 0;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
				direction = 1;
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
				direction = -1;
			}
			ship.rotate(direction * ship.rotation_factor);

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
				float abs_rotation = ship.getRotation();

				// Create a normalized vector in the direction of travel
				float xN = static_cast<float>(sin(2 * M_PI * (abs_rotation / 360)));
				float yN = static_cast<float>(cos(2 * M_PI * (abs_rotation / 360)));

				// Add to velocity vector (using minus for y because of coordinate system)
				ship.velocity.x += xN * ship.acceleration;
				ship.velocity.y -= yN * ship.acceleration;
			}

			// Drag or wind or some shit
			const float dragFactor = 0.01;

			// Use Stokes' law to apply drag to the ship
			ship.velocity.x = ship.velocity.x - ship.velocity.x * dragFactor;
			ship.velocity.y = ship.velocity.y - ship.velocity.y * dragFactor;

			// TODO: Fix the extremely rare case where the ship can stuck at any of the corners if hit perfectly
			//	sf::CircleShape* circle_shape = static_cast<sf::CircleShape*>(ship.meshPtr);
			for (auto i : renderables) {
				i->Tick(deltaTime);
				if (i->name == "Asteroid") {
					auto d = std::sqrt(std::pow(ship.getPosition().x - i->getPosition().x, 2) + std::pow(ship.getPosition().y - i->getPosition().y, 2));
					if (d < ((ship.GetRadius() * ship.getScale().x) + ((ship.meshPtr->getGlobalBounds().width + ship.meshPtr->getGlobalBounds().height) / 2))) {
						std::cout << "COLLISION" << std::endl;
					}

				}

				window.draw(*i);
			}
		}
		else { // On menu
			window.draw(menuTitle);
			while (window.pollEvent(e)) {
				if (e.type == sf::Event::EventType::Closed) {
					window.close(); break;
				} // End of checking the event type
			} // End of event polling
			
			for (auto i : gui_buttons) {
				i->Tick(win_ptr, sf::Mouse::isButtonPressed(sf::Mouse::Button::Left));
				window.draw(*i);
			}

		}
		
		window.display();
	}

	
	std::cout << "\nQuitting successfully... thank you for playing Math Asteroids! :)" << std::endl;
	// Exit successfully
	return 0;
}
