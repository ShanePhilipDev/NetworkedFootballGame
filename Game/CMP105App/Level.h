#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>

#include "Framework/Input.h"
#include "Framework/AudioManager.h"
#include "Framework/GameState.h"
#include "Lobby.h"
#include "MainMenu.h"
#include "ObjectManager.h"
#include "NetworkManager.h"

class Level{
public:
	Level(sf::RenderWindow* hwnd, Input* in, NetworkManager* nm);
	~Level();

	void handleInput(float dt);
	void update(float dt);
	void render();

private:
	// Default functions for rendering to the screen.
	void beginDraw();
	void endDraw();

	// Objects for each of the manager classes.
	sf::RenderWindow* window;
	Input* input;
	GameState gameState;
	AudioManager audio;
	Lobby lobby;
	MainMenu mainMenu;
	ObjectManager objectManager;
	NetworkManager* networkManager;
};