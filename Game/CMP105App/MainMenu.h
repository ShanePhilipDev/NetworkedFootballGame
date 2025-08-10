#pragma once
#include <iostream>
#include <SFML\Graphics\RenderWindow.hpp>

#include "Framework/GameObject.h"
#include "Framework/GameState.h"
#include "Framework/AudioManager.h"
#include "Framework/Collision.h"

#include "ObjectManager.h"
#include "Lobby.h"
#include "Cursor.h"
#include "Button.h"
#include "NetworkManager.h"


class NetworkManager;
class ObjectManager;
class Lobby;

// Main menu class. Used to switch between game states and set whether the user is the host.
class MainMenu
{
public:
	MainMenu();
	~MainMenu();

	// Main functions.
	void handleInput(float dt);
	void update(float dt);
	void render();

	// Initialise pointers and variables that rely on them.
	void init(GameState* gs, Input* in, sf::RenderWindow* hwnd, ObjectManager* om, Lobby* l, NetworkManager* nm, AudioManager* a);

private:
	// Enum for button selection.
	enum Selection { HOST = 0, JOIN, QUIT };

	// Pointers to essential objects.
	GameState* gameState;
	Input* input;
	sf::RenderWindow* window;
	AudioManager* audio;
	ObjectManager* objectManager;
	Lobby* lobby;
	NetworkManager* networkManager;
	
	// Font to be used for all text and button objects.
	sf::Font font;

	// Text object that displays the game's controls.
	sf::Text controls;

	// Colours for buttons
	sf::Color selectedColour;
	sf::Color notSelectedColour;

	// Integer to store the current selected button.
	int currentSelection;

	// Volume variables.
	int volume;
	int maxVolume;
	sf::Text volumeText;

	// Buttons.
	Button hostButton;
	Button joinButton;
	Button quitButton;
	Button increaseVolume;
	Button decreaseVolume;
	Button mute;

	// Cursor object for collisions with buttons.
	Cursor cursor;
};

