#include "StartMenuScreen.h"
#include "UsernameScreen.h"
#include "SelectWorldScreen.h"
#include "ProgressScreen.h"
#include "JoinGameScreen.h"
#include "OptionsScreen.h"
#include "JavaOptionsScreen.h"
#include "PauseScreen.h"
#include "InvalidLicenseScreen.h"
#include "PrerenderTilesScreen.h" // test button
//#include "BuyGameScreen.h"

#include "../../../util/Mth.h"
#include "../../../platform/time.h"

#include "../Font.h"
#include "../components/SmallButton.h"
#include "../components/ScrolledSelectionList.h"

#include "../../Minecraft.h"
#include "../../renderer/Tesselator.h"
#include "../../../AppPlatform.h"
#include "../../../LicenseCodes.h"
#include "SimpleChooseLevelScreen.h"
#include "../../renderer/Textures.h"
#include "../../../SharedConstants.h"
#include "../../sound/SoundEngine.h"

#ifdef PLATFORM_DESKTOP
#include <vector>
#endif

#ifdef PLATFORM_DESKTOP
// Java's splashes.txt hashCode for "missingno" (excluded from the rotation)
static int javaSplashHash(const std::string& s)
{
	int h = 0;
	for (unsigned int i = 0; i < s.size(); ++i)
		h = 31 * h + (signed char)s[i];
	return h;
}
#endif

// Some kind of default settings, might be overridden in ::init
StartMenuScreen::StartMenuScreen()
:	bHost(    2, 0, 0, 160, 24, "Start Game"),
	bJoin(    3, 0, 0, 160, 24, "Join Game"),
	bOptions( 4, 0, 0,  78, 22, "Options"),
	bBuy(     5, 0, 0, 78, 22, "Buy"),
	bTest(    999, 0, 0, 78, 22, "Create")
#ifdef PLATFORM_DESKTOP
	, bSingleplayer(1, "Singleplayer"),
	bMultiplayer(2, "Multiplayer"),
	bOptionsJava(0, 0, 0, 98, 20, "Options"),
	bQuit(4, 0, 0, 98, 20, "Quit"),
	panoramaTimer(0),
	_panoramaTex(0),
	_panoramaTexReady(false)
#endif
{
}

StartMenuScreen::~StartMenuScreen()
{
#ifdef PLATFORM_DESKTOP
	if (_panoramaTexReady)
		glDeleteTextures(1, &_panoramaTex);
#endif
}

void StartMenuScreen::init()
{
	if (minecraft->options.username.empty()) {
		return; // tick() will redirect to UsernameScreen
	}

#ifdef PLATFORM_DESKTOP
	buttons.push_back(&bSingleplayer);
	buttons.push_back(&bMultiplayer);
	buttons.push_back(&bOptionsJava);
	buttons.push_back(&bQuit);
	tabButtons.insert(tabButtons.end(), buttons.begin(), buttons.end());
	bSingleplayer.active = bMultiplayer.active = bOptionsJava.active = bQuit.active = true;

	// Pick a splash exactly like Java: one random line, only once, from the real splashes.txt
	if (splashText.empty()) {
		BinaryBlob blob = minecraft->platform()->readAssetFile("images/java/splashes.txt");
		if (blob.data && blob.size > 0) {
			std::vector<std::string> candidates;
			std::string line;
			for (unsigned int i = 0; i < blob.size; ++i) {
				const char c = (char)blob.data[i];
				if (c == '\n') {
					if (!line.empty())
						candidates.push_back(line);
					line.clear();
				} else if (c != '\r') {
					line += c;
				}
			}
			if (!line.empty())
				candidates.push_back(line);

			unsigned int valid = 0;
			for (unsigned int i = 0; i < candidates.size(); ++i)
				if (javaSplashHash(candidates[i]) != 125780783)
					++valid;
			if (valid == 0)
				splashText = "missingno!";
			else {
				unsigned int pick = (unsigned int)Mth::random((int)valid);
				splashText = candidates[0];
				for (unsigned int i = 0; i < candidates.size(); ++i) {
					if (javaSplashHash(candidates[i]) == 125780783)
						continue;
					if (pick == 0) {
						splashText = candidates[i];
						break;
					}
					--pick;
				}
			}
		}
		if (splashText.empty())
			splashText = "missingno!";
	}

	if (minecraft->soundEngine)
		minecraft->soundEngine->playMenuMusic();
	return;
#endif

	buttons.push_back(&bHost);
	buttons.push_back(&bJoin);
	//buttons.push_back(&bTest);

	tabButtons.push_back(&bHost);
	tabButtons.push_back(&bJoin);

	#ifndef RPI
		buttons.push_back(&bOptions);
		tabButtons.push_back(&bOptions);
	#endif

	#ifdef DEMO_MODE
		buttons.push_back(&bBuy);
		tabButtons.push_back(&bBuy);
	#endif

	copyright = "\xffMojang AB";//. Do not distribute!";

	// always show base version string, suffix was previously added for Android builds
	std::string versionString = Common::getGameVersionString();

	#ifdef DEMO_MODE
	#ifdef __APPLE__
		version = versionString + " (Lite)";
	#else
		version = versionString + " (Demo)";
	#endif
	#else
		#ifdef RPI
			version = "v0.1.1 alpha";//(MCPE " + versionString + " compatible)";
		#else
			version = versionString;
		#endif
	#endif

	bJoin.active = bHost.active = bOptions.active = false;
}

