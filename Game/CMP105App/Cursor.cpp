#include "Cursor.h"

Cursor::Cursor()
{
	cursorTexture.loadFromFile("gfx/icon.png"); // load texture
	setTexture(&cursorTexture); // set texture
	setSize(sf::Vector2f(48, 48)); // set size - dimensions used are the pixel dimensions of the texture's file
	setInput(input); // set input so that we can track the mouses position
	setCollisionBox(sf::FloatRect(4, 4, 8, 8)); // the cursor's hitbox will be in the top left corner of the object, where the cursor points
}

Cursor::~Cursor()
{
}

void Cursor::update(float dt)
{
	// set the position of the cursor to where the mouse cursor's position is
	setPosition(input->getMouseX(), input->getMouseY());
}
