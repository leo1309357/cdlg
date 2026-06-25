#ifndef CDLG_SESSION_H
#define CDLG_SESSION_H

#include "Database.h"
#include "Types.h"

#include <string>
#include <vector>

namespace cdlg {

enum class SessionState {
    Inactive,
    ShowingLine,
    WaitingChoice,
    Ended
};

class Session {
public:
    bool start(const Database& database, const std::string& dialogueId);
    void reset();

    bool isActive() const;
    bool isWaitingForChoice() const;
    SessionState state() const;

    const std::string& dialogueId() const;
    const std::string& currentNodeId() const;
    const std::string& speaker() const;
    const std::string& line() const;
    const std::vector<ChoiceOption>& choices() const;

    bool advanceLine();
    bool choose(std::size_t choiceIndex);

    std::vector<FlagAssignment> takePendingFlags();

private:
    void enterNode(const std::string& nodeId);
    void processUntilPause();
    void finish();

    const Database* database_ = nullptr;
    const DialogueScript* script_ = nullptr;
    SessionState state_ = SessionState::Inactive;
    std::string dialogueId_;
    std::string currentNodeId_;
    std::string speaker_;
    std::string line_;
    std::vector<ChoiceOption> choices_;
    std::vector<FlagAssignment> pendingFlags_;
    std::size_t stepIndex_ = 0;
};

} // namespace cdlg

#endif
