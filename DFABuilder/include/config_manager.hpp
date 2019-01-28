
#pragma once

#include <map>
#include <string>

namespace dragontooth {

class ConfigManager {
public:
    ConfigManager() { initDefault(); }
    ~ConfigManager() {}

    int ParseArgs(int argc, char** argv);

    bool& Switch(const std::string name) { return switchs.at(name); }
    std::string& Value(const std::string name) { return values.at(name); } 

    bool getOptionSwitch(const std::string& name) { return switchs.at(name); }
    std::string getOptionValue(const std::string& name) {
        return values.at(name);
    }

    std::string lexical_analysis() { return values["lexical_analysis"]; }
    std::string gramma_analysis() { return values["gramma_analysis"]; }

    std::string output() { return values["output"]; }

    // this one will open the support of utf8, it will automatically create some
    // states for one Unicode char.
    bool utf8() { return switchs["utf8"]; }

    // this one will support Estring, automatically decode and tranlsate into
    // utf16, then we will use equivalent class to tranlate the input char. It
    // can not both open with utf8
    bool estring() { return switchs["estring"]; }

    // It will first mapping the char into equivalence class, then using the
    // class id as the input char
    bool equivalence_class() { return switchs["equivalence_class"]; }

private:
    void initDefault();

    std::map<std::string, bool> switchs;
    std::map<std::string, std::string> values;
};

}  // namespace dragontooth