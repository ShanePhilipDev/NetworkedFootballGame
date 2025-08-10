#include "NetworkManager.h"

NetworkManager::NetworkManager()
{
	// Tick rate of 30. Increasing tick rate increases the rate that positions and collisions are sent / received, but also uses up more bandwidth.
	// Initially tried a tick rate of 60, but that used too much bandwidth sending positions/collisions 60 times a second.
	// A tick rate of 30 allows fairly smooth movement while ensuring enough collisions are received for the game to function correctly.
	tickRate = 30;

	// Ping once a second.
	pingRate = 1;
	pingValue = 0;

	// Set TCP socket to non blocking.
	tcpSocket.setBlocking(false);
	tcpListener.setBlocking(false);

	// Setup TCP listener.
	if (tcpListener.listen(tcpSocket.getLocalPort() != sf::Socket::Done))
	{
		// error
	};

	// Set IP and port variables.
	myLocalIP = sf::IpAddress::getLocalAddress();
	myPublicIP = sf::IpAddress::getPublicAddress();
	myPort = tcpListener.getLocalPort();
	
	// Setup UDP socket on same port number as the TCP listener.
	udpSocket.bind(myPort);
	udpSocket.setBlocking(false);
	
	// Set default values
	// ----
	connected = false;
	isUdpSetup = false;
	toSendCollision = false;

	mostRecentVelocity = sf::Vector2f(0, 0);
	mostRecentPosition = sf::Vector2f(0, 0);
	mostRecentPositionTime = 0;
	mostRecentBallCollisionTime = 0;
	// ----
}

NetworkManager::~NetworkManager()
{

}

void NetworkManager::init(GameState* gs, Lobby* l, ObjectManager* om, AudioManager* a)
{
	// Setup pointers.
	gameState = gs;
	lobby = l;
	objectManager = om;
	audio = a;
}

// Ping function. Calculates ping by sending a packet, then starting a timer. When it receives the pong response, the round trip time is calculated and the ping is set.
// Uses TCP as the packets should always arrive, and this allows the function to be used to regularly check if the other player has disconnected.
void NetworkManager::ping()
{
	// Create a packet, set the type, and add the type to the packet.
	sf::Packet packet;
	unsigned short type = PING;
	packet << type;

	// Attempt to send a packet...
	sf::Socket::Status status;
	if ((status = tcpSocket.send(packet)) != sf::Socket::Done)
	{
		// Call disconnect function if the socket has disconnect.
		if (status == sf::Socket::Disconnected)
		{
			disconnect();
		}
	}
	else
	{
		// If successful, get the time that the packet was sent. This will be used again once the response is received to calculate the round trip time.
		sendTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	}
	
}

// Second half of pinging process. When a pong packet is received, you calculate ping.
void NetworkManager::pong()
{
	// Create a packet, set the type, and add the type to the packet.
	sf::Packet packet;
	unsigned short type = PONG;
	packet << type;

	// Attempt to send a packet...
	if (tcpSocket.send(packet) != sf::Socket::Done)
	{
		// Error
	}
	else
	{
		// Successfully sent.
	}

}

// Reset function - disconnect the TCP socket and set values back to default.
void NetworkManager::reset()
{
	tcpSocket.disconnect();

	mostRecentVelocity = sf::Vector2f(0, 0);
	mostRecentPosition = sf::Vector2f(0, 0);
	mostRecentPositionTime = 0;
	mostRecentBallCollisionTime = 0;
}

// Connect function used by the client trying to connect to the host.
bool NetworkManager::connect(sf::IpAddress IP, int port)
{
	// Set the TCP socket to block while connecting.
	tcpSocket.setBlocking(true);
	sf::Socket::Status status = tcpSocket.connect(IP, port);

	// If the connect attempt failed, stop blocking and return false.
	if (status != sf::Socket::Done)
	{
		tcpSocket.setBlocking(false);
		return false;
	}
	else // Otherwise connection was successful. Set blocking to false, connected to true, setup recipient information and send the currently selected character to the host.
	{
		std::cout << "Successfully connected.\n";
		tcpSocket.setBlocking(false);
		connected = true;
		recipientIP = tcpSocket.getRemoteAddress();
		recipientPort = tcpSocket.getRemotePort();
		sendCharacter(lobby->getClientChar());
		return true;
	}
}

