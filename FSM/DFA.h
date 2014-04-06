//
//  DFA.h
//  FSM
//
//  Created by Kristof Niederholtmeyer on 29.03.14.
//  Copyright (c) 2014 Kristof Niederholtmeyer. All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __Parser__DFA__
#define __Parser__DFA__

////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <set>
#include <map>
#include <vector>

#include "NFA.h"

////////////////////////////////////////////////////////////////////////////////

// Deterministic Finite Automat
template <typename Action>
class DFA {
    typedef int State;
    typedef std::set<State> StateSet;
    
public:
    // Type definitions for the Evaluator
    typedef State EvaluationState;
    typedef Action EvaluationAction;
    
private:
    typedef ActionFilter<Action> Filter;
    struct Transition {
        State destination;
        Filter filter;
    }; // Transition
    typedef std::vector<Transition> Transitions;
    std::map<State, Transitions> _transition_table;
    StateSet _accepting_states;
    
public:
    // DFAs can only be created from a NFA.
    DFA(NFA<Action> const& nfa) {
        struct TmpState {
            State state;
            StateSet set;
        }; // TmpState
        
        std::vector<TmpState> states{ { 0, nfa.epsilon_closure({ 0 }) } };
        size_t current = 0;
        State next_state = 1;
        std::vector<Filter> filters;
        
        while (current < states.size()) {
            filters = nfa.atomic_filters(states[current].set);
            for (Filter const& f : filters) {
                StateSet r;
                if (nfa.successor(states[current].set, f, r)) {
                    State s = 0;
                    
                    bool found = false;
                    for (auto const& t : states) {
                        if (t.set == r) {
                            found = true;
                            s = t.state;
                        }
                    }
                    
                    if (!found) {
                        s = next_state++;
                        if (nfa.accepted(r)) {
                            _accepting_states.insert(s);
                        }
                        states.push_back({ s, r });
                    }
                    
                    _transition_table[states[current].state].push_back({
                        s, f
                    });
                }
            }
            current++;
        }
    }
    
    // Finds the state reachable by the action. If no such state exists, the
    // method returns false and the output stays unchanged.
    bool successor(EvaluationState const& from,
                   EvaluationAction const& action,
                   EvaluationState& output) const {
        auto it = _transition_table.find(from);
        if (it == _transition_table.end()) {
            return false;
        }
        
        for (Transition const& t : it->second) {
            if (t.filter.includes(action)) {
                output = t.destination;
                return true;
            }
        }
        
        return false;
    }
    
    // Returns true if the state is an accepting state.
    bool accepted(EvaluationState const& state) const {
        return _accepting_states.find(state) != _accepting_states.end();
    }
    
    // Returns the initial state.
    EvaluationState initial() const {
        return 0;
    }
}; // DFA

////////////////////////////////////////////////////////////////////////////////

#endif /* defined(__Parser__DFA__) */

////////////////////////////////////////////////////////////////////////////////
