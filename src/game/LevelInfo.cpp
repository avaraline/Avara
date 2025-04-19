//
//  LevelInfo.cpp
//  Avara
//
//  Created by Tom on 4/20/25.
//

#include "LevelInfo.h"

#include <regex>
#include "Tags.h"

std::vector<std::string> LevelInfo::Authors() const {
    // Try to extract author's name/handle from "designer" variable.  Here are some real examples (with names changed):
    //   Real Name <author>
    //   Real Name (author)  <email@example.com>
    //   Real Name [author]
    //   Real 'author' Name - email@example.com   << Gunner's Playeground
    //   Real "author" Name <email@example.com>   << Firing Line
    //   Author <email@example.com>
    //   Author -email@example.com-
    //   Author - email@example.com
    //   The Author - email@example.com
    //   Mr. Author
    //   X. Author
    //   Juri Author                              << if nothing else use last name
    //   me AUTHOR <example@email.com>
    //   Designed by Author
    //   design - author
    //   design - author1, teamwork - author2         << /load (teams)
    //   Author (with thanks to various people)
    //   KOTH scripting by Author1, layout by Author2 << Arsenic
    //   Name Author | email@example.com              << autriche
    //   Name Author -> http://avara.org              << slime
    //   author

    std::vector<std::string> authors;

    // first strip out emails (really anything with @ in it), and leading whitespace,-,–(emdash?) or |
    std::string result = std::regex_replace(designer, std::regex("[-–\\|\\s]+[^ ]+@[^ ]+"), "");
    // strip URLs too (slime, flying blocks...)
    result = std::regex_replace(result, std::regex("[-\\>\\s]+(http|e-mail)[^ ]+"), "");
    // remove trailing whitespace (spring)
    result = std::regex_replace(result, std::regex("\\s+$"), "");
    // lowercase the whole string so the returned names are lowercase
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);

    // now look for any case between brackets, parens or quotes but withOUT spaces
    std::smatch matches;
    if (std::regex_search(result, matches, std::regex("\\[([^ ]+)\\]")) ||
        std::regex_search(result, matches, std::regex("\\<([^ ]+)\\>")) ||
        std::regex_search(result, matches, std::regex("\\(([^ ]+)\\)")) ||
        std::regex_search(result, matches, std::regex("\\'([^ ]+)\\'")) ||
        std::regex_search(result, matches, std::regex("\"([^ ]+)\""))) {
        authors.push_back(matches.str(1));
    } else {
        // remove parens with multi-word text
        result = std::regex_replace(result, std::regex("[ ]+\\(.+\\)"), "");

        // look for the "- author" and "by author" cases here
        for (auto re : {"- ([^ ,]+)", "by ([^ ,]+)"}) {
            // start the initial search at the beginning of the string
            auto searchStr = result;
            while (std::regex_search(searchStr, matches, std::regex(re))) {
                authors.push_back(matches.str(1));
                // start the next search after this match
                searchStr = matches.suffix();
            }
        }

        if (authors.size() == 0) {
            // take the last word of whatever is left
            auto pos = result.find_last_of(" ");
            if (pos == std::string::npos) {
                authors.push_back(result);
            } else {
                authors.push_back(result.substr(pos+1)) ;
            }
        }
    }

    for (auto a : authors) {
        SDL_Log("author of %s is @%s\n", URL().c_str(), a.c_str());
    }
    return authors;
}

std::string LevelInfo::TagsString() const {
    return Tags::TagsStringForLevel(Tags::LevelURL(setTag, levelTag));
}
