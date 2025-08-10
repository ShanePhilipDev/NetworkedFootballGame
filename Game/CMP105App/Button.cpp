#include "Button.h"

Button::Button()
{
}

Button::~Button()
{
}

void Button::setText(sf::Text t)
{
	// Set text then adjust size and collision box to wrap around the text.
	buttonText = t;
	setSize(sf::Vector2f(buttonText.getLocalBounds().width, buttonText.getLocalBounds().height * 1.4));
	setCollisionBox(sf::FloatRect(0, 0, getSize().x, getSize().y));
}

void Button::setButtonPosition(float x, float y, Alignment align)
{
	// Align the text, then set the position.
	if (align == Alignment::LEFT)
	{
		buttonText.setPosition(x, y);
	}
	else 
	{
		buttonText.setPosition(x - buttonText.getLocalBounds().width / 2, y);
	}

	// Move button to text's position.
	setPosition(buttonText.getPosition());
}
