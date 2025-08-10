#include "Lobby.h"

Lobby::Lobby()
{
	// Load font
	if (!font.loadFromFile("font/arial.ttf"))
	{
		std::cout << "Error loading font.\n";
	}

	// Load textures
	messi.loadFromFile("gfx/messi.png");
	ronaldo.loadFromFile("gfx/ronaldo.png");
	kirby.loadFromFile("gfx/kirby.png");
	miedema.loadFromFile("gfx/miedema.png");

	// Set previews
	leftPlayerPreview.setTexture(messi);
	leftPlayerPreview.setScale(sf::Vector2f(0.75, 0.75));
	rightPlayerPreview.setTexture(ronaldo);
	rightPlayerPreview.setScale(sf::Vector2f(0.75, 0.75));

	// Flip right preview
	sf::IntRect textureRect = rightPlayerPreview.getTextureRect();
	rightPlayerPreview.setTextureRect(sf::IntRect(textureRect.left + textureRect.width, textureRect.top, -textureRect.width, textureRect.height));
	

	// Set colours
	notSelectedColour = sf::Color::White;
	selectedColour = sf::Color::Green;

	// Set default values
	// ----
	hostChar = MESSI;
	clientChar = RONALDO;

	currentSelection = 0;

	textBoxSelection = 0;

	startCountdown = 3;
	startTimer = startCountdown;

	ready = false;
	connectionFailed = false;
	postMatch = false;
	// ----

	// Setup buttons
	// ----
	sf::Text buttonText;
	buttonText.setFont(font);
	buttonText.setCharacterSize(80);
	buttonText.setFillColor(notSelectedColour);
	buttonText.setOutlineColor(sf::Color::Black);
	buttonText.setOutlineThickness(2);

	buttonText.setString("BACK");
	backButton.setText(buttonText);

	buttonText.setString("READY");
	readyButton.setText(buttonText);

	buttonText.setCharacterSize(40);
	buttonText.setOutlineThickness(1);
	buttonText.setString("CONNECT");
	connectButton.setText(buttonText);

	buttonText.setString(" < ");
	characterBack.setText(buttonText);

	buttonText.setString(" > ");
	characterForward.setText(buttonText);

	sf::Text infoText;
	infoText.setFont(font);
	infoText.setCharacterSize(20);
	infoText.setFillColor(sf::Color::White);
	infoText.setOutlineColor(sf::Color::Black);
	infoText.setOutlineThickness(1);

	previousScore = infoText;

	infoText.setString("Host IP: 0.0.0.0");
	hostIP = infoText;

	infoText.setString("Local IP: 0.0.0.0");
	localIP = infoText;

	infoText.setString("Host Port: 0");
	hostPort = infoText;

	infoText.setString("Host Ready: NO");
	hostReady = infoText;

	infoText.setString("Client IP: N/A");
	clientIP = infoText;

	infoText.setString("Client Port: N/A");
	clientPort = infoText;

	infoText.setString("Client Ready: NO");
	clientReady = infoText;

	infoText.setString("Ping: N/A");
	ping = infoText;

	infoText.setString("Connection status: Waiting for client to connect");
	connectionStatus = infoText;

	infoText.setString("Ready Countdown: X");
	timer = infoText;

	infoText.setString("Host IP Address: ");
	connectIP = infoText;

	infoText.setString("Host Port: ");
	connectPort = infoText;

	inputIP = "";
	inputPort = "";
	infoText.setString(inputIP);
	textBoxIP.setText(infoText);
	textBoxPort.setText(infoText);

	connectionFailedText.setFont(font);
	connectionFailedText.setCharacterSize(20);
	connectionFailedText.setFillColor(sf::Color::Red);
	connectionFailedText.setOutlineColor(sf::Color::Black);
	connectionFailedText.setOutlineThickness(1);
	connectionFailedText.setString("Connection failed :(");

	textBoxSize = sf::Vector2f(200, 20);
	// ----
}

Lobby::~Lobby()
{
}

void Lobby::init(GameState* gs, Input* in, sf::RenderWindow* hwnd, AudioManager* am, ObjectManager* om, MainMenu* mm, NetworkManager* nm)
{
	// Setup pointers.
	gameState = gs;
	input = in;
	window = hwnd;
	audioManager = am;
	objectManager = om;
	mainMenu = mm;
	networkManager = nm;
	networkManager->setHost(false);
	cursor.setInput(input);
}

