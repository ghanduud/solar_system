#include <SFML/Graphics.hpp>
#include <Box2D.h>
#include <cmath>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>

// Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float SCALE = 100.0f;           // Pixels per meter
const float SUN_RADIUS = 50.0f;      // Radius of the Sun
const float PLANET_RADIUS = 10.0f;   // Radius of each planet
const float G = 0.0001f;              // Custom gravitational constant

// Utility function to normalize a vector
sf::Vector2f normalize(const sf::Vector2f& v) {
    float length = std::sqrt(v.x * v.x + v.y * v.y);
    if (length != 0)
        return sf::Vector2f(v.x / length, v.y / length);
    return v;
}

// Class representing the Sun
class Sun {
private:
    sf::CircleShape shape;

public:
    Sun(sf::Vector2f position) {
        shape.setRadius(SUN_RADIUS);
        shape.setOrigin(SUN_RADIUS, SUN_RADIUS);
        shape.setFillColor(sf::Color::Yellow);
        shape.setPosition(position);
    }

    sf::CircleShape& getShape() { return shape; }
    sf::Vector2f getPosition() const { return shape.getPosition(); }
};

// Class representing a Planet
class Planet {
private:
    sf::CircleShape shape;
    b2Body* body;                     // Box2D body for physics
    sf::VertexArray trail;
    sf::Color color;                  // Random color for the planet and its trail

public:
    Planet(b2World& world, sf::Vector2f position, sf::Vector2f sunPosition)
        : trail(sf::LineStrip) {
        shape.setRadius(PLANET_RADIUS);
        shape.setOrigin(PLANET_RADIUS, PLANET_RADIUS);

        // Random color for the planet and trail
        color = sf::Color(std::rand() % 256, std::rand() % 256, std::rand() % 256);
        shape.setFillColor(color);

        // Box2D body definition
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(position.x / SCALE, position.y / SCALE);

        body = world.CreateBody(&bodyDef);

        // Box2D shape and fixture
        b2CircleShape circleShape;
        circleShape.m_radius = PLANET_RADIUS / SCALE;

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &circleShape;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 1.0f; // Elastic collisions
        body->CreateFixture(&fixtureDef);

        // Initial velocity for counterclockwise orbit
        sf::Vector2f direction = position - sunPosition;
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        // Ensure the distance is scaled properly
        distance /= SCALE;

        // Calculate orbital velocity using the formula v = sqrt(GM / r)
        float orbitalSpeed = std::sqrt((G * SUN_RADIUS) / distance) * 15;

        // Create a perpendicular tangential velocity
        sf::Vector2f tangentVelocity(-direction.y, direction.x);
        tangentVelocity = normalize(tangentVelocity) * orbitalSpeed;

        // Apply the velocity, converted to Box2D units
        b2Vec2 velocity(tangentVelocity.x, tangentVelocity.y);
        body->SetLinearVelocity(velocity);
    }

    void updateTrail() {
        trail.append(sf::Vertex(sf::Vector2f(body->GetPosition().x * SCALE, body->GetPosition().y * SCALE), color));
        if (trail.getVertexCount() > 70) {
            for (std::size_t i = 1; i < trail.getVertexCount(); ++i) {
                trail[i - 1] = trail[i];
            }
            trail.resize(70);
        }
    }

    void applyGravity(const sf::Vector2f& sunPosition) {
        sf::Vector2f position(body->GetPosition().x * SCALE, body->GetPosition().y * SCALE);
        sf::Vector2f direction = sunPosition - position;
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        // Scale distance for Box2D units
        float scaledDistance = distance / SCALE;

        // Apply gravity if distance is non-zero
        if (scaledDistance > 0) {
            float forceMagnitude = (G * SUN_RADIUS * PLANET_RADIUS) / (scaledDistance * scaledDistance);
            sf::Vector2f force = normalize(direction) * forceMagnitude;

            // Convert force to Box2D units and apply
            body->ApplyForceToCenter(b2Vec2(force.x, force.y), true);
        }
    }

    void update() {
        shape.setPosition(body->GetPosition().x * SCALE, body->GetPosition().y * SCALE);
        updateTrail();
    }

    bool checkCollision(const sf::Vector2f& sunPosition) const {
        sf::Vector2f position(body->GetPosition().x * SCALE, body->GetPosition().y * SCALE);
        float distance = std::sqrt(
            std::pow(position.x - sunPosition.x, 2) +
            std::pow(position.y - sunPosition.y, 2));
        return distance < SUN_RADIUS;
    }

    void draw(sf::RenderWindow& window) {
        window.draw(trail);
        window.draw(shape);
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Sun and Planets with Box2D");
    window.setFramerateLimit(60);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    Sun sun(sf::Vector2f(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f));

    b2World world(b2Vec2(0.0f, 0.0f)); // No global gravity, custom gravity used
    std::vector<std::unique_ptr<Planet>> planets;

    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePosition(event.mouseButton.x, event.mouseButton.y);
                planets.emplace_back(std::make_unique<Planet>(world, mousePosition, sun.getPosition()));
            }
        }

        float deltaTime = clock.restart().asSeconds() * 4;

        // Apply gravity and update planets
        for (auto it = planets.begin(); it != planets.end();) {
            (*it)->applyGravity(sun.getPosition());
            (*it)->update();

            if ((*it)->checkCollision(sun.getPosition())) {
                it = planets.erase(it);
            }
            else {
                ++it;
            }
        }

        world.Step(deltaTime, 8, 3); // Step the Box2D world

        // Draw everything
        window.clear();
        window.draw(sun.getShape());
        for (auto& planet : planets) {
            planet->draw(window);
        }
        window.display();
    }

    return 0;
}
