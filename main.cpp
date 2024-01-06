#include <iostream>
#include <random>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

uint16_t PlayerWidth = 16, PlayerHeight = 64, BallRadius = 16, WindowWidth = 1280, WindowHeight = 720, BallSpeed = 320.0f;
uint8_t PlayerScore = 0, EnemyScore = 0, MaxScore = 5;
char FontPath[] = "assets/Minecraft.ttf";

enum class SceneType { MainMenu, Game, GameOver };

class Entity {
public:
    Entity(uint16_t x, uint16_t y, uint16_t width, const uint16_t height)
        : shape(sf::Vector2f(width, height)), x(x), y(y) {
        shape.setPosition(sf::Vector2f(x, y));
    }

    void updatePlayer(const float dt) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && shape.getPosition().y > 0)
            shape.move(0, static_cast<float>(-BallSpeed) * dt * 2);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && shape.getPosition().y + shape.getSize().y < static_cast<float>(WindowHeight))
            shape.move(0, static_cast<float>(BallSpeed) * dt * 2);
    }

    void updateBot(const float dt, const sf::CircleShape &ballShape) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

        const float ballY = ballShape.getPosition().y + ballShape.getRadius();
        const float enemyY = shape.getPosition().y + shape.getSize().y / 2;

        if (dis(gen) > 0.2f) {
            if (enemyY < ballY && shape.getPosition().y + shape.getSize().y < static_cast<float>(WindowHeight)) {
                shape.move(0, static_cast<float>(BallSpeed) * dt);
            } else if (enemyY > ballY && shape.getPosition().y > 0) {
                shape.move(0, static_cast<float>(-BallSpeed) * dt);
            }
        } else {
            const float randomMove = dis(gen);
            shape.move(0, randomMove * static_cast<float>(BallSpeed) * dt);
        }
    }

    void reset() { shape.setPosition(x, y); }
    sf::RectangleShape &getShape() { return shape; }

private:
    sf::RectangleShape shape;
    uint16_t x, y;
};

class Ball {
public:
    Ball(Entity &player, Entity &enemy) : player(player), enemy(enemy), shape(BallRadius), dX(1), dY(1) {
        shape.setPosition(static_cast<float>(WindowWidth) / 2, static_cast<float>(WindowHeight) / 2);
    }

    void update(const float dt) {
        if (isColliding(player) || isColliding(enemy)) dX *= -1;
        if (shape.getPosition().y <= 0 || shape.getPosition().y + shape.getRadius() * 2 >= static_cast<float>(
                WindowHeight))
            dY *= -1;
        shape.move(static_cast<float>(BallSpeed) * 2 * dX * dt, static_cast<float>(BallSpeed) * 2 * dY * dt);

        if (shape.getPosition().x < 0) {
            EnemyScore++;
            reset();
        }
        if (shape.getPosition().x + shape.getRadius() * 2 > static_cast<float>(WindowWidth)) {
            PlayerScore++;
            reset();
        }
        if (PlayerScore == MaxScore || EnemyScore == MaxScore) {
            player.reset();
            enemy.reset();
        }
    }

    bool isColliding(Entity &entity) const {
        return shape.getGlobalBounds().intersects(entity.getShape().getGlobalBounds());
    }

    void reset() {
        shape.setPosition(static_cast<float>(WindowWidth) / 2, static_cast<float>(WindowHeight) / 2);
    }

    sf::CircleShape &getShape() { return shape; }

private:
    Entity &player, &enemy;
    sf::CircleShape shape;
    float dX, dY;
};

void drawText(const sf::Font &font, uint8_t size, const std::string &textToDisplay, float x, float y,
              sf::RenderWindow &window) {
    sf::Text text(textToDisplay, font, size);
    text.setFillColor(sf::Color::White);
    text.setPosition(x - text.getGlobalBounds().width / 2.0f, y - text.getGlobalBounds().height / 2.0f);
    window.draw(text);
}

void renderMainMenu(sf::RenderWindow &window, const sf::Font &font) {
    drawText(font, 64, "Pong!!!", static_cast<float>(WindowWidth) / 2.0f, static_cast<float>(WindowHeight) / 2.0f,
             window);
    drawText(font, 32, "Press Space to start | Q to quit", static_cast<float>(WindowWidth) / 2.0f,
             static_cast<float>(WindowHeight) / 2.0f + 60, window);
}

void renderGame(sf::RenderWindow &window, Entity &player, Entity &enemy, Ball &ball, const sf::Font &font) {
    drawText(font, 64, std::to_string(PlayerScore), static_cast<float>(WindowWidth) / 2.0f - 100, 10, window);
    drawText(font, 64, std::to_string(EnemyScore), static_cast<float>(WindowWidth) / 2.0f + 100, 10, window);
    window.draw(player.getShape());
    window.draw(enemy.getShape());
    window.draw(ball.getShape());
}

void renderGameOver(sf::RenderWindow &window, const sf::Font &font) {
    std::string winnerText;
    if (PlayerScore == MaxScore) {
        winnerText = "Winner: Player - " + std::to_string(PlayerScore);
    } else if (EnemyScore == MaxScore) {
        winnerText = "Winner: Enemy - " + std::to_string(EnemyScore);
    }

    drawText(font, 64, winnerText, static_cast<float>(WindowWidth) / 2.0f, static_cast<float>(WindowHeight) / 2.0f,
             window);
    drawText(font, 32, "Press Space to restart | Q to quit", static_cast<float>(WindowWidth) / 2.0f,
             static_cast<float>(WindowHeight) / 2.0f + 60, window);
}

void reset(Entity &player, Entity &enemy, Ball &ball) {
    PlayerScore = 0;
    EnemyScore = 0;
    player.reset();
    enemy.reset();
    ball.reset();
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WindowWidth, WindowHeight), "Pong!!!",
                            sf::Style::Titlebar | sf::Style::Close);
    Entity player(10, WindowHeight / 2 - PlayerWidth, PlayerWidth, PlayerHeight);
    Entity enemy(WindowWidth - PlayerWidth - 10, WindowHeight / 2 - PlayerWidth, PlayerWidth, PlayerHeight);
    Ball ball(player, enemy);
    sf::Font font;
    if (!font.loadFromFile(FontPath)) std::cout << "Error loading font" << std::endl;

    sf::Clock clock;
    SceneType currentScene = SceneType::MainMenu;
    bool resetGame = false;

    while (window.isOpen()) {
        const float dt = clock.restart().asSeconds();
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        if ((PlayerScore == MaxScore || EnemyScore == MaxScore) && !resetGame) {
            currentScene = SceneType::GameOver;
            resetGame = true;
        }

        window.clear();
        switch (currentScene) {
            case SceneType::MainMenu:
                reset(player, enemy, ball);
                renderMainMenu(window, font);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                    currentScene = SceneType::Game;
                    resetGame = true;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) window.close();
                break;
            case SceneType::Game:
                renderGame(window, player, enemy, ball, font);
                enemy.updateBot(dt, ball.getShape());
                player.updatePlayer(dt);
                ball.update(dt);
                break;
            case SceneType::GameOver:
                renderGameOver(window, font);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                    currentScene = SceneType::Game;
                    reset(player, enemy, ball);
                    resetGame = true;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
                    currentScene = SceneType::MainMenu;
                    reset(player, enemy, ball);
                    resetGame = true;
                }
                break;
        }
        window.display();
    }
    return EXIT_SUCCESS;
}