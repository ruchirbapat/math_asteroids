/* MIT License
 *
 * Copyright (c) 2017, Victor Cushman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _SFML_COLLISION_H_
#define _SFML_COLLISION_H_

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace sfcollision
{

// Returns true if A and B are overlapping (i.e. colliding)
bool check_collision(const sf::Shape& A, const sf::Shape& B);
// ...
bool check_collision(const sf::Vector2f& A, const sf::Vector2f&       B);
bool check_collision(const sf::Vector2f& A, const sf::RectangleShape& B);
bool check_collision(const sf::Vector2f& A, const sf::CircleShape&    B);
bool check_collision(const sf::Vector2f& A, const sf::ConvexShape&    B);
bool check_collision(const sf::Vector2f& A, const sf::Sprite&         B);
// ...
bool check_collision(const sf::RectangleShape& A, const sf::Vector2f&       B);
bool check_collision(const sf::RectangleShape& A, const sf::RectangleShape& B);
bool check_collision(const sf::RectangleShape& A, const sf::CircleShape&    B);
bool check_collision(const sf::RectangleShape& A, const sf::ConvexShape&    B);
bool check_collision(const sf::RectangleShape& A, const sf::Sprite&         B);
// ...
bool check_collision(const sf::CircleShape& A, const sf::Vector2f&       B);
bool check_collision(const sf::CircleShape& A, const sf::RectangleShape& B);
bool check_collision(const sf::CircleShape& A, const sf::CircleShape&    B);
bool check_collision(const sf::CircleShape& A, const sf::ConvexShape&    B);
bool check_collision(const sf::CircleShape& A, const sf::Sprite&         B);
// ...
bool check_collision(const sf::ConvexShape& A, const sf::Vector2f&       B);
bool check_collision(const sf::ConvexShape& A, const sf::RectangleShape& B);
bool check_collision(const sf::ConvexShape& A, const sf::CircleShape&    B);
bool check_collision(const sf::ConvexShape& A, const sf::ConvexShape&    B);
bool check_collision(const sf::ConvexShape& A, const sf::Sprite&         B);
// ...
bool check_collision(const sf::Sprite& A, const sf::Vector2f& B);
bool check_collision(const sf::Sprite& A, const sf::RectangleShape& B);
bool check_collision(const sf::Sprite& A, const sf::CircleShape& B);
bool check_collision(const sf::Sprite& A, const sf::ConvexShape& B);
bool check_collision(const sf::Sprite& A, const sf::Sprite& B);

} // !sfcollision
#endif
