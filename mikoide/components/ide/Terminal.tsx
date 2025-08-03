import { useEffect, useRef, useState } from 'react';
import { Terminal as TerminalIcon, X, Minus, Square, ChevronDown, Plus, Columns2, Trash } from 'lucide-react';
import { useIDEStore } from '@/store/ide-store';
import { cn } from '@/lib/utils';
import { Terminal } from 'xterm';
import { FitAddon } from '@xterm/addon-fit';
import { WebLinksAddon } from '@xterm/addon-web-links';

export const TerminalComponent = () => {
  const { terminalVisible, setTerminalVisible } = useIDEStore();
  const terminalRef = useRef<HTMLDivElement>(null);
  const [terminal, setTerminal] = useState<Terminal | null>(null);
  const [terminalId, setTerminalId] = useState<string | null>(null);
  const fitAddon = useRef<FitAddon>(new FitAddon());

  useEffect(() => {
    if (!terminalRef.current || !terminalVisible) return;

    // Initialize xterm.js terminal
    const term = new Terminal({
      theme: {
        background: '#1e1e1e',
        foreground: '#d4d4d4',
        cursor: '#ffffff',
        black: '#000000',
        red: '#cd3131',
        green: '#0dbc79',
        yellow: '#e5e510',
        blue: '#2472c8',
        magenta: '#bc3fbc',
        cyan: '#11a8cd',
        white: '#e5e5e5',
        brightBlack: '#666666',
        brightRed: '#f14c4c',
        brightGreen: '#23d18b',
        brightYellow: '#f5f543',
        brightBlue: '#3b8eea',
        brightMagenta: '#d670d6',
        brightCyan: '#29b8db',
        brightWhite: '#e5e5e5'
      },
      fontSize: 14,
      fontFamily: 'Consolas, "Courier New", monospace',
      cursorBlink: true,
      allowTransparency: true,
      rows: 24,
      cols: 80
    });

    // Add addons
    term.loadAddon(fitAddon.current);
    term.loadAddon(new WebLinksAddon());

    // Open terminal in the DOM
    term.open(terminalRef.current);
    
    // Fit terminal to container
    setTimeout(() => {
      fitAddon.current.fit();
    }, 100);

    setTerminal(term);

    // Create terminal process via Electron IPC
    if (window.electronAPI) {
      window.electronAPI.terminal.create().then((id: string) => {
        setTerminalId(id);
        
        // Listen for data from terminal process
        window.electronAPI.terminal.onData(id, (data: string) => {
          term.write(data);
        });

        // Listen for terminal exit
        window.electronAPI.terminal.onExit(id, (code: number) => {
          term.write(`\r\n\r\nProcess exited with code ${code}\r\n`);
        });
      });
    }

    // Handle user input
    term.onData((data) => {
      if (terminalId && window.electronAPI) {
        window.electronAPI.terminal.write(terminalId, data);
      }
    });

    // Handle resize
    const handleResize = () => {
      if (fitAddon.current && term) {
        fitAddon.current.fit();
        if (terminalId && window.electronAPI) {
          const { cols, rows } = term;
          window.electronAPI.terminal.resize(terminalId, cols, rows);
        }
      }
    };

    window.addEventListener('resize', handleResize);

    return () => {
      window.removeEventListener('resize', handleResize);
      if (terminalId && window.electronAPI) {
        window.electronAPI.terminal.kill(terminalId);
      }
      term.dispose();
    };
  }, [terminalVisible]);

  const handleNewTerminal = () => {
    // Logic to create a new terminal tab
    console.log('New terminal requested');
  };

  const handleSplitTerminal = () => {
    // Logic to split terminal
    console.log('Split terminal requested');
  };

  const handleKillTerminal = () => {
    if (terminalId && window.electronAPI) {
      window.electronAPI.terminal.kill(terminalId);
    }
  };

  const handleMinimizeTerminal = () => {
    setTerminalVisible(false);
  };

  if (!terminalVisible) {
    return null;
  }

  return (
    <div className="flex flex-col h-full bg-terminal">
      {/* Terminal Header - Keep existing UI */}
      <div className="flex items-center justify-between px-3 py-1">
        <div className="flex items-center gap-2">
          <TerminalIcon size={14} className="text-terminal-foreground" />
          <span className="text-sm font-medium text-terminal-foreground">Terminal</span>
        </div>
        <div className="flex items-center gap-1">
          <div className='flex items-center space-x-1 pr-2'>
            <button className='text-xs font-semibold bg-[#272b33] text-terminal-foreground px-2 py-1 rounded-full'>
              <p>Powershell</p>
            </button>
            <div className='flex items-center'>
              <button 
                onClick={handleNewTerminal}
                className='text-xs font-semibold bg-[#272b33] text-terminal-foreground px-2 py-1 rounded-l-full border-r flex items-center space-x-1 hover:bg-[#3a3f47] transition-colors'
              >
                <Plus size={16} />
                <p>Add</p>
              </button>
              <button className='text-xs font-semibold bg-[#272b33] text-terminal-foreground px-2 py-1 rounded-r-full hover:bg-[#3a3f47] transition-colors'>
                <ChevronDown size={16} />
              </button>
            </div>
          </div>
          <div className='flex items-center space-x-1'>
            <button 
              onClick={handleSplitTerminal}
              className='p-1 hover:bg-terminal-hover rounded transition-colors text-terminal-foreground'
              title="Split Terminal"
            >
              <Columns2 size={16} />
            </button>
            <button 
              onClick={handleKillTerminal}
              className='p-1 hover:bg-terminal-hover rounded transition-colors text-terminal-foreground'
              title="Kill Terminal"
            >
              <Trash size={16} />
            </button>
            <button 
              onClick={handleMinimizeTerminal}
              className='p-1 hover:bg-terminal-hover rounded transition-colors text-terminal-foreground'
              title="Minimize Terminal"
            >
              <Minus size={16} />
            </button>
          </div>
          <button
            onClick={() => setTerminalVisible(false)}
            className="p-1 hover:bg-terminal-hover rounded transition-colors text-terminal-foreground"
            title="Close Terminal"
          >
            <X size={14} />
          </button>
        </div>
      </div>

      {/* Terminal Content - Replace with xterm.js */}
      <div className="flex-1 min-h-0 p-3">
        <div 
          ref={terminalRef} 
          className="w-full h-full"
          style={{ minHeight: '300px' }}
        />
      </div>
    </div>
  );
};

// Export as Terminal to maintain compatibility
export { TerminalComponent as Terminal };