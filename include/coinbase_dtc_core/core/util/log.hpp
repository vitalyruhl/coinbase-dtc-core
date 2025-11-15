#pragma once
#include <string>
#include <iostream>

// Simple logging functions that avoid namespace conflicts
namespace open_dtc_server
{
    namespace util
    {
        // Simple logging without the problematic log() function
        inline void simple_log(const std::string &message)
        {
            std::cout << message << std::endl;
        }

        // For compatibility
        inline void write_log(const std::string &message)
        {
            simple_log(message);
        }

        // Legacy compatibility - redirects to simple logging
        inline void log_debug(const std::string &message)
        {
// Reduced verbosity - only output in debug builds
#ifdef _DEBUG
            simple_log("[DEBUG] " + message);
#endif
        }

        inline void log_info(const std::string &message)
        {
            simple_log("[INFO] " + message);
        }

        inline void log_warning(const std::string &message)
        {
            simple_log("[WARNING] " + message);
        }

        inline void log_error(const std::string &message)
        {
            simple_log("[ERROR] " + message);
        }

    } // namespace util
} // namespace open_dtc_server
