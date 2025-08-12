import { createSignal, onMount, onCleanup } from "solid-js";
import MenuBar from "./menubar"; // 👈 import
import CommandPalette from "./cmdpalette";
import { Search, PanelLeft, PanelBottom, PanelRight, LayoutGrid, Aperture, ChevronDown, Settings, User, LogOut, Github, Monitor } from "lucide-solid";
import chromeIPC from "../data/chromeipc";
import microsoftIcon from "../assets/images/brand/microsoft.svg";
import googleIcon from "../assets/images/brand/google.svg";

import "../styles/titlebar.css";

interface TitleBarProps {
    onCaptureClick?: () => void;
    editorInstance?: any; // Monaco editor instance
    panelStates?: {
        left: boolean;
        right: boolean;
        bottom: boolean;
        grid: boolean;
    };
    onPanelStateChange?: (panel: 'left' | 'right' | 'bottom' | 'grid', isOpen: boolean) => void;
}

function TitleBar(props: TitleBarProps) {
    const [searchQuery, setSearchQuery] = createSignal("");
    // const [isSearchFocused, setIsSearchFocused] = createSignal(false);
    const [isAccountMenuOpen, setIsAccountMenuOpen] = createSignal(false);
    const [isLoggedIn, setIsLoggedIn] = createSignal(false);
    const [isCommandPaletteOpen, setIsCommandPaletteOpen] = createSignal(false);
    const [panelStates, setPanelStates] = createSignal({
        left: props.panelStates?.left ?? true,
        right: props.panelStates?.right ?? false,
        bottom: props.panelStates?.bottom ?? false,
        grid: props.panelStates?.grid ?? false
    });

    // Close dropdown when clicking outside
    const handleClickOutside = (e: MouseEvent) => {
        const target = e.target as HTMLElement;
        if (!target.closest('.account-menu-container')) {
            setIsAccountMenuOpen(false);
        }
        // Don't close command palette here - it has its own backdrop handler
    };

    onMount(() => {
        document.addEventListener('click', handleClickOutside);
    });

    onCleanup(() => {
        document.removeEventListener('click', handleClickOutside);
    });

    // Handle search functionality
    // const handleSearch = async (query: string) => {
    //     if (query.trim()) {
    //         try {
    //             await chromeIPC.searchFiles(query);
    //         } catch (error) {
    //             console.error('Search failed:', error);
    //         }
    //     }
    // };


    // Handle command palette close
    const handleCommandPaletteClose = () => {
        setIsCommandPaletteOpen(false);
    };

    // Handle command execution
    const handleCommandExecute = (command: string) => {
        console.log('Executing command:', command);
        setIsCommandPaletteOpen(false);
        setSearchQuery("");
    };

    // Panel toggle handlers
    const handlePanelToggle = async (panel: 'left' | 'bottom' | 'right' | 'grid') => {
        try {
            const newState = !panelStates()[panel];
            setPanelStates(prev => ({ ...prev, [panel]: newState }));
            
            // Notify parent component about panel state change
            if (props.onPanelStateChange) {
                props.onPanelStateChange(panel, newState);
            }
            
            await chromeIPC.togglePanel(panel);
        } catch (error) {
            console.error(`Failed to toggle ${panel} panel:`, error);
            // Revert state on error
            setPanelStates(prev => ({ ...prev, [panel]: !prev[panel] }));
        }
    };

    // New window handler
    const handleNewWindow = async () => {
        try {
            await chromeIPC.spawnNewWindow();
        } catch (error) {
            console.error('Failed to spawn new window:', error);
        }
    };

    // Capture handler
    const handleCapture = async () => {
        try {
            await chromeIPC.captureEditor();
            if (props.onCaptureClick) {
                props.onCaptureClick();
            }
        } catch (error) {
            console.error('Capture failed:', error);
        }
    };

    return (
        <div class="fixed w-screen flex items-center text-white select-none z-[999] h-[40px] px-2" style={{ "-webkit-app-region": "drag" }}>
            {/* ซ้าย */}
            <div class="flex items-center" style={{ "-webkit-app-region": "no-drag" }}>
                <MenuBar />
            </div>

            {/* กลาง (search button) */}
            <div class="absolute left-1/2 top-1/2 -translate-x-1/2 -translate-y-1/2 flex" style={{ "-webkit-app-region": "no-drag" }}>
                <p class="text-white text-xs font-semibold">MikoIDE</p>
                <p class="text-xs opacity-55">/welcome</p>
            </div>

            {/* Command Palette - Centered on screen */}
            <CommandPalette
                isOpen={isCommandPaletteOpen()}
                searchQuery={searchQuery()}
                onClose={handleCommandPaletteClose}
                onExecute={handleCommandExecute}
                onSearchChange={setSearchQuery}
                editorInstance={props.editorInstance}
            />

            {/* ขวา */}
            <div class="flex gap-1 ml-auto h-full">
                {/* cmdpalette */}
                <button
                    class={`flex items-center px-3 py-1 transition-colors duration-200 cursor-pointer hover:bg-neutral-800 ${
                        isCommandPaletteOpen() ? 'border-blue-500/60 bg-neutral-800' : 'border-white/20 hover:border-white/40'
                    }`}
                    onClick={(e) => {
                        e.stopPropagation();
                        setIsCommandPaletteOpen(true);
                    }}
                    title="Open Command Palette (Ctrl+Shift+P)"
                >
                    <Search class="w-4 h-4 text-white" />
                </button>
                {/* capture button */}
                <button
                    class="p-2 w-8 h-full hover:bg-white/10 transition-colors rounded"
                    onClick={handleCapture}
                    title="Capture Code Editor (Ctrl+Alt+F)"
                >
                    <Aperture class="w-4 h-4" />
                </button>
                
                {/* Panel toggle buttons with active states */}
                <button
                    class={`p-2 w-8 h-full transition-colors rounded ${
                        panelStates().left 
                            ? 'bg-blue-600/20 text-blue-400 hover:bg-blue-600/30' 
                            : 'hover:bg-white/10'
                    }`}
                    onClick={() => handlePanelToggle('left')}
                    title="Toggle Left Panel (Explorer, Search, Git)"
                >
                    <PanelLeft class="w-4 h-4" />
                </button>
                <button
                    class={`p-2 w-8 h-full transition-colors rounded ${
                        panelStates().bottom 
                            ? 'bg-blue-600/20 text-blue-400 hover:bg-blue-600/30' 
                            : 'hover:bg-white/10'
                    }`}
                    onClick={() => handlePanelToggle('bottom')}
                    title="Toggle Bottom Panel (Terminal, Problems, Output)"
                >
                    <PanelBottom class="w-4 h-4" />
                </button>
                <button
                    class={`p-2 w-8 h-full transition-colors rounded ${
                        panelStates().right 
                            ? 'bg-blue-600/20 text-blue-400 hover:bg-blue-600/30' 
                            : 'hover:bg-white/10'
                    }`}
                    onClick={() => handlePanelToggle('right')}
                    title="Toggle Right Panel (Terminal, Debug, Performance)"
                >
                    <PanelRight class="w-4 h-4" />
                </button>
                <button
                    class={`p-2 w-8 h-full transition-colors rounded ${
                        panelStates().grid 
                            ? 'bg-blue-600/20 text-blue-400 hover:bg-blue-600/30' 
                            : 'hover:bg-white/10'
                    }`}
                    onClick={() => handlePanelToggle('grid')}
                    title="Toggle Grid Layout"
                >
                    <LayoutGrid class="w-4 h-4" />
                </button>
                
                {/* New window button */}
                <button
                    class="p-2 w-8 h-full hover:bg-white/10 transition-colors rounded"
                    onClick={handleNewWindow}
                    title="Open New Window (Ctrl+Shift+N)"
                >
                    <Monitor class="w-4 h-4" />
                </button>
                <div class="p-2 h-full flex items-center relative account-menu-container">
                    {/* account menu */}
                    <button
                        class="bg-[#171717] rounded-full flex items-center space-x-1 p-1 hover:bg-[#252525] transition-colors"
                        onClick={() => setIsAccountMenuOpen(!isAccountMenuOpen())}
                    >
                        <div
                            class="w-5 h-5 rounded-full bg-center bg-cover"
                            style={{ "background-image": isLoggedIn() ? "url('https://avatars.githubusercontent.com/u/99713905?v=4')" : "" }}
                            classList={{ "bg-gray-600": !isLoggedIn() }}
                        >
                            {!isLoggedIn() && <User class="w-3 h-3 text-gray-400 m-1" />}
                        </div>
                        {isLoggedIn() && <p class="text-xs pr-1 opacity-60">Ariz Kamizuki</p>}
                        <ChevronDown class="w-3 h-3 opacity-60" />
                    </button>

                    {/* Dropdown Menu */}
                    {isAccountMenuOpen() && (
                        <div class="absolute -left-34 top-full mt-2 bg-[#1a1a1a98] backdrop-blur-lg border border-[#323132] shadow-lg rounded-md min-w-48 z-[999] py-1">
                            {isLoggedIn() ? (
                                <>
                                    <div class="px-3 py-2 border-b border-[#323132]">
                                        <p class="text-xs font-medium">Ariz Kamizuki</p>
                                        <p class="text-xs opacity-60">ariz@example.com</p>
                                    </div>
                                    <button class="w-full px-3 py-2 text-left text-xs hover:bg-white/10 transition-colors flex items-center gap-2">
                                        <User class="w-4 h-4" />
                                        Manage Account
                                    </button>
                                    <button class="w-full px-3 py-2 text-left text-xs hover:bg-white/10 transition-colors flex items-center gap-2">
                                        <Settings class="w-4 h-4" />
                                        IDE Settings
                                    </button>
                                    <div class="border-t border-[#323132] mt-1 pt-1">
                                        <button
                                            class="w-full px-3 py-2 text-left text-xs hover:bg-white/10 transition-colors flex items-center gap-2 text-red-400"
                                            onClick={() => setIsLoggedIn(false)}
                                        >
                                            <LogOut class="w-4 h-4" />
                                            Sign Out
                                        </button>
                                    </div>
                                </>
                            ) : (
                                <>
                                    <div class="px-3 py-2 border-b border-[#323132]">
                                        <p class="text-xs font-medium">Sign in to MikoIDE</p>
                                        <p class="text-xs opacity-60">Access your projects and settings</p>
                                    </div>
                                    <button
                                        class="w-full px-3 py-2 text-left text-xs hover:bg-white/10 transition-colors flex items-center gap-2"
                                        onClick={() => setIsLoggedIn(true)}
                                    >
                                        <Github class="w-4 h-4" />
                                        Sign in with GitHub
                                    </button>
                                    <button class="w-full px-3 py-2 text-left text-xs hover:bg-white/10 transition-colors flex items-center gap-2">
                                        <img src={microsoftIcon} alt="Microsoft" class="w-4 h-4" />
                                        Sign in with Microsoft
                                    </button>
                                    <button class="w-full px-3 py-2 text-left text-xs hover:bg-white/10 transition-colors flex items-center gap-2">
                                        <img src={googleIcon} alt="Google" class="w-4 h-4" />
                                        Sign in with Google
                                    </button>
                                    <div class="border-t border-[#323132] mt-1 pt-1">
                                        <button class="w-full px-3 py-2 text-left text-xs hover:bg-white/10 transition-colors flex items-center gap-2">
                                            <Settings class="w-4 h-4" />
                                            IDE Settings
                                        </button>
                                    </div>
                                </>
                            )}
                        </div>
                    )}
                </div>

            </div>
        </div>
    );
}

export default TitleBar;
