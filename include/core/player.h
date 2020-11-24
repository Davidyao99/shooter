#pragma once

#include "cinder/gl/gl.h"
#include "entity.h"
#include "weapon.h"

namespace shooter {

    enum Direction {
        left,
        up,
        right,
        down
    };

    /**
     * Player class
     */

    class Player : public Entity {

    public:
      Player(glm::vec2 position, float radius,
             int hit_points, Weapon weapon);

      /**
       * Override Move method from Entity to simulate friction by
       * multiplying 0.9 to velocity every call
       */
      void Move();

      /**
       * Accelerates in direction
       * @param direction direction to accelerate towards
       */
      void Accelerate(Direction direction);

      const Weapon& Player::get_curr_weapon_() const;

      /**
       * Zero X component of Velocity, to be called when player at right
       * or left boundary
       */
      void ZeroXVelocity();

      void ReloadWeapon();

      /**
       * Zero Y component of velocity, to be called when player at top or
       * bottom boundary
       */
      void ZeroYVelocity();

      /**
       * Get duration in percentage out of 1 sec from last_fire. If more than
       * 1 sec ago, return 1
       * @return
       */
      float GetWeaponReloadStatus() const;

      /**
       * Change weapon
       * @param is_next true for next weapon, false for prev weapon
       */
      void ChangeWeapon(bool is_next);


      /**
       * Adds weapon to weapons
       */
      void AddWeapon(Weapon weapon);

      void ChangeNextWeapon();

      void ChangePrevWeapon();

    private:
        std::chrono::system_clock::time_point last_fire_;
        std::vector<Weapon> weapons_;
        int curr_weapon_index_;

    };
}