void Lobby::handleInput(float dt)
{
	if (Collision::checkBoundingBox(&cursor, &backButton)) // Sets current selection to back when hovering cursor over back button. Resets lobby and returns to main menu if user clicks.
	{
		currentSelection = Selection::BACK;

		if (input->isMouseLDown())
		{
			input->setMouseLDown(false);

			reset();
			gameState->setCurrentState(State::MENU);
		}
	}

	if (isHost)
	{
		if (clientConnected)
		{
			if (Collision::checkBoundingBox(&cursor, &readyButton)) // Sets current selection to ready when hovering cursor over ready button. Changes ready state then sends it to the other player.
			{
				currentSelection = Selection::READY;

				if (input->isMouseLDown())
				{
					input->setMouseLDown(false);

					ready = !ready;
					networkManager->sendReadyState(ready);
				}
			}
		}
	}
	else
	{
		if (clientConnected)
		{
			if (Collision::checkBoundingBox(&cursor, &readyButton)) // Sets current selection to ready when hovering cursor over ready button. Changes ready state then sends it to the other player when the button is pressed.
			{
				currentSelection = Selection::READY;

				if (input->isMouseLDown())
				{
					input->setMouseLDown(false);
					ready = !ready;
					networkManager->sendReadyState(ready);
				}
			}
		}
		else
		{
			if (Collision::checkBoundingBox(&cursor, &connectButton)) // Sets current selection to connect when hovering cursor over connect button. Attempts to connect if button is pressed.
			{
				currentSelection = Selection::CONNECT;

				if (input->isMouseLDown())
				{
					input->setMouseLDown(false);
					
					connect();
				}
			}
		}
	}

	if (clientConnected && !ready)
	{
		if (Collision::checkBoundingBox(&cursor, &characterBack)) // Button for changing character. Decreases the relevant character value and then sends it to the other player.
		{
			if (input->isMouseLDown())
			{
				input->setMouseLDown(false);

				if (isHost)
				{
					int currentChar = hostChar;

					if (currentChar == 0)
					{
						hostChar = Character(3);
					}
					else
					{
						hostChar = Character(currentChar - 1);
					}

					networkManager->sendCharacter(hostChar);
				}
				else
				{
					int currentChar = clientChar;

					if (currentChar == 0)
					{
						clientChar = Character(3);
					}
					else
					{
						clientChar = Character(currentChar - 1);
					}

					networkManager->sendCharacter(clientChar);
				}

			}
		}
		else if (Collision::checkBoundingBox(&cursor, &characterForward)) // Button for changing character. Increases the relevant character value and then sends it to the other player.
		{
			if (input->isMouseLDown())
			{
				input->setMouseLDown(false);

				if (isHost)
				{
					int currentChar = hostChar;

					if (currentChar < 3)
					{
						hostChar = Character(currentChar + 1);
					}
					else
					{
						hostChar = Character(0);
					}

					networkManager->sendCharacter(hostChar);
				}
				else
				{
					int currentChar = clientChar;

					if (currentChar < 3)
					{
						clientChar = Character(currentChar + 1);
					}
					else
					{
						clientChar = Character(0);
					}

					networkManager->sendCharacter(clientChar);
				}
			}
		}
	}

	if (!isHost && !clientConnected)
	{
		if (Collision::checkBoundingBox(&cursor, &textBoxIP)) // When you click on the IP textbox, the selected text box changes.
		{
			if (input->isMouseLDown())
			{
				input->setMouseLDown(false);

				textBoxSelection = 0;
			}
		}
		else if (Collision::checkBoundingBox(&cursor, &textBoxPort)) // When you click on the port textbox, the selected text box changes.
		{
			if (input->isMouseLDown())
			{
				input->setMouseLDown(false);

				textBoxSelection = 1;
			}
		}

		if (input->isKeyDown(sf::Keyboard::Tab)) // Pressing tab switches between text boxes.
		{
			input->setKeyUp(sf::Keyboard::Tab);
		
			if (textBoxSelection == 0)
			{
				textBoxSelection = 1;
			}
			else if(textBoxSelection == 1)
			{
				textBoxSelection = 0;
			}
		}

		if (input->isKeyDown(sf::Keyboard::Enter)) // If you press enter on the IP text box, switch to port text box. Otherwise attempt to connect.
		{
			input->setKeyUp(sf::Keyboard::Enter);

			if (textBoxSelection == 0)
			{
				textBoxSelection = 1;
			}
			else if (textBoxSelection == 1)
			{
				connect();
			}
		}

		// Text input.
		if ((inputIP.size() < 15 && textBoxSelection == 0) || (inputPort.size() < 5 && textBoxSelection == 1)) // Limit size of string for each text box.
		{
			// Get keypresses and set them to char if they are valid inputs.
			// ----
			char c = NULL;
			for (int i = sf::Keyboard::Num0; i <= sf::Keyboard::Num9; i++)
			{
				if (input->isKeyDown(i))
				{
					input->setKeyUp(i);

					c = char(i + 22);
				}
			}

			for (int i = sf::Keyboard::Numpad0; i <= sf::Keyboard::Numpad9; i++)
			{
				if (input->isKeyDown(i))
				{
					input->setKeyUp(i);

					c = char(i - 27);
				}
			}

			if (input->isKeyDown(sf::Keyboard::Period))
			{
				input->setKeyUp(sf::Keyboard::Period);

				c = '.';
			}
			// ----

			// Add char to the relevant string if valid input was entered.
			if (c != NULL)
			{
				if (textBoxSelection == 0)
				{
					inputIP += c;
				}
				else if (textBoxSelection == 1)
				{
					inputPort += c;
				}
			}
		}

		// Delete last char in string if backspace is pressed.
		if (input->isKeyDown(sf::Keyboard::Backspace))
		{
			input->setKeyUp(sf::Keyboard::Backspace);

			if (inputIP.size() > 0 && textBoxSelection == 0)
			{
				inputIP.pop_back();
			}
			else if (inputPort.size() > 0 && textBoxSelection == 1)
			{
				inputPort.pop_back();
			}
		}
	}
}

