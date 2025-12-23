#pragma once

#include "json.hpp"
#include "Message.h"

namespace msgnet
{

class ExampleMessageHandler
{
public:
    static nlohmann::json handlePing(const Message &msg);
    static nlohmann::json handleEcho(const Message &msg);
    static nlohmann::json handleAdd(const Message &msg);
};

} // namespace msgnet