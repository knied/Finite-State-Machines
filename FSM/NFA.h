//
//  NFA.h
//  FSM
//
//  Created by Kristof Niederholtmeyer on 27.03.14.
//  Copyright (c) 2014 Kristof Niederholtmeyer. All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __Parser__NFA__
#define __Parser__NFA__

////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <map>
#include <set>
#include <sstream>

#include "ActionFilter.h"

////////////////////////////////////////////////////////////////////////////////

// Nondeterministic Finite Automat
template<typename Action>
class NFA {
    typedef int State;
    typedef std::set<State> StateSet;
    
public:
    // Type definitions for the Evaluator
    typedef StateSet EvaluationState;
    typedef Action EvaluationAction;
    
private:
    typedef ActionFilter<Action> Filter;
    struct Transition {
        State destination;
        bool epsilon;
        Filter filter;
    }; // Transition
    typedef std::vector<Transition> Transitions;
    std::map<State, Transitions> _transition_table;
    StateSet _accepting_states;
    
public:
    typedef enum {
        Good,
        EmptyFilter
    } AddTransitionResult;
    
    // Adds or extends a filtered transition to the NFA
    AddTransitionResult add_transition(State source,
                                       Filter const& filter,
                                       State destination) {
        if (filter.empty()) {
            return EmptyFilter;
        }
        Transitions& transitions = _transition_table[source];
        for (Transition& t : transitions) {
            if (t.destination == destination && !t.epsilon) {
                t.filter += filter;
                return Good;
            }
        }
        transitions.push_back({ destination, false, filter });
        return Good;
    }
    
    // Adds an epsilon transition to the NFA
    AddTransitionResult add_transition(State source,
                                       State destination) {
        Transitions& transitions = _transition_table[source];
        for (Transition& t : transitions) {
            if (t.destination == destination && t.epsilon) {
                return Good;
            }
        }
        transitions.push_back({ destination, true, Filter() });
        return Good;
    }
    
    // Sets the set of accepting states.
    void set_accepting_states(StateSet const& accepting_states) {
        _accepting_states = accepting_states;
    }
    
    // Returns a set of all states that can be reached by epsilon transitions
    // from a state in 'set'.
    EvaluationState epsilon_closure(EvaluationState const& set) const {
        StateSet result = set;
        size_t old_size = 0;
        while (result.size() > old_size) {
            old_size = result.size();
            for (auto s : result) {
                auto it = _transition_table.find(s);
                if (it != _transition_table.end()) {
                    for (auto& t : it->second) {
                        if (t.epsilon) {
                            result.insert(t.destination);
                        }
                    }
                }
            }
        }
        
        return result;
    }
    
    // Creates a set of states, reachable by any action included in the filter.
    // If no such state exists, the method returns false and the output stays
    // unchanged.
    bool successor(EvaluationState const& from,
                   Filter const& filter,
                   EvaluationState& output) const {
        StateSet result;
        for (auto s : from) {
            auto it = _transition_table.find(s);
            if (it != _transition_table.end()) {
                for (auto& t : it->second) {
                    if (!t.epsilon && t.filter.includes(filter)) {
                        result.insert(t.destination);
                    }
                }
            }
        }
        if (result.size() <= 0) {
            return false;
        }
        output = epsilon_closure(result);
        return true;
    }
    
    // Creates a set of states, reachable by the action. If no such state
    // exists, the method returns false and the output stays unchanged.
    bool successor(EvaluationState const& from,
                   Action const& action,
                   EvaluationState& output) const {
        StateSet result;
        for (auto s : from) {
            auto it = _transition_table.find(s);
            if (it != _transition_table.end()) {
                for (auto& t : it->second) {
                    if (!t.epsilon && t.filter.includes(action)) {
                        result.insert(t.destination);
                    }
                }
            }
        }
        if (result.size() <= 0) {
            return false;
        }
        output = epsilon_closure(result);
        return true;
    }
    
    // Returns a vector of all atomic filters relevant to the given set of
    // states.
    std::vector<Filter> atomic_filters(StateSet const& set) const {
        std::vector<Filter> result;
        for (auto s : set) {
            auto it = _transition_table.find(s);
            if (it != _transition_table.end()) {
                for (auto& t : it->second) {
                    if (!t.epsilon) {
                        result.push_back(t.filter);
                    }
                }
            }
        }
        return atomize(result);
    }
    
    // Returns true if one of the states in the given set of states is an
    // accepting state.
    bool accepted(EvaluationState const& state) const {
        for (auto s : state) {
            if (_accepting_states.find(s) != _accepting_states.end()) {
                return true;
            }
        }
        return false;
    }
    
    // Returns the initial set of states.
    EvaluationState initial() const {
        return epsilon_closure({ 0 });
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
                if (t.epsilon) {
                    ss << "&#949;";
                } else {
                    ss << t.filter;
                }
                ss << "\" ];" << std::endl;
            }
        }
        ss << "}" << std::endl;
        return ss.str();
    }
}; // NFA

////////////////////////////////////////////////////////////////////////////////

#endif /* defined(__Parser__NFA__) */

////////////////////////////////////////////////////////////////////////////////