void Lobby::update(float dt)
{
	sf::View view(sf::FloatRect(0, 0, window->getSize().x, window->getSize().y));
	window->setView(view);

	// Update cursor position.
	cursor.update(dt);

	// Get connection status from network manager.
	clientConnected = networkManager->getConnected();

	// Set textures for host and client character previews.
	// ----
	switch (hostChar)
	{
	case MESSI:
		leftPlayerPreview.setTexture(messi);
		break;
	case RONALDO:
		leftPlayerPreview.setTexture(ronaldo);
		break;
	case KIRBY:
		leftPlayerPreview.setTexture(kirby);
		break;
	case MIEDEMA:
		leftPlayerPreview.setTexture(miedema);
		break;
	}

	switch (clientChar)
	{
	case MESSI:
		rightPlayerPreview.setTexture(messi);
		break;
	case RONALDO:
		rightPlayerPreview.setTexture(ronaldo);
		break;
	case KIRBY:
		rightPlayerPreview.setTexture(kirby);
		break;
	case MIEDEMA:
		rightPlayerPreview.setTexture(miedema);
		break;
	}
	// ----

	if (networkManager->getReadyStatus()) // If both players are ready in the network manager...
	{
		// If the countdown hasn't been synced yet, the host will sync it and reset the ball and player positions in game.
		if (networkManager->getHost() && !countdownSynced)
		{
			networkManager->syncCountdown();
			countdownSynced = true;
			objectManager->goalReset();
		}

		// Decrease timer by delta time.
		startTimer -= dt;

		// Once the timer reaches 0, start the game. Both clients' times should be synced at this point, so the game should start at the same time.
		if (startTimer < 0)
		{
			gameState->setCurrentState(State::LEVEL);
			objectManager->start();
		}
	}
	else
	{
		// If at least one of the players isn't ready, reset countdown timer and sync status.
		startTimer = startCountdown;
		countdownSynced = false;
	}

	// Setup buttons and text.
	setupButtonsText();
}

void Lobby::render()
{
	if (isHost) // Draw local IP if host. Draw back button, and ready button if there is a client connected.
	{
		window->draw(localIP);
		window->draw(backButton.getText());
		if (clientConnected)
		{
			window->draw(readyButton.getText());
		}
		
	}
	else // Client draws back and ready button if connected, otherwise draws back and connect buttons.
	{
		if (clientConnected)
		{
			window->draw(backButton.getText());

			window->draw(readyButton.getText());
		}
		else
		{
			window->draw(backButton.getText());

			window->draw(connectButton.getText());
		}
	}

	// Render status texts
	if (isHost || clientConnected) // Draw the network status text objects, and character preview objects.
	{
		window->draw(hostIP);
		window->draw(hostPort);
		window->draw(hostReady);
		window->draw(clientIP);
		window->draw(clientPort);
		window->draw(clientReady);
		window->draw(ping);
		window->draw(connectionStatus);
		window->draw(timer);

		if (clientConnected)
		{
			window->draw(leftPlayerPreview);
			window->draw(rightPlayerPreview);
			window->draw(characterBack);
			window->draw(characterBack.getText());
			window->draw(characterForward);
			window->draw(characterForward.getText());
		}
		
		if (postMatch == true) // Draw previous score if in a post match lobby.
		{
			window->draw(previousScore);
		}
	}
	else if (!isHost && !clientConnected) // If not connected to a host, draw the connect screen objects.
	{
		window->draw(connectIP);
		window->draw(connectPort);
		window->draw(textBoxIP);
		window->draw(textBoxIP.getText());
		window->draw(textBoxPort);
		window->draw(textBoxPort.getText());

		if (connectionFailed) // Draw connection failed text when you fail to connect.
		{
			window->draw(connectionFailedText);
		}
	}
}

