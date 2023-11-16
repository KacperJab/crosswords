//
// Created by Kacper Jablonski on 14/11/2023.
//

#include "crosswords.h"

#include <iostream>

using std::min;
using std::max;
using std::string;
using std::weak_ordering;
using std::pair;

Word::Word(size_t x, size_t y, orientation_t o, const string &s)
        : starting_pos(pos_t(x, y))
        , orientation(o)
        , content(s) {
    if (s.empty())
        content = DEFAULT_WORD;
}

pos_t Word::get_start_position() const {
    return starting_pos;
}

pos_t Word::get_end_position() const {
    return pos_t(starting_pos.first + (!orientation) * (length() - 1), starting_pos.second + orientation * (length() - 1));
}

orientation_t Word::get_orientation() const {
    return orientation;
}

char Word::at(size_t position) const {
    if(length() <= position)
        return DEFAULT_CHAR;
    return content[position];
}

size_t Word::length() const {
    return content.size();
}

weak_ordering Word::operator <=> (const Word &word) const {
    return pair(starting_pos, orientation) <=> pair(word.starting_pos, word.orientation);
}

bool Word::operator != (const Word &word) const {
    return *this < word or word < *this;
}

bool Word::operator == (const Word &word) const {
    return *this <= word and word <= *this;
}

RectArea Word::rect_area() const {
    return RectArea(get_start_position(), get_end_position());
}

pos_t RectArea::get_left_top() const {
    return left_top;
}

pos_t RectArea::get_right_bottom() const {
    return right_bottom;
}

void RectArea::set_left_top(pos_t lt) {
    left_top = lt;
}

void RectArea::set_right_bottom(pos_t rb) {
    right_bottom = rb;
}

dim_t RectArea::size() const {
    size_t x = right_bottom.first < left_top.first ? 0 : right_bottom.first - left_top.first + 1;
    size_t y = right_bottom.second < left_top.second ? 0 : right_bottom.second - left_top.second + 1;
    return min(x, y) == 0 ? dim_t(0, 0) : dim_t(x, y);
}

bool RectArea::empty() const {
    return size() == dim_t(0, 0);
}

RectArea RectArea::operator * (const RectArea &rectArea) const {
    if(empty() or rectArea.empty())
        return DEFAULT_EMPTY_RECT_AREA;

    pos_t lt1 = get_left_top(), rb1 = get_right_bottom();
    pos_t lt2 = rectArea.get_left_top(), rb2 = rectArea.get_right_bottom();

    if(rb1.first < lt2.first or rb2.first < lt1.first)
        return DEFAULT_EMPTY_RECT_AREA;
    if(rb1.second < lt2.second or rb2.second < lt1.second)
        return DEFAULT_EMPTY_RECT_AREA;

    pos_t lt3 = pos_t(max(lt1.first, lt2.first), max(lt1.second, lt2.second));
    pos_t rb3 = pos_t(min(rb1.first, rb2.first), min(rb1.second, rb2.second));

    return RectArea(lt3, rb3);
}

RectArea& RectArea::operator *= (const RectArea &rectArea) {
    return *this = *this * rectArea;
}

void RectArea::embrace(pos_t pos) {
    left_top.first = min(left_top.first, pos.first);
    left_top.second = min(left_top.second, pos.second);
    right_bottom.first = max(right_bottom.first, pos.first);
    right_bottom.second = max(right_bottom.second, pos.second);
}

// CROSSWORD

dim_t Crossword::size() {
    return dim_t{h_size, v_size};
}

dim_t Crossword::word_count() {
    return dim_t{h_count, v_count};
}

bool Crossword::insert_word(Word &word) {
    for (auto w: _words) {
        if (!word_cross(word, w)) {
            return false;
        }
    }
    _words.push_back(word);
    _area.embrace(word.get_start_position());
    _area.embrace(word.get_end_position());
    if (word.get_orientation()) {
        v_count++;
    } else {
        h_count++;
    }
    return true;
}