// Disconnect function - reset variables then return to lobby.
void NetworkManager::disconnect()
{
	if (connected)
	{
		std::cout << "Disconnected.\n";
	}
	connected = false;
	isUdpSetup = false;
	toSendCollision = false;

	recipientIP = "0.0.0.0";
	recipientPort = 0;

	hostReady = false;
	clientReady = false;
	pingValue = 0;
	gameState->setCurrentState(State::LOBBY);
}

// Tick function - run game tick amount of times a second and handles various aspects of the game's networking.
void NetworkManager::tick()
{
	// If in the lobby, run the lobby tick function.
	if (gameState->getCurrentState() == State::LOBBY)
	{
		lobbyTick();
	}
	else if (gameState->getCurrentState() == State::LEVEL) // If in game, run game tick function.
	{
		gameTick();
	}
}

// Lobby tick - handles host connection and ready statuses.
void NetworkManager::lobbyTick()
{
	if (isHost && !connected)
	{
		// Check if anyone is trying to connect.
		if (tcpListener.accept(tcpSocket) != sf::Socket::Done)
		{
			// Error - no connection made.
		}
		else
		{
			// Successfully connected. Set IP, port and send selected character to client.
			std::cout << "Client connected.\n";
			connected = true;
			recipientIP = tcpSocket.getRemoteAddress();
			recipientPort = tcpSocket.getRemotePort();
			sendCharacter(lobby->getHostChar());
		}
	}
	else if (!isHost && !connected)
	{
		// Client will connect through the connect button and function.
	}

	// Setup players in object manager and set ready states when connected.
	if (connected)
	{
		objectManager->setupPlayers();

		if (isHost)
		{
			hostReady = lobby->getReady();
		}
		else
		{
			clientReady = lobby->getReady();
		}
	}
}

void NetworkManager::gameTick()
{
	// Send player's position.
	sendPosition();
	if (toSendCollision) // Check if a collision is in queue to be sent. By sending packet on tick instead of when the collision happens, it improves performance as less bandwidth is used. 
	{
		sendBallCollision();
	}

	// Handle UDP packets that were received outside of the tick function.
	handleUDP();
	
	

	// After handling packets, get the other player's most recent position, and put it to the front of the most recent positions list.
	positionHistory.push_front(mostRecentPosition);
	velocityHistory.push_front(mostRecentVelocity);
	positionTimes.push_front(mostRecentPositionTime);

	// Only keep last 3 positions.
	if (positionHistory.size() > 3)
	{
		positionHistory.pop_back();
		velocityHistory.pop_back();
		positionTimes.pop_back();
	}
}

void NetworkManager::receiveTCP()
{
	// Create variable for received packet.
	sf::Packet packet;

	// Type of packet received. Type is always at the front of packets when they are sent.
	unsigned short type;

	// Attempt to receive packets until there are no more packets to receive.
	while (tcpSocket.receive(packet) == sf::Socket::Done)
	{
		// Extract type from packet.
		packet >> type;

		// Switch statement to decide what to do with the packet. Calls relevant function based on the type.
		switch (type)
		{
		case READY:
			receiveReadyState(packet);
			break;
		case BALL_COLLISION:
			handleBallCollision(packet);
			break;
		case PING:
			pong();
			break;
		case PONG:
			// Calculate ping.
			receiveTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
			pingValue = receiveTime.count() - sendTime.count();
			break;
		case TIME_SYNC:
			syncTime(packet);
			break;
		case COUNTDOWN_SYNC:
			syncCountdown(packet);
			break;
		case GOAL:
			handleGoal(packet);
			break;
		case CHARACTER:
			lobby->setOpponentChar(packet);
			break;
		default:
			break;
		}
	}
}

void NetworkManager::receiveUDP()
{
	// Packet to receive to.
	sf::Packet packet;

	// Sender details.
	sf::IpAddress sender;
	unsigned short port;
	while (udpSocket.receive(packet, sender, port) == sf::Socket::Done)
	{
		if (!isUdpSetup) // Save IP and port for replying if UDP hasn't been set up yet. Setting it once when the first packet is received (on first position) prevents other people from sending a UDP packet and potentially breaking the game by changing the recipient details.
		{
			// Setup recipient details.
			recipientIP = sender;
			recipientPort = port;
			isUdpSetup = true;
		}

		if (sender == recipientIP && port == recipientPort) // Basic security measure - only deal with packets if they're from the expected address.
		{
			receivedPackets.push_back(packet); // Add packet to received packet list.
		}
	}
}

