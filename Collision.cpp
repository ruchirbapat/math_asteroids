#include <cmath>
#include "Collision.h"

namespace sfcollision
{

struct interval{float min, max;};

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Dot product of A and B
static inline float inner_product(const sf::Vector2f A, const sf::Vector2f B)
{
    return A.x*B.x + A.y*B.y;
}

////////////////////////////////////////////////////////////////////////////////
// Returns vec as a unit vector
static inline sf::Vector2f normalize(sf::Vector2f vec)
{
    float magnitude = std::sqrt(vec.x*vec.x + vec.y*vec.y);
    vec.x /= magnitude;
    vec.y /= magnitude;
    return vec;
}

////////////////////////////////////////////////////////////////////////////////
// Rotate vector by +90deg to get a perpendicular Vector2f
static inline sf::Vector2f perpendicular(const sf::Vector2f vec)
{
    // Normal vector transformed from edge using (x, y) => (-y, x).
    // The transformation (x, y) => (y, -x) is also acceptable, but would
    // would rotate it -90deg instead of +90deg.
    return sf::Vector2f(-vec.y, vec.x);
}

////////////////////////////////////////////////////////////////////////////////
// Returns the distance between two points squared.
// Distance formula uses a square root, but this value is usually squared anyway
// so this squared value is returned to avoid wasting clock cycles.
static inline float distance_squared(const sf::Vector2f A, const sf::Vector2f B)
{
    return (B.x - A.x)*(B.x - A.x) + (B.y - A.y)*(B.y - A.y);
}

////////////////////////////////////////////////////////////////////////////////
// Get the global position of the center of a CircleShape
static sf::Vector2f get_circle_center_position(const sf::CircleShape& circle)
{
    sf::Vector2f local_center = circle.getPoint(0);
    local_center.y += circle.getRadius(); // determined from testing
    return circle.getTransform().transformPoint(local_center);
}

////////////////////////////////////////////////////////////////////////////////
static interval project(const sf::Vector2f& point, const sf::Vector2f& axis)
{
    float dot = inner_product(point, normalize(axis));
    return interval {dot, dot};
}

////////////////////////////////////////////////////////////////////////////////
static interval project(const sf::CircleShape& circle, const sf::Vector2f& axis)
{
    interval I = project(get_circle_center_position(circle), normalize(axis));
    I.min -= circle.getRadius();
    I.max += circle.getRadius();
    return I;
}

////////////////////////////////////////////////////////////////////////////////
static interval project(const sf::Shape& polygon, const sf::Vector2f& axis)
{
    sf::Transform transform = polygon.getTransform();
    unsigned point_count = polygon.getPointCount();
    sf::Vector2f normalized_axis = normalize(axis);

    sf::Vector2f point = transform.transformPoint(polygon.getPoint(0));
    float dot = inner_product(point, normalized_axis);
    interval I {dot, dot};
    for (unsigned i = 1; i < point_count; ++i)
    {
        point = transform.transformPoint(polygon.getPoint(i));
        dot = inner_product(point, normalized_axis);
        if (dot < I.min)
        {
            I.min = dot;
        }
        else if (dot > I.max)
        {
            I.max = dot;
        }
    }
    return I;
}

////////////////////////////////////////////////////////////////////////////////
static interval project(const sf::Sprite& sprite, const sf::Vector2f& axis)
{
    sf::Transform transform = sprite.getTransform();
    sf::FloatRect local_rect = sprite.getLocalBounds();
    sf::Vector2f normalized_axis = normalize(axis);

    std::vector<sf::Vector2f> points {
        sf::Vector2f(local_rect.left, local_rect.top), // top left
        sf::Vector2f(local_rect.left + local_rect.width, local_rect.top), // top right
        sf::Vector2f(local_rect.left + local_rect.width, local_rect.top + local_rect.height), // bottom right
        sf::Vector2f(local_rect.left, local_rect.top + local_rect.height) // bottom left
    };

    sf::Vector2f point = transform.transformPoint(points[0]);
    float dot = inner_product(point, normalized_axis);
    interval I {dot, dot};
    for (unsigned i = 1; i < points.size(); ++i)
    {
        point = transform.transformPoint(points[i]);
        dot = inner_product(point, normalized_axis);
        if (dot < I.min)
        {
            I.min = dot;
        }
        else if (dot > I.max)
        {
            I.max = dot;
        }
    }
    return I;
}

////////////////////////////////////////////////////////////////////////////////
static inline bool is_overlap(const interval& A, const interval& B)
{
    return !(A.max < B.min || B.max < A.min);
}

////////////////////////////////////////////////////////////////////////////////
// return all four points that make up the box of a Sprite
static std::vector<sf::Vector2f> get_sprite_points(const sf::Sprite& sprite)
{
    sf::FloatRect local_rect = sprite.getLocalBounds();
    return std::vector<sf::Vector2f> {
        sf::Vector2f(local_rect.left, local_rect.top), // top left
        sf::Vector2f(local_rect.left + local_rect.width, local_rect.top), // top right
        sf::Vector2f(local_rect.left + local_rect.width, local_rect.top + local_rect.height), // bottom right
        sf::Vector2f(local_rect.left, local_rect.top + local_rect.height) // bottom left
    };
}

////////////////////////////////////////////////////////////////////////////////
template <typename T>
static bool test_projection_overlap_using_polygon(const sf::Shape& polygon, const T& other)
{
    unsigned point_count = polygon.getPointCount();
    sf::Transform transform = polygon.getTransform();
    for (unsigned i = 0; i < point_count; ++i)
    {
        sf::Vector2f edge = transform.transformPoint(polygon.getPoint((i+1) % point_count)) 
            - transform.transformPoint(polygon.getPoint(i));
        sf::Vector2f perp = perpendicular(edge);
        interval projA = project(polygon, perp);
        interval projB = project(other, perp);
        if (!is_overlap(projA, projB))
        {
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
static bool test_projection_overlap_using_circle(const sf::CircleShape& circle, const sf::Shape& polygon)
{
    unsigned point_count = polygon.getPointCount();
    sf::Transform transform = polygon.getTransform();
    sf::Vector2f circle_center = get_circle_center_position(circle);
    for (unsigned i = 0; i < point_count; ++i)
    {
        sf::Vector2f poly_point = transform.transformPoint(polygon.getPoint(i));
        sf::Vector2f edge = poly_point - circle_center;
        interval projA = project(polygon, edge);
        interval projB = project(circle, edge);
        if (!is_overlap(projA, projB))
        {
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
template <typename T>
static bool test_projection_overlap_using_sprite(const sf::Sprite& sprite, const T& other)
{
    sf::Transform transform = sprite.getTransform();
    auto points = get_sprite_points(sprite);

    for (unsigned i = 0; i < points.size(); ++i)
    {
        sf::Vector2f edge = transform.transformPoint(points[i+1 % points.size()]) 
            - transform.transformPoint(points[i]);
        sf::Vector2f perp = perpendicular(edge);
        interval projA = project(sprite, perp);
        interval projB = project(other, perp);
        if (!is_overlap(projA, projB))
        {
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool check_collision(const sf::Shape& A, const sf::Shape& B)
{
    return test_projection_overlap_using_polygon(A, B) &&
        test_projection_overlap_using_polygon(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::Vector2f& A, const sf::Vector2f& B)
{
    return A == B;
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::Vector2f& A, const sf::RectangleShape& B)
{
    return test_projection_overlap_using_polygon(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::Vector2f& A, const sf::CircleShape& B)
{
    float R = B.getRadius();
    sf::Vector2f B_pos = get_circle_center_position(B);
    float Dsq = distance_squared(A, B_pos);
    float Rsq = R*R;
    return Dsq <= Rsq;
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::Vector2f& A, const sf::ConvexShape& B)
{
    return test_projection_overlap_using_polygon(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::Vector2f& A, const sf::Sprite& B)
{
    return test_projection_overlap_using_sprite(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::RectangleShape& A, const sf::Vector2f& B)
{
    return check_collision(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::RectangleShape& A, const sf::RectangleShape& B)
{
    return test_projection_overlap_using_polygon(A, B) &&
        test_projection_overlap_using_polygon(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::RectangleShape& A, const sf::CircleShape& B)
{
    return test_projection_overlap_using_polygon(A, B) &&
        test_projection_overlap_using_circle(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::RectangleShape& A, const sf::ConvexShape& B)
{
    return test_projection_overlap_using_polygon(A, B) &&
        test_projection_overlap_using_polygon(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::RectangleShape& A, const sf::Sprite& B)
{
    return test_projection_overlap_using_polygon(A, B) &&
        test_projection_overlap_using_sprite(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::CircleShape& A, const sf::Vector2f& B)
{
    return check_collision(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::CircleShape& A, const sf::RectangleShape& B)
{
    return check_collision(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::CircleShape& A, const sf::CircleShape& B)
{
    float RA = A.getRadius();
    float RB = B.getRadius();
    float Dsq = distance_squared(get_circle_center_position(A),
        get_circle_center_position(B));
    return Dsq <= (RA+RB)*(RA+RB);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::CircleShape& A, const sf::ConvexShape& B)
{
    return test_projection_overlap_using_circle(A, B) &&
        test_projection_overlap_using_polygon(B, A); 
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::CircleShape& A, const sf::Sprite& B)
{
    bool is_projection_collision_circle_sprite = true;
    std::vector<sf::Vector2f> points = get_sprite_points(B);
    sf::Transform transform = B.getTransform();
    sf::Vector2f circle_center = get_circle_center_position(A);
    for (unsigned i = 0; i < points.size(); ++i)
    {
        sf::Vector2f point = transform.transformPoint(points[i]);
        sf::Vector2f edge = point - circle_center;
        interval projA = project(B, edge);
        interval projB = project(A, edge);
        if (!is_overlap(projA, projB))
        {
            is_projection_collision_circle_sprite = false;
        }
    }

    return is_projection_collision_circle_sprite &&
        test_projection_overlap_using_sprite(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::ConvexShape& A, const sf::Vector2f& B)
{
    return check_collision(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::ConvexShape& A, const sf::RectangleShape& B)
{
    return check_collision(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::ConvexShape& A, const sf::CircleShape& B)
{
    return check_collision(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::ConvexShape& A, const sf::ConvexShape& B)
{
    return test_projection_overlap_using_polygon(A, B) &&
        test_projection_overlap_using_polygon(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::ConvexShape& A, const sf::Sprite& B)
{
    return test_projection_overlap_using_polygon(A, B) &&
        test_projection_overlap_using_sprite(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::Sprite& A, const sf::Vector2f& B)
{
    return check_collision(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::Sprite& A, const sf::RectangleShape& B)
{
    return check_collision(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::Sprite& A, const sf::CircleShape& B)
{
    return check_collision(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::Sprite& A, const sf::ConvexShape& B)
{
    return check_collision(B, A);
}

////////////////////////////////////////////////////////////////////////////////
bool check_collision(const sf::Sprite& A, const sf::Sprite& B)
{
    return test_projection_overlap_using_sprite(A, B) &&
        test_projection_overlap_using_sprite(B, A);
}

} // !sfcollision
