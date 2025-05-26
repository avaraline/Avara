/*
    Copyright ©2020, Tom Anderson (tra on github)
    All rights reserved.

    File: RolloverCounter.h
*/

#pragma once

#include <limits>
#include <iostream>

template<class T>
class RolloverCounter {
private:
    T counter;

    const static bool isUnsigned = (std::numeric_limits<T>::min() == 0);
    const static T midPoint = (std::numeric_limits<T>::min() +
                               std::numeric_limits<T>::max()) / 2;

        // Integer subtraction across a rollover point will give a positive result if the
        // operands are cast (i.e. T(x-y)) because the C standard defines it as a modulo behavior.
        // It should be noted that *unsigned* integers have this well-defined behavior:
        //     https://en.wikipedia.org/wiki/Integer_overflow
        // But *signed* integers may or may not have this behavior so it's best to
        // template this class with an unsigned integer type even though it will probably
        // work fine with signed integers too.

    // used by comparison operators to compare against midPoint
    T compare(T x, T y) {
        if (isUnsigned) {
            // with unsigned types the midpoint will be MAX/2 so we use the x-y value
            return (x - y);
        } else {
            // for signed types the midpoint will be 0 so compare the y-x value
            return (y - x);
        }
    }

public:
    RolloverCounter(T initialValue = 0) {
        counter = initialValue;
    }

    // casting operator so you can assign to a variable of type T
    operator T() const { return counter; }


    RolloverCounter<T>& operator++() {  // ++prefix
        counter++;
        return *this;
    }

    RolloverCounter<T> operator++(int) {  // postfix++
        RolloverCounter<T> temp(counter);
        counter++;
        return temp;  // return temp copy
    }

    RolloverCounter<T>& operator+=(T increment) {
        counter += increment;
        return *this;
    }
    // the 'int' version makes it easier to += any number/constant/enum
    RolloverCounter<T>& operator+=(int increment) {
        counter += increment;
        return *this;
    }
    
    // when adding a value, assumed to be a small delta and that you want a RolloverCounter object returned
    RolloverCounter<T> operator+(T value) {
        return RolloverCounter<T>(counter + value);
    }
    RolloverCounter<T> operator+(int value) {
        return RolloverCounter<T>(counter + value);
    }

    // when subtracting a value, assumed to be a small-ish delta and that you want a RolloverCounter object returned
    RolloverCounter<T> operator-(T value) {
        return RolloverCounter<T>(counter - value);
    }
    RolloverCounter<T> operator-(int value) {
        return RolloverCounter<T>(counter - value);
    }
    
    // when subtracting another RolloverCounter<T>, it is comparing 2 RolloverCounter objects so return value of type int (can be negative)
    int operator-(const RolloverCounter<T>& that) {
        T diff;
        if (*this < that) {
            // in case we have unsigned counters, subtract bigger value then negate
            diff = that.counter - counter;
            return -int(diff);
        }
        diff = counter - that.counter;
        return diff;
    }

    bool operator>=(const T value) { return compare(counter, value) <= midPoint; }
    bool operator<(const T value) { return !(*this >= value); }

    bool operator<=(const T value) { return compare(value, counter) <= midPoint; }
    bool operator>(const T value) { return !(*this <= value); }

    bool operator==(const T value) { return (counter == value); }

    T operator&(T value) { return counter & value; }
    T operator&(int value) { return counter & value; }
    
    T operator&=(T value) { return (counter &= value); }
    T operator&=(int value) { return (counter &= value); }
    
    // so you can do things like: cout << "my counter = " << rc << endl;
    friend std::ostream& operator<<(std::ostream& os, const RolloverCounter<T>& rc)
    {
        if (sizeof(T) == 1) {
            // want chars to print as numbers
            os << (int)rc.counter;
        } else {
            os << rc.counter;
        }
        return os;
    }
};
