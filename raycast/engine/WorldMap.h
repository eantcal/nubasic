// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.


/* -------------------------------------------------------------------------- */

#pragma once

#include "MapCell.h"
#include "Player.h"
#include "Texture.h"
#include "WorldBlock.h"

#pragma warning(disable : 4786)
#include <map>
#include <math.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/* -------------------------------------------------------------------------- */

class WorldMap {
public:
    using Cell = ::Cell;
    using Point2d = std::pair<double, double>;
    using TextureList = std::map<std::string, std::string>;

    // World-configured text for the bottom teletype event log. Templates may
    // contain {name} (item name) and {amount} (e.g. healed hit points)
    // placeholders.
    struct MessageLogConfig {
        bool enabled = true;
        int maxLines = 4;
        std::string keyPickup = "Picked up {name}";
        std::string ammoPickup = "Ammunition replenished";
        std::string healthPickup = "Health restored (+{amount})";
        std::string mapUnlocked = "Area map online";
        std::string mapActorsUnlocked = "Hostile tracking online";
        std::string weaponReload = "Reloading {name}";
        std::string itemPickup = "Picked up {name}";
    };

    WorldMap() = default;

    const MessageLogConfig& messageLog() const noexcept { return m_messageLog; }

    MessageLogConfig& messageLog() noexcept { return m_messageLog; }

    const Texture* getTexture(MapCell::TextureResourceKey key) const noexcept
    {
        if (key >= m_textures.size()) {
            return nullptr;
        }

        return m_textures[static_cast<size_t>(key)].get();
    }

    const Point2d& getPlayerCellPos() const noexcept { return m_playerCellPos; }

    bool hasPlayerStart() const noexcept { return m_hasPlayerStart; }

    double getPlayerFacingDegrees() const noexcept
    {
        return m_playerFacingDegrees;
    }

    double getPlayerTurnBaseDegPerSec() const noexcept
    {
        return m_playerTurnBaseDegPerSec;
    }

    double getPlayerTurnMaxDegPerSec() const noexcept
    {
        return m_playerTurnMaxDegPerSec;
    }

    double getPlayerTurnAccelDegPerSecSq() const noexcept
    {
        return m_playerTurnAccelDegPerSecSq;
    }

    // Overrides the progressive-turn feel. Non-positive arguments leave the
    // current value untouched, so a world can specify only the fields it cares
    // about. The cap is kept at least as large as the base rate.
    void setPlayerTurnConfig(double baseDegPerSec, double maxDegPerSec,
        double accelDegPerSecSq) noexcept
    {
        if (baseDegPerSec > 0.0) {
            m_playerTurnBaseDegPerSec = baseDegPerSec;
        }

        if (maxDegPerSec > 0.0) {
            m_playerTurnMaxDegPerSec = maxDegPerSec;
        }

        if (accelDegPerSecSq > 0.0) {
            m_playerTurnAccelDegPerSecSq = accelDegPerSecSq;
        }

        if (m_playerTurnMaxDegPerSec < m_playerTurnBaseDegPerSec) {
            m_playerTurnMaxDegPerSec = m_playerTurnBaseDegPerSec;
        }
    }

    int getRowCount() const noexcept { return int(m_map.size()); }

    int getColCount() const noexcept
    {
        return int(m_map.empty() ? 0 : m_map[0].size());
    }

    uint32_t getCellDx() const noexcept { return m_cellDx; }

    uint32_t getCellDy() const noexcept { return m_cellDy; }

    std::vector<Cell>& operator[](uint32_t index) throw()
    {
        return m_map[index];
    }

    const std::vector<Cell>& operator[](uint32_t index) const throw()
    {
        return m_map[index];
    }


    void resizeCell(uint32_t cellDx, uint32_t cellDy) noexcept
    {
        m_cellDx = cellDx;
        m_cellDy = cellDy;
        m_maxX = getCellDx() * getColCount();
        m_maxY = getCellDy() * getRowCount();
    }

    void setPlayerPos(int x, int y) noexcept
    {
        m_playerCellPos.first = /*player.getX()*/ x / getCellDx();
        m_playerCellPos.second = /*player.getY()*/ y / getCellDy();
        m_hasPlayerStart = true;
    }

    void setPlayerStartCell(
        double xCell, double yCell, double facingDegrees) noexcept
    {
        m_playerCellPos.first = xCell;
        m_playerCellPos.second = yCell;
        m_playerFacingDegrees = facingDegrees;
        m_hasPlayerStart = true;
    }

