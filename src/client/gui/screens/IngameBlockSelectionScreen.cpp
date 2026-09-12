#include "IngameBlockSelectionScreen.h"
#include "../../renderer/TileRenderer.h"
#include "../../player/LocalPlayer.h"
#include "../../renderer/gles.h"
#include "../../Minecraft.h"
#include "../../sound/SoundEngine.h"
#include "../../../world/entity/player/Inventory.h"
#include "../../../platform/input/Mouse.h"

#include "../Gui.h"
#include "../../renderer/Textures.h"
#include "../../gamemode/GameMode.h"
#include "ArmorScreen.h"
#include "../components/Button.h"

#if defined(__APPLE__)
    static const std::string demoVersionString("Not available in the Lite version");
#else
    static const std::string demoVersionString("Not available in the demo version");
#endif

IngameBlockSelectionScreen::IngameBlockSelectionScreen() 
:	selectedItem(0),
	_area(0,0,0,0),
	_pendingQuit(false),
	InventoryRows(1),
	InventoryCols(1),
	InventorySize(1),
	bArmor(1, "Armor")
{
}

void IngameBlockSelectionScreen::init()
{
	Inventory* inventory = minecraft->player->inventory;
#ifdef PLATFORM_DESKTOP
	// Java inventory layout: nine columns regardless of game mode
	InventoryCols = 9;
#else
	InventoryCols = minecraft->isCreativeMode()? 13 : 9;
#endif
	InventorySize = inventory->getContainerSize() - Inventory::MAX_SELECTION_SIZE;
	InventoryRows = 1 + (InventorySize - 1) / InventoryCols;

	_area = RectangleArea(	(float)getSlotPosX(0) - 4,
							(float)getSlotPosY(0) - 4,
							(float)getSlotPosX(InventoryCols) + 4, 
							(float)getSlotPosY(InventoryRows) + 4);

	ItemInstance* selected = inventory->getSelected();
	if (!selected || selected->isNull()) {
		selectedItem = 0;
		return;
	}

	for (int i = Inventory::MAX_SELECTION_SIZE; i < InventorySize; i++) {
		if (selected == minecraft->player->inventory->getItem(i))
		{
			selectedItem = i - Inventory::MAX_SELECTION_SIZE;
			break;
		}
	}
	if (!isAllowed(selectedItem))
		selectedItem = 0;

	if (!minecraft->isCreativeMode()) {
		bArmor.width = 42;
		bArmor.x = 0;
		bArmor.y = height - bArmor.height;
		buttons.push_back(&bArmor);
	}
}

void IngameBlockSelectionScreen::removed()
{
	minecraft->gui.inventoryUpdated();
}

