
#include <iostream>
#include <random>
#include <bits/random.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

constexpr u_int16_t PlayerWidth = 16;
constexpr u_int16_t PlayerHeight = 64;
constexpr u_int16_t BallRadius = 16;
constexpr float BallSpeed = 320.0f;
constexpr int MaxScore = 5;
constexpr int WindowWidth = 1280;
constexpr int WindowHeight = 720;
constexpr char FontPath[] = "assets/Minecraft.ttf";


class Score {
public:
    Score() : score(0), maxScore(MaxScore) {
    }

    [[nodiscard]] u_int8_t get() const { return score; }
    [[nodiscard]] bool isMax() const { return score == maxScore; }
    void update() { score++; }
    void reset() { score = 0; }

private:
    u_int8_t score;
    u_int8_t maxScore;
};

class Entity {
public:
    Entity(u_int16_t x, u_int16_t y, u_int16_t width, u_int16_t height)
        : shape(sf::Vector2f(width, height)), x(x), y(y) {
        shape.setPosition(sf::Vector2f(x, y));
    }

    void update(float dt) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && shape.getPosition().y > 0)
            shape.move(0, -BallSpeed * dt * 2);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && shape.getPosition().y + shape.getSize().y < WindowHeight)
            shape.move(0, BallSpeed * dt * 2);
    }

    void update(float dt, const sf::CircleShape &ballShape) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution dis(-1.0f, 1.0f);

        const float ballY = ballShape.getPosition().y + ballShape.getRadius();
        const float enemyY = shape.getPosition().y + shape.getSize().y / 2;

        if (dis(gen) > 0.2f) {
            if (enemyY < ballY && shape.getPosition().y + shape.getSize().y < WindowHeight) {
                shape.move(0, BallSpeed * dt);
            } else if (enemyY > ballY && shape.getPosition().y > 0) {
                shape.move(0, -BallSpeed * dt);
            }
        } else {
            const float randomMove = dis(gen);
            shape.move(0, randomMove * BallSpeed * dt);
        }
    }


    void render(sf::RenderWindow &window) const { window.draw(shape); }
    void reset() { shape.setPosition(x, y); }
    sf::RectangleShape &get() { return shape; }

private:
    sf::RectangleShape shape;
    u_int16_t x;
    u_int16_t y;
};

class Ball {
public:
    Ball(Entity &player, Entity &enemy) : player(player), enemy(enemy), shape(BallRadius), dX(1), dY(1) {
        reset();
        scorePlayer = Score();
        scoreEnemy = Score();
    }

    void update(float dt) {
        if (isColliding(player) || isColliding(enemy)) dX *= -1;
        if (shape.getPosition().y <= 0 || shape.getPosition().y + shape.getRadius() * 2 >= WindowHeight) dY *= -1;
        shape.move(BallSpeed * 2 * dX * dt, BallSpeed * 2 * dY * dt);

        if (shape.getPosition().x < 0) {
            scoreEnemy.update();
            reset();
        }
        if (shape.getPosition().x + shape.getRadius() * 2 > WindowWidth) {
            scorePlayer.update();
            reset();
        }
        if (scorePlayer.isMax() || scoreEnemy.isMax()) {
            player.reset();
            enemy.reset();
        }
    }

    void render(sf::RenderWindow &window) const { window.draw(shape); }

    bool isColliding(Entity &entity) const {
        return shape.getGlobalBounds().intersects(entity.get().getGlobalBounds());
    }

    void reset() {
        shape.setPosition(static_cast<float>(WindowWidth) / 2, static_cast<float>(WindowHeight) / 2);
        std::random_device rd;
        std::mt19937 gen(rd());
        dX = std::uniform_real_distribution(1.0f, 1.0f)(gen);
        dY = std::uniform_real_distribution(1.0f, 1.0f)(gen);
    }

    Score &getScorePlayer() { return scorePlayer; }
    Score &getScoreEnemy() { return scoreEnemy; }

    sf::CircleShape &get() { return shape; }

private:
    Entity &player;
    Entity &enemy;
    Score scorePlayer;
    Score scoreEnemy;
    sf::CircleShape shape;
    float dX;
    float dY;
};

class SceneManager {
public:
    enum class SceneType { MainMenu, Game, GameOver };

    SceneManager(Entity &player, Entity &enemy, Ball &ball, sf::RenderWindow &window) : player(player), enemy(enemy),
        ball(ball), window(window) {
        if (!font.loadFromFile(FontPath)) std::cout << "Error loading font" << std::endl;
    }

    void update(float dt) {
        switch (currentScene) {
            case SceneType::MainMenu:
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                    reset();
                    this->currentScene = SceneType::Game;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
                    window.close();
                break;
            case SceneType::Game:
                player.update(dt);
                enemy.update(dt, ball.get());
                ball.update(dt);

                if (ball.getScoreEnemy().isMax() || ball.getScorePlayer().isMax()) {
                    this->currentScene = SceneType::GameOver;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
                    reset();
                    this->currentScene = SceneType::MainMenu;
                }

                break;
            case SceneType::GameOver:
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                    reset();
                    this->currentScene = SceneType::Game;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
                    reset();
                    this->currentScene = SceneType::MainMenu;
                }

                break;
        }
    }

    void render() {
        switch (currentScene) {
            case SceneType::MainMenu:
                renderMainMenu();
                break;
            case SceneType::Game:
                player.render(window);
                enemy.render(window);
                ball.render(window);
                renderScore();
                break;
            case SceneType::GameOver:
                renderGameOver();
                break;
        }
    }

