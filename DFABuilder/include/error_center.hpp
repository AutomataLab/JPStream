#pragma once

#include <string>
#include <iostream>
#include <fstream>

namespace dragontooth {

enum MsgLevel {
    ml_debug = 1,
    ml_info = 2,
    ml_warn = 4,
    ml_error = 8,
    ml_fatal = 16,
    ml_default = ml_warn | ml_error | ml_fatal,
    ml_all = ml_debug | ml_info | ml_default
};

/**
 * @brief ErrorCenter is a class for recording error message and debug message.
 */
class ErrorCenter {
public:
    ErrorCenter() : level(ml_default) {}
    ErrorCenter(unsigned int level) : level(level) {}
    ~ErrorCenter() {
        if (need_close) {
            std::ofstream* os = (std::ofstream*)out;
            os->close();
        }
    }

    void Fatal(const std::string& msg) { log(msg, ml_fatal); }
    void Error(const std::string& msg) { log(msg, ml_error); }
    void Warn(const std::string& msg) { log(msg, ml_warn); }
    void Info(const std::string& msg) { log(msg, ml_info); }
    void Debug(const std::string& msg) { log(msg, ml_debug); }
    
    void setLevel(unsigned int level) { this->level = level; }
    void setOutput(std::ostream& os) { out = &os; }
    void setOutputFile(std::fstream& os) { out = &os; }
    void setOutputFile(const std::string& path) { 
        out = new std::ofstream(path); 
        need_close = true;
    }
    
protected:
    void log(const std::string& msg, int level);
    unsigned int level;
    std::ostream* out; 
    bool need_close = false;
};

}  // namespace dragontooth