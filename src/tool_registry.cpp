#include "tool_registry.h"

bool ToolRegistry::registerTool(const Tool& tool) {
    if (tool.name.empty()) {
        return false;
    }

    tools_[tool.name] = tool;
    return true;
}

std::optional<Tool> ToolRegistry::getTool(const std::string& toolName) const {
    auto it = tools_.find(toolName);

    if (it == tools_.end()) {
        return std::nullopt;
    }

    return it->second;
}

bool ToolRegistry::exists(const std::string& toolName) const {
    return tools_.find(toolName) != tools_.end();
}