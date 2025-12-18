#pragma once

#include "json.hpp"
#include "Message.h"

class BaseHandler
{
public:
    virtual nlohmann::json handle(const Message &msg) = 0;
    virtual ~BaseHandler() = default;


};

class ExampleHandler : public BaseHandler
{
public:
    nlohmann::json handle(const Message &msg) override;
    static nlohmann::json handlePing(const Message &msg);
    static nlohmann::json handleEcho(const Message &msg);
    static nlohmann::json handleAdd(const Message &msg);
};