void NetworkManager::handleUDP()
{
	// Go through each packet that has been received...
	for (int i = 0; i < receivedPackets.size(); i++)
	{
		// Get packet at front of queue.
		sf::Packet packet = receivedPackets.front();
		
		// Get type from packet.
		unsigned short type;
		packet >> type;

		// Switch statement for packet types. UDP is currently only used for position, but there is room to expand if needed.
		switch (type)
		{
		case POSITION:
			receivePosition(packet);
			break;
		default:
			break;
		}

		// Remove packet for queue.
		receivedPackets.pop_front();
	}
}

void NetworkManager::runPrediction(float dt)
{
	if (positionHistory.size() < 3)
	{
		// Don't run prediction if don't have full history list.
		return;
	}

	sf::Vector2f pos[3];
	sf::Vector2f vel[3];
	float time[3];

	// Get positions from list.
	int i = 0;
	for (std::list<sf::Vector2f>::iterator it = positionHistory.begin(); it != positionHistory.end(); ++it)
	{
		pos[i] = sf::Vector2f(it->x, it->y);
		i++;
	}

	// Get position times from list.
	i = 0;
	for (std::list<float>::iterator it = positionTimes.begin(); it != positionTimes.end(); ++it)
	{
		time[i] = *it;
		i++;
	}

	// Get velocities from list.
	i = 0;
	for (std::list<sf::Vector2f>::iterator it = velocityHistory.begin(); it != velocityHistory.end(); ++it)
	{
		vel[i] = *it;
		i++;
	}
	
	// Linear prediction - predict where the other player will move based on their current velocity. 
	otherPlayer->setPosition(otherPlayer->getPosition().x + vel[0].x * dt, otherPlayer->getPosition().y + vel[0].y * dt);
	
	/*
	// Other prediction methods - not used.
	
	// Linear prediction method using time
	sf::Vector2f velocity;
	float timeSinceLastMsg, timeBetweenPositions;
	sf::Vector2f displacement;
	sf::Vector2f distanceBetweenPositions;

	timeSinceLastMsg = objectManager->getTime() - time[0];

	timeBetweenPositions = time[0] - time[1];
	distanceBetweenPositions.x = pos[0].x - pos[1].x;
	distanceBetweenPositions.y = pos[0].y - pos[1].y;

	// v = d/t
	velocity.x = distanceBetweenPositions.x / timeBetweenPositions;
	velocity.y = distanceBetweenPositions.y / timeBetweenPositions;

	// s = vt
	displacement.x = velocity.x * timeSinceLastMsg;
	displacement.y = velocity.y * timeSinceLastMsg;

	otherPlayer->setPosition(sf::Vector2f(sf::Vector2f(otherPlayer->getPosition().x + displacement.x, sf::Vector2f(otherPlayer->getPosition().y + displacement.y));
	
	// Quadratic prediction

	sf::Vector2f displacement;
	sf::Vector2f velocity;
	sf::Vector2f initialVelocity;
	float changeInTime;
	sf::Vector2f acceleration;
	float timeSinceLastMsg = objectManager->getTime() - time[0];

	velocity.x = (pos[0].x - pos[1].x) / (time[0] - time[1]);
	velocity.y = (pos[0].y - pos[1].y) / (time[0] - time[1]);

	initialVelocity.x = (pos[1].x - pos[2].x) / (time[1] - time[2]);
	initialVelocity.y = (pos[1].y - pos[2].y) / (time[1] - time[2]);

	changeInTime = time[0] - time[2];

	acceleration = (velocity - initialVelocity) / changeInTime;

	displacement = (velocity * timeSinceLastMsg) + (0.5f * acceleration * powf(timeSinceLastMsg, 2));

	otherPlayer->setPosition(sf::Vector2f(otherPlayer->getPosition().x + displacement.x, otherPlayer->getPosition().y + displacement.y));
	*/
}

void NetworkManager::resetPositionData()
{
	// Reset all position related values to default.
	mostRecentPositionTime = 0;
	mostRecentBallCollisionTime = 0;
	positionHistory.clear();
	velocityHistory.clear();
	positionTimes.clear();
	receivedPackets.clear();
}