bool Crossword::word_cross(Word &w1, Word &w2) {
    auto r1 = RectArea(w1.get_start_position(), w1.get_end_position());
    auto r2 = RectArea(w2.get_start_position(), w2.get_end_position());

    if (w1.get_orientation() != w1.get_orientation()) {
        auto common = r1 * r2;
        if (common.size() == dim_t{1,1}) {
            auto point = common.get_left_top();
            auto a = letter_at(w1, point);
            auto b = letter_at(w2, point);
            return a == b;
        }
    } else {
        // extend size of one word by 1 in each direction
        auto w1s = w1.get_start_position();
        auto w1e = w1.get_end_position();
        auto x1 = (w1s.first == 0) ? 0 : w1s.first - 1;
        auto y1 = (w1s.second == 0) ? 0 : w1s.second - 1;
        auto x2 = (w1e.first == SIZE_T_MAX) ? w1e.first : w1e.first + 1;
        auto y2 = (w1e.second == SIZE_T_MAX) ? w1e.second : w1e.second + 1;
        auto r3 = RectArea({x1, y1},{x2, y2});
        return (r2 * r3).empty();
    }
}

char Crossword::letter_at(Word &word, pos_t p) {
    if (word.get_start_position().first == p.first) {
        return word.at(p.second - word.get_start_position().second);
    } else {
        return word.at(p.first - word.get_start_position().first);
    }
}

Crossword Crossword::operator+(const Crossword &other) {
    auto res = *this;
    for (auto w: other._words) {
        res.insert_word(w);
    }
    return res;
}

Crossword &Crossword::operator+=(const Crossword &other) {
    for (auto w: other._words) {
        insert_word(w);
    }
    return *this;
}

Crossword::Crossword(Crossword &&crossword) noexcept :
        _words(std::move(crossword._words)),
        _area(std::move(crossword._area)),
        h_count(crossword.h_count),
        v_count(crossword.v_count),
        h_size(crossword.h_size),
        v_size(crossword.v_size) {
    if (_area.empty()) {
        _area = DEFAULT_EMPTY_RECT_AREA;
    }
}

Crossword &Crossword::operator=(const Crossword &other) {
    if (this != &other) {
        _words = other._words;
        _area = other._area;
        h_count = other.h_count;
        v_count = other.v_count;
        h_size = other.h_size;
        v_size = other.v_size;
    }
    return *this;
}

Crossword &Crossword::operator=(Crossword &&other) noexcept {
    if (this != &other) {
        _words = std::move(other._words);
        _area = std::move(other._area);
        h_count = other.h_count;
        v_count = other.v_count;
        h_size = other.h_size;
        v_size = other.v_size;
        if (_area.empty()) {
            _area = DEFAULT_EMPTY_RECT_AREA;
        }
    }
    return *this;
}

void Crossword::print(std::ostream &os) const {
    auto x = _area.get_left_top().first;
    auto y = _area.get_left_top().second;
    auto rows = _area.size().first + 2;
    auto cols = _area.size().second + 2;
    std::vector<std::vector<char>> printable_area(rows, std::vector<char>(cols, CROSSWORD_BACKGROUND));
    for (const auto& word: _words) {
        auto start_x = word.get_start_position().first - x + 1;
        auto start_y = word.get_start_position().second - y + 1;
        if (word.get_orientation()) {
            // vertical
            for (int i = 0; i < word.length(); i++) {
                printable_area[start_x][start_y + i] = word.at(i);
            }
        } else {
            //horizontal
            for (int i = 0; i < word.length(); i++) {
                printable_area[start_x + i][start_y] = word.at(i);
            }
        }
    }
    for (size_t i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            os << printable_area[i][j];
            if (j < cols - 1) {
                os << ' ';
            }
        }
        os << '\n';
    }
}

Crossword::Crossword(Word &word, const std::initializer_list<Word> &words) :
        _area(DEFAULT_EMPTY_RECT_AREA)
{
    insert_word(word);
    for (auto w: words) {
        insert_word(w);
    }
}