void StartMenuScreen::setupPositions() {
	int yBase;
#ifdef PLATFORM_DESKTOP
	// Minecraft Java 1.4.7 title menu layout:
	// Singleplayer (w/2-100, var4, 200x20), Multiplayer (var4+24),
	// Options & Quit together as a 98x20 pair at var4+72+12.
	const int var4 = height / 4 + 48;

	bSingleplayer.width = 200;
	bSingleplayer.height = 20;
	bMultiplayer.width = 200;
	bMultiplayer.height = 20;

	bSingleplayer.x = (width - 200) / 2;
	bSingleplayer.y = var4;
	bMultiplayer.x = (width - 200) / 2;
	bMultiplayer.y = var4 + 24;
	bOptionsJava.x = width / 2 - 98;
	bOptionsJava.y = var4 + 72 + 12;
	bQuit.x = width / 2 + 2;
	bQuit.y = var4 + 72 + 12;

	copyrightPosX = width - minecraft->font->width("Copyright Mojang AB. Do not distribute!") - 2;
	versionPosX = 2;
	return;
#endif

	yBase = height / 2 + 25;

	//#ifdef ANDROID
	bHost.y =	 yBase - 28;
#ifdef RPI
	bJoin.y =	 yBase + 4;
#else
	bJoin.y =	 yBase;
#endif

	bOptions.y = yBase + 28 + 2;
	bTest.y = bBuy.y = bOptions.y;
	//#endif

	// Center buttons
	bHost.x = (width - bHost.width) / 2;
	bJoin.x = (width - bJoin.width) / 2;
	bOptions.x = (width - bJoin.width) / 2;
	bTest.x = bBuy.x = bOptions.x + bOptions.width + 4;

	copyrightPosX = width - minecraft->font->width(copyright) - 1;
	versionPosX = (width - minecraft->font->width(version)) / 2;// - minecraft->font->width(version) - 2;
}

void StartMenuScreen::tick() {
	if (minecraft->options.username.empty()) {
		minecraft->setScreen(new UsernameScreen());
		return;
	}
#ifdef PLATFORM_DESKTOP
	++panoramaTimer;
#else
	_updateLicense();
#endif
}

