#include <unordered_map>
#include <functional>
#include <utility>

#include "json.hpp"
#include "Message.h"
#include "RequestHandler.h"

using namespace nlohmann;
using namespace std;

class Dispatcher
{
public:
    using Handler = function<json(const Message &)>;

    void registerHandler(const string &type, Handler fn)
    {
        handlers_[type] = move(fn);
    }

    json dispatch(const Message &msg) const
    {
        // 통신에 필요한 항목만 체크한다.

        const json &req = msg.json;
        const int client_id = msg.client_id;

        // type 체크
        if (!req.contains("type") || !req["type"].is_string())
        {
            return makeError(req, "missing_or_invalid_type");
        }

        if (!req.contains("req_id"))
        {
            return makeError(req, "missing_req_id");
        }

        const string type = req["type"].get<string>();
        auto it = handlers_.find(type);
        if (it == handlers_.end())
        {
            return makeError(req, "unknown_type: " + type);
        }

        try
        {
            return it->second(msg);
        }
        catch (const exception &e)
        {
            return makeError(req, string("handler_exception: ") + e.what());
        }
    }

private:
    unordered_map<string, Handler> handlers_;

    static json makeError(const json &req, const string &reason)
    {
        json res;
        res["type"] = "error";
        res["ok"] = false;
        res["reason"] = reason;
        if (req.contains("req_id"))
            res["req_id"] = req["req_id"];
        return res;
    }
};
