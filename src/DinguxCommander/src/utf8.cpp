#include "utf8.h"

#include <algorithm>
#include <cstddef>

namespace utf8 {

void replaceTabsWithSpaces(std::string *line, std::size_t tab_width)
{
    const std::size_t num_tabs = std::count(line->begin(), line->end(), '\t');
    if (num_tabs == 0) return;
    std::string result;
    result.reserve(line->size() + num_tabs * (tab_width - 1));
    std::size_t prev_tab_end = 0;
    std::size_t column = 0;
    for (std::size_t i = 0; i < line->size();
         i += codePointLen(line->data() + i)) {
        if ((*line)[i] == '\t') {
            result.append(*line, prev_tab_end, i - prev_tab_end);
            const std::size_t num_spaces = tab_width - (column % tab_width);
            result.append(num_spaces, ' ');
            prev_tab_end = i + 1;
            column += num_spaces;
        } else {
            ++column;
        }
    }
    result.append(*line, prev_tab_end, std::string::npos);
    *line = std::move(result);
}

} // namespace utf8
