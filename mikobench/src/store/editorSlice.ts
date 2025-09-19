import { createSlice } from '@reduxjs/toolkit';
import type { PayloadAction } from '@reduxjs/toolkit';

export interface FileTab {
  id: string;
  title: string;
  content: string;
  language: string;
  isDirty: boolean;
  closable: boolean;
  filePath?: string;
}

export type ViewMode = 'code' | 'diff' | 'markdown' | 'split-horizontal' | 'split-vertical' | 'markdown-split';

interface EditorState {
  tabs: FileTab[];
  activeTabId: string | null;
  currentViewMode: ViewMode;
}

const initialState: EditorState = {
  tabs: [],
  activeTabId: null,
  currentViewMode: 'code',
};

const editorSlice = createSlice({
  name: 'editor',
  initialState,
  reducers: {
    addTab: (state, action: PayloadAction<FileTab>) => {
      const existingTab = state.tabs.find(tab => tab.id === action.payload.id);
      if (!existingTab) {
        state.tabs.push(action.payload);
      }
      state.activeTabId = action.payload.id;
    },
    
    removeTab: (state, action: PayloadAction<string>) => {
      const tabIndex = state.tabs.findIndex(tab => tab.id === action.payload);
      if (tabIndex === -1) return;
      
      state.tabs.splice(tabIndex, 1);
      
      // If closing active tab, switch to another tab
      if (state.activeTabId === action.payload && state.tabs.length > 0) {
        const newActiveIndex = Math.min(tabIndex, state.tabs.length - 1);
        state.activeTabId = state.tabs[newActiveIndex].id;
      } else if (state.tabs.length === 0) {
        state.activeTabId = null;
      }
    },
    
    setActiveTab: (state, action: PayloadAction<string>) => {
      const tab = state.tabs.find(tab => tab.id === action.payload);
      if (tab) {
        state.activeTabId = action.payload;
      }
    },
    
    updateTabContent: (state, action: PayloadAction<{ tabId: string; content: string }>) => {
      const tab = state.tabs.find(tab => tab.id === action.payload.tabId);
      if (tab) {
        tab.content = action.payload.content;
        tab.isDirty = true;
      }
    },
    
    updateTabTitle: (state, action: PayloadAction<{ tabId: string; title: string }>) => {
      const tab = state.tabs.find(tab => tab.id === action.payload.tabId);
      if (tab) {
        tab.title = action.payload.title;
      }
    },
    
    updateTabLanguage: (state, action: PayloadAction<{ tabId: string; language: string }>) => {
      const tab = state.tabs.find(tab => tab.id === action.payload.tabId);
      if (tab) {
        tab.language = action.payload.language;
      }
    },
    
    markTabSaved: (state, action: PayloadAction<string>) => {
      const tab = state.tabs.find(tab => tab.id === action.payload);
      if (tab) {
        tab.isDirty = false;
      }
    },
    
    setViewMode: (state, action: PayloadAction<ViewMode>) => {
      state.currentViewMode = action.payload;
    },
    
    setTabs: (state, action: PayloadAction<FileTab[]>) => {
      state.tabs = action.payload;
    },
  },
});

export const {
  addTab,
  removeTab,
  setActiveTab,
  updateTabContent,
  updateTabTitle,
  updateTabLanguage,
  markTabSaved,
  setViewMode,
  setTabs,
} = editorSlice.actions;

export default editorSlice.reducer;