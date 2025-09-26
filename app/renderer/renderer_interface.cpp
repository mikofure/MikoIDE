#include "renderer_interface.hpp"
#include "../utils/logger.hpp"

// Default implementations for common functionality

IRenderer::BufferUpdateStats IRenderer::GetBufferStats() const {
    // Default implementation returns empty stats
    // Derived classes should override this with actual statistics
    return BufferUpdateStats{0, 0, 0, 0.0};
}

void IRenderer::LogPerformanceStats() {
    BufferUpdateStats stats = GetBufferStats();
    Logger::LogMessage("Renderer Performance Stats:");
    Logger::LogMessage("  Total Updates: " + std::to_string(stats.totalUpdates));
    Logger::LogMessage("  Cache Hits: " + std::to_string(stats.cacheHits));
    Logger::LogMessage("  Cache Misses: " + std::to_string(stats.cacheMisses));
    Logger::LogMessage("  Average Update Time: " + std::to_string(stats.avgUpdateTime) + "ms");
    
    if (stats.totalUpdates > 0) {
        double hitRate = (double)stats.cacheHits / stats.totalUpdates * 100.0;
        Logger::LogMessage("  Cache Hit Rate: " + std::to_string(hitRate) + "%");
    }
}

std::string IRenderer::GetAdapterInfo() {
    // Default implementation - derived classes should override
    return "Generic Renderer - No adapter info available";
}

bool IRenderer::CheckFeatureSupport() {
    // Default implementation - derived classes should override with specific checks
    Logger::LogMessage("CheckFeatureSupport: Using default implementation");
    return true;
}

void IRenderer::EnableVSync(bool enable) {
    // Default implementation - derived classes should override
    Logger::LogMessage("EnableVSync: " + std::string(enable ? "enabled" : "disabled") + " (default implementation)");
}

void IRenderer::SetMultiSampleCount(int samples) {
    // Default implementation - derived classes should override
    Logger::LogMessage("SetMultiSampleCount: " + std::to_string(samples) + " samples (default implementation)");
}

void IRenderer::EnablePartialUpdates(bool enable) {
    // Default implementation - derived classes should override
    Logger::LogMessage("EnablePartialUpdates: " + std::string(enable ? "enabled" : "disabled") + " (default implementation)");
}

void IRenderer::ClearTextureCache() {
    // Default implementation - derived classes should override
    Logger::LogMessage("ClearTextureCache: Using default implementation");
}

void IRenderer::SetDirtyRegion(int x, int y, int width, int height) {
    // Default implementation - derived classes should override
    Logger::LogMessage("SetDirtyRegion: (" + std::to_string(x) + ", " + std::to_string(y) + 
                      ", " + std::to_string(width) + ", " + std::to_string(height) + ") (default implementation)");
}