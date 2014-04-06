//
//  ActionFilter.h
//  FSM
//
//  Created by Kristof Niederholtmeyer on 27.03.14.
//  Copyright (c) 2014 Kristof Niederholtmeyer. All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __Parser__ActionFilter__
#define __Parser__ActionFilter__

////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>

////////////////////////////////////////////////////////////////////////////////

template<typename Action>
class ActionRange {
    Action _front;
    Action _back;
    
public:
    ActionRange(Action const& action) : _front(action), _back(action) {}
    ActionRange(Action const& front, Action const& back)
    : _front(front), _back(back) {
        if (_front > _back) {
            throw std::runtime_error("ActionRange: back is greater than front.");
        }
    }
    
    Action const& front() const { return _front; }
    Action const& back() const { return _back; }
    
    bool includes(Action const& action) const {
        return action >= _front && action <= _back;
    }
    
    bool operator == (ActionRange<Action> const& range) const {
        return _front == range._front && _back == range._back;
    }
}; // ActionRange

template<typename Action>
std::ostream& operator << (std::ostream& stream,
                           ActionRange<Action> const& range) {
    if (range.front() == range.back()) {
        stream << "'" << range.front() << "'";
    } else {
        stream << "['" << range.front() << "' - '" << range.back() << "']";
    }
    return stream;
}

template<typename Action>
bool intersection(ActionRange<Action> const& a,
                  ActionRange<Action> const& b,
                  ActionRange<Action>& result) {
    if (a.front() <= b.front()) {
        if (a.back() >= b.front()) {
            // intersection: a0--------a1      or a0-------------a1
            //                    b0--------b1         b0---b1
            result = ActionRange<Action>(b.front(),
                                         std::min(a.back(), b.back()));
            return true;
        }
    } else {
        if (b.back() >= a.front()) {
            // intersection: b0--------b1      or b0-------------b1
            //                    a0--------a1         a0---a1
            result = ActionRange<Action>(a.front(),
                                         std::min(a.back(), b.back()));
            return true;
        }
    }
    return false;
}

template<typename Action>
bool intersecting(ActionRange<Action> const& a,
                  ActionRange<Action> const& b) {
    if (a.front() <= b.front()) {
        if (a.back() >= b.front()) {
            return true;
        }
    } else {
        if (b.back() >= a.front()) {
            return true;
        }
    }
    return false;
}

// Note: only works for integer types (because of the +1)
template<typename Action>
bool touching(ActionRange<Action> const& a,
              ActionRange<Action> const& b) {
    if (a.front() <= b.front()) {
        if ((a.back() + 1) >= b.front()) {
            return true;
        }
    } else {
        if ((b.back() + 1) >= a.front()) {
            return true;
        }
    }
    return false;
}

