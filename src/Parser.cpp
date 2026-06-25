#include "cdlg/Parser.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <utility>

namespace cdlg {

namespace {

struct SourceLine {
    int lineNumber = 0;
    int indent = 0;
    std::string text;
};

std::string trim(const std::string& value) {
    std::size_t begin = 0;
    while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin])) != 0) {
        begin++;
    }
    std::size_t end = value.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        end--;
    }
    return value.substr(begin, end - begin);
}

std::vector<SourceLine> splitSourceLines(const std::string& source) {
    std::vector<SourceLine> lines;
    std::istringstream stream(source);
    std::string rawLine;
    int lineNumber = 0;

    while (std::getline(stream, rawLine)) {
        lineNumber++;

        int indent = 0;
        while (indent < static_cast<int>(rawLine.size())
               && rawLine[static_cast<std::size_t>(indent)] == ' ') {
            indent++;
        }

        std::string text = trim(rawLine.substr(static_cast<std::size_t>(indent)));
        if (text.empty() || text[0] == '#') {
            continue;
        }

        lines.push_back({lineNumber, indent, text});
    }

    return lines;
}

std::string commandToken(const std::string& text) {
    const std::size_t space = text.find(' ');
    if (space == std::string::npos) {
        return text;
    }
    return text.substr(0, space);
}

std::string argumentText(const std::string& text) {
    const std::size_t space = text.find(' ');
    if (space == std::string::npos) {
        return {};
    }
    return trim(text.substr(space + 1));
}

bool splitChoice(const std::string& text, std::string& label, std::string& target) {
    const std::size_t arrow = text.find("->");
  const std::size_t pipe = text.find('|');
    const std::size_t separator = arrow != std::string::npos ? arrow : pipe;
    if (separator == std::string::npos) {
        return false;
    }

    label = trim(text.substr(0, separator));
    target = trim(text.substr(separator + (arrow != std::string::npos ? 2 : 1)));
    return !label.empty() && !target.empty();
}

class Parser {
public:
    Parser(std::vector<SourceLine> lines, std::string sourceName)
        : lines_(std::move(lines)),
          sourceName_(std::move(sourceName)) {}

    ParseResult run() {
        while (cursor_ < lines_.size()) {
            const SourceLine& line = lines_[cursor_];
            const std::string command = commandToken(line.text);
            if (line.indent != 0 || command != "dialogue") {
                if (line.indent == 0 && command == "end") {
                    cursor_++;
                    continue;
                }
                error(line, "expected top-level 'dialogue <id>' block");
                cursor_++;
                continue;
            }
            parseDialogue();
        }

        ParseResult result;
        result.database = std::move(database_);
        result.errors = std::move(errors_);
        return result;
    }

private:
    void parseDialogue() {
        const SourceLine& header = lines_[cursor_];
        const std::string dialogueId = argumentText(header.text);
        if (dialogueId.empty()) {
            error(header, "dialogue id is required");
            cursor_++;
            return;
        }

        DialogueScript script;
        script.id = dialogueId;
        cursor_++;

        const int bodyIndent = header.indent + 2;
        while (cursor_ < lines_.size()) {
            const SourceLine& line = lines_[cursor_];
            if (line.indent == 0) {
                if (commandToken(line.text) == "end") {
                    cursor_++;
                }
                break;
            }
            if (line.indent != bodyIndent) {
                error(line, "unexpected indentation inside dialogue block");
                cursor_++;
                continue;
            }

            const std::string command = commandToken(line.text);
            if (command == "entry") {
                script.entryNodeId = argumentText(line.text);
                if (script.entryNodeId.empty()) {
                    error(line, "entry node id is required");
                }
                cursor_++;
                continue;
            }

            if (command == "node") {
                parseNode(script, bodyIndent);
                continue;
            }

            if (command == "end") {
                cursor_++;
                break;
            }

            error(line, "expected 'entry', 'node', or 'end' inside dialogue block");
            cursor_++;
        }

        validateScript(script);
        database_.addScript(std::move(script));
    }

