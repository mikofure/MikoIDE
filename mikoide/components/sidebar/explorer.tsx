import { createSignal, For } from "solid-js";
import { ChevronRight, ChevronDown, File, Folder, FolderOpen, AlertCircle } from "lucide-solid";
import chromeIPC from "../../data/chromeipc";
import { useI18n } from "../../i18n";
import { showTrustDialog } from "../dialog";

interface FileItem {
    name: string;
    type: 'file' | 'folder';
    path: string;
    children?: FileItem[];
    expanded?: boolean;
}

function Explorer() {
    const { t } = useI18n();
    const [files, setFiles] = createSignal<FileItem[]>([]);
    const [hasProject, setHasProject] = createSignal(false);
    const [isLoading, setIsLoading] = createSignal(false);
    const [error, setError] = createSignal<string | null>(null);

    // Load actual project files from the opened folder
    const loadProjectFiles = async (folderPath: string) => {
        try {
            setIsLoading(true);
            setError(null);
            
            const response = await chromeIPC.listDirectory(folderPath);
            if (response.success && response.data) {
                // Convert the directory listing to FileItem format
                const items: FileItem[] = response.data.map((item: any) => ({
                    name: item.name,
                    type: item.type,
                    path: item.path,
                    expanded: false,
                    children: item.type === 'folder' ? [] : undefined
                }));
                
                setFiles(items);
                setHasProject(true);
            } else {
                throw new Error(response.error || t('ui.explorer.failedToLoadDirectory'));
            }
        } catch (error) {
            console.error('Failed to load project files:', error);
            setError(error instanceof Error ? error.message : t('ui.explorer.failedToLoadFolder'));
        } finally {
            setIsLoading(false);
        }
    };

    const handleOpenFolder = async () => {
        // Check if CEF is available
        if (!chromeIPC.isAvailable()) {
            setError(t('ui.explorer.folderNotSupportedBrowser'));
            return;
        }

        try {
            setError(null);
            const response = await chromeIPC.executeMenuAction('file.open_folder');
            
            if (response.success && response.data?.folderPath) {
                // Show trust dialog for CEF folder opening
                const trusted = await showTrustDialog(response.data.folderPath);
                
                if (trusted) {
                    // Load the actual folder structure
                    await loadProjectFiles(response.data.folderPath);
                } else {
                    setError(t('ui.explorer.folderNotTrusted'));
                }
            } else if (response.data?.folderPath) {
                // Fallback: try to load even if response doesn't indicate success
                const trusted = await showTrustDialog(response.data.folderPath);
                
                if (trusted) {
                    await loadProjectFiles(response.data.folderPath);
                } else {
                    setError(t('ui.explorer.folderNotTrusted'));
                }
            }
        } catch (error) {
            console.error('Failed to open folder:', error);
            setError(t('ui.explorer.failedToOpenFolder'));
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
                <h3 class="text-xs font-medium text-gray-300 uppercase tracking-wide">{t('ui.explorer.title')}</h3>
            </div>
            <div class="flex-1 overflow-y-auto  items-center w-full">
                {hasProject() ? (
                    <For each={files()}>
                        {(item) => <FileTreeItem item={item} depth={0} />}
                    </For>
                ) : (
                    <div class="text-center py-8">
                        {error() ? (
                            <>
                                <AlertCircle class="w-12 h-12 text-red-500 mx-auto mb-4" />
                                <p class="text-red-400 mb-4 text-xs">{error()}</p>
                                {chromeIPC.isAvailable() && (
                                    <button
                                        onClick={handleOpenFolder}
                                        class="px-3 py-1.5 bg-white text-black hover:bg-gray-400 text-xs rounded transition-colors"
                                    >
                                        {t('ui.explorer.tryAgain')}
                                    </button>
                                )}
                            </>
                        ) : isLoading() ? (
                            <>
                                <div class="w-12 h-12 mx-auto mb-4 animate-spin">
                                    <Folder class="w-full h-full text-gray-500" />
                                </div>
                                <p class="text-gray-400 mb-4 text-xs">{t('ui.explorer.loadingFolder')}</p>
                            </>
                        ) : (
                            <>
                                <Folder class="w-12 h-12 text-gray-500 mx-auto mb-4" />
                                <p class="text-gray-400 mb-4 text-xs">{t('ui.explorer.noFolderOpened')}</p>
                                {chromeIPC.isAvailable() ? (
                                    <button
                                        onClick={handleOpenFolder}
                                        class="px-3 py-1.5 bg-white text-black hover:bg-gray-400 text-xs rounded transition-colors"
                                    >
                                        {t('ui.explorer.openFolder')}
                                    </button>
                                ) : (
                                    <p class="text-gray-500 text-xs">{t('ui.explorer.folderNotSupported')}</p>
                                )}
                            </>
                        )}
                    </div>
                )}
            </div>
        </div>
    );
}

export default Explorer;