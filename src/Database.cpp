#include "cdlg/Database.h"

namespace cdlg {

void Database::clear() {
    scripts_.clear();
}

void Database::addScript(DialogueScript script) {
    scripts_[script.id] = std::move(script);
}

void Database::merge(Database other) {
    for (auto& entry : other.scripts_) {
        scripts_[entry.first] = std::move(entry.second);
    }
}

bool Database::hasScript(const std::string& dialogueId) const {
    return scripts_.find(dialogueId) != scripts_.end();
}

const DialogueScript* Database::findScript(const std::string& dialogueId) const {
    const auto it = scripts_.find(dialogueId);
    if (it == scripts_.end()) {
        return nullptr;
    }
    return &it->second;
}

const DialogueNode* Database::findNode(const std::string& dialogueId, const std::string& nodeId) const {
    const DialogueScript* script = findScript(dialogueId);
    if (script == nullptr) {
        return nullptr;
    }
    const auto it = script->nodes.find(nodeId);
    if (it == script->nodes.end()) {
        return nullptr;
    }
    return &it->second;
}

const std::unordered_map<std::string, DialogueScript>& Database::scripts() const {
    return scripts_;
}

} // namespace cdlg
