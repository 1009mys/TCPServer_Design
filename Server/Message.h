#include "json.hpp"

struct Message {
    int client_id;
    nlohmann::json json;
};
