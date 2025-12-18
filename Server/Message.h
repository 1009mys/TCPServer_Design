#include "json.hpp"

#pragma once

struct Message
{
    int client_id;
    nlohmann::json json;
};