template<typename Action>
bool merge(ActionRange<Action> const& a,
           ActionRange<Action> const& b,
           ActionRange<Action>& result) {
    if (touching(a, b)) {
        result = ActionRange<Action>(std::min(a.front(), b.front()),
                                     std::max(a.back(), b.back()));
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

template<typename Action>
class ActionFilter {
    typedef ActionRange<Action> Range;
    std::vector<Range> _ranges;
    
public:
    ActionFilter() {}
    ActionFilter(Action const& action) : _ranges{ Range(action) } {}
    ActionFilter(Range const& range) : _ranges{ range } {}
    
    ActionFilter<Action> const& operator = (Range const& range) {
        _ranges = { range };
        return *this;
    }
    
    ActionFilter<Action> const& operator += (Range range) {
        for (int i = 0; i < _ranges.size(); ++i) {
            if (merge(_ranges[i], range, range)) {
                _ranges.erase(_ranges.begin() + i);
                i--;
            }
        }
        _ranges.push_back(range);
        return *this;
    }
    
    ActionFilter<Action> const& operator += (ActionFilter<Action> const& filter) {
        for (auto& r : filter.ranges()) {
            *this += r;
        }
        return *this;
    }
    
    ActionFilter<Action> const& operator -= (Range const& range) {
        std::vector<Range> new_ranges;
        for (auto const& r : _ranges) {
            if (intersecting(range, r)) {
                if (range.front() > r.front()) {
                    new_ranges.push_back(Range(r.front(),
                                               range.front() - 1));
                }
                if (range.back() < r.back()) {
                    new_ranges.push_back(Range(range.back() + 1,
                                               r.back()));
                }
            } else {
                new_ranges.push_back(r);
            }
        }
        _ranges = new_ranges;
        return *this;
    }
    
    ActionFilter<Action> const& operator -= (ActionFilter<Action> const& filter) {
        for (auto& r : filter.ranges()) {
            *this -= r;
        }
        return *this;
    }
    
    std::vector<Range> const& ranges() const { return _ranges; }
    
    bool includes(Action const& action) const {
        for (auto const& r : _ranges) {
            if (r.includes(action)) {
                return true;
            }
        }
        return false;
    }
    bool includes(ActionFilter<Action> const& filter) const {
        return (filter - *this).empty();
    }
    
    bool operator == (ActionFilter<Action> const& filter) const {
        if (_ranges.size() != filter._ranges.size()) {
            return false;
        }
        for (int i = 0; i < _ranges.size(); ++i) {
            if (_ranges[i] != filter._ranges[i]) {
                return false;
            }
        }
        return true;
    }
    
    bool empty() const { return _ranges.size() <= 0; }
}; // ActionFilter

template<typename Action>
ActionFilter<Action> operator + (ActionRange<Action> const& a,
                                 ActionRange<Action> const& b) {
    return ActionFilter<Action>(a) + b;
}

template<typename Action>
ActionFilter<Action> operator + (ActionFilter<Action> const& a,
                                 ActionRange<Action> const& b) {
    return ActionFilter<Action>(a) += b;
}

template<typename Action>
ActionFilter<Action> operator + (ActionRange<Action> const& a,
                                 ActionFilter<Action> const& b) {
    return ActionFilter<Action>(a) += b;
}

template<typename Action>
ActionFilter<Action> operator + (ActionFilter<Action> const& a,
                                 ActionFilter<Action> const& b) {
    return ActionFilter<Action>(a) += b;
}

template<typename Action>
ActionFilter<Action> operator - (ActionRange<Action> const& a,
                                 ActionRange<Action> const& b) {
    return ActionFilter<Action>(a) - b;
}

template<typename Action>
ActionFilter<Action> operator - (ActionFilter<Action> const& a,
                                 ActionRange<Action> const& b) {
    return ActionFilter<Action>(a) -= b;
}

template<typename Action>
ActionFilter<Action> operator - (ActionRange<Action> const& a,
                                 ActionFilter<Action> const& b) {
    return ActionFilter<Action>(a) -= b;
}

template<typename Action>
ActionFilter<Action> operator - (ActionFilter<Action> const& a,
                                 ActionFilter<Action> const& b) {
    return ActionFilter<Action>(a) -= b;
}

template<typename Action>
bool intersecting(ActionFilter<Action> const& a,
                  ActionFilter<Action> const& b) {
    for (auto const& r0 : a.ranges()) {
        for (auto const& r1 : b.ranges()) {
            if (intersecting(r0, r1)) {
                return true;
            }
        }
    }
    return false;
}

template <typename Action>
ActionFilter<Action> intersection(ActionFilter<Action> const& a,
                                  ActionFilter<Action> const& b) {
    ActionRange<Action> r(0);
    ActionFilter<Action> result;
    for (auto const& r0 : a.ranges()) {
        for (auto const& r1 : b.ranges()) {
            if (intersection(r0, r1, r)) {
                result += r;
            }
        }
    }
    return result;
}

template<typename Action>
std::vector<ActionFilter<Action>> atomize(std::vector<ActionFilter<Action>> const& filters) {
    std::vector<ActionFilter<Action>> result;
    for (ActionFilter<Action> f : filters) {
        std::vector<ActionFilter<Action>> tmp_result;
        for (int i = 0; i < result.size(); ++i) {
            ActionFilter<Action> d = result[i] - f;
            if (!d.empty()) {
                tmp_result.push_back(d);
            }
            d = intersection(result[i], f);
            if (!d.empty()) {
                tmp_result.push_back(d);
            }
            f = f - result[i];
        }
        if (!f.empty()) {
            tmp_result.push_back(f);
        }
        result = tmp_result;
    }
    return result;
}

template<typename Action>
std::ostream& operator << (std::ostream& stream,
                           ActionFilter<Action> const& filter) {
    stream << "(";
    for (int i = 0; i < filter.ranges().size(); ++i) {
        if (i != 0) {
            stream << " | ";
        }
        stream << filter.ranges()[i];
    }
    stream << ")";
    return stream;
}

////////////////////////////////////////////////////////////////////////////////

#endif /* defined(__Parser__ActionFilter__) */

////////////////////////////////////////////////////////////////////////////////
