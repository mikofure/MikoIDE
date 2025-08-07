import { createSignal, For, Show } from "solid-js";
import { Search, File, Folder, Settings, Command, Code, Edit, Copy, Scissors, RotateCcw, RotateCw } from "lucide-solid";
import * as monaco from "monaco-editor";

interface CommandPaletteProps {
    isOpen: boolean;
    searchQuery: string;
    onClose: () => void;
    onExecute: (command: string) => void;
    editorInstance?: monaco.editor.IStandaloneCodeEditor;
}

interface CommandItem {
    id: string;
    title: string;
    description?: string;
    icon: any;
    category: string;
    action: () => void;
}

function CommandPalette(props: CommandPaletteProps) {
    const [selectedIndex, setSelectedIndex] = createSignal(0);
    
    // Get all Monaco editor commands dynamically
    const getMonacoCommands = (): CommandItem[] => {
        if (!props.editorInstance) return [];
        
        try {
            // Get all available actions from Monaco editor
            const actions = props.editorInstance.getSupportedActions();
            
            // Map of action IDs to custom titles and descriptions
            const actionMap: Record<string, { title: string; description: string; icon: any }> = {
                'editor.action.formatDocument': { title: 'Format Document', description: 'Format the entire document', icon: Code },
                'editor.action.commentLine': { title: 'Toggle Line Comment', description: 'Toggle line comment', icon: Edit },
                'editor.action.copyLinesDownAction': { title: 'Copy Line Down', description: 'Copy line down', icon: Copy },
                'editor.action.copyLinesUpAction': { title: 'Copy Line Up', description: 'Copy line up', icon: Copy },
                'editor.action.deleteLines': { title: 'Delete Line', description: 'Delete current line', icon: Scissors },
                'undo': { title: 'Undo', description: 'Undo last action', icon: RotateCcw },
                'redo': { title: 'Redo', description: 'Redo last action', icon: RotateCw },
                'editor.action.selectAll': { title: 'Select All', description: 'Select all text', icon: Edit },
                'editor.action.gotoLine': { title: 'Go to Line', description: 'Go to specific line number', icon: Search },
                'editor.action.quickOutline': { title: 'Go to Symbol', description: 'Go to symbol in editor', icon: Search },
                'editor.action.quickCommand': { title: 'Command Palette', description: 'Show all commands', icon: Command },
                'editor.action.find': { title: 'Find', description: 'Find in current file', icon: Search },
                'editor.action.startFindReplaceAction': { title: 'Replace', description: 'Find and replace', icon: Edit },
                'editor.action.addCursorsToLineEnds': { title: 'Add Cursors to Line Ends', description: 'Add cursors to line ends', icon: Edit },
                'editor.action.addCursorsToBottom': { title: 'Add Cursors to Bottom', description: 'Add cursors to bottom', icon: Edit },
                'editor.action.addCursorsToTop': { title: 'Add Cursors to Top', description: 'Add cursors to top', icon: Edit },
                'editor.action.insertCursorAbove': { title: 'Add Cursor Above', description: 'Add cursor above', icon: Edit },
                'editor.action.insertCursorBelow': { title: 'Add Cursor Below', description: 'Add cursor below', icon: Edit },
                'editor.action.changeAll': { title: 'Change All Occurrences', description: 'Change all occurrences', icon: Edit },
                'editor.action.addSelectionToNextFindMatch': { title: 'Add Selection to Next Find Match', description: 'Add selection to next find match', icon: Search },
                'editor.action.addSelectionToPreviousFindMatch': { title: 'Add Selection to Previous Find Match', description: 'Add selection to previous find match', icon: Search },
                'editor.action.convertIndentationToSpaces': { title: 'Convert Indentation to Spaces', description: 'Convert indentation to spaces', icon: Code },
                'editor.action.convertIndentationToTabs': { title: 'Convert Indentation to Tabs', description: 'Convert indentation to tabs', icon: Code },
                'editor.action.tabDisplaySize': { title: 'Change Tab Display Size', description: 'Change tab display size', icon: Settings },
                'editor.action.moveLinesDownAction': { title: 'Move Line Down', description: 'Move line down', icon: Edit },
                'editor.action.moveLinesUpAction': { title: 'Move Line Up', description: 'Move line up', icon: Edit },
                'editor.action.duplicateSelection': { title: 'Duplicate Selection', description: 'Duplicate selection', icon: Copy },
                'editor.action.sortLinesAscending': { title: 'Sort Lines Ascending', description: 'Sort lines ascending', icon: Edit },
                'editor.action.sortLinesDescending': { title: 'Sort Lines Descending', description: 'Sort lines descending', icon: Edit },
                'editor.action.trimTrailingWhitespace': { title: 'Trim Trailing Whitespace', description: 'Trim trailing whitespace', icon: Code },
                'editor.action.transformToUppercase': { title: 'Transform to Uppercase', description: 'Transform to uppercase', icon: Edit },
                'editor.action.transformToLowercase': { title: 'Transform to Lowercase', description: 'Transform to lowercase', icon: Edit },
                'editor.action.transformToTitlecase': { title: 'Transform to Title Case', description: 'Transform to title case', icon: Edit },
                'editor.action.joinLines': { title: 'Join Lines', description: 'Join lines', icon: Edit },
                'editor.action.transpose': { title: 'Transpose Characters', description: 'Transpose characters around cursor', icon: Edit },
                'editor.action.clipboardCutAction': { title: 'Cut', description: 'Cut selection', icon: Scissors },
                'editor.action.clipboardCopyAction': { title: 'Copy', description: 'Copy selection', icon: Copy },
                'editor.action.clipboardPasteAction': { title: 'Paste', description: 'Paste from clipboard', icon: Edit },
                'editor.action.indentLines': { title: 'Indent Lines', description: 'Indent lines', icon: Code },
                'editor.action.outdentLines': { title: 'Outdent Lines', description: 'Outdent lines', icon: Code },
                'editor.action.blockComment': { title: 'Toggle Block Comment', description: 'Toggle block comment', icon: Edit },
                'editor.action.showHover': { title: 'Show Hover', description: 'Show hover information', icon: Search },
                'editor.action.showDefinitionPreviewHover': { title: 'Show Definition Preview', description: 'Show definition preview', icon: Search },
                'editor.action.revealDefinition': { title: 'Go to Definition', description: 'Go to definition', icon: Search },
                'editor.action.revealDeclaration': { title: 'Go to Declaration', description: 'Go to declaration', icon: Search },
                'editor.action.goToTypeDefinition': { title: 'Go to Type Definition', description: 'Go to type definition', icon: Search },
                'editor.action.goToImplementation': { title: 'Go to Implementation', description: 'Go to implementation', icon: Search },
                'editor.action.goToReferences': { title: 'Go to References', description: 'Go to references', icon: Search },
                'editor.action.rename': { title: 'Rename Symbol', description: 'Rename symbol', icon: Edit },
                'editor.action.refactor': { title: 'Refactor', description: 'Show refactor options', icon: Code },
                'editor.action.sourceAction': { title: 'Source Action', description: 'Show source actions', icon: Code },
                'editor.action.organizeImports': { title: 'Organize Imports', description: 'Organize imports', icon: Code },
                'editor.action.autoFix': { title: 'Auto Fix', description: 'Auto fix problems', icon: Code },
                'editor.action.fixAll': { title: 'Fix All', description: 'Fix all problems', icon: Code },
                'editor.action.quickFix': { title: 'Quick Fix', description: 'Show quick fixes', icon: Code },
                'editor.action.marker.next': { title: 'Go to Next Problem', description: 'Go to next problem', icon: Search },
                'editor.action.marker.prev': { title: 'Go to Previous Problem', description: 'Go to previous problem', icon: Search },
                'editor.action.wordHighlight.next': { title: 'Go to Next Highlight', description: 'Go to next highlight', icon: Search },
                'editor.action.wordHighlight.prev': { title: 'Go to Previous Highlight', description: 'Go to previous highlight', icon: Search },
                'editor.action.toggleWordWrap': { title: 'Toggle Word Wrap', description: 'Toggle word wrap', icon: Settings },
                'editor.action.toggleRenderWhitespace': { title: 'Toggle Render Whitespace', description: 'Toggle render whitespace', icon: Settings },
                'editor.action.toggleRenderControlCharacter': { title: 'Toggle Render Control Characters', description: 'Toggle render control characters', icon: Settings },
                'editor.action.toggleColumnSelection': { title: 'Toggle Column Selection', description: 'Toggle column selection mode', icon: Edit },
                'editor.action.selectHighlights': { title: 'Select All Highlights', description: 'Select all highlights', icon: Edit },
                'editor.action.changeAllOccurrences': { title: 'Change All Occurrences', description: 'Change all occurrences', icon: Edit },
                'editor.action.selectAllMatches': { title: 'Select All Matches', description: 'Select all matches', icon: Edit },
                'editor.action.expandLineSelection': { title: 'Expand Line Selection', description: 'Expand line selection', icon: Edit },
                'editor.action.smartSelect.expand': { title: 'Expand Selection', description: 'Expand selection', icon: Edit },
                'editor.action.smartSelect.shrink': { title: 'Shrink Selection', description: 'Shrink selection', icon: Edit },
                'cursorUndo': { title: 'Cursor Undo', description: 'Undo cursor position', icon: RotateCcw },
                'cursorRedo': { title: 'Cursor Redo', description: 'Redo cursor position', icon: RotateCw },
                'editor.fold': { title: 'Fold', description: 'Fold region', icon: Code },
                'editor.unfold': { title: 'Unfold', description: 'Unfold region', icon: Code },
                'editor.foldAll': { title: 'Fold All', description: 'Fold all regions', icon: Code },
                'editor.unfoldAll': { title: 'Unfold All', description: 'Unfold all regions', icon: Code },
                'editor.foldAllMarkerRegions': { title: 'Fold All Marker Regions', description: 'Fold all marker regions', icon: Code },
                'editor.unfoldAllMarkerRegions': { title: 'Unfold All Marker Regions', description: 'Unfold all marker regions', icon: Code },
                'editor.foldAllBlockComments': { title: 'Fold All Block Comments', description: 'Fold all block comments', icon: Code },
                'editor.foldLevel1': { title: 'Fold Level 1', description: 'Fold level 1', icon: Code },
                'editor.foldLevel2': { title: 'Fold Level 2', description: 'Fold level 2', icon: Code },
                'editor.foldLevel3': { title: 'Fold Level 3', description: 'Fold level 3', icon: Code },
                'editor.foldLevel4': { title: 'Fold Level 4', description: 'Fold level 4', icon: Code },
                'editor.foldLevel5': { title: 'Fold Level 5', description: 'Fold level 5', icon: Code },
                'editor.foldLevel6': { title: 'Fold Level 6', description: 'Fold level 6', icon: Code },
                'editor.foldLevel7': { title: 'Fold Level 7', description: 'Fold level 7', icon: Code },
                'editor.unfoldRecursively': { title: 'Unfold Recursively', description: 'Unfold recursively', icon: Code },
                'editor.foldRecursively': { title: 'Fold Recursively', description: 'Fold recursively', icon: Code }
            };
            
            return actions
                .filter(action => {
                    // Filter out actions we don't want to show
                    const excludeActions = [
                        'editor.action.quickCommand', // This would open Monaco's command palette
                        'editor.action.showContextMenu',
                        'editor.action.webvieweditor.showFind'
                    ];
                    return !excludeActions.includes(action.id) && action.id !== 'editor.action.quickCommand';
                })
                .map(action => {
                    const customInfo = actionMap[action.id];
                    return {
                        id: action.id,
                        title: customInfo?.title || action.label || action.id,
                        description: customInfo?.description || `Execute ${action.label || action.id}`,
                        icon: customInfo?.icon || Code,
                        category: "Editor",
                        action: () => {
                            try {
                                props.editorInstance?.getAction(action.id)?.run();
                            } catch (error) {
                                console.warn(`Failed to execute action ${action.id}:`, error);
                            }
                        }
                    };
                });
        } catch (error) {
            console.warn('Failed to get Monaco actions:', error);
            return [];
        }
    };
    
    // Sample commands - can be expanded
    const generalCommands: CommandItem[] = [
        {
            id: "file.new",
            title: "New File",
            description: "Create a new file",
            icon: File,
            category: "File",
            action: () => console.log("New file")
        },
        {
            id: "file.open",
            title: "Open File",
            description: "Open an existing file",
            icon: Folder,
            category: "File",
            action: () => console.log("Open file")
        },
        {
            id: "view.settings",
            title: "Open Settings",
            description: "Open IDE settings",
            icon: Settings,
            category: "View",
            action: () => console.log("Open settings")
        },
        {
            id: "search.files",
            title: "Search Files",
            description: "Search for files in workspace",
            icon: Search,
            category: "Search",
            action: () => console.log("Search files")
        }
    ];
    
    // Combine all commands
    const commands = () => [...generalCommands, ...getMonacoCommands()];
    
    // Filter commands based on search query
    const filteredCommands = () => {
        const allCommands = commands();
        if (!props.searchQuery.trim()) return allCommands;
        return allCommands.filter(cmd => 
            cmd.title.toLowerCase().includes(props.searchQuery.toLowerCase()) ||
            cmd.description?.toLowerCase().includes(props.searchQuery.toLowerCase()) ||
            cmd.category.toLowerCase().includes(props.searchQuery.toLowerCase())
        );
    };
    
    // Handle keyboard navigation
    const handleKeyDown = (e: KeyboardEvent) => {
        const filtered = filteredCommands();
        
        switch (e.key) {
            case "ArrowDown":
                e.preventDefault();
                setSelectedIndex(prev => (prev + 1) % filtered.length);
                break;
            case "ArrowUp":
                e.preventDefault();
                setSelectedIndex(prev => (prev - 1 + filtered.length) % filtered.length);
                break;
            case "Enter":
                e.preventDefault();
                if (filtered[selectedIndex()]) {
                    filtered[selectedIndex()].action();
                    props.onClose();
                }
                break;
            case "Escape":
                e.preventDefault();
                props.onClose();
                break;
        }
    };
    
    // Reset selected index when search query changes
    const resetSelection = () => {
        setSelectedIndex(0);
    };
    
    // Watch for search query changes
    (() => {
        props.searchQuery; // Track dependency
        resetSelection();
    })();
    
    return (
        <Show when={props.isOpen}>
            <div 
                class="absolute left-0 top-full mt-2 bg-[#1a1a1a98] backdrop-blur-lg border border-[#323132] shadow-lg rounded-md min-w-96 z-[999] py-1 max-h-80 overflow-y-auto"
                onKeyDown={handleKeyDown}
                tabIndex={-1}
            >
                <Show 
                    when={filteredCommands().length > 0}
                    fallback={
                        <div class="px-3 py-4 text-center text-gray-400 text-xs">
                            <Search class="w-6 h-6 mx-auto mb-2 opacity-50" />
                            No commands found for "{props.searchQuery}"
                        </div>
                    }
                >
                    <For each={filteredCommands()}>
                        {(command, index) => {
                            const IconComponent = command.icon;
                            return (
                                <button
                                    class={`w-full px-3 py-2 text-left text-xs hover:bg-white/10 transition-colors flex items-center gap-3 ${
                                        index() === selectedIndex() ? 'bg-white/10' : ''
                                    }`}
                                    onClick={() => {
                                        command.action();
                                        props.onClose();
                                    }}
                                    onMouseEnter={() => setSelectedIndex(index())}
                                >
                                    <IconComponent class="w-4 h-4 text-gray-400" />
                                    <div class="flex-1">
                                        <div class="text-white font-medium">{command.title}</div>
                                        <Show when={command.description}>
                                            <div class="text-gray-400 text-xs opacity-70">{command.description}</div>
                                        </Show>
                                    </div>
                                    <div class="text-gray-500 text-xs opacity-60">{command.category}</div>
                                </button>
                            );
                        }}
                    </For>
                </Show>
                
                {/* Footer with keyboard shortcuts */}
                <div class="border-t border-[#323132] mt-1 pt-2 px-3 pb-2">
                    <div class="flex items-center justify-between text-xs text-gray-500">
                        <div class="flex items-center gap-4">
                            <span class="flex items-center gap-1">
                                <kbd class="px-1 py-0.5 bg-gray-700 rounded text-xs">↑↓</kbd>
                                Navigate
                            </span>
                            <span class="flex items-center gap-1">
                                <kbd class="px-1 py-0.5 bg-gray-700 rounded text-xs">Enter</kbd>
                                Select
                            </span>
                            <span class="flex items-center gap-1">
                                <kbd class="px-1 py-0.5 bg-gray-700 rounded text-xs">Esc</kbd>
                                Close
                            </span>
                        </div>
                        <div class="flex items-center gap-1 opacity-60">
                            <Command class="w-3 h-3" />
                            <span>Command Palette</span>
                        </div>
                    </div>
                </div>
            </div>
        </Show>
    );
}

export default CommandPalette;