void StartMenuScreen::buttonClicked(Button* button) {
#ifdef PLATFORM_DESKTOP
	if (button == &bSingleplayer) {
		minecraft->screenChooser.setScreen(SCREEN_SELECTWORLD);
		return;
	}
	if (button == &bMultiplayer) {
		minecraft->locateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
		return;
	}
	if (button == &bOptionsJava) {
		minecraft->setScreen(new JavaOptionsScreen(JAVA_OPTIONS_BACK_TO_MENU));
		return;
	}
	if (button == &bQuit) {
		minecraft->quit();
		return;
	}
	Screen::buttonClicked(button);
	return;
#endif

	if (button->id == bHost.id)
	{
        #if defined(DEMO_MODE) || defined(APPLE_DEMO_PROMOTION)
			minecraft->setScreen( new SimpleChooseLevelScreen("_DemoLevel") );
		#else
			minecraft->screenChooser.setScreen(SCREEN_SELECTWORLD);
		#endif
	}
	if (button->id == bJoin.id)
	{
		minecraft->locateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
	}
	if (button->id == bOptions.id)
	{
		minecraft->setScreen(new OptionsScreen());
	}
	if (button->id == bTest.id)
	{
		//minecraft->setScreen(new PauseScreen());
		//minecraft->setScreen(new PrerenderTilesScreen());
	}
	if (button->id == bBuy.id)
	{
		minecraft->platform()->buyGame();
		//minecraft->setScreen(new BuyGameScreen());
	}
}

bool StartMenuScreen::isInGameScreen() { return false; }

