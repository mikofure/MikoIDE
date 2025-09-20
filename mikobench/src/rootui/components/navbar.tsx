import React from 'react';
import { File, FolderTree, Search, GitCommit, BugPlay, Blocks, Database, Terminal, Package, Wrench, TestTube, FileText, Globe, Eye, Hammer } from 'lucide-react';
const Navbar: React.FC = () => {
  const iconSize = 15;
  return (
    <div className="w-full h-16 pt-8 flex items-center bg-[#141414] border-b border-[#454545]">
      <div className='flex items-center h-full'>
        {/* Explorer */}
        <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
          <FolderTree size={iconSize} className='text-[#cccccc] text-base' />
        </div>

        {/* Search */}
        <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
          <Search size={iconSize} className='text-[#cccccc] text-base' />
        </div>

        {/* Source Control */}
        <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
          <GitCommit size={iconSize} className='text-[#cccccc] text-base' />
        </div>

        {/* Run and Debug */}
        <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
          <BugPlay size={iconSize} className='text-[#cccccc] text-base' />
        </div>

        {/* Extensions */}
        <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
          <Blocks size={iconSize} className='text-[#cccccc] text-base' />
        </div>

        {/* Separator Line */}
        <div className='mx-2 border-r border-[#454545] h-4' />

        {/* New File */}
        <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
          <File size={iconSize} className='text-[#cccccc] text-base' />
        </div>

        {/* Separator Line */}
        <div className='mx-2 border-r border-[#454545] h-4' />

        {/* Database/SQL Server */}
        <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
          <Database size={iconSize} className='text-[#cccccc] text-base' />
        </div>

        {/* Terminal/Command Prompt */}
        <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
          <Terminal size={iconSize} className='text-[#cccccc] text-base' />
        </div>

        {/* Package Manager/NuGet */}
        <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
          <Package size={iconSize} className='text-[#cccccc] text-base' />
        </div>

        {/* Test Explorer */}
        <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
          <TestTube size={iconSize} className='text-[#cccccc] text-base' />
        </div>

        {/* Tools/Diagnostics */}
        <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
          <Wrench size={iconSize} className='text-[#cccccc] text-base' />
        </div>

        {/* Documentation/Help */}
        <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
          <FileText size={iconSize} className='text-[#cccccc] text-base' />
        </div>

      </div>
      <div className="flex-1"></div>
      {/* Web Browser/Live Server */}
      <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
        <Globe size={iconSize} className='text-[#cccccc] text-base' />
      </div>

      {/* View */}
      <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
        <Eye size={iconSize} className='text-[#cccccc] text-base' />
      </div>

      {/* build */}
      <div className="h-full px-2 flex items-center justify-center hover:bg-[#37373d] cursor-pointer border-b-2 border-transparent hover:border-[#007acc] transition-colors">
        <Hammer size={iconSize} className='text-[#cccccc] text-base' />
      </div>
    </div>
  );
};

export default Navbar;
