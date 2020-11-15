#include "visualizer/shooter_app.h"
#include "cinder/gl/gl.h"
#include <cinder/app/App.h>

namespace shooter {

    namespace visualizer {

        ShooterApp::ShooterApp()
                : engine_(static_cast<float>(kScreenLength * 4),
                          static_cast<float>(kScreenHeight * 4)),
                  moves_(),
                  screen_(glm::ivec2(kMargin, kMargin), kScreenLength, kScreenHeight) {

            ci::app::setWindowSize( kWindowLength, kWindowHeight);

        }

        void ShooterApp::draw() {

          ci::Color8u background_color(255, 246, 148);  // light yellow
          ci::gl::clear(background_color);

          screen_.Draw(engine_.GetPlayerPosition(),
                         engine_.get_enemies_(),
                         engine_.get_bullets_());

        }
        void ShooterApp::update() {
            engine_.update(moves_);
        }

        void ShooterApp::setup(){};

        void ShooterApp::keyDown(ci::app::KeyEvent event) {
            switch (event.getChar()) {
                case ci::app::KeyEvent::KEY_w:
                    moves_.insert(up);
                    break;
                case ci::app::KeyEvent::KEY_s:
                    moves_.insert(down);
                    break;
                case ci::app::KeyEvent::KEY_d:
                    moves_.insert(right);
                    break;
                case ci::app::KeyEvent::KEY_a:
                    moves_.insert(left);
                    break;
            }
        }

        void ShooterApp::keyUp(ci::app::KeyEvent event) {
            switch (event.getChar()) {
                case ci::app::KeyEvent::KEY_w:
                    moves_.erase(up);
                    break;
                case ci::app::KeyEvent::KEY_s:
                    moves_.erase(down);
                    break;
                case ci::app::KeyEvent::KEY_d:
                    moves_.erase(right);
                    break;
                case ci::app::KeyEvent::KEY_a:
                    moves_.erase(left);
                    break;
            }
        }
        void ShooterApp::mouseDown(ci::app::MouseEvent event) {
            glm::vec2 cursor = event.getPos();
            engine_.HandleShoot(cursor);
        }
    }
}
