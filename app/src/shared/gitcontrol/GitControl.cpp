#include "shared/gitcontrol/GitControl.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <filesystem>

namespace miko {
namespace gitcontrol {

// Private implementation class
class GitControl::Impl {
public:
    git_repository* repo = nullptr;
    std::string lastError;
    ErrorCallback errorCallback;
    ProgressCallback progressCallback;
    GitCredentials currentCredentials;
    
    Impl() {
        git_libgit2_init();
    }
    
    ~Impl() {
        if (repo) {
            git_repository_free(repo);
        }
        git_libgit2_shutdown();
    }
};

GitControl::GitControl() : pImpl(std::make_unique<Impl>()) {}

GitControl::~GitControl() = default;

// Repository operations
bool GitControl::init(const std::string& path, bool bare, const std::string& initialBranch) {
    git_repository* repo = nullptr;
    git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;
    
    opts.flags = GIT_REPOSITORY_INIT_MKPATH;
    if (bare) {
        opts.flags |= GIT_REPOSITORY_INIT_BARE;
    }
    
    if (!initialBranch.empty()) {
        opts.initial_head = initialBranch.c_str();
    }
    
    int error = git_repository_init_ext(&repo, path.c_str(), &opts);
    if (error != 0) {
        setLastError("Failed to initialize repository: " + std::string(git_error_last()->message));
        return false;
    }
    
    if (pImpl->repo) {
        git_repository_free(pImpl->repo);
    }
    pImpl->repo = repo;
    
    return true;
}

bool GitControl::clone(const std::string& url, const std::string& path, const GitCloneOptions& options) {
    git_clone_options cloneOpts = GIT_CLONE_OPTIONS_INIT;
    git_checkout_options checkoutOpts = GIT_CHECKOUT_OPTIONS_INIT;
    
    // Set up checkout options
    checkoutOpts.checkout_strategy = options.checkout ? GIT_CHECKOUT_SAFE : GIT_CHECKOUT_NONE;
    cloneOpts.checkout_opts = checkoutOpts;
    
    // Set up branch
    if (!options.branch.empty()) {
        cloneOpts.checkout_branch = options.branch.c_str();
    }
    
    // Set up credentials
    pImpl->currentCredentials = options.credentials;
    cloneOpts.fetch_opts.callbacks.credentials = credentialsCallback;
    cloneOpts.fetch_opts.callbacks.payload = this;
    
    // Set up progress callback
    if (pImpl->progressCallback) {
        cloneOpts.fetch_opts.callbacks.sideband_progress = progressCallback;
        cloneOpts.fetch_opts.callbacks.transfer_progress = fetchProgressCallback;
    }
    
    // Set bare repository
    cloneOpts.bare = options.bare;
    
    git_repository* repo = nullptr;
    int error = git_clone(&repo, url.c_str(), path.c_str(), &cloneOpts);
    
    if (error != 0) {
        setLastError("Failed to clone repository: " + std::string(git_error_last()->message));
        return false;
    }
    
    if (pImpl->repo) {
        git_repository_free(pImpl->repo);
    }
    pImpl->repo = repo;
    
    return true;
}

bool GitControl::open(const std::string& path) {
    git_repository* repo = nullptr;
    int error = git_repository_open(&repo, path.c_str());
    
    if (error != 0) {
        setLastError("Failed to open repository: " + std::string(git_error_last()->message));
        return false;
    }
    
    if (pImpl->repo) {
        git_repository_free(pImpl->repo);
    }
    pImpl->repo = repo;
    
    return true;
}

void GitControl::close() {
    if (pImpl->repo) {
        git_repository_free(pImpl->repo);
        pImpl->repo = nullptr;
    }
}

bool GitControl::isRepository(const std::string& path) {
    git_repository* repo = nullptr;
    int error = git_repository_open(&repo, path.c_str());
    
    if (error == 0 && repo) {
        git_repository_free(repo);
        return true;
    }
    
    return false;
}

std::string GitControl::getRepositoryPath() const {
    if (!pImpl->repo) {
        return "";
    }
    
    const char* path = git_repository_path(pImpl->repo);
    return path ? std::string(path) : "";
}

std::string GitControl::getWorkingDirectory() const {
    if (!pImpl->repo) {
        return "";
    }
    
    const char* workdir = git_repository_workdir(pImpl->repo);
    return workdir ? std::string(workdir) : "";
}

bool GitControl::setWorkingDirectory(const std::string& path) {
    if (!pImpl->repo) {
        setLastError("No repository opened");
        return false;
    }
    
    int error = git_repository_set_workdir(pImpl->repo, path.c_str(), 1);
    if (error != 0) {
        setLastError("Failed to set working directory: " + std::string(git_error_last()->message));
        return false;
    }
    
    return true;
}

// File operations
bool GitControl::add(const std::vector<std::string>& files) {
    if (!pImpl->repo) {
        setLastError("No repository opened");
        return false;
    }
    
    git_index* index = nullptr;
    int error = git_repository_index(&index, pImpl->repo);
    if (error != 0) {
        setLastError("Failed to get repository index: " + std::string(git_error_last()->message));
        return false;
    }
    
    for (const auto& file : files) {
        error = git_index_add_bypath(index, file.c_str());
        if (error != 0) {
            git_index_free(index);
            setLastError("Failed to add file " + file + ": " + std::string(git_error_last()->message));
            return false;
        }
    }
    
    error = git_index_write(index);
    git_index_free(index);
    
    if (error != 0) {
        setLastError("Failed to write index: " + std::string(git_error_last()->message));
        return false;
    }
    
    return true;
}

bool GitControl::addAll() {
    if (!pImpl->repo) {
        setLastError("No repository opened");
        return false;
    }
    
    git_index* index = nullptr;
    int error = git_repository_index(&index, pImpl->repo);
    if (error != 0) {
        setLastError("Failed to get repository index: " + std::string(git_error_last()->message));
        return false;
    }
    
    git_strarray pathspec = {nullptr, 0};
    error = git_index_add_all(index, &pathspec, GIT_INDEX_ADD_DEFAULT, nullptr, nullptr);
    
    if (error == 0) {
        error = git_index_write(index);
    }
    
    git_index_free(index);
    
    if (error != 0) {
        setLastError("Failed to add all files: " + std::string(git_error_last()->message));
        return false;
    }
    
    return true;
}

bool GitControl::remove(const std::vector<std::string>& files, bool keepInWorkdir) {
    if (!pImpl->repo) {
        setLastError("No repository opened");
        return false;
    }
    
    git_index* index = nullptr;
    int error = git_repository_index(&index, pImpl->repo);
    if (error != 0) {
        setLastError("Failed to get repository index: " + std::string(git_error_last()->message));
        return false;
    }
    
    for (const auto& file : files) {
        if (keepInWorkdir) {
            error = git_index_remove_bypath(index, file.c_str());
        } else {
            error = git_index_remove_bypath(index, file.c_str());
            if (error == 0) {
                std::filesystem::remove(getWorkingDirectory() + "/" + file);
            }
        }
        
        if (error != 0) {
            git_index_free(index);
            setLastError("Failed to remove file " + file + ": " + std::string(git_error_last()->message));
            return false;
        }
    }
    
    error = git_index_write(index);
    git_index_free(index);
    
    if (error != 0) {
        setLastError("Failed to write index: " + std::string(git_error_last()->message));
        return false;
    }
    
    return true;
}

// Commit operations
bool GitControl::commit(const std::string& message, const GitCommitOptions& options) {
    if (!pImpl->repo) {
        setLastError("No repository opened");
        return false;
    }
    
    git_signature* signature = nullptr;
    git_tree* tree = nullptr;
    git_index* index = nullptr;
    git_oid tree_id, commit_id;
    git_commit* parent = nullptr;
    
    // Create signature
    if (!options.authorName.empty() && !options.authorEmail.empty()) {
        signature = createSignature(options.authorName, options.authorEmail);
    } else {
        int error = git_signature_default(&signature, pImpl->repo);
        if (error != 0) {
            setLastError("Failed to create signature: " + std::string(git_error_last()->message));
            return false;
        }
    }
    
    // Get the index and write tree
    int error = git_repository_index(&index, pImpl->repo);
    if (error != 0) {
        freeSignature(signature);
        setLastError("Failed to get repository index: " + std::string(git_error_last()->message));
        return false;
    }
    
    error = git_index_write_tree(&tree_id, index);
    git_index_free(index);
    
    if (error != 0) {
        freeSignature(signature);
        setLastError("Failed to write tree: " + std::string(git_error_last()->message));
        return false;
    }
    
    error = git_tree_lookup(&tree, pImpl->repo, &tree_id);
    if (error != 0) {
        freeSignature(signature);
        setLastError("Failed to lookup tree: " + std::string(git_error_last()->message));
        return false;
    }
    
    // Get parent commit (if not initial commit)
    git_reference* head = nullptr;
    error = git_repository_head(&head, pImpl->repo);
    
    const git_commit* parents[1] = {nullptr};
    size_t parent_count = 0;
    
    if (error == 0) {
        const git_oid* target_oid = git_reference_target(head);
        if (target_oid) {
            error = git_commit_lookup(&parent, pImpl->repo, target_oid);
            if (error == 0) {
                parents[0] = parent;
                parent_count = 1;
            }
        }
        git_reference_free(head);
    }
    
    // Create commit
    error = git_commit_create_v(
        &commit_id,
        pImpl->repo,
        "HEAD",
        signature,
        signature,
        nullptr,
        message.c_str(),
        tree,
        parent_count,
        parents[0]
    );
    
    // Cleanup
    git_tree_free(tree);
    if (parent) {
        git_commit_free(parent);
    }
    freeSignature(signature);
    
    if (error != 0) {
        setLastError("Failed to create commit: " + std::string(git_error_last()->message));
        return false;
    }
    
    return true;
}

// Status operations
GitStatusResult GitControl::status(bool includeUntracked, bool includeIgnored) {
    GitStatusResult result;
    
    if (!pImpl->repo) {
        setLastError("No repository opened");
        return result;
    }
    
    git_status_list* status_list = nullptr;
    git_status_options opts = GIT_STATUS_OPTIONS_INIT;
    
    opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED;
    if (includeIgnored) {
        opts.flags |= GIT_STATUS_OPT_INCLUDE_IGNORED;
    }
    
    int error = git_status_list_new(&status_list, pImpl->repo, &opts);
    if (error != 0) {
        setLastError("Failed to get status: " + std::string(git_error_last()->message));
        return result;
    }
    
    size_t count = git_status_list_entrycount(status_list);
    for (size_t i = 0; i < count; ++i) {
        const git_status_entry* entry = git_status_byindex(status_list, i);
        if (!entry) continue;
        
        const char* path = entry->head_to_index ? entry->head_to_index->new_file.path :
                          entry->index_to_workdir ? entry->index_to_workdir->new_file.path : nullptr;
        
        if (!path) continue;
        
        std::string filepath(path);
        
        if (entry->status & GIT_STATUS_WT_MODIFIED || entry->status & GIT_STATUS_INDEX_MODIFIED) {
            result.modified.push_back(filepath);
        }
        if (entry->status & GIT_STATUS_WT_NEW || entry->status & GIT_STATUS_INDEX_NEW) {
            if (entry->status & GIT_STATUS_INDEX_NEW) {
                result.added.push_back(filepath);
            } else {
                result.untracked.push_back(filepath);
            }
        }
        if (entry->status & GIT_STATUS_WT_DELETED || entry->status & GIT_STATUS_INDEX_DELETED) {
            result.deleted.push_back(filepath);
        }
        if (entry->status & GIT_STATUS_IGNORED) {
            result.ignored.push_back(filepath);
        }
        if (entry->status & GIT_STATUS_CONFLICTED) {
            result.conflicted.push_back(filepath);
        }
    }
    
    git_status_list_free(status_list);
    return result;
}

// Branch operations
std::vector<GitBranchInfo> GitControl::listBranches(bool includeRemote) {
    std::vector<GitBranchInfo> branches;
    
    if (!pImpl->repo) {
        setLastError("No repository opened");
        return branches;
    }
    
    git_branch_iterator* iter = nullptr;
    git_branch_t type = includeRemote ? GIT_BRANCH_ALL : GIT_BRANCH_LOCAL;
    
    int error = git_branch_iterator_new(&iter, pImpl->repo, type);
    if (error != 0) {
        setLastError("Failed to create branch iterator: " + std::string(git_error_last()->message));
        return branches;
    }
    
    git_reference* ref = nullptr;
    git_branch_t branch_type;
    
    while (git_branch_next(&ref, &branch_type, iter) == 0) {
        GitBranchInfo info;
        
        const char* branch_name = nullptr;
        git_branch_name(&branch_name, ref);
        if (branch_name) {
            info.name = std::string(branch_name);
        }
        
        info.remote = (branch_type == GIT_BRANCH_REMOTE);
        info.current = git_branch_is_head(ref) == 1;
        
        const git_oid* oid = git_reference_target(ref);
        if (oid) {
            info.commit = oidToString(oid);
        }
        
        branches.push_back(info);
        git_reference_free(ref);
    }
    
    git_branch_iterator_free(iter);
    return branches;
}

bool GitControl::createBranch(const std::string& name, const std::string& startPoint, bool checkout) {
    if (!pImpl->repo) {
        setLastError("No repository opened");
        return false;
    }
    
    git_reference* branch_ref = nullptr;
    git_commit* target_commit = nullptr;
    
    // Get target commit
    if (startPoint.empty()) {
        git_reference* head = nullptr;
        int error = git_repository_head(&head, pImpl->repo);
        if (error != 0) {
            setLastError("Failed to get HEAD: " + std::string(git_error_last()->message));
            return false;
        }
        
        const git_oid* oid = git_reference_target(head);
        error = git_commit_lookup(&target_commit, pImpl->repo, oid);
        git_reference_free(head);
        
        if (error != 0) {
            setLastError("Failed to lookup commit: " + std::string(git_error_last()->message));
            return false;
        }
    } else {
        git_oid oid;
        int error = git_oid_fromstr(&oid, startPoint.c_str());
        if (error != 0) {
            // Try to resolve as reference
            git_reference* ref = nullptr;
            error = git_reference_lookup(&ref, pImpl->repo, startPoint.c_str());
            if (error == 0) {
                const git_oid* ref_oid = git_reference_target(ref);
                if (ref_oid) {
                    error = git_commit_lookup(&target_commit, pImpl->repo, ref_oid);
                }
                git_reference_free(ref);
            }
        } else {
            error = git_commit_lookup(&target_commit, pImpl->repo, &oid);
        }
        
        if (error != 0) {
            setLastError("Failed to resolve start point: " + std::string(git_error_last()->message));
            return false;
        }
    }
    
    // Create branch
    int error = git_branch_create(&branch_ref, pImpl->repo, name.c_str(), target_commit, 0);
    git_commit_free(target_commit);
    
    if (error != 0) {
        setLastError("Failed to create branch: " + std::string(git_error_last()->message));
        return false;
    }
    
    if (checkout) {
        git_reference_free(branch_ref);
        return this->checkout(name);
    }
    
    git_reference_free(branch_ref);
    return true;
}

bool GitControl::checkout(const std::string& ref, bool force, bool createBranch) {
    if (!pImpl->repo) {
        setLastError("No repository opened");
        return false;
    }
    
    git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
    opts.checkout_strategy = force ? GIT_CHECKOUT_FORCE : GIT_CHECKOUT_SAFE;
    
    git_reference* target_ref = nullptr;
    git_object* target_obj = nullptr;
    
    // Try to lookup reference first
    int error = git_reference_lookup(&target_ref, pImpl->repo, ref.c_str());
    if (error != 0) {
        // Try as branch name
        std::string branch_name = "refs/heads/" + ref;
        error = git_reference_lookup(&target_ref, pImpl->repo, branch_name.c_str());
        
        if (error != 0 && createBranch) {
            // Create new branch
            if (this->createBranch(ref, "", false)) {
                error = git_reference_lookup(&target_ref, pImpl->repo, branch_name.c_str());
            }
        }
    }
    
    if (error == 0) {
        error = git_reference_peel(&target_obj, target_ref, GIT_OBJECT_COMMIT);
        if (error == 0) {
            error = git_checkout_tree(pImpl->repo, target_obj, &opts);
            if (error == 0) {
                error = git_repository_set_head(pImpl->repo, git_reference_name(target_ref));
            }
        }
    }
    
    if (target_ref) git_reference_free(target_ref);
    if (target_obj) git_object_free(target_obj);
    
    if (error != 0) {
        setLastError("Failed to checkout: " + std::string(git_error_last()->message));
        return false;
    }
    
    return true;
}

std::string GitControl::getCurrentBranch() {
    if (!pImpl->repo) {
        return "";
    }
    
    git_reference* head = nullptr;
    int error = git_repository_head(&head, pImpl->repo);
    if (error != 0) {
        return "";
    }
    
    const char* branch_name = nullptr;
    error = git_branch_name(&branch_name, head);
    git_reference_free(head);
    
    return (error == 0 && branch_name) ? std::string(branch_name) : "";
}

// Remote operations
std::vector<GitRemoteInfo> GitControl::listRemotes() {
    std::vector<GitRemoteInfo> remotes;
    
    if (!pImpl->repo) {
        setLastError("No repository opened");
        return remotes;
    }
    
    git_strarray remote_names = {nullptr, 0};
    int error = git_remote_list(&remote_names, pImpl->repo);
    if (error != 0) {
        setLastError("Failed to list remotes: " + std::string(git_error_last()->message));
        return remotes;
    }
    
    for (size_t i = 0; i < remote_names.count; ++i) {
        git_remote* remote = nullptr;
        error = git_remote_lookup(&remote, pImpl->repo, remote_names.strings[i]);
        if (error == 0) {
            GitRemoteInfo info;
            info.name = std::string(remote_names.strings[i]);
            
            const char* url = git_remote_url(remote);
            if (url) {
                info.url = std::string(url);
            }
            
            const char* push_url = git_remote_pushurl(remote);
            if (push_url) {
                info.pushUrl = std::string(push_url);
            } else {
                info.pushUrl = info.url;
            }
            
            remotes.push_back(info);
            git_remote_free(remote);
        }
    }
    
    git_strarray_dispose(&remote_names);
    return remotes;
}

bool GitControl::addRemote(const std::string& name, const std::string& url) {
    if (!pImpl->repo) {
        setLastError("No repository opened");
        return false;
    }
    
    git_remote* remote = nullptr;
    int error = git_remote_create(&remote, pImpl->repo, name.c_str(), url.c_str());
    
    if (remote) {
        git_remote_free(remote);
    }
    
    if (error != 0) {
        setLastError("Failed to add remote: " + std::string(git_error_last()->message));
        return false;
    }
    
    return true;
}

// Fetch/Pull/Push operations
bool GitControl::fetch(const std::string& remote, const std::string& refspec, const GitCredentials& credentials) {
    if (!pImpl->repo) {
        setLastError("No repository opened");
        return false;
    }
    
    git_remote* git_remote = nullptr;
    int error = git_remote_lookup(&git_remote, pImpl->repo, remote.c_str());
    if (error != 0) {
        setLastError("Failed to lookup remote: " + std::string(git_error_last()->message));
        return false;
    }
    
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
    pImpl->currentCredentials = credentials;
    fetch_opts.callbacks.credentials = credentialsCallback;
    fetch_opts.callbacks.payload = this;
    
    if (pImpl->progressCallback) {
        fetch_opts.callbacks.sideband_progress = progressCallback;
        fetch_opts.callbacks.transfer_progress = fetchProgressCallback;
    }
    
    git_strarray refspecs = {nullptr, 0};
    if (!refspec.empty()) {
        char* refspec_str = const_cast<char*>(refspec.c_str());
        refspecs.strings = &refspec_str;
        refspecs.count = 1;
    }
    
    error = git_remote_fetch(git_remote, refspecs.count > 0 ? &refspecs : nullptr, &fetch_opts, nullptr);
    git_remote_free(git_remote);
    
    if (error != 0) {
        setLastError("Failed to fetch: " + std::string(git_error_last()->message));
        return false;
    }
    
    return true;
}

// Helper methods
bool GitControl::initializeLibgit2() {
    return git_libgit2_init() >= 0;
}

void GitControl::shutdownLibgit2() {
    git_libgit2_shutdown();
}

git_signature* GitControl::createSignature(const std::string& name, const std::string& email) {
    git_signature* signature = nullptr;
    git_signature_now(&signature, name.c_str(), email.c_str());
    return signature;
}

void GitControl::freeSignature(git_signature* signature) {
    if (signature) {
        git_signature_free(signature);
    }
}

std::string GitControl::oidToString(const git_oid* oid) {
    if (!oid) return "";
    
    char oid_str[GIT_OID_HEXSZ + 1];
    git_oid_tostr(oid_str, sizeof(oid_str), oid);
    return std::string(oid_str);
}

bool GitControl::stringToOid(git_oid* oid, const std::string& str) {
    return git_oid_fromstr(oid, str.c_str()) == 0;
}

void GitControl::setLastError(const std::string& error) {
    pImpl->lastError = error;
    if (pImpl->errorCallback) {
        pImpl->errorCallback(error);
    }
}

std::string GitControl::getLastError() const {
    return pImpl->lastError;
}

void GitControl::setErrorCallback(ErrorCallback callback) {
    pImpl->errorCallback = callback;
}

void GitControl::setProgressCallback(ProgressCallback callback) {
    pImpl->progressCallback = callback;
}

bool GitControl::isValidRepository() const {
    return pImpl->repo != nullptr;
}

// Callback implementations
int GitControl::credentialsCallback(git_credential** out, const char* url, const char* username_from_url, unsigned int allowed_types, void* payload) {
    GitControl* self = static_cast<GitControl*>(payload);
    const GitCredentials& creds = self->pImpl->currentCredentials;
    
    if (allowed_types & GIT_CREDENTIAL_USERPASS_PLAINTEXT) {
        if (!creds.username.empty() && !creds.password.empty()) {
            return git_credential_userpass_plaintext_new(out, creds.username.c_str(), creds.password.c_str());
        }
    }
    
    if (allowed_types & GIT_CREDENTIAL_SSH_KEY) {
        if (!creds.publicKey.empty() && !creds.privateKey.empty()) {
            return git_credential_ssh_key_new(out, 
                creds.username.empty() ? username_from_url : creds.username.c_str(),
                creds.publicKey.c_str(),
                creds.privateKey.c_str(),
                creds.passphrase.empty() ? nullptr : creds.passphrase.c_str());
        }
    }
    
    return GIT_PASSTHROUGH;
}

int GitControl::progressCallback(const char* str, int len, void* payload) {
    GitControl* self = static_cast<GitControl*>(payload);
    if (self->pImpl->progressCallback) {
        std::string message(str, len);
        self->pImpl->progressCallback("fetch", 0, 0);
    }
    return 0;
}

int GitControl::fetchProgressCallback(const git_indexer_progress* stats, void* payload) {
    GitControl* self = static_cast<GitControl*>(payload);
    if (self->pImpl->progressCallback) {
        self->pImpl->progressCallback("fetch", stats->received_objects, stats->total_objects);
    }
    return 0;
}

int GitControl::pushProgressCallback(unsigned int current, unsigned int total, size_t bytes, void* payload) {
    GitControl* self = static_cast<GitControl*>(payload);
    if (self->pImpl->progressCallback) {
        self->pImpl->progressCallback("push", current, total);
    }
    return 0;
}

// Stub implementations for remaining methods
bool GitControl::reset(const std::vector<std::string>& files, const std::string& commit) {
    // TODO: Implement reset functionality
    setLastError("Reset not yet implemented");
    return false;
}

bool GitControl::resetHard(const std::string& commit) {
    // TODO: Implement hard reset
    setLastError("Hard reset not yet implemented");
    return false;
}

bool GitControl::resetSoft(const std::string& commit) {
    // TODO: Implement soft reset
    setLastError("Soft reset not yet implemented");
    return false;
}

bool GitControl::clean(bool dryRun, bool force, bool directories, bool ignored) {
    // TODO: Implement clean functionality
    setLastError("Clean not yet implemented");
    return false;
}

std::vector<GitCommitInfo> GitControl::log(const GitLogOptions& options) {
    // TODO: Implement log functionality
    return {};
}

std::optional<GitCommitInfo> GitControl::show(const std::string& commit) {
    // TODO: Implement show functionality
    return std::nullopt;
}

std::string GitControl::diff(const GitDiffOptions& options) {
    // TODO: Implement diff functionality
    return "";
}

bool GitControl::hasChanges() {
    auto status_result = status();
    return !status_result.modified.empty() || !status_result.added.empty() || 
           !status_result.deleted.empty() || !status_result.untracked.empty();
}

bool GitControl::hasUncommittedChanges() {
    auto status_result = status();
    return !status_result.modified.empty() || !status_result.added.empty() || !status_result.deleted.empty();
}

bool GitControl::deleteBranch(const std::string& name, bool force) {
    // TODO: Implement delete branch
    setLastError("Delete branch not yet implemented");
    return false;
}

bool GitControl::renameBranch(const std::string& oldName, const std::string& newName) {
    // TODO: Implement rename branch
    setLastError("Rename branch not yet implemented");
    return false;
}

bool GitControl::removeRemote(const std::string& name) {
    // TODO: Implement remove remote
    setLastError("Remove remote not yet implemented");
    return false;
}

bool GitControl::renameRemote(const std::string& oldName, const std::string& newName) {
    // TODO: Implement rename remote
    setLastError("Rename remote not yet implemented");
    return false;
}

bool GitControl::setRemoteUrl(const std::string& name, const std::string& url, bool push) {
    // TODO: Implement set remote URL
    setLastError("Set remote URL not yet implemented");
    return false;
}

bool GitControl::pull(const std::string& remote, const std::string& branch, bool rebase, const GitCredentials& credentials) {
    // TODO: Implement pull
    setLastError("Pull not yet implemented");
    return false;
}

bool GitControl::push(const std::string& remote, const std::string& refspec, bool force, bool setUpstream, const GitCredentials& credentials) {
    // TODO: Implement push
    setLastError("Push not yet implemented");
    return false;
}

GitMergeResult GitControl::merge(const std::string& branch, bool noFastForward, bool squash) {
    // TODO: Implement merge
    GitMergeResult result;
    result.success = false;
    result.message = "Merge not yet implemented";
    return result;
}

bool GitControl::abortMerge() {
    // TODO: Implement abort merge
    setLastError("Abort merge not yet implemented");
    return false;
}

bool GitControl::continueMerge() {
    // TODO: Implement continue merge
    setLastError("Continue merge not yet implemented");
    return false;
}

bool GitControl::rebase(const std::string& upstream, const std::string& onto, bool interactive) {
    // TODO: Implement rebase
    setLastError("Rebase not yet implemented");
    return false;
}

bool GitControl::abortRebase() {
    // TODO: Implement abort rebase
    setLastError("Abort rebase not yet implemented");
    return false;
}

bool GitControl::continueRebase() {
    // TODO: Implement continue rebase
    setLastError("Continue rebase not yet implemented");
    return false;
}

bool GitControl::cherryPick(const std::vector<std::string>& commits, bool noCommit) {
    // TODO: Implement cherry-pick
    setLastError("Cherry-pick not yet implemented");
    return false;
}

bool GitControl::revert(const std::vector<std::string>& commits, bool noCommit) {
    // TODO: Implement revert
    setLastError("Revert not yet implemented");
    return false;
}

std::vector<GitTagInfo> GitControl::listTags(const std::string& pattern) {
    // TODO: Implement list tags
    return {};
}

bool GitControl::createTag(const std::string& name, const std::string& commit, const std::string& message, bool force) {
    // TODO: Implement create tag
    setLastError("Create tag not yet implemented");
    return false;
}

bool GitControl::deleteTag(const std::string& name) {
    // TODO: Implement delete tag
    setLastError("Delete tag not yet implemented");
    return false;
}

bool GitControl::pushTags(const std::string& remote, const GitCredentials& credentials) {
    // TODO: Implement push tags
    setLastError("Push tags not yet implemented");
    return false;
}

bool GitControl::stash(const std::string& message, bool includeUntracked, bool keepIndex) {
    // TODO: Implement stash
    setLastError("Stash not yet implemented");
    return false;
}

std::vector<GitStashInfo> GitControl::stashList() {
    // TODO: Implement stash list
    return {};
}

bool GitControl::stashApply(size_t index, bool restoreIndex) {
    // TODO: Implement stash apply
    setLastError("Stash apply not yet implemented");
    return false;
}

bool GitControl::stashPop(size_t index, bool restoreIndex) {
    // TODO: Implement stash pop
    setLastError("Stash pop not yet implemented");
    return false;
}

bool GitControl::stashDrop(size_t index) {
    // TODO: Implement stash drop
    setLastError("Stash drop not yet implemented");
    return false;
}

bool GitControl::stashClear() {
    // TODO: Implement stash clear
    setLastError("Stash clear not yet implemented");
    return false;
}

bool GitControl::setAuthor(const std::string& name, const std::string& email, bool global) {
    // TODO: Implement set author
    setLastError("Set author not yet implemented");
    return false;
}

std::pair<std::string, std::string> GitControl::getAuthor(bool global) {
    // TODO: Implement get author
    return {"Unknown", "unknown@example.com"};
}

bool GitControl::setConfig(const std::string& key, const std::string& value, bool global) {
    // TODO: Implement set config
    setLastError("Set config not yet implemented");
    return false;
}

std::string GitControl::getConfig(const std::string& key, bool global) {
    // TODO: Implement get config
    return "";
}

bool GitControl::archive(const std::string& outputPath, const std::string& ref, const std::string& format, const std::string& prefix) {
    // TODO: Implement archive
    setLastError("Archive not yet implemented");
    return false;
}

std::string GitControl::getRepositoryInfo() {
    // TODO: Implement get repository info
    return "Repository info not yet implemented";
}

std::vector<std::string> GitControl::listSubmodules() {
    // TODO: Implement list submodules
    return {};
}

bool GitControl::addSubmodule(const std::string& url, const std::string& path) {
    // TODO: Implement add submodule
    setLastError("Add submodule not yet implemented");
    return false;
}

bool GitControl::updateSubmodules(bool init) {
    // TODO: Implement update submodules
    setLastError("Update submodules not yet implemented");
    return false;
}

bool GitControl::removeSubmodule(const std::string& path) {
    // TODO: Implement remove submodule
    setLastError("Remove submodule not yet implemented");
    return false;
}

} // namespace gitcontrol
} // namespace miko