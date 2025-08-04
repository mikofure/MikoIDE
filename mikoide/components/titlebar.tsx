// import { createSignal } from "solid-js";
import MenuBar from "./menubar"; // 👈 import
import { Search, PanelLeft, PanelBottom, PanelRight, LayoutGrid } from "lucide-solid";
import "../styles/titlebar.css";

function TitleBar() {
    // const [isRestored, setIsRestored] = createSignal(false);

    return (
        <div class="fixed w-screen flex items-center text-white select-none z-[999] h-[40px] px-2" style={{ "-webkit-app-region": "drag" }}>
            {/* ซ้าย */}
            <div class="flex items-center" style={{ "-webkit-app-region": "no-drag" }}>
                <MenuBar />
            </div>

            {/* กลาง (search กลางเป๊ะ) */}
            <div class="absolute left-1/2 top-1/2 -translate-x-1/2 -translate-y-1/2" style={{ "-webkit-app-region": "no-drag" }}>
                <div class="flex items-center gap-2 border border-white/20 rounded-full bg-neutral-900 px-3 py-1 hover:border-white/40 transition-colors duration-200">
                    <Search class="w-4 h-4 text-gray-400" />
                    <input
                        type="text"
                        placeholder="Search"
                        class="bg-transparent text-xs placeholder:text-gray-500 outline-none w-40 focus:w-60 transition-all duration-300 ease-in-out"
                    />
                </div>
            </div>

            {/* ขวา */}
            <div class="flex gap-1 ml-auto h-full">
                <button class="p-2 w-8 h-full">
                    <PanelLeft class="w-4 h-4" />
                </button>
                <button class="p-2 w-8 h-full">
                    <PanelBottom class="w-4 h-4" />
                </button>
                <button class="p-2 w-8 h-full">
                    <PanelRight class="w-4 h-4" />
                </button>
                <button class="p-2 w-8 h-full">
                    <LayoutGrid class="w-4 h-4" />
                </button>
                <div class="p-2 h-full flex items-center">
                    <div class="bg-[#171717] rounded-full flex items-center space-x-1 p-1">
                        <div
                            class="w-5 h-5 rounded-full bg-center bg-cover"
                            style={{ "background-image": "url('https://avatars.githubusercontent.com/u/99713905?v=4')" }}
                        />
                        <p class="text-xs pr-1 opacity-60">Ariz Kamizuki</p>
                    </div>
                </div>

            </div>
        </div>
    );
}

export default TitleBar;
