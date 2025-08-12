import { createSignal, onMount } from "solid-js";
import {  Folder, GitBranch, Settings,  Book, Star, Terminal, Lightbulb, Plus, FolderOpen, Zap } from "lucide-solid";
import chromeIPC from "../../data/chromeipc";
import { getComponentColors } from "../../data/theme/default";
import "../../appearance/theme/init";

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

  const [themeColors, setThemeColors] = createSignal({
    background: 'var(--bg-primary)',
    cardBackground: 'var(--bg-card)',
    titleText: 'var(--text-primary)',
    subtitleText: 'var(--text-secondary)',
    accentColor: 'var(--accent-primary)',
    hoverColor: 'var(--accent-primary-hover)',
    buttonBackground: 'var(--bg-button)',
    buttonHover: 'var(--bg-button-hover)',
    borderColor: 'var(--border-primary)',
    badgeBackground: 'var(--accent-blue)'
  });

  onMount(async () => {
    try {
      const colors = await getComponentColors('welcome');
      setThemeColors(colors as {
        background: string;
        cardBackground: string;
        titleText: string;
        subtitleText: string;
        accentColor: string;
        hoverColor: string;
        buttonBackground: string;
        buttonHover: string;
        borderColor: string;
        badgeBackground: string;
      });
    } catch (error) {
      console.warn('Failed to load theme colors, using fallback:', error);
    }
  });

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
    <div 
      class="flex items-center justify-center overflow-auto h-full select-none"
      style={{ 
        background: themeColors().background, 
        color: themeColors().titleText 
      }}
    >
      <div class="max-w-7xl mx-auto p-4">
        {/* Hero Section */}
        <div class="text-left p-6">
          <h1 
            class="text-4xl font-light mb-1 tracking-wide"
            style={{ color: themeColors().titleText }}
          >
            MikoIDE
          </h1>
          <p 
            class="text-sm mb-4"
            style={{ color: themeColors().subtitleText }}
          >
            A modern, cross-platform Integrated Development Environment (IDE) built with cutting-edge web technologies and native performance.
          </p>
          <div class="flex justify-start gap-4">
            <button
              onClick={handleNewFile}
              class="flex items-center text-xs gap-2 px-2 py-1 rounded-lg transition-colors font-medium"
              style={{ color: themeColors().titleText }}
            >
              <Plus class="w-5 h-5" />
              New File
            </button>
            <button
              onClick={handleOpenFolder}
              class="flex items-center text-xs gap-2 px-2 py-1 rounded-lg transition-colors font-medium border"
              style={{ 
                background: themeColors().buttonBackground,
                color: themeColors().titleText,
                "border-color": themeColors().borderColor
              }}
              onMouseEnter={(e) => (e.target as HTMLElement).style.background = themeColors().buttonHover}
              onMouseLeave={(e) => (e.target as HTMLElement).style.background = themeColors().buttonBackground}
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
            <h2 
              class="text-lg px-2 font-semibold mb-4 flex items-center gap-2"
              style={{ color: themeColors().titleText }}
            >
              <Zap class="w-5 h-5" style={{ color: themeColors().accentColor }} />
              Quick Actions
            </h2>
            <div class="space-y-0 w-full">
              <button
                onClick={handleCloneRepository}
                class="flex items-center gap-3 transition-colors text-left w-full p-2 rounded"
                style={{ color: themeColors().accentColor }}
                onMouseEnter={(e) => {
                  (e.target as HTMLElement).style.color = themeColors().hoverColor;
                  (e.target as HTMLElement).style.background = themeColors().cardBackground;
                }}
                onMouseLeave={(e) => {
                  (e.target as HTMLElement).style.color = themeColors().accentColor;
                  (e.target as HTMLElement).style.background = 'transparent';
                }}
              >
                <GitBranch class="w-4 h-4" />
                <span class="text-xs">Clone Git Repository</span>
              </button>
              <button 
                class="flex items-center gap-3 transition-colors text-left w-full p-2 rounded"
                style={{ color: themeColors().accentColor }}
                onMouseEnter={(e) => {
                  (e.target as HTMLElement).style.color = themeColors().hoverColor;
                  (e.target as HTMLElement).style.background = themeColors().cardBackground;
                }}
                onMouseLeave={(e) => {
                  (e.target as HTMLElement).style.color = themeColors().accentColor;
                  (e.target as HTMLElement).style.background = 'transparent';
                }}
              >
                <Settings class="w-4 h-4" />
                <span class="text-xs">Connect to Remote</span>
              </button>
              <button 
                class="flex items-center gap-3 transition-colors text-left w-full p-2 rounded"
                style={{ color: themeColors().accentColor }}
                onMouseEnter={(e) => {
                  (e.target as HTMLElement).style.color = themeColors().hoverColor;
                  (e.target as HTMLElement).style.background = themeColors().cardBackground;
                }}
                onMouseLeave={(e) => {
                  (e.target as HTMLElement).style.color = themeColors().accentColor;
                  (e.target as HTMLElement).style.background = 'transparent';
                }}
              >
                <Book class="w-4 h-4" />
                <span class="text-xs">New Workspace</span>
              </button>
              <button 
                class="flex items-center gap-3 transition-colors text-left w-full p-2 rounded"
                style={{ color: themeColors().accentColor }}
                onMouseEnter={(e) => {
                  (e.target as HTMLElement).style.color = themeColors().hoverColor;
                  (e.target as HTMLElement).style.background = themeColors().cardBackground;
                }}
                onMouseLeave={(e) => {
                  (e.target as HTMLElement).style.color = themeColors().accentColor;
                  (e.target as HTMLElement).style.background = 'transparent';
                }}
              >
                <Terminal class="w-4 h-4" />
                <span class="text-xs">Open Terminal</span>
              </button>
            </div>
          </div>

          {/* Recent Projects */}
          <div class="p-4">
            <h2 
              class="text-lg px-2 font-semibold mb-4 flex items-center gap-2"
              style={{ color: themeColors().titleText }}
            >
              <Folder class="w-5 h-5" style={{ color: themeColors().accentColor }} />
              Recent Projects
            </h2>
            <div class="space-y-0">
              {recentProjects().map((project) => (
                <button 
                  class="flex items-center justify-between w-full text-left px-2 py-1 rounded group transition-colors"
                  onMouseEnter={(e) => (e.target as HTMLElement).style.background = themeColors().cardBackground}
                  onMouseLeave={(e) => (e.target as HTMLElement).style.background = 'transparent'}
                >
                  <div class="flex items-center gap-3">
                    <Folder class="w-4 h-4" style={{ color: themeColors().accentColor }} />
                    <div class="flex flex-col">
                      <span 
                        class="text-sm font-medium"
                        style={{ color: themeColors().titleText }}
                      >
                        {project.name}
                      </span>
                      <span 
                        class="text-xs"
                        style={{ color: themeColors().subtitleText }}
                      >
                        {project.path}
                      </span>
                    </div>
                  </div>
                </button>
              ))}
              <button 
                class="flex items-center gap-2 transition-colors text-left w-full mt-4 p-2"
                style={{ color: themeColors().accentColor }}
                onMouseEnter={(e) => (e.target as HTMLElement).style.color = themeColors().hoverColor}
                onMouseLeave={(e) => (e.target as HTMLElement).style.color = themeColors().accentColor}
              >
                <span class="text-sm">Show more...</span>
              </button>
            </div>
          </div>

          {/* Learning Resources */}
          <div class="p-2">
            <h2 
              class="text-lg font-semibold mb-4 flex items-center gap-2"
              style={{ color: themeColors().titleText }}
            >
              <Star class="w-5 h-5" style={{ color: themeColors().accentColor }} />
              Learning Resources
            </h2>
            <div class="space-y-4">
              <div 
                class="border-l-2 pl-3"
                style={{ "border-color": themeColors().accentColor }}
              >
                <h3 
                  class="text-sm font-medium mb-1"
                  style={{ color: themeColors().titleText }}
                >
                  Get started with MikoIDE
                </h3>
                <p 
                  class="text-xs"
                  style={{ color: themeColors().subtitleText }}
                >
                  Learn the basics and start coding
                </p>
              </div>
              
              <div 
                class="border-l-2 pl-3"
                style={{ "border-color": themeColors().accentColor }}
              >
                <h3 
                  class="text-sm font-medium mb-1"
                  style={{ color: themeColors().titleText }}
                >
                  Accessibility Features
                </h3>
                <p 
                  class="text-xs"
                  style={{ color: themeColors().subtitleText }}
                >
                  Tools and shortcuts for accessibility
                </p>
              </div>
              
              <div class="flex items-start gap-2">
                <Terminal 
                  class="w-4 h-4 mt-0.5" 
                  style={{ color: themeColors().titleText }}
                />
                <div class="flex items-center gap-2">
                  <h3 
                    class="text-sm font-medium"
                    style={{ color: themeColors().titleText }}
                  >
                    PowerShell Guide
                  </h3>
                  <span 
                    class="text-white text-xs px-2 py-0.5 rounded"
                    style={{ background: themeColors().badgeBackground }}
                  >
                    New
                  </span>
                </div>
              </div>
              
              <div class="flex items-start gap-2">
                <Lightbulb 
                  class="w-4 h-4 mt-0.5" 
                  style={{ color: themeColors().titleText }}
                />
                <h3 
                  class="text-sm font-medium"
                  style={{ color: themeColors().titleText }}
                >
                  Learn the Fundamentals
                </h3>
              </div>
              
              <div class="flex items-start gap-2">
                <GitBranch 
                  class="w-4 h-4 mt-0.5" 
                  style={{ color: themeColors().titleText }}
                />
                <div class="flex items-center gap-2">
                  <h3 
                    class="text-sm font-medium"
                    style={{ color: themeColors().titleText }}
                  >
                    GitLens Guide
                  </h3>
                  <span 
                    class="text-white text-xs px-2 py-0.5 rounded"
                    style={{ background: themeColors().badgeBackground }}
                  >
                    Updated
                  </span>
                </div>
              </div>
              
              <button 
                class="flex items-center gap-2 transition-colors text-left w-full mt-4 p-2"
                style={{ color: themeColors().accentColor }}
                onMouseEnter={(e) => (e.target as HTMLElement).style.color = themeColors().hoverColor}
                onMouseLeave={(e) => (e.target as HTMLElement).style.color = themeColors().accentColor}
              >
                <span class="text-sm">Browse all tutorials...</span>
              </button>
            </div>
          </div>
        </div>

        {/* Footer */}
        <div 
          class="mt-8 pt-4 border-t text-center"
          style={{ "border-color": themeColors().borderColor }}
        >
          <p 
            class="text-xs"
            style={{ color: themeColors().subtitleText }}
          >
            Welcome to MikoIDE - Your accessible development environment
          </p>
          <div class="flex justify-center gap-4 mt-2">
            <button 
              class="text-xs transition-colors"
              style={{ color: themeColors().accentColor }}
              onMouseEnter={(e) => (e.target as HTMLElement).style.color = themeColors().hoverColor}
              onMouseLeave={(e) => (e.target as HTMLElement).style.color = themeColors().accentColor}
            >
              Documentation
            </button>
            <button 
              class="text-xs transition-colors"
              style={{ color: themeColors().accentColor }}
              onMouseEnter={(e) => (e.target as HTMLElement).style.color = themeColors().hoverColor}
              onMouseLeave={(e) => (e.target as HTMLElement).style.color = themeColors().accentColor}
            >
              Community
            </button>
            <button 
              class="text-xs transition-colors"
              style={{ color: themeColors().accentColor }}
              onMouseEnter={(e) => (e.target as HTMLElement).style.color = themeColors().hoverColor}
              onMouseLeave={(e) => (e.target as HTMLElement).style.color = themeColors().accentColor}
            >
              Support
            </button>
          </div>
        </div>
      </div>
    </div>
  );
}

export default Welcome;