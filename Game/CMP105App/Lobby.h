#pragma once
#include <iostream>
#include <iomanip>
#include <sstream>
#include <SFML\Graphics\RenderWindow.hpp>

#include "Framework/GameObject.h"
#include "Framework/GameState.h"
#include "Framework/AudioManager.h"
#include "Framework/Collision.h"

#include "ObjectManager.h"
#include "MainMenu.h"
#include "Cursor.h"
#include "Button.h"
#include "NetworkManager.h"

class MainMenu;
class NetworkManager;
class ObjectManager;

// Lobby class. This handles some of the interaction between the user and the network manager class. Here the user can either host a game or connect to a host. 
// Once they have connected they can change character and set their ready status, which will then be synced via the network manager.
// Once both players are ready, a countdown will start which will switch both players into the actual game.
class Lobby
{
public:
	Lobby();
	~Lobby();

	// Main functions.
	void handleInput(float dt);
	void update(float dt);
	void render();

	// Initialise pointers and variables that rely on the pointers.
	void init(GameState* gs, Input* in, sf::RenderWindow* hwnd, AudioManager* am, ObjectManager* om, MainMenu* mm, NetworkManager* nm);

	// Setter functions
	// ----
	void setHost(bool b) {
		isHost = b;
	};

	void setCountdownTimer(float t)
	{
		startTimer = t;
	};

	// Sets the character controlled by the other player. This is called in the network manager when it receives a packet containing the selected character.
	void setOpponentChar(sf::Packet p);
	// ----

	// Getter functions
	// ----
	float getCountdownTimer()
	{
		return startTimer;
	};

	int getHostChar()
	{
		return hostChar;
	};

	int getClientChar()
	{
		return clientChar;
	};

	bool getReady()
	{
		return ready;
	};
	// ----

	// Resets all variables in the lobby.
	void reset();

	// Attempts to connect to a host.
	bool connect();

	// Sets the lobby into post match mode (displays the previous score).
	void setPostMatchLobby(int leftScore, int rightScore);

	// Sets positions, strings, etc. for all of the buttons and text objects.
	void setupButtonsText();

	// Character enum. Decides which texture to use for each player. This is public as the object manager uses this in game when setting up players.
	enum Character { MESSI = 0, RONALDO, KIRBY, MIEDEMA };
private:
	// Button selection.
	enum Selection { BACK = 0, CONNECT, READY };
	
	// Pointers to objects needed in the class.
	GameState* gameState;
	Input* input;
	sf::RenderWindow* window;
	AudioManager* audioManager;
	ObjectManager* objectManager;
	MainMenu* mainMenu;
	NetworkManager* networkManager;

	// Font to be used for all buttons and text.
	sf::Font font;

	// Button colours.
	sf::Color selectedColour;
	sf::Color notSelectedColour;

	// Current selected button.
	int currentSelection;

	// Cursor for interacting with buttons.
	Cursor cursor;

	// Text objects to display information about the network and game.
	sf::Text localIP;
	sf::Text hostIP; 
	sf::Text hostPort; 
	sf::Text hostReady;
	sf::Text clientIP; 
	sf::Text clientPort; 
	sf::Text clientReady;
	sf::Text ping; 
	sf::Text connectionStatus; 
	sf::Text timer;
	sf::Text connectionFailedText;
	sf::Text previousScore;
	std::string score;

	// Integer to store which text box is selected.
	int textBoxSelection;

	// Only used if not host. For displaying entered IP and port when joining a server. Buttons for entering text and connecting.
	sf::Text connectIP;
	sf::Text connectPort;
	std::string inputIP;
	std::string inputPort;
	sf::Vector2f textBoxSize;
	Button textBoxIP;
	Button textBoxPort;
	Button connectButton;

	// Buttons for leaving the lobby and readying up.
	Button backButton;
	Button readyButton;

	// Lobby state bools.
	bool isHost;
	bool clientConnected;
	bool connectionFailed;
	bool ready;
	bool postMatch;

	// Variables relating to the ready countdown.
	float startCountdown;
	float startTimer;
	bool countdownSynced;

	// Enum to store the host's and client's characters.
	Character hostChar;
	Character clientChar;

	// Sprites to preview the selected characters.
	sf::Sprite leftPlayerPreview;
	sf::Sprite rightPlayerPreview;

	// Textures for each of the players.
	sf::Texture messi;
	sf::Texture ronaldo;
	sf::Texture kirby;
	sf::Texture miedema;

	// Buttons for switching between characters.
	Button characterBack;
	Button characterForward;

};

