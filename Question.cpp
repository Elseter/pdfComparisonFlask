


#include "Question.h"
#include <algorithm>

void Question::addComparison(const Question& q2, double s) {
    topComparisons.emplace_back(q2, s);
    std::sort(topComparisons.begin(), topComparisons.end(), [](const auto& a, const auto& b) {
        return std::get<1>(a) > std::get<1>(b);
    });
    if (topComparisons.size() > 10) {
        topComparisons.pop_back();
    }
}

std::string Question::toString() const {
    std::string result = text + " (Page: " + std::to_string(page) +
                         ", x: " + std::to_string(x) +
                         ", y: " + std::to_string(y) +
                         ", width: " + std::to_string(width) +
                         ", height: " + std::to_string(height) + ")\n";
    return result;
}

void Question::updateLocation(int x, int y, int w, int h) {
    this->x = x;
    this->y = y;
    this->width = w;
    this->height = h;
    this->hasLocation = true;
}