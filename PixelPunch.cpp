#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <cmath>
#include <sstream>

using namespace sf;
using namespace std;
enum GameState { MENU, INSTRUCTIONS, GAME, EXIT };
GameState currentState = MENU;

const string PLAYER1NAME = "Rayyan";
const string PLAYER2NAME = "Maaz";

const int totalPunchFrames = 11;
const int totalFrames = 8;
const float groundY = 490.0f;
const float gravity = 0.002f;
const float jumpVelocity = -1.0f;
const float window_Width = 800, window_Height = 600;

class Fighter {
private:
    string fighterName;
    Texture walkTextures[totalFrames];
    Texture punchTextures[totalPunchFrames];
    Texture idleTexture;
    Sprite player;
    Clock animationClock;
    Clock animationPunchClock;
    Vector2f movement;
    float verticalVelocity;
    float speed;
    float deadzone;
    float frameDuration;
    float framePunchDuration;
    int currentFrame;
    int currentPunchFrame;
    bool isJumping;
    bool isPunching;
    bool isMoving;
    bool hasHit;

public:
    int health;

    Fighter(string str, float xposi) {
        fighterName = str;
        for (int i = 0; i < totalFrames; ++i)
            walkTextures[i].loadFromFile("assets/" + fighterName + "_walk_" + to_string(i + 1) + ".png");
        for (int i = 0; i < totalPunchFrames; ++i)
            punchTextures[i].loadFromFile("assets/" + fighterName + "_punch_" + to_string(i + 1) + ".png");
        idleTexture.loadFromFile("assets/" + fighterName + ".png");

        player.setTexture(idleTexture);
        player.setPosition(xposi, groundY);
        player.setScale(0.5f, 0.5f);

        FloatRect bounds = player.getLocalBounds();
        player.setOrigin(bounds.width / 2, bounds.height / 2);

        movement = Vector2f(0, 0);
        verticalVelocity = 0.0f;
        speed = 0.3f;
        deadzone = 15.0f;
        frameDuration = 0.1f;
        framePunchDuration = 0.02f;
        currentFrame = 0;
        currentPunchFrame = 0;
        isJumping = false;
        isPunching = false;
        isMoving = false;
        hasHit = false;
        health = 300;
    }

    void applyGravity() {
        verticalVelocity += gravity;
        player.move(0, verticalVelocity);
        if (player.getPosition().y >= groundY) {
            player.setPosition(player.getPosition().x, groundY);
            verticalVelocity = 0.0f;
            isJumping = false;
        }
    }

    void applyMovement(bool isJoystick) {
        movement.x = 0;

        if (isJoystick && Joystick::isConnected(0)) {
            if (Joystick::isButtonPressed(0, 0) && !isPunching) {
                isPunching = true;
                currentPunchFrame = 0;
                animationPunchClock.restart();
                hasHit = false;
            }

            float x = Joystick::getAxisPosition(0, Joystick::X);
            float y = Joystick::getAxisPosition(0, Joystick::Y);

            if (!isPunching) {
                if (y < -deadzone && !isJumping) {
                    verticalVelocity = jumpVelocity;
                    isJumping = true;
                }
                if (abs(x) > deadzone)
                    movement.x = x / 100.0f * speed;
            }
        }
        else if (!isJoystick) {
            if (Keyboard::isKeyPressed(Keyboard::J) && !isPunching) {
                isPunching = true;
                currentPunchFrame = 0;
                animationPunchClock.restart();
                hasHit = false;
            }

            if (!isPunching) {
                if (Keyboard::isKeyPressed(Keyboard::W) && !isJumping) {
                    verticalVelocity = jumpVelocity;
                    isJumping = true;
                }
                if (Keyboard::isKeyPressed(Keyboard::A)) movement.x -= speed;
                if (Keyboard::isKeyPressed(Keyboard::D)) movement.x += speed;
            }
        }

        isMoving = (abs(movement.x) > 0);
        player.move(movement);

        if (movement.x < 0) player.setScale(-0.5f, 0.5f);
        if (movement.x > 0) player.setScale(0.5f, 0.5f);

        if (isPunching) {
            if (animationPunchClock.getElapsedTime().asSeconds() > framePunchDuration) {
                player.setTexture(punchTextures[currentPunchFrame]);
                currentPunchFrame++;
                animationPunchClock.restart();
                if (currentPunchFrame >= totalPunchFrames) {
                    isPunching = false;
                    currentPunchFrame = 0;
                    player.setTexture(idleTexture);
                }
            }
        }
        else if (isMoving && !isJumping) {
            if (animationClock.getElapsedTime().asSeconds() > frameDuration) {
                currentFrame = (currentFrame + 1) % totalFrames;
                player.setTexture(walkTextures[currentFrame]);
                animationClock.restart();
            }
        }
        else {
            if (!isJumping && !isPunching)
                player.setTexture(idleTexture);
        }

        Vector2f pos = player.getPosition();
        FloatRect bounds = player.getGlobalBounds();
        float halfWidth = bounds.width / 2.0f;
        if (pos.x - halfWidth < 0) player.setPosition(halfWidth, pos.y);
        if (pos.x + halfWidth > window_Width) player.setPosition(window_Width - halfWidth, pos.y);
    }