#ifdef PLATFORM_DESKTOP
void StartMenuScreen::renderJavaPanorama()
{
	if (!_panoramaTexReady) {
		glGenTextures(1, &_panoramaTex);
		glBindTexture2(GL_TEXTURE_2D, _panoramaTex);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D2(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		_panoramaTexReady = true;
	}

	static const char* paths[6] = {
		"java/panorama0.png", "java/panorama1.png", "java/panorama2.png",
		"java/panorama3.png", "java/panorama4.png", "java/panorama5.png"
	};
	static const float rots[6][3] = {
		{ 0,    0, 0 },
		{ 90,   0, 1 },
		{ 180,  0, 1 },
		{ -90,  0, 1 },
		{ 90,   1, 0 },
		{ -90,  1, 0 }
	};

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glViewport(0, 0, 256, 256);

	glDisable2(GL_CULL_FACE);
	glDisable2(GL_DEPTH_TEST);
	glDepthMask(false);
	glDisable2(GL_ALPHA_TEST);
	glEnable2(GL_BLEND);
	glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f2(1, 1, 1, 1);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity2();
	gluPerspective(120.0f, 1.0f, 0.05f, 10.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity2();
	glRotatef2(180.0f, 1, 0, 0);

	// drawPanorama: 8x8 grid of camera jitter tiles, each compositing all 6 faces
	const float timer = (float)panoramaTimer;
	const int grid = 8;
	Tesselator& t = Tesselator::instance;
	for (int tile = 0; tile < grid * grid; ++tile) {
		glPushMatrix2();
		glTranslatef2(((float)(tile % grid) / (float)grid - 0.5f) / 64.0f,
		              ((float)(tile / grid) / (float)grid - 0.5f) / 64.0f, 0.0f);
		glRotatef2(Mth::sin((timer + (float)tile) / 400.0f) * 25.0f + 20.0f, 1, 0, 0);
		glRotatef2(-(timer + (float)tile) * 0.1f, 0, 1, 0);

		for (int face = 0; face < 6; ++face) {
			glPushMatrix2();
			if (face > 0)
				glRotatef2(rots[face][0], rots[face][1], rots[face][2], rots[face][3]);
			minecraft->textures->loadAndBindTexture(paths[face]);
			const int alpha = 255 / (tile + 1);
			t.begin();
			t.color(1.0f, 1.0f, 1.0f, (float)alpha / 255.0f);
			t.vertexUV(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f);
			t.vertexUV( 1.0f, -1.0f, 1.0f, 1.0f, 0.0f);
			t.vertexUV( 1.0f,  1.0f, 1.0f, 1.0f, 1.0f);
			t.vertexUV(-1.0f,  1.0f, 1.0f, 0.0f, 1.0f);
			t.draw();
			glPopMatrix2();
		}
		glPopMatrix2();
		glColorMask(true, true, true, false);
	}
	glColorMask(true, true, true, true);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// rotateAndBlurSkybox: 8 passes, copying the framebuffer into the viewport texture
	// and smearing it across the 256x256 target with three offsets
	glBindTexture2(GL_TEXTURE_2D, _panoramaTex);
	for (int pass = 0; pass < 8; ++pass) {
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 256, 256);
		glEnable2(GL_BLEND);
		glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColorMask(true, true, true, false);

		for (int k = 0; k < 3; ++k) {
			const float shift = (float)(k - 1) / 256.0f;
			t.begin();
			t.color(1.0f, 1.0f, 1.0f, 1.0f / (float)(k + 1));
			t.vertexUV((float)width, (float)height, blitOffset, shift, 0.0f);
			t.vertexUV((float)width, 0.0f,              blitOffset, 1.0f + shift, 0.0f);
			t.vertexUV(0.0f,         0.0f,              blitOffset, 1.0f + shift, 1.0f);
			t.vertexUV(0.0f,         (float)height,     blitOffset, shift, 1.0f);
			t.draw();
		}
		glColorMask(true, true, true, true);
	}

	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	// Final fit stretch onto the whole screen
	glBindTexture2(GL_TEXTURE_2D, _panoramaTex);
	glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glEnable2(GL_BLEND);
	glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	const float fit = (width > height) ? 120.0f / (float)width : 120.0f / (float)height;
	const float hu = (float)height * fit / 256.0f;
	const float wu = (float)width * fit / 256.0f;
	glColor4f2(1, 1, 1, 1);
	t.begin();
	t.color(1, 1, 1, 1);
	t.vertexUV(0.0f,         (float)height, blitOffset, 0.5f - hu, 0.5f + wu);
	t.vertexUV((float)width, (float)height, blitOffset, 0.5f - hu, 0.5f - wu);
	t.vertexUV((float)width, 0.0f,          blitOffset, 0.5f + hu, 0.5f - wu);
	t.vertexUV(0.0f,         0.0f,          blitOffset, 0.5f + hu, 0.5f + wu);
	t.draw();

	glDepthMask(true);
	glEnable2(GL_DEPTH_TEST);
	glEnable2(GL_CULL_FACE);
	glEnable2(GL_ALPHA_TEST);
	glDisable2(GL_BLEND);
}

void StartMenuScreen::renderJavaTitleScreen(int xm, int ym, float a)
{
	renderJavaPanorama();

	// Java 1.4.7 brightens the already-lit sky top and darkens the bottom
	fillGradient(0, 0, width, height, 0x80ffffff, 0x00ffffff);
	fillGradient(0, 0, width, height, 0x00000000, 0x80000000);

	// Existing Minecraft logo, Java title position
	TextureId id = minecraft->textures->loadTexture("gui/title.png");
	const TextureData* data = minecraft->textures->getTemporaryTextureData(id);
	if (data) {
		minecraft->textures->bind(id);

		const float x = (float)width / 2;
		const float y = 30;
		const float wh = Mth::Min((float)width / 2.0f, (float)data->w / 2);
		const float scale = 2.0f * wh / (float)data->w;
		const float h = scale * (float)data->h;

		Tesselator& t = Tesselator::instance;
		glColor4f2(1, 1, 1, 1);
		t.begin();
		t.vertexUV(x - wh, y + h, blitOffset, 0, 1);
		t.vertexUV(x + wh, y + h, blitOffset, 1, 1);
		t.vertexUV(x + wh, y + 0, blitOffset, 1, 0);
		t.vertexUV(x - wh, y + 0, blitOffset, 0, 0);
		t.draw();
	}

	// Splash text, Java style (bobbing, rotated -20 degrees, yellow)
	const float scale0 = 1.8f - Mth::abs(Mth::sin(getTimeS() * 3.14159265f)) * 0.1f;
	const float splashScale = scale0 * 100.0f / (float)(font->width(splashText) + 32);
	glPushMatrix2();
	glTranslatef2((float)(width / 2 + 90), 70.0f, 0);
	glRotatef2(-20.0f, 0, 0, 1);
	glScalef2(splashScale, splashScale, 1);
	drawCenteredString(font, splashText, 0, -8, 0xffff00);
	glPopMatrix2();

	// Version (bottom-left) and copyright (bottom-right)
	drawString(font, "Minecraft 1.4.7", versionPosX, height - 10, 0xffffff);
	drawString(font, "Copyright Mojang AB. Do not distribute!", copyrightPosX, height - 10, 0xffffff);

	Screen::render(xm, ym, a);
}
#endif

void StartMenuScreen::render( int xm, int ym, float a )
{
#ifdef PLATFORM_DESKTOP
	renderJavaTitleScreen(xm, ym, a);
	return;
#endif

	renderBackground();

#if defined(RPI)
	TextureId id = minecraft->textures->loadTexture("gui/pi_title.png");
#else
	TextureId id = minecraft->textures->loadTexture("gui/title.png");
#endif
	const TextureData* data = minecraft->textures->getTemporaryTextureData(id);

	if (data) {
		minecraft->textures->bind(id);

		const float x = (float)width / 2;
		const float y = 4;
		//const float scale = Mth::Min(
		const float wh = Mth::Min((float)width/2.0f, (float)data->w / 2);
		const float scale = 2.0f * wh / (float)data->w;
		const float h = scale * (float)data->h;

		// Render title text
		Tesselator& t = Tesselator::instance;
		glColor4f2(1, 1, 1, 1);
		t.begin();
		t.vertexUV(x-wh, y+h, blitOffset, 0, 1);
		t.vertexUV(x+wh, y+h, blitOffset, 1, 1);
		t.vertexUV(x+wh, y+0, blitOffset, 1, 0);
		t.vertexUV(x-wh, y+0, blitOffset, 0, 0);
		t.draw();
	}

#if defined(RPI)
	if (Textures::isTextureIdValid(minecraft->textures->loadAndBindTexture("gui/logo/raknet_high_72.png")))
		blit(0, height - 12, 0, 0, 43, 12, 256, 72+72);
#endif

	drawString(font, version, versionPosX, 62, /*50,*/ 0xffcccccc);//0x666666);
	drawString(font, copyright, copyrightPosX, height - 10, 0xffffff);
	glEnable2(GL_BLEND);
	glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f2(1, 1, 1, 1);
	if (Textures::isTextureIdValid(minecraft->textures->loadAndBindTexture("gui/logo/github.png")))
		blit(2, height - 10, 0, 0, 8, 8, 256, 256);
{
			std::string txt = "Kolyah35/minecraft-pe-0.6.1";
			float wtxt = font->width(txt);
			Gui::drawColoredString(font, txt, 12, height - 10, 255);
			// underline link
			float y0 = height - 10 + font->lineHeight - 1;
			this->fill(12, (int)y0, 12 + (int)wtxt, (int)(y0 + 1), 0xffffffff);
    }
}

void StartMenuScreen::_updateLicense()
{
#ifdef PLATFORM_DESKTOP
	return;
#endif
	int id = minecraft->getLicenseId();
	if (LicenseCodes::isReady(id))
	{
		if (LicenseCodes::isOk(id))
			bJoin.active = bHost.active = bOptions.active = true;
		else
		{
			bool hasBuyButton = minecraft->platform()->hasBuyButtonWhenInvalidLicense();
			minecraft->setScreen(new InvalidLicenseScreen(id, hasBuyButton));
		}
	} else {
		bJoin.active = bHost.active = bOptions.active = false;
	}
}

void StartMenuScreen::mouseClicked(int x, int y, int buttonNum) {
	const int logoX = 2;
	const int logoW = 8 + 2 + font->width("Kolyah35/minecraft-pe-0.6.1");
	const int logoY = height - 10;
	const int logoH = 10;
	if (x >= logoX && x <= logoX + logoW && y >= logoY && y <= logoY + logoH)
		minecraft->platform()->openURL("https://gitea.sffempire.ru/Kolyah35/minecraft-pe-0.6.1");
	else
		Screen::mouseClicked(x, y, buttonNum);
}

bool StartMenuScreen::handleBackEvent( bool isDown ) {
	minecraft->quit();
	return true;
}
