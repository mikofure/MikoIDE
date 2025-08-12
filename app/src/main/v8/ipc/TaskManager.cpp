#include "main/v8/ipc/TaskManager.hpp"

#include <chrono>
#include <ctime>
#include <iostream>
#include <random>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#elif __linux__
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <fstream>
#include <sstream>
#elif __APPLE__
#include <sys/types.h>
#include <signal.h>
#include <libproc.h>
#endif

namespace app
{

namespace v8
{

namespace ipc
{

bool TaskManager::Execute(const CefString &name, CefRefPtr<CefV8Value> object, const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception)
{
    if (name == "getProcessList")
    {
        std::vector<ProcessInfo> processes = getProcessList();
        CefRefPtr<CefV8Value> array = CefV8Value::CreateArray(static_cast<int>(processes.size()));
        
        for (size_t i = 0; i < processes.size(); ++i)
        {
            array->SetValue(static_cast<int>(i), processInfoToV8Object(processes[i]));
        }
        
        retval = array;
        return true;
    }
    else if (name == "endProcess")
    {
        if (arguments.size() > 0 && arguments[0]->IsInt())
        {
            int processId = arguments[0]->GetIntValue();
            bool success = endProcess(processId);
            retval = CefV8Value::CreateBool(success);
            return true;
        }
        else
        {
            exception = "Invalid arguments for endProcess";
            return true;
        }
    }
    else if (name == "getSystemStats")
    {
        CefRefPtr<CefV8Value> stats = CefV8Value::CreateObject(nullptr, nullptr);
        
        // Mock system statistics
        stats->SetValue("totalCpuUsage", CefV8Value::CreateDouble(static_cast<double>(rand() % 100)), V8_PROPERTY_ATTRIBUTE_NONE);
        stats->SetValue("totalMemoryUsage", CefV8Value::CreateDouble(static_cast<double>(rand() % 8192 + 1024)), V8_PROPERTY_ATTRIBUTE_NONE);
        stats->SetValue("totalNetworkUsage", CefV8Value::CreateDouble(static_cast<double>(rand() % 1024)), V8_PROPERTY_ATTRIBUTE_NONE);
        
        retval = stats;
        return true;
    }

    return false;
}

std::vector<ProcessInfo> TaskManager::getProcessList()
{
    std::vector<ProcessInfo> processes;
    
    // Mock data for demonstration - in a real implementation, this would query the actual system
    std::vector<std::string> processNames = {
        "Toolchain Interpreter", "CMake Build System", "GCC Compiler", "Python Interpreter",
        "Node.js Runtime", "Git Version Control", "Package Manager", "Build Cache Service",
        "Dependency Resolver", "Code Formatter", "Linter Service", "Debug Adapter", "Language Server"
    };
    
    std::vector<std::string> processTypes = {
        "Extension", "Utility", "Utility", "Utility", "Utility", "Utility", "Utility", "Utility",
        "Utility", "Plugin", "Plugin", "Plugin", "Plugin"
    };
    
    std::vector<std::string> processStatuses = {
        "Running", "Running", "Running", "Running", "Running", "Running", "Running", "Running",
        "Running", "Running", "Running", "Suspended", "Running"
    };
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> cpuDist(0.0, 25.0);
    std::uniform_real_distribution<> memDist(10.0, 500.0);
    std::uniform_real_distribution<> netDist(0.0, 100.0);
    std::uniform_int_distribution<> pidDist(2000, 9999);
    
    for (size_t i = 0; i < processNames.size(); ++i)
    {
        ProcessInfo process;
        process.id = static_cast<int>(i + 1);
        process.name = processNames[i];
        process.type = processTypes[i];
        process.pid = pidDist(gen);
        process.cpuUsage = cpuDist(gen);
        process.memoryUsage = memDist(gen);
        process.networkUsage = netDist(gen);
        process.status = processStatuses[i];
        process.url = "";
        process.parentId = (i == 0) ? 0 : 1; // First process has no parent, others are children of first
        
        processes.push_back(process);
    }
    
    return processes;
}

bool TaskManager::endProcess(int processId)
{
    // Mock implementation - in a real scenario, this would terminate the actual process
    // For safety reasons, we'll just return true to simulate success
    return processId > 0;
}

CefRefPtr<CefV8Value> TaskManager::processInfoToV8Object(const ProcessInfo& process)
{
    CefRefPtr<CefV8Value> obj = CefV8Value::CreateObject(nullptr, nullptr);
    
    obj->SetValue("id", CefV8Value::CreateInt(process.id), V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("name", CefV8Value::CreateString(process.name), V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("type", CefV8Value::CreateString(process.type), V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("pid", CefV8Value::CreateInt(process.pid), V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("cpuUsage", CefV8Value::CreateDouble(process.cpuUsage), V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("memoryUsage", CefV8Value::CreateDouble(process.memoryUsage), V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("networkUsage", CefV8Value::CreateDouble(process.networkUsage), V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("status", CefV8Value::CreateString(process.status), V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("url", CefV8Value::CreateString(process.url), V8_PROPERTY_ATTRIBUTE_NONE);
    obj->SetValue("parentId", CefV8Value::CreateInt(process.parentId), V8_PROPERTY_ATTRIBUTE_NONE);
    
    return obj;
}

void TaskManager::init()
{
    // Register a V8 extension with JavaScript code that calls native methods
    std::string code =
        ""
        "var TaskManager;"
        "if (!TaskManager) {"
        "  TaskManager = {};"
        "}"
        ""
        "(function() {"
        "  TaskManager.getProcessList = function() {"
        "    native function getProcessList();"
        "    return getProcessList();"
        "  };"
        "  TaskManager.endProcess = function(processId) {"
        "    native function endProcess();"
        "    return endProcess(processId);"
        "  };"
        "  TaskManager.getSystemStats = function() {"
        "    native function getSystemStats();"
        "    return getSystemStats();"
        "  };"
        ""
        "})();";

    CefRegisterExtension("v8/taskmanager", code, new app::v8::ipc::TaskManager());
}

} // namespace ipc
} // namespace v8
} // namespace app