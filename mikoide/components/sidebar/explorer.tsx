import { createSignal, For } from "solid-js";
import { ChevronRight, ChevronDown, File, Folder, FolderOpen } from "lucide-solid";
import chromeIPC from "../../data/chromeipc";

interface FileItem {
    name: string;
    type: 'file' | 'folder';
    path: string;
    children?: FileItem[];
    expanded?: boolean;
}

function Explorer() {
    const [files, setFiles] = createSignal<FileItem[]>([]);
    const [hasProject, setHasProject] = createSignal(false);

    // Mock function to load project files
    const loadProject = () => {
        setFiles([
            {
                name: "src",
                type: "folder",
                path: "/src",
                expanded: true,
                children: [
                    {
                        name: "components",
                        type: "folder",
                        path: "/src/components",
                        expanded: false,
                        children: [
                            { name: "Button.tsx", type: "file", path: "/src/components/Button.tsx" },
                            { name: "Input.tsx", type: "file", path: "/src/components/Input.tsx" }
                        ]
                    },
                    { name: "App.tsx", type: "file", path: "/src/App.tsx" },
                    { name: "index.tsx", type: "file", path: "/src/index.tsx" }
                ]
            },
            { name: "package.json", type: "file", path: "/package.json" },
            { name: "README.md", type: "file", path: "/README.md" }
        ]);
        setHasProject(true);
    };

    const handleOpenFolder = async () => {
        try {
            await chromeIPC.executeMenuAction('file.open_folder');
            // In a real implementation, this would load the actual folder structure
            loadProject();
        } catch (error) {
            console.error('Failed to open folder:', error);
        }
    };

    const toggleFolder = (path: string) => {
        setFiles(prev => 
            prev.map(item => updateItemExpanded(item, path))
        );
    };

    // Handle file opening
    const handleFileOpen = async (filePath: string) => {
        try {
            await chromeIPC.openFile(filePath);
        } catch (error) {
            console.error('Failed to open file:', error);
        }
    };

    const updateItemExpanded = (item: FileItem, targetPath: string): FileItem => {
        if (item.path === targetPath && item.type === 'folder') {
            return { ...item, expanded: !item.expanded };
        }
        if (item.children) {
            return {
                ...item,
                children: item.children.map(child => updateItemExpanded(child, targetPath))
            };
        }
        return item;
    };

    const FileTreeItem = (props: { item: FileItem; depth: number }) => {
        const { item, depth } = props;
        
        return (
            <div>
                <div 
                    class="flex items-center gap-1 px-2 py-1 hover:bg-neutral-800 cursor-pointer text-sm"
                    style={{ "padding-left": `${8 + depth * 16}px` }}
                    onClick={() => item.type === 'folder' ? toggleFolder(item.path) : handleFileOpen(item.path)}
                >
                    {item.type === 'folder' ? (
                        <>
                            {item.expanded ? <ChevronDown size={14} /> : <ChevronRight size={14} />}
                            {item.expanded ? <FolderOpen size={14} class="text-blue-400" /> : <Folder size={14} class="text-blue-400" />}
                        </>
                    ) : (
                        <>
                            <div class="w-[14px]" /> {/* Spacer for alignment */}
                            <File size={14} class="text-gray-400" />
                        </>
                    )}
                    <span class="text-gray-300 truncate text-xs">{item.name}</span>
                </div>
                
                {item.type === 'folder' && item.expanded && item.children && (
                    <For each={item.children}>
                        {(child) => <FileTreeItem item={child} depth={depth + 1} />}
                    </For>
                )}
            </div>
        );
    };

    return (
        <div class="h-full w-full flex flex-col">
            <div class="p-2 border-b border-neutral-800">
                <h3 class="text-xs font-medium text-gray-300 uppercase tracking-wide">Explorer</h3>
            </div>
            <div class="flex-1 overflow-y-auto  items-center w-full">
                {hasProject() ? (
                    <For each={files()}>
                        {(item) => <FileTreeItem item={item} depth={0} />}
                    </For>
                ) : (
                    <div class="text-center py-8">
                        <Folder class="w-12 h-12 text-gray-500 mx-auto mb-4" />
                        <p class="text-gray-400 mb-4 text-xs">No folder opened</p>
                        <button
                            onClick={handleOpenFolder}
                            class="px-3 py-1.5 bg-white text-black hover:bg-gray-400 text-xs rounded transition-colors"
                        >
                            Open Folder
                        </button>
                    </div>
                )}
            </div>
        </div>
    );
}

export default Explorer;