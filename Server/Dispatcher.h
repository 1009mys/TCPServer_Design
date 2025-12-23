#include <unordered_map>
#include <functional>
#include <utility>

#include "json.hpp"
#include "Message.h"
#include "ExampleMessageHandler.h"

namespace msgnet
{

class Dispatcher
{
public:
    using MessageHandler = std::function<nlohmann::json(const Message &)>;

    void registerMessageHandler(const std::string &type, MessageHandler fn)
    {
        handlers_[type] = std::move(fn);
    }

    nlohmann::json dispatch(const Message &msg) const
    {
        // 통신에 필요한 항목만 체크한다.

        const nlohmann::json &req = msg.json;
        const int client_id = msg.client_id;

        // type 체크
        if (!req.contains("type") || !req["type"].is_string())
        {
            return makeError(req, "missing_or_invalid_type");
        }

        const std::string type = req["type"].get<std::string>();
        auto it = handlers_.find(type);
        if (it == handlers_.end())
        {
            return makeError(req, "unknown_type: " + type);
        }

        try
        {
            return it->second(msg);
        }
        catch (const std::exception &e)
        {
            return makeError(req, std::string("handler_exception: ") + e.what());
        }
    }

private:
    std::unordered_map<std::string, MessageHandler> handlers_;

    static nlohmann::json makeError(const nlohmann::json &req, const std::string &reason)
    {
        nlohmann::json res;
        res["type"] = "error";
        res["ok"] = false;
        res["reason"] = reason;
        if (req.contains("req_id"))
            res["req_id"] = req["req_id"];
        return res;
    }
};

} // namespace msgnet