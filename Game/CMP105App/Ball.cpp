#include "Ball.h"

Ball::Ball()
{
	// Initialise variables.
	velocity.x = 0;
	velocity.y = 0;

	position.x = 0;
	position.y = 0;

	lagPosition.x = 0;
	lagPosition.y = 0;

	interpolating = false;

	gravity = 9.8;
	drag = 0.5;
	gameScale = 100;
	
	
}

Ball::~Ball()
{
}

void Ball::update(float dt)
{
	// Set velocity along x axis. Slow the ball down to simulate drag.
	velocity.x -= velocity.x * drag * dt;
	
	// Checks if the ball is either on the floor, or on top of each of the goals. If it is, set velocity in y axis to 0 to prevent it from falling through the ground.
	if ((position.y > 575 - (getSize().y / 2) + 1 && velocity.y > -10) || ((position.y > 375 - (getSize().y / 2) + 1) && (position.y < 385 - (getSize().y / 2) + 1) && (position.x < 170 || position.x > 1030) && velocity.y > -10))
	{
		velocity.y = 0;
	}
	else // Otherwise, increase ball's y velocity by gravity.
	{
		velocity.y += gravity * gameScale * dt;
	}

	// Calculate position based on previously calculated velocity.
	position += velocity * dt;
	
	// Interpolation moves the ball in a straight line towards the desired position. Moves at triple the speed so that it catches up with the actual position quickly without looking like the ball has just teleported.
	// Potential to improve this in the future by storing the positions for each step when collision is simulated after receiving the data from other client. Then interpolate between those positions so that it follows the ball's actual path as current method can be janky in high latencies.
	// The ball is told to interpolate whenever the client receives a collision packet.
	if (interpolating)
	{
		// Calculate how close the ball is to the desired position.
		sf::Vector2f posDif = position - lagPosition;

		// Once the ball is close enough to the desired position, stop interpolating.
		if (abs(posDif.x) < 10.0f && abs(posDif.y < 10.0f))
		{
			interpolating = false;
		}

		// Move the ball towards the desired position.
		sf::Vector2f dir = calculateDirection(lagPosition, position);
		lagPosition.x += dir.x * abs(velocity.x) * 3 * dt;
		lagPosition.y += dir.y * abs(velocity.y) * 3 * dt;
	}
	
	// When not interpolating, set the lagged position to be equal to the calculated position.
	if(!interpolating)
	{
		lagPosition = position;
	}

	// Set hitbox to the ball's actual position, not the position that is lagged behind. When lagged position has caught up, the hit box will be at exactly the correct position.
	sf::Vector2f hitboxOffset = position - lagPosition;
	setCollisionBox(sf::FloatRect(-getSize().x / 2 + hitboxOffset.x, -getSize().y / 2 + hitboxOffset.y, getCollisionBox().width, getCollisionBox().height));
	
	// Set the position of the ball. If the ball is interpolating, this will be behind the actual position of the ball.
	setPosition(lagPosition);
	
	// Rotates the ball based on its x velocity to simulate the ball spinning. This has no effect on the ball's trajectory.
	rotate(velocity.x / 100);
}

sf::Vector2f Ball::calculateDirection(sf::Vector2f pos1, sf::Vector2f pos2)
{
	sf::Vector2f output;

	// Difference in positions.
	output = pos2 - pos1;
	
	// Magnitude of the vector.
	float magnitude = sqrt(pow(output.x, 2) + pow(output.y, 2));

	// Convert to unit vector by dividing by magnitude.
	output = output / magnitude;

	return output;
}