import React from 'react';

const Navbar: React.FC = () => {
  return (
    <div className="w-10 h-full flex flex-col bg-[#2d2d30] border-r border-[#454545] flex-shrink-0">
      {/* Explorer */}
      <div className="w-full h-10 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-l-2 border-transparent hover:border-[#007acc] transition-colors">
        <i className="codicon codicon-files text-[#cccccc] text-base"></i>
      </div>
      
      {/* Search */}
      <div className="w-full h-10 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-l-2 border-transparent hover:border-[#007acc] transition-colors">
        <i className="codicon codicon-search text-[#cccccc] text-base"></i>
      </div>
      
      {/* Source Control */}
      <div className="w-full h-10 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-l-2 border-transparent hover:border-[#007acc] transition-colors">
        <i className="codicon codicon-source-control text-[#cccccc] text-base"></i>
      </div>
      
      {/* Run and Debug */}
      <div className="w-full h-10 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-l-2 border-transparent hover:border-[#007acc] transition-colors">
        <i className="codicon codicon-debug-alt text-[#cccccc] text-base"></i>
      </div>
      
      {/* Extensions */}
      <div className="w-full h-10 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-l-2 border-transparent hover:border-[#007acc] transition-colors">
        <i className="codicon codicon-extensions text-[#cccccc] text-base"></i>
      </div>
      
      {/* Settings */}
      <div className="w-full h-10 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-l-2 border-transparent hover:border-[#007acc] transition-colors mt-auto">
        <i className="codicon codicon-settings-gear text-[#cccccc] text-base"></i>
      </div>
    </div>
  );
};

export default Navbar;
