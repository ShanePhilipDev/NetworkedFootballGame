#include "ObjectManager.h"

ObjectManager::ObjectManager()
{
	// Load font.
	font.loadFromFile("font/arial.ttf");

	// Load character textures.
	messi.loadFromFile("gfx/messi.png");
	ronaldo.loadFromFile("gfx/ronaldo.png");
	kirby.loadFromFile("gfx/kirby.png");
	miedema.loadFromFile("gfx/miedema.png");
	
	// Set default values.
	// ----
	leftScore = 0;
	rightScore = 0;

	physicsStep = 1.0f/180.0f; // High physics rate to ensure smooth movement and timely collisions.
	physicsTimer = 0;

	goalScored = false;
	resetLength = 2;
	resetTimer = 0;
	
	gameLength = 90;
	gameTimer = 0;
	// ----

	// Setup ball.
	ballSprite.loadFromFile("gfx/football.png");
	ball.setTexture(&ballSprite);
	ball.setSize(sf::Vector2f(50, 50));
	ball.setOrigin(ball.getSize().x / 2, ball.getSize().y / 2);
	ball.setCollisionBox(-ball.getSize().x / 2, -ball.getSize().y / 2, ball.getSize().x, ball.getSize().y);

	// Setup ball collision box outline.
	ballBox.setOutlineThickness(2);
	ballBox.setOutlineColor(sf::Color::Red);
	ballBox.setFillColor(sf::Color::Transparent);
	ballBox.setSize(ball.getSize());
	ballBox.setPosition(ball.getCollisionBox().left, ball.getCollisionBox().top);
	
	// Setup players.
	// ----
	leftPlayer.setTexture(&messi);
	leftPlayer.setSize(sf::Vector2f(100, 100));
	leftPlayer.setCollisionBox(0, 0, leftPlayer.getSize().x, leftPlayer.getSize().y);

	rightPlayer.setTexture(&ronaldo);
	rightPlayer.setSize(sf::Vector2f(100, 100));
	rightPlayer.setCollisionBox(0, 0, leftPlayer.getSize().x, leftPlayer.getSize().y);

	leftPlayer.setFacingRight(true);
	rightPlayer.setFacingRight(false);
	// ----

	// Setup goals.
	// ----
	goalSprite.loadFromFile("gfx/goalposts.png");
	leftGoal.setTexture(&goalSprite);
	leftGoal.setSize(sf::Vector2f(160, 200));
	leftGoal.setCollisionBox(0, 0, leftGoal.getSize().x, leftGoal.getSize().y);

	rightGoal.setTexture(&goalSprite);
	rightGoal.setSize(sf::Vector2f(160, 200));
	rightGoal.setCollisionBox(0, 0, rightGoal.getSize().x, rightGoal.getSize().y);
	// ----
	
	// Setup text objects.
	// ----
	sf::Text infoText;
	infoText.setFont(font);
	infoText.setCharacterSize(20);
	infoText.setFillColor(sf::Color::White);
	infoText.setOutlineColor(sf::Color::Black);
	infoText.setOutlineThickness(1);

	ping = infoText;
	score = infoText;
	time = infoText;

	infoText.setCharacterSize(40);
	infoText.setOutlineThickness(4);
	infoText.setFillColor(sf::Color::Yellow);

	goal = infoText;
	goal.setString("GOAL!");
	// ----

	// Seed random nmuber generator with time.
	srand(std::time(NULL));
}

ObjectManager::~ObjectManager()
{
}