    void applyTextureToPanel(
        int panelKey, std::shared_ptr<Texture> texture) noexcept
    {
        applyTexture(MapCell::solidWallTexture(panelKey), std::move(texture));
    }

    void applyTexture(MapCell::TextureResourceKey textureKey,
        std::shared_ptr<Texture> texture) noexcept
    {
        if (textureKey >= m_textures.size()) {
            m_textures.resize(static_cast<size_t>(textureKey) + 1);
        }

        m_textures[static_cast<size_t>(textureKey)] = std::move(texture);
    }

    int getMaxX() const noexcept { return m_maxX; }

    int getMaxY() const noexcept { return m_maxY; }

    void set(int row, int col, Cell cellVal)
    {
        if (col < getColCount() && row < getRowCount())
            m_map[row][col] = cellVal;
    }

    bool setCells(const Cell* cells, uint32_t rows, uint32_t cols)
    {
        return setMapInfo(cells, rows, cols);
    }

    Cell cellAtWorld(double x, double y) const noexcept
    {
        if (x < 0.0 || y < 0.0 || x >= getMaxX() || y >= getMaxY()) {
            return 0;
        }

        const auto col = static_cast<uint32_t>(x / getCellDx());
        const auto row = static_cast<uint32_t>(y / getCellDy());
        return m_map[row][col];
    }

    bool isSolidAtWorld(double x, double y) const noexcept
    {
        if (x < 0.0 || y < 0.0 || x >= getMaxX() || y >= getMaxY()) {
            return true;
        }

        const auto col = static_cast<int>(x / getCellDx());
        const auto row = static_cast<int>(y / getCellDy());
        const auto* block = blockAtCell(row, col);
        if (block != nullptr) {
            if (block->door.enabled) {
                return block->door.blocksWhenClosed && !isDoorOpenAt(row, col);
            }

            return block->hasAnyCollidingSpan;
        }

        const auto cell = cellAtWorld(x, y);
        if (MapCell::hasAnyWall(cell)) {
            return MapCell::hasSolidWall(cell);
        }

        return false;
    }

    bool isSolidCellHit(int row, int column, Cell cell) const noexcept;

    bool isDoorOpenAt(int row, int column) const noexcept;

    double doorOpenAmountAt(int row, int column) const noexcept;

    void setDoorKeyring(const std::vector<std::string>& keys)
    {
        m_doorKeyring = keys;
    }

    struct DoorEvent {
        enum class Type { OpeningStarted };

        Type type = Type::OpeningStarted;
        int row = -1;
        int column = -1;
        WinRayCast::BlockId blockId = 0;
    };

    struct TextureLayer {
        MapCell::TextureKey base = 0;
        MapCell::TextureKey overlay = 0;
    };

    TextureLayer textureLayerForWallSpanAt(int row, int column,
        const WinRayCast::WallSpan& span, WinRayCast::WallFace face,
        bool internalWall = false) const noexcept;

    MapCell::TextureKey textureForWallSpanAt(int row, int column,
        const WinRayCast::WallSpan& span, WinRayCast::WallFace face,
        bool internalWall = false) const noexcept;

    TextureLayer transparentWallTextureLayerAt(int row, int column,
        MapCell::TextureKey fallback, WinRayCast::WallFace face,
        bool internalWall = false) const noexcept;

    MapCell::TextureKey transparentWallTextureAt(int row, int column,
        MapCell::TextureKey fallback, WinRayCast::WallFace face,
        bool internalWall = false) const noexcept;

    void updateDoors(double playerX, double playerY,
        const std::vector<Point2d>& actorPositions, double deltaSeconds,
        std::vector<DoorEvent>* events = nullptr) noexcept;

    void advanceDynamicTextures(double deltaSeconds) noexcept;

    void forceDoorClosingAt(int row, int column, double deltaSeconds) noexcept;
    void forceDoorOpenAt(int row, int column) noexcept;

    void setPlayerInteriorOverride(
        MapCell::TextureKey key, int row, int column) noexcept
    {
        m_playerInteriorOverride = key;
        m_playerInteriorRow = row;
        m_playerInteriorCol = column;
    }

    MapCell::TextureKey playerInteriorOverride() const noexcept
    {
        return m_playerInteriorOverride;
    }

    const TextureList& getTextureList() const noexcept { return m_textureList; }

    TextureList& getTextureList() noexcept { return m_textureList; }