    FloatRect getPunchBox() {
        // Punch box is placed in front of the fighter
        FloatRect self = player.getGlobalBounds();
        float punchOffset = 40.0f;
        float punchWidth = 40.0f;
        float punchHeight = self.height * 0.15f;
        float punchTop = self.top + self.height * 0.25f;

        if (player.getScale().x > 0) // facing right
            return FloatRect(self.left + self.width - 50.0f, punchTop, punchWidth, punchHeight);
        else
            return FloatRect(self.left - punchWidth + 50.0f, punchTop, punchWidth, punchHeight);
    }

    FloatRect getHitBox() {
        // Hitbox is a smaller central portion of the player
        FloatRect self = player.getGlobalBounds();
        float hitWidth = self.width * 0.35f;
        float hitHeight = self.height * 0.6f;
        float hitLeft = self.left + (self.width - hitWidth) / 2.0f;
        float hitTop = self.top + (self.height - hitHeight) / 2.0f;
        return FloatRect(hitLeft, hitTop, hitWidth, hitHeight);
    }

    void checkPunchHit(Fighter& opponent) {
        if (!isPunching || hasHit) return;

        if (getPunchBox().intersects(opponent.getHitBox())) {
            opponent.health -= 10;
            hasHit = true;
        }
    }

    void draw(RenderWindow& window) {
        window.draw(player);

        // Debug: Draw punch box (red)
        FloatRect pb = getPunchBox();
        RectangleShape punchBoxShape(Vector2f(pb.width, pb.height));
        punchBoxShape.setPosition(pb.left, pb.top);
        punchBoxShape.setFillColor(Color(255, 0, 0, 100));
        //window.draw(punchBoxShape);

        // Debug: Draw hit box (green)
        FloatRect hb = getHitBox();
        RectangleShape hitBoxShape(Vector2f(hb.width, hb.height));
        hitBoxShape.setPosition(hb.left, hb.top);
        hitBoxShape.setFillColor(Color(0, 255, 0, 100));
        //window.draw(hitBoxShape);
    }

    void drawHealthBar(RenderWindow& window, bool leftSide) {
        float maxHealth = 300.0f;
        Vector2f pos = leftSide ? Vector2f(20, 20) : Vector2f(window_Width - 320, 20);

        RectangleShape border(Vector2f(300, 20));
        border.setPosition(pos);
        border.setFillColor(Color::Transparent);
        border.setOutlineColor(Color::White);
        border.setOutlineThickness(2);

        RectangleShape bar(Vector2f(max(0.f, (float)health), 20));
        bar.setPosition(pos);
        bar.setFillColor(Color::Green);

        window.draw(border);
        window.draw(bar);
    }
};

bool isTextClicked(Text& text, Vector2f mousePos) {
    return text.getGlobalBounds().contains(mousePos);
}

