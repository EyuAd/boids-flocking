#pragma once

#include <cstddef>
#include <random>
#include <vector>

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2 operator+(Vec2 other) const { return {x + other.x, y + other.y}; }
    Vec2 operator-(Vec2 other) const { return {x - other.x, y - other.y}; }
    Vec2 operator*(float scale) const { return {x * scale, y * scale}; }
    Vec2 operator/(float scale) const { return {x / scale, y / scale}; }
    Vec2& operator+=(Vec2 other) { x += other.x; y += other.y; return *this; }
};

struct Boid {
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
};

struct FlockSettings {
    float neighborRadius = 68.0f;
    float separationWeight = 1.65f;
    float alignmentWeight = 0.85f;
    float cohesionWeight = 0.70f;
    float maxSpeed = 125.0f;       // pixels per second
    float maxForce = 260.0f;      // pixels per second squared
    bool separation = true;
    bool alignment = true;
    bool cohesion = true;
};

class Flock {
public:
    static constexpr float width = 1000.0f;
    static constexpr float height = 700.0f;
    static constexpr std::size_t minCount = 10;
    static constexpr std::size_t maxCount = 500;

    Flock();
    void reset(std::size_t count = 100);
    void add(std::size_t count);
    void remove(std::size_t count);
    void update(float dt);

    const std::vector<Boid>& boids() const { return boids_; }
    FlockSettings& settings() { return settings_; }
    const FlockSettings& settings() const { return settings_; }

private:
    Boid makeBoid();
    Vec2 steerToward(Vec2 desiredVelocity, Vec2 currentVelocity) const;
    Vec2 boundaryForce(const Boid& boid) const;

    std::vector<Boid> boids_;
    FlockSettings settings_;
    std::mt19937 random_;
};
