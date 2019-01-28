#pragma once

#include "config_manager.hpp"
#include "error_center.hpp"

namespace dragontooth {

class DataHolder;
class IPassable {
public:
    virtual ~IPassable() = default;
};

/**
 * @brief Pass is the main interface for user to use this project.
 *
 * Each stage in this project can be changed the order and do some additonal job
 * by yourself. This is because of the Pass class. You can combine some stages
 * to a workflow. Each stage is a subclass of Pass so you can connect them and
 * add some additional pass to deal with the data as soon as you can keep the
 * transfrom data not change the interface.
 */
class Pass {
public:
    // Those two will be passed to all passes in this link
    void setAll(ErrorCenter* _ec, ConfigManager* _cm) {
        ec = _ec; cm = _cm;
        if (branch_pass) branch_pass->setAll(_ec, _cm);
        if (next_pass) next_pass->setAll(_ec, _cm);
    }

    virtual void Setup() {}

    // Execute function will do the main job in a pass, and generate a
    // meaningful result
    virtual IPassable* Execute(IPassable* data,
                               IPassable* join_data = nullptr) = 0;

    // Branch function is an optional function, if some pass will create another
    // workflow, it will be useful. The branch stage will run after this stage
    // and before the next stage, all of this branch stages will finish.
    virtual IPassable* Branch(IPassable* data) { return nullptr; }

    // This function will run all pass, but the behaiver can be changed in
    // specific pass
    virtual IPassable* ExecuteAll(IPassable* data);

    template <typename T>
    T* ExecuteAll(IPassable* data) {
        return (T*) ExecuteAll(data);
    }

    // Set the next pass
    Pass* next(Pass* pass) {
        next_pass = pass;
        return pass;
    }
    // Set the branch pass
    Pass* branch(Pass* pass) {
        branch_pass = pass;
        return this;
    }

    Pass* join(DataHolder* pass) {
        join_point = pass;
        return this;
    }

    inline void error(const std::string& msg) { ec->Error(msg); }
    inline void warn(const std::string& msg) { ec->Warn(msg); }
    inline void fatal(const std::string& msg) { ec->Fatal(msg); }
    inline void info(const std::string& msg) { ec->Info(msg); }
    inline void debug(const std::string& msg) { ec->Debug(msg); }

protected:
    ErrorCenter* ec = nullptr;
    ConfigManager* cm = nullptr;
    Pass* next_pass = nullptr;
    Pass* branch_pass = nullptr;
    DataHolder* join_point = nullptr;
};

/**
 * @brief It's a holder in the end of workflow. You can get the result from
 * here. especially useful when you have some branches.
 * 
 * DataHolder is used at the end of workflow to hold the result. This class also
 * can be used to join the data. When you want to combine one workflow's into
 * another, you can call join at the second workflow.
 */
class DataHolder : public Pass {
public:
    virtual IPassable* Execute(IPassable* _data, IPassable* _join_data) {
        data = _data;
        return _data;
    }

    template<typename T>
    T* getData() { return (T*)data; }

protected:
    IPassable* data;
};

}  // namespace dragontooth