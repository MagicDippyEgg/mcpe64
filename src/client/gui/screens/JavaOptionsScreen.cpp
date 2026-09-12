#include "JavaOptionsScreen.h"

#include "../../Minecraft.h"
#include "ScreenChooser.h"
#include "../../../platform/input/Keyboard.h"

JavaOptionsScreen::JavaOptionsScreen(JavaOptionsBack backTarget)
:	super(),
	backTarget(backTarget),
	doneButton(NULL)
{
}

JavaOptionsScreen::~JavaOptionsScreen()
{
	delete doneButton;
	for (unsigned int i = 0; i < optionButtons.size(); ++i)
		delete optionButtons[i];
	optionButtons.clear();
}

void JavaOptionsScreen::init()
{
	options.push_back(&Options::Option::INVERT_MOUSE);
	options.push_back(&Options::Option::VIEW_BOBBING);
	options.push_back(&Options::Option::DIFFICULTY);
	options.push_back(&Options::Option::GRAPHICS);
	options.push_back(&Options::Option::THIRD_PERSON);
	options.push_back(&Options::Option::AMBIENT_OCCLUSION);
	options.push_back(&Options::Option::LIMIT_FRAMERATE);
	options.push_back(&Options::Option::ANAGLYPH);

	for (unsigned int i = 0; i < options.size(); ++i) {
		optionButtons.push_back(new Button((int)options.size(), 0, 0, 150, 20, optionLabel(options[i])));
		buttons.push_back(optionButtons.back());
		tabButtons.push_back(optionButtons.back());
	}

	doneButton = new Button(99, 0, 0, 200, 20, "Done");
	buttons.push_back(doneButton);
	tabButtons.push_back(doneButton);
}

void JavaOptionsScreen::setupPositions()
{
	for (unsigned int i = 0; i < optionButtons.size(); ++i) {
		optionButtons[i]->x = width / 2 - 155 + (int)(i % 2) * 160;
		optionButtons[i]->y = height / 6 - 12 + 24 * (int)(i / 2);
		optionButtons[i]->width = 150;
		optionButtons[i]->height = 20;
	}

	doneButton->x = width / 2 - 100;
	doneButton->y = height / 6 + 168;
	doneButton->width = 200;
	doneButton->height = 20;
}

std::string JavaOptionsScreen::optionLabel(const Options::Option* option)
{
	std::string label = "Option";

	if (option == &Options::Option::INVERT_MOUSE)
		label = "Invert Mouse";
	else if (option == &Options::Option::VIEW_BOBBING)
		label = "View Bobbing";
	else if (option == &Options::Option::DIFFICULTY)
		label = "Difficulty";
	else if (option == &Options::Option::GRAPHICS)
		label = "Graphics";
	else if (option == &Options::Option::THIRD_PERSON)
		label = "Third Person View";
	else if (option == &Options::Option::AMBIENT_OCCLUSION)
		label = "Ambient Occlusion";
	else if (option == &Options::Option::LIMIT_FRAMERATE)
		label = "Limit Framerate";
	else if (option == &Options::Option::ANAGLYPH)
		label = "Anaglyph 3D";

	if (option == &Options::Option::DIFFICULTY) {
		int d = minecraft->options.getIntValue(option) & 3;
		const char* name = "Peaceful";
		if (d == 1) name = "Easy";
		else if (d == 2) name = "Normal";
		else if (d == 3) name = "Hard";
		return label + ": " + name;
	}

	if (option == &Options::Option::GRAPHICS)
		return label + ": " + (minecraft->options.fancyGraphics? "Fancy" : "Fast");

	return label + ": " + (minecraft->options.getBooleanValue(option)? "ON" : "OFF");
}

void JavaOptionsScreen::refreshOptionButton(unsigned int index)
{
	if (index >= optionButtons.size())
		return;
	optionButtons[index]->msg = optionLabel(options[index]);
}

void JavaOptionsScreen::buttonClicked(Button* button)
{
	if (button == doneButton) {
		minecraft->options.save();
		minecraft->screenChooser.setScreen(backTarget == JAVA_OPTIONS_BACK_TO_MENU? SCREEN_STARTMENU : SCREEN_PAUSE);
		return;
	}

	for (unsigned int i = 0; i < optionButtons.size(); ++i) {
		if (button == optionButtons[i]) {
			minecraft->options.toggle(options[i], 1);
			refreshOptionButton(i);
			return;
		}
	}

	super::buttonClicked(button);
}

void JavaOptionsScreen::keyPressed(int eventKey)
{
	if (eventKey == Keyboard::KEY_ESCAPE) {
		minecraft->options.save();
		minecraft->screenChooser.setScreen(backTarget == JAVA_OPTIONS_BACK_TO_MENU? SCREEN_STARTMENU : SCREEN_PAUSE);
		return;
	}

	super::keyPressed(eventKey);
}

void JavaOptionsScreen::render(int xm, int ym, float a)
{
	renderBackground();
	drawCenteredString(font, "Options", width / 2, 15, 0xffffff);
	super::render(xm, ym, a);
}