// Syncing time functions.
// These work by sending the current system time and the recipient adds the difference to their timer. This will work reliably on local machines or if both system's clocks have benn synced to an NTP server.
// If system clocks are not in sync, then it will use the ping value instead which is less reliable. This could be improved by using an average ping value.
// ----
void NetworkManager::syncTime()
{
	// Send packet with system time.
	sf::Packet packet;
	unsigned short type = TIME_SYNC;
	long long syncTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	packet << type << syncTime;

	if (tcpSocket.send(packet))
	{
		// Error.
	}
	else
	{
		// Sent successfully.
	}
}

void NetworkManager::syncTime(sf::Packet packet)
{
	// Receive other player's system time.
	long long receivedTime;
	packet >> receivedTime;
	
	// Calculate difference in times.
	long long timeDif = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - receivedTime);

	if (timeDif < 0.5) // If difference in system times is usable (such as if times are synced using NTP or if running on local machine), use that value.
	{
		objectManager->setTime(float(timeDif) / 1000);
	}
	else // Otherwise use ping to sync times.
	{
		objectManager->setTime((float(pingValue) / 1000) / 2);
	}
}
// ----

// Functions for setting the ready countdown's timer. Follows same principle as the time sync functions.
// ----
void NetworkManager::syncCountdown()
{
	// Send packet with system time.
	sf::Packet packet;
	unsigned short type = COUNTDOWN_SYNC;
	long long syncTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	packet << type << syncTime;

	if (tcpSocket.send(packet))
	{
		// Error.
	}
	else
	{
		// Sent successfully.
	}
}

void NetworkManager::syncCountdown(sf::Packet packet)
{
	// Receive other player's system time.
	long long receivedTime;
	packet >> receivedTime;

	// Calculate difference in times.
	long long timeDif = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - receivedTime);

	if (timeDif < 0.5) // If system times are closely synced between machines, use that value.
	{
		lobby->setCountdownTimer(3.0f - float(timeDif) / 1000);
	}
	else // Otherwise use ping.
	{
		lobby->setCountdownTimer(3.0f - (float(pingValue) / 1000) / 2);
	}
}
// ----

// Function for sending position, velocity and kicking status to the other player.
void NetworkManager::sendPosition()
{
	// Setup packet with type, position, time, velocity, and kicking status.
	sf::Packet packet;
	unsigned short type = POSITION;
	sf::Vector2f position = controlledPlayer->getPosition();
	float time = objectManager->getTime();
	sf::Vector2f velocity = controlledPlayer->getVelocity();
	bool kicking = controlledPlayer->getKicking();
	packet << type << time << position.x << position.y << velocity.x << velocity.y << kicking;

	// Send packet.
	if (udpSocket.send(packet, recipientIP, recipientPort))
	{
		// Error
	}
	else
	{
		// Packet successfuly sent.
	}
	
	
}

void NetworkManager::receivePosition(sf::Packet packet)
{
	// Variables to receive from packet.
	sf::Vector2f position;
	sf::Vector2f velocity;
	float time;
	bool kicking;

	// Retrieve data from packet.
	packet >> time >> position.x >> position.y >> velocity.x >> velocity.y >> kicking;

	if (time > mostRecentPositionTime) // Use the most recent position packet's data.
	{
		// Set most recent values.
		mostRecentPositionTime = time;
		mostRecentPosition = position;
		mostRecentVelocity = velocity;

		// Determine direction based on velocity. The player's direction could have been sent with the packet, but working out direction locally saves bandwidth.
		if (velocity.x > 0)
		{
			otherPlayer->setFacingRight(true);
		}
		else if (velocity.x < 0)
		{
			otherPlayer->setFacingRight(false);
		}

		// Set kicking state.
		otherPlayer->setKicking(kicking);

		// Set position.
		otherPlayer->setPosition(mostRecentPosition);
	}
}

// Function to be called when a collision occurs.
void NetworkManager::setBallCollision(sf::Vector2f pos, sf::Vector2f vel)
{
	// This notifies the network manager that a collision needs to be sent, and sets the details of the collision. On next tick, the collision will be sent.
	toSendCollision = true;
	collisionPos = pos;
	collisionVel = vel;
	collisionTime = objectManager->getTime();
}

// Function to send collision details.
void NetworkManager::sendBallCollision()
{
	// Setup packet with type, time, ball position, and ball velocity.
	sf::Packet packet;
	unsigned short type = BALL_COLLISION;
	float time = collisionTime;
	sf::Vector2f position = collisionPos;
	sf::Vector2f velocity = collisionVel;
	packet << type << time << position.x << position.y << velocity.x << velocity.y;

	// Send packet.
	if (tcpSocket.send(packet) != sf::Socket::Done)
	{
		// Error
	}
	else
	{
		// Packet successfully sent.
		toSendCollision = false;
	}
	
}

