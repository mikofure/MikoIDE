import {
  Files,
  Search,
  GitBranch,
  Package,
  Settings,
  Sun,
  Moon,
  Menu,
} from "lucide-react";
import { useIDEStore } from "@/store/ide-store";
import { FileExplorer } from "./FileExplorer";
import { cn } from "@/lib/utils";

const sidebarPanels = [
  { id: "explorer" as const, icon: Files, label: "Explorer" },
  { id: "search" as const, icon: Search, label: "Search" },
  { id: "git" as const, icon: GitBranch, label: "Source Control" },
  { id: "extensions" as const, icon: Package, label: "Extensions" },
];

export const IDESidebar = () => {
  const {
    sidebarCollapsed,
    activePanel,
    theme,
    setSidebarCollapsed,
    setActivePanel,
    toggleTheme,
  } = useIDEStore();

  const renderPanelContent = () => {
    switch (activePanel) {
      case "explorer":
        return <FileExplorer />;
      case "search":
        return (
          <div className="p-4 text-center text-muted-foreground">
            <Search size={48} className="mx-auto mb-4 opacity-50" />
            <p>Search functionality coming soon...</p>
          </div>
        );
      case "git":
        return (
          <div className="p-4 text-center text-muted-foreground">
            <GitBranch size={48} className="mx-auto mb-4 opacity-50" />
            <p>Git integration coming soon...</p>
          </div>
        );
      case "extensions":
        return (
          <div className="p-4 text-center text-muted-foreground">
            <Package size={48} className="mx-auto mb-4 opacity-50" />
            <p>Extensions marketplace coming soon...</p>
          </div>
        );
      default:
        return null;
    }
  };

  return (
    <div className="flex h-full bg-sidebar dark:bg-sidebar border-r border-sidebar-border">
      {/* Activity Bar */}
      <div className="w-10 bg-editor-sidebar dark:bg-editor-sidebar border-r border-editor-border flex flex-col">
        {/* Panel Icons */}
        <div className="flex-1">
          {sidebarPanels.map((panel) => (
            <button
              key={panel.id}
              onClick={() => {
                if (activePanel === panel.id) {
                  setSidebarCollapsed(!sidebarCollapsed);
                } else {
                  setActivePanel(panel.id);
                  setSidebarCollapsed(false);
                }
              }}
              className={cn(
                "w-full h-8 flex items-center justify-center transition-colors group",
                "hover:bg-sidebar-accent",
                activePanel === panel.id && !sidebarCollapsed
                  ? "bg-sidebar-accent text-sidebar-accent-foreground border-r-2 border-r-editor-accent"
                  : "text-muted-foreground hover:text-foreground",
              )}
              title={panel.label}
            >
              <panel.icon size={16} />
            </button>
          ))}
        </div>

        {/* Bottom Actions */}
        <div className="border-t border-editor-border">
          <button
            onClick={toggleTheme}
            className="w-full h-12 flex items-center justify-center text-muted-foreground hover:text-foreground hover:bg-sidebar-accent transition-colors"
            title={`Switch to ${theme === "dark" ? "light" : "dark"} theme`}
          >
            {theme === "dark" ? <Sun size={16} /> : <Moon size={16} />}
          </button>

          <button
            onClick={() => setSidebarCollapsed(!sidebarCollapsed)}
            className="w-full h-12 flex items-center justify-center text-muted-foreground hover:text-foreground hover:bg-sidebar-accent transition-colors"
            title="Toggle Sidebar"
          >
            <Menu size={16} />
          </button>
        </div>
      </div>

      {/* Panel Content */}
      {activePanel && (
        <div className="flex-1 bg-sidebar flex flex-col min-w-0">
          {renderPanelContent()}
        </div>
      )}
    </div>
  );
};
