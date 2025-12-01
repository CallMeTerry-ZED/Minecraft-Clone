/*
* © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef YOJIMBOMESSAGEFACTORY_H
#define YOJIMBOMESSAGEFACTORY_H

#pragma once

// This header MUST be included AFTER spdlog in .cpp files
// It includes yojimbo.h which defines macros that conflict with spdlog
#include <yojimbo.h>
#include "Networking/NetworkMessages.h"

namespace MinecraftClone
{
    // Message factory (must be defined where yojimbo.h is included)
    YOJIMBO_MESSAGE_FACTORY_START(GameMessageFactory, (int)GameMessageType::COUNT);
    YOJIMBO_DECLARE_MESSAGE_TYPE((int)GameMessageType::PLAYER_POSITION, PlayerPositionMessage);
    YOJIMBO_DECLARE_MESSAGE_TYPE((int)GameMessageType::BLOCK_UPDATE, BlockUpdateMessage);
    YOJIMBO_DECLARE_MESSAGE_TYPE((int)GameMessageType::PLAYER_JOINED, PlayerJoinedMessage);
    YOJIMBO_MESSAGE_FACTORY_FINISH();
}

#endif