// Function to be called when receiving ball collisions.
void NetworkManager::handleBallCollision(sf::Packet packet)
{
	// Extract time, ball position and ball velocity from packet.
	// ----
	float time;
	sf::Vector2f position;
	sf::Vector2f velocity;

	packet >> time >> position.x >> position.y >> velocity.x >> velocity.y;
	// ----

	// Check if it's the most recent collision.
	if (time > mostRecentBallCollisionTime)
	{
		// Set new most recent time if it is.
		mostRecentBallCollisionTime = time;

		// Get pointer to the ball.
		Ball* ball = objectManager->getBall();

		// Time that has passed since the collision.
		float latency = objectManager->getTime() - time;

		// Save ball position from before the collision.
		sf::Vector2f previousPos = ball->getPosition();

		// Set the ball's position to where the collision occured, and set velocity.
		ball->setPositionXY(position.x, position.y);
		ball->setVelocity(velocity.x, velocity.y);

		// Set time simulated to 0, and how much the simulation is going to be incremented.
		float simTime = 0;
		float simIncrement = objectManager->getPhysicsStep();
		
		while (simTime < latency) // Simulate ahead to check if the ball is going to collide with anything.
		{
			objectManager->checkBallCollision();
			ball->update(simIncrement);
			simTime += simIncrement;
		}

		// Set lagged position to the position before the collision.
		ball->setLagPosition(previousPos.x, previousPos.y);

		// Tell the ball to interpolate
		ball->setInterpolating(true);
		
		// Play kicking sound.
		audio->playSoundbyName("kick");
	}
}

// Send goal function. Only the host will send this - their simulation is treated as the 'correct' one.
void NetworkManager::sendGoal(Side s)
{
	// Setup packet with type, time, and side that scored the goal.
	sf::Packet packet;
	unsigned short type = GOAL;
	float time = objectManager->getTime();
	unsigned short side = s;
	packet << type << time << side;

	// Send packet.
	if (connected)
	{
		if (tcpSocket.send(packet))
		{
			// Error.
		}
		else
		{
			// Successfully sent.
		}
	}
}

// Handle goal function. Only the receiving client will call this function.
void NetworkManager::handleGoal(sf::Packet packet)
{
	// Extract data from packet.
	float time;
	unsigned short side;
	packet >> time >> side;

	// Set their game's goal scored status, and adjust the reset timer to account for latency.
	objectManager->setGoalScored(true);
	objectManager->setResetTimer(objectManager->getTime() - time);

	// Increase score based on which side scored.
	if (side == Side::LEFT)
	{
		objectManager->increaseLeftScore();
	}
	else if (side == Side::RIGHT)
	{
		objectManager->increaseRightScore();
	}

	// Play goal sound.
	audio->playSoundbyName("goal");
}

// Send ready state function used in lobby.
void NetworkManager::sendReadyState(bool ready)
{
	// Setup packet with type and ready state.
	sf::Packet packet;
	unsigned short type = READY;
	packet << type << ready;

	if (tcpSocket.send(packet) != sf::Socket::Done)
	{
		// Error.
	}
	else
	{
		// Packet successfully sent.
		std::cout << "Ready status sent.\n";
	}
	
}

// Receive ready state function.
void NetworkManager::receiveReadyState(sf::Packet packet)
{
	// Extract data from packet.
	bool ready;
	packet >> ready;

	// Set other player's ready status.
	if (isHost)
	{
		clientReady = ready;
	}
	else if (!isHost)
	{
		hostReady = ready;
	}
	
	std::cout << "Ready status received.\n";
}

// Send currently selected character.
void NetworkManager::sendCharacter(int n)
{
	// Setup packet with type and character.
	sf::Packet packet;
	unsigned short type = CHARACTER;
	packet << type << n;

	// Send packet.
	if (tcpSocket.send(packet) != sf::Socket::Done)
	{
		// Error
	}
	else
	{
		// Successfully sent.
	}
}


// Function that checks if both players are ready, and returns true or false.
bool NetworkManager::getReadyStatus()
{
	if (hostReady && clientReady)
	{
		return true;
	}
	else
	{
		return false;
	}
}


