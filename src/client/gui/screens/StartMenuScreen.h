#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__StartMenuScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__StartMenuScreen_H__

#include "../Screen.h"
#include "../components/Button.h"

class StartMenuScreen: public Screen
{
public:
	StartMenuScreen();
	virtual ~StartMenuScreen();

	void init();
	void setupPositions();

	void tick();
	void render(int xm, int ym, float a);

	void buttonClicked(Button* button);
	virtual void mouseClicked(int x, int y, int buttonNum);
	bool handleBackEvent(bool isDown);
	bool isInGameScreen();
private:
	void _updateLicense();

#ifdef PLATFORM_DESKTOP
	void renderJavaTitleScreen(int xm, int ym, float a);
	void renderJavaPanorama();
#endif

	Button bHost;
	Button bJoin;
	Button bOptions;
	Button bTest;
	Button bBuy;

#ifdef PLATFORM_DESKTOP
	Button bSingleplayer;
	Button bMultiplayer;
	Button bOptionsJava;
	Button bQuit;
	int panoramaTimer;
	std::string splashText;
	unsigned int _panoramaTex;
	bool _panoramaTexReady;
#endif

	std::string copyright;
	int copyrightPosX;

	std::string version;
	int versionPosX;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__StartMenuScreen_H__*/
