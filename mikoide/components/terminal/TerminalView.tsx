import { createSignal, For, createEffect } from "solid-js";
import { Plus, ChevronDown, Terminal, Settings, AlertTriangle } from "lucide-solid";
import TerminalTab from "./TerminalTab";
import TerminalInstance from "./TerminalInstance";
import chromeIPC from "../../core/chromeipc";

interface TerminalData {
    id: string;
    name: string;
    status: 'idle' | 'running' | 'error';
}

interface TerminalViewProps {
    height?: number;
}

function TerminalView(_props: TerminalViewProps) {
    const [terminals, setTerminals] = createSignal<TerminalData[]>([
        {
            id: '1',
            name: 'PowerShell',
            status: 'idle'
        }
    ]);
    
    const [activeTerminalId, setActiveTerminalId] = createSignal('1');
    const [showTabs, setShowTabs] = createSignal(true);
    const [isTerminalAvailable, setIsTerminalAvailable] = createSignal(true);
    
    // Check if terminal operations are available (CEF only)
    createEffect(async () => {
        try {
            const response = await chromeIPC.openTerminal();
            if (!response.success && response.error?.includes('only available in desktop mode')) {
                setIsTerminalAvailable(false);
            }
        } catch (error) {
            setIsTerminalAvailable(false);
        }
    });
    
    const activeTerminal = () => terminals().find(t => t.id === activeTerminalId());
    
    const addNewTerminal = () => {
        const newId = (terminals().length + 1).toString();
        const newTerminal: TerminalData = {
            id: newId,
            name: `PowerShell ${newId}`,
            status: 'idle'
        };
        
        setTerminals(prev => [...prev, newTerminal]);
        setActiveTerminalId(newId);
    };
    
    const closeTerminal = (terminalId: string) => {
        if (terminals().length === 1) return; // Don't close the last terminal
        
        setTerminals(prev => prev.filter(t => t.id !== terminalId));
        
        if (activeTerminalId() === terminalId) {
            const remainingTerminals = terminals().filter(t => t.id !== terminalId);
            if (remainingTerminals.length > 0) {
                setActiveTerminalId(remainingTerminals[0].id);
            }
        }
    };
    
    const handleStatusChange = (terminalId: string, status: 'idle' | 'running' | 'error') => {
        setTerminals(prev => prev.map(t => 
            t.id === terminalId ? { ...t, status } : t
        ));
    };
    
    const handleCommand = (terminalId: string, command: string) => {
        console.log(`Terminal ${terminalId} executed: ${command}`);
        // Here you could integrate with actual terminal backend
    };
    
    return (
        <div class="flex flex-col h-full bg-neutral-900 border border-neutral-800 overflow-hidden">
            {/* Terminal Tabs */}
            {showTabs() && terminals().length > 1 && (
                <div class="flex items-center bg-neutral-800 border-b border-neutral-700 px-2 py-1">
                    <div class="flex items-center gap-1 flex-1 overflow-x-auto">
                        <For each={terminals()}>
                            {(terminal) => (
                                <TerminalTab
                                    id={terminal.id}
                                    name={terminal.name}
                                    status={terminal.status}
                                    isActive={terminal.id === activeTerminalId()}
                                    onSelect={setActiveTerminalId}
                                    onClose={closeTerminal}
                                    canClose={terminals().length > 1}
                                />
                            )}
                        </For>
                    </div>
                    
                    {/* Terminal Actions */}
                    <div class="flex items-center gap-1 ml-2">
                        <button 
                            class={`p-1.5 rounded transition-colors ${
                                isTerminalAvailable() 
                                    ? 'text-gray-400 hover:text-white hover:bg-neutral-700' 
                                    : 'text-gray-600 cursor-not-allowed'
                            }`}
                            onClick={isTerminalAvailable() ? addNewTerminal : undefined}
                            title={isTerminalAvailable() ? "New Terminal" : "Terminal not available in web browser"}
                            disabled={!isTerminalAvailable()}
                        >
                            <Plus size={14} />
                        </button>
                        <button 
                            class="p-1.5 text-gray-400 hover:text-white hover:bg-neutral-700 rounded transition-colors"
                            onClick={() => setShowTabs(!showTabs())}
                            title="Toggle Tabs"
                        >
                            <ChevronDown size={14} class={showTabs() ? '' : 'rotate-180'} />
                        </button>
                        <button 
                            class="p-1.5 text-gray-400 hover:text-white hover:bg-neutral-700 rounded transition-colors"
                            title="Terminal Settings"
                        >
                            <Settings size={14} />
                        </button>
                    </div>
                </div>
            )}
            
            {/* Single Terminal Header (when only one terminal) */}
            {(!showTabs() || terminals().length === 1) && (
                <div class="flex items-center justify-between px-4 py-2 bg-neutral-800 border-b border-neutral-700">
                    <div class="flex items-center gap-2">
                        <Terminal size={14} class="text-green-400" />
                        <span class="text-xs font-medium text-white">{activeTerminal()?.name || 'Terminal'}</span>
                        <div class={`w-2 h-2 rounded-full ${
                            activeTerminal()?.status === 'running' ? 'bg-yellow-400' :
                            activeTerminal()?.status === 'error' ? 'bg-red-400' : 'bg-green-400'
                        }`}></div>
                    </div>
                    
                    <div class="flex items-center gap-1">
                        <button 
                            class={`p-1.5 rounded transition-colors ${
                                isTerminalAvailable() 
                                    ? 'text-gray-400 hover:text-white hover:bg-neutral-700' 
                                    : 'text-gray-600 cursor-not-allowed'
                            }`}
                            onClick={isTerminalAvailable() ? addNewTerminal : undefined}
                            title={isTerminalAvailable() ? "New Terminal" : "Terminal not available in web browser"}
                            disabled={!isTerminalAvailable()}
                        >
                            <Plus size={14} />
                        </button>
                        <button 
                            class="p-1.5 text-gray-400 hover:text-white hover:bg-neutral-700 rounded transition-colors"
                            title="Terminal Settings"
                        >
                            <Settings size={14} />
                        </button>
                    </div>
                </div>
            )}
            
            {/* Terminal Content */}
            <div class="flex-1 overflow-hidden">
                {!isTerminalAvailable() ? (
                    <div class="flex flex-col items-center justify-center h-full p-8 text-center">
                        <AlertTriangle size={48} class="text-yellow-500 mb-4" />
                        <h3 class="text-sm font-semibold text-white mb-2">Terminal Not Available</h3>
                        <p class="text-gray-400 mb-4 max-w-md text-sm">
                            Terminal operations are only available in desktop application. 
                            Web browser environment is not supported.
                        </p>
                        <div class="text-xs text-gray-500">
                            Please use the desktop version of MikoIDE to access terminal functionality.
                        </div>
                    </div>
                ) : activeTerminal() && (
                    <TerminalInstance
                        id={activeTerminal()!.id}
                        name={activeTerminal()!.name}
                        status={activeTerminal()!.status}
                        onStatusChange={handleStatusChange}
                        onCommand={handleCommand}
                    />
                )}
            </div>
        </div>
    );
}

export default TerminalView;