// Reset values back to default.
void Lobby::reset()
{
	isHost = false; 
	networkManager->setHost(false);
	clientConnected = false;
	connectionFailed = false;
	postMatch = false;
	countdownSynced = false;

	inputIP = "";
	inputPort = "";
	
	textBoxSelection = 0;
	currentSelection = 0;
	
	networkManager->reset();
	networkManager->disconnect();
	
	hostChar = MESSI;
	clientChar = RONALDO;
}

bool Lobby::connect()
{
	if (inputIP != "" && inputPort != "") // Check IP and port are not blank to prevent crash.
	{
		// Attempt to connect to the other player using the user's entered IP and port.
		if (networkManager->connect(sf::IpAddress(inputIP), std::stoi(inputPort)))
		{
			// If successful, set statuses and return true.
			clientConnected = true;
			connectionFailed = false;
			return true;
		}
		else
		{
			// If unsuccessful, set statuses, reset text input and return false.
			clientConnected = false;
			connectionFailed = true;
			inputIP = "";
			inputPort = "";
			textBoxSelection = 0;
			return false;
		}
		return false;
	}
	
}

void Lobby::setPostMatchLobby(int leftScore, int rightScore)
{
	// Set score strings, reset ready status and timer, then set post match to be true.
	score = "Last Match Result: " + std::to_string(leftScore) + " - " + std::to_string(rightScore);
	ready = false;
	startTimer = startCountdown;
	postMatch = true;
	networkManager->sendReadyState(ready); // Send ready state to other player so that the ready statuses are synced.
}

void Lobby::setOpponentChar(sf::Packet p)
{
	// Extract character num from the packet
	int n;
	p >> n;

	// Set the client's character if host is receiving, otherwise set the host's character.
	if (isHost)
	{
		clientChar = Character(n);
	}
	else
	{
		hostChar = Character(n);
	}
}

