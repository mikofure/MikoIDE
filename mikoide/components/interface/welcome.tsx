import { createSignal } from "solid-js";
import {  Folder, GitBranch, Settings,  Book, Star, Terminal, Lightbulb, Plus, FolderOpen, Zap } from "lucide-solid";
import chromeIPC from "../../data/chromeipc";

interface WelcomeProps {
  onNewFile: () => void;
  onOpenFolder: () => void;
}

function Welcome(props: WelcomeProps) {
  const [recentProjects] = createSignal([
    { name: "mikolite", path: "H:\\dev4" },
    { name: "mikoide-web", path: "H:\\dev4" },
    { name: "solid-boilerplate", path: "H:\\dev4" },
    { name: "learingfuck", path: "H:\\dev4" },
    { name: "mikojs", path: "H:\\dev4" }
  ]);

  const handleNewFile = async () => {
    try {
      await chromeIPC.newFile();
      props.onNewFile();
    } catch (error) {
      console.error('Failed to create new file:', error);
      props.onNewFile();
    }
  };

  const handleOpenFolder = async () => {
    try {
      await chromeIPC.executeMenuAction('file.open_folder');
      props.onOpenFolder();
    } catch (error) {
      console.error('Failed to open folder:', error);
    }
  };

  const handleCloneRepository = async () => {
    try {
      await chromeIPC.executeMenuAction('git.clone');
    } catch (error) {
      console.error('Failed to clone repository:', error);
    }
  };

  return (
    <div class="flex items-center justify-center bg-[#0e0e0e] text-[#cccccc] overflow-auto h-full select-none">

      <div class="max-w-7xl mx-auto p-4">
        {/* Hero Section */}
        <div class="text-left p-6">
          <h1 class="text-4xl font-light mb-1 text-[#cccccc] tracking-wide">MikoIDE</h1>
          <p class="text-[#8c8c8c] text-sm mb-4">A modern, cross-platform Integrated Development Environment (IDE) built with cutting-edge web technologies and native performance.</p>
          <div class="flex justify-start gap-4">
            <button
              onClick={handleNewFile}
              class="flex items-center text-xs gap-2 text-white px-2 py-1 rounded-lg transition-colors font-medium"
            >
              <Plus class="w-5 h-5" />
              New File
            </button>
            <button
              onClick={handleOpenFolder}
              class="flex items-center text-xs gap-2 bg-[#2d2d30] hover:bg-[#3e3e42] text-[#cccccc] px-2 py-1 rounded-lg transition-colors font-medium border border-[#464647]"
            >
              <FolderOpen class="w-5 h-5" />
              Open Folder
            </button>
          </div>
        </div>

        {/* Main Content Grid */}
        <div class="grid grid-cols-1 lg:grid-cols-3 gap-0">
          {/* Quick Actions */}
          <div class="p-4">
            <h2 class="text-lg px-2 font-semibold mb-4 text-[#cccccc] flex items-center gap-2">
              <Zap class="w-5 h-5 text-[#4fc3f7]" />
              Quick Actions
            </h2>
            <div class="space-y-0 w-full">
              <button
                onClick={handleCloneRepository}
                class="flex items-center gap-3 text-[#4fc3f7] hover:text-[#29b6f6] hover:bg-[#2a2d2e] transition-colors text-left w-full p-2 rounded"
              >
                <GitBranch class="w-4 h-4" />
                <span class="text-xs">Clone Git Repository</span>
              </button>
              <button class="flex items-center gap-3 text-[#4fc3f7] hover:text-[#29b6f6] hover:bg-[#2a2d2e] transition-colors text-left w-full p-2 rounded">
                <Settings class="w-4 h-4" />
                <span class="text-xs">Connect to Remote</span>
              </button>
              <button class="flex items-center gap-3 text-[#4fc3f7] hover:text-[#29b6f6] hover:bg-[#2a2d2e] transition-colors text-left w-full p-2 rounded">
                <Book class="w-4 h-4" />
                <span class="text-xs">New Workspace</span>
              </button>
              <button class="flex items-center gap-3 text-[#4fc3f7] hover:text-[#29b6f6] hover:bg-[#2a2d2e] transition-colors text-left w-full p-2 rounded">
                <Terminal class="w-4 h-4" />
                <span class="text-xs">Open Terminal</span>
              </button>
            </div>
          </div>

          {/* Recent Projects */}
          <div class="p-4">
            <h2 class="text-lg px-2 font-semibold mb-4 text-[#cccccc] flex items-center gap-2">
              <Folder class="w-5 h-5 text-[#4fc3f7]" />
              Recent Projects
            </h2>
            <div class="space-y-0">
              {recentProjects().map((project) => (
                <button class="flex items-center justify-between w-full text-left hover:bg-[#2a2d2e] px-2 py-1 rounded group transition-colors">
                  <div class="flex items-center gap-3">
                    <Folder class="w-4 h-4 text-[#4fc3f7]" />
                    <div class="flex flex-col">
                      <span class="text-[#cccccc] text-sm font-medium">{project.name}</span>
                      <span class="text-[#8c8c8c] text-xs">{project.path}</span>
                    </div>
                  </div>
                </button>
              ))}
              <button class="flex items-center gap-2 text-[#4fc3f7] hover:text-[#29b6f6] transition-colors text-left w-full mt-4 p-2">
                <span class="text-sm">Show more...</span>
              </button>
            </div>
          </div>

          {/* Learning Resources */}
          <div class="p-2">
            <h2 class="text-lg font-semibold mb-4 text-[#cccccc] flex items-center gap-2">
              <Star class="w-5 h-5 text-[#4fc3f7]" />
              Learning Resources
            </h2>
            <div class="space-y-4">
              <div class="border-l-2 border-[#4fc3f7] pl-3">
                <h3 class="text-[#cccccc] text-sm font-medium mb-1">Get started with MikoIDE</h3>
                <p class="text-[#8c8c8c] text-xs">Learn the basics and start coding</p>
              </div>
              
              <div class="border-l-2 border-[#4fc3f7] pl-3">
                <h3 class="text-[#cccccc] text-sm font-medium mb-1">Accessibility Features</h3>
                <p class="text-[#8c8c8c] text-xs">Tools and shortcuts for accessibility</p>
              </div>
              
              <div class="flex items-start gap-2">
                <Terminal class="w-4 h-4 text-[#cccccc] mt-0.5" />
                <div class="flex items-center gap-2">
                  <h3 class="text-[#cccccc] text-sm font-medium">PowerShell Guide</h3>
                  <span class="bg-[#0078d4] text-white text-xs px-2 py-0.5 rounded">New</span>
                </div>
              </div>
              
              <div class="flex items-start gap-2">
                <Lightbulb class="w-4 h-4 text-[#cccccc] mt-0.5" />
                <h3 class="text-[#cccccc] text-sm font-medium">Learn the Fundamentals</h3>
              </div>
              
              <div class="flex items-start gap-2">
                <GitBranch class="w-4 h-4 text-[#cccccc] mt-0.5" />
                <div class="flex items-center gap-2">
                  <h3 class="text-[#cccccc] text-sm font-medium">GitLens Guide</h3>
                  <span class="bg-[#0078d4] text-white text-xs px-2 py-0.5 rounded">Updated</span>
                </div>
              </div>
              
              <button class="flex items-center gap-2 text-[#4fc3f7] hover:text-[#29b6f6] transition-colors text-left w-full mt-4 p-2">
                <span class="text-sm">Browse all tutorials...</span>
              </button>
            </div>
          </div>
        </div>

        {/* Footer */}
        <div class="mt-8 text-center">
          <div class="flex justify-center items-center gap-6 text-[#8c8c8c] text-sm">
            <button class="hover:text-[#4fc3f7] transition-colors">Documentation</button>
            <span>•</span>
            <button class="hover:text-[#4fc3f7] transition-colors">Keyboard Shortcuts</button>
            <span>•</span>
            <button class="hover:text-[#4fc3f7] transition-colors">Release Notes</button>
            <span>•</span>
            <button class="hover:text-[#4fc3f7] transition-colors">GitHub</button>
          </div>
          <div class="flex justify-center items-center gap-6 text-[#8c8c8c] text-xs mt-2">
            <p>MikoIDE Version: 0.1.0</p>
            <p>Chromium Version: 120.0.6099.234</p>
            <p>CEF Version: 120.2.7</p>
          </div>
        </div>
      </div>
    </div>
  );
}

export default Welcome;