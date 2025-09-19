import React, { useEffect, useRef, useState } from 'react';
import * as monaco from 'monaco-editor';


interface MonacoEditorProps {
  value?: string;
  language?: string;
  theme?: string;
  onChange?: (value: string) => void;
  onMount?: (editor: monaco.editor.IStandaloneCodeEditor) => void;
  options?: monaco.editor.IStandaloneEditorConstructionOptions;
  width?: string | number;
  height?: string | number;
}

const MonacoEditor: React.FC<MonacoEditorProps> = (props) => {
  const containerRef = useRef<HTMLDivElement>(null);
  const [editor, setEditor] = useState<monaco.editor.IStandaloneCodeEditor | null>(null);
  const [isEditorReady, setIsEditorReady] = useState(false);

  useEffect(() => {
    if (containerRef.current) {
      // Configure Monaco Editor
      monaco.editor.defineTheme('vs-code-dark', {
        base: 'vs-dark',
        inherit: true,
        rules: [],
        colors: {
          'editor.background': '#1e1e1e',
          'editor.foreground': '#d4d4d4',
          'editorLineNumber.foreground': '#858585',
          'editor.selectionBackground': '#264f78',
          'editor.inactiveSelectionBackground': '#3a3d41'
        }
      });

      // Create editor instance
      const editorInstance = monaco.editor.create(containerRef.current, {
        value: props.value || '',
        language: props.language || 'javascript',
        theme: props.theme || 'vs-code-dark',
        automaticLayout: false, // Disable automatic layout to handle manually
        minimap: { enabled: true },
        scrollBeyondLastLine: false,
        fontSize: 14,
        lineNumbers: 'on',
        renderWhitespace: 'selection',
        tabSize: 2,
        insertSpaces: true,
        wordWrap: 'on',
        ...props.options
      });

      setEditor(editorInstance);

      // Set up change listener
      const disposable = editorInstance.onDidChangeModelContent(() => {
        if (editorInstance && props.onChange) {
          props.onChange(editorInstance.getValue());
        }
      });

      // Note: ResizeObserver is handled by the parent workbench component
      // to avoid conflicts and ResizeObserver loops

      setIsEditorReady(true);
      
      // Call onMount callback if provided
      if (props.onMount) {
        props.onMount(editorInstance);
      }

      // Initial layout
      requestAnimationFrame(() => {
        if (editorInstance) {
          editorInstance.layout();
        }
      });

      return () => {
        disposable.dispose();
        editorInstance.dispose();
      };
    }
  }, []);

  // Update editor value when props change
  useEffect(() => {
    if (editor && isEditorReady && props.value !== undefined) {
      const currentValue = editor.getValue();
      if (currentValue !== props.value) {
        editor.setValue(props.value);
      }
    }
  }, [editor, isEditorReady, props.value]);

  // Update editor language when props change
  useEffect(() => {
    if (editor && isEditorReady && props.language) {
      const model = editor.getModel();
      if (model) {
        monaco.editor.setModelLanguage(model, props.language);
      }
    }
  }, [editor, isEditorReady, props.language]);

  // Update editor theme when props change
  useEffect(() => {
    if (editor && isEditorReady && props.theme) {
      monaco.editor.setTheme(props.theme);
    }
  }, [editor, isEditorReady, props.theme]);

  return (
    <div
      ref={containerRef}
      style={{
        width: typeof props.width === 'number' ? `${props.width}px` : props.width || '100%',
        height: typeof props.height === 'number' ? `${props.height}px` : props.height || '100%',
        position: 'absolute',
        top: '0',
        left: '0',
        right: '0',
        bottom: '0'
      }}
      className="monaco-editor-container"
    />
  );
};

export default MonacoEditor;
export type { MonacoEditorProps };