void ObjectManager::init(sf::RenderWindow* hwnd, Input* in, NetworkManager* nm, GameState* gs, Lobby* l, AudioManager* a)
{
	// Set pointers.
	window = hwnd;
	input = in;
	networkManager = nm;
	gameState = gs;
	lobby = l;
	audio = a;

	// Set inputs.
	leftPlayer.setInput(input);
	rightPlayer.setInput(input);

	// Setup objects.
	// ----
	floor.setSize(sf::Vector2f(window->getSize().x, 100));
	floor.setFillColor(sf::Color::Green);
	floor.setPosition(0, window->getSize().y - floor.getSize().y);
	floor.setCollisionBox(0, 0, floor.getSize().x, floor.getSize().y);

	ceiling.setSize(sf::Vector2f(window->getSize().x, 10));
	ceiling.setFillColor(sf::Color::Black);
	ceiling.setPosition(0, 0);
	ceiling.setCollisionBox(0, 0, ceiling.getSize().x, ceiling.getSize().y);

	leftWall.setSize(sf::Vector2f(10, window->getSize().y));
	leftWall.setFillColor(sf::Color::Black);
	leftWall.setPosition(0, 0);
	leftWall.setCollisionBox(0, 0, leftWall.getSize().x, leftWall.getSize().y);

	leftGoal.setPosition(leftWall.getSize().x, floor.getPosition().y - leftGoal.getSize().y);
	leftGoal.setCollisionBox(0, 0, leftGoal.getSize().x, leftGoal.getSize().y * 0.02);

	rightWall.setSize(sf::Vector2f(10, window->getSize().y));
	rightWall.setFillColor(sf::Color::Black);
	rightWall.setPosition(window->getSize().x - rightWall.getSize().x, 0);
	rightWall.setCollisionBox(0, 0, rightWall.getSize().x, rightWall.getSize().y);

	rightGoal.setPosition(rightWall.getPosition().x - rightGoal.getSize().x, floor.getPosition().y - rightGoal.getSize().y);
	rightGoal.setCollisionBox(0, 0, rightGoal.getSize().x, rightGoal.getSize().y * 0.02);

	// Flip texture
	sf::IntRect textureRect = rightGoal.getTextureRect();
	rightGoal.setTextureRect(sf::IntRect(textureRect.left + textureRect.width, textureRect.top, -textureRect.width, textureRect.height));

	ball.setPositionXY(window->getSize().x * 0.5 - 0.5 * ball.getSize().x, window->getSize().y * 0.2);
	ball.setLagPosition(window->getSize().x * 0.5 - 0.5 * ball.getSize().x, window->getSize().y * 0.2);
	// ----
}

void ObjectManager::handleInput(float dt)
{
	// Use controlled player's handle input function.
	controlledPlayer->handleInput(dt);
}

