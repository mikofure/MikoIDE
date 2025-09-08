import type { Component } from 'solid-js';
import { createEffect, createSignal, onCleanup, onMount } from 'solid-js';
import * as monaco from 'monaco-editor';

interface DiffEditorProps {
  original?: string;
  modified?: string;
  language?: string;
  theme?: string;
  onMount?: (editor: monaco.editor.IStandaloneDiffEditor) => void;
  options?: monaco.editor.IStandaloneDiffEditorConstructionOptions;
  width?: string | number;
  height?: string | number;
  readOnly?: boolean;
}

const DiffEditor: Component<DiffEditorProps> = (props) => {
  let containerRef: HTMLDivElement | undefined;
  let diffEditor: monaco.editor.IStandaloneDiffEditor | undefined;
  const [isEditorReady, setIsEditorReady] = createSignal(false);

  onMount(() => {
    if (containerRef) {
      // Configure Monaco Editor theme if not already defined
      try {
        monaco.editor.defineTheme('vs-code-dark', {
          base: 'vs-dark',
          inherit: true,
          rules: [],
          colors: {
            'editor.background': '#1e1e1e',
            'editor.foreground': '#d4d4d4',
            'editorLineNumber.foreground': '#858585',
            'editor.selectionBackground': '#264f78',
            'editor.inactiveSelectionBackground': '#3a3d41',
            'diffEditor.insertedTextBackground': '#9bb95533',
            'diffEditor.removedTextBackground': '#ff638433'
          }
        });
      } catch (e) {
        // Theme already exists
      }

      // Create models for original and modified content
      const originalModel = monaco.editor.createModel(
        props.original || '',
        props.language || 'javascript'
      );
      
      const modifiedModel = monaco.editor.createModel(
        props.modified || '',
        props.language || 'javascript'
      );

      // Create diff editor instance
      diffEditor = monaco.editor.createDiffEditor(containerRef, {
        theme: props.theme || 'vs-code-dark',
        automaticLayout: false,
        readOnly: props.readOnly || false,
        renderSideBySide: true,
        enableSplitViewResizing: true,
        renderIndicators: true,
        originalEditable: !props.readOnly,
        fontSize: 14,
        lineNumbers: 'on',
        renderWhitespace: 'selection',
        //@ts-expect-error
        tabSize: 2,
        insertSpaces: true,
        wordWrap: 'on',
        ...props.options
      });

      // Set the models
      diffEditor.setModel({
        original: originalModel,
        modified: modifiedModel
      });

      setIsEditorReady(true);
      
      // Call onMount callback if provided
      if (props.onMount) {
        props.onMount(diffEditor);
      }

      // Initial layout
      requestAnimationFrame(() => {
        if (diffEditor) {
          diffEditor.layout();
        }
      });
    }
  });

  // Update original content when props change
  createEffect(() => {
    if (diffEditor && isEditorReady() && props.original !== undefined) {
      const model = diffEditor.getModel();
      if (model && model.original) {
        const currentValue = model.original.getValue();
        if (currentValue !== props.original) {
          model.original.setValue(props.original);
        }
      }
    }
  });

  // Update modified content when props change
  createEffect(() => {
    if (diffEditor && isEditorReady() && props.modified !== undefined) {
      const model = diffEditor.getModel();
      if (model && model.modified) {
        const currentValue = model.modified.getValue();
        if (currentValue !== props.modified) {
          model.modified.setValue(props.modified);
        }
      }
    }
  });

  // Update language when props change
  createEffect(() => {
    if (diffEditor && isEditorReady() && props.language) {
      const model = diffEditor.getModel();
      if (model) {
        if (model.original) {
          monaco.editor.setModelLanguage(model.original, props.language);
        }
        if (model.modified) {
          monaco.editor.setModelLanguage(model.modified, props.language);
        }
      }
    }
  });

  // Update theme when props change
  createEffect(() => {
    if (diffEditor && isEditorReady() && props.theme) {
      monaco.editor.setTheme(props.theme);
    }
  });

  onCleanup(() => {
    if (diffEditor) {
      const model = diffEditor.getModel();
      if (model) {
        model.original?.dispose();
        model.modified?.dispose();
      }
      diffEditor.dispose();
    }
  });

  return (
    <div
      ref={containerRef}
      class={`absolute inset-0 w-full h-full monaco-diff-editor-container ${props.width ? '' : 'w-full'} ${props.height ? '' : 'h-full'}`}
      style={{
        width: typeof props.width === 'number' ? `${props.width}px` : props.width,
        height: typeof props.height === 'number' ? `${props.height}px` : props.height
      }}
    />
  );
};

export default DiffEditor;
export type { DiffEditorProps };