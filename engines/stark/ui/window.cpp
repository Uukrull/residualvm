/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/stark/ui/window.h"

#include "engines/stark/cursor.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/resources/pattable.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"

namespace Stark {
 
Window::Window(Gfx::Driver *gfx, Cursor *cursor) :
		_gfx(gfx),
		_cursor(cursor),
		_unscaled(false),
		_visible(false),
		_objectUnderCursor(nullptr) {
}

Window::~Window() {
}

void Window::render() {
	if (!_visible) {
		return;
	}

	_gfx->setViewport(_position, _unscaled);

	onRender();
}

bool Window::isMouseInside() const {
	if (!_visible) {
		return false;
	}

	Common::Point mousePos = _cursor->getMousePosition(_unscaled);
	return _position.contains(mousePos);
}

bool Window::isVisible() const {
	return _visible;
}

Common::Point Window::getMousePosition() const {
	Common::Point mousePos = _cursor->getMousePosition(_unscaled);
	return mousePos - Common::Point(_position.left, _position.top);
}

Common::Point Window::getScreenMousePosition() const {
	return _cursor->getMousePosition(_unscaled);
}

void Window::setCursor(Cursor::CursorType type) {
	_cursor->setCursorType(type);
}

void Window::setCursorHint(const Common::String &hint) {
	_cursor->setMouseHint(hint);
}

void Window::handleMouseMove() {
	if (!_visible) {
		return;
	}

	if (isMouseInside()) {
		onMouseMove(getMousePosition());
	}
}

void Window::handleClick() {
	if (!_visible) {
		return;
	}

	if (isMouseInside()) {
		onClick(getMousePosition());
	}
}

void Window::handleRightClick() {
	if (!_visible) {
		return;
	}

	if (isMouseInside()) {
		onRightClick(getMousePosition());
	}
}

void Window::updateItems() {
	// Check for game world mouse overs
	UserInterface *ui = StarkServices::instance().userInterface;
	Common::Point pos = getMousePosition();


	_objectUnderCursor = nullptr;

	// Render entries are sorted from the farthest to the camera to the nearest
	// Loop in reverse order
	for (int i = _renderEntries.size() - 1; i >= 0; i--) {
		if (_renderEntries[i]->containsPoint(pos, _objectRelativePosition)) {
			_objectUnderCursor = _renderEntries[i]->getOwner();
			break;
		}
	}

	Resources::ActionArray actionsPossible;
	if (_objectUnderCursor) {
		actionsPossible = ui->getActionsPossibleForObject(_objectUnderCursor, _objectRelativePosition);
	}

	if (actionsPossible.empty()) {
		// Only consider items with runnable scripts
		_objectUnderCursor = nullptr;
	}

	Common::String mouseHint;
	if (_objectUnderCursor) {
		setCursorDependingOnActionsAvailable(actionsPossible);

		mouseHint = ui->getItemTitle(_objectUnderCursor, true, _objectRelativePosition);
	} else {
		// Not an object
		_cursor->setCursorType(Cursor::kPassive);
	}
	_cursor->setMouseHint(mouseHint);
}

void Window::setCursorDependingOnActionsAvailable(Resources::ActionArray actionsAvailable) {
	if (actionsAvailable.empty()) {
		_cursor->setCursorType(Cursor::kPassive);
		return;
	}

	uint32 count = 0;
	Cursor::CursorType cursorType;
	for (uint i = 0; i < actionsAvailable.size(); i++) {
		switch (actionsAvailable[i]) {
			case Resources::PATTable::kActionLook:
				cursorType = Cursor::kEye;
				count++;
				break;
			case Resources::PATTable::kActionTalk:
				cursorType = Cursor::kMouth;
				count++;
				break;
			case Resources::PATTable::kActionUse:
				cursorType = Cursor::kHand;
				count++;
				break;
		}
	}

	if (count == 1) {
		_cursor->setCursorType(cursorType);
	} else {
		_cursor->setCursorType(Cursor::kActive);
	}
}

} // End of namespace Stark
