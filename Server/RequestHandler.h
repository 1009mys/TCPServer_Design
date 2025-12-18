#include "json.hpp"

class RequestHandler
{
public:
    nlohmann::json handlePing(int client_id, const nlohmann::json &req);
    nlohmann::json handleEcho(int client_id, const nlohmann::json &req);
    nlohmann::json handleAdd(int client_id, const nlohmann::json &req);
};