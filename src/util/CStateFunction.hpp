// CStateFunction.hpp
// Copyright 2021 Giacomo Parolini @silverweed
// https://silverweed.github.io/Functional_State_Machines_in_C++/

#pragma once

#include <functional> // for std::function
#include <utility>    // for std::move

struct StateFunction {
	template <typename T>
	StateFunction(const T& f) : f(f) {}
	StateFunction(StateFunction&& s) = default;
	StateFunction(const StateFunction& s) = delete;
	StateFunction operator()() {
		return f();
	}
	StateFunction& operator=(StateFunction&& s) {
		f = std::move(s.f);
		return *this;
	}
	StateFunction& operator=(const StateFunction&) = delete;
	operator bool() const { return bool(f); }
	std::function<StateFunction()> f;
};