void ObjectManager::update(float dt)
{
	// Increase timers.
	gameTimer += dt;
	physicsTimer += dt;

	// If game timer exceeds the length of time the game is meant to run for, return to the lobby and set the post match lobby with score.
	if (gameTimer > gameLength)
	{
		gameState->setCurrentState(State::LOBBY);
		lobby->setPostMatchLobby(leftScore, rightScore);
		gameTimer = 0;
	}

	// If player is kicking, set their colour to red. Otherwise set their colour to white (normal).
	// ----
	if (leftPlayer.getKicking())
	{
		leftPlayer.setFillColor(sf::Color::Red);
	}
	else
	{
		leftPlayer.setFillColor(sf::Color::White);
	}
	
	if (rightPlayer.getKicking())
	{
		rightPlayer.setFillColor(sf::Color::Red);
	}
	else
	{
		rightPlayer.setFillColor(sf::Color::White);
	}
	// ----

	// Run physics when timer exceeds the physics step rate.
	if (physicsTimer > physicsStep)
	{
		physicsTimer -= physicsStep;

		// Check ball and player collisions with the environment (not each other).
		checkBallCollision();
		checkPlayerCollision(controlledPlayer);

		// Ball physics and prediction.
		networkManager->runPrediction(physicsStep);
		ball.update(physicsStep);

		// Check for collision between ball and player.
		checkPlayerBallCollision();

		// Update player.
		controlledPlayer->update(physicsStep);
	}

	// When a goal hasn't been scored yet, check if a goal has been scored.
	if (!goalScored)
	{
		if ((ball.getPosition().x < leftGoal.getPosition().x + leftGoal.getSize().x - ball.getSize().x / 2) && (ball.getPosition().y > leftGoal.getCollisionBox().top)) // If ball is within left goal.
		{
			if (networkManager->getHost()) // If host, set goal scored to true, increase score, send the goal notification to the other player, and play the goal sound. Other client will do the same once it receives the packet from the host.
			{
				goalScored = true;
				rightScore += 1;
				networkManager->sendGoal(NetworkManager::Side::RIGHT);
				audio->playSoundbyName("goal");
			}
		}
		else if ((ball.getPosition().x > rightGoal.getPosition().x + ball.getSize().x / 2) && (ball.getPosition().y > rightGoal.getCollisionBox().top)) // Same as above, but for right goal.
		{
			if (networkManager->getHost())
			{
				goalScored = true;
				leftScore += 1;
				networkManager->sendGoal(NetworkManager::Side::LEFT);
				audio->playSoundbyName("goal");
			}
		}
	}
	else if (goalScored) // When a goal has been scored...
	{
		// Increase reset timer.
		resetTimer += dt;

		if (resetTimer > 0.90 * resetLength) // Move ball back to start point just before doing reset.
		{
			resetBallPosition();
		}

		// Reset the ball and players after the reset length has been exceeded.
		if (resetTimer > resetLength)
		{
			goalReset();
		}
	}
	
	// Setup text objects
	// ----
	ping.setPosition(window->getSize().x * 0.1, window->getSize().y * 0.1);
	ping.setString("Ping: " + std::to_string(networkManager->getPing()));
	
	score.setPosition(window->getSize().x * 0.5 - score.getGlobalBounds().width * 0.5, window->getSize().y * 0.1);
	score.setString(std::to_string(leftScore) + " - " + std::to_string(rightScore));

	time.setPosition(window->getSize().x * 0.8, window->getSize().y * 0.1);

	std::stringstream stream;
	stream << "Time: " << std::fixed << std::setprecision(2) << gameTimer;
	std::string timeString = stream.str();
	time.setString(timeString);

	goal.setPosition(window->getSize().x * 0.5 - goal.getGlobalBounds().width * 0.5, window->getSize().y * 0.3);
	// ----

	// Box to follow the ball's collision box - show's where ball actually is when interpolating.
	ballBox.setPosition(ball.getCollisionBox().left, ball.getCollisionBox().top);
}

