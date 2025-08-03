import { useState } from "react";
import {
  Folder,
  FolderOpen,
  File,
  FileText,
  FileCode,
  FileImage,
  FileJson,
  ChevronRight,
  ChevronDown,
} from "lucide-react";
import { useIDEStore } from "@/store/ide-store";
import { cn } from "@/lib/utils";

interface FileNode {
  id: string;
  name: string;
  type: "file" | "folder";
  path: string;
  children?: FileNode[];
  language?: string;
}

// Mock file structure
const mockFileStructure: FileNode[] = [
  {
    id: "1",
    name: "src",
    type: "folder",
    path: "/src",
    children: [
      {
        id: "2",
        name: "components",
        type: "folder",
        path: "/src/components",
        children: [
          {
            id: "3",
            name: "Button.tsx",
            type: "file",
            path: "/src/components/Button.tsx",
            language: "typescript",
          },
          {
            id: "4",
            name: "Modal.tsx",
            type: "file",
            path: "/src/components/Modal.tsx",
            language: "typescript",
          },
        ],
      },
      {
        id: "5",
        name: "pages",
        type: "folder",
        path: "/src/pages",
        children: [
          {
            id: "6",
            name: "Home.tsx",
            type: "file",
            path: "/src/pages/Home.tsx",
            language: "typescript",
          },
          {
            id: "7",
            name: "About.tsx",
            type: "file",
            path: "/src/pages/About.tsx",
            language: "typescript",
          },
        ],
      },
      {
        id: "8",
        name: "App.tsx",
        type: "file",
        path: "/src/App.tsx",
        language: "typescript",
      },
      {
        id: "9",
        name: "main.tsx",
        type: "file",
        path: "/src/main.tsx",
        language: "typescript",
      },
      {
        id: "10",
        name: "index.css",
        type: "file",
        path: "/src/index.css",
        language: "css",
      },
    ],
  },
  {
    id: "11",
    name: "public",
    type: "folder",
    path: "/public",
    children: [
      {
        id: "12",
        name: "vite.svg",
        type: "file",
        path: "/public/vite.svg",
        language: "svg",
      },
      {
        id: "13",
        name: "favicon.ico",
        type: "file",
        path: "/public/favicon.ico",
        language: "binary",
      },
    ],
  },
  {
    id: "14",
    name: "package.json",
    type: "file",
    path: "/package.json",
    language: "json",
  },
  {
    id: "15",
    name: "README.md",
    type: "file",
    path: "/README.md",
    language: "markdown",
  },
  {
    id: "16",
    name: "vite.config.ts",
    type: "file",
    path: "/vite.config.ts",
    language: "typescript",
  },
];

const getFileIcon = (fileName: string, language?: string) => {
  const ext = fileName.split(".").pop()?.toLowerCase();

  switch (ext) {
    case "tsx":
    case "ts":
    case "js":
    case "jsx":
      return <FileCode size={16} className="text-blue-400" />;
    case "json":
      return <FileJson size={16} className="text-yellow-400" />;
    case "md":
      return <FileText size={16} className="text-gray-400" />;
    case "css":
    case "scss":
    case "sass":
      return <FileCode size={16} className="text-purple-400" />;
    case "png":
    case "jpg":
    case "jpeg":
    case "gif":
    case "svg":
      return <FileImage size={16} className="text-green-400" />;
    default:
      return <File size={16} className="text-gray-400" />;
  }
};

interface FileTreeNodeProps {
  node: FileNode;
  level: number;
}

const FileTreeNode = ({ node, level }: FileTreeNodeProps) => {
  const [isExpanded, setIsExpanded] = useState(level < 2); // Auto-expand first two levels
  const { addTab } = useIDEStore();

  const handleClick = () => {
    if (node.type === "folder") {
      setIsExpanded(!isExpanded);
    } else {
      // Open file in editor
      const sampleContent = `// ${node.name}
// This is a sample file content for demonstration

export default function ${node.name.split(".")[0]}() {
  return (
    <div>
      <h1>Hello from ${node.name}</h1>
    </div>
  );
}`;

      addTab({
        name: node.name,
        path: node.path,
        content: sampleContent,
        language: node.language || "typescript",
        isDirty: false,
      });
    }
  };

  return (
    <>
      <div
        className={cn(
          "flex items-center gap-1 py-1 px-2 hover:bg-sidebar-accent cursor-pointer text-xs transition-colors",
          "group",
        )}
        style={{ paddingLeft: `${level * 12 + 8}px` }}
        onClick={handleClick}
      >
        {node.type === "folder" && (
          <button className="p-0.5">
            {isExpanded ? (
              <ChevronDown size={12} className="text-muted-foreground" />
            ) : (
              <ChevronRight size={12} className="text-muted-foreground" />
            )}
          </button>
        )}

        <div className="flex items-center gap-2 flex-1 min-w-0">
          {node.type === "folder" ? (
            isExpanded ? (
              <FolderOpen size={16} className="text-blue-400 flex-shrink-0" />
            ) : (
              <Folder size={16} className="text-blue-400 flex-shrink-0" />
            )
          ) : (
            getFileIcon(node.name, node.language)
          )}
          <span className="truncate text-sidebar-foreground group-hover:text-sidebar-accent-foreground">
            {node.name}
          </span>
        </div>
      </div>

      {node.type === "folder" && isExpanded && node.children && (
        <div>
          {node.children.map((child) => (
            <FileTreeNode key={child.id} node={child} level={level + 1} />
          ))}
        </div>
      )}
    </>
  );
};

export const FileExplorer = () => {
  return (
    <div className="flex flex-col h-full">
      <div className="p-3 border-b border-sidebar-border">
        <h3 className="text-sm font-medium text-sidebar-foreground">
          Explorer
        </h3>
      </div>

      <div className="flex-1 overflow-y-auto">
        <div className="p-1">
          {mockFileStructure.map((node) => (
            <FileTreeNode key={node.id} node={node} level={0} />
          ))}
        </div>
      </div>
    </div>
  );
};
