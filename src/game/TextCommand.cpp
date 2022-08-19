#include <sstream>      // std::istringstream
#include "TextCommand.h"

TextCommand::TextCommand(const char* usage, TextCommandCallbackArgs cback) {
    usageStr = usage;
    // it is assumed that /command is the first word in usage
    std::istringstream iss(usage);
    iss >> commandStr;
    callbackArgs = cback;
    callbackCmd = NULL;
}

TextCommand::TextCommand(const char* usage, TextCommandCallbackCmd cback) {
    usageStr = usage;
    // it is assumed that /command is the first word in usage
    std::istringstream iss(usage);
    iss >> commandStr;
    callbackArgs = NULL;
    callbackCmd = cback;
}

std::vector<TextCommand*> TextCommand::registeredCommands;

void TextCommand::Register(TextCommand* command) {
    registeredCommands.push_back(command);
}

bool TextCommand::FindMatchingCommands(std::string& fullCommand,
                                       std::function<bool(TextCommand *, std::string&, VectorOfArgs)> matchCb) {
    bool success = false;

    // must match at least 2 chars (eg. /xyz must have at least /x)
    if (fullCommand.length() < 2) {
        return false;
    }

    std::istringstream iss(fullCommand);
    std::string cmd;
    // std::operator>>(istringstream) tokenizes the stream on whitepace
    iss >> cmd;

    // put arguments into a vector
    std::string arg;
    VectorOfArgs vargs;
    while (iss >> arg) {
        vargs.push_back(arg);
    }

    // loop over registered commands until a match is found
    for (auto command : registeredCommands) {
        // if the typed command is a substring of this registered command string...
        // Example: "/lo" will match to commands "/load" or "/lol" so the first match would be returned
        if (command->commandStr.rfind(cmd, 0) == 0) {
            success = matchCb(command, cmd, vargs);
            if (success) {
                break;
            }
        }
    }

    return success;
}

const bool TextCommand::ExecuteMatchingCallbacks(std::string& fullCommand) {

    return FindMatchingCommands(fullCommand,
                                [](TextCommand* command, std::string& cmd, VectorOfArgs vargs) -> bool {
        if (command->callbackArgs) {
            return command->callbackArgs(vargs);
        } else if (command->callbackCmd) {
            return command->callbackCmd(cmd);
        }
        return false;
    });
}

const std::string TextCommand::ListOfCommands(std::string delimeter) {
    return std::accumulate(registeredCommands.begin() + 1,
                           registeredCommands.end(),
                           registeredCommands[0]->commandStr,
        [&](std::string s0, TextCommand* const& tc) { return s0 += delimeter + tc->commandStr; });
}

std::string TextCommand::UsageForCommand(std::string& fullCommand) {
    std::string usage;

    FindMatchingCommands(fullCommand,
                         [&](TextCommand* command, std::string cmd, VectorOfArgs vargs) -> bool {
        usage = command->GetUsage();
        return true;
    });

    return usage;
}
