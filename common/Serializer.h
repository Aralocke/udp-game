#pragma once

#include "Message.h"

namespace Common
{
extern template class Serializer<AcknowledgeMessage>;
extern template class Serializer<LoginMessage>;
extern template class Serializer<Message>;
extern template class Serializer<PingMessage>;
}
