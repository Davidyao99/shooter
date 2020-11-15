#include "core/player.h"

namespace shooter {

    Player::Player(glm::vec2 position, float radius, int hit_points) :
            Entity(position, radius, hit_points), last_fire_(std::chrono::system_clock::now())
    {
    }

    void Player::Accelerate(Direction direction) {
        switch (direction) {
            case left:
              velocity_ += glm::vec2(-1,0);
              break;
            case up:
              velocity_ += glm::vec2(0, -1);
              break;
            case right:
              velocity_ += glm::vec2(1,0);
              break;
            case down:
              velocity_ += glm::vec2(0,1);
            break;
        }
    }

    const glm::vec2 Player::get_velocity() const {
      return velocity_;
    }

    void Player::Move() {
        position_ += velocity_;
        velocity_ *= 0.9;
        if (glm::length(velocity_) < 0.1) {
            velocity_ = glm::vec2(0,0);
        }
    }

    bool Player::Shoot() {
        std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_fire_);
        if ( duration.count() > 100) {
            last_fire_ = std::chrono::system_clock::now();
            return true;
        }
        return false;
    }

    void Player::ZeroX() {
      velocity_.x = 0;
    }

    void Player::ZeroY() {
      velocity_.y = 0;
    }

}
