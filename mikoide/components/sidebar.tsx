import { onCleanup } from "solid-js";
import { Folder, FilePlus, Settings } from "lucide-solid";

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
        if (newWidth > 100 && newWidth < 600) {
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
            class="flex h-full bg-neutral-900 rounded-[8px] relative border border-neutral-800"
            style={{ width: `${props.width}px` }}
        >
            {/* Sidebar menu */}
            <div class="flex flex-col justify-between p-3 text-gray-300 text-sm border-r border-neutral-800">
                <div class="flex flex-col gap-3 mb-4">
                    <button class="flex items-center gap-2 hover:text-white transition-colors">
                        <Folder size={16} />
                    </button>
                    <button class="flex items-center gap-2 hover:text-white transition-colors">
                        <FilePlus size={16} />
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
