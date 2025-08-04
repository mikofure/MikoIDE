import { GitBranch, Zap } from "lucide-solid";

interface StatusBarProps {
  line: number;
  col: number;
  language: string;
  gitBranch: string;
  wordCount: number;
  charCount: number;
}

function StatusBar(props: StatusBarProps) {
  return (
    <div class="h-6 text-[11px] text-gray-300 flex justify-between items-center px-3 select-none">
      {/* ฝั่งซ้าย */}
      <div class="flex items-center gap-4">
        <span>Ln {props.line}, Col {props.col}</span>
        <span>{props.language}</span>
        <span>{props.wordCount} words, {props.charCount} chars</span>
      </div>

      {/* ฝั่งขวา */}
      <div class="flex items-center gap-4">
        <div class="flex items-center gap-1">
          <Zap size={14} class="text-yellow-400" />
          <span>Prettier</span>
        </div>
        <div class="flex items-center gap-1">
          <GitBranch size={14} class="text-green-400" />
          <span>{props.gitBranch}</span>
        </div>
      </div>
    </div>
  );
}

export default StatusBar;
