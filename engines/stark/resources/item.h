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

#ifndef STARK_RESOURCES_ITEM_H
#define STARK_RESOURCES_ITEM_H

#include "engines/stark/resources/object.h"
#include "engines/stark/resourcereference.h"

#include "common/rect.h"
#include "common/str.h"

#include "math/vector3d.h"

namespace Stark {

class Visual;

namespace Gfx {
class RenderEntry;
}

namespace Formats {
class XRCReadStream;
}

namespace Resources {

class Anim;
class AnimHierarchy;
class BonesMesh;
class Bookmark;
class TextureSet;

/**
 * A scene element
 *
 * Can be a character, background, animation, ...
 */
class Item : public Object {
public:
	static const Type::ResourceType TYPE = Type::kItem;

	enum SubType {
		kItemGlobalTemplate = 1,
		kItemInventory = 2,
		kItemLevelTemplate = 3,
		kItemStaticProp = 5,
		kItemAnimatedProp = 6,
		kItemBackgroundElement = 7,
		kItemBackground = 8,
		kItemMesh = 10
	};

	/** Item factory */
	static Object *construct(Object *parent, byte subType, uint16 index, const Common::String &name);

	Item(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Item();

	// Resource API
	virtual void readData(Formats::XRCReadStream *stream) override;
	void saveLoad(ResourceSerializer *serializer) override;

	/** Is the item present in the scene */
	bool isEnabled() const;

	/** Enable or disable the item */
	virtual void setEnabled(bool enabled);

	/** Get the item's character index */
	int32 getCharacterIndex() const;

	/** Obtain the render entry to use to display the item */
	virtual Gfx::RenderEntry *getRenderEntry(const Common::Point &positionOffset);

	/** Obtain the concrete instance of an item template */
	virtual Item *getSceneInstance();
protected:
	void printData() override;

	bool _enabled;
	int32 _characterIndex;
};

/**
 * A renderable item
 *
 * Renderable items are found in location layers
 */
class ItemVisual : public Item {
public:
	ItemVisual(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ItemVisual();

	// Resource API
	virtual void readData(Formats::XRCReadStream *stream) override;
	virtual void onAllLoaded() override;

	// Item API
	void setEnabled(bool enabled) override;

	/** Get the hotspot index for an item relative position */
	int getHotspotIndexForPoint(const Common::Point &point);

	/** Obtain the title for one of the item's hotspots */
	Common::String getHotspotTitle(uint32 hotspotIndex);

	/** Check whether the item has runnable scripts for the specified action */
	bool canPerformAction(uint32 action, uint32 hotspotIndex);

	/** Perform an action on one of the item's hotspots */
	bool doAction(uint32 action, uint32 hotspotIndex);

	/** Define the current animation kind for the item */
	void setAnimKind(int32 usage);

protected:
	// Resource API
	void printData() override;

	Anim *getAnim();
	Visual *getVisual();

	Gfx::RenderEntry *_renderEntry;

	AnimHierarchy *_animHierarchy;
	int32 _currentAnimKind;
	bool _clickable;
};

/**
 * An item template
 *
 * Item templates need to be instanciated into renderable items to be displayed
 */
class ItemTemplate : public Item {
public:
	ItemTemplate(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ItemTemplate();

	// Resource API
	void onAllLoaded() override;

	// Item API
	Item *getSceneInstance() override;

	/** Obtain the bone mesh to use to render the item */
	virtual BonesMesh *findBonesMesh() = 0;

	/** Obtain the texture to use to render the item */
	virtual TextureSet *findTextureSet(uint32 textureType) = 0;

	/** Obtain the animation hierarchy to fetch animations from */
	virtual AnimHierarchy *findStockAnimHierarchy() = 0;

	/** Change the item's mesh */
	void setBonesMesh(int32 index);

	/** Set the mesh main or face texture */
	void setTexture(int32 index, uint32 textureType);

	/** Set the scene instanciation for this template */
	void setInstanciatedItem(Item *instance);

protected:
	int32 _meshIndex;
	int32 _textureNormalIndex;
	int32 _textureFaceIndex;
	int32 _animHierarchyIndex;

	Item *_instanciatedItem;
	ItemTemplate *_referencedItem;
};

/**
 * A global item template
 *
 * Global item templates are found in the global level
 */
class GlobalItemTemplate : public ItemTemplate {
public:
	GlobalItemTemplate(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~GlobalItemTemplate();

	// ItemTemplate API
	BonesMesh *findBonesMesh() override;
	TextureSet *findTextureSet(uint32 textureType) override;
	AnimHierarchy *findStockAnimHierarchy() override;

protected:
};

/**
 * An inventory item
 */
class InventoryItem : public ItemVisual {
public:
	InventoryItem(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~InventoryItem();

	// Item API
	Gfx::RenderEntry *getRenderEntry(const Common::Point &positionOffset) override;
	void setEnabled(bool enabled) override;

	/** Obtain an action menu icon */
	Visual *getActionVisual(bool active) const;

	/** Obtain an inventory item cursor */
	Visual *getCursorVisual() const;

protected:
};

/**
 * A level item template
 *
 * Level item templates are found in levels so that they can be shared between
 * locations.
 */
class LevelItemTemplate : public ItemTemplate {
public:
	LevelItemTemplate(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~LevelItemTemplate();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onAllLoaded() override;

	// ItemTemplate API
	BonesMesh *findBonesMesh() override;
	TextureSet *findTextureSet(uint32 textureType) override;
	AnimHierarchy *findStockAnimHierarchy() override;

	/** Get the item's level or global template if any */
	ItemTemplate *getItemTemplate() const;

protected:
	void printData() override;

	ResourceReference _reference;
};

/**
 * 3D positioned item
 *
 * Items with a 3D position, used in 3D layers. The sort key determines the order
 * in which such items are drawn in.
 */
class FloorPositionedItem : public ItemVisual {
public:
	FloorPositionedItem(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~FloorPositionedItem();

	/** Move the item to a bookmarked position */
	void placeOnBookmark(Bookmark *target);

	/** Move the item */
	void setPosition3D(const Math::Vector3d &position);

	/** Get the floor face index the item is standing on */
	int32 getFloorFaceIndex() const;
	/** Change the face the item is standing on */
	void setFloorFaceIndex(int32 faceIndex);

	/** Set the direction the item faces */
	void setDirection(uint direction);

	/** Obtain the sort value for the item, used to compute the draw order */
	float getSortKey() const;

protected:
	int32 _floorFaceIndex;
	Math::Vector3d _position3D;
	float _direction3D;
};

/**
 * 3D positioned image item
 *
 * Used to display still images or animated images in 3D layers
 */
class FloorPositionedImageItem : public FloorPositionedItem {
public:
	FloorPositionedImageItem(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~FloorPositionedImageItem();

	// Resource API
	virtual void readData(Formats::XRCReadStream *stream) override;

	// Item API
	Gfx::RenderEntry *getRenderEntry(const Common::Point &positionOffset) override;

protected:
	void printData() override;

	Common::Point _position;
};

/**
 * Mesh item
 *
 * Used to draw characters
 */
class MeshItem : public FloorPositionedItem {
public:
	MeshItem(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~MeshItem();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onAllLoaded() override;
	void onEnterLocation() override;

	// Item API
	Gfx::RenderEntry *getRenderEntry(const Common::Point &positionOffset) override;

	/** Set the mesh main or face texture */
	void setTexture(int32 index, uint32 textureType);

	/** Change the item's mesh */
	void setBonesMesh(int32 index);

	/** Obtain the bone mesh to use to render the item */
	BonesMesh *findBonesMesh();

	/** Obtain the texture to use to render the item */
	TextureSet *findTextureSet(uint32 textureType);

	/** Get the item's level or global template if any */
	ItemTemplate *getItemTemplate() const;

	/** Update the item's animation after a texture / mesh change */
	void updateAnim();

protected:
	void printData() override;

	int32 _meshIndex;
	int32 _textureNormalIndex;
	int32 _textureFaceIndex;

	ResourceReference _reference;
	ItemTemplate *_referencedItem;
};

/**
 * 2D positioned image item
 *
 * Used to display background elements in 2D layers
 */
class ImageItem : public ItemVisual {
public:
	ImageItem(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ImageItem();

	// Resource API
	virtual void readData(Formats::XRCReadStream *stream) override;

	// Item API
	Gfx::RenderEntry *getRenderEntry(const Common::Point &positionOffset) override;

protected:
	void printData() override;

	ResourceReference _reference;
	Common::Point _position;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_ITEM_H
