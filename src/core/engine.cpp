#include "core/engine.h"
#include <cstdlib>

namespace shooter {


  Engine::Engine(float length, float height)
      : Engine(length, height, glm::vec2(length / 2, height / 2)) {
  }

  Engine::Engine(float length, float height, glm::vec2 player_position)
      : player_(player_position, 10.0f, 50,
                Weapon(std::string("Pistol"), bullet, 0.3f, 1000,
                       ProjectileBlueprint(10.0f, 1, 10.0f, false))),
        board_dimensions_(length, height),
        enemy_spawns_(),
        begin_time_(std::chrono::system_clock::now()),
        last_enemy_wave_(std::chrono::system_clock::now()),
        explosives_(),
        score_(0){
    CreateWeapons();
    CreateEnemySpawn();
  }

  void Engine::CreateWeapons() {
    Weapon sniper("Sniper", bullet, 0.0f, 1000,
                 ProjectileBlueprint(10.0f, 100, 30.0f, false));
    player_.AddWeapon(sniper);
    Weapon rifle("Rifle", bullet, 0.2f, 400,
                 ProjectileBlueprint(15.0f, 30, 25.0f, false));
    player_.AddWeapon(rifle);
    Weapon laser("Laser", beam, 0.0f, 200,
                 ProjectileBlueprint(5.0f, 10, 0, false));
    player_.AddWeapon(laser);
    Weapon rocket("Rocket", bullet, 0.01f, 1500,
                  ProjectileBlueprint(15.0f, 0, 20.0f, true));
    player_.AddWeapon(rocket);
  }

  void Engine::CreateEnemySpawn() {
    int x_unit = static_cast<int>(board_dimensions_.x) / 20;
    int y_unit = static_cast<int>(board_dimensions_.y) / 20;
    for (size_t x_coord = 0; x_coord != board_dimensions_.x + x_unit;
         x_coord += x_unit) {
      enemy_spawns_.emplace_back(x_coord, 0);
      enemy_spawns_.emplace_back(x_coord, board_dimensions_.y);
    }
    for (size_t y_coord = y_unit; y_coord != board_dimensions_.y;
         y_coord += y_unit) {
      enemy_spawns_.emplace_back(0, y_coord);
      enemy_spawns_.emplace_back(board_dimensions_.x, y_coord);
    }
  }

  const std::vector<glm::vec2>& Engine::get_enemy_spawns_() const {
    return enemy_spawns_;
  }

  void Engine::update(std::set<Direction> moves) {
    glm::vec2 player_pos = player_.get_position_();
    for (Direction direction : moves) {
      player_.Accelerate(direction);
    }
    HandlePlayerAtBoundary();
    player_.Move();
    for (Bullet& bullet : bullets_) {
      bullet.Move();
    }
    for (Enemy& enemy : enemies_) {
      enemy.Accelerate(player_pos);
      enemy.Move();
    }
    HandleCollisions();
    SpawnEnemy();
    HandleDeaths();
  }

  void Engine::HandleDeaths() {
    for (size_t bullet_idx = bullets_.size() - 1; bullet_idx < bullets_.size();
         bullet_idx--) {

      if (bullets_.at(bullet_idx).IsDead()) {
        if (bullets_.at(bullet_idx).get_is_explosive_()) { // explode upon death
          Explode(bullets_.at(bullet_idx).get_position_());
          explosives_.push_back(bullets_.at(bullet_idx).get_position_());
        }
        bullets_.erase(bullets_.begin() + bullet_idx);
      }
    }
    for (size_t enemy_idx = enemies_.size() - 1; enemy_idx < enemies_.size();
         enemy_idx--) {
      if (enemies_.at(enemy_idx).IsDead()) {
        enemies_.erase(enemies_.begin() + enemy_idx);
        score_ += 10;
      }
    }
    if (player_.IsDead()) {
      exit(0);
    }
  }

  ProjectileType Engine::HandleShoot(glm::vec2 cursor) {
    const Weapon& weapon = player_.GetCurrentWeapon();
    ProjectileType type = weapon.get_projectile_type_();
    if (type != beam) {
      Bullet bullet = player_.FireBullet(cursor);
      AddBullet(bullet);
    } else {
      ShootBeam(cursor, weapon.get_projectile_blueprint_());
    }
    player_.ReloadWeapon();  // reset reload timing
    return type;
  }

  void Engine::ShootBeam(glm::vec2 cursor,
                         ProjectileBlueprint projectile_blueprint) {
    glm::vec2 laser_unit_vector = cursor / glm::length(cursor);
    for (auto& enemy : enemies_) {
      glm::vec2 player_to_enemy = enemy.get_position_() - player_.get_position_();
      float player_to_enemy_dist = glm::length(player_to_enemy);
      float dot_product = glm::dot(player_to_enemy, laser_unit_vector);
      if (dot_product <= 0) {  // ensure beam does not shoot backwards
        continue;
      }
      float perp_dist = sqrt(pow(player_to_enemy_dist, 2) - pow(dot_product, 2));
      if (perp_dist <= enemy.get_radius_() + projectile_blueprint.radius_) {
        // push enemy away from source of laser
        enemy.Hit(projectile_blueprint.damage_,
                  enemy.get_position_() - laser_unit_vector);
      }
    }
  };

