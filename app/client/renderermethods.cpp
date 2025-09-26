#include "windowed.hpp"
#include "../utils/logger.hpp"

void SDL3Window::EnableHardwareAcceleration(bool enable) {
  if (!cross_platform_renderer_) {
    Logger::LogMessage("EnableHardwareAcceleration: No renderer available");
    return;
  }

  if (enable) {
    // Enable hardware acceleration features if supported
    if (cross_platform_renderer_->CheckFeatureSupport()) {
      Logger::LogMessage("Hardware acceleration enabled for " + 
                         cross_platform_renderer_->GetRendererName());
    } else {
      Logger::LogMessage("Hardware acceleration not supported by " + 
                         cross_platform_renderer_->GetRendererName());
    }
  } else {
    Logger::LogMessage("Hardware acceleration disabled");
  }
}