import { createSignal, For, onMount, createEffect } from "solid-js";
// @ts-expect-error
import { GitBranch, GitCommit, GitPullRequest, Plus, Minus, RotateCcw, MoreHorizontal, RefreshCw, FolderOpen, Download } from "lucide-solid";
import { gitControl } from "../../core/gitcontrol/index";
import { gitOperations } from "../../core/git/operations";
import { commandPaletteHandler } from "../../core/cmdpalettehandler";

interface GitFile {
    name: string;
    status: 'modified' | 'added' | 'deleted' | 'untracked';
    path: string;
}

interface ContextMenuProps {
    x: number;
    y: number;
    visible: boolean;
    file?: GitFile;
    isStaged: boolean;
    onClose: () => void;
    onStage?: (file: GitFile) => void;
    onUnstage?: (file: GitFile) => void;
    onDiscard?: (file: GitFile) => void;
    onOpenFile?: (file: GitFile) => void;
}

function ContextMenu(props: ContextMenuProps) {
    return (
        <div
            class="git-context-menu"
            style={{
                display: props.visible ? 'block' : 'none',
                position: 'fixed',
                left: `${props.x}px`,
                top: `${props.y}px`,
                'z-index': '1001'
            }}
            onClick={(e) => e.stopPropagation()}
        >
            <div class="menu-dropdown">
                {props.isStaged ? (
                    <>
                        <div class="menu-item" onClick={() => { props.onUnstage?.(props.file!); props.onClose(); }}>
                            <div class="menu-item-content">
                                <Minus size={14} />
                                <span class="menu-item-label">Unstage Changes</span>
                            </div>
                        </div>
                        <div class="menu-separator"></div>
                        <div class="menu-item" onClick={() => { props.onOpenFile?.(props.file!); props.onClose(); }}>
                            <div class="menu-item-content">
                                <span class="menu-item-label">Open File</span>
                            </div>
                        </div>
                    </>
                ) : (
                    <>
                        <div class="menu-item" onClick={() => { props.onStage?.(props.file!); props.onClose(); }}>
                            <div class="menu-item-content">
                                <Plus size={14} />
                                <span class="menu-item-label">Stage Changes</span>
                            </div>
                        </div>
                        <div class="menu-item" onClick={() => { props.onDiscard?.(props.file!); props.onClose(); }}>
                            <div class="menu-item-content">
                                <RotateCcw size={14} />
                                <span class="menu-item-label">Discard Changes</span>
                            </div>
                        </div>
                        <div class="menu-separator"></div>
                        <div class="menu-item" onClick={() => { props.onOpenFile?.(props.file!); props.onClose(); }}>
                            <div class="menu-item-content">
                                <span class="menu-item-label">Open File</span>
                            </div>
                        </div>
                    </>
                )}
            </div>
        </div>
    );
}

