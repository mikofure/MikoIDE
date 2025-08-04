import { createSignal, For } from "solid-js";
import { ChevronRight, ChevronDown, File, Folder, FolderOpen } from "lucide-solid";

interface FileItem {
    name: string;
    type: 'file' | 'folder';
    path: string;
    children?: FileItem[];
    expanded?: boolean;
}

function Explorer() {
    const [files, setFiles] = createSignal<FileItem[]>([
        {
            name: "src",
            type: "folder",
            path: "/src",
            expanded: true,
            children: [
                { name: "main.tsx", type: "file", path: "/src/main.tsx" },
                { name: "App.tsx", type: "file", path: "/src/App.tsx" },
                {
                    name: "components",
                    type: "folder",
                    path: "/src/components",
                    expanded: false,
                    children: [
                        { name: "Button.tsx", type: "file", path: "/src/components/Button.tsx" },
                        { name: "Modal.tsx", type: "file", path: "/src/components/Modal.tsx" }
                    ]
                }
            ]
        },
        {
            name: "public",
            type: "folder",
            path: "/public",
            expanded: false,
            children: [
                { name: "index.html", type: "file", path: "/public/index.html" },
                { name: "favicon.ico", type: "file", path: "/public/favicon.ico" }
            ]
        },
        { name: "package.json", type: "file", path: "/package.json" },
        { name: "README.md", type: "file", path: "/README.md" }
    ]);

    const toggleFolder = (path: string) => {
        setFiles(prev => 
            prev.map(item => updateItemExpanded(item, path))
        );
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
                    onClick={() => item.type === 'folder' ? toggleFolder(item.path) : console.log('Open file:', item.path)}
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
        <div class="h-full flex flex-col">
            <div class="p-2 border-b border-neutral-800">
                <h3 class="text-xs font-medium text-gray-300 uppercase tracking-wide">Explorer</h3>
            </div>
            <div class="flex-1 overflow-y-auto">
                <For each={files()}>
                    {(item) => <FileTreeItem item={item} depth={0} />}
                </For>
            </div>
        </div>
    );
}

export default Explorer;