void Lobby::setupButtonsText()
{
	// Setting button positions, colours, strings, etc
	// ----
	backButton.setColour(notSelectedColour);
	connectButton.setColour(notSelectedColour);
	readyButton.setColour(notSelectedColour);

	switch (currentSelection)
	{
	case Selection::BACK:
		backButton.setColour(selectedColour);
		break;
	case Selection::CONNECT:
		connectButton.setColour(selectedColour);
		break;
	case Selection::READY:
		readyButton.setColour(selectedColour);
		break;
	}

	connectionFailedText.setPosition(window->getSize().x * 0.4, window->getSize().y * 0.50);

	backButton.setButtonPosition(window->getSize().x * 0.1, window->getSize().y * 0.8);
	connectButton.setButtonPosition(window->getSize().x * 0.4 + connectButton.getSize().x * 0.5, window->getSize().y * 0.55);
	readyButton.setButtonPosition(window->getSize().x * 0.5, window->getSize().y * 0.8);

	if (isHost)
	{
		localIP.setPosition(window->getSize().x * 0.01, window->getSize().y * 0.05);
		hostIP.setPosition(window->getSize().x * 0.01, window->getSize().y * 0.10);
		hostPort.setPosition(window->getSize().x * 0.01, window->getSize().y * 0.15);
		hostReady.setPosition(window->getSize().x * 0.01, window->getSize().y * 0.20);
	}
	else
	{
		hostIP.setPosition(window->getSize().x * 0.01, window->getSize().y * 0.05);
		hostPort.setPosition(window->getSize().x * 0.01, window->getSize().y * 0.10);
		hostReady.setPosition(window->getSize().x * 0.01, window->getSize().y * 0.15);
	}

	clientIP.setPosition(window->getSize().x * 0.99 - clientIP.getLocalBounds().width, window->getSize().y * 0.05);
	clientPort.setPosition(window->getSize().x * 0.99 - clientPort.getLocalBounds().width, window->getSize().y * 0.10);
	clientReady.setPosition(window->getSize().x * 0.99 - clientReady.getLocalBounds().width, window->getSize().y * 0.15);

	ping.setPosition(window->getSize().x * 0.5 - ping.getLocalBounds().width * 0.5, window->getSize().y * 0.05);
	connectionStatus.setPosition(window->getSize().x * 0.5 - connectionStatus.getLocalBounds().width * 0.5, window->getSize().y * 0.10);
	timer.setPosition(window->getSize().x * 0.5 - timer.getLocalBounds().width * 0.5, readyButton.getPosition().y - 30);

	connectIP.setPosition(window->getSize().x * 0.4, window->getSize().y * 0.4);
	connectPort.setPosition(window->getSize().x * 0.4, window->getSize().y * 0.45);

	previousScore.setString(score);
	previousScore.setPosition(window->getSize().x * 0.5 - previousScore.getLocalBounds().width * 0.5, window->getSize().y * 0.15);

	leftPlayerPreview.setPosition(window->getSize().x * 0.25 - leftPlayerPreview.getLocalBounds().width / 2, window->getSize().y * 0.5 - leftPlayerPreview.getLocalBounds().height / 2);
	rightPlayerPreview.setPosition(window->getSize().x * 0.75 - rightPlayerPreview.getLocalBounds().width / 2, window->getSize().y * 0.5 - rightPlayerPreview.getLocalBounds().height / 2);

	if (isHost)
	{
		characterBack.setButtonPosition(leftPlayerPreview.getPosition().x, leftPlayerPreview.getPosition().y + leftPlayerPreview.getLocalBounds().height);
		characterForward.setButtonPosition(leftPlayerPreview.getPosition().x + leftPlayerPreview.getLocalBounds().width - characterForward.getLocalBounds().width, leftPlayerPreview.getPosition().y + leftPlayerPreview.getLocalBounds().height);
	}
	else
	{
		characterBack.setButtonPosition(rightPlayerPreview.getPosition().x, rightPlayerPreview.getPosition().y + rightPlayerPreview.getLocalBounds().height);
		characterForward.setButtonPosition(rightPlayerPreview.getPosition().x + rightPlayerPreview.getLocalBounds().width - characterForward.getLocalBounds().width, rightPlayerPreview.getPosition().y + rightPlayerPreview.getLocalBounds().height);
	}


	if (isHost)
	{
		localIP.setString("Local IP: " + networkManager->getMyLocalIP());
		hostIP.setString("Host IP: " + networkManager->getMyPublicIP());
		hostPort.setString("Host Port: " + std::to_string(networkManager->getMyPort()));

		clientIP.setString("Client IP: " + networkManager->getRecipientIP());
		clientPort.setString("Client Port: " + std::to_string(networkManager->getRecipientPort()));
	}
	else
	{
		hostIP.setString("Host IP: " + networkManager->getRecipientIP());
		hostPort.setString("Host Port: " + std::to_string(networkManager->getRecipientPort()));

		clientIP.setString("Client IP: " + networkManager->getMyPublicIP());
		clientPort.setString("Client Port: " + std::to_string(networkManager->getMyPort()));
	}

	if (textBoxSelection == 0)
	{
		sf::Text text;
		text = textBoxIP.getText();
		text.setString(">" + inputIP);
		textBoxIP.setText(text);

		text = textBoxPort.getText();
		text.setString(inputPort);
		textBoxPort.setText(text);
	}
	else
	{
		sf::Text text;
		text = textBoxIP.getText();
		text.setString(inputIP);
		textBoxIP.setText(text);

		text = textBoxPort.getText();
		text.setString(">" + inputPort);
		textBoxPort.setText(text);
	}

	textBoxIP.setButtonSize(textBoxSize.x, textBoxSize.y);
	textBoxPort.setButtonSize(textBoxSize.x, textBoxSize.y);

	textBoxIP.setButtonPosition((window->getSize().x * 0.4) + connectIP.getLocalBounds().width + 5, window->getSize().y * 0.4 + 3, Button::Alignment::LEFT);
	textBoxPort.setButtonPosition((window->getSize().x * 0.4) + connectPort.getLocalBounds().width + 5, window->getSize().y * 0.45 + 3, Button::Alignment::LEFT);

	if (clientConnected)
	{
		connectionStatus.setString("Connection status: Connected");
	}
	else
	{
		connectionStatus.setString("Connection status: Waiting for opponent...");
	}

	if (networkManager->getClientReady())
	{
		clientReady.setString("Client Ready: YES");
	}
	else
	{
		clientReady.setString("Client Ready: NO");
	}

	if (networkManager->getHostReady())
	{
		hostReady.setString("Host Ready: YES");
	}
	else
	{
		hostReady.setString("Host Ready: NO");
	}

	std::stringstream stream;
	stream << "Ready Countdown: " << std::fixed << std::setprecision(2) << startTimer;
	std::string string = stream.str();
	timer.setString(string);

	ping.setString("Ping: " + std::to_string(networkManager->getPing()));
	// ----
}