    void parseNode(DialogueScript& script, int dialogueIndent) {
        const SourceLine& header = lines_[cursor_];
        const std::string nodeId = argumentText(header.text);
        if (nodeId.empty()) {
            error(header, "node id is required");
            cursor_++;
            return;
        }

        DialogueNode node;
        node.id = nodeId;
        cursor_++;

        const int bodyIndent = dialogueIndent + 2;
        while (cursor_ < lines_.size()) {
            const SourceLine& line = lines_[cursor_];
            if (line.indent <= dialogueIndent) {
                break;
            }
            if (line.indent != bodyIndent) {
                error(line, "unexpected indentation inside node block");
                cursor_++;
                continue;
            }

            const std::string command = commandToken(line.text);
            if (command == "speaker") {
                Instruction instruction;
                instruction.kind = InstructionKind::Speaker;
                instruction.speaker = argumentText(line.text);
                if (instruction.speaker.empty()) {
                    error(line, "speaker name is required");
                } else {
                    node.instructions.push_back(std::move(instruction));
                }
                cursor_++;
                continue;
            }

            if (command == "line") {
                Instruction instruction;
                instruction.kind = InstructionKind::Line;
                instruction.text = argumentText(line.text);
                if (instruction.text.empty()) {
                    error(line, "line text is required");
                } else {
                    node.instructions.push_back(std::move(instruction));
                }
                cursor_++;
                continue;
            }

            if (command == "choice") {
                Instruction instruction;
                instruction.kind = InstructionKind::Choice;
                const std::string choiceText = argumentText(line.text);
                if (!splitChoice(choiceText, instruction.text, instruction.targetNode)) {
                    error(line, "choice must use '<label> -> <node>' or '<label> | <node>'");
                } else {
                    node.instructions.push_back(std::move(instruction));
                }
                cursor_++;
                continue;
            }

            if (command == "goto") {
                Instruction instruction;
                instruction.kind = InstructionKind::Goto;
                instruction.targetNode = argumentText(line.text);
                if (instruction.targetNode.empty()) {
                    error(line, "goto target node is required");
                } else {
                    node.instructions.push_back(std::move(instruction));
                }
                cursor_++;
                continue;
            }

            if (command == "set") {
                Instruction instruction;
                instruction.kind = InstructionKind::SetFlag;
                const std::string args = argumentText(line.text);
                const std::size_t space = args.find(' ');
                if (space == std::string::npos) {
                    error(line, "set requires '<flag> <value>'");
                } else {
                    instruction.flagKey = trim(args.substr(0, space));
                    instruction.flagValue = trim(args.substr(space + 1));
                    if (instruction.flagKey.empty() || instruction.flagValue.empty()) {
                        error(line, "set requires non-empty flag and value");
                    } else {
                        node.instructions.push_back(std::move(instruction));
                    }
                }
                cursor_++;
                continue;
            }

            if (command == "end") {
                Instruction instruction;
                instruction.kind = InstructionKind::End;
                node.instructions.push_back(std::move(instruction));
                cursor_++;
                break;
            }

            error(line, "unknown node instruction");
            cursor_++;
        }

        if (script.nodes.count(node.id) > 0) {
            error(header, "duplicate node id '" + node.id + "'");
        } else {
            script.nodes.emplace(node.id, std::move(node));
        }
    }

    void validateScript(const DialogueScript& script) {
        if (script.entryNodeId.empty()) {
            errors_.push_back({0, 0, sourceName_, "dialogue '" + script.id + "' is missing entry node"});
            return;
        }
        if (script.nodes.count(script.entryNodeId) == 0) {
            errors_.push_back(
                {0, 0, sourceName_, "dialogue '" + script.id + "' entry '" + script.entryNodeId + "' not found"});
        }

        for (const auto& nodeEntry : script.nodes) {
            const DialogueNode& node = nodeEntry.second;
            for (const Instruction& instruction : node.instructions) {
                if (instruction.kind == InstructionKind::Goto || instruction.kind == InstructionKind::Choice) {
                    const std::string& target = instruction.targetNode;
                    if (script.nodes.count(target) == 0) {
                        errors_.push_back(
                            {0,
                             0,
                             sourceName_,
                             "dialogue '" + script.id + "' node '" + node.id + "' references missing node '" + target
                                 + "'"});
                    }
                }
            }
            if (node.instructions.empty()) {
                errors_.push_back(
                    {0, 0, sourceName_, "dialogue '" + script.id + "' node '" + node.id + "' has no instructions"});
            }
        }
    }

    void error(const SourceLine& line, const std::string& message) {
        errors_.push_back({line.lineNumber, 1, sourceName_, message});
    }

    std::vector<SourceLine> lines_;
    std::string sourceName_;
    std::size_t cursor_ = 0;
    Database database_;
    std::vector<ParseError> errors_;
};

} // namespace

bool ParseResult::success() const {
    return errors.empty();
}

ParseResult parseString(const std::string& source, const std::string& sourceName) {
    Parser parser(splitSourceLines(source), sourceName);
    return parser.run();
}

ParseResult parseFile(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        ParseResult result;
        result.errors.push_back({0, 0, path, "failed to open file"});
        return result;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return parseString(buffer.str(), path);
}

} // namespace cdlg
