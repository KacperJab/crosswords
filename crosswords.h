//
// Created by Kacper Jablonski on 13/11/2023.
//

#ifndef CROSSWORDS_CROSSWORDS_H
#define CROSSWORDS_CROSSWORDS_H
#include "vector"
#include <string>
#include <compare>
#include <utility>
#include <cstddef>

enum orientation_t {H = false, V = true};
using pos_t=std::pair<size_t, size_t>;
using dim_t=std::pair<size_t, size_t>;

constexpr std::string DEFAULT_WORD = "?";
constexpr char DEFAULT_CHAR = '?';

class RectArea {
private:
    pos_t left_top, right_bottom;
public:
    constexpr RectArea(pos_t lt, pos_t rb)
            : left_top(lt)
            , right_bottom(rb) {}

    RectArea(const RectArea &rectArea) = default;
    RectArea(RectArea &&rectArea) = default;

    RectArea& operator = (const RectArea &rectArea) = default;
    RectArea& operator = (RectArea &&rectArea) = default;

    pos_t get_left_top() const;
    pos_t get_right_bottom() const;

    void set_left_top(pos_t lt);
    void set_right_bottom(pos_t rb);

    dim_t size() const;
    bool empty() const;

    RectArea operator * (const RectArea &rectArea) const;
    RectArea& operator *= (const RectArea &rectArea);

    void embrace(pos_t pos);
};

class Word {
private:
    pos_t starting_pos;
    orientation_t orientation;
    std::string content;

public:
    Word(size_t x, size_t y, orientation_t o, const std::string &s);

    Word(const Word &word) = default;
    Word(Word &&word) = default;

    Word& operator = (const Word &word) = default;
    Word& operator = (Word &&word) = default;

    std::weak_ordering operator<=>(const Word &word) const;

    bool operator == (const Word &word) const;
    bool operator != (const Word &word) const;

    pos_t get_start_position() const;
    pos_t get_end_position() const;
    orientation_t get_orientation() const;
    char at(size_t position) const;

    size_t length() const;

    RectArea rect_area() const;
};

constexpr RectArea DEFAULT_EMPTY_RECT_AREA(pos_t(1, 1), pos_t(0, 0));

inline constexpr char CROSSWORD_BACKGROUND = '.';

class Crossword {
private:
    std::vector<Word> _words;
    RectArea _area;
    size_t h_count = 0;
    size_t v_count = 0;
    size_t h_size = 0;
    size_t v_size = 0;

    static bool word_cross(Word &w1, Word &w2);
    static char letter_at(Word &word, pos_t p);
    void print(std::ostream &ostream) const;

public:
    Crossword(Word& word, const std::initializer_list<Word>& words);
    Crossword(const Crossword &crossword) = default;
    Crossword(Crossword &&crossword) noexcept;

    Crossword& operator = (const Crossword &other);
    Crossword& operator = (Crossword &&other) noexcept;

    dim_t size();
    dim_t word_count();
    bool insert_word(Word &word);

    Crossword operator +(const Crossword& other);
    Crossword& operator +=(const Crossword& other);

    friend std::ostream& operator<<(std::ostream& os, const Crossword& crossword) {
        crossword.print(os);
        return os;
    }
};

#endif //CROSSWORDS_CROSSWORDS_H
