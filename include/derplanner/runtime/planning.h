//
// Copyright (c) 2015 Alexander Shafranov shafranov@gmail.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef DERPLANNER_RUNTIME_PLANNING_H_
#define DERPLANNER_RUNTIME_PLANNING_H_

#include "derplanner/runtime/assert.h"
#include "derplanner/runtime/memory.h"
#include "derplanner/runtime/types.h"

namespace plnnr {

/// Planning_State

struct Planning_State_Config
{
    // maximum expansion depth.
    size_t max_depth;
    // maximum plan length.
    size_t max_plan_length;
    // maximum size of the expansions data block (arguments & preconditions).
    size_t expansion_data_size;
    // maximum size of the plan data block (arguments & preconditions).
    size_t plan_data_size;
};

Planning_State create_planning_state(Memory* mem, Planning_State_Config config);
void destroy(Memory* mem, Planning_State& s);

/// Stack

template <typename T>
inline T* top(const Stack<T>& stack)
{
    return (stack.size > 0) ? stack.frames + (stack.size - 1) : 0;
}

template <typename T>
inline T* pop(Stack<T>& stack)
{
    plnnr_assert(stack.size > 0);
    T* result = stack.frames + (stack.size - 1);
    --stack.size;
    return result;
}

template <typename T>
inline void push(Stack<T>& stack, const T& value)
{
    plnnr_assert(stack.size < stack.max_size);
    stack.frames[stack.size++] = value;
}

/// Planning functions.

bool find_plan(const Domain_Info* domain, Fact_Database* db, Planning_State* state);

}

#endif