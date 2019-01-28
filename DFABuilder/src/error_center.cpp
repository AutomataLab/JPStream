#include "error_center.hpp"

namespace dragontooth {

void ErrorCenter::log(const std::string& msg, int level) {
    switch (level) {
        case ml_fatal: *out << "FATAL: "; break;
        case ml_error: *out << "ERROR: "; break;
        case ml_warn: *out << "WARN: "; break;
        case ml_info: *out << "INFO: "; break;
        case ml_debug: *out << "DEBUG: "; break;
    }
    *out << msg << std::endl;
}

}  // namespace dragontooth
