#pragma once
#include "Framework/GameObject.h"

// Class derived from GameObject
class Cursor :
	public GameObject
{
public:
	// Constructor and destructor
	Cursor();
	~Cursor();

	// Update function
	void update(float dt) override;

private:
	// Texture of the mouse cursor
	sf::Texture cursorTexture;
};

