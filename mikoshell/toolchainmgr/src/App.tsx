import type { Component } from 'solid-js';
import { createSignal,  For, Show } from 'solid-js';
import { Download, Settings, CheckCircle, AlertCircle, ExternalLink, Folder, Monitor, Package } from 'lucide-solid';

interface Toolchain {
  id: string;
  name: string;
  description: string;
  version?: string;
  installed: boolean;
  category: 'compiler' | 'runtime' | 'build' | 'package';
  downloadUrl?: string;
  installLocation?: string;
  size?: string;
}

interface InstallTarget {
  id: string;
  name: string;
  path: string;
  description: string;
  recommended: boolean;
}

const App: Component = () => {
  const [selectedCategory, setSelectedCategory] = createSignal<string>('all');
  const [searchQuery, setSearchQuery] = createSignal<string>('');
  const [installTarget, setInstallTarget] = createSignal<string>('sandbox');
  const [isInstalling, setIsInstalling] = createSignal<string>('');

  // Mock data for toolchains
  const [toolchains] = createSignal<Toolchain[]>([
    {
      id: 'cmake',
      name: 'CMake',
      description: 'Cross-platform build system generator',
      version: '3.28.1',
      installed: false,
      category: 'build',
      downloadUrl: 'https://cmake.org/download/',
      size: '45 MB'
    },
    {
      id: 'python',
      name: 'Python',
      description: 'High-level programming language',
      version: '3.12.1',
      installed: true,
      category: 'runtime',
      downloadUrl: 'https://python.org/downloads/',
      installLocation: 'C:\\Python312',
      size: '120 MB'
    },
    {
      id: 'nodejs',
      name: 'Node.js',
      description: 'JavaScript runtime built on Chrome\'s V8 engine',
      version: '20.10.0',
      installed: false,
      category: 'runtime',
      downloadUrl: 'https://nodejs.org/download/',
      size: '85 MB'
    },
    {
      id: 'bun',
      name: 'Bun',
      description: 'Fast all-in-one JavaScript runtime',
      version: '1.0.15',
      installed: true,
      category: 'runtime',
      downloadUrl: 'https://bun.sh/',
      installLocation: 'C:\\Users\\user\\.bun',
      size: '65 MB'
    },
    {
      id: 'gcc',
      name: 'GNU Compiler Collection',
      description: 'Collection of compilers for C, C++, and other languages',
      version: '13.2.0',
      installed: false,
      category: 'compiler',
      downloadUrl: 'https://gcc.gnu.org/',
      size: '200 MB'
    },
    {
      id: 'clang',
      name: 'Clang',
      description: 'C language family frontend for LLVM',
      version: '17.0.6',
      installed: false,
      category: 'compiler',
      downloadUrl: 'https://clang.llvm.org/',
      size: '180 MB'
    },
    {
      id: 'git',
      name: 'Git',
      description: 'Distributed version control system',
      version: '2.43.0',
      installed: true,
      category: 'build',
      downloadUrl: 'https://git-scm.com/downloads',
      installLocation: 'C:\\Program Files\\Git',
      size: '95 MB'
    },
    {
      id: 'npm',
      name: 'npm',
      description: 'Node.js package manager',
      version: '10.2.4',
      installed: false,
      category: 'package',
      downloadUrl: 'https://npmjs.com/',
      size: '25 MB'
    }
  ]);

  const installTargets: InstallTarget[] = [
    {
      id: 'sandbox',
      name: 'Sandbox Toolchain',
      path: '~/.mikoide/toolchain',
      description: 'Isolated environment (Recommended)',
      recommended: true
    },
    {
      id: 'system',
      name: 'Local System',
      path: 'C:\\Program Files',
      description: 'System-wide installation (Not Recommended)',
      recommended: false
    }
  ];

  const categories = [
    { id: 'all', name: 'All Tools', icon: Package },
    { id: 'compiler', name: 'Compilers', icon: Settings },
    { id: 'runtime', name: 'Runtimes', icon: Monitor },
    { id: 'build', name: 'Build Tools', icon: Folder },
    { id: 'package', name: 'Package Managers', icon: Package }
  ];

  const filteredToolchains = () => {
    return toolchains().filter(tool => {
      const matchesCategory = selectedCategory() === 'all' || tool.category === selectedCategory();
      const matchesSearch = tool.name.toLowerCase().includes(searchQuery().toLowerCase()) ||
                           tool.description.toLowerCase().includes(searchQuery().toLowerCase());
      return matchesCategory && matchesSearch;
    });
  };

  const handleInstall = async (toolchain: Toolchain) => {
    setIsInstalling(toolchain.id);
    // Simulate installation process
    await new Promise(resolve => setTimeout(resolve, 2000));
    setIsInstalling('');
    // In a real implementation, this would update the toolchain state
  };

  const handleUninstall = async (toolchain: Toolchain) => {
    setIsInstalling(toolchain.id);
    // Simulate uninstallation process
    await new Promise(resolve => setTimeout(resolve, 1500));
    setIsInstalling('');
    // In a real implementation, this would update the toolchain state
  };

  return (
    <div class="h-screen bg-background text-text font-primary overflow-hidden select-none">
      {/* Header */}
      <header class="bg-surface border-b border-border px-4 py-2">
        <div class="flex items-center justify-between">
          <div>
            <h1 class="text-md font-semibold text-white">Toolchain Manager</h1>
            <p class="text-xs text-textSecondary mt-1">Install and manage development tools and toolchains</p>
          </div>
          <div class="flex items-center gap-4">
            <div class="flex items-center gap-2 text-sm text-textSecondary">
              <span>Install Target:</span>
              <select 
                class="bg-background border border-border rounded px-3 py-1 text-text focus:outline-none focus:ring-2 focus:ring-primary"
                value={installTarget()}
                onChange={(e) => setInstallTarget(e.target.value)}
              >
                <For each={installTargets}>
                  {(target) => (
                    <option value={target.id}>
                      {target.name} {target.recommended ? '(Recommended)' : ''}
                    </option>
                  )}
                </For>
              </select>
            </div>
          </div>
        </div>
      </header>

      <div class="flex h-[calc(100vh-80px)]">
        {/* Sidebar */}
        <aside class="w-64 bg-sidebar border-r border-border p-4">
          <div class="mb-6">
            <input
              type="text"
              placeholder="Search tools..."
              class="w-full bg-background border border-border rounded-lg px-3 py-2 text-sm text-text placeholder-textSecondary focus:outline-none focus:ring-2 focus:ring-primary"
              value={searchQuery()}
              onInput={(e) => setSearchQuery(e.target.value)}
            />
          </div>

          <nav class="space-y-1">
            <For each={categories}>
              {(category) => {
                const Icon = category.icon;
                return (
                  <button
                    class={`w-full flex items-center gap-3 px-3 py-2 rounded-lg text-sm font-medium transition-colors ${
                      selectedCategory() === category.id
                        ? 'bg-selected text-white'
                        : 'text-textSecondary hover:bg-hover hover:text-text'
                    }`}
                    onClick={() => setSelectedCategory(category.id)}
                  >
                    <Icon size={16} />
                    {category.name}
                  </button>
                );
              }}
            </For>
          </nav>

          <div class="mt-8 p-4 bg-background rounded-lg border border-border">
            <h3 class="text-sm font-medium text-white mb-2">Install Location</h3>
            <p class="text-xs text-textSecondary mb-3">
              {installTargets.find(t => t.id === installTarget())?.description}
            </p>
            <div class="text-xs text-textSecondary">
              <strong>Path:</strong><br />
              <code class="text-accent">{installTargets.find(t => t.id === installTarget())?.path}</code>
            </div>
          </div>
        </aside>

        {/* Main Content */}
        <main class="flex-1 p-6 overflow-y-auto">
          <div class="mb-6">
            <h2 class="text-lg font-semibold text-white mb-0">
              {categories.find(c => c.id === selectedCategory())?.name || 'All Tools'}
            </h2>
            <p class="text-sm text-textSecondary">
              {filteredToolchains().length} tools available
            </p>
          </div>

          <div class="grid gap-4">
            <For each={filteredToolchains()}>
              {(toolchain) => (
                <div class=" border border-border rounded-lg p-4 hover:border-primary/50 transition-colors">
                  <div class="flex items-start justify-between">
                    <div class="flex-1">
                      <div class="flex items-center gap-3 mb-0">
                        <h3 class="text-sm font-semibold text-white">{toolchain.name}</h3>
                        <Show when={toolchain.installed}>
                          <div class="flex items-center gap-1 px-2 py-0 text-success rounded text-xs font-medium">
                            <CheckCircle size={12} />
                            Installed
                          </div>
                        </Show>
                        <Show when={toolchain.version}>
                          <span class="px-2 py-0 bg-background text-textSecondary rounded text-xs font-mono">
                            v{toolchain.version}
                          </span>
                        </Show>
                      </div>
                      <p class="text-xs text-textSecondary mb-0">{toolchain.description}</p>
                      
                      <div class="flex items-center gap-4 text-sm text-textSecondary pt-2">
                        <Show when={toolchain.size}>
                          <span>Size: {toolchain.size}</span>
                        </Show>
                        <Show when={toolchain.installLocation}>
                          <span>Location: <code class="text-accent">{toolchain.installLocation}</code></span>
                        </Show>
                      </div>
                    </div>

                    <div class="flex items-center gap-2 ml-4">
                      <Show when={toolchain.downloadUrl}>
                        <button
                          class="flex items-center gap-2 px-3 py-1 bg-background border border-border rounded-lg text-sm text-textSecondary hover:text-text hover:border-primary/50 transition-colors"
                          onClick={() => window.open(toolchain.downloadUrl, '_blank')}
                        >
                          <ExternalLink size={14} />
                          Website
                        </button>
                      </Show>
                      
                      <Show when={!toolchain.installed}>
                        <button
                          class="flex items-center gap-2 px-4 py-1  hover:bg-primary/80 text-white rounded-lg text-sm font-medium transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
                          onClick={() => handleInstall(toolchain)}
                          disabled={isInstalling() === toolchain.id}
                        >
                          <Show when={isInstalling() === toolchain.id} fallback={<Download size={14} />}>
                            <div class="w-4 h-4 border-2 border-white/30 border-t-white rounded-full animate-spin" />
                          </Show>
                          {isInstalling() === toolchain.id ? 'Installing...' : 'Install'}
                        </button>
                      </Show>
                      
                      <Show when={toolchain.installed}>
                        <button
                          class="flex items-center gap-2 px-4 py-1  hover:bg-error/80 text-white rounded-lg text-sm font-medium transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
                          onClick={() => handleUninstall(toolchain)}
                          disabled={isInstalling() === toolchain.id}
                        >
                          <Show when={isInstalling() === toolchain.id} fallback={<AlertCircle size={14} />}>
                            <div class="w-4 h-4 border-2 border-white/30 border-t-white rounded-full animate-spin" />
                          </Show>
                          {isInstalling() === toolchain.id ? 'Uninstalling...' : 'Uninstall'}
                        </button>
                      </Show>
                    </div>
                  </div>
                </div>
              )}
            </For>
          </div>

          <Show when={filteredToolchains().length === 0}>
            <div class="text-center py-12">
              <Package size={48} class="mx-auto text-textSecondary mb-4" />
              <h3 class="text-lg font-medium text-white mb-2">No tools found</h3>
              <p class="text-textSecondary">Try adjusting your search or category filter.</p>
            </div>
          </Show>
        </main>
      </div>
    </div>
  );
};

export default App;