void ObjectManager::render()
{
	// Render objects in world
	//window->draw(ballBox); // ball collision box
	window->draw(ball);
	window->draw(floor);
	window->draw(ceiling);
	window->draw(leftWall);
	window->draw(rightWall);

	// Change render order of players based on whether you're host so that your player is on top.
	if (networkManager->getHost())
	{
		window->draw(rightPlayer);
		window->draw(leftPlayer);
	}
	else
	{
		window->draw(leftPlayer);
		window->draw(rightPlayer);
	}
	
	// Render rest of objects
	window->draw(leftGoal);
	window->draw(rightGoal);
	window->draw(ping);
	window->draw(score);
	window->draw(time);

	// If a goal has been scored, display goal text.
	if (goalScored)
	{
		window->draw(goal);
	}
}


 
void ObjectManager::checkPlayerCollision(Player* player)
{
	// Check player - floor collision, then set player position, velocity and jump state.
	if (Collision::checkBoundingBox(player, &floor))
	{
		if (player->getVelocity().y > 0)
		{
			player->setPosition(player->getPosition().x, floor.getPosition().y - floor.getCollisionBox().height);
			player->setVelocity(player->getVelocity().x, 0);
			player->setJumping(false);
		}
	}

	// Check player - ceiling collision, then set player velocity.
	if (Collision::checkBoundingBox(player, &ceiling))
	{
		if (player->getVelocity().y < 0)
		{
			player->setVelocity(player->getVelocity().x, 0);
		}
	}

	// Check player - wall collision, then set player velocity.
	if (Collision::checkBoundingBox(player, &leftWall))
	{
		if (player->getVelocity().x < 0)
		{
			player->setVelocity(0, player->getVelocity().y);
		}
	}

	// Check player - wall collision, then set player velocity.
	if (Collision::checkBoundingBox(player, &rightWall))
	{
		if (player->getVelocity().x > 0)
		{
			player->setVelocity(0, player->getVelocity().y);
		}
	}

	// Check player - goal collision. Additional checks so that player only collides with top of goal. Appropriate variables set according to where collision occured.
	if (Collision::checkBoundingBox(player, &leftGoal))
	{
		if (player->getCollisionBox().left < leftGoal.getCollisionBox().left + leftGoal.getCollisionBox().width)
		{
			if (player->getVelocity().x < 0 && player->getPosition().x > 150)
			{
				player->setVelocity(0, player->getVelocity().y);
				player->setPosition(leftGoal.getCollisionBox().left + leftGoal.getCollisionBox().width + 1, player->getPosition().y);
			}
				
			
			if (player->getPosition().x < leftGoal.getCollisionBox().left + leftGoal.getCollisionBox().width)
			{
				if (player->getVelocity().y > 0)
				{
					player->setPosition(player->getPosition().x, leftGoal.getPosition().y - player->getSize().y);
					player->setVelocity(player->getVelocity().x, 0);
					player->setJumping(false);
				}
				else if (player->getVelocity().y < 0)
				{
					player->setPosition(player->getPosition().x, leftGoal.getPosition().y + leftGoal.getCollisionBox().height);
					player->setVelocity(player->getVelocity().x, 0);
				}
			}
		}		
	}

	// Check player - goal collision. Additional checks so that player only collides with top of goal. Appropriate variables set according to where collision occured.
	if (Collision::checkBoundingBox(player, &rightGoal))
	{
		if (player->getCollisionBox().left + player->getCollisionBox().width > rightGoal.getCollisionBox().left)
		{
			if (player->getVelocity().x > 0 && player->getPosition().x < 1050 - player->getCollisionBox().width)
			{
				player->setVelocity(0, player->getVelocity().y);
				player->setPosition(rightGoal.getCollisionBox().left - player->getCollisionBox().width, player->getPosition().y);
			}

			
			if (player->getPosition().x > rightGoal.getCollisionBox().left - player->getCollisionBox().width)
			{
				if (player->getVelocity().y > 0)
				{
					player->setPosition(player->getPosition().x, rightGoal.getPosition().y - player->getSize().y);
					player->setVelocity(player->getVelocity().x, 0);
					player->setJumping(false);
				}
				else if (player->getVelocity().y < 0)
				{
					player->setPosition(player->getPosition().x, rightGoal.getPosition().y + rightGoal.getCollisionBox().height);
					player->setVelocity(player->getVelocity().x, 0);
				}
			}
		}
	}
}

