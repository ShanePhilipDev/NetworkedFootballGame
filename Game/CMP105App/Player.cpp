#include "Player.h"

Player::Player()
{
	// Initialise player states.
	jumping = false;
	doubleJumping = false;
	kicking = false;
	facingRight = true;

	// Initialise gravity and scale to be used with it.
	gameScale = 100;
	gravity = 9.8;

	// Player's speed on x axis is 400.
	xSpeed = 400;

	// Kick duration is 0.5 seconds.
	kickDuration = 0.5f;
	kickTimer = 0.0f;
}

Player::~Player()
{
}

void Player::handleInput(float dt)
{
	// If pressing A, move left. If pressing D, move right. If pressing neither. Don't move in x axis.
	if (input->isKeyDown(sf::Keyboard::A))
	{
		velocity.x = -xSpeed;
	}
	else if (input->isKeyDown(sf::Keyboard::D))
	{
		velocity.x = xSpeed;
	}
	else
	{
		velocity.x = 0;
	}

	// If W or Space key is pressed, the player will jump.
	if (input->isKeyDown(sf::Keyboard::W) || input->isKeyDown(sf::Keyboard::Space))
	{
		input->setKeyUp(sf::Keyboard::W);
		input->setKeyUp(sf::Keyboard::Space);

		jump();
	}

	// If F is pressed or the left mouse button is clicked, kick if not already doing so.
	if (input->isKeyDown(sf::Keyboard::F) || input->isMouseLDown())
	{
		input->setKeyUp(sf::Keyboard::F);
		input->setMouseLDown(false);

		if (!kicking)
		{
			kick();
		}
	}
}

void Player::update(float dt)
{
	// Set direction based on velocity in x axis.
	if (velocity.x > 0)
	{
		facingRight = true;
	}
	else if (velocity.x < 0)
	{
		facingRight = false;
	}

	// If moving up, the player is jumping.
	if (velocity.y > 0)
	{
		jumping = true;
	}

	// If the player is kicking, increase the timer until it exceeds the kick's duration then reset.
	if (kicking)
	{
		kickTimer += dt;
		if (kickTimer > kickDuration)
		{
			kickTimer = 0;
			kicking = false;
		}
	}

	// Right facing texture rectangle.
	setTextureRect(sf::IntRect(0, 0, getTexture()->getSize().x, getTexture()->getSize().y));

	// Flip the texture to face left if the player is facing left.
	if (!facingRight) 
	{
		sf::IntRect textureRect = getTextureRect();
		setTextureRect(sf::IntRect(textureRect.left + textureRect.width, textureRect.top, -textureRect.width, textureRect.height));
	}

	// Increase player's y velocity by gravity.
	velocity.y += gravity * gameScale * dt;

	// Move player based on their velocity.
	setPosition(getPosition().x + velocity.x * dt, getPosition().y + velocity.y * dt);
}

void Player::render()
{
}

void Player::kick() // Set player's kicking state to be true. The getter for kicking will be used in collisions to determine the ball's velocity.
{
	kicking = true;
}

void Player::jump()
{
	// If not jumping already...
	if (!jumping)
	{
		// Start jumping by changing the y velocity and moving up slightly.
		jumping = true;
		velocity.y = -700;
		setPosition(getPosition().x, getPosition().y - 10);
	}
	else if (jumping && !doubleJumping) // If already jumping but not double jumping...
	{
		// Set y velocity again.
		doubleJumping = true;
		velocity.y = -600;
	}
}

void Player::setFacingRight(bool fr)
{
	// Set the direction that the player faces, then flip the texture if it's not facing right. Used for the other player as the update function is not called for them.
	facingRight = fr;
	setTextureRect(sf::IntRect(0, 0, getTexture()->getSize().x, getTexture()->getSize().y));

	if (!facingRight)
	{
		sf::IntRect textureRect = getTextureRect();
		setTextureRect(sf::IntRect(textureRect.left + textureRect.width, textureRect.top, -textureRect.width, textureRect.height));
	}
}

