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

#ifndef STARK_RESOURCES_FMV_H
#define STARK_RESOURCES_FMV_H

#include "common/str.h"

#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

/**
 * A full motion video
 */
class FMV : public Object {
public:
	static const Type::ResourceType TYPE = Type::kFMV;

	FMV(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~FMV();

	void readData(Formats::XRCReadStream *stream) override;

	Common::String getFilename() const { return _filename; }
protected:
	void printData() override;

	Common::String _filename;
	uint32 _diaryAddEntryOnPlay;
	uint32 _gameDisc;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_FMV_H