void ObjectManager::checkBallCollision()
{
	// Check ball - floor collision, then adjust velocity for ball to bounce.
	if (Collision::checkBoundingBox(&ball, &floor))
	{
		//ball.setPositionXY(ball.getPosition().x, floor.getCollisionBox().top - ball.getSize().x / 2 - 5);
		if (ball.getVelocity().y > 0)
		{
			ball.setVelocity(ball.getVelocity().x, -ball.getVelocity().y * 0.75);
		}
	}

	// Check ball - ceiling collision, then adjust velocity for ball to bounce.
	if (Collision::checkBoundingBox(&ball, &ceiling))
	{
		if (ball.getVelocity().y < 0)
		{
			ball.setVelocity(ball.getVelocity().x, -ball.getVelocity().y);
		}
	}

	// Check ball - wall collision, then adjust velocity for ball to bounce.
	if (Collision::checkBoundingBox(&ball, &leftWall))
	{
		if (ball.getVelocity().x < 0)
		{
			ball.setVelocity(-ball.getVelocity().x * 0.9, ball.getVelocity().y);
		}
	}

	// Check ball - wall collision, then adjust velocity for ball to bounce.
	if (Collision::checkBoundingBox(&ball, &rightWall))
	{
		if (ball.getVelocity().x > 0)
		{
			ball.setVelocity(-ball.getVelocity().x * 0.9, ball.getVelocity().y);
		}
	}

	// Check ball - goal collision. Additional checks so that ball only collides with top of goal. Bounce determined by where the collision occurs. If the front of the crossbar is hit, play a sound.
	if (Collision::checkBoundingBox(&ball, &leftGoal))
	{
		if (ball.getCollisionBox().left < leftGoal.getCollisionBox().left + leftGoal.getCollisionBox().width)
		{
			if (ball.getVelocity().x < 0 && ball.getPosition().x > 150)
			{
				ball.setVelocity(-ball.getVelocity().x * 0.9, ball.getVelocity().y);
				audio->playSoundbyName("post");
			}

			if (ball.getPosition().x < leftGoal.getCollisionBox().left + leftGoal.getCollisionBox().width)
			{
				if (ball.getVelocity().y > 0)
				{
					ball.setVelocity(ball.getVelocity().x, -ball.getVelocity().y * 0.75);
				}
				else if (ball.getVelocity().y < 0)
				{
					ball.setVelocity(ball.getVelocity().x, -ball.getVelocity().y);
				}
			}
		}
	}

	// Check ball - goal collision. Additional checks so that ball only collides with top of goal. Bounce determined by where the collision occurs. If the front of the crossbar is hit, play a sound.
	if (Collision::checkBoundingBox(&ball, &rightGoal))
	{
		if (ball.getCollisionBox().left + ball.getCollisionBox().width > rightGoal.getCollisionBox().left)
		{
			if (ball.getVelocity().x > 0 && ball.getPosition().x < 1050)
			{
				ball.setVelocity(-ball.getVelocity().x * 0.9, ball.getVelocity().y);
				audio->playSoundbyName("post");
			}


			if (ball.getPosition().x > rightGoal.getCollisionBox().left - ball.getCollisionBox().width)
			{
				if (ball.getVelocity().y > 0)
				{
					ball.setVelocity(ball.getVelocity().x, -ball.getVelocity().y * 0.75);
				}
				else if (ball.getVelocity().y < 0)
				{
					ball.setVelocity(ball.getVelocity().x, -ball.getVelocity().y);
				}
			}
		}
	}
}

void ObjectManager::checkPlayerBallCollision()
{
	// When the player collides with the ball...
	if (Collision::checkBoundingBox(controlledPlayer, &ball))
	{
		// Calculate direction vector between centre of both objects
		sf::Vector2f directionVector = calculateDirection(controlledPlayer->getCentre(), ball.getCentre());

		// Randomise height of shot.
		float yPower = rand() % 750 + 2000; 

		// Calculate velocity of ball based on direction, the player's speed, and whether they're kicking or not.
		if (controlledPlayer->getKicking())
		{
			ball.setVelocity(controlledPlayer->getVelocity().x * 0.5 + directionVector.x * 1500, controlledPlayer->getVelocity().y * 0.5 + -abs(directionVector.y) * yPower);
		}
		else
		{
			if (controlledPlayer->getCollisionBox().top + controlledPlayer->getCollisionBox().height - 5 < ball.getPosition().y - ball.getSize().y/2) // Don't use absolute y direction if player is hitting from above.
			{
				ball.setVelocity(controlledPlayer->getVelocity().x * 0.5 + directionVector.x * 500, controlledPlayer->getVelocity().y * 0.5 + directionVector.y * 500);
			}
			else
			{
				ball.setVelocity(controlledPlayer->getVelocity().x * 0.5 + directionVector.x * 500, controlledPlayer->getVelocity().y * 0.5 + -abs(directionVector.y) * 500);
			}
			
		}

		// Send collision to the other player.
		networkManager->setBallCollision(ball.getPosition(), ball.getVelocity());

		// Play kicking sound effect.
		audio->playSoundbyName("kick");
	}
}

