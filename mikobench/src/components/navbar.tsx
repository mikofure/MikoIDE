import type { Component } from 'solid-js';

const Navbar: Component = () => {
  return (
    <div class="w-10 h-full flex flex-col bg-[#2d2d30] border-r border-[#454545] flex-shrink-0">
      {/* Explorer */}
      <div class="w-full h-10 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-l-2 border-transparent hover:border-[#007acc] transition-colors">
        <i class="codicon codicon-files text-[#cccccc] text-base"></i>
      </div>
      
      {/* Search */}
      <div class="w-full h-10 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-l-2 border-transparent hover:border-[#007acc] transition-colors">
        <i class="codicon codicon-search text-[#cccccc] text-base"></i>
      </div>
      
      {/* Source Control */}
      <div class="w-full h-10 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-l-2 border-transparent hover:border-[#007acc] transition-colors">
        <i class="codicon codicon-source-control text-[#cccccc] text-base"></i>
      </div>
      
      {/* Run and Debug */}
      <div class="w-full h-10 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-l-2 border-transparent hover:border-[#007acc] transition-colors">
        <i class="codicon codicon-debug-alt text-[#cccccc] text-base"></i>
      </div>
      
      {/* Extensions */}
      <div class="w-full h-10 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-l-2 border-transparent hover:border-[#007acc] transition-colors">
        <i class="codicon codicon-extensions text-[#cccccc] text-base"></i>
      </div>
      
      {/* Spacer */}
      <div class="flex-1"></div>
      
      {/* Settings */}
      <div class="w-full h-10 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-l-2 border-transparent hover:border-[#007acc] transition-colors">
        <i class="codicon codicon-settings-gear text-[#cccccc] text-base"></i>
      </div>
    </div>
  );
};

export default Navbar
