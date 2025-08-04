import { onCleanup } from "solid-js";
import { Folder, Settings, Search, Blocks, Database,Pyramid, FlaskConical } from "lucide-solid";
import GitSrcControl from "../assets/images/tools/gitcontrol.svg"
import CmakeIcon from "../assets/images/tools/cmake.svg"
interface SideBarProps {
    width: number;
    onResize: (newWidth: number) => void;
}

function SideBar(props: SideBarProps) {
    let resizer: HTMLDivElement | undefined;
    let isResizing = false;

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

    return (
        <div
            class="flex h-full bg-neutral-900 rounded-[8px] relative border border-neutral-800 select-none"
            style={{ width: `${props.width}px` }}
        >
            {/* Sidebar menu */}
            <div class="flex flex-col justify-between p-3 text-gray-300 text-sm border-r border-neutral-800">
                <div class="flex flex-col gap-3 mb-4">
                    <button class="flex items-center gap-2 hover:text-white transition-colors">
                         {/* Explorer */}
                        <Folder size={16} />
                    </button>
                    <button class="flex items-center gap-2 hover:text-white transition-colors">
                         {/* Searc File */}
                        <Search size={16} />
                    </button>
                    <button class="flex items-center gap-2 hover:text-white transition-colors">
                        <div class="w-4 h-4 bg-center bg-cover" style={{ "background-image": `url(${GitSrcControl})` }} />
                    </button>
                    <button class="flex items-center gap-2 hover:text-white transition-colors">
                        {/* Extension */}
                        <Blocks size={16} /> 
                    </button>
                    <button class="flex items-center gap-2 hover:text-white transition-colors">
                         {/* Database Manager */}
                        <Database size={16} />
                    </button>
                    <button class="flex items-center gap-2 hover:text-white transition-colors">
                         {/* Prisma Manager */}
                        <Pyramid size={16} />
                    </button>
                    <button class="flex items-center gap-2 hover:text-white transition-colors">
                         {/* API tester */}
                        <FlaskConical size={16} />
                    </button>
                    <button class="flex items-center gap-2 hover:text-white transition-colors">
                         {/* CMake */}
                        <div class="w-4 h-4 bg-center bg-cover" style={{ "background-image": `url(${CmakeIcon})` }} />
                    </button>
                </div>
                <div>
                    <button class="flex items-center gap-2 text-gray-400 hover:text-white transition-colors">
                        <Settings size={16} />
                    </button>
                </div>
            </div>

            <div class="flex-1 p-2 overflow-y-auto">
                <p class="text-xs">Some content here</p>
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
