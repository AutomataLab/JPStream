#include "pass.hpp"

namespace dragontooth {

IPassable* Pass::ExecuteAll(IPassable* data) {
        IPassable* result;
        Setup();
        if (join_point == nullptr) result = Execute(data);
        else result = Execute(data, join_point->getData<IPassable>());
        if (branch_pass) {
            IPassable* br_result = Branch(data);
            if (br_result == nullptr) br_result = result;
            branch_pass->ExecuteAll(br_result);
        }
        if (next_pass == nullptr) return result;
        return next_pass->ExecuteAll(result);
    }

}