import React, { useEffect, useRef, useState } from 'react';
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

const DiffEditor: React.FC<DiffEditorProps> = (props) => {
  const [editor, setEditor] = useState<monaco.editor.IStandaloneDiffEditor | null>(null);
  const containerRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (!containerRef.current) return;

    // Create models for original and modified content
    const originalModel = monaco.editor.createModel(
      props.original || '',
      props.language || 'javascript'
    );
    const modifiedModel = monaco.editor.createModel(
      props.modified || '',
      props.language || 'javascript'
    );

    // Create the diff editor
    const diffEditor = monaco.editor.createDiffEditor(containerRef.current, {
      theme: props.theme || 'vs-dark',
      automaticLayout: true,
      readOnly: props.readOnly || false,
      ...props.options,
    });

    // Set the models
    diffEditor.setModel({
      original: originalModel,
      modified: modifiedModel,
    });

    setEditor(diffEditor);

    // Call onMount callback
    props.onMount?.(diffEditor);

    return () => {
      diffEditor.dispose();
      originalModel.dispose();
      modifiedModel.dispose();
    };
  }, []);

  // Update original content when props change
  useEffect(() => {
    if (editor && props.original !== undefined) {
      const model = editor.getModel();
      if (model?.original) {
        const currentValue = model.original.getValue();
        if (currentValue !== props.original) {
          model.original.setValue(props.original);
        }
      }
    }
  }, [editor, props.original]);

  // Update modified content when props change
  useEffect(() => {
    if (editor && props.modified !== undefined) {
      const model = editor.getModel();
      if (model?.modified) {
        const currentValue = model.modified.getValue();
        if (currentValue !== props.modified) {
          model.modified.setValue(props.modified);
        }
      }
    }
  }, [editor, props.modified]);

  // Update language when props change
  useEffect(() => {
    if (editor && props.language) {
      const model = editor.getModel();
      if (model?.original && model?.modified) {
        monaco.editor.setModelLanguage(model.original, props.language);
        monaco.editor.setModelLanguage(model.modified, props.language);
      }
    }
  }, [editor, props.language]);

  // Update theme when props change
  useEffect(() => {
    if (props.theme) {
      monaco.editor.setTheme(props.theme);
    }
  }, [props.theme]);

  // Handle container resize
  useEffect(() => {
    if (editor) {
      // Trigger layout recalculation
      setTimeout(() => {
        editor.layout();
      }, 0);
    }
  }, [editor]);

  return (
    <div
      ref={containerRef}
      style={{
        width: typeof props.width === 'number' ? `${props.width}px` : props.width || '100%',
        height: typeof props.height === 'number' ? `${props.height}px` : props.height || '100%',
      }}
    />
  );
};

export default DiffEditor;
export type { DiffEditorProps };