private:
    SceneType currentScene = SceneType::MainMenu;
    Entity &player;
    Entity &enemy;
    Ball &ball;
    sf::Font font;
    sf::RenderWindow &window;
    int blinkCounter = 1000;

    void reset() const {
        player.reset();
        enemy.reset();
        ball.reset();
        ball.getScoreEnemy().reset();
        ball.getScorePlayer().reset();
    }

    void renderScore() const {
        sf::Text scorePlayerText;
        sf::Text scoreEnemyText;
        scorePlayerText.setFont(font);
        scoreEnemyText.setFont(font);
        scorePlayerText.setString(std::to_string(ball.getScorePlayer().get()));
        scoreEnemyText.setString(std::to_string(ball.getScoreEnemy().get()));
        scorePlayerText.setCharacterSize(64);
        scoreEnemyText.setCharacterSize(64);
        scorePlayerText.setFillColor(sf::Color::White);
        scoreEnemyText.setFillColor(sf::Color::White);

        scoreEnemyText.setPosition(static_cast<float>(WindowWidth) / 2 + 64, 0);
        scorePlayerText.setPosition(static_cast<float>(WindowWidth) / 2 - 64, 0);

        window.draw(scorePlayerText);
        window.draw(scoreEnemyText);
    }

    void renderGameOver() {
        sf::Text gameOverText;
        sf::Text blinkyText;
        gameOverText.setFont(font);
        blinkyText.setFont(font);

        if (ball.getScorePlayer().isMax()) {
            gameOverText.setString("Winner: Player - " + std::to_string(ball.getScorePlayer().get()));
        } else if (ball.getScoreEnemy().isMax()) {
            gameOverText.setString("Winner: Enemy - " + std::to_string(ball.getScoreEnemy().get()));
        }

        gameOverText.setCharacterSize(64);
        gameOverText.setFillColor(sf::Color::White);

        blinkyText.setString("Press Space to restart | Q to quit");
        blinkyText.setCharacterSize(32);
        blinkyText.setFillColor(sf::Color::White);
        blinkyText.setPosition(static_cast<float>(WindowWidth) / 2.0f - 250,
                               static_cast<float>(WindowHeight) / 2.0f + 40);

        sf::FloatRect textRect = gameOverText.getLocalBounds();
        gameOverText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        gameOverText.setPosition(static_cast<float>(WindowWidth) / 2.0f, static_cast<float>(WindowHeight) / 2.0f);

        window.draw(gameOverText);

        blinkCounter++;
        if (blinkCounter < 2000) {
            blinkyText.setFillColor(sf::Color(255 - blinkCounter / 8, 255 - blinkCounter / 8, 255 - blinkCounter / 8));
        } else if (blinkCounter >= 2500) {
            blinkCounter = 1000;
        }

        window.draw(blinkyText);
    }

    void renderMainMenu() {
        sf::Text titleText;
        sf::Text blinkyText;

        titleText.setFont(font);
        blinkyText.setFont(font);

        titleText.setString("Pong!!!");
        titleText.setCharacterSize(64);
        titleText.setFillColor(sf::Color::White);

        blinkyText.setString("Press Space to start | Q to quit");
        blinkyText.setCharacterSize(32);
        blinkyText.setFillColor(sf::Color::White);


        sf::FloatRect textRect = titleText.getLocalBounds();
        titleText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        titleText.setPosition(static_cast<float>(WindowWidth) / 2.0f, static_cast<float>(WindowHeight) / 2.0f);

        blinkyText.setPosition(static_cast<float>(WindowWidth) / 2.0f - 220,
                               static_cast<float>(WindowHeight) / 2.0f + 40);


        window.draw(titleText);


        blinkCounter++;
        if (blinkCounter < 2000) {
            blinkyText.setFillColor(sf::Color(255 - blinkCounter / 8, 255 - blinkCounter / 8, 255 - blinkCounter / 8));
        } else if (blinkCounter >= 2500) {
            blinkCounter = 1000;
        }

        window.draw(blinkyText);
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(WindowWidth, WindowHeight), "Pong!!!",
                            sf::Style::Titlebar | sf::Style::Close);
    Entity player(10, window.getSize().y / 2 - PlayerWidth, PlayerWidth, PlayerHeight);
    Entity enemy(window.getSize().x - PlayerWidth - 10, window.getSize().y / 2 - PlayerWidth, PlayerWidth,
                 PlayerHeight);
    Ball ball(player, enemy);
    SceneManager sceneManager(player, enemy, ball, window);

    sf::Clock clock;
    while (window.isOpen()) {
        const float dt = clock.restart().asSeconds();
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }
        sceneManager.update(dt);
        window.clear();
        sceneManager.render();
        window.display();
    }
    return EXIT_SUCCESS;
}
