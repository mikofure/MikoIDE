import { createSignal, createEffect, onMount, For } from "solid-js";
import { Square, RotateCcw, Copy } from "lucide-solid";

interface TerminalOutput {
    id: string;
    content: string;
    type: 'command' | 'output' | 'error' | 'info';
    timestamp: Date;
}

interface TerminalInstanceProps {
    id: string;
    name: string;
    status: 'idle' | 'running' | 'error';
    onStatusChange: (id: string, status: 'idle' | 'running' | 'error') => void;
    onCommand: (id: string, command: string) => void;
}

function TerminalInstance(props: TerminalInstanceProps) {
    const [output, setOutput] = createSignal<TerminalOutput[]>([
        {
            id: '1',
            content: 'MikoIDE Terminal v1.0.0',
            type: 'info',
            timestamp: new Date()
        },
        {
            id: '2',
            content: 'Copyright (c) 2024 MikoIDE. All rights reserved.',
            type: 'info',
            timestamp: new Date()
        },
        {
            id: '3',
            content: '',
            type: 'output',
            timestamp: new Date()
        }
    ]);
    
    const [currentInput, setCurrentInput] = createSignal('');
    const [commandHistory, setCommandHistory] = createSignal<string[]>([]);
    const [historyIndex, setHistoryIndex] = createSignal(-1);
    
    let terminalRef: HTMLDivElement | undefined;
    let inputRef: HTMLInputElement | undefined;
    
    const scrollToBottom = () => {
        if (terminalRef) {
            terminalRef.scrollTop = terminalRef.scrollHeight;
        }
    };
    
    createEffect(() => {
        scrollToBottom();
    });
    
    const addOutput = (content: string, type: TerminalOutput['type'] = 'output') => {
        const newOutput: TerminalOutput = {
            id: Date.now().toString(),
            content,
            type,
            timestamp: new Date()
        };
        setOutput(prev => [...prev, newOutput]);
    };
    
    const executeCommand = (command: string) => {
        if (!command.trim()) return;
        
        // Add command to history
        setCommandHistory(prev => [...prev, command]);
        setHistoryIndex(-1);
        
        // Add command to output
        addOutput(`PS H:\\dev4\\mikolite> ${command}`, 'command');
        
        // Notify parent
        props.onCommand(props.id, command);
        props.onStatusChange(props.id, 'running');
        
        // Simulate command execution
        setTimeout(() => {
            // Mock command responses
            switch (command.toLowerCase().trim()) {
                case 'clear':
                case 'cls':
                    setOutput([]);
                    break;
                case 'help':
                    addOutput('Available commands:', 'info');
                    addOutput('  clear, cls    - Clear terminal', 'info');
                    addOutput('  help          - Show this help', 'info');
                    addOutput('  ls, dir       - List directory contents', 'info');
                    addOutput('  pwd           - Show current directory', 'info');
                    addOutput('  echo <text>   - Echo text', 'info');
                    break;
                case 'pwd':
                    addOutput('H:\\dev4\\mikolite', 'output');
                    break;
                case 'ls':
                case 'dir':
                    addOutput('Directory: H:\\dev4\\mikolite', 'info');
                    addOutput('', 'output');
                    addOutput('Mode                 LastWriteTime         Length Name', 'output');
                    addOutput('----                 -------------         ------ ----', 'output');
                    addOutput('d-----        12/15/2024   2:30 PM                mikoide', 'output');
                    addOutput('d-----        12/15/2024   2:30 PM                mikoshell', 'output');
                    addOutput('-a----        12/15/2024   2:30 PM           1234 package.json', 'output');
                    addOutput('-a----        12/15/2024   2:30 PM           5678 README.md', 'output');
                    break;
                default:
                    if (command.toLowerCase().startsWith('echo ')) {
                        const text = command.substring(5);
                        addOutput(text, 'output');
                    } else {
                        addOutput(`'${command}' is not recognized as an internal or external command,`, 'error');
                        addOutput('operable program or batch file.', 'error');
                        props.onStatusChange(props.id, 'error');
                        setTimeout(() => props.onStatusChange(props.id, 'idle'), 2000);
                        return;
                    }
            }
            
            props.onStatusChange(props.id, 'idle');
        }, Math.random() * 1000 + 500); // Random delay between 500-1500ms
    };
    
    const handleKeyDown = (e: KeyboardEvent) => {
        switch (e.key) {
            case 'Enter':
                executeCommand(currentInput());
                setCurrentInput('');
                break;
            case 'ArrowUp':
                e.preventDefault();
                const history = commandHistory();
                if (history.length > 0) {
                    const newIndex = historyIndex() === -1 ? history.length - 1 : Math.max(0, historyIndex() - 1);
                    setHistoryIndex(newIndex);
                    setCurrentInput(history[newIndex]);
                }
                break;
            case 'ArrowDown':
                e.preventDefault();
                const hist = commandHistory();
                if (historyIndex() !== -1) {
                    const newIndex = historyIndex() + 1;
                    if (newIndex >= hist.length) {
                        setHistoryIndex(-1);
                        setCurrentInput('');
                    } else {
                        setHistoryIndex(newIndex);
                        setCurrentInput(hist[newIndex]);
                    }
                }
                break;
            case 'Tab':
                e.preventDefault();
                // Basic tab completion could be implemented here
                break;
        }
    };
    
    const clearTerminal = () => {
        setOutput([]);
    };
    
    const copyOutput = () => {
        const text = output().map(o => o.content).join('\n');
        navigator.clipboard.writeText(text);
    };
    
    const getOutputClass = (type: TerminalOutput['type']) => {
        switch (type) {
            case 'command': return 'text-blue-300';
            case 'error': return 'text-red-400';
            case 'info': return 'text-gray-300';
            default: return 'text-green-400';
        }
    };
    
    onMount(() => {
        if (inputRef) {
            inputRef.focus();
        }
    });
    
    return (
        <div class="flex flex-col h-full bg-black text-green-400 font-mono text-sm overflow-hidden">
            {/* Terminal Output */}
            <div 
                ref={terminalRef}
                class="flex-1 p-4 overflow-auto scrollbar-thin scrollbar-track-neutral-800 scrollbar-thumb-neutral-600 hover:scrollbar-thumb-neutral-500"
                onClick={() => inputRef?.focus()}
            >
                <For each={output()}>
                    {(line) => (
                        <div class={`mb-1 whitespace-pre-wrap break-words ${getOutputClass(line.type)}`}>
                            {line.content}
                        </div>
                    )}
                </For>
                
                {/* Current Input Line */}
                <div class="flex items-center">
                    <span class="mr-2 text-blue-400 select-none">PS H:\dev4\mikolite&gt;</span>
                    <input 
                        ref={inputRef}
                        type="text" 
                        class="bg-transparent border-none outline-none flex-1 text-green-400 font-mono" 
                        placeholder={props.status === 'running' ? 'Command running...' : 'Enter command...'}
                        value={currentInput()}
                        disabled={props.status === 'running'}
                        onInput={(e) => setCurrentInput(e.currentTarget.value)}
                        onKeyDown={handleKeyDown}
                        autocomplete="off"
                        spellcheck={false}
                    />
                </div>
                
                {/* Running Indicator */}
                {props.status === 'running' && (
                    <div class="flex items-center mt-2 text-yellow-400">
                        <div class="animate-spin mr-2">⟳</div>
                        <span class="text-xs">Executing command...</span>
                    </div>
                )}
            </div>
            
            {/* Terminal Actions */}
            <div class="flex items-center justify-between px-4 py-1 bg-neutral-800 border-t border-neutral-700">
                <div class="flex items-center gap-4 text-xs text-gray-400">
                    <span>Shell: PowerShell</span>
                    <span>Encoding: UTF-8</span>
                    <span>Lines: {output().length}</span>
                </div>
                
                <div class="flex items-center gap-1">
                    <button 
                        class="p-1.5 text-gray-400 hover:text-white hover:bg-neutral-700 rounded transition-colors"
                        onClick={copyOutput}
                        title="Copy Output"
                    >
                        <Copy size={12} />
                    </button>
                    <button 
                        class="p-1.5 text-gray-400 hover:text-white hover:bg-neutral-700 rounded transition-colors"
                        onClick={clearTerminal}
                        title="Clear Terminal"
                    >
                        <RotateCcw size={12} />
                    </button>
                    {props.status === 'running' && (
                        <button 
                            class="p-1.5 text-red-400 hover:text-red-300 hover:bg-neutral-700 rounded transition-colors"
                            title="Stop Process"
                        >
                            <Square size={12} />
                        </button>
                    )}
                </div>
            </div>
        </div>
    );
}

export default TerminalInstance;