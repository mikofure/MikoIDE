import type { Component } from 'solid-js';
import { createEffect, createSignal, onMount } from 'solid-js';

interface MarkdownPreviewProps {
  content?: string;
  theme?: 'light' | 'dark';
  width?: string | number;
  height?: string | number;
  className?: string;
}

// Simple markdown parser for basic formatting
const parseMarkdown = (markdown: string): string => {
  if (!markdown) return '';
  
  let html = markdown
    // Headers
    .replace(/^### (.*$)/gim, '<h3>$1</h3>')
    .replace(/^## (.*$)/gim, '<h2>$1</h2>')
    .replace(/^# (.*$)/gim, '<h1>$1</h1>')
    // Bold
    .replace(/\*\*(.*?)\*\*/g, '<strong>$1</strong>')
    .replace(/__(.*?)__/g, '<strong>$1</strong>')
    // Italic
    .replace(/\*(.*?)\*/g, '<em>$1</em>')
    .replace(/_(.*?)_/g, '<em>$1</em>')
    // Code blocks
    .replace(/```([\s\S]*?)```/g, '<pre><code>$1</code></pre>')
    // Inline code
    .replace(/`(.*?)`/g, '<code>$1</code>')
    // Links
    .replace(/\[([^\]]+)\]\(([^\)]+)\)/g, '<a href="$2" target="_blank" rel="noopener noreferrer">$1</a>')
    // Images
    .replace(/!\[([^\]]*)\]\(([^\)]+)\)/g, '<img alt="$1" src="$2" style="max-width: 100%; height: auto;" />')
    // Strikethrough
    .replace(/~~(.*?)~~/g, '<del>$1</del>')
    // Horizontal rule
    .replace(/^---$/gim, '<hr>')
    // Blockquotes
    .replace(/^> (.*$)/gim, '<blockquote>$1</blockquote>')
    // Unordered lists
    .replace(/^\* (.*$)/gim, '<li>$1</li>')
    .replace(/^- (.*$)/gim, '<li>$1</li>')
    // Ordered lists
    .replace(/^\d+\. (.*$)/gim, '<li>$1</li>')
    // Line breaks
    .replace(/\n/g, '<br>');

  // Wrap consecutive <li> elements in <ul> or <ol>
  html = html.replace(/(<li>.*?<\/li>(?:<br>)*)+/g, (match) => {
    const items = match.replace(/<br>/g, '');
    return `<ul>${items}</ul>`;
  });

  // Wrap consecutive <blockquote> elements
  html = html.replace(/(<blockquote>.*?<\/blockquote>(?:<br>)*)+/g, (match) => {
    return match.replace(/<br>/g, '');
  });

  return html;
};

const MarkdownPreview: Component<MarkdownPreviewProps> = (props) => {
  const [parsedContent, setParsedContent] = createSignal('');
  let containerRef: HTMLDivElement | undefined;

  // Parse markdown content when it changes
  createEffect(() => {
    const content = props.content || '';
    setParsedContent(parseMarkdown(content));
  });

  onMount(() => {
    // Add syntax highlighting for code blocks if needed
    // This could be enhanced with libraries like Prism.js or highlight.js
  });

  const getThemeClasses = () => {
    const isDark = props.theme === 'dark';
    return `absolute inset-0 w-full h-full overflow-auto p-5 text-sm leading-relaxed font-sans box-border ${
      isDark 
        ? 'bg-[#1e1e1e] text-[#d4d4d4]' 
        : 'bg-white text-gray-800'
    }`;
  };

  const getContentStyles = () => {
    const isDark = props.theme === 'dark';
    return `
      .markdown-preview h1, .markdown-preview h2, .markdown-preview h3 {
        margin-top: 24px;
        margin-bottom: 16px;
        font-weight: 600;
        line-height: 1.25;
      }
      .markdown-preview h1 {
        font-size: 2em;
        border-bottom: 1px solid ${isDark ? '#30363d' : '#d1d9e0'};
        padding-bottom: 0.3em;
      }
      .markdown-preview h2 {
        font-size: 1.5em;
        border-bottom: 1px solid ${isDark ? '#30363d' : '#d1d9e0'};
        padding-bottom: 0.3em;
      }
      .markdown-preview h3 {
        font-size: 1.25em;
      }
      .markdown-preview code {
        background-color: ${isDark ? '#343942' : '#f6f8fa'};
        color: ${isDark ? '#e6edf3' : '#24292f'};
        padding: 0.2em 0.4em;
        border-radius: 6px;
        font-family: 'SFMono-Regular', Consolas, 'Liberation Mono', Menlo, monospace;
        font-size: 85%;
      }
      .markdown-preview pre {
        background-color: ${isDark ? '#0d1117' : '#f6f8fa'};
        border-radius: 6px;
        padding: 16px;
        overflow: auto;
        margin: 16px 0;
      }
      .markdown-preview pre code {
        background-color: transparent;
        padding: 0;
        border-radius: 0;
        font-size: 100%;
      }
      .markdown-preview blockquote {
        border-left: 4px solid ${isDark ? '#30363d' : '#d1d9e0'};
        padding-left: 16px;
        margin: 16px 0;
        color: ${isDark ? '#8b949e' : '#656d76'};
      }
      .markdown-preview ul, .markdown-preview ol {
        padding-left: 24px;
        margin: 16px 0;
      }
      .markdown-preview li {
        margin: 4px 0;
      }
      .markdown-preview hr {
        border: none;
        border-top: 1px solid ${isDark ? '#30363d' : '#d1d9e0'};
        margin: 24px 0;
      }
      .markdown-preview a {
        color: ${isDark ? '#58a6ff' : '#0969da'};
        text-decoration: none;
      }
      .markdown-preview a:hover {
        text-decoration: underline;
      }
      .markdown-preview img {
        max-width: 100%;
        height: auto;
        margin: 16px 0;
        border-radius: 6px;
      }
      .markdown-preview del {
        color: ${isDark ? '#8b949e' : '#656d76'};
      }
    `;
  };

  return (
    <>
      <style>{getContentStyles()}</style>
      <div
        ref={containerRef}
        class={`markdown-preview ${getThemeClasses()} ${props.className || ''}`}
        style={{
          width: typeof props.width === 'number' ? `${props.width}px` : props.width,
          height: typeof props.height === 'number' ? `${props.height}px` : props.height
        }}
        innerHTML={parsedContent()}
      />
    </>
  );
};

export default MarkdownPreview;
export type { MarkdownPreviewProps };