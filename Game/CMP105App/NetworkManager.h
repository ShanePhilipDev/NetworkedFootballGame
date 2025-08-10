#pragma once
#include <SFML/Network.hpp>
#include <iostream>
#include <chrono>
#include <list>
#include "Framework/GameState.h"
#include "Lobby.h"
#include "Player.h"
#include <deque>

class ObjectManager;
class Lobby;

class NetworkManager
{
public:
	NetworkManager();
	~NetworkManager();

	// Enum for which side has scored a goal.
	enum Side { LEFT = 0, RIGHT };

	// Setup pointers.
	void init(GameState* gs, Lobby* l, ObjectManager* om, AudioManager* a);

	// Function for calculating ping.
	void ping();

	// Function for handling certain networking tasks at a fixed rate.
	void tick();
	
	// Getter functions.
	// ----
	int getTickRate() {
		return tickRate;
	};

	int getPingRate() {
		return pingRate;
	};

	int getPing()
	{
		return pingValue;
	};

	bool getConnected()
	{
		return connected;
	};

	std::string getMyLocalIP()
	{
		return myLocalIP.toString();
	};

	std::string getMyPublicIP()
	{
		return myPublicIP.toString();
	}

	std::string getRecipientIP()
	{
		return recipientIP.toString();
	};

	int getMyPort()
	{
		return myPort;
	};

	int getRecipientPort()
	{
		return recipientPort;
	};

	bool getClientReady()
	{
		return clientReady;
	};
	bool getHostReady()
	{
		return hostReady;
	};

	bool getHost()
	{
		return isHost;
	};
	// ----

	// Setter functions.
	// ----
	void setHost(bool host)
	{
		isHost = host;
	};

	void setControlledPlayer(Player* player)
	{
		controlledPlayer = player;
	};

	void setOtherPlayer(Player* player)
	{
		otherPlayer = player;
	};
	// ----

	// Connect and disconnect functions.
	bool connect(sf::IpAddress IP, int port);
	void disconnect();

	// Function to check if both players are ready.
	bool getReadyStatus();

	// Functions for sending data between clients.
	void sendReadyState(bool ready);
	void sendCharacter(int n);
	void sendGoal(Side s);
	void sendBallCollision();

	// Ball collisions are handled on each tick rather than when they happen to save bandwidth.
	void setBallCollision(sf::Vector2f pos, sf::Vector2f vel);
	
	// Function to reset the network manager.
	void reset();

	// Functions for receiving data on each socket. These are called every frame, to ensure packets are received as soon as possible after they are sent.
	void receiveTCP();
	void receiveUDP();

	// Functions for predicting the other players movement, and resetting the data that is used for these predictions.
	void runPrediction(float dt);
	void resetPositionData();

	// Functions for syncing time between clients.
	void syncTime();
	void syncCountdown();
	
private:
	// Enum for the different types of packets that will be sent.
	enum PacketType { PING = 0, PONG, READY, POSITION, BALL_COLLISION, TIME_SYNC, COUNTDOWN_SYNC, GOAL, CHARACTER, END };

	// Response to receiving a ping packet.
	void pong();
	
	// Different tick functions for use depending on game state.
	void lobbyTick();
	void gameTick();

	// Functions for sending and receiving position data.
	void sendPosition();
	void receivePosition(sf::Packet packet);

	// Functions for handling incoming packets.
	void handleUDP();
	void handleGoal(sf::Packet packet);
	void handleBallCollision(sf::Packet packet);
	void receiveReadyState(sf::Packet packet);
	void syncTime(sf::Packet packet);
	void syncCountdown(sf::Packet packet);
	
	// Pointers needed by the class.
	GameState* gameState;
	Lobby* lobby;
	ObjectManager* objectManager;
	AudioManager* audio;

	// Pointers to the player objects.
	Player* controlledPlayer;
	Player* otherPlayer;

	// TCP socket and listener. Used for most communication.
	sf::TcpSocket tcpSocket;
	sf::TcpListener tcpListener;

	// UDP socket. Used for position updates.
	sf::UdpSocket udpSocket;

	// Information about user's IP and port.
	sf::IpAddress myLocalIP;
	sf::IpAddress myPublicIP;
	unsigned short myPort;

	// Information about other player's IP and port.
	sf::IpAddress recipientIP;
	unsigned short recipientPort;

	// Rate at which the game ticks and rate at which the game pings the other player.
	int tickRate;
	int pingRate;

	// The round trip time between the two clients.
	int pingValue;

	// Times used for calculating round trip time.
	std::chrono::milliseconds sendTime;
	std::chrono::milliseconds receiveTime;

	// The network manager's states.
	bool isHost;
	bool connected;
	bool hostReady;
	bool clientReady;
	bool isUdpSetup;
	bool toSendCollision;

	// Details about the most recent collision received.
	sf::Vector2f collisionPos;
	sf::Vector2f collisionVel;
	float collisionTime;
	float mostRecentBallCollisionTime;

	// Details about the most recent position received.
	float mostRecentPositionTime;
	sf::Vector2f mostRecentPosition;
	sf::Vector2f mostRecentVelocity;

	// Received packets stored in deque so you can add or remove at both ends.
	// Received position details stored in a list.
	std::deque<sf::Packet> receivedPackets;
	std::list<sf::Vector2f> positionHistory;
	std::list<sf::Vector2f> velocityHistory;
	std::list<float> positionTimes;
	
	
};

