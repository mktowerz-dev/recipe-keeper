#include "TextUtils.h"
#include <algorithm>
#include <cctype>
#include <sstream>

std::string toLower(const std::string& text)
{
    std::string result = text;
    for (char& c : result)
    {
        // std::tolower needs an unsigned char, or letters like 'é' can crash it.
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

std::string trim(const std::string& text)
{
    const std::string whitespace = " \t\r\n";
    size_t start = text.find_first_not_of(whitespace);
    if (start == std::string::npos)
        return "";  // the text was all whitespace (or empty)

    size_t end = text.find_last_not_of(whitespace);
    return text.substr(start, end - start + 1);
}

std::vector<std::string> splitLines(const std::string& text)
{
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line))
    {
        line = trim(line);
        if (!line.empty())
            lines.push_back(line);
    }
    return lines;
}

std::string joinLines(const std::vector<std::string>& lines)
{
    std::string text;
    for (size_t i = 0; i < lines.size(); i++)
    {
        if (i > 0)
            text += "\n";  // newline between entries, but not after the last one
        text += lines[i];
    }
    return text;
}

bool containsIgnoreCase(const std::vector<std::string>& list, const std::string& value)
{
    std::string target = toLower(value);
    for (const std::string& item : list)
    {
        if (toLower(item) == target)
            return true;
    }
    return false;
}

// Returns true if word a should come before word b (A-Z, ignoring capitals).
static bool wordComesBefore(const std::string& a, const std::string& b)
{
    return toLower(a) < toLower(b);
}

void sortAlphabetically(std::vector<std::string>& list)
{
    std::sort(list.begin(), list.end(), wordComesBefore);
}

bool containsTextIgnoreCase(const std::string& text, const std::string& part)
{
    // Make both lowercase so "CHICK" and "chick" are treated the same,
    // then find() looks for part inside text. It returns std::string::npos
    // ("no position") when part isn't there.
    std::string lowerText = toLower(text);
    std::string lowerPart = toLower(trim(part));
    return lowerText.find(lowerPart) != std::string::npos;
}
