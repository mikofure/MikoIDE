#include "shared/gitcontrol/GitIPCHandler.hpp"
#include <iostream>
#include <vector>
#include <filesystem>

namespace miko {
namespace gitcontrol {

GitIPCHandler::GitIPCHandler() : gitControl(std::make_unique<GitControl>()) {
}

GitIPCHandler::~GitIPCHandler() = default;

void GitIPCHandler::handleMessage(const GitIPCMessage& message, ResponseCallback callback) {
    GitIPCResponse response;
    response.requestId = message.requestId;
    
    try {
        // Route to appropriate handler based on operation
        if (message.operation == "init") {
            response = handleInit(message.params);
        } else if (message.operation == "clone") {
            response = handleClone(message.params);
        } else if (message.operation == "add") {
            response = handleAdd(message.params);
        } else if (message.operation == "remove") {
            response = handleRemove(message.params);
        } else if (message.operation == "commit") {
            response = handleCommit(message.params);
        } else if (message.operation == "status") {
            response = handleStatus(message.params);
        } else if (message.operation == "fetch") {
            response = handleFetch(message.params);
        } else if (message.operation == "pull") {
            response = handlePull(message.params);
        } else if (message.operation == "push") {
            response = handlePush(message.params);
        } else if (message.operation == "log") {
            response = handleLog(message.params);
        } else if (message.operation == "show") {
            response = handleShow(message.params);
        } else if (message.operation == "diff") {
            response = handleDiff(message.params);
        } else if (message.operation == "reset") {
            response = handleReset(message.params);
        } else if (message.operation == "listBranches") {
            response = handleListBranches(message.params);
        } else if (message.operation == "createBranch") {
            response = handleCreateBranch(message.params);
        } else if (message.operation == "checkout") {
            response = handleCheckout(message.params);
        } else if (message.operation == "deleteBranch") {
            response = handleDeleteBranch(message.params);
        } else if (message.operation == "merge") {
            response = handleMerge(message.params);
        } else if (message.operation == "rebase") {
            response = handleRebase(message.params);
        } else if (message.operation == "cherryPick") {
            response = handleCherryPick(message.params);
        } else if (message.operation == "revert") {
            response = handleRevert(message.params);
        } else if (message.operation == "createTag") {
            response = handleCreateTag(message.params);
        } else if (message.operation == "listTags") {
            response = handleListTags(message.params);
        } else if (message.operation == "deleteTag") {
            response = handleDeleteTag(message.params);
        } else if (message.operation == "listRemotes") {
            response = handleListRemotes(message.params);
        } else if (message.operation == "addRemote") {
            response = handleAddRemote(message.params);
        } else if (message.operation == "removeRemote") {
            response = handleRemoveRemote(message.params);
        } else if (message.operation == "renameRemote") {
            response = handleRenameRemote(message.params);
        } else if (message.operation == "setRemoteUrl") {
            response = handleSetRemoteUrl(message.params);
        } else if (message.operation == "stash") {
            response = handleStash(message.params);
        } else if (message.operation == "stashApply") {
            response = handleStashApply(message.params);
        } else if (message.operation == "stashPop") {
            response = handleStashPop(message.params);
        } else if (message.operation == "stashList") {
            response = handleStashList(message.params);
        } else if (message.operation == "stashDrop") {
            response = handleStashDrop(message.params);
        } else if (message.operation == "isRepository") {
            response = handleIsRepository(message.params);
        } else if (message.operation == "getRepositoryInfo") {
            response = handleGetRepositoryInfo(message.params);
        } else if (message.operation == "clean") {
            response = handleClean(message.params);
        } else if (message.operation == "archive") {
            response = handleArchive(message.params);
        } else {
            response = createErrorResponse("Unknown operation: " + message.operation);
        }
    } catch (const std::exception& e) {
        response = createErrorResponse("Exception: " + std::string(e.what()));
    } catch (...) {
        response = createErrorResponse("Unknown exception occurred");
    }
    
    response.requestId = message.requestId;
    callback(response);
}

void GitIPCHandler::setWorkingDirectory(const std::string& path) {
    workingDirectory = path;
    if (gitControl) {
        gitControl->setWorkingDirectory(path);
    }
}

std::string GitIPCHandler::getWorkingDirectory() const {
    return workingDirectory;
}

void GitIPCHandler::setErrorCallback(std::function<void(const std::string&)> callback) {
    if (gitControl) {
        gitControl->setErrorCallback(callback);
    }
}

void GitIPCHandler::setProgressCallback(std::function<void(const std::string&, int, int)> callback) {
    if (gitControl) {
        gitControl->setProgressCallback(callback);
    }
}

// Operation handlers
GitIPCResponse GitIPCHandler::handleInit(const std::map<std::string, std::any>& params) {
    std::string dir = getParam<std::string>(params, "dir", workingDirectory);
    bool bare = getParam<bool>(params, "bare", false);
    std::string initialBranch = getParam<std::string>(params, "initialBranch", "main");
    
    if (gitControl->init(dir, bare, initialBranch)) {
        return createSuccessResponse({{"path", dir}});
    }
    return createErrorResponse(gitControl->getLastError());
}

GitIPCResponse GitIPCHandler::handleClone(const std::map<std::string, std::any>& params) {
    std::string url = getParam<std::string>(params, "url");
    std::string dir = getParam<std::string>(params, "dir", workingDirectory);
    GitCloneOptions options = getCloneOptions(params);
    
    if (gitControl->clone(url, dir, options)) {
        setWorkingDirectory(dir);
        return createSuccessResponse({{"path", dir}});
    }
    return createErrorResponse(gitControl->getLastError());
}

GitIPCResponse GitIPCHandler::handleAdd(const std::map<std::string, std::any>& params) {
    std::string dir = getParam<std::string>(params, "dir", workingDirectory);
    ensureRepository(dir);
    
    std::vector<std::string> files = getStringArray(params, "files");
    
    if (files.empty()) {
        if (gitControl->addAll()) {
            return createSuccessResponse();
        }
    } else {
        if (gitControl->add(files)) {
            return createSuccessResponse();
        }
    }
    return createErrorResponse(gitControl->getLastError());
}

GitIPCResponse GitIPCHandler::handleRemove(const std::map<std::string, std::any>& params) {
    std::string dir = getParam<std::string>(params, "dir", workingDirectory);
    ensureRepository(dir);
    
    std::vector<std::string> files = getStringArray(params, "files");
    bool keepInWorkdir = getParam<bool>(params, "keepInWorkdir", false);
    
    if (gitControl->remove(files, keepInWorkdir)) {
        return createSuccessResponse();
    }
    return createErrorResponse(gitControl->getLastError());
}

GitIPCResponse GitIPCHandler::handleCommit(const std::map<std::string, std::any>& params) {
    std::string dir = getParam<std::string>(params, "dir", workingDirectory);
    ensureRepository(dir);
    
    std::string message = getParam<std::string>(params, "message");
    GitCommitOptions options = getCommitOptions(params);
    
    if (gitControl->commit(message, options)) {
        return createSuccessResponse();
    }
    return createErrorResponse(gitControl->getLastError());
}

GitIPCResponse GitIPCHandler::handleStatus(const std::map<std::string, std::any>& params) {
    std::string dir = getParam<std::string>(params, "dir", workingDirectory);
    ensureRepository(dir);
    
    bool includeUntracked = getParam<bool>(params, "includeUntracked", true);
    bool includeIgnored = getParam<bool>(params, "includeIgnored", false);
    
    GitStatusResult status = gitControl->status(includeUntracked, includeIgnored);
    return createSuccessResponse(statusToMap(status));
}

GitIPCResponse GitIPCHandler::handleFetch(const std::map<std::string, std::any>& params) {
    std::string dir = getParam<std::string>(params, "dir", workingDirectory);
    ensureRepository(dir);
    
    std::string remote = getParam<std::string>(params, "remote", "origin");
    std::string ref = getParam<std::string>(params, "ref", "");
    GitCredentials credentials = getCredentials(params);
    
    if (gitControl->fetch(remote, ref, credentials)) {
        return createSuccessResponse();
    }
    return createErrorResponse(gitControl->getLastError());
}

GitIPCResponse GitIPCHandler::handleListBranches(const std::map<std::string, std::any>& params) {
    std::string dir = getParam<std::string>(params, "dir", workingDirectory);
    ensureRepository(dir);
    
    bool remote = getParam<bool>(params, "remote", false);
    bool all = getParam<bool>(params, "all", false);
    
    std::vector<GitBranchInfo> branches = gitControl->listBranches(remote || all);
    
    std::vector<std::map<std::string, std::any>> branchData;
    for (const auto& branch : branches) {
        branchData.push_back(branchInfoToMap(branch));
    }
    
    return createSuccessResponse({{"branches", branchData}});
}

GitIPCResponse GitIPCHandler::handleCreateBranch(const std::map<std::string, std::any>& params) {
    std::string dir = getParam<std::string>(params, "dir", workingDirectory);
    ensureRepository(dir);
    
    std::string name = getParam<std::string>(params, "name");
    std::string startPoint = getParam<std::string>(params, "startPoint", "");
    bool checkout = getParam<bool>(params, "checkout", false);
    
    if (gitControl->createBranch(name, startPoint, checkout)) {
        return createSuccessResponse();
    }
    return createErrorResponse(gitControl->getLastError());
}

GitIPCResponse GitIPCHandler::handleCheckout(const std::map<std::string, std::any>& params) {
    std::string dir = getParam<std::string>(params, "dir", workingDirectory);
    ensureRepository(dir);
    
    std::string ref = getParam<std::string>(params, "ref");
    bool force = getParam<bool>(params, "force", false);
    bool createBranch = getParam<bool>(params, "createBranch", false);
    
    if (gitControl->checkout(ref, force, createBranch)) {
        return createSuccessResponse();
    }
    return createErrorResponse(gitControl->getLastError());
}

GitIPCResponse GitIPCHandler::handleListRemotes(const std::map<std::string, std::any>& params) {
    std::string dir = getParam<std::string>(params, "dir", workingDirectory);
    ensureRepository(dir);
    
    std::vector<GitRemoteInfo> remotes = gitControl->listRemotes();
    
    std::vector<std::map<std::string, std::any>> remoteData;
    for (const auto& remote : remotes) {
        remoteData.push_back(remoteInfoToMap(remote));
    }
    
    return createSuccessResponse({{"remotes", remoteData}});
}

GitIPCResponse GitIPCHandler::handleAddRemote(const std::map<std::string, std::any>& params) {
    std::string dir = getParam<std::string>(params, "dir", workingDirectory);
    ensureRepository(dir);
    
    std::string name = getParam<std::string>(params, "name");
    std::string url = getParam<std::string>(params, "url");
    
    if (gitControl->addRemote(name, url)) {
        return createSuccessResponse();
    }
    return createErrorResponse(gitControl->getLastError());
}

GitIPCResponse GitIPCHandler::handleIsRepository(const std::map<std::string, std::any>& params) {
    std::string dir = getParam<std::string>(params, "dir", workingDirectory);
    
    bool isRepo = gitControl->isRepository(dir);
    return createSuccessResponse({{"isRepository", isRepo}});
}

// Stub implementations for remaining handlers
GitIPCResponse GitIPCHandler::handlePull(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Pull operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handlePush(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Push operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleLog(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Log operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleShow(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Show operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleDiff(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Diff operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleReset(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Reset operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleDeleteBranch(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Delete branch operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleMerge(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Merge operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleRebase(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Rebase operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleCherryPick(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Cherry-pick operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleRevert(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Revert operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleCreateTag(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Create tag operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleListTags(const std::map<std::string, std::any>& params) {
    return createErrorResponse("List tags operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleDeleteTag(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Delete tag operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleRemoveRemote(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Remove remote operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleRenameRemote(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Rename remote operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleSetRemoteUrl(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Set remote URL operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleStash(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Stash operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleStashApply(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Stash apply operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleStashPop(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Stash pop operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleStashList(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Stash list operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleStashDrop(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Stash drop operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleGetRepositoryInfo(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Get repository info operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleClean(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Clean operation not yet implemented");
}

GitIPCResponse GitIPCHandler::handleArchive(const std::map<std::string, std::any>& params) {
    return createErrorResponse("Archive operation not yet implemented");
}

// Helper methods
template<typename T>
T GitIPCHandler::getParam(const std::map<std::string, std::any>& params, const std::string& key, const T& defaultValue) const {
    auto it = params.find(key);
    if (it != params.end()) {
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast&) {
            // Return default value if cast fails
        }
    }
    return defaultValue;
}

std::vector<std::string> GitIPCHandler::getStringArray(const std::map<std::string, std::any>& params, const std::string& key) const {
    auto it = params.find(key);
    if (it != params.end()) {
        try {
            return std::any_cast<std::vector<std::string>>(it->second);
        } catch (const std::bad_any_cast&) {
            // Return empty vector if cast fails
        }
    }
    return {};
}

GitCredentials GitIPCHandler::getCredentials(const std::map<std::string, std::any>& params) const {
    GitCredentials creds;
    
    auto credIt = params.find("credentials");
    if (credIt != params.end()) {
        try {
            auto credMap = std::any_cast<std::map<std::string, std::any>>(credIt->second);
            creds.username = getParam<std::string>(credMap, "username");
            creds.password = getParam<std::string>(credMap, "password");
            creds.privateKey = getParam<std::string>(credMap, "privateKey");
            creds.publicKey = getParam<std::string>(credMap, "publicKey");
            creds.passphrase = getParam<std::string>(credMap, "passphrase");
        } catch (const std::bad_any_cast&) {
            // Use empty credentials if cast fails
        }
    }
    
    return creds;
}

GitCloneOptions GitIPCHandler::getCloneOptions(const std::map<std::string, std::any>& params) const {
    GitCloneOptions options;
    options.branch = getParam<std::string>(params, "branch");
    options.depth = getParam<int>(params, "depth", 0);
    options.credentials = getCredentials(params);
    options.bare = getParam<bool>(params, "bare", false);
    options.checkout = getParam<bool>(params, "checkout", true);
    return options;
}

GitCommitOptions GitIPCHandler::getCommitOptions(const std::map<std::string, std::any>& params) const {
    GitCommitOptions options;
    
    auto authorIt = params.find("author");
    if (authorIt != params.end()) {
        try {
            auto authorMap = std::any_cast<std::map<std::string, std::any>>(authorIt->second);
            options.authorName = getParam<std::string>(authorMap, "name");
            options.authorEmail = getParam<std::string>(authorMap, "email");
        } catch (const std::bad_any_cast&) {
            // Use empty author if cast fails
        }
    }
    
    options.amend = getParam<bool>(params, "amend", false);
    options.allowEmpty = getParam<bool>(params, "allowEmpty", false);
    return options;
}

GitDiffOptions GitIPCHandler::getDiffOptions(const std::map<std::string, std::any>& params) const {
    GitDiffOptions options;
    options.fromCommit = getParam<std::string>(params, "from");
    options.toCommit = getParam<std::string>(params, "to");
    options.cached = getParam<bool>(params, "cached", false);
    options.nameOnly = getParam<bool>(params, "nameOnly", false);
    options.contextLines = getParam<int>(params, "unified", 3);
    return options;
}

GitLogOptions GitIPCHandler::getLogOptions(const std::map<std::string, std::any>& params) const {
    GitLogOptions options;
    options.ref = getParam<std::string>(params, "ref");
    options.maxCount = getParam<int>(params, "maxCount", 0);
    options.skip = getParam<int>(params, "skip", 0);
    options.since = getParam<std::string>(params, "since");
    options.until = getParam<std::string>(params, "until");
    options.author = getParam<std::string>(params, "author");
    options.grep = getParam<std::string>(params, "grep");
    return options;
}

void GitIPCHandler::ensureRepository(const std::string& dir) {
    std::string repoDir = dir.empty() ? workingDirectory : dir;
    if (!repoDir.empty() && !gitControl->isValidRepository()) {
        gitControl->open(repoDir);
    }
}

GitIPCResponse GitIPCHandler::createErrorResponse(const std::string& error) const {
    GitIPCResponse response;
    response.success = false;
    response.error = error;
    return response;
}

GitIPCResponse GitIPCHandler::createSuccessResponse(const std::map<std::string, std::any>& data) const {
    GitIPCResponse response;
    response.success = true;
    response.data = data;
    return response;
}

// Conversion helpers
std::map<std::string, std::any> GitIPCHandler::statusToMap(const GitStatusResult& status) const {
    std::map<std::string, std::any> result;
    result["modified"] = status.modified;
    result["added"] = status.added;
    result["deleted"] = status.deleted;
    result["untracked"] = status.untracked;
    result["ignored"] = status.ignored;
    result["conflicted"] = status.conflicted;
    return result;
}

std::map<std::string, std::any> GitIPCHandler::branchInfoToMap(const GitBranchInfo& branch) const {
    std::map<std::string, std::any> result;
    result["name"] = branch.name;
    result["current"] = branch.current;
    result["commit"] = branch.commit;
    result["remote"] = branch.remote;
    return result;
}

std::map<std::string, std::any> GitIPCHandler::remoteInfoToMap(const GitRemoteInfo& remote) const {
    std::map<std::string, std::any> result;
    result["name"] = remote.name;
    result["url"] = remote.url;
    result["pushUrl"] = remote.pushUrl;
    return result;
}

std::map<std::string, std::any> GitIPCHandler::commitInfoToMap(const GitCommitInfo& commit) const {
    std::map<std::string, std::any> result;
    result["oid"] = commit.oid;
    result["message"] = commit.message;
    result["authorName"] = commit.authorName;
    result["authorEmail"] = commit.authorEmail;
    result["committerName"] = commit.committerName;
    result["committerEmail"] = commit.committerEmail;
    result["timestamp"] = commit.timestamp;
    result["parents"] = commit.parents;
    return result;
}

std::map<std::string, std::any> GitIPCHandler::tagInfoToMap(const GitTagInfo& tag) const {
    std::map<std::string, std::any> result;
    result["name"] = tag.name;
    result["oid"] = tag.oid;
    result["message"] = tag.message;
    result["taggerName"] = tag.taggerName;
    result["taggerEmail"] = tag.taggerEmail;
    result["timestamp"] = tag.timestamp;
    return result;
}

std::map<std::string, std::any> GitIPCHandler::stashInfoToMap(const GitStashInfo& stash) const {
    std::map<std::string, std::any> result;
    result["index"] = static_cast<int>(stash.index);
    result["message"] = stash.message;
    result["oid"] = stash.oid;
    return result;
}

std::map<std::string, std::any> GitIPCHandler::mergeResultToMap(const GitMergeResult& mergeResult) const {
    std::map<std::string, std::any> result;
    result["success"] = mergeResult.success;
    result["conflicts"] = mergeResult.conflicts;
    result["message"] = mergeResult.message;
    return result;
}

} // namespace gitcontrol
} // namespace miko