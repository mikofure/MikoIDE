#pragma once

#include "GitControl.hpp"
#include <string>
#include <memory>
#include <functional>
#include <map>
#include <any>

namespace miko {
namespace gitcontrol {

// IPC message structure
struct GitIPCMessage {
    std::string operation;
    std::map<std::string, std::any> params;
    std::string requestId;
};

// IPC response structure
struct GitIPCResponse {
    bool success = false;
    std::string error;
    std::map<std::string, std::any> data;
    std::string requestId;
};

// Response callback type
using ResponseCallback = std::function<void(const GitIPCResponse&)>;

class GitIPCHandler {
public:
    GitIPCHandler();
    ~GitIPCHandler();

    // Main IPC message handler
    void handleMessage(const GitIPCMessage& message, ResponseCallback callback);
    
    // Set working directory for Git operations
    void setWorkingDirectory(const std::string& path);
    std::string getWorkingDirectory() const;
    
    // Error and progress callbacks
    void setErrorCallback(std::function<void(const std::string&)> callback);
    void setProgressCallback(std::function<void(const std::string&, int, int)> callback);

private:
    std::unique_ptr<GitControl> gitControl;
    std::string workingDirectory;
    
    // Operation handlers
    GitIPCResponse handleInit(const std::map<std::string, std::any>& params);
    GitIPCResponse handleClone(const std::map<std::string, std::any>& params);
    GitIPCResponse handleAdd(const std::map<std::string, std::any>& params);
    GitIPCResponse handleRemove(const std::map<std::string, std::any>& params);
    GitIPCResponse handleCommit(const std::map<std::string, std::any>& params);
    GitIPCResponse handleStatus(const std::map<std::string, std::any>& params);
    GitIPCResponse handleFetch(const std::map<std::string, std::any>& params);
    GitIPCResponse handlePull(const std::map<std::string, std::any>& params);
    GitIPCResponse handlePush(const std::map<std::string, std::any>& params);
    GitIPCResponse handleLog(const std::map<std::string, std::any>& params);
    GitIPCResponse handleShow(const std::map<std::string, std::any>& params);
    GitIPCResponse handleDiff(const std::map<std::string, std::any>& params);
    GitIPCResponse handleReset(const std::map<std::string, std::any>& params);
    GitIPCResponse handleListBranches(const std::map<std::string, std::any>& params);
    GitIPCResponse handleCreateBranch(const std::map<std::string, std::any>& params);
    GitIPCResponse handleCheckout(const std::map<std::string, std::any>& params);
    GitIPCResponse handleDeleteBranch(const std::map<std::string, std::any>& params);
    GitIPCResponse handleMerge(const std::map<std::string, std::any>& params);
    GitIPCResponse handleRebase(const std::map<std::string, std::any>& params);
    GitIPCResponse handleCherryPick(const std::map<std::string, std::any>& params);
    GitIPCResponse handleRevert(const std::map<std::string, std::any>& params);
    GitIPCResponse handleCreateTag(const std::map<std::string, std::any>& params);
    GitIPCResponse handleListTags(const std::map<std::string, std::any>& params);
    GitIPCResponse handleDeleteTag(const std::map<std::string, std::any>& params);
    GitIPCResponse handleListRemotes(const std::map<std::string, std::any>& params);
    GitIPCResponse handleAddRemote(const std::map<std::string, std::any>& params);
    GitIPCResponse handleRemoveRemote(const std::map<std::string, std::any>& params);
    GitIPCResponse handleRenameRemote(const std::map<std::string, std::any>& params);
    GitIPCResponse handleSetRemoteUrl(const std::map<std::string, std::any>& params);
    GitIPCResponse handleStash(const std::map<std::string, std::any>& params);
    GitIPCResponse handleStashApply(const std::map<std::string, std::any>& params);
    GitIPCResponse handleStashPop(const std::map<std::string, std::any>& params);
    GitIPCResponse handleStashList(const std::map<std::string, std::any>& params);
    GitIPCResponse handleStashDrop(const std::map<std::string, std::any>& params);
    GitIPCResponse handleIsRepository(const std::map<std::string, std::any>& params);
    GitIPCResponse handleGetRepositoryInfo(const std::map<std::string, std::any>& params);
    GitIPCResponse handleClean(const std::map<std::string, std::any>& params);
    GitIPCResponse handleArchive(const std::map<std::string, std::any>& params);
    
    // Helper methods
    template<typename T>
    T getParam(const std::map<std::string, std::any>& params, const std::string& key, const T& defaultValue = T{}) const;
    
    std::vector<std::string> getStringArray(const std::map<std::string, std::any>& params, const std::string& key) const;
    GitCredentials getCredentials(const std::map<std::string, std::any>& params) const;
    GitCloneOptions getCloneOptions(const std::map<std::string, std::any>& params) const;
    GitCommitOptions getCommitOptions(const std::map<std::string, std::any>& params) const;
    GitDiffOptions getDiffOptions(const std::map<std::string, std::any>& params) const;
    GitLogOptions getLogOptions(const std::map<std::string, std::any>& params) const;
    
    void ensureRepository(const std::string& dir = "");
    GitIPCResponse createErrorResponse(const std::string& error) const;
    GitIPCResponse createSuccessResponse(const std::map<std::string, std::any>& data = {}) const;
    
    // Conversion helpers
    std::map<std::string, std::any> statusToMap(const GitStatusResult& status) const;
    std::map<std::string, std::any> branchInfoToMap(const GitBranchInfo& branch) const;
    std::map<std::string, std::any> remoteInfoToMap(const GitRemoteInfo& remote) const;
    std::map<std::string, std::any> commitInfoToMap(const GitCommitInfo& commit) const;
    std::map<std::string, std::any> tagInfoToMap(const GitTagInfo& tag) const;
    std::map<std::string, std::any> stashInfoToMap(const GitStashInfo& stash) const;
    std::map<std::string, std::any> mergeResultToMap(const GitMergeResult& result) const;
};

} // namespace gitcontrol
} // namespace miko