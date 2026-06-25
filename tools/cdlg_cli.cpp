#include "cdlg/cdlg.h"

#include <iostream>

namespace {

void printUsage() {
    std::cout
        << "cdlg-cli <file.cdlg> [--dump]\n"
        << "  Parse .cdlg dialogue scripts without linking the game.\n";
}

void dumpScript(const cdlg::DialogueScript& script) {
    std::cout << "dialogue " << script.id << " entry=" << script.entryNodeId << '\n';
    for (const auto& nodeEntry : script.nodes) {
        const cdlg::DialogueNode& node = nodeEntry.second;
        std::cout << "  node " << node.id << " (" << node.instructions.size() << " instructions)\n";
        for (const cdlg::Instruction& instruction : node.instructions) {
            switch (instruction.kind) {
            case cdlg::InstructionKind::Speaker:
                std::cout << "    speaker " << instruction.speaker << '\n';
                break;
            case cdlg::InstructionKind::Line:
                std::cout << "    line " << instruction.text << '\n';
                break;
            case cdlg::InstructionKind::Choice:
                std::cout << "    choice " << instruction.text << " -> " << instruction.targetNode << '\n';
                break;
            case cdlg::InstructionKind::Goto:
                std::cout << "    goto " << instruction.targetNode << '\n';
                break;
            case cdlg::InstructionKind::SetFlag:
                std::cout << "    set " << instruction.flagKey << ' ' << instruction.flagValue << '\n';
                break;
            case cdlg::InstructionKind::End:
                std::cout << "    end\n";
                break;
            }
        }
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    const std::string path = argv[1];
    bool dump = false;
    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "--dump") {
            dump = true;
        }
    }

    const cdlg::ParseResult result = cdlg::parseFile(path);
    if (!result.success()) {
        for (const cdlg::ParseError& error : result.errors) {
            std::cerr << error.source << ':' << error.line << ": error: " << error.message << '\n';
        }
        return 2;
    }

    std::cout << "parsed " << result.database.scripts().size() << " dialogue(s)\n";
    if (dump) {
        for (const auto& entry : result.database.scripts()) {
            dumpScript(entry.second);
        }
    }

    return 0;
}
