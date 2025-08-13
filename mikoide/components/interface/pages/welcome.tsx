import { createSignal, onMount } from "solid-js";
import {  Folder, GitBranch, Settings,  Book, Star, Terminal, Lightbulb, Plus, FolderOpen, Zap } from "lucide-solid";
import chromeIPC from "../../../data/chromeipc";
import { getComponentColors } from "../../../data/theme/default";
import { useI18n } from "../../../i18n";
import "../../../appearance/theme/init";

interface RecentProject {
  name: string;
  path: string;
  lastOpened: Date;
}

interface WelcomeProps {
  onNewFile: () => void;
  onOpenFolder: () => void;
}

function Welcome(props: WelcomeProps) {
  const { t } = useI18n();
  const [recentProjects, setRecentProjects] = createSignal<RecentProject[]>([]);
  const [isLoadingProjects, setIsLoadingProjects] = createSignal(false);

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

  // Load recent projects from storage
  const loadRecentProjects = async () => {
    try {
      setIsLoadingProjects(true);
      const response = await chromeIPC.getSetting('recentProjects');
      if (response.success && response.data) {
        const projects = JSON.parse(response.data).map((p: any) => ({
          ...p,
          lastOpened: new Date(p.lastOpened)
        }));
        // Sort by last opened date (most recent first)
        projects.sort((a: RecentProject, b: RecentProject) => 
          b.lastOpened.getTime() - a.lastOpened.getTime()
        );
        setRecentProjects(projects.slice(0, 5)); // Show only 5 most recent
      }
    } catch (error) {
      console.warn('Failed to load recent projects:', error);
    } finally {
      setIsLoadingProjects(false);
    }
  };

  // Add project to recent list
  const addToRecentProjects = async (projectPath: string) => {
    try {
      const projectName = projectPath.split(/[\\/]/).pop() || 'Unknown Project';
      const newProject: RecentProject = {
        name: projectName,
        path: projectPath,
        lastOpened: new Date()
      };
      
      const currentProjects = recentProjects();
      // Remove if already exists
      const filteredProjects = currentProjects.filter(p => p.path !== projectPath);
      // Add to beginning
      const updatedProjects = [newProject, ...filteredProjects].slice(0, 10); // Keep max 10
      
      setRecentProjects(updatedProjects.slice(0, 5)); // Show only 5
      
      // Save to storage
      await chromeIPC.setSetting('recentProjects', JSON.stringify(updatedProjects));
    } catch (error) {
      console.error('Failed to add to recent projects:', error);
    }
  };

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
    
    // Load recent projects
    await loadRecentProjects();
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
      const response = await chromeIPC.executeMenuAction('file.open_folder');
      if (response.success && response.data?.folderPath) {
        await addToRecentProjects(response.data.folderPath);
      }
      props.onOpenFolder();
    } catch (error) {
      console.error('Failed to open folder:', error);
    }
  };

  const handleOpenRecentProject = async (projectPath: string) => {
    try {
      // Try to open the folder directly
      const response = await chromeIPC.listDirectory(projectPath);
      if (response.success) {
        await addToRecentProjects(projectPath);
        props.onOpenFolder();
      } else {
        console.error('Project folder no longer exists:', projectPath);
      }
    } catch (error) {
      console.error('Failed to open recent project:', error);
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
            {t('ui.welcome.title')}
          </h1>
          <p 
            class="text-sm mb-4"
            style={{ color: themeColors().subtitleText }}
          >
            {t('ui.welcome.subtitle')}
          </p>
          <div class="flex justify-start gap-4">
            <button
              onClick={handleNewFile}
              class="flex items-center text-xs gap-2 px-2 py-1 rounded-lg transition-colors font-medium"
              style={{ color: themeColors().titleText }}
            >
              <Plus class="w-5 h-5" />
              {t('ui.welcome.newFile')}
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
              {t('ui.welcome.openFolder')}
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
              {t('ui.welcome.quickActions')}
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
                <span class="text-xs">{t('ui.welcome.cloneRepository')}</span>
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
                <span class="text-xs">{t('ui.welcome.connectRemote')}</span>
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
                <span class="text-xs">{t('ui.welcome.newWorkspace')}</span>
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
                <span class="text-xs">{t('ui.welcome.openTerminal')}</span>
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
              {t('ui.welcome.recentProjects')}
            </h2>
            <div class="space-y-0">
              {isLoadingProjects() ? (
                <div class="flex items-center gap-2 px-2 py-4">
                  <div class="w-4 h-4 animate-spin rounded-full border-2 border-transparent border-t-current" style={{ color: themeColors().accentColor }}></div>
                  <span class="text-xs" style={{ color: themeColors().subtitleText }}>{t('ui.welcome.loadingRecentProjects')}</span>
                </div>
              ) : recentProjects().length > 0 ? (
                <>
                  {recentProjects().map((project) => (
                    <button 
                      class="flex items-center justify-between w-full text-left px-2 py-1 rounded group transition-colors"
                      onClick={() => handleOpenRecentProject(project.path)}
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
                      <span 
                        class="text-xs opacity-0 group-hover:opacity-100 transition-opacity"
                        style={{ color: themeColors().subtitleText }}
                      >
                        {project.lastOpened.toLocaleDateString()}
                      </span>
                    </button>
                  ))}
                  {recentProjects().length >= 5 && (
                    <button 
                      class="flex items-center gap-2 transition-colors text-left w-full mt-4 p-2"
                      style={{ color: themeColors().accentColor }}
                      onClick={loadRecentProjects}
                      onMouseEnter={(e) => (e.target as HTMLElement).style.color = themeColors().hoverColor}
                      onMouseLeave={(e) => (e.target as HTMLElement).style.color = themeColors().accentColor}
                    >
                      <span class="text-sm">{t('ui.welcome.refreshRecentProjects')}</span>
                    </button>
                  )}
                </>
              ) : (
                <div class="text-center py-4">
                  <span class="text-xs" style={{ color: themeColors().subtitleText }}>{t('ui.welcome.noRecentProjects')}</span>
                </div>
              )}
            </div>
          </div>

          {/* Learning Resources */}
          <div class="p-2">
            <h2 
              class="text-lg font-semibold mb-4 flex items-center gap-2"
              style={{ color: themeColors().titleText }}
            >
              <Star class="w-5 h-5" style={{ color: themeColors().accentColor }} />
              {t('ui.welcome.learningResources')}
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
                  {t('ui.welcome.getStarted')}
                </h3>
                <p 
                  class="text-xs"
                  style={{ color: themeColors().subtitleText }}
                >
                  {t('ui.welcome.getStartedDesc')}
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
                  {t('ui.welcome.accessibilityFeatures')}
                </h3>
                <p 
                  class="text-xs"
                  style={{ color: themeColors().subtitleText }}
                >
                  {t('ui.welcome.accessibilityDesc')}
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
                    {t('ui.welcome.powershellGuide')}
                  </h3>
                  <span 
                    class="text-white text-xs px-2 py-0.5 rounded"
                    style={{ background: themeColors().badgeBackground }}
                  >
                    {t('ui.welcome.badgeNew')}
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
                  {t('ui.welcome.learnFundamentals')}
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
                    {t('ui.welcome.gitlensGuide')}
                  </h3>
                  <span 
                    class="text-white text-xs px-2 py-0.5 rounded"
                    style={{ background: themeColors().badgeBackground }}
                  >
                    {t('ui.welcome.badgeUpdated')}
                  </span>
                </div>
              </div>
              
              <button 
                class="flex items-center gap-2 transition-colors text-left w-full mt-4 p-2"
                style={{ color: themeColors().accentColor }}
                onMouseEnter={(e) => (e.target as HTMLElement).style.color = themeColors().hoverColor}
                onMouseLeave={(e) => (e.target as HTMLElement).style.color = themeColors().accentColor}
              >
                <span class="text-sm">{t('ui.welcome.browseAllTutorials')}</span>
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
            {t('ui.welcome.footerText')}
          </p>
          <div class="flex justify-center gap-4 mt-2">
            <button 
              class="text-xs transition-colors"
              style={{ color: themeColors().accentColor }}
              onMouseEnter={(e) => (e.target as HTMLElement).style.color = themeColors().hoverColor}
              onMouseLeave={(e) => (e.target as HTMLElement).style.color = themeColors().accentColor}
            >
              {t('ui.welcome.documentation')}
            </button>
            <button 
              class="text-xs transition-colors"
              style={{ color: themeColors().accentColor }}
              onMouseEnter={(e) => (e.target as HTMLElement).style.color = themeColors().hoverColor}
              onMouseLeave={(e) => (e.target as HTMLElement).style.color = themeColors().accentColor}
            >
              {t('ui.welcome.community')}
            </button>
            <button 
              class="text-xs transition-colors"
              style={{ color: themeColors().accentColor }}
              onMouseEnter={(e) => (e.target as HTMLElement).style.color = themeColors().hoverColor}
              onMouseLeave={(e) => (e.target as HTMLElement).style.color = themeColors().accentColor}
            >
              {t('ui.welcome.support')}
            </button>
          </div>
        </div>
      </div>
    </div>
  );
}

export default Welcome;