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
    DFA() {}
    
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
    
    // Adds or extends an existing transition. In case of a conflict with a pre
    // existing transition, only the non conflicting parts of the filter will
    // be inserted.
    void add_transition(State source,
                        Filter filter,
                        State destination) {
        if (filter.empty()) {
            return;
        }
        Transitions& transitions = _transition_table[source];
        
        // check for conflicts
        for (Transition& t : transitions) {
            if (t.destination != destination && intersecting(t.filter, filter)) {
                filter -= t.filter; // TODO: Some warning would be nice...
            }
        }
        if (filter.empty()) {
            return;
        }
        
        // add transition
        for (Transition& t : transitions) {
            if (t.destination == destination) {
                t.filter += filter;
                return;
            }
        }
        transitions.push_back({ destination, filter });
    }
    
    // Sets the set of accepting states.
    void set_accepting_states(StateSet const& accepting_states) {
        _accepting_states = accepting_states;
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
    
    // Creates a graphviz visualization.
    std::string graphviz(std::string const& name = "NFA") const {
        std::stringstream ss;
        ss << "digraph " << name << " {" << std::endl;
        ss << "  rankdir=LR;" << std::endl;
        ss << "  size=\"8,5\"" << std::endl;
        ss << "  node [shape = doublecircle];";
        for (auto a : _accepting_states) {
            ss << " S" << a;
        }
        ss << ";" << std::endl;
        ss << "  node [shape = circle];" << std::endl;
        for (auto transitions : _transition_table) {
            auto start = transitions.first;
            for (auto t : transitions.second) {
                auto end = t.destination;
                ss << "  S" << start << " -> S" << end << " [ label = \"";
                ss << t.filter;
                ss << "\" ];" << std::endl;
            }
        }
        ss << "}" << std::endl;
        return ss.str();
    }
}; // DFA

////////////////////////////////////////////////////////////////////////////////

#endif /* defined(__Parser__DFA__) */

////////////////////////////////////////////////////////////////////////////////
