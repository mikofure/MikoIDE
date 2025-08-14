import { createSignal } from "solid-js";
import { Terminal, AlertCircle, CheckCircle, Info, Zap, Play} from "lucide-solid";
import TerminalView from "../terminal/TerminalView";
// import chromeIPC from "../../data/chromeipc";

interface BottomPanelProps {
    height: number;
    onResize: (newHeight: number) => void;
}

type BottomPanelPage = 'terminal' | 'problems' | 'output' | 'debug' | 'tasks' | 'ports';

function BottomPanel(props: BottomPanelProps) {
    const [activePage, setActivePage] = createSignal<BottomPanelPage>('terminal');



    const renderActivePage = () => {
        switch (activePage()) {
            case 'terminal': return (
                <TerminalView height={props.height - 80} />
            );
            case 'problems': return (
                <div class="p-4 text-gray-400 text-sm">
                    <div class="flex items-center gap-2 mb-4">
                        <AlertCircle size={16} class="text-red-400" />
                        <h3 class="text-white font-semibold">Problems</h3>
                    </div>
                    <div class="space-y-2">
                        <div class="flex items-center gap-2 text-red-400">
                            <AlertCircle size={14} />
                            <span class="text-xs">Error: Undefined variable 'example' at line 42</span>
                        </div>
                        <div class="flex items-center gap-2 text-yellow-400">
                            <Info size={14} />
                            <span class="text-xs">Warning: Unused import 'React' at line 1</span>
                        </div>
                        <div class="flex items-center gap-2 text-blue-400">
                            <Info size={14} />
                            <span class="text-xs">Info: Consider using const instead of let</span>
                        </div>
                    </div>
                </div>
            );
            case 'output': return (
                <div class="p-4 text-gray-400 text-sm">
                    <div class="flex items-center gap-2 mb-4">
                        <CheckCircle size={16} class="text-green-400" />
                        <h3 class="text-white font-semibold">Output</h3>
                    </div>
                    <div class="space-y-1 font-mono text-xs">
                        <div class="text-green-400">[INFO] Build started...</div>
                        <div class="text-blue-400">[DEBUG] Compiling TypeScript files...</div>
                        <div class="text-green-400">[INFO] Build completed successfully</div>
                        <div class="text-gray-300">[LOG] Server running on http://localhost:3000</div>
                    </div>
                </div>
            );
            case 'debug': return (
                <div class="p-4 text-gray-400 text-sm">
                    <div class="flex items-center gap-2 mb-4">
                        <Zap size={16} class="text-orange-400" />
                        <h3 class="text-white font-semibold">Debug Console</h3>
                    </div>
                    <div class="space-y-2">
                        <div class="text-xs opacity-60">No active debugging session</div>
                        <div class="flex gap-2">
                            <button class="bg-blue-600 hover:bg-blue-700 text-white px-3 py-1 rounded text-xs transition-colors">
                                Start Debugging
                            </button>
                            <button class="bg-gray-600 hover:bg-gray-700 text-white px-3 py-1 rounded text-xs transition-colors">
                                Attach to Process
                            </button>
                        </div>
                    </div>
                </div>
            );
            case 'tasks': return (
                <div class="p-4 text-gray-400 text-sm">
                    <div class="flex items-center gap-2 mb-4">
                        <Play size={16} class="text-purple-400" />
                        <h3 class="text-white font-semibold">Tasks</h3>
                    </div>
                    <div class="space-y-2">
                        <div class="flex items-center justify-between p-2 bg-neutral-800 rounded">
                            <span class="text-xs">Build Project</span>
                            <button class="bg-green-600 hover:bg-green-700 text-white px-2 py-1 rounded text-xs transition-colors">
                                Run
                            </button>
                        </div>
                        <div class="flex items-center justify-between p-2 bg-neutral-800 rounded">
                            <span class="text-xs">Test Suite</span>
                            <button class="bg-blue-600 hover:bg-blue-700 text-white px-2 py-1 rounded text-xs transition-colors">
                                Run
                            </button>
                        </div>
                        <div class="flex items-center justify-between p-2 bg-neutral-800 rounded">
                            <span class="text-xs">Deploy</span>
                            <button class="bg-purple-600 hover:bg-purple-700 text-white px-2 py-1 rounded text-xs transition-colors">
                                Run
                            </button>
                        </div>
                    </div>
                </div>
            );
            case 'ports': return (
                <div class="p-4 text-gray-400 text-sm">
                    <h3 class="text-white font-semibold mb-3">Ports</h3>
                    <div class="space-y-2">
                        <div class="flex items-center justify-between p-2 bg-neutral-800 rounded">
                            <div>
                                <div class="text-xs font-medium text-white">localhost:3000</div>
                                <div class="text-xs opacity-60">Development Server</div>
                            </div>
                            <div class="flex items-center gap-2">
                                <span class="w-2 h-2 bg-green-400 rounded-full"></span>
                                <span class="text-xs text-green-400">Running</span>
                            </div>
                        </div>
                        <div class="flex items-center justify-between p-2 bg-neutral-800 rounded">
                            <div>
                                <div class="text-xs font-medium text-white">localhost:5000</div>
                                <div class="text-xs opacity-60">API Server</div>
                            </div>
                            <div class="flex items-center gap-2">
                                <span class="w-2 h-2 bg-gray-400 rounded-full"></span>
                                <span class="text-xs text-gray-400">Stopped</span>
                            </div>
                        </div>
                    </div>
                </div>
            );
            default: return (
                <div class="flex flex-col h-full">
                    <div class="flex items-center justify-between px-4 py-2 bg-neutral-800 border-b border-neutral-700">
                        <div class="flex items-center gap-2">
                            <Terminal size={14} class="text-green-400" />
                            <span class="text-sm font-medium">Terminal</span>
                        </div>
                    </div>
                    <div class="flex-1 p-4 bg-black text-green-400 font-mono text-sm overflow-auto">
                        <div class="mb-2">MikoIDE Terminal v1.0.0</div>
                        <div class="mb-1">$ Welcome to integrated terminal</div>
                    </div>
                </div>
            );
        }
    };

    return (
        <div
            class="flex flex-col bg-neutral-900 rounded-md border border-neutral-800 select-none"
            style={{ height: `${props.height}px` }}
        >
            {/* Tab bar */}
            <div class="flex items-center px-3 py-2 border-b border-neutral-700">
                <div class="flex gap-1">
                    <button 
                        class={`px-3 py-1 flex items-center rounded text-xs transition-colors ${
                            activePage() === 'terminal' ? 'bg-neutral-700 text-white' : 'text-gray-400 hover:text-white hover:bg-neutral-700'
                        }`}
                        onClick={() => setActivePage('terminal')}
                        title="Terminal"
                    >
                        <Terminal size={12} class="mr-1" /> Terminal
                    </button>
                    <button 
                        class={`px-3 py-1 flex items-center  rounded text-xs transition-colors ${
                            activePage() === 'problems' ? 'bg-neutral-700 text-white' : 'text-gray-400 hover:text-white hover:bg-neutral-700'
                        }`}
                        onClick={() => setActivePage('problems')}
                        title="Problems"
                    >
                        <AlertCircle size={12} class="mr-1" /> Problems
                    </button>
                    <button 
                        class={`px-3 py-1 flex items-center  rounded text-xs transition-colors ${
                            activePage() === 'output' ? 'bg-neutral-700 text-white' : 'text-gray-400 hover:text-white hover:bg-neutral-700'
                        }`}
                        onClick={() => setActivePage('output')}
                        title="Output"
                    >
                        <CheckCircle size={12} class="mr-1" /> Output
                    </button>
                    <button 
                        class={`px-3 py-1 flex items-center  rounded text-xs transition-colors ${
                            activePage() === 'debug' ? 'bg-neutral-700 text-white' : 'text-gray-400 hover:text-white hover:bg-neutral-700'
                        }`}
                        onClick={() => setActivePage('debug')}
                        title="Debug Console"
                    >
                        <Zap size={12} class="mr-1" /> Debug
                    </button>
                    <button 
                        class={`px-3 py-1 flex items-center  rounded text-xs transition-colors ${
                            activePage() === 'tasks' ? 'bg-neutral-700 text-white' : 'text-gray-400 hover:text-white hover:bg-neutral-700'
                        }`}
                        onClick={() => setActivePage('tasks')}
                        title="Tasks"
                    >
                        <Play size={12} class="mr-1" /> Tasks
                    </button>
                    <button 
                        class={`px-3 py-1 flex items-center  rounded text-xs transition-colors ${
                            activePage() === 'ports' ? 'bg-neutral-700 text-white' : 'text-gray-400 hover:text-white hover:bg-neutral-700'
                        }`}
                        onClick={() => setActivePage('ports')}
                        title="Ports"
                    >
                        <Info size={12} class="mr-1" /> Ports
                    </button>
                </div>
            </div>

            {/* Active page content */}
            <div class="flex-1 overflow-hidden">
                {renderActivePage()}
            </div>
        </div>
    );
}

export default BottomPanel;