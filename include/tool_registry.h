#pragma once

#include "models.h"
#include <unordered_map>
#include <optional>
#include <string>

class ToolRegistry {
public:
    bool registerTool(const Tool& tool);
    std::optional<Tool> getTool(const std::string& toolName) const;
    bool exists(const std::string& toolName) const;

private:
    std::unordered_map<std::string, Tool> tools_;
};