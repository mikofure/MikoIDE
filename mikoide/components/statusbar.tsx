import { GitBranch, Zap, Bug, AlertCircle, CheckCircle, GitCommit, Wifi, WifiOff, Activity } from "lucide-solid";

interface StatusBarProps {
  line: number;
  col: number;
  language: string;
  gitBranch: string;
  wordCount: number;
  charCount: number;
  gitStatus?: {
    hasChanges: boolean;
    ahead: number;
    behind: number;
    staged: number;
    unstaged: number;
  };
  debugMode?: boolean;
  errors?: number;
  warnings?: number;
  isOnline?: boolean;
  lspStatus?: 'connected' | 'disconnected' | 'loading';
  encoding?: string;
  eol?: 'LF' | 'CRLF';
}

function StatusBar(props: StatusBarProps) {
  const getGitStatusColor = () => {
    if (!props.gitStatus) return "text-green-400";
    if (props.gitStatus.hasChanges || props.gitStatus.unstaged > 0) return "text-orange-400";
    if (props.gitStatus.ahead > 0) return "text-blue-400";
    return "text-green-400";
  };

  const getLspStatusIcon = () => {
    switch (props.lspStatus) {
      case 'connected': return <CheckCircle size={12} class="text-green-400" />;
      case 'loading': return <Activity size={12} class="text-yellow-400 animate-pulse" />;
      case 'disconnected': return <AlertCircle size={12} class="text-red-400" />;
      default: return null;
    }
  };

  return (
    <div class="h-6 text-[11px] text-gray-300 flex justify-between items-center px-3 select-none">
      {/* Left side */}
      <div class="flex items-center gap-4">
        <span>Ln {props.line}, Col {props.col}</span>
        <span>{props.language}</span>
        <span>{props.wordCount} words, {props.charCount} chars</span>
        
        {/* Encoding and EOL */}
        {props.encoding && <span class="text-gray-400">{props.encoding}</span>}
        {props.eol && <span class="text-gray-400">{props.eol}</span>}
        
        {/* Error and Warning indicators */}
        {(props.errors || 0) > 0 && (
          <div class="flex items-center gap-1 text-red-400">
            <AlertCircle size={12} />
            <span>{props.errors}</span>
          </div>
        )}
        {(props.warnings || 0) > 0 && (
          <div class="flex items-center gap-1 text-yellow-400">
            <AlertCircle size={12} />
            <span>{props.warnings}</span>
          </div>
        )}
      </div>

      {/* Right side */}
      <div class="flex items-center gap-4">
        {/* Debug mode indicator */}
        {props.debugMode && (
          <div class="flex items-center gap-1 text-red-400">
            <Bug size={14} />
            <span>Debug</span>
          </div>
        )}
        
        {/* LSP Status */}
        {props.lspStatus && (
          <div class="flex items-center gap-1">
            {getLspStatusIcon()}
            <span class="text-xs">LSP</span>
          </div>
        )}
        
        {/* Online/Offline status */}
        <div class="flex items-center gap-1">
          {props.isOnline ? (
            <Wifi size={12} class="text-green-400" />
          ) : (
            <WifiOff size={12} class="text-red-400" />
          )}
        </div>
        
        {/* Prettier */}
        <div class="flex items-center gap-1">
          <Zap size={14} class="text-yellow-400" />
          <span>Prettier</span>
        </div>
        
        {/* Git status with enhanced information */}
        <div class="flex items-center gap-1">
          <GitBranch size={14} class={getGitStatusColor()} />
          <span class={getGitStatusColor()}>{props.gitBranch}</span>
          
          {/* Git status indicators */}
          {props.gitStatus && (
            <div class="flex items-center gap-1 ml-1">
              {props.gitStatus.staged > 0 && (
                <span class="text-green-400 text-xs">+{props.gitStatus.staged}</span>
              )}
              {props.gitStatus.unstaged > 0 && (
                <span class="text-orange-400 text-xs">~{props.gitStatus.unstaged}</span>
              )}
              {props.gitStatus.ahead > 0 && (
                <span class="text-blue-400 text-xs">↑{props.gitStatus.ahead}</span>
              )}
              {props.gitStatus.behind > 0 && (
                <span class="text-red-400 text-xs">↓{props.gitStatus.behind}</span>
              )}
              {props.gitStatus.hasChanges && (
                <GitCommit size={12} class="text-orange-400" />
              )}
            </div>
          )}
        </div>
      </div>
    </div>
  );
}

export default StatusBar;
