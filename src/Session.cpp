#include "cdlg/Session.h"

namespace cdlg {

bool Session::start(const Database& database, const std::string& dialogueId) {
    reset();

    const DialogueScript* script = database.findScript(dialogueId);
    if (script == nullptr || script->entryNodeId.empty()) {
        return false;
    }

    database_ = &database;
    script_ = script;
    dialogueId_ = dialogueId;
    state_ = SessionState::ShowingLine;
    enterNode(script->entryNodeId);
    return state_ == SessionState::ShowingLine || state_ == SessionState::WaitingChoice;
}

void Session::reset() {
    database_ = nullptr;
    script_ = nullptr;
    state_ = SessionState::Inactive;
    dialogueId_.clear();
    currentNodeId_.clear();
    speaker_.clear();
    line_.clear();
    choices_.clear();
    pendingFlags_.clear();
    stepIndex_ = 0;
}

bool Session::isActive() const {
    return state_ == SessionState::ShowingLine || state_ == SessionState::WaitingChoice;
}

bool Session::isWaitingForChoice() const {
    return state_ == SessionState::WaitingChoice;
}

SessionState Session::state() const {
    return state_;
}

const std::string& Session::dialogueId() const {
    return dialogueId_;
}

const std::string& Session::currentNodeId() const {
    return currentNodeId_;
}

const std::string& Session::speaker() const {
    return speaker_;
}

const std::string& Session::line() const {
    return line_;
}

const std::vector<ChoiceOption>& Session::choices() const {
    return choices_;
}

bool Session::advanceLine() {
    if (state_ != SessionState::ShowingLine || script_ == nullptr) {
        return false;
    }

    stepIndex_++;
    processUntilPause();
    return isActive();
}

bool Session::choose(std::size_t choiceIndex) {
    if (state_ != SessionState::WaitingChoice || choiceIndex >= choices_.size()) {
        return false;
    }

    const std::string targetNode = choices_[choiceIndex].targetNodeId;
    choices_.clear();
    enterNode(targetNode);
    return isActive();
}

std::vector<FlagAssignment> Session::takePendingFlags() {
    std::vector<FlagAssignment> flags = std::move(pendingFlags_);
    pendingFlags_.clear();
    return flags;
}

void Session::enterNode(const std::string& nodeId) {
    if (script_ == nullptr) {
        finish();
        return;
    }

    const DialogueNode* node = script_->nodes.count(nodeId) > 0 ? &script_->nodes.at(nodeId) : nullptr;
    if (node == nullptr) {
        finish();
        return;
    }

    currentNodeId_ = nodeId;
    stepIndex_ = 0;
    processUntilPause();
}

void Session::processUntilPause() {
    if (script_ == nullptr) {
        finish();
        return;
    }

    const DialogueNode* node = script_->nodes.count(currentNodeId_) > 0 ? &script_->nodes.at(currentNodeId_) : nullptr;
    if (node == nullptr) {
        finish();
        return;
    }

    while (stepIndex_ < node->instructions.size()) {
        const Instruction& instruction = node->instructions[stepIndex_];

        switch (instruction.kind) {
        case InstructionKind::Speaker:
            speaker_ = instruction.speaker;
            stepIndex_++;
            continue;

        case InstructionKind::SetFlag:
            pendingFlags_.push_back({instruction.flagKey, instruction.flagValue});
            stepIndex_++;
            continue;

        case InstructionKind::Line:
            line_ = instruction.text;
            state_ = SessionState::ShowingLine;
            return;

        case InstructionKind::Choice: {
            choices_.clear();
            while (stepIndex_ < node->instructions.size()
                   && node->instructions[stepIndex_].kind == InstructionKind::Choice) {
                const Instruction& choiceInstruction = node->instructions[stepIndex_];
                choices_.push_back({choiceInstruction.text, choiceInstruction.targetNode});
                stepIndex_++;
            }
            state_ = SessionState::WaitingChoice;
            return;
        }

        case InstructionKind::Goto:
            enterNode(instruction.targetNode);
            return;

        case InstructionKind::End:
            finish();
            return;
        }
    }

    finish();
}

void Session::finish() {
    state_ = SessionState::Ended;
    choices_.clear();
    line_.clear();
}

} // namespace cdlg
