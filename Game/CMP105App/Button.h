#pragma once
#include "Framework/GameObject.h"

// Button class - no networking. Used for navigating menus and can be used as a text box by modifying the button's text. Inherits from GameObject for collision checking.
class Button : public GameObject
{
public:
	Button();
	~Button();

	// Text alignment enum.
	enum class Alignment { LEFT, CENTRE, RIGHT };

	// Set button's text attribute.
	void setText(sf::Text t);
	
	// Set the colour of the button's text.
	void setColour(sf::Color c) 
	{
		buttonText.setFillColor(c);
	};

	// Set the button's position and optionally, the alignment of the text within the button.
	void setButtonPosition(float x, float y, Alignment align = Alignment::CENTRE);

	// Sets size of the button and adjusts the collision box accordingly.
	void setButtonSize(float x, float y) 
	{
		setSize(sf::Vector2f(x, y));
		setCollisionBox(sf::FloatRect(0, 0, getSize().x, getSize().y));
	};

	// Returns the button's text variable.
	sf::Text getText() 
	{
		return buttonText;
	};

private:
	sf::Text buttonText;
};

