#include "MainMenu.h"

MainMenu::MainMenu()
{
	// Load font
	if (!font.loadFromFile("font/arial.ttf"))
	{
		std::cout << "Error loading font.\n";
	}

	// Set colours
	notSelectedColour = sf::Color::White;
	selectedColour = sf::Color::Green;

	// Set default selection
	currentSelection = 0;

	// Set default and max volume
	volume = 50;
	maxVolume = 100;

	// A text variable with default values, to be used to initialise button and text objects.
	sf::Text buttonText;
	buttonText.setFont(font);
	buttonText.setCharacterSize(80);
	buttonText.setFillColor(notSelectedColour);
	buttonText.setOutlineColor(sf::Color::Black);
	buttonText.setOutlineThickness(2);

	// Setup each button / text object
	// ----
	buttonText.setString("HOST");
	hostButton.setText(buttonText);

	buttonText.setString("JOIN");
	joinButton.setText(buttonText);

	buttonText.setString("QUIT");
	quitButton.setText(buttonText);

	buttonText.setCharacterSize(50);
	buttonText.setString(" + ");
	increaseVolume.setText(buttonText);

	buttonText.setString(" - ");
	decreaseVolume.setText(buttonText);

	buttonText.setString("MUTE");
	mute.setText(buttonText);

	volumeText = buttonText;

	buttonText.setCharacterSize(30);
	controls = buttonText;
	controls.setString("Controls:\nWASD - Move\nSpace - Jump\nF or LMB - Shoot");
	// ----

}

MainMenu::~MainMenu()
{
}

void MainMenu::init(GameState* gs, Input* in, sf::RenderWindow* hwnd, ObjectManager* om, Lobby* l, NetworkManager* nm, AudioManager* a)
{
	// Set pointers.
	gameState = gs;
	input = in;
	window = hwnd;
	objectManager = om;
	lobby = l;
	networkManager = nm;
	audio = a;

	// Pass input pointer to cursor.
	cursor.setInput(input);
}

void MainMenu::handleInput(float dt)
{

	// Mouse input
	if (Collision::checkBoundingBox(&cursor, &hostButton)) // If the user hovers over the host button...
	{
		currentSelection = Selection::HOST; // Set the current selection to host.

		if (input->isMouseLDown()) // If the user clicks while hovering over the button...
		{
			input->setMouseLDown(false);

			// Set user as host in lobby and network manager, then switch to lobby state.
			lobby->setHost(true);
			networkManager->setHost(true);
			gameState->setCurrentState(State::LOBBY);
		}
	}
	else if (Collision::checkBoundingBox(&cursor, &joinButton)) // If the user hovers over the join button...
	{
		currentSelection = Selection::JOIN; // Set selection.

		if (input->isMouseLDown()) // If the user clicks while hovering over the button...
		{
			input->setMouseLDown(false);

			// Set user to not be the host in lobby and network manager, then switch to lobby state.
			lobby->setHost(false);
			networkManager->setHost(false);
			gameState->setCurrentState(State::LOBBY);
		}
	}
	else if (Collision::checkBoundingBox(&cursor, &quitButton)) // If the user hovers over the join button...
	{
		currentSelection = Selection::QUIT; // Set selection.

		if (input->isMouseLDown()) // If the user clicks while hovering over the button...
		{
			input->setMouseLDown(false);

			// Exit game.
			window->close();
		}
	}
	else if (Collision::checkBoundingBox(&cursor, &increaseVolume)) // Increase volume when the user clicks this button.
	{
		if (input->isMouseLDown())
		{
			input->setMouseLDown(false);

			if (volume < maxVolume) // Don't exceed maximum volume.
			{
				volume += 10;
			}
		}
	}
	else if (Collision::checkBoundingBox(&cursor, &decreaseVolume)) // Decrease volume when the user clicks this button.
	{
		if (input->isMouseLDown())
		{
			input->setMouseLDown(false);

			if (volume > 0) // Don't go below 0 volume.
			{
				volume -= 10;
			}
		}
	}
	else if (Collision::checkBoundingBox(&cursor, &mute)) // Set volume to 0 when user clicks this button.
	{
		if (input->isMouseLDown())
		{
			input->setMouseLDown(false);

			volume = 0;
		}
	}
	
}

void MainMenu::update(float dt)
{
	sf::View view(sf::FloatRect(0, 0, window->getSize().x, window->getSize().y));
	window->setView(view);

	// Update cursor position.
	cursor.update(dt);

	// Set each buttons colours.
	hostButton.setColour(notSelectedColour);
	joinButton.setColour(notSelectedColour);
	quitButton.setColour(notSelectedColour);

	// Set each button's position.
	hostButton.setButtonPosition(window->getSize().x / 2, window->getSize().y * 0.1);
	joinButton.setButtonPosition(window->getSize().x / 2, window->getSize().y * 0.3);
	quitButton.setButtonPosition(window->getSize().x / 2, window->getSize().y * 0.5);

	// Set controls' position.
	controls.setPosition(window->getSize().x * 0.02, window->getSize().y * 0.02);

	// Update volume in audio manager.
	audio->setMusicVolume(volume);
	audio->setSoundVolume(volume);

	// Setup volume buttons and text.
	volumeText.setString("Volume: " + std::to_string(volume));
	volumeText.setPosition(window->getSize().x / 2 - volumeText.getLocalBounds().width / 2, window->getSize().y * 0.9);
	decreaseVolume.setButtonSize(50, 50);
	decreaseVolume.setButtonPosition(volumeText.getPosition().x - decreaseVolume.getSize().x - 20, volumeText.getPosition().y, Button::Alignment::LEFT);
	increaseVolume.setButtonSize(50, 50);
	increaseVolume.setButtonPosition(volumeText.getPosition().x + volumeText.getLocalBounds().width + 20, volumeText.getPosition().y, Button::Alignment::LEFT);
	mute.setButtonPosition(decreaseVolume.getPosition().x - mute.getSize().x - 20, decreaseVolume.getPosition().y);

	// Change the colour of the selected button.
	switch (currentSelection)
	{
	case Selection::HOST:
		hostButton.setColour(selectedColour);
		break;
	case Selection::JOIN:
		joinButton.setColour(selectedColour);
		break;
	case Selection::QUIT:
		quitButton.setColour(selectedColour);
		break;
	}
}

void MainMenu::render()
{
	// Render buttons and text objects.
	//window->draw(hostButton);
	window->draw(hostButton.getText());

	//window->draw(joinButton);
	window->draw(joinButton.getText());
	
	//window->draw(quitButton);
	window->draw(quitButton.getText());

	window->draw(increaseVolume);
	window->draw(increaseVolume.getText());

	window->draw(decreaseVolume);
	window->draw(decreaseVolume.getText());

	window->draw(mute);
	window->draw(mute.getText());

	window->draw(volumeText);

	window->draw(controls);
}


