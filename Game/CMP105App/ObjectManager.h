#pragma once
#include <iostream>
#include "Player.h"
#include "Ball.h"
#include "Framework/GameObject.h"
#include "Framework/Collision.h"
#include "NetworkManager.h"
#include "Lobby.h"

class NetworkManager;
class Lobby;

// Object manager class. This is where the gameplay is controlled. This class controls both players and the ball, and their interactions.
// Updates are sent to the network manager when certain events occur, such as collisions or goals being scored.
class ObjectManager
{
public:
	ObjectManager();
	~ObjectManager();

	// Main functions.
	void handleInput(float dt);
	void update(float dt);
	void render();

	// Initialise pointers and variables that rely on the pointers.
	void init(sf::RenderWindow* hwnd, Input* input, NetworkManager* nm, GameState* gs, Lobby* l, AudioManager* a);

	// Functions for checking collisions.
	void checkPlayerCollision(Player* player);
	void checkBallCollision();
	void checkPlayerBallCollision();

	// Function for setting up the player pointers.
	void setupPlayers();

	// Reset functions.
	void goalReset();
	void resetBallPosition();

	// Function called when starting the game.
	void start();

	// Setter functions.
	// ----
	void setTime(float time)
	{
		gameTimer = time;
	}

	void setGoalScored(bool b)
	{
		goalScored = b;
	}

	void setResetTimer(float t)
	{
		resetTimer = t;
	}
	// ----

	// Getter functions
	// ----
	float getTime()
	{
		return gameTimer;
	}

	Ball* getBall()
	{
		return &ball;
	}

	GameObject* getLeftWall()
	{
		return &leftWall;
	}

	GameObject* getRightWall()
	{
		return &rightWall;
	}

	GameObject* getFloor()
	{
		return &floor;
	}

	GameObject* getCeiling()
	{
		return &ceiling;
	}

	float getPhysicsStep()
	{
		return physicsStep;
	}
	// ----
	
	// Functions for increasing score
	// ----
	void increaseLeftScore()
	{
		leftScore++;
	}

	void increaseRightScore()
	{
		rightScore++;
	}
	// ----
	
	// Function to calculate direction between two points.
	sf::Vector2f calculateDirection(sf::Vector2f pos1, sf::Vector2f pos2);
private:
	// Pointers to objects needed in the class.
	sf::RenderWindow* window;
	Input* input;
	NetworkManager* networkManager;
	GameState* gameState;
	Lobby* lobby;
	AudioManager* audio;

	// Font and text objects.
	sf::Font font;
	sf::Text ping;
	sf::Text score;
	sf::Text time;
	sf::Text goal;

	// Pointer to the player that is being controlled by the user.
	Player* controlledPlayer;

	// Player objects.
	Player leftPlayer;
	Player rightPlayer;

	// Collidable map boundaries and goalposts.
	GameObject floor;
	GameObject ceiling;
	GameObject leftGoal;
	GameObject rightGoal;
	GameObject leftWall;
	GameObject rightWall;

	sf::RectangleShape ballBox;
	// Ball object and texture.
	Ball ball;
	sf::Texture ballSprite;

	// Length of the match and a timer to keep track of game time.
	float gameLength;
	float gameTimer;
	
	// Integers for saving the score.
	int leftScore;
	int rightScore;

	// How often to perform physics calculations and a timer to keep track of this.
	float physicsStep;
	float physicsTimer;
	
	// Boolean for whether a goal has been scored and the variables for resetting the map afterwards.
	bool goalScored;
	float resetLength;
	float resetTimer;

	// Textures for each character and the goalposts.
	sf::Texture messi;
	sf::Texture ronaldo;
	sf::Texture kirby;
	sf::Texture miedema;
	sf::Texture goalSprite;
};