void IngameBlockSelectionScreen::renderSlots()
{
	glColor4f2(1, 1, 1, 1);

	blitOffset = -90;

	minecraft->textures->loadAndBindTexture("gui/gui.png");
	for (int r = 0; r < InventoryRows; r++)
	{
		int x = getSlotPosX(0) - 3;
		int y = getSlotPosY(r) - 3;

#ifdef PLATFORM_DESKTOP
		// Java 1.4.7 inventory look: a dark translucent panel behind the
		// whole grid, then a beveled slot box per item.
		fill(getSlotPosX(0) - 5, getSlotPosY(0) - 5,
		     getSlotPosX(InventoryCols) + 5, getSlotPosY(InventoryRows) - 3,
		     0x50 << 24);
		fill(getSlotPosX(0) - 5, getSlotPosY(0) - 5,
		     getSlotPosX(InventoryCols) + 5, getSlotPosY(0) - 4, 0xa0 << 24);
		fill(getSlotPosX(0) - 5, getSlotPosY(InventoryRows) - 4,
		     getSlotPosX(InventoryCols) + 5, getSlotPosY(InventoryRows) - 3, 0xa0 << 24);
		break;
#else
		if (InventoryCols == 9) {
			blit(x, y, 0, 0, 182, 22);
		} else {
			// first 8 slots
			blit(x, y, 0, 0, 182-20, 22);
			// last k slots
			const int k = 5;
			const int w = k * 20;
			blit(x + 162, y, 182-w, 0, w, 22);
		}
#endif
	}
#ifndef PLATFORM_DESKTOP
	if (selectedItem >= 0)
	{
		int x = getSlotPosX(selectedItem % InventoryCols) - 4;
		int y = getSlotPosY(selectedItem / InventoryCols) - 4;
		blit(x, y, 0, 22, 24, 22);
	}
#endif
	for (int r = 0; r < InventoryRows; r++)
	{
		for (int i = 0; i < InventoryCols; i++) {
			int x = getSlotPosX(i);
			int y = getSlotPosY(r);

#ifdef PLATFORM_DESKTOP
			const int sIdx = r * InventoryCols + i;
			// beveled slot box (Java container slots)
			fill(x - 3, y - 3, x + 18, y + 18, 0x40382f25);
			fill(x - 3, y - 3, x + 18, y - 2, 0x80a8a8a8);
			fill(x - 3, y - 3, x - 2, y + 18, 0x80a8a8a8);
			fill(x + 17, y - 3, x + 18, y + 18, 0x80555555);
			fill(x - 3, y + 17, x + 18, y + 18, 0x80555555);
			if (sIdx == selectedItem) {
				fill(x - 4, y - 4, x + 19, y - 3, 0xffffffff);
				fill(x - 4, y - 4, x - 3, y + 19, 0xffffffff);
				fill(x + 18, y - 4, x + 19, y + 19, 0xffffffff);
				fill(x - 4, y + 18, x + 19, y + 19, 0xffffffff);
			}
#endif
			renderSlot(r * InventoryCols + i + Inventory::MAX_SELECTION_SIZE, x, y, 0);
		}
	}

#ifdef PLATFORM_DESKTOP
	drawCenteredString(minecraft->font, "Inventory",
	                   width / 2, getSlotPosY(0) - 12, 0xffffffff);
#endif
}

int IngameBlockSelectionScreen::getSlotPosX(int slotX) {
	return width / 2 - InventoryCols * 10 + slotX * 20 + 2;
}

int IngameBlockSelectionScreen::getSlotPosY(int slotY) {
	//return height - 63 - 22 * (3 - slotY);
	int yy = InventoryCols==9? 8 : 3;
	return yy + slotY * getSlotHeight();
}

//int IngameBlockSelectionScreen::getLinearSlotId(int x, int y) {
//	return 
//}


#include "../../../world/item/ItemInstance.h"
#include "../../renderer/entity/ItemRenderer.h"

void IngameBlockSelectionScreen::renderSlot(int slot, int x, int y, float a)
{
	ItemInstance* item = minecraft->player->inventory->getItem(slot);
	if (!item) return;

	ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, item, (float)x, (float)y, true);

	if (minecraft->gameMode->isCreativeType()) return;
	if (!isAllowed(slot - Inventory::MAX_SELECTION_SIZE)) return;

	glPushMatrix2();
	glScalef2(Gui::InvGuiScale + Gui::InvGuiScale, Gui::InvGuiScale + Gui::InvGuiScale, 1);
	const float k = 0.5f * Gui::GuiScale;
	minecraft->gui.renderSlotText(item, k*x, k*y, true, true);
	glPopMatrix2();
}

void IngameBlockSelectionScreen::keyPressed(int eventKey)
{
	int selX = selectedItem % InventoryCols;
	int selY = selectedItem / InventoryCols;

	int tmpSelectedSlot = selectedItem;

	Options& o = minecraft->options;
	if (eventKey == o.keyLeft.key && selX > 0)
	{
		tmpSelectedSlot -= 1;
	}
	else if (eventKey == o.keyRight.key && selX < (InventoryCols - 1))
	{
		tmpSelectedSlot += 1;
	}
	else if (eventKey == o.keyDown.key && selY < (InventoryRows - 1))
	{
		tmpSelectedSlot += InventoryCols;
	}
	else if (eventKey == o.keyUp.key && selY > 0)
	{
		tmpSelectedSlot -= InventoryCols;
	}

	if (isAllowed(tmpSelectedSlot))
		selectedItem = tmpSelectedSlot;

	if (eventKey == o.keyMenuOk.key)
		selectSlotAndClose();

#ifdef RPI
	if (eventKey == o.keyMenuCancel.key
		||	eventKey == Keyboard::KEY_ESCAPE)
		minecraft->setScreen(NULL);
#else
	if (eventKey == o.keyMenuCancel.key)
		minecraft->setScreen(NULL);
#endif
}

