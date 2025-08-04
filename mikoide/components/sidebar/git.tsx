import { createSignal, For } from "solid-js";
// @ts-expect-error
import { GitBranch, GitCommit, GitPullRequest, Plus, Minus, RotateCcw } from "lucide-solid";

interface GitFile {
    name: string;
    status: 'modified' | 'added' | 'deleted' | 'untracked';
    path: string;
}

function GitPage() {
    //@ts-expect-error
    const [currentBranch, setCurrentBranch] = createSignal("main");
    const [commitMessage, setCommitMessage] = createSignal("");
    const [stagedFiles, setStagedFiles] = createSignal<GitFile[]>([
        { name: "src/main.tsx", status: "modified", path: "src/main.tsx" },
        { name: "package.json", status: "modified", path: "package.json" }
    ]);
    const [unstagedFiles, setUnstagedFiles] = createSignal<GitFile[]>([
        { name: "README.md", status: "modified", path: "README.md" },
        { name: "src/new-component.tsx", status: "untracked", path: "src/new-component.tsx" }
    ]);

    const getStatusIcon = (status: string) => {
        switch (status) {
            case 'modified': return <span class="text-orange-400 text-xs">M</span>;
            case 'added': return <span class="text-green-400 text-xs">A</span>;
            case 'deleted': return <span class="text-red-400 text-xs">D</span>;
            case 'untracked': return <span class="text-blue-400 text-xs">U</span>;
            default: return <span class="text-gray-400 text-xs">?</span>;
        }
    };

    const stageFile = (file: GitFile) => {
        setUnstagedFiles(prev => prev.filter(f => f.path !== file.path));
        setStagedFiles(prev => [...prev, file]);
    };

    const unstageFile = (file: GitFile) => {
        setStagedFiles(prev => prev.filter(f => f.path !== file.path));
        setUnstagedFiles(prev => [...prev, file]);
    };

    const handleCommit = () => {
        if (!commitMessage().trim() || stagedFiles().length === 0) return;
        console.log('Committing:', commitMessage(), stagedFiles());
        setCommitMessage("");
        setStagedFiles([]);
    };

    return (
        <div class="h-full flex flex-col">
            <div class="p-3 border-b border-neutral-800">
                <h3 class="text-xs font-medium text-gray-300 uppercase tracking-wide mb-3">Source Control</h3>
                
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
                        class="w-full px-3 py-2 bg-neutral-800 placeholder:text-xs border border-neutral-700 rounded text-sm text-gray-300 placeholder-gray-500 focus:outline-none focus:border-blue-500 resize-none"
                        rows="3"
                    />
                    <button
                        onClick={handleCommit}
                        disabled={!commitMessage().trim() || stagedFiles().length === 0}
                        class="w-full mt-2 px-3 py-2 bg-white disabled:bg-white/10 disabled:text-white/60 disabled:cursor-not-allowed text-black text-xs rounded transition-colors"
                    >
                        <GitCommit size={14} class="inline mr-2" />
                        Commit ({stagedFiles().length})
                    </button>
                </div>
            </div>

            <div class="flex-1 overflow-y-auto">
                {/* Staged Changes */}
                <div class="border-b border-neutral-800">
                    <div class="p-2 bg-neutral-800/50">
                        <h4 class="text-xs font-medium text-gray-400 uppercase tracking-wide">
                            Staged Changes ({stagedFiles().length})
                        </h4>
                    </div>
                    <For each={stagedFiles()}>
                        {(file) => (
                            <div class="flex items-center justify-between p-2 hover:bg-neutral-800 group">
                                <div class="flex items-center gap-2 flex-1 min-w-0">
                                    {getStatusIcon(file.status)}
                                    <span class="text-xs text-gray-300 truncate">{file.name}</span>
                                </div>
                                <button
                                    onClick={() => unstageFile(file)}
                                    class="opacity-0 group-hover:opacity-100 p-1 text-gray-400 hover:text-gray-300 transition-opacity"
                                    title="Unstage"
                                >
                                    <Minus size={12} />
                                </button>
                            </div>
                        )}
                    </For>
                </div>

                {/* Unstaged Changes */}
                <div>
                    <div class="p-2 bg-neutral-800/50">
                        <h4 class="text-xs font-medium text-gray-400 uppercase tracking-wide">
                            Changes ({unstagedFiles().length})
                        </h4>
                    </div>
                    <For each={unstagedFiles()}>
                        {(file) => (
                            <div class="flex items-center justify-between p-2 hover:bg-neutral-800 group">
                                <div class="flex items-center gap-2 flex-1 min-w-0">
                                    {getStatusIcon(file.status)}
                                    <span class="text-sm text-gray-300 truncate">{file.name}</span>
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
                                        class="p-1 text-gray-400 hover:text-gray-300"
                                        title="Discard Changes"
                                    >
                                        <RotateCcw size={12} />
                                    </button>
                                </div>
                            </div>
                        )}
                    </For>
                </div>
            </div>
        </div>
    );
}

export default GitPage;