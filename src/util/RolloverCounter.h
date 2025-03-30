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

    const static T halfMax = std::numeric_limits<T>::max() / 2;

    int diff(T x, T y) {
        // This is the crux of this class.
        // Integer subtraction across a rollover point will give a positive result if the
        // operands are cast (i.e. T(x-y)) because the C standard defines it as a modulo behavior.
        // It should be noted that *unsigned* integers have this well-defined behavior:
        //     https://en.wikipedia.org/wiki/Integer_overflow
        // But *signed* integers may or may not have this behavior so it's best to
        // template this class with an unsigned integer type even though it will probably
        // work fine with signed integers too.
        int diff = 0;
        if (x > y) {
            if ((x - y) <= halfMax) {
                diff = (x - y);
            } else {
                diff = -T(y - x);
            }
        } else if (x < y) {
            if ((y - x) < halfMax) {
                diff = (x - y);
            } else {
                diff = T(x - y);
            }
        }
        return diff;
    }

public:
    RolloverCounter(T initialValue = 0) {
        counter = initialValue;
    }

    // casting operator so you can assign to a variable of type T
    operator T()   const { return counter; }


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
        return RolloverCounter<T>(diff(counter, value));
    }
    RolloverCounter<T> operator-(int value) {
        return RolloverCounter<T>(diff(counter, value));
    }
    
    // when subtracting another RolloverCounter<T>, it is comparing 2 RolloverCounter objects so return value of type int (can be negative)
    int operator-(const RolloverCounter<T>& that) {
        return diff(counter, that.counter);
    }

    bool operator>(const T value) { return diff(counter, value) > 0; }

    bool operator>=(const T value) { return !(*this < value); }

    bool operator<(const T value) { return diff(counter, value) < 0; }

    bool operator<=(const T value) { return !(*this > value); }

    bool operator==(const T value) { return (counter == value); }
    // bool operator==(const int value) { return (counter == value); }

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