function GitPage() {
    const [currentBranch, setCurrentBranch] = createSignal("main");
    const [commitMessage, setCommitMessage] = createSignal("");
    const [stagedFiles, setStagedFiles] = createSignal<GitFile[]>([]);
    const [unstagedFiles, setUnstagedFiles] = createSignal<GitFile[]>([]);
    const [isLoading, setIsLoading] = createSignal(false);
    const [isRepository, setIsRepository] = createSignal(false);
    const [contextMenu, setContextMenu] = createSignal({
        visible: false,
        x: 0,
        y: 0,
        file: undefined as GitFile | undefined,
        isStaged: false
    });
    // @ts-expect-error
    const [workingDirectory, setWorkingDirectory] = createSignal("");

    // Initialize Git status on mount
    onMount(async () => {
        await checkRepository();
        if (isRepository()) {
            await refreshGitStatus();
            await loadCurrentBranch();
        }
        
        // Listen for repository changes from centralized operations
        //@ts-expect-error
        gitOperations.addListener(async () => {
            await checkRepository();
            if (isRepository()) {
                await refreshGitStatus();
                await loadCurrentBranch();
            }
        });
    });

    // Close context menu when clicking outside
    createEffect(() => {
        const handleClickOutside = () => {
            setContextMenu(prev => ({ ...prev, visible: false }));
        };

        if (contextMenu().visible) {
            document.addEventListener('click', handleClickOutside);
            return () => document.removeEventListener('click', handleClickOutside);
        }
    });

    const getStatusIcon = (status: string) => {
        switch (status) {
            case 'modified': return <span class="text-orange-400 text-xs">M</span>;
            case 'added': return <span class="text-green-400 text-xs">A</span>;
            case 'deleted': return <span class="text-red-400 text-xs">D</span>;
            case 'untracked': return <span class="text-blue-400 text-xs">U</span>;
            default: return <span class="text-gray-400 text-xs">?</span>;
        }
    };

    const checkRepository = async () => {
        try {
            const isRepo = await gitControl.isRepository();
            setIsRepository(isRepo);
        } catch (error) {
            console.error('Failed to check repository:', error);
            setIsRepository(false);
        }
    };

    const refreshGitStatus = async () => {
        try {
            setIsLoading(true);

            // First check if we're in a repository
            await checkRepository();
            if (!isRepository()) {
                setStagedFiles([]);
                setUnstagedFiles([]);
                return;
            }

            const status = await gitControl.status();

            // Parse Git status and update file lists
            const staged: GitFile[] = [];
            const unstaged: GitFile[] = [];

            if (status && Array.isArray(status)) {
                status.forEach((item: any) => {
                    const file: GitFile = {
                        name: item.path.split('/').pop() || item.path,
                        path: item.path,
                        status: item.workdir || item.index || 'modified'
                    };

                    if (item.index) {
                        staged.push(file);
                    } else if (item.workdir) {
                        unstaged.push(file);
                    }
                });
            }

            setStagedFiles(staged);
            setUnstagedFiles(unstaged);
        } catch (error) {
            console.error('Failed to refresh Git status:', error);
            setIsRepository(false);
        } finally {
            setIsLoading(false);
        }
    };

    const loadCurrentBranch = async () => {
        try {
            const branches = await gitControl.listBranches();
            const current = branches.find((b: any) => b.current);
            if (current) {
                setCurrentBranch(current.name);
            }
        } catch (error) {
            console.error('Failed to load current branch:', error);
        }
    };

    const handleRightClick = (e: MouseEvent, file: GitFile, isStaged: boolean) => {
        e.preventDefault();
        e.stopPropagation();

        setContextMenu({
            visible: true,
            x: e.clientX,
            y: e.clientY,
            file,
            isStaged
        });
    };

    const stageFile = async (file: GitFile) => {
        try {
            await gitControl.add(file.path);
            await refreshGitStatus();
        } catch (error) {
            console.error('Failed to stage file:', error);
        }
    };

    const unstageFile = async (file: GitFile) => {
        try {
            await gitControl.reset(file.path);
            await refreshGitStatus();
        } catch (error) {
            console.error('Failed to unstage file:', error);
        }
    };

    const discardChanges = async (file: GitFile) => {
        try {
            // This would require a checkout operation to discard changes
            await gitControl.checkout(file.path);
            await refreshGitStatus();
        } catch (error) {
            console.error('Failed to discard changes:', error);
        }
    };

    const openFile = (file: GitFile) => {
        // This would integrate with the file explorer/editor
        console.log('Opening file:', file.path);
    };

    const handleCommit = async () => {
        if (!commitMessage().trim() || stagedFiles().length === 0) return;

        try {
            setIsLoading(true);
            await gitControl.commit(commitMessage());
            setCommitMessage("");
            await refreshGitStatus();
        } catch (error) {
            console.error('Failed to commit:', error);
        } finally {
            setIsLoading(false);
        }
    };

    const stageAllFiles = async () => {
        try {
            setIsLoading(true);
            await gitControl.add('.');
            await refreshGitStatus();
        } catch (error) {
            console.error('Failed to stage all files:', error);
        } finally {
            setIsLoading(false);
        }
    };

    const handleOpenFolder = async () => {
        try {
            setIsLoading(true);
            const result = await gitOperations.openFolder();
            
            if (result.success) {
                // Refresh Git status after opening folder
                await checkRepository();
                if (isRepository()) {
                    await refreshGitStatus();
                    await loadCurrentBranch();
                }
            } else {
                console.error(result.message);
            }
        } catch (error) {
            console.error('Failed to open folder:', error);
        } finally {
            setIsLoading(false);
        }
    };

    const handleCloneRepository = async () => {
        // Use the centralized command palette handler for clone operations
        const url = prompt('Enter repository URL to clone:');
        if (url) {
            try {
                // @ts-expect-error
                await commandPaletteHandler.handleClone(url);
                // Refresh Git status after successful clone
                await checkRepository();
                if (isRepository()) {
                    await refreshGitStatus();
                    await loadCurrentBranch();
                }
            } catch (error) {
                console.error('Failed to clone repository:', error);
            }
        }
    };

    return (
        <>
            <style>{gitStyles}</style>
            <div class="h-full flex flex-col">
                <div class="p-3 border-b border-neutral-800">
                    <div class="flex items-center justify-between mb-3">
                        <h3 class="text-xs font-medium text-gray-300 uppercase tracking-wide">Source Control</h3>
                        <button
                            onClick={refreshGitStatus}
                            disabled={isLoading()}
                            class="p-1 text-gray-400 hover:text-gray-300 disabled:opacity-50"
                            title="Refresh"
                        >
                            <RefreshCw size={14} class={isLoading() ? "animate-spin" : ""} />
                        </button>
                    </div>
                </div>

                {!isRepository() ? (
                    /* Welcome Interface - No Repository */
                    <div class="flex-1 flex items-center justify-center">
                        <div class="text-center space-y-4 p-4 max-w-sm">
                            <div class="text-sm text-gray-400 mb-6">
                                In order to use Git features, you can open a folder containing a Git repository or clone from a URL.
                            </div>

                            <button
                                onClick={handleOpenFolder}
                                disabled={isLoading()}
                                class="w-full bg-white hover:bg-white/50 text-black py-2 px-4 rounded-md transition-colors flex items-center justify-center space-x-2 text-xs"
                            >
                                <FolderOpen size={16} />
                                <span>Open Folder</span>
                            </button>

                            <button
                                onClick={handleCloneRepository}
                                disabled={isLoading()}
                                class="w-full bg-white hover:bg-white/50 text-black py-2 px-4 rounded-md transition-colors flex items-center justify-center space-x-2 text-xs"
                            >
                                <Download size={16} />
                                <span>Clone Repository</span>
                            </button>

                            <div class="text-xs text-gray-500 mt-4">
                                To learn more about how to use Git and source control{' '}
                                <a href="#" class="text-blue-500 hover:text-blue-600 underline">
                                    read our docs
                                </a>
                                .
                            </div>
                        </div>
                    </div>
                ) : (
                    /* Normal Git Interface - Repository Available */
                    <>
                        <div class="p-3">
                            {/* Branch Info */}
                            <div class="flex items-center gap-2 mb-3 p-2 bg-neutral-800 rounded">
                                <GitBranch size={14} class="text-green-400" />
                                <span class="text-xs text-gray-300">{currentBranch()}</span>
                            </div>

                            {/* Commit Message */}
                            <div class="mb-3">
                                <textarea
                                    placeholder="Commit message"
                                    value={commitMessage()}
                                    onInput={(e) => setCommitMessage(e.currentTarget.value)}
                                    class="w-full text-xs px-3 py-2 bg-neutral-800 placeholder:text-xs border border-neutral-700 rounded  text-gray-300 placeholder-gray-500 focus:outline-none focus:border-blue-500 resize-none"
                                    rows="3"
                                />
                                <div class="flex gap-2 mt-2">
                                    <button
                                        onClick={handleCommit}
                                        disabled={!commitMessage().trim() || stagedFiles().length === 0 || isLoading()}
                                        class="flex-1 px-3 py-2 bg-white disabled:bg-white/10 disabled:text-white/60 disabled:cursor-not-allowed text-black text-xs rounded transition-colors"
                                    >
                                        <GitCommit size={14} class="inline mr-2" />
                                        Commit ({stagedFiles().length})
                                    </button>
                                    <button
                                        onClick={stageAllFiles}
                                        disabled={unstagedFiles().length === 0 || isLoading()}
                                        class="px-3 py-2 bg-neutral-700 hover:bg-neutral-600 disabled:bg-neutral-800 disabled:text-white/60 disabled:cursor-not-allowed text-white text-xs rounded transition-colors"
                                        title="Stage All"
                                    >
                                        <Plus size={14} />
                                    </button>
                                </div>
                            </div>
                        </div>

                        <div class="flex-1 overflow-y-auto">
                            {/* Staged Changes */}
                            <div class="border-b border-neutral-800">
                                <div class="p-2 bg-neutral-800/50">
                                    <h4 class="text-[11px] font-medium text-gray-400 uppercase tracking-wide">
                                        Staged Changes ({stagedFiles().length})
                                    </h4>
                                </div>
                                <For each={stagedFiles()}>
                                    {(file) => (
                                        <div
                                            class="flex items-center justify-between p-2 hover:bg-neutral-800 group"
                                            onContextMenu={(e) => handleRightClick(e, file, true)}
                                        >
                                            <div class="flex items-center gap-2 flex-1 min-w-0">
                                                {getStatusIcon(file.status)}
                                                <span class="text-xs text-gray-300 truncate">{file.name}</span>
                                            </div>
                                            <div class="flex gap-1 opacity-0 group-hover:opacity-100 transition-opacity">
                                                <button
                                                    onClick={() => unstageFile(file)}
                                                    class="p-1 text-gray-400 hover:text-gray-300"
                                                    title="Unstage"
                                                >
                                                    <Minus size={12} />
                                                </button>
                                                <button
                                                    onClick={(e) => {
                                                        e.stopPropagation();
                                                        handleRightClick(e, file, true);
                                                    }}
                                                    class="p-1 text-gray-400 hover:text-gray-300"
                                                    title="More options"
                                                >
                                                    <MoreHorizontal size={12} />
                                                </button>
                                            </div>
                                        </div>
                                    )}
                                </For>
                            </div>

                            {/* Unstaged Changes */}
                            <div>
                                <div class="p-2 bg-neutral-800/50">
                                    <h4 class="text-[11px] font-medium text-gray-400 uppercase tracking-wide">
                                        Changes ({unstagedFiles().length})
                                    </h4>
                                </div>
                                <For each={unstagedFiles()}>
                                    {(file) => (
                                        <div
                                            class="flex items-center justify-between p-2 hover:bg-neutral-800 group"
                                            onContextMenu={(e) => handleRightClick(e, file, false)}
                                        >
                                            <div class="flex items-center gap-2 flex-1 min-w-0">
                                                {getStatusIcon(file.status)}
                                                <span class="text-xs text-gray-300 truncate">{file.name}</span>
                                            </div>
                                            <div class="flex gap-1 opacity-0 group-hover:opacity-100 transition-opacity">
                                                <button
                                                    onClick={() => stageFile(file)}
                                                    class="p-1 text-gray-400 hover:text-gray-300"
                                                    title="Stage"
                                                >
                                                    <Plus size={12} />
                                                </button>
                                                <button
                                                    onClick={() => discardChanges(file)}
                                                    class="p-1 text-gray-400 hover:text-gray-300"
                                                    title="Discard Changes"
                                                >
                                                    <RotateCcw size={12} />
                                                </button>
                                                <button
                                                    onClick={(e) => {
                                                        e.stopPropagation();
                                                        handleRightClick(e, file, false);
                                                    }}
                                                    class="p-1 text-gray-400 hover:text-gray-300"
                                                    title="More options"
                                                >
                                                    <MoreHorizontal size={12} />
                                                </button>
                                            </div>
                                        </div>
                                    )}
                                </For>
                            </div>
                        </div>
                    </>
                )}
            </div>

            <ContextMenu
                {...contextMenu()}
                onClose={() => setContextMenu(prev => ({ ...prev, visible: false }))}
                onStage={stageFile}
                onUnstage={unstageFile}
                onDiscard={discardChanges}
                onOpenFile={openFile}
            />

        </>
    );
}

const gitStyles = `
.git-context-menu {
    position: fixed;
    z-index: 1001;
}

.menu-dropdown {
    position: absolute;
    top: 100%;
    left: 0;
    background: var(--bg-primary, #1f1f1f);
    border: 1px solid var(--border-color, #404040);
    border-radius: 4px;
    box-shadow: 0 2px 8px rgba(0, 0, 0, 0.3);
    min-width: 200px;
    z-index: 1001;
}

.menu-item {
    position: relative;
    cursor: pointer;
    transition: background-color 0.2s;
    color: var(--text-primary, #e0e0e0);
}

.menu-item:hover:not(.disabled) {
    background: var(--bg-hover, #2a2a2a);
}

.menu-item.disabled {
    opacity: 0.5;
    cursor: not-allowed;
}

.menu-item-content {
    display: flex;
    align-items: center;
    padding: 8px 12px;
    gap: 8px;
}

.menu-item-label {
    flex: 1;
    font-size: 12px;
}

.menu-separator {
    height: 1px;
    background: var(--border-color, #404040);
    margin: 4px 0;
}
`;

export default GitPage;