import type { Component } from 'solid-js';
import { createSignal, createEffect, For, Show, onMount, onCleanup } from 'solid-js';
import { X, RefreshCw, Search, AlertTriangle, Cpu, MemoryStick, Zap, ChevronDown, ChevronUp, Globe, FileText, Puzzle, Gamepad2, Settings, Monitor, Plug, HelpCircle } from 'lucide-solid';
import { taskManagerIPC, type ProcessInfo as IPCProcessInfo } from '../ipc/chromeipc';

// Use the ProcessInfo interface from IPC
type Process = IPCProcessInfo & {
  icon?: string;
};

type SortField = 'name' | 'cpuUsage' | 'memoryUsage' | 'networkUsage' | 'pid';
type SortDirection = 'asc' | 'desc';

const App: Component = () => {
  const [processes, setProcesses] = createSignal<Process[]>([]);
  const [filteredProcesses, setFilteredProcesses] = createSignal<Process[]>([]);
  const [searchQuery, setSearchQuery] = createSignal<string>('');
  const [selectedProcess, setSelectedProcess] = createSignal<number | null>(null);
  const [sortField, setSortField] = createSignal<SortField>('memoryUsage');
  const [sortDirection, setSortDirection] = createSignal<SortDirection>('desc');
  const [showOnlyTabs, setShowOnlyTabs] = createSignal<boolean>(false);
  const [autoRefresh, setAutoRefresh] = createSignal<boolean>(true);
  const [refreshInterval, setRefreshInterval] = createSignal<number>(2000);
  
  let refreshTimer: number | undefined;

  // Fetch processes from backend
  const fetchProcesses = async (): Promise<Process[]> => {
    try {
      const processes = await taskManagerIPC.getProcessList();
      return processes;
    } catch (error) {
      console.error('Failed to fetch processes:', error);
      return [];
    }
  };

  const refreshProcesses = async () => {
    const newProcesses = await fetchProcesses();
    setProcesses(newProcesses);
  };

  const sortProcesses = (processes: Process[]) => {
    return [...processes].sort((a, b) => {
      const field = sortField();
      const direction = sortDirection();
      const aVal = a[field];
      const bVal = b[field];
      
      let comparison = 0;
      if (typeof aVal === 'string' && typeof bVal === 'string') {
        comparison = aVal.localeCompare(bVal);
      } else {
        comparison = (aVal as number) - (bVal as number);
      }
      
      return direction === 'asc' ? comparison : -comparison;
    });
  };

  const filterProcesses = () => {
    let filtered = processes();
    
    if (showOnlyTabs()) {
      filtered = filtered.filter(p => p.type === 'Tab');
    }
    
    if (searchQuery()) {
      const query = searchQuery().toLowerCase();
      filtered = filtered.filter(p => 
        p.name.toLowerCase().includes(query) ||
        p.type.toLowerCase().includes(query) ||
        p.url?.toLowerCase().includes(query) ||
        p.pid.toString().includes(query)
      );
    }
    
    setFilteredProcesses(sortProcesses(filtered));
  };

  const handleSort = (field: SortField) => {
    if (sortField() === field) {
      setSortDirection(sortDirection() === 'asc' ? 'desc' : 'asc');
    } else {
      setSortField(field);
      setSortDirection('desc');
    }
  };

  const endProcess = async (processId: number) => {
    try {
      const success = await taskManagerIPC.endProcess(processId);
      if (success) {
        setProcesses(prev => prev.filter(p => p.id !== processId));
        if (selectedProcess() === processId) {
          setSelectedProcess(null);
        }
      } else {
        console.error('Failed to end process');
      }
    } catch (error) {
      console.error('Error ending process:', error);
    }
  };

  const getStatusColor = (status: Process['status']) => {
    switch (status) {
      case 'Running': return 'text-success';
      case 'Suspended': return 'text-warning';
      case 'Not Responding': return 'text-error';
      default: return 'text-textSecondary';
    }
  };

  const getTypeIcon = (type: Process['type']) => {
    switch (type) {
      case 'Browser': return <Globe size={16} class="text-primary" />;
      case 'Tab': return <FileText size={16} class="text-accent" />;
      case 'Extension': return <Puzzle size={16} class="text-warning" />;
      case 'GPU': return <Gamepad2 size={16} class="text-success" />;
      case 'Utility': return <Settings size={16} class="text-info" />;
      case 'Renderer': return <Monitor size={16} class="text-textSecondary" />;
      case 'Plugin': return <Plug size={16} class="text-error" />;
      default: return <HelpCircle size={16} class="text-textSecondary" />;
    }
  };

  const formatMemory = (mb: number) => {
    if (mb >= 1024) {
      return `${(mb / 1024).toFixed(1)} GB`;
    }
    return `${mb.toFixed(1)} MB`;
  };

  const formatNetwork = (kbps: number) => {
    if (kbps >= 1024) {
      return `${(kbps / 1024).toFixed(1)} MB/s`;
    }
    return `${kbps.toFixed(1)} KB/s`;
  };

  const getTotalStats = () => {
    const total = processes().reduce(
      (acc, p) => ({
        cpu: acc.cpu + p.cpuUsage,
        memory: acc.memory + p.memoryUsage,
        network: acc.network + p.networkUsage,
      }),
      { cpu: 0, memory: 0, network: 0 }
    );
    return total;
  };

  onMount(() => {
    refreshProcesses();
    
    if (autoRefresh()) {
      refreshTimer = window.setInterval(() => {
        if (autoRefresh()) {
          refreshProcesses();
        }
      }, refreshInterval());
    }
  });

  onCleanup(() => {
    if (refreshTimer) {
      clearInterval(refreshTimer);
    }
  });

  createEffect(() => {
    filterProcesses();
  });

  createEffect(() => {
    if (refreshTimer) {
      clearInterval(refreshTimer);
    }
    
    if (autoRefresh()) {
      refreshTimer = window.setInterval(() => {
        refreshProcesses();
      }, refreshInterval());
    }
  });

  const SortIcon = (props: { field: SortField }) => {
    if (sortField() !== props.field) return null;
    return sortDirection() === 'asc' ? <ChevronUp size={14} /> : <ChevronDown size={14} />;
  };

  return (
    <div class="min-h-screen bg-background text-text font-primary">
      {/* Header */}
      <header class="bg-surface border-b border-border px-2 py-2">
        <div class="flex items-center justify-between">
          <div>
            <h1 class="text-xs font-semibold text-white flex items-center gap-1">
              <Cpu size={16} class="text-primary" />
              Task Manager
            </h1>
            <p class="text-xs text-textSecondary mt-1">
              {processes().length} processes • {filteredProcesses().length} shown
            </p>
          </div>
          
          <div class="flex items-center gap-4">
            {/* Stats Summary */}
            <div class="flex items-center gap-6 text-xs">
              <div class="flex items-center gap-2">
                <Cpu size={16} class="text-info" />
                <span class="text-textSecondary">CPU:</span>
                <span class="font-mono text-white">{getTotalStats().cpu.toFixed(1)}%</span>
              </div>
              <div class="flex items-center gap-2">
                <MemoryStick size={16} class="text-warning" />
                <span class="text-textSecondary">Memory:</span>
                <span class="font-mono text-white">{formatMemory(getTotalStats().memory)}</span>
              </div>
              <div class="flex items-center gap-2">
                <Zap size={16} class="text-success" />
                <span class="text-textSecondary">Network:</span>
                <span class="font-mono text-white">{formatNetwork(getTotalStats().network)}</span>
              </div>
            </div>
            
            {/* Controls */}
            <div class="flex items-center gap-2">
              <button
                class="flex items-center gap-2 px-2 py-1 bg-background border border-border rounded-lg text-xs text-textSecondary hover:text-text hover:border-primary/50 transition-colors"
                onClick={refreshProcesses}
              >
                <RefreshCw size={14} />
                Refresh
              </button>
              
              <label class="flex items-center gap-2 text-xs text-textSecondary">
                <input
                  type="checkbox"
                  checked={autoRefresh()}
                  onChange={(e) => setAutoRefresh(e.target.checked)}
                  class="rounded border-border bg-background text-primary focus:ring-primary"
                />
                Auto-refresh
              </label>
            </div>
          </div>
        </div>
      </header>

      <div class="flex h-[calc(100vh-80px)]">
        {/* Sidebar */}
        <aside class="w-64 bg-sidebar border-r border-border p-4">
          <div class="space-y-4">
            {/* Search */}
            <div class="relative">
              <Search size={16} class="absolute left-2 top-1/2 transform -translate-y-1/2 text-textSecondary" />
              <input
                type="text"
                placeholder="Search processes..."
                class="w-full bg-background border border-border rounded-lg pl-8 pr-2 py-1 text-xs text-text placeholder-textSecondary focus:outline-none focus:ring-2 focus:ring-primary"
                value={searchQuery()}
                onInput={(e) => setSearchQuery(e.target.value)}
              />
            </div>

            {/* Filters */}
            <div class="space-y-2">
              <h3 class="text-xs font-medium text-white">Filters</h3>
              <label class="flex items-center gap-2 text-xs text-textSecondary">
                <input
                  type="checkbox"
                  checked={showOnlyTabs()}
                  onChange={(e) => setShowOnlyTabs(e.target.checked)}
                  class="rounded border-border bg-background text-primary focus:ring-primary"
                />
                Show only tabs
              </label>
            </div>

            {/* Refresh Settings */}
            <div class="space-y-2">
              <h3 class="text-xs font-medium text-white">Refresh Rate</h3>
              <select
                class="w-full bg-background border border-border rounded px-2 py-1 text-xs text-text focus:outline-none focus:ring-2 focus:ring-primary"
                value={refreshInterval()}
                onChange={(e) => setRefreshInterval(parseInt(e.target.value))}
              >
                <option value={1000}>1 second</option>
                <option value={2000}>2 seconds</option>
                <option value={5000}>5 seconds</option>
                <option value={10000}>10 seconds</option>
              </select>
            </div>

            {/* Selected Process Details */}
            <Show when={selectedProcess()}>
              {(processId) => {
                const process = () => processes().find(p => p.id === processId());
                return (
                  <div class="mt-6 p-4 bg-background rounded-lg border border-border">
                    <h3 class="text-sm font-medium text-white mb-3">Process Details</h3>
                    <Show when={process()}>
                      {(p) => (
                        <div class="space-y-2 text-xs">
                          <div>
                            <span class="text-textSecondary">Name:</span>
                            <span class="ml-2 text-white">{p().name}</span>
                          </div>
                          <div>
                            <span class="text-textSecondary">PID:</span>
                            <span class="ml-2 font-mono text-white">{p().pid}</span>
                          </div>
                          <div>
                            <span class="text-textSecondary">Type:</span>
                            <span class="ml-2 text-white">{p().type}</span>
                          </div>
                          <div>
                            <span class="text-textSecondary">Status:</span>
                            <span class={`ml-2 ${getStatusColor(p().status)}`}>{p().status}</span>
                          </div>
                          <Show when={p().url}>
                            <div>
                              <span class="text-textSecondary">URL:</span>
                              <div class="mt-1 text-accent text-xs break-all">{p().url}</div>
                            </div>
                          </Show>
                          <div class="pt-2 border-t border-border">
                            <button
                              class="w-full flex items-center justify-center gap-2 px-3 py-2 bg-error hover:bg-error/80 text-white rounded text-xs font-medium transition-colors"
                              onClick={() => endProcess(p().id)}
                            >
                              <X size={12} />
                              End Process
                            </button>
                          </div>
                        </div>
                      )}
                    </Show>
                  </div>
                );
              }}
            </Show>
          </div>
        </aside>

        {/* Main Content */}
        <main class="flex-1 overflow-hidden">
          <div class="h-full flex flex-col">
            {/* Table Header */}
            <div class="bg-surface border-b border-border px-2 py-2">
              <div class="grid grid-cols-12 gap-3 text-xs font-medium text-textSecondary">
                <button
                  class="col-span-4 flex items-center gap-1 text-left hover:text-text transition-colors"
                  onClick={() => handleSort('name')}
                >
                  Process
                  <SortIcon field="name" />
                </button>
                <button
                  class="col-span-1 flex items-center gap-1 text-center hover:text-text transition-colors"
                  onClick={() => handleSort('pid')}
                >
                  PID
                  <SortIcon field="pid" />
                </button>
                <button
                  class="col-span-2 flex items-center gap-1 text-right hover:text-text transition-colors"
                  onClick={() => handleSort('cpuUsage')}
                >
                  CPU
                  <SortIcon field="cpuUsage" />
                </button>
                <button
                  class="col-span-2 flex items-center gap-1 text-right hover:text-text transition-colors"
                  onClick={() => handleSort('memoryUsage')}
                >
                  Memory
                  <SortIcon field="memoryUsage" />
                </button>
                <button
                  class="col-span-2 flex items-center gap-1 text-right hover:text-text transition-colors"
                  onClick={() => handleSort('networkUsage')}
                >
                  Network
                  <SortIcon field="networkUsage" />
                </button>
                <div class="col-span-1 text-center">Actions</div>
              </div>
            </div>

            {/* Process List */}
            <div class="flex-1 overflow-y-auto">
              <For each={filteredProcesses()}>
                {(process) => (
                  <div
                    class={`grid grid-cols-12 gap-4 px-4 py-3 border-b border-border hover:bg-hover transition-colors cursor-pointer ${
                      selectedProcess() === process.id ? 'bg-selected' : ''
                    }`}
                    onClick={() => setSelectedProcess(process.id)}
                  >
                    {/* Process Name & Type */}
                    <div class="col-span-4 flex items-center gap-3">
                      {getTypeIcon(process.type)}
                      <div class="min-w-0 flex-1">
                        <div class="text-sm font-medium text-white truncate">{process.name}</div>
                        <div class="text-xs text-textSecondary">{process.type}</div>
                        <Show when={process.url}>
                          <div class="text-xs text-accent truncate">{process.url}</div>
                        </Show>
                      </div>
                      <Show when={process.status !== 'Running'}>
                        <AlertTriangle size={14} class={getStatusColor(process.status)} />
                      </Show>
                    </div>

                    {/* PID */}
                    <div class="col-span-1 flex items-center justify-center">
                      <span class="text-sm font-mono text-textSecondary">{process.pid}</span>
                    </div>

                    {/* CPU Usage */}
                    <div class="col-span-2 flex items-center justify-end">
                      <div class="text-right">
                        <div class="text-sm font-mono text-white">{process.cpuUsage.toFixed(1)}%</div>
                        <div class="w-16 h-1 bg-background rounded-full mt-1">
                          <div
                            class={`h-full rounded-full transition-all ${
                              process.cpuUsage > 50 ? 'bg-error' : process.cpuUsage > 20 ? 'bg-warning' : 'bg-success'
                            }`}
                            style={{ width: `${Math.min(process.cpuUsage, 100)}%` }}
                          />
                        </div>
                      </div>
                    </div>

                    {/* Memory Usage */}
                    <div class="col-span-2 flex items-center justify-end">
                      <div class="text-right">
                        <div class="text-sm font-mono text-white">{formatMemory(process.memoryUsage)}</div>
                        <div class="w-16 h-1 bg-background rounded-full mt-1">
                          <div
                            class={`h-full rounded-full transition-all ${
                              process.memoryUsage > 300 ? 'bg-error' : process.memoryUsage > 100 ? 'bg-warning' : 'bg-info'
                            }`}
                            style={{ width: `${Math.min((process.memoryUsage / 500) * 100, 100)}%` }}
                          />
                        </div>
                      </div>
                    </div>

                    {/* Network Usage */}
                    <div class="col-span-2 flex items-center justify-end">
                      <div class="text-right">
                        <div class="text-sm font-mono text-white">{formatNetwork(process.networkUsage)}</div>
                        <div class="w-16 h-1 bg-background rounded-full mt-1">
                          <div
                            class={`h-full rounded-full transition-all ${
                              process.networkUsage > 50 ? 'bg-error' : process.networkUsage > 20 ? 'bg-warning' : 'bg-success'
                            }`}
                            style={{ width: `${Math.min((process.networkUsage / 100) * 100, 100)}%` }}
                          />
                        </div>
                      </div>
                    </div>

                    {/* Actions */}
                    <div class="col-span-1 flex items-center justify-center">
                      <button
                        class="p-1 text-textSecondary hover:text-error hover:bg-error/20 rounded transition-colors"
                        onClick={(e) => {
                          e.stopPropagation();
                          endProcess(process.id);
                        }}
                        title="End Process"
                      >
                        <X size={14} />
                      </button>
                    </div>
                  </div>
                )}
              </For>

              <Show when={filteredProcesses().length === 0}>
                <div class="text-center py-12">
                  <Search size={48} class="mx-auto text-textSecondary mb-4" />
                  <h3 class="text-lg font-medium text-white mb-2">No processes found</h3>
                  <p class="text-textSecondary">Try adjusting your search or filter criteria.</p>
                </div>
              </Show>
            </div>
          </div>
        </main>
      </div>
    </div>
  );
};

export default App;