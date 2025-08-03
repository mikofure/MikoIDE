import { 
  X, 
  FileText, 
  FileCode2, 
  Palette, 
  FileJson, 
  Settings, 
  File,
  Plus
} from "lucide-react";
import { useIDEStore } from "@/store/ide-store";
import { cn } from "@/lib/utils";

export const TabBar = () => {
  const { tabs, activeTabId, setActiveTab, removeTab, theme } = useIDEStore();
  
  if (tabs.length === 0) {
    return (
      <div className="relative bg-editor-panel dark:bg-editor-panel border-b border-editor-border/50 h-10 select-none [&]:drag" />
    );
  }

  return (
    <div className="relative bg-editor-panel dark:bg-editor-panel border-b border-editor-border/50 select-none [&]:drag">
      {/* Floating tab container */}
      <div className="flex items-center gap-1 px-3 py-[5px] overflow-x-auto scrollbar-hide pr-32 [&]:drag">
        {tabs.map((tab, index) => (
          <div
            key={tab.id}
            className={cn(
              "relative flex items-center gap-2 px-3 py-1.5 text-sm cursor-pointer group min-w-0 max-w-48 transition-all duration-200 ease-out",
              "rounded-lg backdrop-blur-sm",
              "translate-y-0 [&[data-active=true]]:translate-y-[-1px] [&[data-active=true]]:no-drag",
              activeTabId === tab.id
                ? "bg-editor-bg shadow-lg shadow-black/10 text-foreground scale-[1.02] z-10 border border-editor-border/30"
                : "bg-editor-panel/80 text-muted-foreground hover:bg-editor-bg/60 hover:scale-[1.01] hover:shadow-md hover:shadow-black/5 border border-transparent hover:border-editor-border/20"
            )}
            onClick={() => setActiveTab(tab.id)}
            data-active={activeTabId === tab.id}
          >
            {/* Floating active indicator */}
            {activeTabId === tab.id && (
              <>
                {/* Top glow line */}
                <div className="absolute -top-0.5 left-2 right-2 h-0.5 bg-gradient-to-r from-transparent via-editor-accent to-transparent rounded-full opacity-80" />
                {/* Bottom shadow */}
                <div className="absolute -bottom-1 left-1 right-1 h-2 bg-gradient-to-b from-black/10 to-transparent rounded-lg blur-sm" />
              </>
            )}
            
            <div className="flex items-center gap-2 min-w-0 flex-1 relative z-10">
              {/* File icon with Lucide icons */}
              <div className={cn(
                "transition-all duration-200 flex-shrink-0",
                activeTabId === tab.id ? "scale-110 text-editor-accent" : "scale-100 text-muted-foreground"
              )}>
                {tab.name.endsWith('.md') || tab.name.endsWith('.mdx') ? 
                  <FileText size={14} /> :
                 tab.name.endsWith('.tsx') || tab.name.endsWith('.ts') || tab.name.endsWith('.jsx') || tab.name.endsWith('.js') ? 
                  <FileCode2 size={14} /> :
                 tab.name.endsWith('.css') || tab.name.endsWith('.scss') || tab.name.endsWith('.sass') ? 
                  <Palette size={14} /> :
                 tab.name.endsWith('.json') ? 
                  <FileJson size={14} /> :
                 tab.name.endsWith('.cpp') || tab.name.endsWith('.c') || tab.name.endsWith('.h') ? 
                  <Settings size={14} /> :
                 <File size={14} />}
              </div>
              
              <span className={cn(
                "truncate text-xs font-medium transition-all duration-200",
                activeTabId === tab.id ? "font-semibold" : "font-normal"
              )}>
                {tab.name}
              </span>
              
              {/* Dirty indicator with animation */}
              {tab.isDirty && (
                <div className={cn(
                  "w-1.5 h-1.5 rounded-full flex-shrink-0 transition-all duration-200",
                  "bg-gradient-to-r from-editor-accent to-editor-accent/80",
                  "animate-pulse shadow-sm shadow-editor-accent/50"
                )} />
              )}
            </div>
            
            {/* Close button with Fleet-style design */}
            <button
              onClick={(e) => {
                e.stopPropagation();
                removeTab(tab.id);
              }}
              className={cn(
                "relative transition-all duration-200 rounded-md p-1 ml-1",
                "opacity-0 group-hover:opacity-100 scale-90 group-hover:scale-100",
                "hover:bg-red-500/10 hover:text-red-400",
                "before:absolute before:inset-0 before:rounded-md before:bg-red-500/0 before:transition-colors before:duration-200",
                "hover:before:bg-red-500/10",
                activeTabId === tab.id && "opacity-60 hover:opacity-100"
              )}
              aria-label={`Close ${tab.name}`}
              style={{ WebkitAppRegion: 'no-drag' } as React.CSSProperties}
            >
              <X size={11} className="relative z-10" />
            </button>
          </div>
        ))}
        
        {/* Add new tab button (Fleet-style) */}
        <button className={cn(
          "flex items-center justify-center w-8 h-8 ml-2 rounded-lg transition-all duration-200",
          "text-muted-foreground hover:text-foreground",
          "hover:bg-editor-bg/60 hover:scale-105",
          "border border-transparent hover:border-editor-border/20",
          "no-drag"
        )}>
          <svg width="12" height="12" viewBox="0 0 12 12" className="fill-current">
            <path d="M6.5 1.5v4h4v1h-4v4h-1v-4h-4v-1h4v-4h1z"/>
          </svg>
        </button>
      </div>
      
      {/* Background blur effect */}
      <div className="absolute inset-0 bg-gradient-to-r from-editor-panel/95 via-editor-panel/90 to-editor-panel/95 backdrop-blur-sm -z-10" />
    </div>
  );
};