  bool Engine::Reloaded() const {
    return player_.GetWeaponReloadStatus() == 1.0f;
  }

  void Engine::AddBullet(Bullet bullet) {
    bullets_.emplace_back(bullet);
  }

  void Engine::AddEnemy(glm::vec2 position, float radius, int health, int damage,
                        float level) {
    enemies_.emplace_back(position, radius, health, damage, level);
  }

  void Engine::ChangeWeapon(bool next) {
    if (next) {
      player_.ChangeNextWeapon();
    } else {
      player_.ChangePrevWeapon();
    }
  }

  void Engine::SpawnEnemy() {
    std::chrono::milliseconds duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - begin_time_);
    std::chrono::milliseconds time_since_last_wave =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - last_enemy_wave_);
    if (time_since_last_wave.count() > 5000) { // spawns enemy every 5 secs
      // spawns an additional enemy per spawn every 30 seconds
      size_t num_enemy_spawn = 1 + static_cast<size_t>(duration.count()) / 30000;
      // difficult increases every 20 seconds ( higher health, faster speed for enemy)
      size_t difficulty = 1 + static_cast<size_t>(duration.count()) / 20000;
      if (difficulty > 5) {
        difficulty = 5; // max difficulty at 5
      }
      for (num_enemy_spawn; num_enemy_spawn != 0; num_enemy_spawn--) {
        size_t index = (rand() % enemy_spawns_.size());
        size_t health = static_cast<int>(difficulty * (static_cast<float>(rand()) / RAND_MAX) * 20.0f);
        float level = 0.1f + difficulty * (static_cast<float>(rand()) / RAND_MAX) * 0.18f;
        AddEnemy(enemy_spawns_[index], 10.0f,
                 health, 10, level);
        std::cout<<level<<"---------------"<<std::endl;
      }
      last_enemy_wave_ = std::chrono::system_clock::now();
    }
  }

  const std::vector<Bullet>& Engine::get_bullets_() const {
    return bullets_;
  }

  const std::vector<Enemy>& Engine::get_enemies_() const {
    return enemies_;
  }

  const glm::ivec2& Engine::get_board_dimensions_() const {
    return board_dimensions_;
  }

  int Engine::get_score_() const {
    return score_;
  }

  void Engine::HandlePlayerAtBoundary() {
    const glm::vec2& position = player_.get_position_();
    const glm::vec2& velocity = player_.get_velocity_();
    float radius = player_.get_radius_();
    if ((position.y - radius < 0 && velocity.y < 0) ||
        (position.y + radius > board_dimensions_.y && velocity.y > 0)) {
      player_.ZeroYVelocity();
    }

    if ((position.x - radius < 0 && velocity.x < 0) ||
        (position.x + radius > board_dimensions_.x && velocity.x > 0)) {
      player_.ZeroXVelocity();
    }
  }

  const std::vector<glm::vec2> Engine::get_explosives_() const {
    return explosives_;
  }

  void Engine::ClearExplosions() {
    explosives_.clear();
  }

  void Engine::HandleCollisions() {
    HandleEnemyBulletCollision();
    HandleEnemyPlayerCollision();
  }

  void Engine::Explode(glm::vec2 explosion_position) {
    for (auto& enemy : enemies_) {
      float dist = glm::length(enemy.get_position_() - explosion_position);
      if (dist <= 50 + enemy.get_radius_()) {
        enemy.Hit(40, explosion_position);
      }
    }
    float dist = glm::length(player_.get_position_() - explosion_position);
    if (dist <= 50 + player_.get_radius_()) {
      player_.Hit(10, explosion_position);
    }
  }

  void Engine::HandleEnemyBulletCollision() {
    for (auto& bullet : bullets_) {
      for (auto& enemy : enemies_) {
        float dist = glm::length(bullet.get_position_() - enemy.get_position_());
        if (dist <= bullet.get_radius_() + enemy.get_radius_()) {
          enemy.Collide(bullet);
          break;
        }
      }
    }
  }

  const Player& Engine::get_player_() const {
    return player_;
  }

  void Engine::HandleEnemyPlayerCollision() {
    for (auto& enemy : enemies_) {
      float dist = glm::length(player_.get_position_() - enemy.get_position_());
      if (dist <= player_.get_radius_() + enemy.get_radius_()) {
        player_.Collide(enemy);
      }
    }
  }
} // namespace shooter