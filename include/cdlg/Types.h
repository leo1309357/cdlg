#ifndef CDLG_TYPES_H
#define CDLG_TYPES_H

#include <string>
#include <unordered_map>
#include <vector>

namespace cdlg {

enum class InstructionKind {
    Speaker,
    Line,
    Choice,
    Goto,
    SetFlag,
    End
};

struct Instruction {
    InstructionKind kind = InstructionKind::Line;
    std::string speaker;
    std::string text;
    std::string targetNode;
    std::string flagKey;
    std::string flagValue;
};

struct DialogueNode {
    std::string id;
    std::vector<Instruction> instructions;
};

struct DialogueScript {
    std::string id;
    std::string entryNodeId;
    std::unordered_map<std::string, DialogueNode> nodes;
};

struct ChoiceOption {
    std::string label;
    std::string targetNodeId;
};

struct FlagAssignment {
    std::string key;
    std::string value;
};

} // namespace cdlg

#endif
