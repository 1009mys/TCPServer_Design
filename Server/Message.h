#include "json.hpp"

#pragma once

namespace msgnet
{

struct Message
{
    int client_id;
    nlohmann::json json;
};

} // namespace msgnet