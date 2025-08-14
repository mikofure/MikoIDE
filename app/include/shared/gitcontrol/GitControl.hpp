#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <optional>
#include <git2.h>

namespace miko {
namespace gitcontrol {

// Git credentials structure
struct GitCredentials {
    std::string username;
    std::string password;
    std::string privateKey;
    std::string publicKey;
    std::string passphrase;
};

// Git clone options
struct GitCloneOptions {
    std::string branch;
    int depth = 0;
    GitCredentials credentials;
    bool bare = false;
    bool checkout = true;
};

// Git commit options
struct GitCommitOptions {
    std::string authorName;
    std::string authorEmail;
    bool amend = false;
    bool allowEmpty = false;
};

// Git status result
struct GitStatusResult {
    std::vector<std::string> modified;
    std::vector<std::string> added;
    std::vector<std::string> deleted;
    std::vector<std::string> untracked;
    std::vector<std::string> ignored;
    std::vector<std::string> conflicted;
};

// Git branch information
struct GitBranchInfo {
    std::string name;
    bool current = false;
    std::string commit;
    bool remote = false;
};

// Git remote information
struct GitRemoteInfo {
    std::string name;
    std::string url;
    std::string pushUrl;
};

// Git commit information
struct GitCommitInfo {
    std::string oid;
    std::string message;
    std::string authorName;
    std::string authorEmail;
    std::string committerName;
    std::string committerEmail;
    int64_t timestamp;
    std::vector<std::string> parents;
};

// Git tag information
struct GitTagInfo {
    std::string name;
    std::string oid;
    std::string message;
    std::string taggerName;
    std::string taggerEmail;
    int64_t timestamp;
};

// Git stash information
struct GitStashInfo {
    size_t index;
    std::string message;
    std::string oid;
};

// Git merge result
struct GitMergeResult {
    bool success = false;
    std::vector<std::string> conflicts;
    std::string message;
};

// Git diff options
struct GitDiffOptions {
    std::string fromCommit;
    std::string toCommit;
    bool cached = false;
    bool nameOnly = false;
    int contextLines = 3;
    std::vector<std::string> pathspec;
};

// Git log options
struct GitLogOptions {
    std::string ref;
    int maxCount = 0;
    int skip = 0;
    std::string since;
    std::string until;
    std::string author;
    std::string grep;
    std::vector<std::string> pathspec;
};

// Error callback type
using ErrorCallback = std::function<void(const std::string& error)>;

// Progress callback type
using ProgressCallback = std::function<void(const std::string& operation, int current, int total)>;

class GitControl {
public:
    GitControl();
    ~GitControl();

    // Repository operations
    bool init(const std::string& path, bool bare = false, const std::string& initialBranch = "main");
    bool clone(const std::string& url, const std::string& path, const GitCloneOptions& options = {});
    bool open(const std::string& path);
    void close();
    bool isRepository(const std::string& path);
    std::string getRepositoryPath() const;
    std::string getWorkingDirectory() const;
    bool setWorkingDirectory(const std::string& path);

    // File operations
    bool add(const std::vector<std::string>& files);
    bool addAll();
    bool remove(const std::vector<std::string>& files, bool keepInWorkdir = false);
    bool reset(const std::vector<std::string>& files = {}, const std::string& commit = "HEAD");
    bool resetHard(const std::string& commit = "HEAD");
    bool resetSoft(const std::string& commit = "HEAD");
    bool clean(bool dryRun = false, bool force = false, bool directories = false, bool ignored = false);

    // Commit operations
    bool commit(const std::string& message, const GitCommitOptions& options = {});
    std::vector<GitCommitInfo> log(const GitLogOptions& options = {});
    std::optional<GitCommitInfo> show(const std::string& commit);
    std::string diff(const GitDiffOptions& options = {});

    // Status operations
    GitStatusResult status(bool includeUntracked = true, bool includeIgnored = false);
    bool hasChanges();
    bool hasUncommittedChanges();

