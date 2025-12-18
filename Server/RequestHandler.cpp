#include <iostream>
#include "json.hpp"
#include "RequestHandler.h"
#include "Message.h"

using nlohmann::json;
using namespace std;

nlohmann::json ExampleHandler::handle(const Message &msg)
{
    const json &req = msg.json;
    const string type = req.value("type", "");

    if (type == "ping")
    {
        return handlePing(msg);
    }
    else if (type == "echo")
    {
        return handleEcho(msg);
    }
    else if (type == "add")
    {
        return handleAdd(msg);
    }
    else
    {
        json res;
        res["type"] = "error";
        res["ok"] = false;
        res["req_id"] = req.value("req_id", nullptr);
        res["error"] = "unknown_type: " + type;
        return res;
    }
}

json ExampleHandler::handlePing(const Message &msg)
{
    /*
     * 요청 형식:
     * {
     *   "type": "ping",
     *   "req_id": <요청 ID>,
     *   "payload": { }
     * }
     *
     */

    const json &req = msg.json;
    int client_id = msg.client_id;

#ifdef DEBUG_BUILD
    cout << "[RequestHandler] Received ping from client "
         << client_id << "\n";
#endif

    json res;
    res["type"] = "pong";
    res["ok"] = true;
    res["req_id"] = req["req_id"];

    res["payload"] = {
        {"server_ts", static_cast<int64_t>(time(nullptr))}};

    return res;
}

json ExampleHandler::handleEcho(const Message &msg)
{
    /*
     * 요청 형식:
     * {
     *   "type": "echo",
     *   "req_id": <요청 ID>,
     *   "payload": { ... 임의의 JSON ... }
     * }
     *
     */

    const json &req = msg.json;
    int client_id = msg.client_id;

#ifdef DEBUG_BUILD
    cout << "[RequestHandler] Received echo from client "
         << client_id << "\n";
#endif

    json res;
    res["type"] = "echo_resp";
    res["ok"] = true;
    res["req_id"] = req["req_id"];

    res["payload"] = req.value("payload", json::object());

    return res;
}

json ExampleHandler::handleAdd(const Message &msg)
{
    /*
     * 요청 형식:
     * {
     *   "type": "add",
     *   "req_id": <요청 ID>,
     *   "payload": {
     *       "a": <정수>,
     *       "b": <정수>
     *   }
     * }
     */

    const json &req = msg.json;
    int client_id = msg.client_id;

#ifdef DEBUG_BUILD
    cout << "[RequestHandler] Received add request\n";
#endif

    const auto &payload = req.at("payload");
    int a = payload.value("a", 0);
    int b = payload.value("b", 0);

    json res;
    res["type"] = "add_resp";
    res["ok"] = true;
    res["req_id"] = req["req_id"];

    res["payload"] = {
        {"sum", a + b}};

    return res;
}
