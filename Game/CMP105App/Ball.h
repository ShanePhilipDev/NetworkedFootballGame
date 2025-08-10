#pragma once
#include "Framework/GameObject.h"
#include <iostream>
// Ball class - all of the ball's movement and physics is handled here, including interpolation.
class Ball : public GameObject
{
public:
	// Constructor and destructor
	Ball();
	~Ball();

	// Update the ball using delta time, or the physics step time.
	void update(float dt);

	// Setter functions for velocity, actual position, lagged position and whether the ball is interpolating.
	// ----
	void setVelocity(float x, float y)
	{
		velocity.x = x;
		velocity.y = y;
	};

	void setPositionXY(float x, float y)
	{
		position.x = x;
		position.y = y;
	}

	void setLagPosition(float x, float y)
	{
		lagPosition.x = x;
		lagPosition.y = y;
	}

	void setInterpolating(bool b)
	{
		interpolating = b;
	}
	// ----

	// Getter functions for the ball's velocity, drag, gravity, and centre.
	// ----
	sf::Vector2f getVelocity()
	{
		return velocity;
	}

	float getDrag()
	{
		return drag;
	}

	float getGravity()
	{
		return gravity * gameScale;
	}

	sf::Vector2f getCentre()
	{
		return sf::Vector2f(getPosition().x, getPosition().y); // Origin of ball is at centre, so just return position.
	}
	// ----
private:
	// Function to calculate direction between two points. Used for working out the direction that the ball will move in when interpolating.
	sf::Vector2f calculateDirection(sf::Vector2f pos1, sf::Vector2f pos2);

	// Velocity and position vectors.
	sf::Vector2f velocity;
	sf::Vector2f position;
	sf::Vector2f lagPosition;

	// A boolean to hold whether the ball should be interpolating or not. This is set to true in the network manager when it handles received collisions.
	bool interpolating;

	// Physics properties.
	float gravity;
	float drag;
	float gameScale;
};