    // Branch operations
    std::vector<GitBranchInfo> listBranches(bool includeRemote = false);
    bool createBranch(const std::string& name, const std::string& startPoint = "", bool checkout = false);
    bool checkout(const std::string& ref, bool force = false, bool createBranch = false);
    bool deleteBranch(const std::string& name, bool force = false);
    std::string getCurrentBranch();
    bool renameBranch(const std::string& oldName, const std::string& newName);

    // Remote operations
    std::vector<GitRemoteInfo> listRemotes();
    bool addRemote(const std::string& name, const std::string& url);
    bool removeRemote(const std::string& name);
    bool renameRemote(const std::string& oldName, const std::string& newName);
    bool setRemoteUrl(const std::string& name, const std::string& url, bool push = false);

    // Fetch/Pull/Push operations
    bool fetch(const std::string& remote = "origin", const std::string& refspec = "", const GitCredentials& credentials = {});
    bool pull(const std::string& remote = "origin", const std::string& branch = "", bool rebase = false, const GitCredentials& credentials = {});
    bool push(const std::string& remote = "origin", const std::string& refspec = "", bool force = false, bool setUpstream = false, const GitCredentials& credentials = {});

    // Merge operations
    GitMergeResult merge(const std::string& branch, bool noFastForward = false, bool squash = false);
    bool abortMerge();
    bool continueMerge();

    // Rebase operations
    bool rebase(const std::string& upstream, const std::string& onto = "", bool interactive = false);
    bool abortRebase();
    bool continueRebase();

    // Cherry-pick operations
    bool cherryPick(const std::vector<std::string>& commits, bool noCommit = false);
    bool revert(const std::vector<std::string>& commits, bool noCommit = false);

    // Tag operations
    std::vector<GitTagInfo> listTags(const std::string& pattern = "");
    bool createTag(const std::string& name, const std::string& commit = "HEAD", const std::string& message = "", bool force = false);
    bool deleteTag(const std::string& name);
    bool pushTags(const std::string& remote = "origin", const GitCredentials& credentials = {});

    // Stash operations
    bool stash(const std::string& message = "", bool includeUntracked = false, bool keepIndex = false);
    std::vector<GitStashInfo> stashList();
    bool stashApply(size_t index = 0, bool restoreIndex = false);
    bool stashPop(size_t index = 0, bool restoreIndex = false);
    bool stashDrop(size_t index = 0);
    bool stashClear();

    // Configuration
    bool setAuthor(const std::string& name, const std::string& email, bool global = false);
    std::pair<std::string, std::string> getAuthor(bool global = false);
    bool setConfig(const std::string& key, const std::string& value, bool global = false);
    std::string getConfig(const std::string& key, bool global = false);

    // Archive operations
    bool archive(const std::string& outputPath, const std::string& ref = "HEAD", const std::string& format = "zip", const std::string& prefix = "");

    // Utility operations
    std::string getLastError() const;
    void setErrorCallback(ErrorCallback callback);
    void setProgressCallback(ProgressCallback callback);
    bool isValidRepository() const;
    std::string getRepositoryInfo();

    // Submodule operations
    std::vector<std::string> listSubmodules();
    bool addSubmodule(const std::string& url, const std::string& path);
    bool updateSubmodules(bool init = false);
    bool removeSubmodule(const std::string& path);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;

    // Helper methods
    bool initializeLibgit2();
    void shutdownLibgit2();
    git_signature* createSignature(const std::string& name, const std::string& email);
    void freeSignature(git_signature* signature);
    std::string oidToString(const git_oid* oid);
    bool stringToOid(git_oid* oid, const std::string& str);
    void setLastError(const std::string& error);
    static int credentialsCallback(git_credential** out, const char* url, const char* username_from_url, unsigned int allowed_types, void* payload);
    static int progressCallback(const char* str, int len, void* payload);
    static int fetchProgressCallback(const git_indexer_progress* stats, void* payload);
    static int pushProgressCallback(unsigned int current, unsigned int total, size_t bytes, void* payload);
};

} // namespace gitcontrol
} // namespace miko