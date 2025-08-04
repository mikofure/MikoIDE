import { createSignal, onCleanup } from "solid-js";
import { Folder, Settings, Search, Blocks, Database, Pyramid, FlaskConical } from "lucide-solid";
import GitSrcControl from "../assets/images/tools/gitcontrol.svg";
import CmakeIcon from "../assets/images/tools/cmake.svg";

// Import sidebar pages
import Explorer from "./sidebar/explorer";
import SearchPage from "./sidebar/search";
import GitPage from "./sidebar/git";
import ExtensionsPage from "./sidebar/extensions";

interface SideBarProps {
    width: number;
    onResize: (newWidth: number) => void;
}

type SidebarPage = 'explorer' | 'search' | 'git' | 'extensions' | 'database' | 'prisma' | 'api' | 'cmake';

function SideBar(props: SideBarProps) {
    let resizer: HTMLDivElement | undefined;
    let isResizing = false;
    const [activePage, setActivePage] = createSignal<SidebarPage>('explorer');

    const onMouseDown = (e: MouseEvent) => {
        isResizing = true;
        e.preventDefault();
    };

    const onMouseMove = (e: MouseEvent) => {
        if (!isResizing) return;
        const newWidth = e.clientX;
        if (newWidth > 250 && newWidth < 600) {
            props.onResize(newWidth);
        }
    };

    const onMouseUp = () => {
        isResizing = false;
    };

    // Global events
    window.addEventListener("mousemove", onMouseMove);
    window.addEventListener("mouseup", onMouseUp);

    onCleanup(() => {
        window.removeEventListener("mousemove", onMouseMove);
        window.removeEventListener("mouseup", onMouseUp);
    });

    const renderActivePage = () => {
        switch (activePage()) {
            case 'explorer': return <Explorer />;
            case 'search': return <SearchPage />;
            case 'git': return <GitPage />;
            case 'extensions': return <ExtensionsPage />;
            case 'database': return <div class="p-4 text-gray-400 text-sm">Database Manager - Coming Soon</div>;
            case 'prisma': return <div class="p-4 text-gray-400 text-sm">Prisma Manager - Coming Soon</div>;
            case 'api': return <div class="p-4 text-gray-400 text-sm">API Tester - Coming Soon</div>;
            case 'cmake': return <div class="p-4 text-gray-400 text-sm">CMake Tools - Coming Soon</div>;
            default: return <Explorer />;
        }
    };

    return (
        <div
            class="flex h-full bg-neutral-900 rounded-[8px] relative border border-neutral-800 select-none"
            style={{ width: `${props.width}px` }}
        >
            {/* Sidebar menu */}
            <div class="flex flex-col justify-between p-3 text-gray-300 text-sm border-r border-neutral-800">
                <div class="flex flex-col gap-3 mb-4">
                    <button 
                        class={`flex items-center gap-2 hover:text-white transition-colors ${
                            activePage() === 'explorer' ? 'text-blue-400' : ''
                        }`}
                        onClick={() => setActivePage('explorer')}
                        title="Explorer"
                    >
                        <Folder size={16} />
                    </button>
                    <button 
                        class={`flex items-center gap-2 hover:text-white transition-colors ${
                            activePage() === 'search' ? 'text-blue-400' : ''
                        }`}
                        onClick={() => setActivePage('search')}
                        title="Search"
                    >
                        <Search size={16} />
                    </button>
                    <button 
                        class={`flex items-center gap-2 hover:text-white transition-colors ${
                            activePage() === 'git' ? 'text-blue-400' : ''
                        }`}
                        onClick={() => setActivePage('git')}
                        title="Source Control"
                    >
                        <div class="w-4 h-4 bg-center bg-cover" style={{ "background-image": `url(${GitSrcControl})` }} />
                    </button>
                    <button 
                        class={`flex items-center gap-2 hover:text-white transition-colors ${
                            activePage() === 'extensions' ? 'text-blue-400' : ''
                        }`}
                        onClick={() => setActivePage('extensions')}
                        title="Extensions"
                    >
                        <Blocks size={16} />
                    </button>
                    <button 
                        class={`flex items-center gap-2 hover:text-white transition-colors ${
                            activePage() === 'database' ? 'text-blue-400' : ''
                        }`}
                        onClick={() => setActivePage('database')}
                        title="Database Manager"
                    >
                        <Database size={16} />
                    </button>
                    <button 
                        class={`flex items-center gap-2 hover:text-white transition-colors ${
                            activePage() === 'prisma' ? 'text-blue-400' : ''
                        }`}
                        onClick={() => setActivePage('prisma')}
                        title="Prisma Manager"
                    >
                        <Pyramid size={16} />
                    </button>
                    <button 
                        class={`flex items-center gap-2 hover:text-white transition-colors ${
                            activePage() === 'api' ? 'text-blue-400' : ''
                        }`}
                        onClick={() => setActivePage('api')}
                        title="API Tester"
                    >
                        <FlaskConical size={16} />
                    </button>
                    <button 
                        class={`flex items-center gap-2 hover:text-white transition-colors ${
                            activePage() === 'cmake' ? 'text-blue-400' : ''
                        }`}
                        onClick={() => setActivePage('cmake')}
                        title="CMake Tools"
                    >
                        <div class="w-4 h-4 bg-center bg-cover" style={{ "background-image": `url(${CmakeIcon})` }} />
                    </button>
                </div>
                <div>
                    <button class="flex items-center gap-2 text-gray-400 hover:text-white transition-colors" title="Settings">
                        <Settings size={16} />
                    </button>
                </div>
            </div>

            {/* Active page content */}
            <div class="flex-1 overflow-hidden">
                {renderActivePage()}
            </div>

            {/* Divider for resizing */}
            <div
                ref={resizer}
                onMouseDown={onMouseDown}
                class="absolute top-0 right-0 h-full w-1 cursor-col-resize hover:bg-neutral-600"
            >
            </div>
        </div>
    );
}

export default SideBar;
