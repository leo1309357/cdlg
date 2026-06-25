#ifndef CDLG_DATABASE_H
#define CDLG_DATABASE_H

#include "Types.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace cdlg {

class Database {
public:
    void clear();

    void addScript(DialogueScript script);
    void merge(Database other);

    bool hasScript(const std::string& dialogueId) const;
    const DialogueScript* findScript(const std::string& dialogueId) const;
    const DialogueNode* findNode(const std::string& dialogueId, const std::string& nodeId) const;

    const std::unordered_map<std::string, DialogueScript>& scripts() const;

private:
    std::unordered_map<std::string, DialogueScript> scripts_;
};

} // namespace cdlg

#endif
