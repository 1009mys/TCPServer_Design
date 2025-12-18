#pragma once

#include "json.hpp"
#include "Message.h"


class ExampleHandler
{
public:
    static nlohmann::json handlePing(const Message &msg);
    static nlohmann::json handleEcho(const Message &msg);
    static nlohmann::json handleAdd(const Message &msg);
};