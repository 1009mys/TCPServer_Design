#include <iostream>
#include "json.hpp"
#include "RequestHandler.h"
#include "Message.h"
#include "Logger.h"

using nlohmann::json;
using namespace std;


json ExampleMessageHandler::handlePing(const Message &msg)
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

    LOG_DEBUG("[RequestHandler] Received ping from client ", client_id);

    json res;
    res["type"] = "pong";
    res["ok"] = true;
    res["req_id"] = req["req_id"];

    res["payload"] = {
        {"server_ts", static_cast<int64_t>(time(nullptr))}};

    return res;
}

json ExampleMessageHandler::handleEcho(const Message &msg)
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

    LOG_DEBUG("[RequestHandler] Received echo from client ", client_id);

    json res;
    res["type"] = "echo_resp";
    res["ok"] = true;
    res["req_id"] = req["req_id"];

    res["payload"] = req.value("payload", json::object());

    return res;
}

json ExampleMessageHandler::handleAdd(const Message &msg)
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

    LOG_DEBUG("[RequestHandler] Received add request from client ", client_id);

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