    void setBlockDefinition(
        WinRayCast::BlockId id, WinRayCast::BlockDefinition block)
    {
        block.hasAnySolidSpan = false;
        block.hasAnyTransparentSpan = false;
        block.hasAnyCollidingSpan = false;
        const auto movementHeight = static_cast<int32_t>(getCellDy());
        for (const auto& span : block.walls) {
            if (span.kind == WinRayCast::WallSpanKind::Solid) {
                block.hasAnySolidSpan = true;
            } else {
                block.hasAnyTransparentSpan = true;
            }

            if (span.collision && span.bottom < movementHeight) {
                block.hasAnyCollidingSpan = true;
            }
        }

        m_blocks[id] = std::move(block);
        m_blockPresent[id] = true;
    }

    bool hasBlockDefinition(WinRayCast::BlockId id) const noexcept
    {
        return m_blockPresent[id];
    }

    const WinRayCast::BlockDefinition* blockDefinition(
        WinRayCast::BlockId id) const noexcept
    {
        return m_blockPresent[id] ? &m_blocks[id] : nullptr;
    }

    void setBlockId(int row, int column, WinRayCast::BlockId id) noexcept
    {
        if (row < 0 || column < 0 || row >= getRowCount()
            || column >= getColCount()) {
            return;
        }

        if (m_blockIds.size() != m_map.size()) {
            m_blockIds.assign(m_map.size(),
                std::vector<WinRayCast::BlockId>(m_map[0].size(), 0));
        }

        m_blockIds[row][column] = id;
        m_hasBlockIds = true;
    }

    WinRayCast::BlockId blockIdAt(int row, int column) const noexcept
    {
        if (!m_hasBlockIds) {
            return 0;
        }

        if (row < 0 || column < 0 || row >= int(m_blockIds.size())) {
            return 0;
        }

        const auto& blockRow = m_blockIds[row];
        if (column >= int(blockRow.size())) {
            return 0;
        }

        return blockRow[column];
    }

    const WinRayCast::BlockDefinition* blockAtCell(
        int row, int column) const noexcept
    {
        if (!m_hasBlockIds) {
            return nullptr;
        }

        return blockDefinition(blockIdAt(row, column));
    }

    const WinRayCast::BlockDefinition* blockAtWorld(
        double x, double y) const noexcept
    {
        if (!m_hasBlockIds) {
            return nullptr;
        }

        if (x < 0.0 || y < 0.0 || x >= getMaxX() || y >= getMaxY()) {
            return nullptr;
        }

        const auto col = static_cast<int>(x / getCellDx());
        const auto row = static_cast<int>(y / getCellDy());
        return blockAtCell(row, col);
    }

    bool usesBlockLayout() const noexcept { return m_hasBlockIds; }

private:
    bool setMapInfo(const Cell* array, uint32_t rows, uint32_t cols);

    struct DoorRuntimeState {
        double openAmount = 0.0;
        double closeDelay = 0.0;
    };

    using Row = std::vector<Cell>;
    using Matrix = std::vector<Row>;

    Matrix m_map;

    int m_cellDx = 256;
    int m_cellDy = 256;

    int m_maxX = 0;
    int m_maxY = 0;

    Point2d m_playerCellPos{ 0, 0 };
    double m_playerFacingDegrees = 0.0;
    bool m_hasPlayerStart = false;

    // Progressive-turn feel (degrees/second); keep in sync with the
    // WinRayCast.cpp fallbacks and the WinRaycastEditor defaults.
    double m_playerTurnBaseDegPerSec = 90.0;
    double m_playerTurnMaxDegPerSec = 300.0;
    double m_playerTurnAccelDegPerSecSq = 360.0;

    MessageLogConfig m_messageLog;

    std::vector<std::shared_ptr<Texture>> m_textures
        = std::vector<std::shared_ptr<Texture>>(256);

    TextureList m_textureList;

    WinRayCast::BlockDefinition m_blocks[256]{};
    bool m_blockPresent[256]{};
    std::vector<std::vector<WinRayCast::BlockId>> m_blockIds;
    std::vector<std::vector<DoorRuntimeState>> m_doorStates;
    std::vector<std::string> m_doorKeyring;
    bool m_hasBlockIds = false;
    double m_dynamicTextureTimeSeconds = 0.0;
    MapCell::TextureKey m_playerInteriorOverride = 0;
    int m_playerInteriorRow = -1;
    int m_playerInteriorCol = -1;
};


/* -------------------------------------------------------------------------- */
