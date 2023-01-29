/*
    Copyright ©2023, Tom Anderson (@tra on github)
    All rights reserved.

    File: SlidingHistogram.h
*/

#include <deque>
#include <vector>
#include <iostream>
#include <iomanip>
#include <math.h>

template<class T>
class SlidingHistogram {
private:
    // histogram cells
    T minValue;                         // minimum value of the histogram
    T maxValue;                         // max value allowed
    T stepSize;                         // incremental value between histogram cells
    std::vector<std::size_t> cellCounts;// number of data points in each histogram cell
    // histogram data
    std::deque<T> dataFifo;             // where data values are stored
    std::size_t windowSize;             // the size the dataFifo will grow to when full

    int displayWidth = 100;
    std::string displayTitle = "";

    std::size_t cellIndex(T value) {
        std::size_t idx;
        if (value < minValue) {
            idx = 0;
        } else if (value > maxValue) {
            idx = cellCounts.size() - 1;
        } else {
            idx = (value - minValue) / stepSize;
        }
        return idx;
    }

public:
    // Example: SlidingHistogram<double>(500, 0.0, 3.0, 0.5) keeps 500 data points with cellCounts from 0.0-3.0 in 0.5 increments
    SlidingHistogram(std::size_t dataSize, T minRange, T maxRange, T rangeStep) {
        windowSize = dataSize;
        minValue = minRange;
        maxValue = maxRange;
        stepSize = rangeStep;
        std::size_t numCells = (maxRange - minRange) / rangeStep;
        cellCounts.resize(numCells, 0);
    }

    void push(T value) {
        if (dataFifo.size() == windowSize) {
            // pop value off the front of data queue
            T front = dataFifo.front();
            dataFifo.pop_front();
            // decrement count of the matching cell
            cellCounts[cellIndex(front)]--;
        }
        // push data to the end of data queue
        dataFifo.push_back(value);
        // increment value of matching cell
        cellCounts[cellIndex(value)]++;
    }

    void setTitle(const std::string& title) {
        displayTitle = title;
    }

    std::size_t size() {
        return windowSize;
    }

    // output a ascii graph using: std::cout << mySlidingHistogram;
    friend std::ostream& operator<<(std::ostream& os, const SlidingHistogram<T>& sh)
    {
        int sum = 0;
        float markers[] = {0.5, 0.90, 0.95, 0.99};  // smallest to biggest
        int nextMarker = 0;
        int prec = -log2(sh.stepSize);  // approx how many digits to show

        if (sh.displayTitle.size() > 0) {
            os << std::setfill(' ') << std::setw(prec+5) << " " << sh.displayTitle << std::endl;
        }

        for (int i = 0; i < sh.cellCounts.size(); i++) {
            sum += sh.cellCounts[i];
            // label
            os << std::setprecision(prec) << std::fixed << std::setw(prec+3) << sh.minValue + (i+1)*sh.stepSize << ": ";
            // histogram bar
            int ticks = sh.displayWidth * sh.cellCounts[i] / sh.dataFifo.size();
            for (int j = 0; j < ticks; j++) {
                os << '+';
            }
            if (sh.displayWidth * sh.cellCounts[i] % sh.dataFifo.size()) {
                // indicate a non-zero amount leftover
                os << "-  ";
            }
            // show where marker percentages are
            while (nextMarker < std::size(markers) && sum >= sh.dataFifo.size() * markers[nextMarker]) {
                os.unsetf(std::ios::fixed);
                os << " (" << std::setprecision(3) << (markers[nextMarker] * 100) << "%)";
                nextMarker++;
            }
            os << std::endl;
            if (sum == sh.dataFifo.size() && sh.cellCounts[i] == 0) {
                // if all remaining lines will be empty, don't draw them
                break;
            }
        }
        return os;
    }
};
