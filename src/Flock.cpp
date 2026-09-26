#include "Flock.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace {
constexpr float epsilon = 0.0001f;
constexpr float pi = 3.14159265358979323846f;

float lengthSquared(Vec2 v) { return v.x * v.x + v.y * v.y; }
float length(Vec2 v) { return std::sqrt(lengthSquared(v)); }

// A zero vector stays zero; never divide by a zero-length vector.
Vec2 normalized(Vec2 v) {
    const float magnitude = length(v);
    return magnitude > epsilon ? v / magnitude : Vec2{};
}

Vec2 limited(Vec2 v, float maximum) {
    const float magnitude = length(v);
    return magnitude > maximum && magnitude > epsilon ? v * (maximum / magnitude) : v;
}
}

Flock::Flock() : random_(std::random_device{}()) { reset(); }

Boid Flock::makeBoid() {
    std::uniform_real_distribution<float> x(90.0f, width - 90.0f);
    std::uniform_real_distribution<float> y(90.0f, height - 90.0f);
    std::uniform_real_distribution<float> angle(0.0f, 2.0f * pi);
    std::uniform_real_distribution<float> speed(45.0f, 95.0f);
    const float direction = angle(random_);
    const float initialSpeed = speed(random_);
    return {{x(random_), y(random_)},
            {std::cos(direction) * initialSpeed, std::sin(direction) * initialSpeed},
            {0.0f, 0.0f}};
}

void Flock::reset(std::size_t count) {
    boids_.clear();
    add(count);
}

void Flock::add(std::size_t count) {
    const std::size_t available = maxCount - boids_.size();
    for (std::size_t i = 0; i < std::min(count, available); ++i) {
        boids_.push_back(makeBoid());
    }
}

void Flock::remove(std::size_t count) {
    const std::size_t removable = boids_.size() > minCount ? boids_.size() - minCount : 0;
    boids_.resize(boids_.size() - std::min(count, removable));
}

Vec2 Flock::steerToward(Vec2 desiredVelocity, Vec2 currentVelocity) const {
    if (lengthSquared(desiredVelocity) <= epsilon * epsilon) return {};
    return limited(normalized(desiredVelocity) * settings_.maxSpeed - currentVelocity,
                   settings_.maxForce);
}

Vec2 Flock::boundaryForce(const Boid& boid) const {
    constexpr float margin = 95.0f;
    Vec2 inward;
    if (boid.position.x < margin) inward.x = 1.0f;
    if (boid.position.x > width - margin) inward.x = -1.0f;
    if (boid.position.y < margin) inward.y = 1.0f;
    if (boid.position.y > height - margin) inward.y = -1.0f;
    return steerToward(inward, boid.velocity) * 1.8f;
}

void Flock::update(float dt) {
    if (dt <= 0.0f) return;

    // Every boid reads the same previous-frame positions and velocities.
    const std::vector<Boid> previous = boids_;
    const float radius2 = settings_.neighborRadius * settings_.neighborRadius;
    const float separationRadius2 = radius2 * 0.25f; // half the neighbor radius

    for (std::size_t i = 0; i < previous.size(); ++i) {
        const Boid& self = previous[i];
        Vec2 separationSum, alignmentSum, cohesionSum;
        int separationCount = 0;
        int neighborCount = 0;

        for (std::size_t j = 0; j < previous.size(); ++j) {
            if (i == j) continue;
            const Vec2 away = self.position - previous[j].position;
            const float distance2 = lengthSquared(away);
            if (distance2 <= epsilon * epsilon || distance2 > radius2) continue;

            // Separation: nearby neighbors push harder than distant ones.
            if (distance2 < separationRadius2) {
                separationSum += away / distance2;
                ++separationCount;
            }
            // Alignment: collect neighbor velocities.
            alignmentSum += previous[j].velocity;
            // Cohesion: collect neighbor positions.
            cohesionSum += previous[j].position;
            ++neighborCount;
        }

        Vec2 acceleration = boundaryForce(self);
        if (settings_.separation && separationCount > 0) {
            acceleration += steerToward(separationSum / static_cast<float>(separationCount),
                                        self.velocity) * settings_.separationWeight;
        }
        if (neighborCount > 0) {
            if (settings_.alignment) {
                acceleration += steerToward(alignmentSum / static_cast<float>(neighborCount),
                                            self.velocity) * settings_.alignmentWeight;
            }
            if (settings_.cohesion) {
                const Vec2 center = cohesionSum / static_cast<float>(neighborCount);
                acceleration += steerToward(center - self.position, self.velocity)
                                * settings_.cohesionWeight;
            }
        }
        boids_[i].acceleration = acceleration;
    }

    // Integrate only after all accelerations have been calculated.
    for (Boid& boid : boids_) {
        boid.velocity = limited(boid.velocity + boid.acceleration * dt, settings_.maxSpeed);
        boid.position += boid.velocity * dt;
        // The inward force normally turns boids in time; this is a final safety stop.
        boid.position.x = std::clamp(boid.position.x, 0.0f, width);
        boid.position.y = std::clamp(boid.position.y, 0.0f, height);
    }
}
