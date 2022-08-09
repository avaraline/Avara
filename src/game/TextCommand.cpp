
#include "TextCommand.h"

std::vector<TextCommand*> TextCommand::registeredCommands;

void TextCommand::Register(TextCommand* command) {
    registeredCommands.push_back(command);
}

bool TextCommand::FindMatchingCommands(std::string& fullCommand,
                                 std::function<bool(TextCommand *, VectorOfArgs)> matchCb) {
    bool success = false;

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
            success = matchCb(command, vargs);
            if (success) {
                break;
            }
        }
    }

    return success;
}

const bool TextCommand::ExecuteMatchingCallbacks(std::string& fullCommand) {

    return FindMatchingCommands(fullCommand,
                                [](TextCommand* command, VectorOfArgs vargs) -> bool {
        return command->callback(vargs);
    });
}

const std::string TextCommand::ListOfCommands(std::string delimeter) {
    std::string cmdList;
    std::string delim;
    for (auto cmd : registeredCommands) {
        cmdList += delim + cmd->commandStr;
        delim = delimeter;
    }
    return cmdList;
}

std::string TextCommand::UsageForCommand(std::string& fullCommand) {
    std::string usage;

    FindMatchingCommands(fullCommand,
                         [&](TextCommand* command, VectorOfArgs vargs) -> bool {
        usage = command->GetUsage();
        return true;
    });

    return usage;
}
