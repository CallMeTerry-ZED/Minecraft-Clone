/*
* © 2025 ZED Interactive. All Rights Reserved.
 */

#include <spdlog/spdlog.h>

#include "Networking/NetworkMessages.h"

namespace MinecraftClone
{
    // Explicit template instantiations
    template bool PlayerPositionMessage::Serialize<yojimbo::ReadStream>(yojimbo::ReadStream&);
    template bool PlayerPositionMessage::Serialize<yojimbo::WriteStream>(yojimbo::WriteStream&);
    template bool PlayerPositionMessage::Serialize<yojimbo::MeasureStream>(yojimbo::MeasureStream&);

    template bool BlockUpdateMessage::Serialize<yojimbo::ReadStream>(yojimbo::ReadStream&);
    template bool BlockUpdateMessage::Serialize<yojimbo::WriteStream>(yojimbo::WriteStream&);
    template bool BlockUpdateMessage::Serialize<yojimbo::MeasureStream>(yojimbo::MeasureStream&);

    template bool PlayerJoinedMessage::Serialize<yojimbo::ReadStream>(yojimbo::ReadStream&);
    template bool PlayerJoinedMessage::Serialize<yojimbo::WriteStream>(yojimbo::WriteStream&);
    template bool PlayerJoinedMessage::Serialize<yojimbo::MeasureStream>(yojimbo::MeasureStream&);
}