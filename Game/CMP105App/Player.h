#pragma once
#include "Framework/GameObject.h"
// Player class.
class Player : public GameObject
{
public:
	Player();
	~Player();

	// Player's main functions.
	void handleInput(float dt);
	void update(float dt); // The player's update function is only called for the user's player. The other player will be manipulated using information sent by the other client.
	void render();

	// Functions for the player's actions.
	void kick();
	void jump();

	// Getter and setter functions for the player's properties.
	// ----
	void setKicking(bool k)
	{
		kicking = k;
	}

	void setJumping(bool j)
	{
		jumping = j;
		doubleJumping = j;
	}

	void setFacingRight(bool fr);

	bool getKicking()
	{
		return kicking;
	};

	bool getFacingRight()
	{
		return facingRight;
	}

	sf::Vector2f getCentre()
	{
		return sf::Vector2f(getPosition().x + getSize().x / 2, getPosition().y + getSize().y / 2);
	}
	// ----
private:
	// Booleans for the player's states.
	bool kicking;
	bool jumping;
	bool doubleJumping;
	bool facingRight;

	// Gravity and scale to be used for physics calculations.
	float gravity;
	float gameScale;
	float xSpeed;

	// Duration of player's kicking state, and a timer to track it.
	float kickDuration;
	float kickTimer;
};

