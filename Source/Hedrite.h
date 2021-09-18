#pragma once
#include <JuceHeader.h>
#include "OpenGLWindow.h"

class Hedrite {
public:
	static Hedrite* instance;
	static void Hedrite::openGLCallback();

	std::unique_ptr<OpenGLWindow> openGLWindow;

	Hedrite();
	~Hedrite();
	void initialize();
	void mounted();
};

