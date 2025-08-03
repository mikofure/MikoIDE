import { useEffect, useRef, useState } from 'react';
import { Terminal as TerminalIcon, X, Minus, Square, ChevronDown, Plus, Columns2, Trash } from 'lucide-react';
import { useIDEStore } from '@/store/ide-store';
import { cn } from '@/lib/utils';
import { themeColors } from '@/lib/theme';
import { SimpleTerminal } from './SimpleTerminal';
import { Terminal } from 'xterm';

export const TerminalComponent = () => {
  const { terminalVisible, setTerminalVisible } = useIDEStore();
  const terminalRef = useRef<HTMLDivElement>(null);
  const [terminal, setTerminal] = useState<Terminal | null>(null);

  useEffect(() => {
    if (!terminalRef.current || !terminalVisible) return;

    const theme = window.matchMedia('(prefers-color-scheme: dark)').matches ? 
      themeColors.dark.terminal : 
      themeColors.light.terminal;

    // Initialize xterm.js terminal
    const term = new Terminal({
      theme: {
        background: theme.bg,
        foreground: theme.fg,
        cursor: theme.cursor,
        cursorAccent: theme.cursorAccent,
        black: theme.black,
        red: theme.red,
        green: theme.green,
        yellow: theme.yellow,
        blue: theme.blue,
        magenta: theme.magenta,
        cyan: theme.cyan,
        white: theme.white,
        brightBlack: theme.brightBlack,
        brightRed: theme.brightRed,
        brightGreen: theme.brightGreen,
        brightYellow: theme.brightYellow,
        brightBlue: theme.brightBlue,
        brightMagenta: theme.brightMagenta,
        brightCyan: theme.brightCyan,
        brightWhite: theme.brightWhite
      },
      fontSize: 14,
      fontFamily: 'JetBrains Mono, Consolas, monospace',
      lineHeight: 1.5,
      cursorBlink: true,
      cursorStyle: 'block',
      allowTransparency: true,
      rows: 24,
      cols: 80,
      scrollback: 5000,
      fastScrollModifier: 'alt',
      fastScrollSensitivity: 5,
    });

    // Open terminal and focus
    term.open(terminalRef.current);
    term.focus();
    
    // Initial fit after DOM is ready
    requestAnimationFrame(() => {
      term.write('\x1b[1;36mWelcome to MikoIDE Terminal\x1b[0m\r\n\$ ');
    });

    // Handle user input with command simulation
    term.onData((data) => {
      term.write(data);
      if (data === '\r') {
        term.write('\n\$ ');
      }
    });

    // Handle resize with debounce
    let resizeTimeout: NodeJS.Timeout;
    const handleResize = () => {
      clearTimeout(resizeTimeout);
      resizeTimeout = setTimeout(() => {
        if (term) {
          //@ts-expect-error
          term.fit();
        }
      }, 100);
    };

    window.addEventListener('resize', handleResize);
    
    // Clean up
    return () => {
      clearTimeout(resizeTimeout);
      window.removeEventListener('resize', handleResize);
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
    if (terminal) {
      terminal.clear();
    }
  };

  const handleMinimizeTerminal = () => {
    setTerminalVisible(false);
  };

  if (!terminalVisible) {
    return null;
  }

  return (
    <div className={`flex flex-col h-full bg-[${themeColors.dark.terminal.bg}]`}>
      <div className={`flex items-center justify-between px-3 py-1 bg-[${themeColors.dark.terminal.toolbar}]`}>
        <div className="flex items-center gap-2">
          <TerminalIcon size={14} className={`text-[${themeColors.light.terminal.fg}] dark:text-[${themeColors.dark.terminal.fg}]`} />
          <span className={`text-sm font-medium text-[${themeColors.light.terminal.fg}] dark:text-[${themeColors.dark.terminal.fg}]`}>Terminal</span>
        </div>
        <div className="flex items-center gap-1">
          <div className='flex items-center space-x-1 pr-2'>
            <button className={`text-xs font-semibold 
              bg-[${themeColors.light.terminal.button}] dark:bg-[${themeColors.dark.terminal.button}]
              text-[${themeColors.light.terminal.fg}] dark:text-[${themeColors.dark.terminal.fg}]
              hover:bg-[${themeColors.light.terminal.buttonHover}] dark:hover:bg-[${themeColors.dark.terminal.buttonHover}]
              px-2 py-1 rounded-full transition-colors`}>
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

      <div className="flex-1 min-h-0 p-3 relative">
        <div className="w-full h-full absolute inset-0 overflow-hidden">
          <SimpleTerminal />
        </div>
      </div>
    </div>
  );
};

// Export as Terminal to maintain compatibility
export { TerminalComponent as Terminal };