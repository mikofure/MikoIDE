import React, { useEffect, useRef, useCallback } from 'react';
import * as monaco from 'monaco-editor';
import { useAppDispatch, useAppSelector } from '../store';
import { updateTabContent, markTabSaved } from '../store/editorSlice';

interface DOMEditorProps {
  className?: string;
  theme?: string;
  options?: monaco.editor.IStandaloneEditorConstructionOptions;
  onEditorMount?: (editor: monaco.editor.IStandaloneCodeEditor) => void;
}

export const DOMEditor: React.FC<DOMEditorProps> = ({
  className = '',
  theme = 'vs-code-dark',
  options = {},
  onEditorMount,
}) => {
  const dispatch = useAppDispatch();
  const { tabs, activeTabId } = useAppSelector((state) => state.editor);
  const editorRef = useRef<HTMLDivElement>(null);
  const editorInstanceRef = useRef<monaco.editor.IStandaloneCodeEditor | null>(null);

  const activeTab = tabs.find(tab => tab.id === activeTabId);

  const handleEditorChange = useCallback((value: string) => {
    if (activeTabId) {
      dispatch(updateTabContent({ tabId: activeTabId, content: value }));
    }
  }, [activeTabId, dispatch]);

  const saveCurrentFile = useCallback(() => {
    if (activeTabId) {
      dispatch(markTabSaved(activeTabId));
      console.log('File saved:', activeTab?.title);
    }
  }, [activeTabId, activeTab?.title, dispatch]);

  // Initialize Monaco Editor
  useEffect(() => {
    if (!editorRef.current) return;

    const editor = monaco.editor.create(editorRef.current, {
      value: activeTab?.content || '',
      language: activeTab?.language || 'javascript',
      theme,
      fontSize: 14,
      lineNumbers: 'on',
      minimap: { enabled: true },
      scrollBeyondLastLine: false,
      wordWrap: 'on',
      automaticLayout: true,
      tabSize: 2,
      insertSpaces: true,
      renderWhitespace: 'selection',
      bracketPairColorization: { enabled: true },
      guides: {
        bracketPairs: true,
        indentation: true
      },
      ...options,
    });

    editorInstanceRef.current = editor;

    // Add keyboard shortcuts
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS, () => {
      saveCurrentFile();
    });

    // Listen for content changes
    editor.onDidChangeModelContent(() => {
      const value = editor.getValue();
      handleEditorChange(value);
    });

    // Call onEditorMount callback
    if (onEditorMount) {
      onEditorMount(editor);
    }

    return () => {
      editor.dispose();
      editorInstanceRef.current = null;
    };
  }, []); // Only run once on mount

  // Update editor content when active tab changes
  useEffect(() => {
    const editor = editorInstanceRef.current;
    if (!editor || !activeTab) return;

    const currentValue = editor.getValue();
    if (currentValue !== activeTab.content) {
      editor.setValue(activeTab.content);
    }

    // Update language
    const model = editor.getModel();
    if (model) {
      monaco.editor.setModelLanguage(model, activeTab.language);
    }
  }, [activeTab]);

  // Handle resize
  useEffect(() => {
    const editor = editorInstanceRef.current;
    if (!editor) return;

    const handleResize = () => {
      editor.layout();
    };

    const resizeObserver = new ResizeObserver(() => {
      setTimeout(handleResize, 5);
    });

    if (editorRef.current) {
      resizeObserver.observe(editorRef.current);
    }

    window.addEventListener('resize', handleResize);

    return () => {
      window.removeEventListener('resize', handleResize);
      resizeObserver.disconnect();
    };
  }, []);

  return (
    <div className={`dom-editor ${className}`} style={{ width: '100%', height: '100%' }}>
      <div ref={editorRef} style={{ width: '100%', height: '100%' }} />
    </div>
  );
};

export default DOMEditor;