int main() {
    RenderWindow window(VideoMode(window_Width, window_Height), "SFML Fighting Game");

    Texture backgroundTexture;
    backgroundTexture.loadFromFile("assets/Background.jpg");
    Sprite background(backgroundTexture);

    Font font;
    font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    Fighter player1(PLAYER1NAME, 100);
    Fighter player2(PLAYER2NAME, 400);

    Clock gameClock;
    const int startTime = 180;

    //winning condition
    bool gameOver = false;
    string winnerText = "";
    Color winnerColor = Color::White;


    // Menu Texts
    Text startText("Start Game", font, 36);
    startText.setPosition(270, 180);
    startText.setFillColor(Color::White);

    Text instructionsText("Instructions", font, 36);
    instructionsText.setPosition(270, 250);
    instructionsText.setFillColor(Color::White);

    Text exitText("Exit", font, 36);
    exitText.setPosition(270, 320);
    exitText.setFillColor(Color::White);

    Text backText("Back", font, 24);
    backText.setPosition(20, 550);
    backText.setFillColor(Color::Yellow);

    Text instructionDetails("W / A / D = Move, J = Punch\nJoystick = Left stick + A / square button\nHit opponent to reduce health.", font, 24);
    instructionDetails.setPosition(50, 150);
    instructionDetails.setFillColor(Color::White);

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
                if (currentState == GAME) {
                    currentState = MENU;
                    gameOver = false;
                    winnerText = "";
                }
            }
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                if (currentState == MENU) { 
                    if (isTextClicked(startText, mousePos)) {
                        currentState = GAME;
                        gameClock.restart();
                        player1 = Fighter(PLAYER1NAME, 100);
                        player2 = Fighter(PLAYER2NAME, 400);
                    }
                    else if (isTextClicked(instructionsText, mousePos)) {
                        currentState = INSTRUCTIONS;
                    }
                    else if (isTextClicked(exitText, mousePos)) {
                        window.close();
                    }
                }
                else if (currentState == INSTRUCTIONS) {
                    if (isTextClicked(backText, mousePos)) {
                        currentState = MENU;
                    }
                }
            }
        }

        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

        // Reset all to white
        startText.setFillColor(Color::White);
        instructionsText.setFillColor(Color::White);
        exitText.setFillColor(Color::White);
        backText.setFillColor(Color::Yellow);  // Back is yellow by default

        // Highlight hovered item
        if (currentState == MENU) {
            if (isTextClicked(startText, mousePos)) startText.setFillColor(Color::Yellow);
            if (isTextClicked(instructionsText, mousePos)) instructionsText.setFillColor(Color::Yellow);
            if (isTextClicked(exitText, mousePos)) exitText.setFillColor(Color::Yellow);
        }
        else if (currentState == INSTRUCTIONS) {
            if (isTextClicked(backText, mousePos)) backText.setFillColor(Color::White);
        }

        window.clear();

        if (currentState == MENU) {
            window.draw(background);
            window.draw(startText);
            window.draw(instructionsText);
            window.draw(exitText);
        }

        else if (currentState == INSTRUCTIONS) {
            window.draw(background);
            window.draw(instructionDetails);
            window.draw(backText);
        }

        else if (currentState == GAME) {
            player1.applyMovement(true);
            player2.applyMovement(false);
            player1.applyGravity();
            player2.applyGravity();
            player1.checkPunchHit(player2);
            player2.checkPunchHit(player1);

            int timeLeft = max(0, startTime - (int)gameClock.getElapsedTime().asSeconds());
            stringstream ss;
            ss << "Time: " << timeLeft;
            Text timerText(ss.str(), font, 24);
            timerText.setFillColor(Color::White);
            timerText.setPosition(window_Width / 2 - 50, 20);

            if (!gameOver) {
                if (player1.health <= 0) {
                    winnerText = "Player 2 Wins!";
                    winnerColor = Color::Green;
                    gameOver = true;
                }
                else if (player2.health <= 0) {
                    winnerText = "Player 1 Wins!";
                    winnerColor = Color::Green;
                    gameOver = true;
                }
                else if (timeLeft <= 0) {
                    if (player1.health > player2.health) {
                        winnerText = "Time Up! Player 1 Wins!";
                        winnerColor = Color::Green;
                    }
                    else if (player2.health > player1.health) {
                        winnerText = "Time Up! Player 2 Wins!";
                        winnerColor = Color::Green;
                    }
                    else {
                        winnerText = "It's a Draw!";
                        winnerColor = Color::Red;
                    }
                    gameOver = true;
                }
            }

            window.draw(background);
            player1.draw(window);
            player2.draw(window);
            player1.drawHealthBar(window, true);
            player2.drawHealthBar(window, false);
            window.draw(timerText);
            // winner logic
            if (gameOver) {
                Text resultText(winnerText, font, 48);
                resultText.setFillColor(winnerColor);
                FloatRect bounds = resultText.getLocalBounds();
                resultText.setOrigin(bounds.width / 2, bounds.height / 2);
                resultText.setPosition(window_Width / 2, window_Height / 2);
                window.draw(resultText);
            }
        }

        window.display();
    }

    return 0;
}

