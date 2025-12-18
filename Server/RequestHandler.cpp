#include <iostream>
#include "json.hpp"
#include "RequestHandler.h"

using nlohmann::json;
using namespace std;

json RequestHandler::handlePing(int /*client_id*/, const json &req)
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

json RequestHandler::handleEcho(int /*client_id*/, const json &req)
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

json RequestHandler::handleAdd(int /*client_id*/, const json &req)
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
