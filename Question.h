
#ifndef QUESTION_H
#define QUESTION_H

using namespace std;
#include <string>
#include <utility>
#include <vector>
#include <utility>
#include <algorithm>

class Question {
public:
    std::string text;
    int page;
    std::vector<std::tuple<Question, double>> topComparisons;
    int x, y, width, height;
    bool hasLocation = false;

    Question(string txt, int page, int x, int y, int w, int h)
    : text(std::move(txt)), page(page), x(x), y(y), width(w), height(h) {}

    void addComparison(const Question& q2, double s);

    [[nodiscard]] std::string toString() const;

    void updateLocation(int x, int y, int w, int h);
};


#endif //QUESTION_H
