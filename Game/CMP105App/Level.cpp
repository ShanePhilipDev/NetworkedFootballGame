#include "Level.h"

Level::Level(sf::RenderWindow* hwnd, Input* in, NetworkManager* nm)
{
	window = hwnd;
	input = in;
	networkManager = nm;

	// Initialise menus and management classes.
	mainMenu.init(&gameState, input, window, &objectManager, &lobby, networkManager, &audio);
	lobby.init(&gameState, input, window, &audio, &objectManager, &mainMenu, networkManager);
	objectManager.init(hwnd, input, networkManager, &gameState, &lobby, &audio);
	networkManager->init(&gameState, &lobby, &objectManager, &audio);

	// The game will start at the main menu.
	gameState.setCurrentState(State::MENU);

}

Level::~Level()
{

}

// Handle user input
void Level::handleInput(float dt)
{
	// Switch statement to control what inputs are handled based on the current game state.
	// Each state has their own corresponding object that will handle input.
	switch (gameState.getCurrentState())
	{
	case State::MENU:
		mainMenu.handleInput(dt);
		break;
	case State::LEVEL:
		objectManager.handleInput(dt);
		break;
	case State::LOBBY:
		lobby.handleInput(dt);
		break;
	}
}

// Update game objects
void Level::update(float dt)
{
	// If no music is playing, play music.
	if (audio.getMusic()->getStatus() == sf::Music::Status::Stopped)
	{
		audio.playMusicbyName("music");
	}

	// Switch statement to control what objects are updated based on the current game state.
	switch (gameState.getCurrentState())
	{
	case State::MENU:
		mainMenu.update(dt);
		break;
	case State::LEVEL:
		objectManager.update(dt);
		break;
	case State::LOBBY:
		lobby.update(dt);
		break;
	}
	
}

// Render level
void Level::render()
{
	beginDraw();

	// Switch statement to control what objects are rendered based on the current game state.
	switch (gameState.getCurrentState())
	{
	case State::MENU:
		mainMenu.render();
		break;
	case State::LEVEL:
		objectManager.render();
		break;
	case State::LOBBY:
		lobby.render();
		break;
	}

	endDraw();
}

// Begins rendering to the back buffer. Background colour set to blue.
void Level::beginDraw()
{
	window->clear(sf::Color::Blue); 
}

// Ends rendering to the back buffer, and swaps buffer to the screen.
void Level::endDraw()
{
	window->display();
}