int IngameBlockSelectionScreen::getSelectedSlot(int x, int y)
{
	int left = width / 2 - InventoryCols * 10;
	int top = -4 + getSlotPosY(0);

	if (x >= left && y >= top)
	{
		int xSlot = (x - left) / 20;
		if (xSlot < InventoryCols) {
			int row = ((y-top) / getSlotHeight());
			return row * InventoryCols + xSlot;
		}
	}
	return -1;
}

void IngameBlockSelectionScreen::mouseClicked(int x, int y, int buttonNum)
{
	if (buttonNum == MouseAction::ACTION_LEFT) {

		int slot = getSelectedSlot(x, y);
		if (isAllowed(slot))
		{
			selectedItem = slot;
			//minecraft->soundEngine->playUI("random.click", 1, 1);
		} else {
			_pendingQuit = !_area.isInside((float)x, (float)y)
			            && !bArmor.isInside(x, y);
		}
	}
	if (!_pendingQuit)
		super::mouseClicked(x, y, buttonNum);
}

void IngameBlockSelectionScreen::mouseReleased(int x, int y, int buttonNum)
{
	if (buttonNum == MouseAction::ACTION_LEFT) {

		int slot = getSelectedSlot(x, y);
		if (isAllowed(slot) && slot == selectedItem)
		{
			selectSlotAndClose();
		} else {
			if (_pendingQuit && !_area.isInside((float)x, (float)y))
				minecraft->setScreen(NULL);
		}
	}
	if (!_pendingQuit)
		super::mouseReleased(x, y, buttonNum);
}

void IngameBlockSelectionScreen::selectSlotAndClose()
{
	Inventory* inventory = minecraft->player->inventory;
	// Flash the selected gui item
	//inventory->moveToSelectedSlot(selectedItem + Inventory::MAX_SELECTION_SIZE, true);
	inventory->moveToSelectionSlot(0, selectedItem + Inventory::MAX_SELECTION_SIZE, true);
	inventory->selectSlot(0);
	minecraft->gui.flashSlot(inventory->selected);

	minecraft->soundEngine->playUI("random.click", 1, 1);
	minecraft->setScreen(NULL);
}

void IngameBlockSelectionScreen::render( int xm, int ym, float a )
{
	glDisable2(GL_DEPTH_TEST);
	fill(0, 0, width, height, (0x80) << 24);
	glEnable2(GL_BLEND);

	glDisable2(GL_ALPHA_TEST);
	glEnable2(GL_BLEND);
	glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	renderSlots();
	renderDemoOverlay();

	glEnable2(GL_ALPHA_TEST);
	glDisable2(GL_BLEND);

	glEnable2(GL_DEPTH_TEST);

	Screen::render(xm, ym, a);
}

void IngameBlockSelectionScreen::renderDemoOverlay() {
#ifdef DEMO_MODE
	fill(	getSlotPosX(0) - 3,							getSlotPosY(3) - 3,
			getSlotPosX(InventoryCols) - 3, getSlotPosY(InventoryRows) - 3, 0xa0 << 24);

	const int centerX = (getSlotPosX(4) + getSlotPosX(5)) / 2;
	const int centerY = (getSlotPosY(3) + getSlotPosY(InventoryRows-1)) / 2 + 5;
	drawCenteredString(minecraft->font, demoVersionString, centerX, centerY, 0xffffffff);
#endif /*DEMO_MODE*/
}

bool IngameBlockSelectionScreen::isAllowed(int slot) {
	if (slot < 0 || slot >= InventorySize)
		return false;

	#ifdef DEMO_MODE
		return slot < (minecraft->isCreativeMode()? 28 : 27);
	#endif /*DEMO_MODE*/

	return true;
}

int IngameBlockSelectionScreen::getSlotHeight() {
	return InventoryCols==9? 22 : 20;
}

void IngameBlockSelectionScreen::buttonClicked( Button* button )
{
	if (button == &bArmor) {
		minecraft->setScreen(new ArmorScreen());
	}
	super::buttonClicked(button);
}
