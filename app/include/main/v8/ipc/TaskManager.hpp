#pragma once

#include "include/cef_v8.h"
#include <vector>
#include <string>

namespace app
{

namespace v8
{

namespace ipc
{

// Process information structure
struct ProcessInfo
{
    int id;
    std::string name;
    std::string type;
    int pid;
    double cpuUsage;
    double memoryUsage;
    double networkUsage;
    std::string status;
    std::string url;
    int parentId;
};

// implementation of the V8 handler class for the "taskmanager" extension
class TaskManager : public CefV8Handler
{
public:
    TaskManager() {}
    virtual ~TaskManager() {}

    // return true if the method was handled
    virtual bool Execute(const CefString &name, CefRefPtr<CefV8Value> object, const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception) override;

    static void init();

private:
    // Helper methods
    std::vector<ProcessInfo> getProcessList();
    bool endProcess(int processId);
    CefRefPtr<CefV8Value> processInfoToV8Object(const ProcessInfo& process);
    
    IMPLEMENT_REFCOUNTING(TaskManager);
};

} // namespace ipc
} // namespace v8
} // namespace app