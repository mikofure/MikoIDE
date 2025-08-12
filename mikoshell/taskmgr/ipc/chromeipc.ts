// IPC communication interface for Task Manager

export interface ProcessInfo {
  id: number;
  name: string;
  type: 'Browser' | 'Tab' | 'Extension' | 'GPU' | 'Utility' | 'Renderer' | 'Plugin';
  pid: number;
  cpuUsage: number;
  memoryUsage: number; // in MB
  networkUsage: number; // in KB/s
  status: 'Running' | 'Suspended' | 'Not Responding';
  url?: string;
  parentId?: number;
}

export interface SystemStats {
  totalCpuUsage: number;
  totalMemoryUsage: number;
  totalNetworkUsage: number;
}

// Global TaskManager interface (injected by C++ backend)
declare global {
  interface Window {
    TaskManager?: {
      getProcessList(): ProcessInfo[];
      endProcess(processId: number): boolean;
      getSystemStats(): SystemStats;
    };
    cefQuery?: (request: {
      request: string;
      persistent?: boolean;
      onSuccess: (response: string) => void;
      onFailure: (error_code: number, error_message: string) => void;
    }) => void;
  }
}

export class TaskManagerIPC {
  private static instance: TaskManagerIPC;
  
  private constructor() {}
  
  public static getInstance(): TaskManagerIPC {
    if (!TaskManagerIPC.instance) {
      TaskManagerIPC.instance = new TaskManagerIPC();
    }
    return TaskManagerIPC.instance;
  }
  
  /**
   * Check if TaskManager IPC is available
   */
  public isAvailable(): boolean {
    return typeof window !== 'undefined' && 
           typeof window.TaskManager !== 'undefined' &&
           typeof window.TaskManager.getProcessList === 'function';
  }
  
  /**
   * Get list of all processes
   */
  public async getProcessList(): Promise<ProcessInfo[]> {
    // Try V8 extension first
    if (this.isAvailable()) {
      try {
        const processes = window.TaskManager!.getProcessList();
        return processes || [];
      } catch (error) {
        console.error('Failed to get process list via V8:', error);
      }
    }
    
    // Fallback to message router
    if (typeof window.cefQuery === 'function') {
      try {
        const response = await this.sendCefQuery('get_processes', {});
        if (response.success && response.data) {
          return response.data;
        }
      } catch (error) {
        console.error('Failed to get process list via cefQuery:', error);
      }
    }
    
    console.warn('TaskManager IPC not available, returning mock data');
    return this.getMockProcessList();
  }
  
  /**
   * End a specific process
   */
  public async endProcess(processId: number): Promise<boolean> {
    // Try V8 extension first
    if (this.isAvailable()) {
      try {
        return window.TaskManager!.endProcess(processId);
      } catch (error) {
        console.error('Failed to end process via V8:', error);
      }
    }
    
    // Fallback to message router
    if (typeof window.cefQuery === 'function') {
      try {
        const response = await this.sendCefQuery('end_process', { processId });
        return response.success;
      } catch (error) {
        console.error('Failed to end process via cefQuery:', error);
      }
    }
    
    console.warn('TaskManager IPC not available, simulating process termination');
    return true;
  }
  
  /**
   * Get system statistics
   */
  public async getSystemStats(): Promise<SystemStats> {
    // Try V8 extension first
    if (this.isAvailable()) {
      try {
        return window.TaskManager!.getSystemStats();
      } catch (error) {
        console.error('Failed to get system stats via V8:', error);
      }
    }
    
    // Fallback to message router
    if (typeof window.cefQuery === 'function') {
      try {
        const response = await this.sendCefQuery('get_system_stats', {});
        if (response.success && response.data) {
          return response.data;
        }
      } catch (error) {
        console.error('Failed to get system stats via cefQuery:', error);
      }
    }
    
    console.warn('TaskManager IPC not available, returning mock stats');
    return this.getMockSystemStats();
  }
  
  /**
   * Mock process list for development/fallback
   */
  private getMockProcessList(): ProcessInfo[] {
    const baseProcesses = [
      { name: 'Toolchain Interpreter', type: 'Extension' as const, status: 'Running' as const },
      { name: 'CMake Build System', type: 'Utility' as const, status: 'Running' as const },
      { name: 'GCC Compiler', type: 'Utility' as const, status: 'Running' as const },
      { name: 'Python Interpreter', type: 'Utility' as const, status: 'Running' as const },
      { name: 'Node.js Runtime', type: 'Utility' as const, status: 'Running' as const },
      { name: 'Git Version Control', type: 'Utility' as const, status: 'Running' as const },
      { name: 'Package Manager', type: 'Utility' as const, status: 'Running' as const },
      { name: 'Build Cache Service', type: 'Utility' as const, status: 'Running' as const },
      { name: 'Dependency Resolver', type: 'Utility' as const, status: 'Running' as const },
      { name: 'Code Formatter', type: 'Plugin' as const, status: 'Running' as const },
      { name: 'Linter Service', type: 'Plugin' as const, status: 'Running' as const },
      { name: 'Debug Adapter', type: 'Plugin' as const, status: 'Suspended' as const },
      { name: 'Language Server', type: 'Plugin' as const, status: 'Running' as const },
    ];

    return baseProcesses.map((process, index) => ({
      ...process,
      id: index + 1,
      pid: Math.floor(Math.random() * 8000) + 2000,
      cpuUsage: Math.random() * (process.status === 'Suspended' ? 5 : 5),
      memoryUsage: Math.random() * (process.type === 'Utility' ? 150 : process.type === 'Plugin' ? 100 : 50) + 10,
      networkUsage: Math.random() * (process.type === 'Extension' ? 100 : process.type === 'Utility' ? 50 : 10),
      parentId: index === 0 ? undefined : 1,
    }));
  }
  
  /**
   * Send a CEF query message
   */
  private sendCefQuery(operation: string, data: any): Promise<any> {
    return new Promise((resolve, reject) => {
      if (!window.cefQuery) {
        reject(new Error('cefQuery not available'));
        return;
      }
      
      const request = `App::TaskManager ${JSON.stringify({ operation, ...data })}`;
      
      window.cefQuery({
        request,
        onSuccess: (response: string) => {
          try {
            const parsed = JSON.parse(response);
            resolve(parsed);
          } catch (error) {
            reject(new Error('Failed to parse response'));
          }
        },
        onFailure: (error_code: number, error_message: string) => {
          reject(new Error(`CEF Query failed: ${error_message} (${error_code})`));
        }
      });
    });
  }
  
  /**
   * Mock system stats for development/fallback
   */
  private getMockSystemStats(): SystemStats {
    return {
      totalCpuUsage: Math.random() * 100,
      totalMemoryUsage: Math.random() * 8192 + 1024,
      totalNetworkUsage: Math.random() * 1024,
    };
  }
}

// Export singleton instance
export const taskManagerIPC = TaskManagerIPC.getInstance();