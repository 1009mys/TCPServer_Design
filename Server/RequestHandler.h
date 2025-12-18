#include "json.hpp"
#include "Message.h"

class RequestHandler
{
public:
    nlohmann::json handlePing(const Message &msg);
    nlohmann::json handleEcho(const Message &msg);
    nlohmann::json handleAdd(const Message &msg);
};