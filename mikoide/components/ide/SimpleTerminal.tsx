import { useEffect, useRef, useState } from 'react';
import { themeColors } from '@/lib/theme';

interface TerminalLine {
  content: string;
  isCommand?: boolean;
}

export const SimpleTerminal = () => {
  const [lines, setLines] = useState<TerminalLine[]>([{ content: 'Welcome to MikoIDE Terminal' }]);
  const [currentInput, setCurrentInput] = useState('');
  const [commandHistory, setCommandHistory] = useState<string[]>([]);
  const [historyIndex, setHistoryIndex] = useState(-1);
  const terminalRef = useRef<HTMLDivElement>(null);
  const inputRef = useRef<HTMLInputElement>(null);

  const prompt = '$ ';
  
  const handleCommand = (command: string) => {
    switch (command.trim()) {
      case 'clear':
        setLines([]);
        break;
      case 'help':
        setLines(prev => [...prev, 
          { content: 'Available commands:', isCommand: false },
          { content: '  clear - Clear terminal', isCommand: false },
          { content: '  help - Show this help', isCommand: false },
          { content: '  echo <text> - Print text', isCommand: false },
        ]);
        break;
      default:
        if (command.startsWith('echo ')) {
          setLines(prev => [...prev, { content: command.slice(5), isCommand: false }]);
        } else if (command.trim() !== '') {
          setLines(prev => [...prev, { content: `Command not found: ${command}`, isCommand: false }]);
        }
    }
  };

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter') {
      const command = currentInput;
      setLines(prev => [...prev, { content: prompt + command, isCommand: true }]);
      setCommandHistory(prev => [...prev, command]);
      setHistoryIndex(-1);
      handleCommand(command);
      setCurrentInput('');
    } else if (e.key === 'ArrowUp') {
      e.preventDefault();
      if (historyIndex < commandHistory.length - 1) {
        const newIndex = historyIndex + 1;
        setHistoryIndex(newIndex);
        setCurrentInput(commandHistory[commandHistory.length - 1 - newIndex]);
      }
    } else if (e.key === 'ArrowDown') {
      e.preventDefault();
      if (historyIndex > 0) {
        const newIndex = historyIndex - 1;
        setHistoryIndex(newIndex);
        setCurrentInput(commandHistory[commandHistory.length - 1 - newIndex]);
      } else if (historyIndex === 0) {
        setHistoryIndex(-1);
        setCurrentInput('');
      }
    }
  };

  useEffect(() => {
    terminalRef.current?.scrollTo(0, terminalRef.current.scrollHeight);
    inputRef.current?.focus();
  }, [lines, currentInput]);

  return (
    <div 
      className="w-full h-full bg-black font-mono text-sm p-2 overflow-auto"
      style={{ backgroundColor: themeColors.dark.terminal.bg }}
      onClick={() => inputRef.current?.focus()}
      ref={terminalRef}
    >
      {lines.map((line, i) => (
        <div 
          key={i} 
          style={{ 
            color: line.isCommand ? themeColors.dark.terminal.green : themeColors.dark.terminal.fg 
          }}
        >
          {line.content}
        </div>
      ))}
      <div className="flex items-center">
        <span style={{ color: themeColors.dark.terminal.green }}>{prompt}</span>
        <input
          ref={inputRef}
          type="text"
          value={currentInput}
          onChange={(e) => setCurrentInput(e.target.value)}
          onKeyDown={handleKeyDown}
          className="flex-1 bg-transparent outline-none border-none ml-1"
          style={{ color: themeColors.dark.terminal.fg }}
          autoFocus
        />
      </div>
    </div>
  );
};
