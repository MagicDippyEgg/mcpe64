#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__JavaOptionsScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__JavaOptionsScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "../../Options.h"

#include <vector>

// Minecraft Java 1.4.7-style options menu, used on desktop builds only.
// Android/iOS keep their touch options screens.
class JavaOptionsScreen: public Screen
{
	typedef Screen super;
public:
	JavaOptionsScreen(Screen* parentScreen);
	virtual ~JavaOptionsScreen();

	void init();
	void setupPositions();
	void render(int xm, int ym, float a);

	void buttonClicked(Button* button);
	void keyPressed(int eventKey);
private:
	std::string optionLabel(const Options::Option* option);
	void refreshOptionButton(unsigned int index);

	Screen* parentScreen;
	std::vector<const Options::Option*> options;
	std::vector<Button*> optionButtons;
	Button* doneButton;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__JavaOptionsScreen_H__*/