void ObjectManager::setupPlayers()
{
	// If client is host...
	if (networkManager->getHost()) // Control the left player.
	{
		controlledPlayer = &leftPlayer;
		networkManager->setControlledPlayer(controlledPlayer);
		networkManager->setOtherPlayer(&rightPlayer);
		controlledPlayer->setPosition(window->getSize().x * 0.25 - 0.5 * leftPlayer.getSize().x, 400);
	}
	else if (networkManager->getHost() == false) // Otherwise control the right player.
	{
		controlledPlayer = &rightPlayer;
		networkManager->setControlledPlayer(controlledPlayer);
		networkManager->setOtherPlayer(&leftPlayer);
		controlledPlayer->setPosition(window->getSize().x * 0.65 + 0.5 * rightPlayer.getSize().x, 400);
	}
}

void ObjectManager::goalReset()
{
	// Reset ball's positions.
	resetBallPosition();
	
	// Move player back to start position (only move own player, will receive position update from the other player).
	if (controlledPlayer == &leftPlayer)
	{
		controlledPlayer->setPosition(window->getSize().x * 0.25 - 0.5 * leftPlayer.getSize().x, 400);
	}
	else if (controlledPlayer == &rightPlayer)
	{
		controlledPlayer->setPosition(window->getSize().x * 0.65 + 0.5 * leftPlayer.getSize().x, 400);
	}

	// Reset directions.
	leftPlayer.setFacingRight(true);
	rightPlayer.setFacingRight(false);

	// Reset timer and goal scored bool.
	resetTimer = 0;
	goalScored = false;
}

void ObjectManager::resetBallPosition()
{
	// Reset ball's velocity and position.
	ball.setVelocity(0, 0);
	ball.setPositionXY(window->getSize().x * 0.5 - 0.5 * ball.getSize().x, window->getSize().y * 0.2);
	ball.setLagPosition(window->getSize().x * 0.5 - 0.5 * ball.getSize().x, window->getSize().y * 0.2);
}

void ObjectManager::start()
{
	// Reset scores, network manager data and player/ball positions.
	leftScore = 0;
	rightScore = 0;
	networkManager->resetPositionData();
	goalReset();

	// Set each player's texture based on their selected character
	// ----
	switch (lobby->getHostChar())
	{
	case Lobby::Character::MESSI:
		leftPlayer.setTexture(&messi);
		break;
	case Lobby::Character::RONALDO:
		leftPlayer.setTexture(&ronaldo);
		break;
	case Lobby::Character::KIRBY:
		leftPlayer.setTexture(&kirby);
		break;
	case Lobby::Character::MIEDEMA:
		leftPlayer.setTexture(&miedema);
		break;
	}

	switch (lobby->getClientChar())
	{
	case Lobby::Character::MESSI:
		rightPlayer.setTexture(&messi);
		break;
	case Lobby::Character::RONALDO:
		rightPlayer.setTexture(&ronaldo);
		break;
	case Lobby::Character::KIRBY:
		rightPlayer.setTexture(&kirby);
		break;
	case Lobby::Character::MIEDEMA:
		rightPlayer.setTexture(&miedema);
		break;
	}
	// ----

	// If player is host, sync time to other player.
	if (networkManager->getHost())
	{
		gameTimer = 0;
		networkManager->syncTime();
	}
}

sf::Vector2f ObjectManager::calculateDirection(sf::Vector2f pos1, sf::Vector2f pos2)
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
